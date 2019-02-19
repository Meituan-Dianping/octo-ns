##  ZooKeeper数据初始化   
  
  由于NSC服务启用依赖ZooKeeper数据的path路径，快速构建可使用ZooKeeper自带的zkCli.sh脚本进入ZooKeeper操作控制台操作,也可使用第三方脚本自行创建如Python，其自带了ZkClient包，便于操作.    
  以下给出第一种简单操作方式.
  
### 使用zkCli.sh创建数据
---
切换至ZooKeeper安装的bin文件目录

~~~
cd [dirName] //dirName为zookeeper的bin目录
~~~

创建节点信息test、stage、prod环境path

~~~
cd [dirName] //dirName为zookeeper的bin目录
./zkCli.sh  -timeout 1000  -r -server  127.0.0.1:2181
create /octo  "top octo path"  //永久节点方式
create /octo/nameservice  "nameservice dir"
create /octo/nameservice/test  "test dir"
create /octo/nameservice/stage "stage dir"
create /octo/nameservice/prod "prod dir"
~~~

创建ServiceName环境test、stage、prod路径

~~~
cd [dirName] //dirName为zookeeper的bin目录
./zkCli.sh  -timeout 1000  -r -server  127.0.0.1:2181
create /octo   "top octo path"  //永久节点方式
create /octo/service  "nameservice dir"
create /octo/service/test  "test dir"
create /octo/service/stage  "stage dir"
create /octo/service/prod   "prod dir"
~~~

### 数据导入
将项目中自带的快照数据导入到安装的ZooKeeper数据路径     
  
~~~
scp octo-ns/conf/snapshot  [dirName]
cd [zkBinDir]
./zkServer.sh restart

~~~
