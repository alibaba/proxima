# Proxima SE Benchmark Toolkits
Stand along toolkits for Benchmark test of ProximaSE 
## 1. Requirements
* Python3 (Above 3.6 with setuptools and pip installed)
    * [VirtualEnv](https://virtualenv.pypa.io/en/latest/) Optional, recommend using virtualenv to isolate python environment of ProximaSE

## 2. Environments
* Machine and OS refers to ProximaSE
* MYSQL, The payload of benchmark tests

## 3. Install
Do not need to install, just run it under the directory where toolkits located

## 4. Run

### 4.1 ProximaSE Build Benchmark Tools
#### 4.1.1 Procedure
```text
Step 1: Prepared Repository for ProximaSE
        Requirements: 
          i: table should have first column named by id with property auto_increment
         ii: vector column should be prefixed by vector
        iii: all columns treated as forward attributes except vector and id    
Step 2: Clone source code of ProximaSE
        cd (source of ProximaSE)
Step 3: Build ProximaSE
        mkdir build; cd build; cmake ../; make -j  
Step 4: Run Bench tools
        cd benchmark; pip install -i https://pypi.antfin-inc.com/simple/ -r requirements.txt; pip install PyMySQL
        PYTHONPATH=$(pwd) python scripts/build_bench.py 
```

#### 4.1.2 Help
```shell
$python scripts/build_bench.py 
Usage: build_bench.py [options]

Options:
  -h, --help            show this help message and exit
  --build_root=BUILD_ROOT
                        The build directory of ProximaSE, default value: [ENV
                        variable PROXIMA_SE_BUILD_ROOT or '$(pwd)/../build']
  --repo=JDBC           The source of repository, represented by jdbc string
  -t TABLE, --table=TABLE
                        Target table sync to ProximaSE
  --counts=COUNTS       The number of records will be sync to ProximaSE
  --log_dir=LOG_DIR     Log directory, default is logs
  --grpc_port=GRPC_PORT
                        Proxima SE GRPC service port, default 16000
  --http_port=HTTP_PORT
                        Proxima SE GRPC service port, default 16100
  --index_build_threads=INDEX_BUILD_THREADS
                        Index Agent build threads count, default is 10
  --index_build_qps=INDEX_BUILD_QPS
                        Threshold QPS of incremental records, default 1000000
  --index_directory=INDEX_DIRECTORY
                        Index directory, where indices located, default is
                        'indices'
  --max_docs_per_segment=MAX_DOCS_PER_SEGMENT
                        Max records per segment, default 1000000
  --meta_uri=META_URI   URI of meta store, meta/meta.sqlite
  -o OUTPUT, --output_dir=OUTPUT
                        Output directory, default random directory
  --cleanup             Cleanup all the outputs after finished
  --timeout=TIMEOUT     Timeout in seconds, default is 86400
  --interval=INTERVAL   Progress flush interval, default is 5 seconds
  --report=REPORT       Report file, default write to [output]/report.json
  --summary_progress=SUMMARY
                        Extract interested (approximate Progress, separated by
                        ',') progress records from reports
  --summary_interval=SUMMARY_INTERVAL
                        Extract interested progress records from reports
```

### 4.1.3 Example
Test benchmark with the dataset of face512d
```shell
python scripts/build_bench.py --repo=mysql://root:123456@127.0.0.1:3306/vts_face_fp32_512d \
    -t vts_face_fp32_512d_2w \
    --output vts_face_fp32_512d_2w
```
The report was located in the current directory named by vts_face_fp32_512d_2w, layout as following:
```text
$tree vts_face_fp32_512d_2w/
vts_face_fp32_512d_2w/
├── conf
│   ├── mysql_repo.conf
│   └── proxima_se.conf
├── indices
│   └── vts_face_fp32_512d_2w
│       ├── data.del
│       ├── data.fwd.0
│       ├── data.id
│       ├── data.manifest
│       └── data.pxa.vector.0
├── logs
│   ├── mysql.repo.log
│   ├── proxima_se_stderr.log
│   ├── proxima_se_stdout.log
│   ├── repo
│   │   ├── proxima_se.log.a99a05626.na61.xiaoxin.gxx.log.ERROR.20210113-135327.106363
│   │   ├── proxima_se.log.a99a05626.na61.xiaoxin.gxx.log.INFO.20210113-135311.106363
│   │   ├── proxima_se.log.a99a05626.na61.xiaoxin.gxx.log.WARNING.20210113-135327.106363
│   │   ├── proxima_se.log.ERROR -> proxima_se.log.a99a05626.na61.xiaoxin.gxx.log.ERROR.20210113-135327.106363
│   │   ├── proxima_se.log.INFO -> proxima_se.log.a99a05626.na61.xiaoxin.gxx.log.INFO.20210113-135311.106363
│   │   └── proxima_se.log.WARNING -> proxima_se.log.a99a05626.na61.xiaoxin.gxx.log.WARNING.20210113-135327.106363
│   └── be
│       ├── proxima_se.log.a99a05626.na61.xiaoxin.gxx.log.ERROR.20210113-135304.106289
│       ├── proxima_se.log.a99a05626.na61.xiaoxin.gxx.log.INFO.20210113-135300.106289
│       ├── proxima_se.log.a99a05626.na61.xiaoxin.gxx.log.WARNING.20210113-135304.106289
│       ├── proxima_se.log.ERROR -> proxima_se.log.a99a05626.na61.xiaoxin.gxx.log.ERROR.20210113-135304.106289
│       ├── proxima_se.log.INFO -> proxima_se.log.a99a05626.na61.xiaoxin.gxx.log.INFO.20210113-135300.106289
│       └── proxima_se.log.WARNING -> proxima_se.log.a99a05626.na61.xiaoxin.gxx.log.WARNING.20210113-135304.106289
├── meta
│   └── meta.sqlite
└── report.json

7 directories, 24 files
```

### 4.2 ProximaSE Query Benchmark Tools
#### 4.2.1 Procedure
```text
Step 1: Prepared Repository for ProximaSE
        Requirements: 
          i: ProximaSE should run outside of tools
         ii: Queries which should stored in mysql, with two columns named by id and vector, vector is string format, separated by ","     
Step 2: Clone source code of ProximaSE
        cd (source of ProximaSE)
Step 3: Build ProximaSE
        mkdir build; cd build; cmake ../; make -j  
Step 4: Run Bench tools
        cd benchmark; pip install -i https://pypi.antfin-inc.com/simple/ -r requirements.txt; pip install PyMySQL
        PYTHONPATH=$(pwd) python scripts/query_bench.py -h  
```

#### 4.2.2 Help
```shell
python scripts/query_bench.py -h 
Usage: query_bench.py [options]

Options:
  -h, --help            show this help message and exit
  --query=QUERY         The source of query, which should be DB connection
  --table=TABLE         Table name
  --collection=COLLECTION
                        Proxima SE collection
  --host=HOST           Proxima SE grpc service
  --topk=TOPK           Proxima SE topk, separated by ','
  --threads=THREADS     Number of threads to run test
  --http                Using http proto to test, default is grpc
  --timeout=TIMEOUT     Total seconds for test
  --interval=INTERVAL   Seconds for summary duration

```

### 4.2.3 Example
Test benchmark with the dataset of address512d
```shell
# Test Http protocol
python scripts/query_bench.py \
    --query=mysql://root:123456@11.139.184.242:3306/vts_face_fp32_512d \
    --table=vts_face_fp32_512d_q --collection=vts_face_fp32_512d \
    --host 33.11.240.73:16000 --topk=100 --threads=50 --http
    
# Test GRPC protocol  
python scripts/query_bench.py \
    --query=mysql://root:123456@11.139.184.242:3306/vts_face_fp32_512d \
    --table=vts_face_fp32_512d_q --collection=vts_face_fp32_512d \
    --host 33.11.240.73:16000 --topk=100 --threads=50 
```
The report was output on terminal, after finished test, output as following:
```text
....
python scripts/query_bench.py --query=mysql://root:123456@11.139.184.242:3306/vts_face_fp32_512d --table=vts_face_fp32_512d_q --collection=vts_face_fp32_512d --host 33.11.240.73:16000 --topk=100 --threads=50 
INFO 2021-03-08 17:44:38,747 query_bench.py:225:<module> 	Arguments: {'query': 'mysql://root:123456@11.139.184.242:3306/vts_face_fp32_512d', 'table': 'vts_face_fp32_512d_q', 'collection': 'vts_face_fp32_512d', 'host': '33.11.240.73:16000', 'topk': '100', 'threads': '50', 'http': False, 'timeout': 60, 'interval': 2}
INFO 2021-03-08 17:44:38,747 query_bench.py:78:init 	Init BenchContext.
   Total   Succeed   Failed     QPS  Latency(AVG)  Latency(Min)  Latency(Max)
      87        87        0      71            87            13           164
     908       908        0     453           109            12           251
     963       963        0     479           103            13           212
     979       979        0     488           101            15           248
     971       971        0     460           108            13           427
     948       948        0     467           106            13           254
     913       913        0     455           109            14           242
     883       883        0     437           114            12           267
     933       933        0     460           108            14           249
     946       946        0     470           105            16           234
     940       940        0     468           106            12           236
     947       947        0     470           105            18           254
     965       965        0     480           103            15           233
     933       933        0     464           106            11           244
     992       992        0     481           103            12           254
     968       968        0     480           103            16           259
     942       942        0     467           106            16           249
```

### 4.2.4 Cluster mode of bench test
If you’re planning to run large-scale load tests you need run query_bench distributed mode.
#### 4.2.4.1 Example:
```Shell
for i in $(seq 5); do
    python scripts/query_bench.py \
        --query=mysql://root:123456@11.139.184.242:3306/vts_face_fp32_512d \
        --table=vts_face_fp32_512d_q --collection=vts_face_fp32_512d \
        --host 33.11.240.73:16000 --topk=100 --threads=50 &
done 
```

### 4.3 ProximaSE Recall Tools
#### 4.3.1 Procedure
```text
Step 1: Start ProximaSE outside
Step 2: Prepare Queries of Recall test 
Step 3: Clone source code of ProximaSE
        cd (source of ProximaSE)
Step 4: Build ProximaSE
        mkdir build; cd build; cmake ../; make -j  
Step 5: Run Bench tools
        cd benchmark; pip install -i https://pypi.antfin-inc.com/simple/ -r requirements.txt; pip install PyMySQL
        PYTHONPATH=$(pwd) python scripts/recall.py 
```

#### 4.3.2 Help
```shell
$python scripts/recall.py -h 
Usage: recall.py [options]

Options:
  -h, --help            show this help message and exit
  --query=QUERY         The source of query, which should be DB connection
  --table=TABLE         Table name
  --collection=COLLECTION
                        Proxima SE collection
  --host=HOST           Proxima SE grpc service
  --gt=GT               Proxima SE GT file
  --topk=TOPK           Proxima SE topk, separated by ','
  --counts=COUNTS       Proxima SE query counts
  --dump_mismatch       Dump mismatched result
```

### 4.3.3 Example
Test benchmark with the dataset of face512d
```shell
python scripts/recall.py --query=mysql://root:123456@11.139.184.242:3306/vts_face_fp32_512d  \
    --table=vts_face_fp32_512d_q \
    --collection=vts_face_fp32_512d --topk=1,50,100,200 \
    --host 11.139.203.151:18000  --dump_mismatch \
    --counts=5
INFO 2021-01-28 09:33:59,276 recall.py:37:init 	Init BenchContext.
Processed    @1(%)   @50(%)  @100(%)  @200(%)
        1    100.0     98.0     98.0     98.0
       25    100.0    99.92    99.84    99.72
       50    100.0    99.92    99.86    99.76
       75    100.0    99.92    99.88    99.71
      100     99.0    99.94    99.89    99.66
      125     99.2    99.79    99.78    99.55
      150    99.33    99.79    99.77    99.55
      175    99.43    99.78    99.77    99.55
      200     99.5     99.8    99.76    99.56
      225    99.56    99.81    99.78    99.58
      250     99.6    99.81    99.78    99.58
      275    99.64    99.81    99.76    99.57
      300    99.67     99.8    99.75    99.57
      325    99.69     99.8    99.75    99.57
      350    99.71    99.81    99.74    99.57
      375    99.73    99.81    99.74    99.58
      400    99.75    99.81    99.74    99.57
      425    99.76    99.81    99.74    99.56
      450    99.78    99.82    99.74    99.56
      475    99.79    99.81    99.75    99.57
      500     99.8    99.82    99.76    99.58
--------------------Recall Tests------------------
Processed    @1(%)   @50(%)  @100(%)  @200(%)
      500     99.8    99.82    99.76    99.58
```
