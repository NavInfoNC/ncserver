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
#include "stdafx.h"

#include "fcgi_stdio.h"
#include <signal.h>
#include "ncserver.h"
#include "fcgi_bind.h"
#include "fcgi_service_io.h"
#include "util.h"
#include "nc_log.h"
#include "yaml-cpp/yaml.h"

#ifndef WIN32
#include <sys/wait.h>
#include <sys/mman.h>
#include <thread>
#include <vector>
#endif

bool g_ncServerExit = false;
bool g_ncServerReload = false;

namespace ncserver
{
	class NcServerConfig
	{
	public:
		struct ServerConfig
		{
			int workerCount = 4;
		};

		static NcServerConfig* alloc() { return new NcServerConfig(); }

		ServerConfig server;

	protected:
		NcServerConfig() {}
		~NcServerConfig() {}
		friend void release(NcServerConfig* config);
	};

	void release(NcServerConfig* config)
	{
		delete config;
	}

	//////////////////////////////////////////////////////////////////////////
	Request::Request()
	{
		m_params = StaticStringMap_alloc();
		m_paramIter = RequestParameterIterator_alloc();
		m_paramPool[0] = 0;
	}

	const char* Request::requestMethod()
	{
		return FCGI_getenv("REQUEST_METHOD");
	}

	const char* Request::contentType()
	{
		return FCGI_getenv("CONTENT_TYPE");
	}

	const char* Request::documentUri()
	{
		return FCGI_getenv("DOCUMENT_URI");
	}

	const char* Request::queryString()
	{
		return m_queryString;
	}

	size_t Request::contentLength()
	{
		const char* length = FCGI_getenv("CONTENT_LENGTH");
		return strtoull(length, NULL, 10);
	}

	bool Request::isGet()
	{
		return strcmp(requestMethod(), "GET") == 0;
	}

	bool Request::isPost()
	{
		return strcmp(requestMethod(), "POST") == 0;
	}

	const char* Request::headerForName(const char* name)
	{
		return FCGI_getenv(name);
	}

	const char* Request::parameterForName(const char* name)
	{
		return m_params->get(name);
	}

	const char* Request::parameterForNameWithDefault(const char* name, const char* defaultValue)
	{
		const char* param = m_params->get(name);
		if (param == NULL)
			param = defaultValue;
		return param;
	}

	RequestParameterIterator* Request::getParameterIterator()
	{
		return m_paramIter;
	}

	void Request::reset(const char* urlValue)
	{
		m_params->clear();
		m_paramPool[0] = 0;
		m_queryString[0] = 0;

		urlDecode(urlValue, m_queryString, URL_MAX_LENGTH);

		// split by &
		strncpy(m_paramPool, m_queryString, URL_MAX_LENGTH);

		char* token;
		char* context = m_paramPool;
		while ((token = fcgi_strtok_s(NULL, "&", &context)) != NULL)
		{
			const char* key = token;
			const char* value = "";

			// key = value
			char* equalChar = strchr(token, '=');
			if (equalChar)
			{
				*equalChar = 0;
				value = equalChar + 1;
			}

			m_params->set(key, value);
		}

		m_paramIter->_init(m_params);
	}
	//////////////////////////////////////////////////////////////////////////

	static void handleExitSignalForWorker(int exitSignal)
	{
		signal(exitSignal, handleExitSignalForWorker);
		g_ncServerExit = true;
		FCGX_ShutdownPending();
	}

	static void handleExitSignalForNonWorker(int exitSignal)
	{
		signal(exitSignal, handleExitSignalForNonWorker);
		g_ncServerExit = true;
	}

	static void handleReloadSignalForBoss(int sig)
	{
		signal(sig, handleReloadSignalForBoss);
		g_ncServerReload = true;
	}

	NcServer::NcServer()
	{
		m_config = NcServerConfig::alloc();
#ifndef WIN32
		m_children = nullptr;
		m_childrenStates = nullptr;
		m_mutex = PTHREAD_MUTEX_INITIALIZER;
		pthread_mutex_init(&m_mutex, NULL);
#endif
		reset();
	}

	NcServer::~NcServer()
	{
		release(m_config);
#ifndef WIN32
		delete[] m_children;
		m_children = nullptr;
		delete[] m_childrenStates;
		m_childrenStates = nullptr;

		pthread_mutex_destroy(&m_mutex);
#endif
	}

