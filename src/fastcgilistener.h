/*
* This work is licensed under the Creative Commons Attribution-ShareAlike 3.0 Unported License.
* To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/
* or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
*/

#ifndef FASTCGILISTENER_H
#define FASTCGILISTENER_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QtEndian>
#include <QtCore/QSysInfo>

#include <QtNetwork/QTcpServer>
#include <QtNetwork/QLocalServer>

enum message_type_t {
    TYPE_BEGIN_REQUEST     =  1,
    TYPE_ABORT_REQUEST     =  2,
    TYPE_END_REQUEST       =  3,
    TYPE_PARAMS            =  4,
    TYPE_STDIN             =  5,
    TYPE_STDOUT            =  6,
    TYPE_STDERR            =  7,
    TYPE_DATA              =  8,
    TYPE_GET_VALUES        =  9,
    TYPE_GET_VALUES_RESULT = 10,
    TYPE_UNKNOWN           = 11
};

enum protocol_status_t {
    FCGI_REQUEST_COMPLETE = 0,
    FCGI_CANT_MPX_CONN    = 1,
    FCGI_OVERLOADED       = 2,
    FCGI_UNKNOWN_ROLE     = 3
};

enum role_t {
    FCGI_RESPONDER  = 0,
    FCGI_AUTHORIZER = 1,
    FCGI_FILTER     = 2
};

struct FCGI_Header {
    quint8 version;
    quint8 type;
    quint8 requestIdB1;
    quint8 requestIdB0;
    quint8 contentLengthB1;
    quint8 contentLengthB0;
    quint8 paddingLength;
    quint8 reserved;

    FCGI_Header() {
        memset(this, 0, sizeof(*this));
    }

    FCGI_Header(message_type_t t, quint16 id, quint16 len) :
        version(1),
        type(t),
        requestIdB1(id >> 8),
        requestIdB0(id & 0xff),
        contentLengthB1(len >> 8),
        contentLengthB0(len & 0xff),
        paddingLength(0),
        reserved(0) {
    }
};

struct FCGI_EndRequestBody : public FCGI_Header {
    quint8 appStatusB3;
    quint8 appStatusB2;
    quint8 appStatusB1;
    quint8 appStatusB0;
    quint8 protocolStatus;
    quint8 reserved[3];

    FCGI_EndRequestBody() {
        memset(this, 0, sizeof(*this));
    }

    FCGI_EndRequestBody(quint16 id, quint32 appStatus, protocol_status_t protStatus) :
        FCGI_Header(TYPE_END_REQUEST, id, sizeof(FCGI_EndRequestBody)-sizeof(FCGI_Header)),
        appStatusB3((appStatus >> 24) & 0xff),
        appStatusB2((appStatus >> 16) & 0xff),
        appStatusB1((appStatus >>  8) & 0xff),
        appStatusB0((appStatus >>  0) & 0xff),
        protocolStatus(protStatus) {
        memset(this->reserved, 0, 3);
    }
};

typedef struct {
    quint8 roleB1;
    quint8 roleB0;
    quint8 flags;
    quint8 reserved[5];
} FCGI_BeginRequestBody;

struct FastCgiRequest{
    QMap<QString, QString> headers;
    QIODevice *deviceFcgi;
    int requestId;
    void SendResponse(const QByteArray &response);
};

class FastCgiListener : public QObject {
    Q_OBJECT
public:
    // cons und decon
    FastCgiListener(quint16 port, QObject *parent = 0);
    FastCgiListener(QString file, QObject *parent = 0);
    ~FastCgiListener();

    // public functions
    bool listen();

signals:
    void newRequest(FastCgiRequest *request);

public slots:
    void newConnection();
    void newDataAvailable();

private:
    // some private helper vars to determinate which type of Listener we have (Tcp-Socket-Listener or File-Socket-Listener)
    bool isTcpListener;
    bool isFileListener;

    // vars for tcp listener classtype
    QTcpServer *tcpServer;
    quint16 intPort;

    // vars for file socket classtype
    QLocalServer *localServer;
    QString strFile;
};

#endif // FASTCGILISTENER_H
