ncserver
========

中文 | `English <../README.rst>`__

ncserver是基于FastCGI和Nginx的一个C++在线服务开发框架。
在NavInfo，它是我们开发出搜索、算路、路况等服务的基础。

.. image:: docs/architecture.png

样例程序
--------

本样例(在"example"目录中)展示了 ncserver 的基本功能。

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

测试效果::

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

优势特性
--------

除了入门容易之外，ncserver还有以下优势：

1. 多进程架构
   
   每个进程都是独立的。即使某个请求导致其中一个进程崩溃，后续请求也不受影响。

   因为利用了 Linux 操作系统的写时复制(copy-on-write)特性。所以进程可以共享静态内存。
   即使开很多进程，也不会占用太多内存。

2. 自动补充 worker 进程
   
   Deamon 进程会监控所有worker进程。
   如果某个 worker 进程崩溃了。会自动补充一个新的worker。

3. 无缝(零宕机) reload
   
   如果配置文件或者数据文件改变。服务可以无缝重启。

本项目包含内容
--------------

1. 源代码。可以编译为静态库使用。也可以直接加入工程使用。
2. 服务管理脚本 ncserverctl
3. 一个 example 程序。见 "example" 目录。
4. 预先配置好的Windows版Nginx，方便测试。见dependency目录。

ncserver支持Linux和Windows。Windows只能用于功能调试，不能提供多进程支持和好的性能。
Linux用于压力测试和正式产品部署。

Window下编译&测试
-----------------

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
-----------------

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

`ncserverctl` 是服务管理工具，用于 start/stop/restart/reload 服务。

.. warning:: 需要补充

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