	void NcServer::reset()
	{
#ifndef WIN32
		int workerCount = m_config->server.workerCount;

		if (workerCount > 0)
		{
			delete[] m_children;
			delete[] m_childrenStates;

			m_children = new pid_t[workerCount];
			m_childrenStates = new ChildState[workerCount];

			memset(m_children, -1, sizeof(pid_t) * workerCount);		// set as -1
			memset(m_childrenStates, 0, sizeof(pid_t) * workerCount);	// set as CHILDSTATE_INVALID
		}
#endif
	}

	void NcServer::loadConfigFile()
	{
		NcServerConfig* tmpConfig = NcServerConfig::alloc();

		try
		{
			YAML::Node root = YAML::LoadFile("./.ncserver.yaml");

			YAML::Node serverNode = root["server"];
			if (serverNode)
			{
				NcServerConfig::ServerConfig& serverCfg = tmpConfig->server;

				YAML::Node workerCountCfg = serverNode["workerCount"];
				if (workerCountCfg)
				{
					serverCfg.workerCount = workerCountCfg.as<int>();
				}
			}

			release(m_config);
			m_config = tmpConfig;
			reset();
		}
		// if anything goes wrong during parsing the config file,
		// the whole file would be ignored.
		catch (...)
		{
			release(tmpConfig);
		}
	}

	ServerState NcServer::runAndFork(int port)
	{
		enum class Identity
		{
			Boss,
			Manager,
			Worker,
		};

		Identity identity = Identity::Worker;

		signal(SIGINT, handleExitSignalForNonWorker);
		signal(SIGTERM, handleExitSignalForNonWorker);

#ifndef WIN32
		void* sharedMemory = NULL;
		if ((sharedMemory = mmap(0, sizeof(bool), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0)) == MAP_FAILED)
			return MEMORY_ERROR;

		bool& managerFinishedForking = *(bool*)sharedMemory;
		pid_t manager = fork();
		if (manager == 0)
		{
			identity = Identity::Manager;
			signal(SIGUSR1, SIG_DFL);
		}
		else
		{
			identity = Identity::Boss;
			signal(SIGUSR1, handleReloadSignalForBoss);

			while (!g_ncServerExit)
			{
				sleep(1);
				enum ReloadStatus {
					ReloadStatus_none = 0,
					ReloadStatus_reloading = 1,
					ReloadStatus_succ = 2,
					ReloadStatus_failed = 3,
				};
				auto recordStatus = [](ReloadStatus status) {
					FILE* file = fopen(".status", "r+");
					if (file != NULL)
						fprintf(file, "%d", status);
					fclose(file);
				};
				if (g_ncServerReload)
				{
					managerFinishedForking = false;

					recordStatus(ReloadStatus_reloading);

					pid_t newManager = fork();
					if (newManager == 0)
					{
						identity = Identity::Manager;
						signal(SIGUSR1, SIG_DFL);
						break;
					}
					else
					{
						identity = Identity::Boss;
						while (!managerFinishedForking)
						{
							if (waitpid(newManager, NULL, WNOHANG))
								break;
							sleep(1);
						}
						if (managerFinishedForking)
						{
							kill(manager, SIGTERM);
							waitpid(manager, NULL, 0);
							manager = newManager;
							recordStatus(ReloadStatus_succ);
						}
						else
						{
							recordStatus(ReloadStatus_failed);
						}
					}
					g_ncServerReload = false;
				}
			}
		}
#endif
		if (identity == Identity::Boss)
			return SUCCESS;

		loadConfigFile();

		if (!prepareProcess())
		{
			return PREPAER_PROCESS_ERROR;
		}

		if (!initUnforkableResources())
		{
			return INIT_UNFORKABLE_RESOURCES_ERROR;
		}

		fcgi_init(port);

#ifndef WIN32
		if (forkChildren())
		{
			identity = Identity::Manager;
			managerFinishedForking = true;
			while (!g_ncServerExit)
			{
				sleep(1);
				if (checkChildrenStateAndRefork()) {
					identity = Identity::Manager;
				} else {
					identity = Identity::Worker;
					break;
				}
			}
			if (identity == Identity::Manager)
			{
				int workerCount = m_config->server.workerCount;
				for (int i = 0; i < workerCount; i++)
				{
					kill(m_children[i], SIGTERM);
					waitpid(m_children[i], NULL, 0);
				}
			}
		}
		else
		{
			identity = Identity::Worker;
		}
#endif

		if (identity == Identity::Worker)
		{
			ServerState state = serve();
			if (state != SUCCESS && state != FCGI_ERROR)
			{
				return state;
			}
		}

#ifndef WIN32
		if(identity == Identity::Manager)
#endif
		{
			if (!cleanupUnforkableResources())
			{
				return CLEAN_UP_UNFORKABLE_RESOURCES_ERROR;
			}
		}

		if (!finalizeProcess())
		{
			return FINALIZE_PROCESS_ERROR;
		}

		fcgi_cleanup();

		return (g_ncServerExit) ? SUCCESS : FCGI_ERROR;
	}

