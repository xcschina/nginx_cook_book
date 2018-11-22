### 编译源码安装

1. 编译安装 PCRE , nginx core 和 Rewrite 模块使用PCRE处理正则表达式
```
$ wget ftp://ftp.csx.cam.ac.uk/pub/software/programming/pcre/pcre-8.42.tar.gz
$ tar -zxf pcre-8.42.tar.gz
$ cd pcre-8.42
$ ./configure
$ make
$ sudo make install
```

2. 编译安装 zlib, nginx gzip模块使用。
```
$ wget http://zlib.net/zlib-1.2.11.tar.gz
$ tar -zxf zlib-1.2.11.tar.gz
$ cd zlib-1.2.11
$ ./configure
$ make
$ sudo make install
```

3. 编译安装 OpenSSL，用来支持https协议。
```
$ wget http://www.openssl.org/source/openssl-1.0.2p.tar.gz
$ tar -zxf openssl-1.0.2p.tar.gz
$ cd openssl-1.0.2p
$ ./Configure darwin64-x86_64-cc --prefix=/usr
$ make
$ sudo make install
```

4. 下载 nginx 源代码
```
$ wget https://nginx.org/download/nginx-1.15.4.tar.gz
$ tar zxf nginx-1.15.4.tar.gz
$ cd nginx-1.15.4
```

5. 配置 nginx 编译选项，可以使用 ./configure --help 查询支持的编译选项帮助信息
```
$ ./configure
--sbin-path=/usr/local/nginx/nginx
--conf-path=/usr/local/nginx/nginx.conf
--pid-path=/usr/local/nginx/nginx.pid
--with-pcre=../pcre-8.42
--with-zlib=../zlib-1.2.11
--with-http_ssl_module
--with-stream
--with-mail=dynamic
--add-module=/usr/build/nginx-rtmp-module
--add-dynamic-module=/usr/build/3party_module
```

6. 编译安装
```
$ make
$ sudo make install
```

7. 启动 nginx, 默认使用 /etc/nginx目录中的配置文件，也可以使用 nginx -c a.conf来指定配置文件
```
$ sudo nginx
```

8. 校验是否启动成功，能获取到http返回，表示 nginx 运行正常。
```
$ sudo curl -I http://localhost/
HTTP/1.1 200 OK
Server: nginx/1.13.8
```

