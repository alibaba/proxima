---
title: "监控报警"
linkTitle: "监控报警"
weight: 13
draft: false
---


ProximaBE 基于[brpc](https://github.com/apache/incubator-brpc)的[bvar](https://github.com/apache/incubator-brpc/blob/master/docs/cn/bvar.md)功能，实现了兼容[Prometheus](https://prometheus.io/)的监控功能。

基本流程如下
1. ProximaBE 配置使用 bvar 做监控。
2. 配置 Prometheus，从 ProximaBE 订阅。
3. 配置[Grafana](https://grafana.com/)，便于查询监控。

## 1. 配置bvar

修改 proxima_be.conf，设置`common_config.metrics_config.name`为`bvar`。

```
common_config {
    # ...
    metrics_config {
	    name: "bvar"
	}
    # ...
}
```

## 2. 配置Prometheus
在 prometheus.yml 中的 scrape_configs 中，加入如下配置
```yaml
  - job_name: 'proxima-be'


    # metrics_path defaults to '/metrics'
    metrics_path: '/brpc_metrics'
    # scheme defaults to 'http'.

    static_configs:
    - targets: ['localhost:16000']
```

完整的 prometheus.yml 可以从[这里](/data/prometheus.yml)下载。

然后启动 prometheus（[下载地址](https://prometheus.io/download/)）

```s
$ ./prometheus --config.file=prometheus.yml
```

## 3. 配置Grafana

1. 启动并配置 Grafana
    ```
    docker run -i -p 3000:3000 grafana/grafana
    ```
2. 在浏览器中打开Grafana（地址如 http://localhost:3000/），并登录Grafana用户交互页面。
3. 添加Prometheus数据源，参考[官方文档](https://grafana.com/docs/grafana/latest/datasources/add-a-data-source/)
4. 下载[Grafana配置文件](/data/grafana.json)，参考[官方文档](https://grafana.com/docs/grafana/latest/dashboards/export-import/)导入Grafana。

![](/images/metrics.jpeg)
