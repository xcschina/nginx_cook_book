## http从监听到获取header完成

1. 监听套接字读事件发生调用ngx_event_accept处理新连接

* 创建 ngx_connection_t 对象

* 设置读 "读事件" handler 为 ngx_http_wait_request_handler

* 加入连接超时定时器

* 投递"读请求”

2. 超时或有读事件触发调用 ngx_http_wait_request_handler

* 超时，关闭连接。

* 非超时，设置”读事件“ handler为 ngx_http_process_request_line ，并调用 ngx_http_process_request_line

3. ngx_http_process_request_line 主要解析请求行 GET /index.html HTTP/1.0

* 解析请求行，如果报错，结束掉请求。

* 如果解析成功，设置 读事件 handler 为 ngx_http_process_request_headers ，并调用

4. ngx_http_process_request_headers 主要分两部

* ngx_http_read_request_header 读取http header部分数据

* ngx_http_parse_header_line 解析 header 数据

* 调用 ngx_http_process_request 处理请求

5. ngx_http_process_request 函数处理

* 将读写事件处理handler都设置为 ngx_http_process_request

* 调用 ngx_http_handler

6. ngx_http_handler 函数

* r->phase_handler = 0; 将处理步骤设为 NGX_HTTP_POST_READ_PHASE

* 调用 ngx_http_core_run_phases

7. ngx_http_core_run_phases 根据当前阶段，选择遍历当前阶段的 checker 链来处理。
```C++
void ngx_http_core_run_phases(ngx_http_request_t *r)
{
    ngx_int_t                   rc;
    ngx_http_phase_handler_t   *ph;
    ngx_http_core_main_conf_t  *cmcf;

    cmcf = ngx_http_get_module_main_conf(r, ngx_http_core_module);

    ph = cmcf->phase_engine.handlers;

    while (ph[r->phase_handler].checker) {
		// 不同的阶段 checker 函数不同
		// 默认的 checker 为 ngx_http_core_generic_phase
        rc = ph[r->phase_handler].checker(r, &ph[r->phase_handler]);

        if (rc == NGX_OK) {
            return;
        }
    }
}
```

8. 默认 checker ngx_http_core_generic_phase ，直接调用handler，并将当前handler指向链表的下一个
```C++
ngx_int_t ngx_http_core_generic_phase(ngx_http_request_t *r, ngx_http_phase_handler_t *ph)
{
    ngx_int_t  rc;

    /*
     * generic phase checker,
     * used by the post read and pre-access phases
     */

    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "generic phase: %ui", r->phase_handler);

    rc = ph->handler(r);

    if (rc == NGX_OK) {
        r->phase_handler = ph->next;
        return NGX_AGAIN;
    }

    if (rc == NGX_DECLINED) {
        r->phase_handler++;
        return NGX_AGAIN;
    }

    if (rc == NGX_AGAIN || rc == NGX_DONE) {
        return NGX_OK;
    }

    /* rc == NGX_ERROR || rc == NGX_HTTP_...  */

    ngx_http_finalize_request(r, rc);

    return NGX_OK;
}
```


8. http阶段列表
```C++
typedef enum {
    NGX_HTTP_POST_READ_PHASE = 0,   // 接收到完整的HTTP头部后处理的阶段
 
    NGX_HTTP_SERVER_REWRITE_PHASE,  // URI与location匹配前，修改URI的阶段，用于重定向
 
    NGX_HTTP_FIND_CONFIG_PHASE,     // 根据URI寻找匹配的location块配置项
    NGX_HTTP_REWRITE_PHASE,         // 上一阶段找到location块后再修改URI
    NGX_HTTP_POST_REWRITE_PHASE,    // 防止重写URL后导致的死循环
 
    NGX_HTTP_PREACCESS_PHASE,       // 下一阶段之前的准备
 
    NGX_HTTP_ACCESS_PHASE,          // 让HTTP模块判断是否允许这个请求进入Nginx服务器
    NGX_HTTP_POST_ACCESS_PHASE,     // 向用户发送拒绝服务的错误码，用来响应上一阶段的拒绝
 
    NGX_HTTP_TRY_FILES_PHASE,       // 为访问静态文件资源而设置
    NGX_HTTP_CONTENT_PHASE,         // 处理HTTP请求内容的阶段，大部分HTTP模块介入这个阶段
 
    NGX_HTTP_LOG_PHASE              // 处理完请求后的日志记录阶段
} ngx_http_phases;
```

## 按阶段处理http请求

0. NGX_HTTP_POST_READ_PHASE 接收到完整的HTTP头部后处理的阶段，也就是http模块正式开始处理的阶段

* 包含 ngx_http_realip_handler 模块

* checker() 时调用默认的函数 ngx_http_core_generic_phase 。也就相当于直接调用了 ngx_http_realip_handler 函数

* ngx_http_realip_handler 模块是为了获取真实的客户端ip地址。有些请求是代理服务器转发的，代理服务器会在http请求头中包含真实客户端ip，有时候ip会有多个。

1. NGX_HTTP_SERVER_REWRITE_PHASE URI与location匹配前，修改URI的阶段，用于重定向

* 包含 ngx_http_rewrite_module 模块 checker = ngx_http_core_rewrite_phase;
```C++
ngx_int_t ngx_http_core_rewrite_phase(ngx_http_request_t *r, ngx_http_phase_handler_t *ph)
{
    ngx_int_t  rc;

    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "rewrite phase: %ui", r->phase_handler);

    rc = ph->handler(r);

    if (rc == NGX_DECLINED) {
        r->phase_handler++;
        return NGX_AGAIN;
    }

    if (rc == NGX_DONE) {
        return NGX_OK;
    }

    /* NGX_OK, NGX_AGAIN, NGX_ERROR, NGX_HTTP_...  */

    ngx_http_finalize_request(r, rc);

    return NGX_OK;
}
```
* 