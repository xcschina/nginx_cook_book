### tcp模块

1. 编译选项中需要带--with-stream。如果使用--with-stream=dynamic，则生成的是动态链接库，需要在配置文件中加载动态链接库：
```
load_module "modules/ngx_stream_module.so";
```

2. 基本配置
```
stream {
    server {
        listen 6000;                  # 监听6000端口
        proxy_pass 127.0.0.1:3306;    # 代理3306 mysql端口
    }
}
```

3. 高级配置
```
stream {
    upstream back {
        server 127.0.0.3:555 weight=3;
        server 127.0.0.3:666 weight=10; # 配置后端服务器ip:端口 权重
    }

    server {
        listen 6000;                  # 监听6000端口
        proxy_pass back;              # 代理上面配置的back
        
        proxy_connect_timeout 1s;     #
        proxy_timeout 3s;             #
    }
}
```

4. stream core的变量
```
$binary_remote_addr 二进制格式的客户端地址
$bytes_received 从客户端接收到的字节数
$bytes_sent 发往客户端的字节数
$hostname 连接域名
$msec 毫秒精度的当前时间
$nginx_version nginx 版本
$pid worker进程号
$protocol 通信协议（UDP or TCP）
$remote_addr 客户端ip
$remote_port 客户端端口
$server_addr 接受连接的服务器ip，计算此变量需要一次系统调用。所以避免系统调用，在listen指令里必须指定具体的服务器地址并且使用参数bind。
$server_port 接受连接的服务器端口
$session_time 毫秒精度的会话时间（版本1.11.4开始）
$status 会话状态（版本1.11.4开始), 可以是一下几个值：
200 成功
400 不能正常解析客户端数据
403 禁止访问
500 服务器内部错误
502 网关错误，比如上游服务器无法连接
503 服务不可用，比如由于限制连接等措施导致
$time_iso8601 ISO 8601时间格式
$time_local 普通日志格式的时间戳
```

5. stream的其它模块
```
ngx_stream_core_module
ngx_stream_access_module
ngx_stream_geo_module
ngx_stream_geoip_module
ngx_stream_js_module
ngx_stream_limit_conn_module
ngx_stream_log_module
ngx_stream_map_module
ngx_stream_proxy_module
ngx_stream_realip_module
ngx_stream_return_module
ngx_stream_split_clients_module
ngx_stream_ssl_module
ngx_stream_ssl_preread_module
ngx_stream_upstream_module
ngx_stream_upstream_hc_module
```
