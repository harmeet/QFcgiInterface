/* 
 * This work is licensed under the Creative Commons Attribution-ShareAlike 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#ifndef FASTCGILISTENER_H
#define FASTCGILISTENER_H

// include some qt core stuff
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QtEndian>
#include <QtCore/QSysInfo>

// include some network stuff
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QLocalServer>


typedef struct {
	 quint8 version;
	 quint8 type;
	 quint16 requestId;
	 quint16 contentLength;
	 quint8 paddingLength;
	 quint8 reserved;
 } FCGI_Header;

typedef struct {
	unsigned char roleB1;
	unsigned char roleB0;
	unsigned char flags;
	unsigned char reserved[5];
} FCGI_BeginRequestBody;

typedef struct {
	unsigned char appStatusB3;
	unsigned char appStatusB2;
	unsigned char appStatusB1;
	unsigned char appStatusB0;
	unsigned char protocolStatus;
	unsigned char reserved[3];
} FCGI_EndRequestBody;



struct FastCgiRequest
{
	QMap<QString, QString> *headers;
	QIODevice *deviceFcgi;
	int requestId;
};


class FastCgiListener : public QObject
{
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
