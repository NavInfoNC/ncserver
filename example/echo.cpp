// An example program of ncserver
#include <stdlib.h>
#include "ncserver.h"

using namespace std;
using namespace ncserver;

class EchoServer : public NcServer
{
protected:
	virtual void query(ServiceIo* io, Request *request)
	{
		io->addHeaderField("Content-Type: text/plain; charset=utf-8");
		io->endHeaderField();

		// output request parameters
		io->print("Request-Method: %s\n", request->requestMethod());

		// parse and output query string
		io->print("Query String: %s\n", request->queryString());
		RequestParameteterIterator* iter = request->getParameterIterator();
		while (iter->next())
		{
			io->print("%s: %s\n", iter->name, iter->value);
		}

		// read POST data
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
	server.runAndFork(9009);	// 9009 is only used on Windows. 
								// When running under Linux, Unix Domain Socket is used instead.
	return 0;
}
