NcServer Developer Tutorial
===========================

Preface
-------

In this tutorial, we are going to show you how to build an HTTP service with 
this framework. A simple book managament service would be used as demostration.

First view
----------

.. code-block:: cpp

   class BookManagingServer : public NcServer
   {
   protected:
       virtual void query(Serviceio* io, Request* request)
       {
           io->addHeaderField("Content-Type", "text/html; charset=utf-8");
           io->endHeaderField();
           io->print("<html><head><title>Book Management</title></head><body>");

           io->print("<table border=\"1\">");

           // output request parameters
           io->print("<tr><td>Request Method</td><td>%s</td></tr>", request->requestMethod());

           io->print("<tr><td>Content Length</td><td>%d</td></tr>", request->contentLength());
           
           io->print("<tr><td>Document URI</td><td>%s<td></tr>", request->documentUri());

           // parse and output query string
           io->print("tr><td>Query String</td><td>%s</td></tr></table>", request->queryString());

           RequestParameterIterator* iter = request->getParameterIterator();

           io->print("<table border=\"1\"><tr><th>parameter</th><th>value</th></tr>");

           while (iter->next())
           {
               io->print("<tr><td>%s</td><td>%s</td></tr>", iter->name, iter->value);
           }

           io->print("</table>");

           // read POST data
           if (request->isPost())
           {
               unsigned char* buffer = new unsigned char[request->contentLength()];   

               io->read(buffer, request->contentLength());
               // you can parse the buffer then

               delete[] buffer;
           }

           io->print("</body></html>");
           io->flush();
       }
   };

   int main()
   {
       BookManagingServer server;
       server.runAndFork(9000);    // 9009 is only used on Windows.
                                   // When running under Linux, Unix Domain Socket is used instead.
       return 0;
   }

This example is a brief introduction about how to develop with NcServer framework.

First of all, we need to write a subclass of the NcServer class. 

The ``query()`` interface takes two pointers, one points to a Serviceio object, 
and the other a Request object. The Serviceio object can be used to process data
stream, that is, to receive post data from the requester, and to send response back.
The Request object can be used to retrieve URL, query string, request headers, etc.

Developers should retrieve informations from this two parameters, and do the bussiness
logic inside the ``query()`` interface. And then with the Serviceio object, an HTTP 
response can be generated. 

When the call of the ``query()`` interface ends, the processing of the incoming 
request finishes.

In the main function, what we need to do is just declaring an object of the NcServer
subclass we defined before, and invoke the ``runAndFork()`` interface of it. The 
``runAndFork()`` interface will then build a message roop for you, and once a 
request is coming, the ``query()`` function would be invoked then.

Taking advantages of this framework
----------------------------------

After the introduction above, you may have known how to build an HTTP service with 
this framework. However, it just does what any HTTP service framework should do.
So now I'm going to dive deeper and to show you the advantages of the NcServer 
sssframework.

Life cycle
^^^^^^^^^^

Before going any further, I need to tell you more about what is happening inside 
the ``runAndFork()`` function::

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

Configuration
^^^^^^^^^^^^^

We use a YAML file named ``.ncserver.yaml`` placed under the working directory to 
configure this framework.

Currently, we just support configuration on the worker count.

The file should be like:

.. code-block:: yaml

   server:
       # The value of server.workerCount is an integer indicating the count of 
       # worker processes.
       # By default, workerCount is 4.
       workerCount: 4


Large read-only data loading
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

From the life cycle diagram above, we see ``prepareProcess()`` and 
``initUnforkableResouces()`` are invoked before calling ``fork()`` in the manager 
process.Hence, according to the Copy On Write(COW) feature of the moden linux 
system, we can load large read-only data in ``prepareProcess()``. Then every worker
process can access this data, and this data has only one copy in the system memory.

::

   Note:
   Because we are using ``fork()`` here, we should be aware that file descriptors 
   SHOULD NOT be opened here, otherwise all worker processes would share the same 
   fd, as a result of which, operations on this fd in one worker process would 
   affect all other workers. 

   So, unless you are doing so on purpose, DON'T TRY THIS!

And now we are going to make this BookManagingServer do real something.

