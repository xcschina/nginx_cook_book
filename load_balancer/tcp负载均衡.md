### 介绍

负载平衡是指跨多个后端服务器有效地分配网络流量。

从1.9.0开始，nginx就支持对TCP的转发，而到了1.9.13时，UDP转发也支持了。提供此功能的模块为ngx_stream_core。不过Nginx默认没有开启此模块，所以需要手动安装。

### 先决条件

使用--with-stream配置标志构建的最新开源NGINX（无需额外构建步骤）

通过TCP或UDP进行通信的应用程序，数据库或服务

上游服务器，每个服务器运行应用程序，数据库或服务的相同实例

### 配置样例：
```
stream {
    server {
        listen     12345;
        #TCP traffic will be forwarded to the "stream_backend" upstream group
        proxy_pass stream_backend;
    }
    server {
        listen     12346;
        #TCP traffic will be forwarded to the specified server
        proxy_pass backend.example.com:12346;
    }
    server {
        listen     53 udp;
        #UDP traffic will be forwarded to the "dns_servers" upstream group
        proxy_pass dns_servers;
    }
    # ...
}
```

如果代理服务器具有多个网络接口，则可以选择将NGINX配置为在连接到上游服务器时使用特定的源IP地址。 如果NGINX后面的代理服务器配置为接受来自特定IP网络或IP地址范围的连接，这可能很有用。

包括proxy_bind指令和相应网络接口的IP地址：
```
stream {
    # ...
    server {
        listen     127.0.0.1:12345;
        proxy_pass backend.example.com:12345;
        proxy_bind 127.0.0.1:12345;
    }
}
```

或者，您可以调整两个内存缓冲区的大小，其中NGINX可以从客户端和上游连接中放入数据。 如果存在少量数据，则可以减少缓冲区，这可以节省存储器资源。 如果存在大量数据，则可以增加缓冲区大小以减少套接字读/写操作的数量。 一旦在一个连接上收到数据，NGINX就会读取它并通过另一个连接转发它。 使用proxy_buffer_size指令控制缓冲区：
```
stream {
    # ...
    server {
        listen            127.0.0.1:12345;
        proxy_pass        backend.example.com:12345;
        proxy_buffer_size 16k;
    }
}
```

### 配置上游组使用的负载均衡方法。您可以指定以下方法之一：

* Round Robin (循环) - 默认情况下，NGINX使用循环算法对流量进行负载平衡，将其顺序指向已配置上游组中的服务器。因为它是默认方法，所以没有循环指令;只需在顶级流{}上下文中创建上游{}配置块，然后按照上一步中的说明添加服务器指令。

* Least Connections (最少连接) - NGINX选择具有较少数量的当前活动连接的服务器。

* Hash (哈希) - NGINX根据用户定义的密钥选择服务器，例如源IP地址（$ remote_addr）：
```
upstream stream_backend {
    hash $ remote_addr;
    server backend1.example.com:12345;
    server backend2.example.com:12345;
    server backend3.example.com:12346;
}
```
哈希负载平衡方法还用于配置会话持久性。由于散列函数基于客户端IP地址，因此来自给定客户端的连接始终传递到同一服务器，除非服务器已关闭或不可用。指定可选的一致参数以应用ketama一致性散列方法：
```
hash $ remote_addr一致;
```

* Random (随机) - 每个连接将传递给随机选择的服务器。如果指定了两个参数，首先，NGINX会考虑服务器权重随机选择两台服务器，然后使用指定的方法选择其中一台服务器：

* least_conn - 活动连接数最少
* least_time = connect（NGINX Plus） - 连接上游服务器的时间（$ upstream_connect_time）
* least_time = first_byte（NGINX Plus） - 从服务器接收第一个数据字节的平均时间最短（$ upstream_first_byte_time）
* least_time = last_byte（NGINX Plus） - 从服务器接收最后一个数据字节的平均时间最短（$ upstream_session_time）
```
upsream stream_backend {
    random two_time = last_byte;
    server backend1.example.com:12345;
    server backend2.example.com:12345;
    server backend3.example.com:12346;
    server backend4.example.com:12346;
}
```

