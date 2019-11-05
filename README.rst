A Serious C++ HTTP Service Framework
====================================

`中文 <docs/README.chs.rst>`__ | English

**ncserver** is a C++ HTTP service framework based on FastCGI.
In NavInfo, all our navigation related services(routing, poi searching, traffic information etc) are based on this framework.

.. image:: docs/architecture.png

A Simple Example
----------------

This example program (in folder "example") shows the basic function of ncserver.

.. code-block:: cpp

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
      server.runAndFork(9009);
      return 0;
   }

Test the program::

   $ curl -I "http://127.0.0.1/echo?city=beijing&keyword=coffee"
   HTTP/1.1 200 OK
   Server: nginx/1.7.2
   Date: Tue, 16 Jul 2019 04:07:09 GMT
   Content-Type: text/plain; charset=utf-8
   Connection: keep-alive

   $ curl  "http://127.0.0.1/echo?city=beijing&keyword=coffee"
   Request-Method: GET
   Query String: city=beijing&keyword=coffee
   city: beijing
   keyword: coffee

Features
--------

In addition to having a low learning curve, ncserver also provide the following benefits:

1. Multi-process architecture
   
   With multi-process architecture. Each process is isolated. 
   Even if one process crashed, the following requests will not be affected.

   With the help of the COW(copy-on-write) feature of Linux system, all work processes
   can share static memory data. So more workers don't necessarily means more memory consumption.

2. Automatically re-spawn of worker processes
   
   A daemon process will keep watching on all worker processes.
   If one worker process crashed, a new worker process will be spawned.

3. No-down-time reload
   
   If the configuration file or data file changes, the service can be reloaded with no down-time.

What's Included
---------------

* **Source Code**: Can be compiled into a static library or added to a project directly.
* **Service Control Program**: `ncserverctl`.
* **A Example Project**: See "example" folder.
* **Nginx for Windows**: For debugging under Windows. In "dependency" folder.
  
Build and Test
--------------

`ncserver` supports both Linux and Windows.

* Linux is used in production environment.
* Windows is used solely for coding and debugging. It's single processed and doesn't provide all the benefits of ncserver.
  But we think Visual Studio is indispensable for any productive-minded C++ developer.
  So we finish most of the work on Windows and only compile and deploy service on Linux.

Compile & test steps under Windows
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. Configure dependency/nginx-1.7.2/conf/nginx.conf to make nginx a FastCGI proxy and transfer requests via TCP connection to our backend.

.. code-block:: bash

      location ~ /echo {
            root           html;
            fastcgi_pass   127.0.0.1:9009;
            fastcgi_index  index.php;
            fastcgi_param  SCRIPT_FILENAME  /scripts$fastcgi_script_name;
            include        fastcgi_params;
         }

.. note:: ncserver communicate with nginx with FCGI protocol. The commmunication can either be built via TCP or Unix Domain Socket.
   In this case, since we are under Windows platform, TCP becomes our only choice, and we chose 9009 port for example.
   In the case below where we introduces configuration under Linux, Unix Domain Socket will be used.

2. Double click dependency/nginx-1.7.2/nginx.exe to start nginx.

.. note:: 
   Nginx would run in background. 
   If you modify the configuration of nginx and want the new configuration to take effect, you should use the ``nginx -s reload`` command.
   If you want to stop nginx, you can either use ``nginx -s stop`` command or directly kill the nginx processes in Task Manager.

.. warning:: 
   If you double click nginx more than once, ``nginx -s stop`` command can only be used to stop the nginx processes you started by the last double click.
   Other processes can only be killed in Task Manager.         

3. Double click config/ncserver.sln, compile it in Visual Studio, and then run the example project.
4. Run example/test.py, or directly access http://127.0.0.1/echo?text=abc in browser to test the project.

Compile & test steps under Linux
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. Install nginx using apt-get or yum (depend on your system). 
2. Configure nginx as a fastcgi proxy to transfer requests to our backend via Unix Domain Socket。

.. code-block:: bash

   $ sudo vim /etc/nginx/conf.d/default.conf

   location ~ /echo {
      root           html;
      fastcgi_pass   unix:/etc/ncserver/echo/.ncserver.sock;
      fastcgi_index  index.php;
      fastcgi_param  SCRIPT_FILENAME  /scripts$fastcgi_script_name;
      include        fastcgi_params;
   }

   $ sudo nginx -s reload(or sudo nginx -s start, if nginx is not started yet.)

3. Run the following commands to compile ncserver library and the testing program.

.. code-block:: bash

   $ python make.py
   $ mkdir /etc/ncserver/echo
   $ copy echo /etc/ncserver/echo
   $ ncserverctl start echo
   Starting <echo> on domain socket unix:/etc/ncserver/echo/.ncserver.sock
   spawn-fcgi: child spawned successfully: PID: 32592

4. Use curl to test the echo server.

.. code-block:: bash

   $ curl  "http://127.0.0.1/echo?city=beijing&keyword=coffee"
   Request-Method: GET
   Query String: city=beijing&keyword=coffee
   city: beijing
   keyword: coffee
   
ncserverctl
-----------

`ncserverctl` is the managment program for ncserver services. It's used to start/stop/restart/reload a service.

.. warning:: Needs docmentation

项目背景
--------

ncserver是基于fastCGI二次开发的。
FastCGI是一个支持C语言开发的通用网关接口，通过FastCGI，我们可以直接用C/C++语言开发服务器程序，运行效率高。
然而FastCGI的接口为了兼容普通CGI。导致接口使用并不非常直观。所以我们进行了二次封装。

NcServer的设计目的为：

* 封装FastCGI。
* 提供程序的生命周期框架。
* 提供fork()支持。允许快速复制出服务进程。
  
关于fork()支持。是为了适应导航在线服务的特点而设计。因为算路、搜索等服务，都是基于大量静态数据进行的。
如果多个fcgi进行都去加载大量同样的数据，会浪费许多内存。

所以，可以先由一个进程把静态数据加载完毕之后，再fork()出其它服务进程。
基于Linux操作系统的COW特性，就可以成倍减少内存占用。

Troubleshoot
------------

502 Bad Gateway
^^^^^^^^^^^^^^^

大量并发的时候，可能会有部分502错误。ab -n 100000 -c 200，200并发就发生了。
而100并发没有这个问题。观察nginx日志，会看到::

   2016/01/29 18:10:43 [error] 3059#0: *9190789 connect() to unix:/tmp/nds-tile-server.sock failed (11: Resource temporarily unavailable) while connecting to upstream, client: 192.168.85.22, server: fastcgi.mapbar.com, request: "GET /get/qvf?key=1156125669&fields=gridId,gridData HTTP/1.1", upstream: "fastcgi://unix:/tmp/nds-tile-server.sock:", host: "192.168.0.86"

是nginx无法和ncserver通过unix端口建立联系。

最后发现是spawn-fcgi -b backlog 参数的问题。改为512，就可以承受256并发了。
原来，请求会在unix domain socket上排队。要承受高并发，必须修改这个排队的限制数。

另外，某些CPU bound的服务，比如算路，应用以上修改后，依然有问题。
采用以下方法可以解决:

1. `sysctl -A`，检查net.core.somaxconn、net.core.netdev_max_backlog和net.ipv4.tcp_max_syn_backlog。
2. vim /etc/sysctl.conf可以修改这几个参数。`sysctl -p`应用。

如果问题还没解决，可以试试：（按理说应该不需要改这个）

1. ulimit -a，查看open files
2. 修改 /etc/security/limits.conf，加入:

   * soft nofile 1000000
   * hard nofile 1000000
