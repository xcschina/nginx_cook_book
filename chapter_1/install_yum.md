### 通过yum安装nginx(适合centos、fedora、redhat等版本)

1. 安装 epel 软件仓库。epel 全称是 Extra Packages for Enterprise Linux，是企业版 Linux 附加软件包
```
$ sudo yum install epel-release
```

2. 更新软件包
```
$ sudo yum update
```

3. 安装 nginx
```
$ sudo yum update
```

4. 检查 nginx 是否安装成功。nginx -v 会显示 nginx 的版本信息，如果有显示代表 nignx 安装成功。
```
$ sudo nginx -v
nginx version: nginx/1.6.3
```
