### 通过 upstream 指令和proxy_pass指令，我们可以轻松实现http负载均衡控制

```
http {
    upstream backend {
        # 
        # least_conn; 最少连接
        # ip_hash; 根据客户端ip地址进行hash
        # hash $request_uri consistent; 自定义hash
        # random two least_time=last_byte; 随机
        server backend1.example.com weight=5;
        server backend2.example.com;
        server 192.0.0.1 backup;
    }
    
    server {
        location / {
            proxy_pass http://backend;
        }
    }
}
```

### 负载均衡的方式有以下几种

1. Round Robin，默认的方式：平均分配。根据配置的权重平均分配到各个服务器。

2. Least Connections，最小负载方式，结合最少连接和权重信息分配。

3. IP Hash，根据ip地址的hash值选择服务器，计算hash时，只计算ip地址的前三个字节，保证来自同一个地址的连接被同一个服务器处理。

4. Generic Hash，用户指定hash内容。可以使用path uri等信息进行hash。consistent可选项启用一致性hash，在服务器列表变动时，一致性hash只需要移动很少的key。

5. Random 随机方式，如果配置了two参数，则会随机选出二台服务器，然后根据第三个参数选择最终的服务器。

### 服务器权重

1. 默认权重为1

2. 权重只在Round Robin、Least Connections模式下有效

### 服务器慢启动 slow_start，服务器启动或恢复时，不会一下让它满载，而是慢慢地增加分配的连接。
```
upstream backend {
    server backend1.example.com slow_start=30s;
    server backend2.example.com;
    server 192.0.0.1 backup;
}
```
服务器慢启动功能可防止最近恢复的服务器被连接淹没，这可能会超时并导致服务器再次被标记为失败。

时间值（此处为30秒）设置NGINX 将服务器连接数增加到最大值的时间。

请注意，如果组中只有一个服务器，则忽略server指令的max_fails，fail_timeout和slow_start参数，并且永远不会将服务器视为不可用。

### 服务器被动健康检查

当NGINX认为服务器不可用时，它会暂时停止向服务器发送请求，直到它再次被视为活动状态。 server指令的以下参数配置NGINX认为服务器不可用的条件：

1. max_fails - 设置NGINX将服务器标记为不可用的连续失败尝试次数。

2. fail_timeout - 设置必须发生max_fails参数指定的失败尝试次数以使服务器不可用的时间，以及NGINX在标记为服务器后认为服务器不可用的时间长度。
默认值为1次尝试和10秒。 因此，如果服务器不接受或不响应（即一个）请求，NGINX会立即认为服务器不可用10秒钟。 以下示例显示如何设置这些参数：
```
upstream backend {
    server backend1.example.com;
    server backend2.example.com max_fails=3 fail_timeout=30s;
    server backend3.example.com max_fails=2;
}
```

### 在多个工作进程中共享数据

如果上游块不包含zone指令，则每个工作进程都会保留其自己的服务器组配置副本，并维护自己的一组相关计数器。计数器包括组中每个服务器的当前连接数以及将请求传递到服务器的失败尝试次数。因此，无法动态修改服务器组配置。

当zone指令包含在上游块中时，上游组的配置保存在所有工作进程之间共享的内存区域中。此方案是可动态配置的，因为工作进程访问组配置的相同副本并使用相同的相关计数器。

zone指令对于活动运行状况检查和上游组的动态重新配置是必需的。但是，上游组的其他功能也可以从该指令的使用中受益。

例如，如果未共享组的配置，则每个工作进程都会维护自己的计数器，以便将请求传递给服务器失败（由max_fails参数设置）。在这种情况下，每个请求只能访问一个工作进程。当选择处理请求的工作进程无法将请求传输到服务器时，其他工作进程对此一无所知。虽然某些工作进程可以认为服务器不可用，但其他人可能仍会向此服务器发送请求。对于最终被视为不可用的服务器，fail_timeout参数设置的时间范围内的失败尝试次数必须等于max_fails乘以工作进程数。另一方面，zone指令保证了预期的行为。

同样，如果没有zone指令，Least Connections负载平衡方法可能无法正常工作，至少在低负载下。此方法将请求传递给具有最少活动连接数的服务器。如果未共享组的配置，则每个工作进程使用其自己的计数器作为连接数，并可能将请求发送到另一个工作进程刚刚发送请求的同一服务器。但是，您可以增加请求数以减少此影响。在高负载下，请求在工作进程之间均匀分布，并且最少连接方法按预期工作。

### 设置区域大小

不可能推荐理想的内存区大小，因为使用模式差异很大。 所需的内存量取决于启用了哪些功能（例如会话持久性，运行状况检查或DNS重新解析）以及如何识别上游服务器。

例如，使用sticky_route会话持久性方法并启用单个运行状况检查，256 KB区域可以容纳有关指示的上游服务器数量的信息：

* 128台服务器（每台服务器定义为IP地址：端口对）

* 88个服务器（每个服务器定义为主机名：主机名解析为单个IP地址的端口对）

* 12个服务器（每个服务器定义为主机名：主机名解析为多个IP地址的端口对）



