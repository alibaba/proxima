## 1 文档说明

该文档是 Proxima Bilin Engine 的项目文档，使用 [Hugo](https://gohugo.io/) 进行构建（使用 Docsy 主题）。

## 2 文档编写方法

### 2.1. 安装 Hugo

[下载地址](https://github.com/gohugoio/hugo/releases)

> **注意：**一定要安装 Extended 版本的 Hugo

本文仅仅介绍 Mac 版本的安装方法，其余版本详见[官方文档](https://gohugo.io/getting-started/installing)

下载最新的文件：

```s
$ wget https://github.com/gohugoio/hugo/releases/download/v0.87.0/hugo_extended_0.87.0_macOS-64bit.tar.gz
```

解压后，把可执行文件 hugo 放到 PATH 路径：

```s
$ sudo cp hugo /usr/local/bin/hugo
```

验证安装：
```s
$ hugo version
```

### 2.2 下载 submodule 

需要下载 submodule ：

```s
git submodule update --init --recursive
```

### 2.3 安装 SCSS

在`项目目录`中：
```s
sudo npm install -D --save autoprefixer
sudo npm install -D --save postcss-cli
```

> **注意：**需要预先安装 nodejs 才能使用 npm 命令。

### 2.4 编写 Markdown 文档

在 content/zh 对应的目录中建立 Markdown 文件，并编写文档。

> **注意**: 每个 Markdown 文件需要编写头部信息，例如 `title`，`weight` 等，具体可以参考文中的示例。

### 2.5 预览
```s
$ hugo server
```

编译后可以在 `http://localhost:1313/` 中预览


### 2.6 编译
预览没有问题则可以在编译：
```s
$ hugo
```

最后上传到 Github 即可





 