---
title: "v0.1.2"
linkTitle: "v0.1.2"
weight: 251
draft: false
---



| Test Environment |                                                              |
| ---------------- | ------------------------------------------------------------ |
| Machine          | type=F51<br/>host=33.11.240.75                               |
| CPU              | cores=42<br/>vcores=84                                       |
| Memory           | mem=456G                                                     |
| Service Params   | query_thread=64 <br/>ef_search=300 <br/>ef_construction=500<br/>topk=10 |



| VTS-face Dataset(1 seg) |                                                              |
| ----------------------- | ------------------------------------------------------------ |
| Data description        | scale=1890w<br/>type=FP32<br/>dimension=512<br/>segment=**No Limit** |
| Insert Perf(@64)        | Average qps    : 1965/s<br/>Average latency: 36054us<br/>Maximum latency: 105544us |
| Query Perf(@16)         | Average qps    : 1305/s<br/>Average latency: 12224us<br/>Maximum latency: 25289us |
| Query Perf(@32)         | Average qps    : 2563/s<br/>Average latency: 12556us<br/>Maximum latency: 30060us |
| Query Perf(@48)         | Average qps    : 2794/s<br/>Average latency: 17167us<br/>Maximum latency: 40343us |
| Query Perf(@64)         | Average qps    : 2840/s<br/>Average latency: 22884us<br/>Maximum latency: 97761us |



| VTS-face Dataset(2 seg) |                                                              |
| ----------------------- | ------------------------------------------------------------ |
| Data description        | scale=1890w<br/>type=FP32<br/>dimension=512<br/>segment=**1000w** |
| Insert Perf(@64)        | Average qps    : 2085/s<br/>Average latency: 33463us<br/>Maximum latency: 101366us |
| Query Perf(@16)         | Average qps    : 1081/s<br/>Average latency: 15104us<br/>Maximum latency: 35672us |
| Query Perf(@32)         | Average qps    : 1148/s<br/>Average latency: 28201us<br/>Maximum latency: 105417us |
| Query Perf(@48)         | Average qps    : 1162/s<br/>Average latency: 41144us<br/>Maximum latency: 146889us |
| Query Perf(@64)         | Average qps    : 1200/s<br/>Average latency: 53940us<br/>Maximum latency: 97658us |



| Pailitao Dataset(1 seg) |                                                              |
| ----------------------- | ------------------------------------------------------------ |
| Data description        | scale=1999w<br/>type=FP32<br/>dimension=512<br/>segment=**No Limit** |
| Insert Perf(@64)        | Average qps    : 1868/s<br/>Average latency: 34348us<br/>Maximum latency: 110219us |
| Query Perf(@16)         | Average qps    : 1326/s<br/>Average latency: 12059us<br/>Maximum latency: 36241us |
| Query Perf(@32)         | Average qps    : 2594/s<br/>Average latency: 12274us<br/>Maximum latency: 34152us |
| Query Perf(@48)         | Average qps    : 2897/s<br/>Average latency: 16517us<br/>Maximum latency: 40627us |
| Query Perf(@64)         | Average qps    : 2967/s<br/>Average latency: 21686us<br/>Maximum latency: 89368us |



| Pailitao Dataset(2 seg) |                                                              |
| ----------------------- | ------------------------------------------------------------ |
| Data description        | scale=1999w<br/>type=FP32<br/>dimension=512<br/>segment=**1000w** |
| Insert Perf(@64)        | Average qps    : 1934/s<br/>Average latency: 33118us<br />Maximum latency: 97145us |
| Query Perf(@16)         | Average qps    : 1131/s<br/>Average latency: 14103us<br/>Maximum latency: 31110us |
| Query Perf(@32)         | Average qps    : 1106/s<br/>Average latency: 27734us<br/>Maximum latency: 114401us |
| Query Perf(@48)         | Average qps    : 1178/s<br/>Average latency: 40740us<br/>Maximum latency: 91339us |
| Query Perf(@64)         | Average qps    : 1183/s<br/>Average latency: 54144us<br/>Maximum latency: 94088us |



| Pailitao Bianry Dataset(1 seg) |                                                              |
| ------------------------------ | ------------------------------------------------------------ |
| Data description               | scale=9999w<br/>type=binary<br/>dimension=512<br/>segment=**No Limit** |
| Insert Perf(@64)               | Average qps    : 6453/s<br/>Average latency: 9806us<br/>Maximum latency: 116754us |
| Query Perf(@16)                | Average qps    : 2933/s<br/>Average latency: 5724us<br/>Maximum latency: 24253us |
| Query Perf(@32)                | Average qps    : 6795/s<br/>Average latency: 4789us<br/>Maximum latency: 25856us |
| Query Perf(@48)                | Average qps    : 10081/s<br/>Average latency: 4923us<br/>Maximum latency: 85046us |
| Query Perf(@64)                | Average qps    : 9956/s<br/>Average latency: 5690us<br/>Maximum latency: 57644us |



| Pailitao Bianry Dataset(10 seg) |                                                              |
| ------------------------------- | ------------------------------------------------------------ |
| Data description                | scale=9999w<br/>type=binary<br/>dimension=512<br/>segment=**1000w** |
| Insert Perf(@64)                | Average qps    : 9484/s<br/>Average latency: 6803us<br/>Maximum latency: 34860us |
| Query Perf(@16)                 | Average qps    : 1595/s<br/>Average latency: 10163us<br/>Maximum latency: 30655us |
| Query Perf(@32)                 | Average qps    : 1642/s<br/>Average latency: 19791us<br/>Maximum latency: 39849us |
| Query Perf(@48)                 | Average qps    : 1630/s<br/>Average latency: 29299us<br/>Maximum latency: 41211us |
| Query Perf(@64)                 | Average qps    : 1624/s<br/>Average latency: 39527us<br/>Maximum latency: 64230us |

