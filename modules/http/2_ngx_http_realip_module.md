## realip 模块

1. 功能：获取客户端真实ip，port

### 配置选项

1. set_real_ip_from

* 语法： address | CIDR | unix:;

* 环境： http, server, location

* 功能：定义受信任的传递真实IP地址的代理服务器地址。如果设置了unix，所有的unix域地址都会被信任。受信地址也可以使用使用 hostname 。

* 示例：set_real_ip_from  192.168.1.0/24;

* 示例：set_real_ip_from  192.168.2.1;

* 示例：set_real_ip_from  2001:0db8::/32;

2. real_ip_header

* 语法：real_ip_header field | X-Real-IP | X-Forwarded-For | proxy_protocol;

* 默认值：real_ip_header X-Real-IP;

* 环境：http, server, location

* 功能：定义存放真实客户端地址的header域名称。

3. real_ip_recursive 

* 语法：real_ip_recursive on | off;

* 默认：real_ip_recursive off;

* 环境：http, server, location

* 功能：如果非递归，则客户端地址会被替换成最后的非set_real_ip_from设置的地址。如果递归会被设置成第一个。

4. 变量

* realip_remote_addr 客户端真实ip

* realip_remote_port 客户端真实port