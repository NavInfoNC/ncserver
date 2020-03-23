// An example program of ncserver
#include <stdlib.h>
#include "ncserver/ncserver.h"
#include "ncserver/nc_log.h"

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
		ASYNC_LOG_INFO("Request-Method: %s", request->requestMethod());

		// parse and output query string
		io->print("Query String: %s\n", request->queryString());
		ASYNC_LOG_INFO("Query String: %s", request->queryString());
		RequestParameterIterator* iter = request->getParameterIterator();
		while (iter->next())
		{
			io->print("%s: %s\n", iter->name, iter->value);
			ASYNC_LOG_INFO("%s: %s", iter->name, iter->value);
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
	NcLog::instance().registerUpdateLogLevelSignal();
	NcLog::instance().init("echo", LogLevel_info);
	EchoServer server;
	server.runAndFork(9009);	// 9009 is only used on Windows.
								// When running under Linux, Unix Domain Socket is used instead.
	return 0;
}
