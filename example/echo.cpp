// echo.cpp : Defines the entry point for the console application.
//

#include <stdlib.h>
#include "ncserver.h"

using namespace std;
using namespace ncserver;

class EchoServer : public NcServer
{
protected:
	virtual void query(ServiceIo* io, Request *request)
	{
		io->addHeaderField("Content-Type: text/html; charset=utf-8");
		io->endHeaderField();

		io->print("= Request Header = <br>"
			"Content-Type: %s <br>"
			"Request-Method: %s <br>",
			request->contentType(),
			request->requestMethod());

		io->print("= Query String = %s<br>", request->queryString());
		RequestParameteterIterator* iter = request->getParameterIterator();
		while (iter->next())
		{
			io->print("%s: %s<br>", iter->name, iter->value);
		}

		if (request->isPost())
		{
			unsigned char *buffer = new unsigned char[request->contentLength()];

			io->read(buffer, request->contentLength());
			io->write(buffer, request->contentLength());
		}
		io->flush();
	}
};

int main(int argc, char* argv[])
{
	EchoServer server;
	server.runAndFork(9009);
	return 0;
}
