/*
* This work is licensed under the Creative Commons Attribution-ShareAlike 3.0 Unported License.
* To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/
* or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
*/

#include "fastcgilistener.h"

FastCgiListener::FastCgiListener(quint16 port, QObject *parent) : QObject(parent) {
    this->intPort = port;

    this->tcpServer = new QTcpServer(this);
    this->connect(this->tcpServer, SIGNAL(newConnection()), this, SLOT(newConnection()));

    this->isFileListener = false;
    this->isTcpListener = true;
}

FastCgiListener::FastCgiListener(QString file, QObject *parent) : QObject(parent) {
    this->strFile = file;

    this->localServer = new QLocalServer(this);
    this->connect(this->localServer, SIGNAL(newConnection()), this, SLOT(newConnection()));

    this->isFileListener = true;
    this->isTcpListener = false;
}

FastCgiListener::~FastCgiListener() {
    delete this->tcpServer;
    delete this->localServer;
}

bool FastCgiListener::listen() {
    if(this->isTcpListener) {
        return this->tcpServer->listen(QHostAddress::Any, this->intPort);
    } else {
        return this->localServer->listen(this->strFile);
    }
}

void FastCgiListener::newConnection() {
    QIODevice *newClient;
    if(this->isTcpListener) {
        newClient = (QIODevice*)this->tcpServer->nextPendingConnection();
    } else {
        newClient = (QIODevice*)this->localServer->nextPendingConnection();
    }
    this->connect(newClient, SIGNAL(readyRead()), this, SLOT(newDataAvailable()));
}

void FastCgiListener::newDataAvailable() {
    QIODevice *client = (QIODevice*)this->sender();

    FastCgiRequest *fcgiRequest = new FastCgiRequest;
    fcgiRequest->deviceFcgi = client;

    int contentLength = 0;
    FCGI_Header header;
    FCGI_BeginRequestBody body;

    /// Read FCGI_BEGIN_REQUEST
    client->read((char *)&header, sizeof(FCGI_Header));
    contentLength = header.contentLengthB0 | (header.contentLengthB1<<8);
    if (contentLength != sizeof(FCGI_BeginRequestBody)) {
        delete fcgiRequest;
        return;
    }
    client->read((char *)&body, sizeof(FCGI_BeginRequestBody));

    /// Read FCGI_PARAMS
    client->read((char *)&header, sizeof(FCGI_Header));
    contentLength = header.contentLengthB0 | (header.contentLengthB1<<8);

    while (client->bytesAvailable() > 0) {
        unsigned char charlenKey[5];
        unsigned char charlenVal[5];
        quint32 lenKey = 0;
        quint32 lenVal = 0;

        /// Parse Key length
        client->read((char *)&charlenKey[0], 1);
        if (charlenKey[0]) {
            if (charlenKey[0] >> 7 == 1) {
                client->read((char *)&charlenKey[1], 3);
                lenKey = ((charlenKey[0] & 0x7f) << 24) + (charlenKey[1] << 16) + (charlenKey[2] << 8) + charlenKey[3];
            } else {
                lenKey = charlenKey[0];
            }

            /// Parse Value length
            client->read((char *)&charlenVal[0], 1);
            if (charlenVal[0] >> 7 == 1) {
                client->read((char *)&charlenVal[1], 3);
                lenVal = ((charlenVal[0] & 0x7f) << 24) + (charlenVal[1] << 16) + (charlenVal[2] << 8) + charlenVal[3];
            } else {
                lenVal = charlenVal[0];
            }

            QString strKey(client->read(lenKey));
            QString strValue(client->read(lenVal));
            fcgiRequest->headers.insert(strKey, strValue);
        } else if (client->bytesAvailable() > (header.paddingLength-1)) {
            char padding[256];
            client->read(padding, header.paddingLength-1);

            // Read next header
            client->read((char *)&header, sizeof(FCGI_Header));
            contentLength = header.contentLengthB0 | (header.contentLengthB1<<8);

            if ((header.type == TYPE_PARAMS) && (contentLength > 0)) {
                continue;
            } else {
                break;
            }
        }
    }

    /// Read FCGI_STDIN
    client->read((char *)&header, sizeof(FCGI_Header));

    if(client->bytesAvailable() > 0) {
        QByteArray data = client->readAll();
        qDebug() << "WARNING: " << data.length() << "bytes of data ignored!";
    }

    emit newRequest(fcgiRequest);
}

void FastCgiRequest::SendResponse(const QByteArray &response) {
    FCGI_Header fcgi_hdr(TYPE_STDOUT, 1, response.length());
    deviceFcgi->write((const char *)&fcgi_hdr, sizeof(fcgi_hdr));
    deviceFcgi->write(response);

    FCGI_EndRequestBody fcgi_end(1, 0, FCGI_REQUEST_COMPLETE);
    deviceFcgi->write((const char *)&fcgi_end, sizeof(fcgi_end));
    deviceFcgi->close();

    delete this;
}
