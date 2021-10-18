**bazel.cmake** 是一个 bazel 风格的 CMake 模块框架，用于 C/C++/CUDA 等代码的编译和构建，支持 Ninja, GNU Make, Unix Make, Visual Studio 等构建平台。

## 如何使用

#### 1. 脚本引入 bazel.cmake

```cmake
# CMakeLists.txt
include(<bazel.cmake 脚本路径>)
```

#### 2. 编写构建代码

```cmake
# CMakeLists.txt
find_package(Threads REQUIRED)

cc_library(
    NAME hello STATIC SHARED STRICT
    SRCS *.cc
    LIBS ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS}
    INCS .
  )
```

#### 3. 生成平台构建脚本

```bash
$ cmake -G Ninja ..
$ ninja all
```

## 相关接口

### 1. C/C++ 构建接口

#### 1.1. 新增 C/C++ 构建子目录

```cmake
cc_directory(<source_dir> [binary_dir])
```

#### 1.2. 新增多个 C/C++ 构建子目录

```cmake
cc_directories(<source_dir1> [source_dir2 ...])
```

#### 1.3. 构建 C/C++ 静态或动态库（支持同时生成静态和动态库）

```cmake
cc_library(
    NAME <name>
    [STATIC] [SHARED] [STRICT] [ALWAYS_LINK] [EXCLUDE] [PACKED]
    SRCS <file1> [file2 ...]
    [INCS dir1 ...]
    [PUBINCS public_dir1 ...]
    [DEFS DEF1=1 ...]
    [LIBS lib1 ...]
    [CFLAGS flag1 ...]
    [CXXFLAGS flag1 ...]
    [LDFLAGS flag1 ...]
    [DEPS target1 ...]
    [PACKED_EXCLUDES pattern1 ...]
    [VERSION <version>]
  )
```

#### 1.4. 构建 C/C++ 可执行程序

```cmake
cc_binary(
    NAME <name>
    [STRICT] [PACKED]
    SRCS <file1> [file2 ...]
    [INCS dir1 ...]
    [DEFS DEF1=1 ...]
    [LIBS lib1 ...]
    [CFLAGS flag1 ...]
    [CXXFLAGS flag1 ...]
    [LDFLAGS flag1 ...]
    [DEPS target1 ...]
    [VERSION <version>]
  )
```

#### 1.5. 构建 C/C++ 可执行测试程序

```cmake
cc_test(
    NAME <name>
    [STRICT]
    SRCS <file1> [file2 ...]
    [INCS dir1 ...]
    [DEFS DEF1=1 ...]
    [LIBS lib1 ...]
    [CFLAGS flag1 ...]
    [CXXFLAGS flag1 ...]
    [LDFLAGS flag1 ...]
    [DEPS target1 ...]
    [ARGS args1 ...]
  )
```

#### 1.6. 添加测试用例到测试集合，通过目标 unittest.<suite_name> 可运行

```cmake
cc_test_suite(<suite_name> [test_name ...])
```

#### 1.7. 导入外部已构建的 C/C++ 静态或动态库

```cmake
cc_import(
    NAME <name>
    [STATIC | SHARED] [PACKED]
    PATH <file>
    [INCS dir1 ...]
    [PUBINCS public_dir1 ...]
    [DEPS target1 ...]
    [IMPLIB <file>]
    [PACKED_EXCLUDES pattern1 ...]
  )
```

#### 1.8. 导入外部 C/C++ 接口库（仅依赖头文件，不链接）

```cmake
cc_interface(
    NAME <name>
    [PACKED]
    [INCS dir1 ...]
    [PUBINCS public_dir1 ...]
    [DEPS target1 ...]
    [PACKED_EXCLUDES pattern1 ...]
  )
```

#### 1.9. 构建 C/C++ GoogleTest 单元测试程序

```cmake
cc_gtest(
    NAME <name>
    [STRICT]
    SRCS <file1> [file2 ...]
    [INCS dir1 ...]
    [DEFS DEF1=1 ...]
    [LIBS lib1 ...]
    [CFLAGS flag1 ...]
    [CXXFLAGS flag1 ...]
    [LDFLAGS flag1 ...]
    [DEPS target1 ...]
    [ARGS args1 ...]
    [VERSION <version>]
  )
```

#### 1.10. 构建 C/C++ GoogleMock 单元测试程序

```cmake
cc_gmock(
    NAME <name>
    [STRICT]
    SRCS <file1> [file2 ...]
    [INCS dir1 ...]
    [DEFS DEF1=1 ...]
    [LIBS lib1 ...]
    [CFLAGS flag1 ...]
    [CXXFLAGS flag1 ...]
    [LDFLAGS flag1 ...]
    [DEPS target1 ...]
    [ARGS args1 ...]
    [VERSION <version>]
  )
```

#### 1.11. 构建 C++ protobuf 静态或动态库（支持同时生成静态和动态库）

```cmake
cc_proto_library(
    NAME <name>
    [STATIC] [SHARED] [STRICT] [EXCLUDE] [PACKED]
    SRCS <file1.proto> [file2.proto ...]
    [PROTOROOT path]
    [CXXFLAGS flag1 ...]
    [LDFLAGS flag1 ...]
    [DEPS target1 ...]
    [VERSION <version>]
    [PROTOBUF_VERSION <Protobuf version>]
  )
```

### 2. CUDA 构建接口

#### 2.1. 新增 CUDA 构建子目录

