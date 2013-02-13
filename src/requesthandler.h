#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#include <QObject>
#include "fastcgilistener.h"

class RequestHandler : public QObject
{
    Q_OBJECT
public:
    explicit RequestHandler(QObject *parent = 0);

signals:
    
public slots:
    void newRequest(FastCgiRequest *request);
};

#endif // REQUESTHANDLER_H
