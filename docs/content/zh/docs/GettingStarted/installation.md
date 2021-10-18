---
title: "安装指南"
linkTitle: "安装指南"
weight: 6
draft: false
---


ProximaBE 具备极简依赖、快速部署、多平台支持等特点。其目前支持主流的RedHat系列Linux发行版本，包括RHEL、Centos、taobao7u等，CPU指令集方面支持x86(Intel)和aarch64(Arm)指令集。



## 1. Docker 镜像安装

### 1.1. 安装前提

我们推荐安装 1.12.6 以上版本 Docker。用户可以在安装镜像前确认自己的 Docker 版本以及状态

```s
$ sudo docker info
```



### 1.2. 拉取Docker镜像

```s
$ docker pull ghcr.io/proxima/proxima-be
```


### 1.3. 启动容器

##### 1.3.1. 快速启动容器
使用容器内置的配置文件来启动服务，便于快速上手。但是容器销毁时数据也会丢失，生产环境部署请用映射文件启动的模式。

```s
$ sudo docker run -d --name proxima_be -p 16000:16000 -p 16001:16001 \
      ghcr.io/proxima/proxima-be
```

> **_NOTE:_**  低版本 docker 需要在上述命令中增加`--net=host`选项


##### 1.3.2. 映射文件启动容器
通过将本机目录映射到容器，在容器删除时数据文件仍然可用。

- 创建相关目录，修改配置文件

```s
$ mkdir -p $HOME/proxima-be/{conf,data,log}
$ vim /$HOME/proxima-be/conf/proxima_be.conf # 镜像中有默认配置 /var/lib/proxima-be/conf/proxima_be.conf
```

- 执行下面的命令启动容器
```s
$ sudo docker run -d --name proxima_be -p 16000:16000 -p 16001:16001 \
      -v $HOME/proxima-be/conf:/var/lib/proxima-be/conf \
      -v $HOME/proxima-be/data:/var/lib/proxima-be/data \
      -v $HOME/proxima-be/log:/var/lib/proxima-be/log \
      ghcr.io/proxima/proxima-be/proxima/proxima-be
```

##### 1.3.3. 其他容器操作
停止服务

```s
$ docker stop proxima_be
```

> **_WARNING:_**  kill 容器或者强制删除容器有可能导致数据丢失，慎用下面的命令。
> ```s
> $ docker kill proxima_be
> $ docker rm -f proxima_be
> ```

