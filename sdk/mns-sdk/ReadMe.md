# 构建

MNS-SDK 以静态链接方式编译

MNS-SDK 依赖下列组件:

* [muduo](https://github.com/chenshuo/muduo): 网络工具库
* [boost](https://github.com/boostorg/boost): 基础的C++工具库
* [zlib](https://github.com/madler/zlib): 压缩工具库
* [thrift](https://github.com/apache/thrift): rpc通讯框架
* [log4cplus](https://github.com/log4cplus/log4cplus): 日志库
* [rapidjson](https://github.com/Tencent/rapidjson): json解析库

# 依赖支持

| 库依赖 | 版本 | 备注 |
| ------ | ------ | ------ |
| muduo | 1.1.0 | 有http服务补丁，见 patch/0001-add-patch-for-http.patch |
| boost | 1.45 和 1.55 | 建议centos6 使用1.45,建议centos7 使用1.55  |
| zlib | 1.2.3 |  |
| thrift | 0.8.0 |  |
| log4cplus | 1.1.3 |  |
| rapidjson | 1.1.0 |  |

# 支持环境

* [CentOS6 和 CentOS7](https://www.centos.org/)

### 准备依赖


CentOS通常需要安装EPEL，否则许多软件包默认不可用： 
 
```shell
> sudo yum install epel-release
```

安装通用依赖库： 
 
```shell
> sudo yum install git gcc-c++ cmake
```

安装 [boost](https://github.com/boostorg/boost), [zlib](https://github.com/madler/zlib), [log4cplus](https://github.com/log4cplus/log4cplus),[rapidjson](https://github.com/Tencent/rapidjson):

```shell
> sudo yum install boost-devel zlib-devel log4cplus-devel rapidjson-devel
```

安装[thrift](https://github.com/apache/thrift):  

```shell
thrift一般很少有yum源，请自行参照官方使用文档安装
```

安装 [OCTO-NS]():  
[参照OCTO-NS文档安装环境](./../../docs/ns-quick-start.md)


### 使用 build.sh 编译 MNS-SDK 
```bash 
> git clone https://github.com/Meituan-Dianping/octo-ns.git
```

**使用 build.sh 初始化**  

```bash 
> cd octo-ns/sdk/mns-sdk
> build.sh init  
``` 

**使用 build.sh 编译mns-sdk**

```bash 
> cd octo-ns/sdk/mns-sdk
> build.sh only_lib
```  

**使用 build.sh Run example**  

```bash 
> cd octo-ns/sdk/mns-sdk
> build.sh with_example  
> cd build/bin/  
> ./mns_sdk_example
``` 

**验证结果**  
注册成功标志，客户端输出

```bash 
> =========registService success=========  
```

注册失败成功标志，客户端输出       

```bash 
> =========registService failed=========  
```   

 
发起获取列表成功标志，客户端输出   
    
```bash 
> =========start client success========= 
```  
  
发起获取列表失败标志，客户端输出 
  
```bash 
> =========start client failed========= 
```
  
添加列表监听成功标志，客户端输出   

```bash 
> =========AddUpdateSvrListCallback success=========
```  
  
添加列表监听失败标志，客户端输出  

```bash 
> =========AddUpdateSvrListCallback failed========= 
``` 

列表回调标志，客户端输出   

```bash 
> =========GetServerList success=========      
```


### [详细API](docs/mns_sdk_api.md)
 