随机负载平衡方法应该用于多个负载均衡器将请求传递到同一组后端的分布式环境。对于负载均衡器具有所有请求的完整视图的环境，请使用其他负载平衡方法，例如循环，最少连接和最少时间。

### 可选项：对于每个上游服务器，指定服务器特定的参数，包括最大连接数，服务器权重等：
```
upstream stream_backend {
    hash   $remote_addr consistent;
    server backend1.example.com:12345 weight=5;
    server backend2.example.com:12345;
    server backend3.example.com:12346 max_conns=3;
}
upstream dns_servers {
    least_conn;
    server 192.168.136.130:53;
    server 192.168.136.131:53;
    # ...
}
```

另一种方法是将流量代理到单个服务器而不是upstream组。 如果按主机名标识服务器，并将主机名配置为解析为多个IP地址，则NGINX使用循环算法对IP地址之间的流量进行负载平衡。 在这种情况下，您必须在proxy_pass指令中指定服务器的端口号，并且不得在IP地址或主机名之前指定协议：


### 被动健康监测

如果连接到上游服务器的尝试超时或导致错误，则开源NGINX或NGINX Plus可将服务器标记为不可用，并在指定的时间内停止向其发送请求。 要定义NGINX认为上游服务器不可用的条件，请在server指令中包含以下参数：

* fail_timeout - 指定数量的连接尝试必须失败才能使服务器不可用的时间。 此外，NGINX在标记服务器后认为服务器不可用的时间量。

* max_fails - max_fails在指定时间内认为服务器不可用的失败尝试次数。

* 默认值为10秒和1次尝试。 因此，如果连接尝试在10秒内超时或至少失败一次，NGINX会将服务器标记为不可用10秒钟。 该示例显示如何在30秒内将这些参数设置为2个故障：

```
  上游 stream_backend {
     server backend1.example.com:12345 weight = 5 ;
     server backend2.example.com:12345 max_fails = 2 fail_timeout = 30s ;
     server backend3.example.com:12346 max_conns = 3 ;
 }
```

### 配置实例
```
stream {
    upstream stream_backend {
        least_conn;
        server backend1.example.com:12345 weight=5;
        server backend2.example.com:12345 max_fails=2 fail_timeout=30s;
        server backend3.example.com:12345 max_conns=3;
    }
    
    upstream dns_servers {
        least_conn;
        server 192.168.136.130:53;
        server 192.168.136.131:53;
        server 192.168.136.132:53;
    }
    
    server {
        listen        12345;
        proxy_pass    stream_backend;
        proxy_timeout 3s;
        proxy_connect_timeout 1s;
    }
    
    server {
        listen     53 udp;
        proxy_pass dns_servers;
    }
    
    server {
        listen     12346;
        proxy_pass backend4.example.com:12346;
    }
}
```
在此示例中，所有与TCP和UDP代理相关的功能都在stream块内配置，就像在http块中配置HTTP请求的设置一样。

有两个命名的upstream块，每个块包含三个服务器，它们彼此承载相同的内容。 在eadch服务器的服务器中，服务器名称后跟强制端口号。 根据最少连接负载平衡方法在服务器之间分配连接 ：连接以最少数量的活动连接进入服务器。

三个server块定义了三个虚拟服务器：

* 第一个服务器侦听端口12345并代理到stream_backend上游服务器组的所有TCP连接。 请注意，在stream模块的上下文中定义的proxy_pass指令不得包含协议。指定了两个可选的超时参数： proxy_connect_timeout指令设置与stream_backend组中的服务器建立连接所需的超时。 proxy_timeout指令设置代理到stream_backend组中的一个服务器启动后使用的超时。

* 第二个服务器侦听端口53，并将所有UDP数据报（ listen指令的udp参数）代理到名为dns_servers的上游组。 如果未指定udp参数，则套接字将侦听TCP连接。

* 第三个虚拟服务器侦听端口12346并代理到backend4.example.com的 TCP连接，后者可以解析为使用Round Robin方法进行负载平衡的多个IP地址。


