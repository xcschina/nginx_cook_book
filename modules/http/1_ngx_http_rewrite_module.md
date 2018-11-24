转自 https://www.jianshu.com/p/49b3f81e47f8

## 简介
ngx_http_rewrite_module 模块可用于修改 HTTP 请求的 URI，支持正则表达式匹配。修改之后，返回重定向指令。
该模块也支持条件式地选择配置。

ngx_http_rewrite_module 模块的指令按如下顺序进行处理：

* 在 server 区块中，按顺序执行；

* 重复执行：

- 根据 HTTP 请求的 URI，寻找对应的 location；

- 在找到的 location 中，ngx_http_rewrite_module 模块的指令按顺序执行；

- 在 location 区块中，如果 HTTP 请求的 URI 被重写，将重复整个步骤，但不超过 10 次。

## 模块指令
```
Syntax: break;
Default: —
Context: server, location, if
```
停止当前 ngx_http_rewrite_module 模块指令集的处理。

如果该指令是在 location 中，那么在这个 location 中，对请求的进一步处理将继续进行。

```
Syntax: if (condition) { ... }
Default: —
Context: server, location
```
若条件判断为真，则执行花括号内的指令，并且将括号内的配置应用到 HTTP 请求上。if 指令中的配置是从上一层配置中继承的。
条件判断式的有如下种类：

- 一个变量名；当变量值为0，或为空字符串时，条件为 false；（1.0.1 版之前，以0开头的字符串被判断为 false）
- 对字符串变量进行比较，使用 = 或 !=；
- 测试一个变量是否与一个正则表达式匹配，正则表达式前使用 ~ 或 ~* 操作符，~* 对大小写不敏感。正则表达式可包含“反向引用”，使用 $1..$9 引用匹配的字符串。也支持 !~ 或 !~* 操作符，表示不匹配。如果正则表达式中含有 } 或 ; 符号，应使用单引号或双引号将正则表达式引用起来。
- 测试一个文件是否存在：-f and !-f；
- 测试一个目录是否存在：-d and !-d；
- 测试一个文件、目录、或符号链接文件是否存在：-e and !-e；
- 测试一个文件是否可执行：-x and !-x。

例子：
```
if ($http_user_agent ~ MSIE) {
    rewrite ^(.*)$ /msie/$1 break;
}

if ($http_cookie ~* "id=([^;]+)(?:;|$)") {
    set $id $1;
}

if ($request_method = POST) {
    return 405;
}

if ($slow) {
    limit_rate 10k;
}

if ($invalid_referer) {
    return 403;
}
```
$invalid_referer 是一个内建变量，由 valid_referers 指令进行设置。

```
Syntax: return code [text];
return code URL;
return URL;
Default: —
Context: server, location, if
```
停止处理，返回状态码 code 给客户端。返回 444 将关闭该连接，而且不发送响应 header。

从 0.8.42 版开始，可指定重定向 URL (限于 301，302，303，307 状态码)，或是指定响应 body 为 text (对于除了前面的状态码之外的所有状态码)。不论是指定重定向 URL 还是 text，其中都可以包含变量。重定向的 URL 可以指定为基于本机的 URI，这时，完整的重定向 URL 是根据请求 scheme ($scheme)，server_name_in_redirect 和 port_in_redirect 指令来构建的。

对于临时重定向，一个 URL 和 302 状态码可被指定为唯一的参数。这样的参数必须以 “http://”, “https://”, or “$scheme” 为起始。URL 可包含变量。


```
Syntax: rewrite regex replacement [flag];
Default: —
Context: server, location, if
```

如果 regex 匹配了一个请求的 URI，该 URI 被替换为 replacement。rewrite 指令在配置文件中按照出现的顺序执行。可使用 flag 中止进一步的处理。如果 replacement 以 “http://”, “https://” 为起始，将中止处理，并返回重定向指令给客户端。

flag 参数的值有：

- last：停止当前 ngx_http_rewrite_module 模块指令集的处理，并为修改后的 URI 寻找新的匹配的 location。

- break：停止当前 ngx_http_rewrite_module 模块指令集的处理，与 break 指令作用相同。

- redirect：返回 “临时重定向” 及 302 状态码；仅当 replacement 不以 “http://”, “https://” 为起始时，该 flag 才生效。

permanent：返回 “永久重定向” 及 301 状态码

例子：
```
server {
    ...
    rewrite ^(/download/.*)/media/(.*)\..*$ $1/mp3/$2.mp3 last;
    rewrite ^(/download/.*)/audio/(.*)\..*$ $1/mp3/$2.ra  last;
    return  403;
    ...
}
```
但如果这些指令被放入 “/download/” location 区块中，应将 last flag 替换为 break，否则 nginx 会不断循环，达到 10 次后，返回 500 error。
```
location /download/ {
    rewrite ^(/download/.*)/media/(.*)\..*$ $1/mp3/$2.mp3 break;
    rewrite ^(/download/.*)/audio/(.*)\..*$ $1/mp3/$2.ra  break;
    return  403;
}

```

如果 replacement 包含请求参数，原来的请求参数将被追加在后面。如果不希望追加原来的请求参数，可在 replacement 字符串的末尾添加一个 “?” 符号，例如：
```
rewrite ^/users/(.*)$ /show?user=$1? last;
```

再提醒一次，如果正则表达式中含有 “}” 或 “;” 符号，应使用单引号或双引号将正则表达式引用起来。


```
Syntax: rewrite_log on | off;
Default:
rewrite_log off;
Context: http, server, location, if
```

是否开启 ngx_http_rewrite_module 模块的日志，如果开启，该模块的日志将被记录进入 error_log 中，日志的级别为 notice。



```
Syntax: set $variable value;
Default: —
Context: server, location, if
```
为变量赋值。value 可包含：文本，变量，或文本和变量的组合。



```
Syntax: uninitialized_variable_warn on | off;
Default:
uninitialized_variable_warn on;
Context: http, server, location, if
```
当有变量没有初始化时，是否记录日志。


## 内部实现
ngx_http_rewrite_module 模块的指令在配置阶段被编译进内部指令中，然后在对请求进行处理时被解释执行。解释指令的解释器是一个简单的虚拟栈机器。

```
location /download/ {
    if ($forbidden) {
        return 403;
    }

    if ($slow) {
        limit_rate 10k;
    }

    rewrite ^/(download/.*)/media/(.*)\..*$ /$1/mp3/$2.mp3 break;
}
```

将被翻译为如下的指令：
```
variable $forbidden
check against zero
    return 403
    end of code
variable $slow
check against zero
match of regular expression
copy "/"
copy $1
copy "/mp3/"
copy $2
copy ".mp3"
end of regular expression
end of code
```

有没有注意到，limit_rate 指令并没有没翻译，这是因为 limit_rate 指令不属于 ngx_http_rewrite_module 模块。由于有 if 区块，一份独立的配置将被创建，如果该 if 区块的条件判断为真，那么其中的配置将应用到请求中，在这里的配置是 limit_rate = 10k。


指令：
```
rewrite ^/(download/.*)/media/(.*)\..*$ /$1/mp3/$2.mp3 break;
```
可进一步优化，将第一个 “/” 放入括号中：
```
rewrite ^(/download/.*)/media/(.*)\..*$ $1/mp3/$2.mp3 break;
```
对应的指令为：
```
match of regular expression
copy $1
copy "/mp3/"
copy $2
copy ".mp3"
end of regular expression
end of code
```
减少了 copy "/" 这一步骤。