```cmake
cuda_directory(<source_dir> [binary_dir])
```

#### 2.2. 新增多个 CUDA 构建子目录

```cmake
cuda_directories(<source_dir1> [source_dir2 ...])
```

#### 2.3. 构建 CUDA 静态或动态库（支持同时生成静态和动态库）

```cmake
cuda_library(
    NAME <name>
    [STATIC] [SHARED] [STRICT] [ALWAYS_LINK] [EXCLUDE] [PACKED]
    SRCS <file1> [file2 ...]
    [INCS dir1 ...]
    [PUBINCS public_dir1 ...]
    [DEFS DEF1=1 ...]
    [LIBS lib1 ...]
    [CFLAGS flag1 ...]
    [CXXFLAGS flag1 ...]
    [CUDAFLAGS flag1 ...]
    [LDFLAGS flag1 ...]
    [DEPS target1 ...]
    [PACKED_EXCLUDES pattern1 ...]
    [VERSION <version>]
  )
```

#### 2.4. 构建 CUDA 可执行程序

```cmake
cuda_binary(
    NAME <name>
    [STRICT] [PACKED]
    SRCS <file1> [file2 ...]
    [INCS dir1 ...]
    [DEFS DEF1=1 ...]
    [LIBS lib1 ...]
    [CFLAGS flag1 ...]
    [CXXFLAGS flag1 ...]
    [CUDAFLAGS flag1 ...]
    [LDFLAGS flag1 ...]
    [DEPS target1 ...]
    [VERSION <version>]
  )
```

#### 2.5. 构建 CUDA 可执行测试程序

```cmake
cuda_test(
    NAME <name>
    [STRICT]
    SRCS <file1> [file2 ...]
    [INCS dir1 ...]
    [DEFS DEF1=1 ...]
    [LIBS lib1 ...]
    [CFLAGS flag1 ...]
    [CXXFLAGS flag1 ...]
    [CUDAFLAGS flag1 ...]
    [LDFLAGS flag1 ...]
    [DEPS target1 ...]
    [ARGS args1 ...]
  )
```

#### 2.6. 添加测试用例到测试集合，通过目标 unittest.<suite_name> 可运行

```cmake
cuda_test_suite(<suite_name> [test_name ...])
```

#### 2.7. 导入外部已构建的 C/C++/CUDA 静态或动态库

```cmake
cuda_import(
    NAME <name>
    [STATIC | SHARED] [PACKED]
    PATH <file>
    [INCS dir1 ...]
    [PUBINCS public_dir1 ...]
    [DEPS target1 ...]
    [IMPLIB <file>]
    [PACKED_EXCLUDES pattern1 ...]
  )
```

#### 2.8. 导入外部 C/C++/CUDA 接口库（仅依赖头文件，不链接）

```cmake
cuda_interface(
    NAME <name>
    [PACKED]
    [INCS dir1 ...]
    [PUBINCS public_dir1 ...]
    [DEPS target1 ...]
    [PACKED_EXCLUDES pattern1 ...]
  )
```

#### 2.9. 构建 C/C++/CUDA GoogleTest 单元测试程序

```cmake
cuda_gtest(
    NAME <name>
    [STRICT]
    SRCS <file1> [file2 ...]
    [INCS dir1 ...]
    [DEFS DEF1=1 ...]
    [LIBS lib1 ...]
    [CFLAGS flag1 ...]
    [CXXFLAGS flag1 ...]
    [CUDAFLAGS flag1 ...]
    [LDFLAGS flag1 ...]
    [DEPS target1 ...]
    [ARGS args1 ...]
  )
```

#### 2.10. 构建 C/C++/CUDA GoogleMock 单元测试程序

```cmake
cuda_gmock(
    NAME <name>
    [STRICT]
    SRCS <file1> [file2 ...]
    [INCS dir1 ...]
    [DEFS DEF1=1 ...]
    [LIBS lib1 ...]
    [CFLAGS flag1 ...]
    [CXXFLAGS flag1 ...]
    [CUDAFLAGS flag1 ...]
    [LDFLAGS flag1 ...]
    [DEPS target1 ...]
    [ARGS args1 ...]
  )
```

### 3. 工具接口

#### 3.1. 下载 GIT 库，并添加到项目

```cmake
git_repository(
    NAME <name>
    URL <url>
    [TAG <tag>]
    [PATH <local path>]
  )
```

#### 3.2. 下载 HG 库，并添加到项目

```cmake
hg_repository(
    NAME <name>
    URL <url>
    [TAG <tag>]
    [PATH <local path>]
  )
```

#### 3.3. 下载 SVN 库，并添加到项目

```cmake
svn_repository(
    NAME <name>
    URL <url>
    [REV <rev>]
    [PATH <local path>]
  )
```

#### 3.4. 下载压缩包，并添加到项目

```cmake
http_archive(
    NAME <name>
    URL <url>
    [SHA256 <sha256 value> | SHA1 <sha1 value> | MD5 <md5 value>]
    [PATH <local path>]
  )
```

#### 3.5. 获取库的 GIT 版本信息

```cmake
git_version(
    <result variable>
    <repository path>
  )
```

#### 3.6. 获取库的 HG 版本信息

```cmake
hg_version(
    <result variable>
    <repository path>
  )
```

#### 3.7. 获取库的 SVN 版本信息

```cmake
svn_version(
    <result variable>
    <repository path>
  )
```
