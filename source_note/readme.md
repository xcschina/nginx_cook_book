## 主要概念

#### 1. 多进程？多线程?

  多进程 + 多线程。nginx 进程组包含三种进程：master、worker、cache manager。
  
  * master 进行启动、退出、控制等工作。启动过程它负责启动其它进程；接收到控制命令后，它负责将命令发送到其它进程。
  
  * worker 负责处理处理连接请求，处理连接。由主线程负责处理各种事件(accept、套接字read、套接字write、定时器)，而阻塞性的工作会分配给工作线程池中的线程进行处理。
  
  * cache manager负责管理进程间共享的缓存。


## nginx源码导读

1. 主线程逻辑：master进程主要是等待控制命令，收到命令后分发给其它工作进程。worker进程通过事件系统处理套接字、定时器事件、控制命令。

详情见：[主循环.c]()。工作线程为一个for死循环，不断地调用事件处理接口监听事件的发生、处理事件。详见：[ngx_process_events_and_timers.c]()

2. socket 相关

* nginx 支持select、epoll、devpoll、kqueue、win32_select 等事件处理接口。

* 所有工作线程都能处理监听套接字的连接事件，它们之间通过锁来控制，同一时间只有一个进程处理连接事件。 详见：进程间争抢accept锁 worker进程抢accept锁.c

* 