.. code-block:: cpp

   class BookManagingServer : public NcServer
   {
   protected:
       bool prepareProcess()
       {
           // For simplication, 
           // we use a hard coding map here as demostration.
           // Though the map is small here, it is meant to be
           // a map that is large enough and won't be modified ever since.
           // In real world, this kind of data is usually loaded from files.

           m_id2name[1000] = "Sherlock Holmes";
           m_id2name[1001] = "A Midsummer Night's Dream";
           m_id2name[1002] = "A Tale of Two Cities";
           m_id2name[1003] = "David Copperfield";

           return true;
       }

       bool finalizeProcess()
       {
           // if you are building the map with a memory block in prepareProcess(),
           // you can release the memory block here.
           return true;
       }

       void query(Serviceio io, Request* request)
       {
           const char* documentUri = request->documentUri();

           io->addHeaderField("Content-Type", "text/html; charset=utf-8");
           io->endHeaderField();
           io->print("<html><head><title>Book Management</title></head><body>");

           if (strstr(documentUri, "/search") != NULL)
           {
               const char* bookIdsStr = request->parameterForName("bookIds");

               io->print("<table border=\"1\"><tr><th>id</th><th>name</th></tr>");

               if (bookIdsStr != NULL && bookIdsStr[0] != '\0')
               {
                   char* bookIdsStr2 = strdup(bookIdsStr);
                   char* token = strtok(bookIdsStr2, ",");
                   while (token != NULL)
                   {
                       int bookId = atoi(token);

                       auto iter = m_id2name.find(bookId);
                       if (iter != m_id2name.end())
                       {
                           const char* name = iter->second;

                           io->print("<tr><td>%d</td><td>%s</td></tr>", bookId, name);
                       }
                       else
                       {
                           io->print("<tr><td>%d</td><td>-</td></tr>", bookId);
                       }

                       token = strtok(NULL, ",");
                   }
                   free(bookIdsStr2);
                   io->print("</table>");
               }

               io->print("</body></html>");
           }
           else
           {
               io->print("Unsupported request.");
           }
           
           io->flush();
       }

   private:
       unordered_map<int, const char*> m_id2name;

   };

   int main()
   {
       BookManagingServer server;
       server.runAndFork(9000);
       return 0;
   }

In this example, we initialize a hash map in ``prepareProcess()``, 
and this map is not modified anywhere else. So due to the COW feature,
this map can be accessed in every worker process, while it just hold one copy
in the memory, no matter how large it may be.

Graceful reloading
^^^^^^^^^^^^^^^^^^

For some HTTP services, configurations are loaded when the service is started,
and so may some static data. So when the configurations or these static data get
changed, we need to reload them and make them work.

However, we do not like the services get stopped during reloading. So there comes
the graceful reloading feature of the NcServer framework.

How we implement this is actually so simple. Once a reloading signal is sent to
the boss process, it will prepare a new manager process then. And when this new
manager process finishes creating its worker processes, which means the new worker
set is available to accept incoming requests, the boss process would then tell the
old manager to release all workers and shut down.

So you actually need not do anything to get your service supporting this graceful
reloading feature using the NcServer framework, if you do all your loading actions
inside ``prepareProcess()``, ``initUnforkableResouces()``, or ``startService()``.

Specifically, the ``.ncerver.yaml`` would also be reloaded when a reload action is 
performed. So you can modify workerCount in the YAML file, and then perform a 
reload action to modify the worker count of your service.

Process monitoring
^^^^^^^^^^^^^^^^^^

In the NcServer process hierarchy, we have a boss process which is used to process
the reload signal to support graceful reloading feature mentioned above, we have
several worker processes that is used to process incomming requests, and we also
have a manager process. This manager process aims to monitor the survival of all
worker processes. If any worker is found dead, then the manager process would
generate a new worker process for replacement after releasing the resources 
associated with the dead worker process.

Apparently, this feature requires you no action. It is naturaly supported by the 
NcServer framework.

Customized request headers
^^^^^^^^^^^^^^^^^^^^^^^^^^

NcServer framework is based on FastCGI, so it follows the FastCGI interface.
Request headers should be retrieved using the ``ncserver::Request.headerForName()``
interface. Parameter for this interface is a '_' splited capitalized form of the
header name. For example, to retrieve the 'Content-Length' header, you should use
``request->headerForName("CONTENT_LENGTH")``, to retrieve the 'Content-Type' header,
you should use ``request->headerForName("CONTENT_TYPE")``, etc.

The following headers are supported by default:

* DOCUMENT_ROOT
* DOCUMENT_URI
* REQUEST_URI

* REQUEST_METHOD
* QUERY_STRING
* CONTENT_TYPE
* CONTENT_LENGTH

* SERVER_PROTOCOL

* SCRIPT_FILENAME
* SCRIPT_NAME

* GATEWAY_INTERFACE
* SERVER_SOFTWARE

* REMOTE_ADDR
* REMOTE_PORT
* SERVER_ADDR
* SERVER_PORT
* SERVER_NAME

* HTTPS

If you want to retrieve other headers, for example, if you want to retrieve the
'If-None-Match' header, you should configure nginx first. Steps are shown as follow:

#. Global configuration: edit the fastcgi_params file under nginx directory
   (e.g. /etc/nginx/fastcgi_params), and append ``fastcgi_param IF_NONE_MATCH
   $http_if_none_match;`` to the file;
   or location only configuration: add ``fastcgi_param IF_NONE_MATCH 
   $http_if_none_match;`` into the location configuration for your service;
#. Reload nginx to make the modificatin make effect.

After these steps, nginx would recognise the 'If-None-Match' header, storing its value 
in a variable named ``http_if_none_match``. When a request with this header comes, the
value of the header would be transfered to our backend service in FastCGI way, and the
backend can retrieve it with ``request->headerForName("IF_NONE_MATCH")``.

