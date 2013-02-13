/* 
 * This work is licensed under the Creative Commons Attribution-ShareAlike 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include <QtCore/QCoreApplication>
#include <QByteArray>

#include "fastcgilistener.h"
#include "requesthandler.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    FastCgiListener listener(9000, &a);
    listener.listen();

    RequestHandler handler;

    handler.connect(&listener, SIGNAL(newRequest(FastCgiRequest*)),
                    &handler, SLOT(newRequest(FastCgiRequest*)));

    return a.exec();
}
