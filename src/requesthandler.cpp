#include "requesthandler.h"

RequestHandler::RequestHandler(QObject *parent) :
    QObject(parent)
{
}

void RequestHandler::newRequest(FastCgiRequest *request) {
    QByteArray response;
    response.append("Content-Type: text/plain\r\n\r\n");
    response.append("Hello World!");
    request->SendResponse(response);
}
