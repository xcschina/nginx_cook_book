### 概观

启用缓存后，NGINX会将响应保存在磁盘缓存中，并使用它们响应客户端，而无需每次都代理对相同内容的请求。

### 启用响应缓存

要启用缓存，请在顶级http {}上下文中包含proxy_cache_path指令。 必需的第一个参数是缓存内容的本地文件系统路径，必需的keys_zone参数定义用于存储有关缓存项的元数据的共享内存区的名称和大小：

```
  http {
     ...
     proxy_cache_path / data / nginx / cache keys_zone = one：10m ;
 }
 ```
 
然后在要为其缓存服务器响应的上下文（协议类型，虚拟服务器或位置）中包含proxy_cache指令，并指定由proxy_cache_path指令（在本例中为one ）的keys_zone参数定义的区域名称：

```
  http {
     ...
     proxy_cache_path / data / nginx / cache keys_zone = one：10m ;
     server {
         proxy_cache mycache ;
         location / {
             proxy_pass http://localhost：8000 ;
         }
     }
 }
 ```
 
请注意， keys_zone参数定义的大小不会限制缓存响应数据的总量。 缓存的响应本身与元数据的副本一起存储在文件系统的特定文件中。 要限制缓存的响应数据量，请将max_size参数包含在proxy_cache_path指令中。 （但请注意，缓存数据的数量可能会暂时超过此限制，如以下部分所述。）

### 参与缓存的NGINX进程

缓存涉及两个额外的NGINX进程：

* 定期激活高速缓存管理器以检查高速缓存的状态。 如果高速缓存大小超过max_size参数为proxy_cache_path指令设置的限制，则高速缓存管理器将删除最近最少访问的数据。 如前所述，缓存管理器激活之间的缓存数据量可能会暂时超过限制。

* 在NGINX启动后， 缓存加载程序只运行一次。 它将有关先前缓存数据的元数据加载到共享内存区域。 一次加载整个缓存可能会消耗足够的资源来在启动后的最初几分钟内降低NGINX的性能。 要避免这种情况，请通过将以下参数包含到proxy_cache_path指令来配置缓存的迭代加载：

** loader_threshold - 迭代持续时间，以毫秒为单位（默认为200 ）

** loader_files - 一次迭代中加载的最大项目数（默认为100 ）

** loader_sleeps - 迭代之间的延迟，以毫秒为单位（默认为50 ）

在以下示例中，迭代持续300毫秒或直到加载了200项目：
```
  proxy_cache_path / data / nginx / cache keys_zone = one：10m loader_threshold = 300 loader_files = 200 ;
  
### 指定要缓存的请求

默认情况下，NGINX Plus会在第一次从代理服务器收到此类响应时，缓存对使用HTTP GET和HEAD方法发出的请求的所有响应。 作为请求的密钥（标识符），NGINX Plus使用请求字符串。 如果请求具有与缓存响应相同的密钥，则NGINX Plus会将缓存的响应发送到客户端。 您可以在http {} ， server {}或location {}上下文中包含各种指令，以控制缓存哪些响应。

要更改计算密钥时使用的请求特性，请包含proxy_cache_key指令：

```
  proxy_cache_key “ $ host $ request_uri $ cookie_user” ;
```

要定义在缓存响应之前必须使用相同密钥的请求的最小次数，请包含proxy_cache_min_uses指令：
```
  proxy_cache_min_uses 5 ;
```

要使用GET和HEAD以外的方法缓存对请求的响应，请将它们与GET和HEAD列为proxy_cache_methods指令的参数：
```
  proxy_cache_methods GET HEAD POST ;
```

### 限制或禁用缓存

默认情况下，响应会无限期地保留在缓存中。 只有当缓存超过最大配置大小时才会删除它们，然后按照上次请求的时间长度顺序删除它们。 您可以通过在http {} ， server {}或location {}上下文中包含指令来设置缓存响应被认为有效的时间长度，甚至是否可以使用它们：

要限制具有特定状态代码的缓存响应被视为有效的时间，请包含proxy_cache_valid指令：
```
 proxy_cache_valid 200 302 10m ;
 proxy_cache_valid 404 1m ;
```

在此示例中，代码200或302响应被认为有效10分钟，而代码404响应有效1分钟。 要使用所有状态代码定义响应的有效时间，请将any指定为第一个参数：
```
  proxy_cache_valid any 5m ;
```

要定义NGINX Plus不向客户端发送缓存响应的条件，请包含proxy_cache_bypass指令。 每个参数定义一个条件，并由许多变量组成。 如果至少有一个参数不为空并且不等于“0”（零），则NGINX Plus不会在缓存中查找响应，而是立即将请求转发到后端服务器。
```
  proxy_cache_bypass $ cookie_nocache $ arg_nocache $ arg_comment ;
```

要定义NGINX Plus根本不缓存响应的条件，请包含proxy_no_cache指令，以与proxy_cache_bypass指令相同的方式定义参数。
```
  proxy_no_cache $ http_pragma $ http_authorization ;
```

### 清除缓存中的内容

NGINX可以从缓存中删除过时的缓存文件。 这对于删除过时的缓存内容以防止同时提供新旧版本的网页是必要的。 在收到包含自定义HTTP标头或HTTP PURGE方法的特殊“清除”请求时，将清除缓存。

### 配置缓存清除

让我们设置一个配置，用于标识使用HTTP PURGE方法的请求并删除匹配的URL。

在http {}上下文中，创建一个新变量，例如$purge_method ，它取决于$request_method变量：
```
  http {
     ...
     map $ request_method $ purge_method {
         PURGE 1 ;
         default 0 ;
     }
 }
