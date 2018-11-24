## nginx源码导读

### 主要概念

#### 1. 多进程？多线程?

  多进程 + 多线程。nginx 进程组包含三种进程：master、worker、cache manager。
  
  * master进行启动、退出、控制等工作。启动过程它负责启动其它进程；接收到控制命令后，它负责将命令发送到其它进程。
  
  * worker负责处理处理连接请求，处理连接。
  
  * cache manager负责管理进程间共享的缓存。
