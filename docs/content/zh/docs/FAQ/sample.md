# 文档示例

## 这是一个目录
```{contents}
:depth: 2
```

## 这是一个注解
```{note}
这是一个注解
```

```{admonition} 这是一个警告
:class: warning
这是一个警告
```

```{admonition} 这是一个tip
:class: tip
这是一个tip
```

## 这是一个下载示例
 {download}`我是文件名 <../data/proxima_se.conf>`


## 这是一个数学公式
$$G = 1 - 2^{-1/a}$$

## 这也是一个数学公式
```{math}
w_{t+1} = (1 + r_{t+1}) s(w_t) + y_{t+1}
```


## 这是一个可以缩放比例的图片
``` {image} images/Proxima.png
---
scale: 10
---
```

## 如何连接到指定位置
定义自己的tag，类似于自定义锚点，全局可用

(my_tag)=
我是my_tag对应的内容



{ref}`对应到my_tag <my_tag>`

[这是一个目录](./sample.md#这是一个目录):

[常见错误](./common_faq.md#常见错误):

[这是一个可以缩放比例的图片](./sample.md#这是一个可以缩放比例的图片):

[特征向量](../MainConcepts/vector.md#特征向量):

[集合管理](../SdkReference/go.md#集合管理):

[1. 创建客户端](../SdkReference/go.md#1-创建客户端):

[2. 获取集合配置信息](../SdkReference/go.md#2-获取集合配置信息):


现在可以自动生成标题锚点，锚点命名规则为，删除标点符号，空格转为'-'，英文变为全小写，中文不变，下划线不变。