	ServerState NcServer::serve()
	{
		signal(SIGINT, handleExitSignalForWorker);
		signal(SIGTERM, handleExitSignalForWorker);
		if (!startService())
		{
			return START_SERVICE_ERROR;
		}

		Request request;
		ServiceIo* io = new FCgiServiceIo(FCGI_stdout);

		while (!g_ncServerExit && FCGI_Accept() >= 0)
		{
			const char* qs = FCGI_getenv("QUERY_STRING");

			if (strlen(qs) >= URL_MAX_LENGTH)
			{
				io->addHeaderField("Status: 414 Request-URI Too Long");
				io->endHeaderField();
				continue;
			}

			request.reset(qs);

			query(io, &request);
		}
		delete io;

		if (!stopService())
		{
			return STOP_SERVICE_ERROR;
		}
		return (g_ncServerExit) ? SUCCESS : FCGI_ERROR;
	}

	void NcServer::exit()
	{
		g_ncServerExit = true;
	}

	void NcServer::reforkAllChildren()
	{
#ifndef WIN32
		if (m_children == nullptr)
			return;

		int workerCount = m_config->server.workerCount;
		for (int i = 0; i < workerCount; i++)
		{
			pthread_mutex_lock(&m_mutex);
			m_childrenStates[i] = CHILDSTATE_WAIT_FOR_RELOAD;
			pthread_mutex_unlock(&m_mutex);
		}
#endif
	}

#ifndef WIN32
	bool NcServer::forkOne(int index)
	{
		pid_t pid = fork();
		if (pid == 0)			// child
		{
			return false;
		}
		else					// parent
		{
			pthread_mutex_lock(&m_mutex);
			if (pid > 0)		// fork succeed
			{
				m_children[index] = pid;
				m_childrenStates[index] = CHILDSTATE_LIVING;
			}
			else				// fork fails
			{
				m_children[index] = -1;
				m_childrenStates[index] = CHILDSTATE_INVALID;
			}
			pthread_mutex_unlock(&m_mutex);
		}
		return true;
	}

	bool NcServer::forkChildren()
	{
		int workerCount = m_config->server.workerCount;
		for (int i = 0; i < workerCount; i++)
		{
			if (!forkOne(i))
				return false;
		}
		return true;
	}

	bool NcServer::checkChildrenStateAndRefork()
	{
		bool isManager = true;
		std::vector<pid_t> childrenToKill;
		int workerCount = m_config->server.workerCount;
		for (int i = 0; i < workerCount && isManager && !g_ncServerExit; i++)
		{
			pid_t pidToKill = -1;
			switch (m_childrenStates[i])
			{
			case CHILDSTATE_WAIT_FOR_RELOAD:
				pidToKill = m_children[i];
				isManager = forkOne(i);
				if (isManager)
				{
					kill(pidToKill, SIGTERM);
					childrenToKill.push_back(pidToKill);
				}
				break;
			case CHILDSTATE_LIVING:
				if (waitpid(m_children[i], NULL, WNOHANG))
				{
					isManager = forkOne(i);
				}
				break;
			case CHILDSTATE_INVALID:
				isManager = forkOne(i);
				break;
			}
		}

		if (isManager && !childrenToKill.empty())
		{
			std::thread t([childrenToKill]() {
				sleep(15);
				for (pid_t child : childrenToKill)
				{
					if (waitpid(child, NULL, WNOHANG) == 0)
					{
						kill(child, SIGKILL);
						waitpid(child, NULL, 0);
					}
				}
			});
			t.detach();
		}
		return isManager;
	}

#endif
}