```

在配置了缓存的location {}块中，包含proxy_cache_purge指令以指定缓存清除请求的条件。 在我们的示例中，它是上一步中配置的$purge_method ：
```
  server {
     listen 80 ;
     server_name www.example.com ;

     location / {
         proxy_pass https：// localhost：8002 ;
         proxy_cache mycache ;

         proxy_cache_purge $ purge_method ;
     }
 }
```

### 发送清除命令

配置proxy_cache_purge指令后，需要发送特殊的缓存清除请求来清除缓存。 您可以使用一系列工具发出清除请求，包括curl命令，如下例所示：
```
$ curl -X PURGE -D – "https://www.example.com/*"
HTTP/1.1 204 No Content
Server: nginx/1.15.0
Date: Sat, 19 May 2018 16:33:04 GMT
Connection: keep-alive
```

在该示例中，将清除具有公共URL部分（由星号通配符指定）的资源。 但是，这些缓存条目不会从缓存中完全删除：它们会保留在磁盘上，直到它们被删除，因为它们不存在（由proxy_cache_path指令的inactive参数确定）或缓存proxy_cache_path （使用proxy_cache_path参数启用到proxy_cache_path ）或客户端尝试访问它们。


### 限制对清除命令的访问

我们建议您限制允许发送缓存清除请求的IP地址数：
```
geo $purge_allowed {
   default         0;  # deny from other
   10.0.0.1        1;  # allow from localhost
   192.168.0.0/24  1;  # allow from 10.0.0.0/24
}

map $request_method $purge_method {
   PURGE   $purge_allowed;
   default 0;
}
 ```
在此示例中，NGINX检查是否在请求中使用了PURGE方法，如果是，则分析客户端IP地址。 如果IP地址列入白名单，则$purge_method设置为$purge_allowed ： 1允许清除， 0拒绝清除。


### 从缓存中完全删除文件

要完全删除与星号匹配的缓存文件，请激活一个特殊的cache purger清除进程，该进程将永久遍历所有缓存条目并删除与通配符密钥匹配的条目。 将proxy_cache_path参数包含在http {}上下文中的proxy_cache_path指令中：
```
  proxy_cache_path / data / nginx / cache levels = 1：2 keys_zone = mycache：10m purger = on ;
```

缓存清除配置示例
```
  http {
     ...
     proxy_cache_path / data / nginx / cache levels = 1：2 keys_zone = mycache：10m purger = on ;

     map $ request_method $ purge_method {
         PURGE 1 ;
         default 0 ;
     }

     server {
         listen 80 ;
         server_name www.example.com ;

         location / {
             proxy_pass https：// localhost：8002 ;
             proxy_cache mycache ;
             proxy_cache_purge $ purge_method ;
         }
     }

     geo $ purge_allowed {
       default 0 ;
        10.0.0.1 1 ;
        192.168.0.0/24 1 ;
     }

     map $ request_method $ purge_method {
        PURGE $ purge_allowed ;
        default 0 ;
     }
 }
```

### 字节范围缓存

初始缓存填充操作有时需要很长时间，尤其是对于大型文件。 例如，当视频文件开始下载以满足对文件的一部分的初始请求时，后续请求必须等待整个文件被下载并放入高速缓存中。

NGINX可以缓存这些范围请求，并逐渐使用Cache Slice模块填充缓存，该模块将文件分成较小的“片”。 每个范围请求选择覆盖请求范围的特定切片，如果此范围仍未缓存，则将其放入缓存中。 对这些片的所有其他请求都从缓存中获取数据。

要启用字节范围缓存：

确保使用Cache Slice模块编译NGINX。

使用slice指令指定切片的大小：
```
  location / {
     slice 1m ;
 }
 ```
 
选择切片大小，使切片快速下载。 如果大小太小，则内存使用率可能过高，并且在处理请求时会打开大量文件描述符，而过大的大小可能会导致延迟。

将$slice_range变量包含在缓存键中：
```
  proxy_cache_key $ uri $ is_args $ args $ slice_range ;
```

使用206状态代码启用响应缓存：
```
  proxy_cache_valid 200 206 1h ;
```

通过在Range头字段中设置$slice_range变量，允许将范围请求传递到代理服务器：
```
  proxy_set_header 范围 $ slice_range ;
```

这是完整的配置：
```
  location / {
     slice 1m ;
     proxy_cache cache ;
     proxy_cache_key $ uri $ is_args $ args $ slice_range ;
     proxy_set_header Range $ slice_range ;
     proxy_cache_valid 200 206 1h ;
     proxy_pass http：// localhost：8000 ;
 }
```

请注意，如果启用切片缓存，则不得更改初始文件。

### 组合配置示例

以下示例配置结合了上述一些缓存选项。

  http {
     ...
     proxy_cache_path / data / nginx / cache keys_zone = one：10m loader_threshold = 300 
                      loader_files = 200 max_size = 200m ;

     Server {
         listen 8080 ;
         proxy_cache mycache ;

         location / {
             proxy_pass http：// backend1 ;
         }

         location / some / path {
             proxy_pass http：// backend2 ;
             proxy_cache_valid any 1m ;
             proxy_cache_min_uses 3 ;
             proxy_cache_bypass $ cookie_nocache $ arg_nocache $ arg_comment ;
         }
     }
 }
 
在此示例中，两个位置使用相同的缓存，但方式不同。

由于backend1响应很少更改，因此不包含缓存控制指令。 响应在第一次发出请求时被缓存，并且无限期保持有效。

相反，对backend2提供的请求的响应经常发生变化，因此它们仅被认为有效1分钟，并且在相同的请求进行3次之前不会被缓存。 此外，如果请求与proxy_cache_bypass指令定义的条件匹配，NGINX 会立即将请求传递给backend2而不会在缓存中查找相应的响应。
