ncserver
========

ncserver是基于FastCGI和Nginx的一个在线功能的开发框架。
使用此框架，可以方便的开发出在线搜索、算路、TMC等服务。

.. image:: docs/architecture.png

ncserver包括以下文件：

* ncserver.h
* ncserver.lib(or libncserver.a)
* ncserver.sh启动脚本。

ncserver支持Linux和Windows。Windows只能用于功能调试，不能提供多进程支持和好的性能。
Linux用于压力测试和正式产品部署。

Window下编译&测试
-----------------

按照以下步骤编译和测试。

1. 配置dependency/nginx-1.7.2/conf/nginx.conf

	::	

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

首先，配置nginx。

.. code-block:: bash

	$ sudo vim /etc/nginx/conf.d/default.conf

	location ~ /echo {
		root           html;
		fastcgi_pass   unix:/tmp/echo.sock;
		fastcgi_index  index.php;
		fastcgi_param  SCRIPT_FILENAME  /scripts$fastcgi_script_name;
		include        fastcgi_params;
	}

	$ sudo nginx -s reload

运行以下命令，编译ncserver的lib和测试程序echo。

.. code-block:: bash

	$ python make.py
	$ ./ncserver.sh -s start -m lib/echo.out -d /tmp/echo.sock
	Starting lib/echo.out, threadCount= ...
	spawn-fcgi: child spawned successfully: PID: 32592
	$ python test.py
	.
	----------------------------------------------------------------------
	Ran 1 test in 0.004s

	OK

API说明
-------

Example目录下，实现了一个最简单的样例服务echo.cpp。可以参考。

1. 包含头文件 ncserver.h
2. 继承 ncserver::NcServer 类，实现相关接口。只有NcServer::query()必须实现。
3. 在 main() 中实例化 NcServer 的子类，并调用 runAndFork(PORT)。PORT只在Windows下起作用。
   
ncserver.sh说明
---------------

ncserver.sh是linux下的启动器。它可以指定端口或者Unix Domain Socket，也能指定fork的进程数量。
具体说明用-h查看。

Linux下，推荐用Unix Domain Socket，而不是端口号。这样性能更好。

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

Trouble Shoot
-------------

502 Bad Gateway
^^^^^^^^^^^^^^^

大量并发的时候，可能会有部分502错误。ab -n 100000 -c 200，200并发就发生了。
而100并发没有这个问题。观察nginx日志，会看到::

   2016/01/29 18:10:43 [error] 3059#0: *9190789 connect() to unix:/tmp/nds-tile-server.sock failed (11: Resource temporarily unavailable) while connecting to upstream, client: 192.168.85.22, server: fastcgi.mapbar.com, request: "GET /get/qvf?key=1156125669&fields=gridId,gridData HTTP/1.1", upstream: "fastcgi://unix:/tmp/nds-tile-server.sock:", host: "192.168.0.86"

是nginx无法和ncserver通过unix端口建立联系。

最后发现是spawn-fcgi -b backlog 参数的问题。改为512，就可以承受256并发了。
原来，请求会在unix domain socket上排队。要承受高并发，必须修改这个排队的限制数。

另外，某些CPU bound的服务，比如算路，应用以上修改后，依然有问题。
采用以下方法可以解决：

   `sysctl -A`，检查net.core.somaxconn、net.core.netdev_max_backlog和net.ipv4.tcp_max_syn_backlog。
   
   vim /etc/sysctl.conf可以修改这几个参数。`sysctl -p`应用。

如果问题还没解决，可以试试：（按理说应该不需要改这个）

   ulimit -a，查看open files

   修改 /etc/security/limits.conf，加入:

   * soft nofile 1000000
   * hard nofile 1000000

联系
----

如果发现任何bug，请联系 陈博伟 <mailto://chenbw@mapbar.com>`_ 。