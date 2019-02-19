##  配置项简介  
  
#### conf.json
 
   * SGAgent使用Cthrift作为rpc server，该配置型定义了使用Cthrift Server时的配置.
   * 参数介绍

   参数名  | 参数类型 | 描述
------------- | ------------- | ------------
MnsHost  | string  |  忽略
server.Version | string  | 忽略
server.ListenPort | int  |  监听端口
server.Appkey | int  | 忽略
server.register | int  |  忽略
server.WorkThreadNum | int  | 工作线程数 
server.MaxConnNum | int  |   最大连接数
server.ServerTimeOut | int  | 超时
server.ConnGCTime | int  | 
server.ConnThreadNum | int  | 连接线程数

   
####  sg\_agent\_mutable.xml
 
   * 配置代理通用参数
   * 参数介绍

   参数名  | 参数类型 | 描述
------------- | ------------- | ------------
AgentAppKey | string  | 服务实例Appkey
AgentVersion | string  |  版本信息
ClientPort | int  | 监听端口
AgentLogPath | string  |  日志目录
MNSCacheAppkey | string  |  Mns-Cache Appkey
MnsHost | string  |  标识ZooKeeper的地址信息

####  sg\_agent\_whitelist.xml
 
   * 配置服务注册、服务发现过程中的白名单信息.
      

####  idc.xml
 
   * 配置用于服务分组的参数
   
    该配置项定义服务部署归属的机房、idc及地域信息,用于做路由计算及权重调整使用.

