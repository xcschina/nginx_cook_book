代码路径：src/event/ngx_accept.c

代码主要逻辑如下：
```C
void ngx_event_accept(ngx_event_t *ev){
	ngx_connection_t* lc = ev->data;
    ngx_listening_t* ls = lc->listening;

	// 监听套接字可读时，可能是多个套接字连上来了,所以需要多次调用accept，直到返回错误。
	while(true){
	    // accept连接
	    ngx_socket_t s = accept(lc->fd, &sa.sockaddr, &socklen);

		if(s == (ngx_socket_t) -1){
			return
		}
	    
	    // 创建connection结构
	    ngx_connection_t* c = ngx_get_connection(s, ev->log);
	    
	    // 设置连接
	    c->type = SOCK_STREAM;
	    c->pool = ngx_create_pool(ls->pool_size, ev->log);
	    
	    // 客户端地址
	    c->sockaddr = ngx_palloc(c->pool, socklen);
	    ngx_memcpy(c->sockaddr, &sa, socklen);
	    
	    // 设置为非阻塞
	    ngx_nonblocking(s)
	    
	    // read事件handler
	    c->recv = ngx_recv;
	    // write事件handler
        c->send = ngx_send;
        c->recv_chain = ngx_recv_chain;
        c->send_chain = ngx_send_chain;
        
	    // 日志
        c->log = log;
        c->pool->log = log;
        
	    // 本地地址
        c->socklen = socklen;
        c->listening = ls;
        c->local_sockaddr = ls->sockaddr;
        c->local_socklen = ls->socklen;
	    
	    // 自增编号
	    c->number = ngx_atomic_fetch_add(ngx_connection_counter, 1);
		
		// 调用监听事件的handler: ngx_http_init_connection
		ls->handler(c);
	}
}
```

2. 监听套接字的handler处理
```C
void ngx_http_init_connection(ngx_connection_t *c)
{
	// 分配 ngx_http_connection_t 结构
	ngx_http_connection_t* hc = ngx_pcalloc(c->pool, sizeof(ngx_http_connection_t));

	c->data = hc;
	
	ctx = ngx_palloc(c->pool, sizeof(ngx_http_log_ctx_t));
    if (ctx == NULL) {
        ngx_http_close_connection(c);
        return;
    }

	// 分配 ctx 结构
    ctx = ngx_palloc(c->pool, sizeof(ngx_http_log_ctx_t));
    ctx->connection = c;
    ctx->request = NULL;
    ctx->current_request = NULL;

    c->log->connection = c->number;
    c->log->handler = ngx_http_log_error;
    c->log->data = ctx;
    c->log->action = "waiting for request";
	
	// 设置read write的handler函数
	c->read->handler = ngx_http_wait_request_handler;
    c->write->handler = ngx_http_empty_handler;
	
	// 增加连接超时定时器
	ngx_add_timer(rev, c->listening->post_accept_timeout); // 见3、4
	
	// 投递读请求
	ngx_handle_read_event(rev, 0); // 见 5
}
```

3. 设置连接超时定时器时长，默认为60秒
```C
ngx_conf_merge_msec_value(conf->client_header_timeout,prev->client_header_timeout, 60000);

ls->post_accept_timeout = cscf->client_header_timeout;
```

4. 连接超时后处理逻辑，超时时会调用c->read->handler
```C
static void ngx_http_wait_request_handler(ngx_event_t *rev)
{
	if (rev->timedout) {
        ngx_log_error(NGX_LOG_INFO, c->log, NGX_ETIMEDOUT, "client timed out");
        ngx_http_close_connection(c);
        return;
    }
}
```

5. 投递读请求
```C
ngx_int_t ngx_handle_read_event(ngx_event_t *rev, ngx_uint_t flags)
{
	// 之前没投递过，投递读请求到事件中心
	if (!rev->active && !rev->ready) {
		if (ngx_add_event(rev, NGX_READ_EVENT, NGX_LEVEL_EVENT)
			== NGX_ERROR)
		{
			return NGX_ERROR;
		}

		return NGX_OK;
	}

	// 已经投递过，或在关闭状态，删除读请求
	if (rev->active && (rev->ready || (flags & NGX_CLOSE_EVENT))) {
		if (ngx_del_event(rev, NGX_READ_EVENT, NGX_LEVEL_EVENT | flags)
			== NGX_ERROR)
		{
			return NGX_ERROR;
		}

		return NGX_OK;
	}
}
```