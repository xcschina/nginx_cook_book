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
