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

Window下编译&测试
^^^^^^^^^^^^^^^^^

按照以下步骤编译和测试。

1. 配置dependency/nginx-1.7.2/conf/nginx.conf::

      location ~ /echo {
            root           html;
            fastcgi_pass   127.0.0.1:9009;
            fastcgi_index  index.php;
            fastcgi_param  SCRIPT_FILENAME  /scripts$fastcgi_script_name;
            include        fastcgi_params;
         }

   .. note:: ncserver和nginx之间用FCGI protocol通讯。它可以是TCP，也可以用Unix Domain Socket。
      本例中采用9009 TCP通讯，下面的Linux部署案例中，采用Unix Domain Socket /tmp/echo.sock。

2. 启动dependency/nginx-1.7.2/nginx.exe。

   .. warning:: nginx.exe只能双击一次，否则会出现问题。必须用nginx -s stop停止。用nginx -s reload重新加载配置。

3. 直接打开config的sln文件。编译。按F5运行example。
4. 运行test.py。或者直接访问http://127.0.0.1/echo?text=abc

Ubuntu下编译&测试
^^^^^^^^^^^^^^^^^

首先，配置nginx。让 Nginx 把请求转发给 Unix Domain Socket。

.. code-block:: bash

   $ sudo vim /etc/nginx/conf.d/default.conf

   location ~ /echo {
      root           html;
      fastcgi_pass   unix:/etc/ncserver/echo/.ncserver.sock;
      fastcgi_index  index.php;
      fastcgi_param  SCRIPT_FILENAME  /scripts$fastcgi_script_name;
      include        fastcgi_params;
   }

   $ sudo nginx -s reload

运行以下命令，编译ncserver的lib和测试程序echo。

.. code-block:: bash

   $ python make.py
   $ mkdir /etc/ncserver/echo
   $ copy echo /etc/ncserver/echo
   $ ncserverctl start echo
   Starting <echo> on domain socket unix:/etc/ncserver/echo/.ncserver.sock
   spawn-fcgi: child spawned successfully: PID: 32592
   $ curl  "http://127.0.0.1/echo?city=beijing&keyword=coffee"
   Request-Method: GET
   Query String: city=beijing&keyword=coffee
   city: beijing
   keyword: coffee
   
ncserverctl
-----------

`ncserverctl` is the managment program for ncserver services. It's used to start/stop/restart/reload a service.

   | 用法 ：ncserverctl ${operation} [options] ${server_name}
   | 详细使用方法可查看帮助：ncserverctl --help or ncserverctl ${operation} --help

部署要求
^^^^^^^^

1. 服务的程序和数据，都要安装在 /etc/ncserver 下。每个服务一个目录::

      $ ls
      tile-server
      route-server
      junction-view-server
      ti-tile-server

2. The files in each folder must obey the following rules:
   
   a. The name of the executable must be the same as the folder itself.
   b. The name of the configuration file must be the same as the folder itself, such as SERVEICE.ini or SERVICE.json
   c. Each server must contains a test script::

         $ cd /etc/ncserver/route-server
         $ ls
         route-server      // the executable
         route-server.ini  // the configuration file
         test.py       // server's test file

Stop/Stop Service
^^^^^^^^^^^^^^^^^

**ncserverctl** is used to start/stop/reload a server::

   $ ncserverctl start route-server
   Starting 'route-server' on domain socket unix:/etc/ncserver/route-server/.ncserver.sock
   Done
   $ ncserverctl reload route-server
   $ ncserverctl stop route-server

By starting a server, a Unix domain socket is created. It's located in "/etc/ncserver/SERVICE/.ncserver.sock".

Run the test
^^^^^^^^^^^^

The test script must run test on local server. which is "https://localhost/".

To run the test::

   $ ncserverctl route-server test
   test_child_process_age (__main__.RoutingServerTest) ... skipped 'no longer needed after switching to shared memory structure'
   test_enroute_cities (__main__.RoutingServerTest) ... ok
   test_enroute_restriction (__main__.RoutingServerTest) ... ok
   test_past_eta (__main__.RoutingServerTest) ... ok
   test_route (__main__.RoutingServerTest) ... ok
   test_route_brief (__main__.RoutingServerTest) ... ok
   test_route_diffusion (__main__.RoutingServerTest) ... ok

   ----------------------------------------------------------------------
   Ran 7 tests in 0.720s

Change log level
^^^^^^^^^^^^^^^^

Service log level can be dynamically adjusted by **ncserverctl**::

   $ ncserverctl chloglvl debug route-server

Other
^^^^^

ncserverctl 支持.ncserver.yaml配置文件 ( 配置文件目录/etc/ncserver/**${server_name}**/ )。
目前，worker进程数目可在.ncserver.yaml文件中配置 **processCount: 8**，否则会使用默认值4。
其他可根据服务的需求，进一步定制化。

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
