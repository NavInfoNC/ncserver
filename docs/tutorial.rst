NcServer Developer Tutorial
===========================

First view
----------

.. code-block:: cpp

   class EchoServer : public NcServer
   {
   protected:
       virtual void query(ServiceIo* io, Request* request)
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
               unsigned char* buffer = new unsigned char[request->contentLength()];   

               io->read(buffer, request->contentLength());
               io->write(buffer, request->contentLength());
           }
           io->flush();
       }
   };

   int main()
   {
       NcLog::instance().registerUpdateLogLevelSignal();
       NcLog::instance().init("echo", LogLevel_info);
       EchoServer server;
       server.runAndFork(9009);    // 9009 is only used on Windows.
                                   // When running under Linux, Unix Domain Socket is used instead.
       return 0;
   }

This example is a brief introduction about how to develop with NcServer framework.

First of all, we need to write a subclass of the NcServer class. 

The ``query()`` interface takes two pointers, one points to a ServiceIo object, and the other a Request object.
The ServiceIo object can be used to process data stream, that is, to receive post data from the requester, and to send response back.
The Request object can be used to retrieve URL, query string, request headers, etc.

Developers should retrieve informations from this two parameters, and do the bussiness logic inside the ``query()`` interface.
And then with the ServiceIo object, an HTTP response can be generated. 

When the call of the ``query()`` interface ends, the processing of the incoming request finishes.

In the main function, what we need to do is just declaring an object of the NcServer subclass we defined before, and invoke the ``runAndFork()`` interface of it. The ``runAndFork()`` interface will then build a message roop for you, and once a request is coming,
the ``query()`` function would be invoked then.

Taking advantages of this framework
----------------------------------

After the introduction above, you may have known how to build an HTTP service with this framework.
However, it just does what any HTTP service framework should do.
So now I'm going to dive deeper and to show you the advantages of the NcServer framework.

Life cycle
^^^^^^^^^^

Before going any further, I need to tell you more about what is happening inside the ``runAndFork()`` function::

                     ---------
                     | start |
                     ---------
                        |                              -------
                        v                              | end |
                    ----------                         -------
                    | fork() |                            ^
                    ----------                            | yes
                        |              yes                |
               is parent process ? ------------>  is exit() called ?
                        |          boss process           |
        manager process | no                              | no
                        |                                 v
                        |                   -------------------------------             
                        |                   |       busy waiting          |             
                        |                   | till reload signal received | <--            
                        |                   -------------------------------   |
                        |                                 |                   |
                        |                                 v                   |
                        |                  ---------------------------------  |
                        |                  | fork a new manager process,   |  |
                        |                  | wait till it finishes forking |  |
                        |                  | children, then kill the old   |  |
                        |                  | manager process.              |  |
                        v                  ---------------------------------  |
               --------------------                       |                   |
               | prepareProcess() | <------------ is manager process ?  -------
               --------------------      no                                yes
                        |               
                        v               
           ---------------------------- 
           | initUnforkableResouces() | 
           ---------------------------- 
                        |               
                        v               
                   ----------           
                   | fork() | <----------------------------
                   ----------                             |
                        |                             yes |
                        v              yes                |
               is parent process ? -----------> has more child to fork ?
                        |         manager process         |
         worker process | no                              | no
                        |                                 v            
                        |                    --------------------------------
                        |                    | test if any worker is dead,  |
                        |                    | if so, fork a new worker for | <-
                        |                    | supplement.                  |  |
                        v                    --------------------------------  |  
                 ------------------                       |                    |  
                 | startService() | <------------- is worker process ?         |  
                 ------------------       yes             |                    |  
                        |                            should exit ? -------------  
                        v                                 |             no
           ------------------------------             yes |
           | block till a request comes |                 |
      ---> |     or exit() is called    |                 |
      |    |  or SIG_TERM is received   |                 |
      |    ------------------------------                 |
   -----------          |                                 |
   | query() |          |                                 |
   -----------          |                                 v
      |                 v                  --------------------------------
      ----------- should exit ?            | cleanupUnforkableResources() |
           no           |                  --------------------------------
                        | yes                             |
                        v                                 |
                 -----------------                        |
                 | stopService() |                        |
                 -----------------                        |
                        |                                 |
                        | <--------------------------------
                        v
               ---------------------
               | finalizeProcess() |
               ---------------------
                        |
                        v
                     -------
                     | end |
                     -------

Large read-only data loading
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

From the life cycle diagram above, we see ``prepareProcess()`` and ``initUnforkableResouces()``
are invoked before calling ``fork()`` in the manager process. Hence, according to the Copy On
Write(COW) feature of the moden linux system, we can load large read-only data in ``prepareProcess()``.
Then every worker process can access this data, and this data has only one copy in the system memory.

.. note:: note

   Because we are using ``fork()`` here, we should be aware that file descriptors SHOULD NOT be opened
   here, otherwise all worker processes would share the same fd, as a result of which, operations on
   this fd in one worker process would affect all other workers. So, unless you are doing so on purpose,
   DON'T TRY THIS!

For example:

.. code-block:: cpp

   class DataServer : public NcServer
   {
   protected:
       bool prepareProcess()
       {
           if (!fillIdMapWithFile(&m_id2name))
               return false;
           return true;
       }

       bool finalizeProcess()
       {
           cleanupIdMap(&m_id2name);
       }

       void query(ServiceIo* io, Request* request)
       {
           char* idString = request.parameterForName("Id");

           if (idString != NULL && idString[0] != '\0')
           {
               int id = atoi(idString);
               auto iter = m_id2name.find(id);
               if (iter != m_id2name.end())
               {
                   const char* name = iter.second;

                   int len = strlen(name);
                   io.addHeader("Content-Length", len);
                   io.addHeader("Content-Type", "text/plain");
                   io.endHeader();
                   io.print(name);
                   io.flush();
                   return;
               }
           }
           io.addHeader("Status: 400");
           io.endHeader();
           io.flush();
       }

   private:
       unordered_map<int, const char*> m_id2name;

   };

   int main()
   {
       DataServer server;
       server.runAndFork(9000);
       return 0;
   }


