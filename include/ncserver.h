/*
MIT License

Copyright (c) 2019 GIS Core R&D Department, NavInfo Co., Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once

struct StaticStringMap
{
	void set(const char* name, const char* value);
	const char* get(const char* name);
	void clear();
};

StaticStringMap* StaticStringMap_alloc();
void StaticStringMap_free(StaticStringMap* o);

struct RequestParameterIterator
{
	const char* name;
	const char* value;

	void _init(StaticStringMap* map);

	bool next();
	void reset();
};

RequestParameterIterator* RequestParameterIterator_alloc();
void RequestParameterIterator_free(RequestParameterIterator* o);

//////////////////////////////////////////////////////////////////////////

#define URL_MAX_LENGTH 10240

namespace ncserver
{
	class NcServerConfig;

	class ServiceIo
	{
	public:
		virtual void read(void *buffer, size_t size) = 0;
		virtual void write(void *buffer, size_t size) = 0;
		virtual int print(const char *format, ...) = 0;
		virtual int addHeaderField(const char *format, ...) = 0;
		virtual void endHeaderField(void) = 0;
		virtual void flush(void) = 0;
	};

	enum ServerState
	{
		SUCCESS,
		PREPAER_PROCESS_ERROR,
		INIT_UNFORKABLE_RESOURCES_ERROR,
		START_SERVICE_ERROR,
		STOP_SERVICE_ERROR,
		CLEAN_UP_UNFORKABLE_RESOURCES_ERROR,
		FINALIZE_PROCESS_ERROR,
		FCGI_ERROR,
		MEMORY_ERROR,
		UNKNOWN_ERROR
	};

	class Request
	{
	public:
		Request();

		/**
			@note
				For given request headers:
					content-length=1000\r\n
					content-type=\r\n
					\r\n
				With the most common fastcgi params setting:
					headerForName("CONTENT_LENGTH") would return "1000";
					headerForName("CONTENT_TYPE") would return "";
					headerForName("content-length") would return NULL;
				The specified name must be declared in configuration file of the web server such as nginx.
				Normally, supported headers include:
					DOCUMENT_ROOT

					DOCUMENT_URI
					REQUEST_URI

					REQUEST_METHOD
					QUERY_STRING
					CONTENT_TYPE
					CONTENT_LENGTH

					SERVER_PROTOCOL

					SCRIPT_FILENAME
					SCRIPT_NAME

					GATEWAY_INTERFACE
					SERVER_SOFTWARE

					REMOTE_ADDR
					REMOTE_PORT
					SERVER_ADDR
					SERVER_PORT
					SERVER_NAME

					HTTPS
		*/
		const char* headerForName(const char* name);

		/**
			@note
				For a given url:
					http://host/path?key1=value1&key2=value2&key3
				parameterForName("key1") would return "value1";
				paramaterForName("key2") would return "value2";
				parameterForName("key3") would return "";
				parameterForName("key4") would return NULL;
			@return
				Value for specified key.
				If the key has no value, return "".
				If the key does not appear in the url, return NULL.
		 */
		const char* parameterForName(const char* name);

		/**
			@note
				Similar to parameterForName,
				expect that this function would return defaultValue instead of NULL if the key does not appear in the url.
			@return
				Value for specified key.
				If the key has no value, return "".
				If the key does not appear in the url, return defaultValue.
		 */
		const char* parameterForNameWithDefault(const char* name, const char* defaultValue);

		const char* requestMethod();
		const char* contentType();
		const char* documentUri();
		const char* queryString();
		size_t contentLength();
		bool isGet();
		bool isPost();

		void setQueryString(const char* queryString);

		RequestParameterIterator* getParameterIterator();

	private:
		char m_paramPool[URL_MAX_LENGTH];
		char m_queryString[URL_MAX_LENGTH];
		StaticStringMap* m_params;
		RequestParameterIterator* m_paramIter;
	};

	class NcServer
	{
	public:
		NcServer();
		virtual ~NcServer();

		void reforkAllChildren();

		/**
			Start the service.

			@param port
				WINDOWS ONLY. Configured in nginx.conf.
		 */
		ServerState runAndFork(int port);

		void exit();

	protected:
		/**
			Method to be called before the processes are forked.

			@remarks
				Global initialization should be implemented here.
				Generally, the read-only data should be loaded here.
			@notice
				File descriptors that are opened in this function would
				be shared among all worker processes. Therefore any file
				operation in one worker process would affect the file
				status in another worker process.
		 */
		virtual bool prepareProcess(void) { return true; }

		/**
			Method to be called after prepareProcess() in parent process.

			@remarks
				Global initialization for unforkable resources(eg. threads).
				This method would only run on main process.
		 */
		virtual bool initUnforkableResources(void) { return true; }

		/**
			Method to be called just after the processes was forked.

			@remarks
				Individual initialization for each process should be implemented here.
		 */
		virtual bool startService(void) { return true; }

		/**
			Clean up resources that is loaded in startService()
		 */
		virtual bool stopService(void) { return true; }

		/**
			Clean up resources that is initialized in initUnforkableResources()
		 */
		virtual bool cleanupUnforkableResources(void) { return true; }

		/**
			Clean up resources that is loaded in prepareProcess()
		 */
		virtual bool finalizeProcess(void) { return true; }

		/**
			Callback method of an inbound request.

			@param io
				the input/output stream

			@param request
				The inbound request.
		 */
		virtual void query(ServiceIo *io, Request *request) = 0;

		ServerState serve();

		void loadConfigFile();

	private:
		NcServerConfig* m_config;
		void reset();

#ifndef WIN32
		pid_t* m_children;
		enum ChildState {
			CHILDSTATE_INVALID = 0,
			CHILDSTATE_LIVING = 1,
			CHILDSTATE_WAIT_FOR_RELOAD = 2,
		}* m_childrenStates;
		pthread_mutex_t m_mutex;

		bool forkOne(int index);
		bool forkChildren();
		bool checkChildrenStateAndRefork();
#endif
	};
}
