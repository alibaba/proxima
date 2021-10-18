---
title: "常见错误"
linkTitle: "常见错误"
weight: 151
draft: false
---

## 1、ProximaBE 错误
### 1.1 基础错误


| 错误码 | 错误原因                 |
| ------ | ------------------------ |
| 1000   | 运行时错误               |
| 1001   | 逻辑错误                 |
| 1002   | 状态错误                 |
| 1003   | 载入配置文件错误         |
| 1004   | 配置文件错误             |
| 1005   | 参数无效                 |
| 1006   | 未初始化                 |
| 1007   | 打开文件错误             |
| 1008   | 读取数据错误             |
| 1009   | 写数据错误               |
| 1010   | 超出限制                 |
| 1011   | 序列化错误               |
| 1012   | 反序列化错误             |
| 1013   | 开启服务器错误           |
| 1014   | 尝试访问已经停止的服务器 |



### 1.2 格式错误

| 错误码 | 错误原因               |
| ------ | ---------------------- |
| 2000   | Collection Name 为空   |
| 2001   | Column Name 为空       |
| 2002   | Column 为空            |
| 2003   | Repository Table 为空  |
| 2004   | Repository Name 为空   |
| 2005   | User Name 为空         |
| 2006   | Password 为空          |
| 2007   | URI 无效               |
| 2008   | Collection Status 无效 |
| 2009   | Record 无效            |
| 2010   | Query 无效             |
| 2011   | Index Data Format 无效 |
| 2012   | Write Request 无效     |
| 2013   | Vector Format 无效     |
| 2014   | Repository Type 无效   |
| 2015   | Data Type 无效         |
| 2016   | Index Type 无效        |
| 2017   | Segment 无效           |
| 2018   | Revision 无效          |
| 2019   | Feature 无效           |
| 2020   | Schema 不匹配          |
| 2021   | Magic Number 不匹配    |
| 2022   | Index Column 不匹配    |
| 2023   | Dimension 不匹配       |
| 2024   | Data Type 不匹配       |

### 1.3 Meta 模块错误

| 错误码 | 错误原因                               |
| ------ | -------------------------------------- |
| 3000   | 尝试更新 Status Field （只读）         |
| 3001   | 尝试更新 Revision Field （只读）       |
| 3002   | 尝试更新 CollectionUID Field （只读）  |
| 3003   | 尝试更新 IndexType Field （只读）      |
| 3004   | 尝试更新 DataType Field （只读）       |
| 3005   | 尝试更新 Parameters Filed （只读）     |
| 3006   | 尝试更新 RepositoryType Field （只读） |
| 3007   | 尝试更新 ColumnName Field （只读）     |
| 3008   | 每个 Segment 包含 0 个 Docs            |
| 3009   | 不支持的链接类型                       |


### 1.4 Index 模块错误

| 错误码 | 错误原因                       |
| ------ | ------------------------------ |
| 4000   | Collection 重复                |
| 4001   | Key 重复                       |
| 4002   | Collection 不存在              |
| 4003   | Column 不存在                  |
| 4004   | Key 不存在                     |
| 4005   | Collection 处于 Suspended 状态 |
| 4006   | Segment 丢失                   |
| 4007   | Lsn Context 为空               |
| 4008   | 超过 Rate Limit                |


### 1.5 Query 模块错误

| 错误码 | 错误原因                           |
| ------ | ---------------------------------- |
| 5000   | Segment 不可访问                   |
| 5001   | Forward 列不匹配                   |
| 5002   | Results 超过限制                   |
| 5003   | Compute Queue 还没准备好           |
| 5004   | 任务调度出错                       |
| 5005   | Collection 不可访问                |
| 5006   | Task 正在另外一个 coroutine 中运行 |

## 2. Repository 错误

### 2.1 基础错误


| 错误码 | 错误原因         |
| ------ | ---------------- |
| 1000   | 运行时错误       |
| 1001   | 逻辑错误         |
| 1003   | 载入配置文件错误 |
| 1004   | 配置文件错误     |
| 1005   | 参数无效         |
| 1006   | 未初始化         |
| 1007   | 打开文件错误     |
| 1010   | 超出限制         |


### 2.2 格式错误

| 错误码 | 错误原因            |
| ------ | ------------------- |
| 2020   | Schema 不匹配       |
| 2021   | Magic Number 不匹配 |

### 2.3 和 Agent 模块交互错误

| 错误码 | 错误原因        |
| ------ | --------------- |
| 4000   | Collection 重复 |
| 4008   | 超过 Rate Limit |

### 2.4 MySQL repository 模块错误

| 错误码 | 错误原因                                    |
| ------ | ------------------------------------------- |
| 20000  | 链接 MySQL 失败                             |
| 20001  | MySQL Table 无效                            |
| 20002  | 执行 MySQL 命令错误                         |
| 20003  | Table 中没有新数据                          |
| 20004  | Row 数据无效                                |
| 20005  | 不支持的 MySQL 版本                         |
| 20006  | 执行 Command 失败                           |
| 20007  | Binlog dump 失败                            |
| 20008  | 没有新的 Binlog 数据                        |
| 20009  | MySQL 结果无效                              |
| 20010  | 不支持的 Binglog 格式                       |
| 20011  | 获取 MySQL 结果失败                         |
| 20012  | Binlog 处于 suspended 状态                  |
| 20013  | MySQL Handler 未初始化                      |
| 20014  | MySQL Handler 重复初始化                    |
| 20015  | collection config 无效                      |
| 20016  | Collection 不存在                           |
| 20017  | RPC 失败                                    |
| 20018  | Collection 应当终止                         |
| 20019  | URI 无效                                    |
| 20020  | 初始化 rpc channel 失败                     |
| 20021  | MySQL Handler 无效                          |
| 20022  | LSN context 无效                            |
| 20023  | 没有更多的 Row 数据                         |
| 20024  | Schema 发生了变化                           |
| 20025  | Server 和 repository 的 version 不匹配 |

