##  Naming Service(NS)   
  
  快速进行Java环境配置、依赖管理以及ZooKeeper、项目下载和配置、NSC、NSInvoker、SGAgent四部分部署即可进行服务注册、服务发现功能，下面给出安装部署说明.
  
  
| 步骤| 描述 | 备注 |
| ------ | ------ | ------ |
| 1.Java环境配置 | 服务依赖的Java环境 | |
| 2.依赖管理 |  第三方库|  |
| 3.ZooKeeper部署 |  存储注册数据|  |
| 4.项目下载 | 从git下载项目工程|  |
| 5.NSInvoker打包 | Java SDK打包|  |
| 6.NSC部署 |  NS缓存服务部署|  |
| 7.配置更新 |  更新配置依赖项|  |
| 8.SGAgent部署 | 服务治理代理安装部署 |  |
  
### Java环境配置

64bit OS: Linux/Unix/Mac, Linux/Unix/Mac recommended.

64bit JDK 1.8+: [downloads](https://www.oracle.com/technetwork/java/javase/downloads/jdk8-downloads-2133151.html), [JAVA_HOME settings](https://docs.oracle.com/cd/E19182-01/820-7851/inst_cli_jdk_javahome_t/)

Maven 3.2.x+: [downloads](https://maven.apache.org/download.cgi), [settings](https://maven.apache.org/settings.html).

### 依赖管理

| 库依赖 | 版本 | 备注 |
| ------ | ------ | ------ |
| muduo | v1.1.0 | 有http服务补丁，见 patch/0001-add-patch-for-http.patch |
| ZooKeeper | v3.4.6 |  |



### ZooKeeper部署
---

> [见ZooKeeper安装及部署](https://zookeeper.apache.org/doc/r3.1.2/zookeeperStarted.html)  
> 记录部署的ZooKeeper单机IP或集群IP列表及端口.

### 项目下载
---
~~~
git clone ${https://github.com/Meituan-Dianping/octo-ns.git}  
~~~

### NSInvoker打包
---
~~~
cd mns-invoker   
mvn clean install -Dmaven.test.skip=true 
~~~

### NSC部署
---
依赖初始化  
[Dorado依赖构建](https://github.com/Meituan-Dianping/octo-rpc/blob/master/dorado/dorado-doc/manual-developer/Compile.md)            
 
~~~ 
//idl-common依赖构建
cd common/idl-mns/idl-common
mvn clean install -Dmaven.test.skip=true
  
//idl-mnsc依赖构建
cd common/idl-mns/idl-mnsc
mvn clean install -Dmaven.test.skip=true

//NSInovker依赖下构建(若已构建，跳过)
cd mns-invoker   
mvn clean install -Dmaven.test.skip=true 
~~~

部署运行 
     
~~~  
cd mns-cache  
sh run.sh  
lsof -i:[port] //查看服务端口是否已启用  
~~~

### 配置更新
---
~~~  
cd octo-ns/conf  
更新octo.cfg中mnszk_url=IP:PORT //已部署的ZooKeeper机器地址
更新octo.cfg中mns_url=http://IP:PORT //已部署的NSC服务机器地址，若有多台可配置成为域名)  
sh build_common_cfg.sh  //见conf目录下的README.md介绍
~~~

### SGAgent部署
---
#### 支持环境
  * CentOS(支持CentOS 6系统)
  
#### 依赖初始化
~~~
cd common/build  
sh build_idl.sh  
cd octo-ns/sg_agent  
sh build.sh init  
已自带常用第三方依赖库  //见thrid_party README.md详细介绍
~~~
#### 配置初始化

>日志级别配置见[Log4cplus使用指南](https://github.com/log4cplus/log4cplus)  
cd sg_agent  
修改sg\_agent\_mutable.xml的MnsHost对应地域ZooKeeper地址信息(区分测试和生产环境)

#### 编译
~~~
cd octo-ns/sg_agent  
sh build.sh with_bin  //可执行文件生成
~~~
#### 运行
~~~
cd octo-ns/sg_agent/tool  
sh build_resource.sh  //见配置资源文件介绍 
sh run_server.sh  
lsof -i:[port] //查看服务端口是否已启用  
~~~ 
查看log日志是否有sg_agent自注册日志，如下:

~~~
[2018-12-18 22:43:13:671.423] 
[../util/json_data_tools.cc:177] 
INFO  - out: {  
        "appkey":       "com.octo.mns.sg_agent",  
        "version":      "sg_agent-open",  
        "ip":   "10.4.231.87",  
        "port": 7776,  
        "weight":       10,  
        "status":       2,  
        "role": 0,  
        "env":  1,  
        "lastUpdateTime":       1545144193,  
        "fweight":      10,  
        "serverType":   0,  
        "warmup":       0,  
        "heartbeatSupport":     0,  
        "protocol":     "thrift",  
        "serviceInfo":  {  
}  
}
[2018-12-18 22:43:13:671.443] [registry_service.cc:482] INFO  - to regist when node exists, uptCmd : 1, appkey : com.octo.mns.sg_agent, env : 1, local env : 1, status : 2; fweight : 10; serverType : 0; protocol: thrift, ip : 10.4.231.87
[2018-12-18 22:43:13:674.349] [zk_client.cc:310] INFO  - zoo_set ret = 0, path = /octo/nameservice/test/com.sankuai.inf.newct/provider/10.4.231.87:7776
[2018-12-18 22:43:13:677.031] [zk_client.cc:310] INFO  - zoo_set ret = 0, path = /octo/nameservice/test/com.sankuai.inf.newct/provider
[2018-12-18 22:43:13:677.106] [registry_service.cc:105] INFO  - success registry, appkey:com.octo.mns.sg_agent; ip = 10.4.231.87
~~~



##### 使用C++ NSSdk发起服务注册/服务发现示例
---
见C++ [NSSdk示例说明](https://github.com/Meituan-Dianping/octo-ns/blob/master/mns-sdk/ReadMe.md)
 
##### 使用Java NSInvoker发起服务注册/服务发现示例
添加依赖

~~~
pom.xml添加mns-invoker的maven坐标，如下：
<dependency>
   <groupId>com.meituan.octo</groupId>
   <artifactId>mns-invoker</artifactId>
   <version>1.0.0</version>
</dependency>
pom.xml添加依赖，如下：
<dependency>
   <groupId>com.meituan.octo</groupId>
   <artifactId>mns-invoker</artifactId>
   <version>1.0.0</version>
</dependency>
~~~

> 构建thrift协议示例,观察SGAgent日志.


~~~
String appkey ="com.meituan.octo.dorado.server";
String protocol = "thrift";
int port = 5198;
String version = "original";
String ip = "127.0.0.1";
int serverType = 0;//thrift是0，http是1

SGService service = new SGService();
        service.setAppkey(appkey);
        service.setPort(port);
        service.setVersion(version);
        service.setIp(ip);
        service.setLastUpdateTime((int) (System.currentTimeMillis() / 1000));
        service.setServerType(serverType);
        service.setWeight(10);
        service.setFweight(10.0);
        service.setProtocol(protocol);

MnsInvoker.registServiceWithCmd(1,service);//增量模式注册
~~~

##### 使用NSInvoker发起服务发现流程
下面是发现上述服务的例子

~~~
String consumerAppkey = "com.meituan.octo.dorado.client";
String remoteAppkey = "com.meituan.octo.dorado.server";

ProtocolRequest thriftReq = new ProtocolRequest()
                .setLocalAppkey(consumerAppkey)
                .setRemoteAppkey(remoteAppkey)
                .setProtocol("thrift");
​
List<SGService> thriftList = MnsInvoker.getServiceList(thriftReq);
~~~
