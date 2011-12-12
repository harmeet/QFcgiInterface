/* 
 * This work is licensed under the Creative Commons Attribution-ShareAlike 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */


#include "fastcgilistener.h"

/*
 * Constructor for network listener
 */
FastCgiListener::FastCgiListener(quint16 port, QObject *parent) : QObject(parent)
{
	// save the needed values
	this->intPort = port;

	// init the listener and connect signal(s)
	this->tcpServer = new QTcpServer(this);
	this->connect(this->tcpServer, SIGNAL(newConnection()), this, SLOT(newConnection()));

	// set the helper vars
	this->isFileListener = false;
	this->isTcpListener = true;
}

/*
 * Constructor for file descriptor
 */
FastCgiListener::FastCgiListener(QString file, QObject *parent) : QObject(parent)
{
	// save the needed values
	this->strFile = file;

	// init the listener and connect needed signal(s)
	this->localServer = new QLocalServer(this);
	this->connect(this->localServer, SIGNAL(newConnection()), this, SLOT(newConnection()));

	// set the helper vars
	this->isFileListener = true;
	this->isTcpListener = false;
}


/*
 * Deconstructor
 */
FastCgiListener::~FastCgiListener()
{

}


/*
 * listen - start listening on server or on file
 */
bool FastCgiListener::listen()
{
	// if class was constructed as tcp listener, listen on given port
	// otherwise listen on filesocket
	if(this->isTcpListener) {
		return this->tcpServer->listen(QHostAddress::Any, this->intPort);
	} else {
		return this->localServer->listen(this->strFile);
	}
}

/*
 * newConnection - handle new Connections which from tcp or local listener
 */
void FastCgiListener::newConnection()
{
	// check which classtype was constructed, and get the next pending connection from the right Object
	// new connections will be down cast to QIODevice!
	QIODevice *newClient;
	if(this->isTcpListener) {
		newClient = (QIODevice*)this->tcpServer->nextPendingConnection();
	} else {
		newClient = (QIODevice*)this->localServer->nextPendingConnection();
	}

	// make it possible to handle the incoming data on client asyncronly
	this->connect(newClient, SIGNAL(readyRead()), this, SLOT(newDataAvailable()));
}

void FastCgiListener::newDataAvailable()
{
	// cast the sender to QIODevice, because both Sender types inherit from it!
	QIODevice *client = (QIODevice*)this->sender();

	// construct FastCgiRequest
	FastCgiRequest *fcgiRequest = new FastCgiRequest;
	fcgiRequest->headers = new QMap<QString, QString>();
	fcgiRequest->deviceFcgi = client;


	/// Read Header
	FCGI_Header *header = new FCGI_Header;
	memcpy(header, client->read(sizeof(FCGI_Header)).data(), sizeof(FCGI_Header));

	// normalize byte order
	#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
		header->contentLength = qFromBigEndian(header->contentLength);
		header->requestId = qFromBigEndian(header->requestId);
	#endif

	// read content and padding length
	char* strContentLength = (char*)malloc(header->contentLength);
	memcpy(strContentLength, client->read(header->contentLength).data(), header->contentLength);

	char* strPaddingLength = (char*)malloc(header->paddingLength);
	memcpy(strPaddingLength, client->read(header->paddingLength).data(), header->paddingLength);

	/// Read Body
	FCGI_BeginRequestBody *body = new FCGI_BeginRequestBody;
	memcpy(body, client->read(sizeof(FCGI_BeginRequestBody)).data(), sizeof(FCGI_BeginRequestBody));

	int bla2 = client->bytesAvailable();

	// read http key value pairs until only the end Header is still available
	int sizeofEndRequest = sizeof(FCGI_Header) + sizeof(FCGI_EndRequestBody);
	while(client->bytesAvailable() != sizeofEndRequest)
	{
		bla2 = client->bytesAvailable();


		/// Parse Key length
		quint32 lenKey = 0;
		unsigned char* charlenKey = (unsigned char*)client->read(1).data();
		if(*charlenKey >> 7 == 1) {
			unsigned char* strlenKey2 = (unsigned char*)client->read(3).data();
			lenKey =  ((charlenKey[0] & 0x7f) << 24) + (strlenKey2[0] << 16) + (strlenKey2[1] << 8) + strlenKey2[2];
		} else {
			lenKey =  charlenKey[0];
		}

		/// Parse Value length
		quint32 lenValue = 0;
		unsigned char* charlenValue = (unsigned char*)client->read(1).data();
		if(*charlenValue >> 7 == 1) {
			unsigned char* strlenValue2 = (unsigned char*)client->read(3).data();
			lenValue =  ((charlenValue[0] & 0x7f) << 24) + (strlenValue2[0] << 16) + (strlenValue2[1] << 8) + strlenValue2[2];
		} else {
			lenValue = charlenValue[0];
		}

		// determinate key and value string and append it to the header list
		QString strKey(client->read(lenKey));
		QString strValue(client->read(lenValue));

		// check for content length and increase the length of the end
		if(strKey == "CONTENT_LENGTH") {
			sizeofEndRequest += strValue.toInt();
		}

		// check for content length and increase the length of the end
		if(strKey == "HTTP_HOST") {
			bla2 = client->bytesAvailable();
			QByteArray array = client->readAll();
			int bla = array.length();
			bla = 0;
		}




		fcgiRequest->headers->insert(strKey, strValue);
	}

	int bla = 0;



}








