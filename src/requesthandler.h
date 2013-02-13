/*
* This work is licensed under the Creative Commons Attribution-ShareAlike 3.0 Unported License.
* To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/
* or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
*/

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
