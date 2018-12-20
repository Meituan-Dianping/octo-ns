# 1 概述说明
Mns-Invoker是OCTO-NS的轻量级SDK，提供服务注册与注销，服务发现，基础工具类等功能。

# 2 服务注册和发现
## 2.1 服务注册
### 描述
用于注册thrift和http协议的服务节点 

```
public static void registServiceWithCmd(int uptCmd, SGService sgService) throws TException
```

### 请求参数

参数名  | 参数类型 | 描述
------------- | ------------- | ------------
uptCmd  | int  | 注册服务的命令码  
sgService | SGService  | 参见相关结构体一节

注册服务的命令码:

0，重置(代表后面的serviceName list就是该应用支持的全量接口);               
1，增加(代表后面的serviceName list是该应用新增的接口);                           
2，减少(代表后面的serviceName list是该应用删除的接口)。 

【NOTE】

服务注册和注销的接口是异步的，因此，注册完后，获取服务列表的接口调用需要一定的反应时间，在正常情况下是20秒内。

### 请求示例
```
String providerAppkey = "com.XXX.XXX.server";
SGService service = getDefaultSGService(providerAppkey, 5198, true);
        service.setProtocol("thrift");
        MnsInvoker.registServiceWithCmd(0,service);
private static SGService getDefaultSGService(final String appkey, final int port, final boolean isThrift) {
        String protocol = isThrift ? "thrift" : "http";
        SGService service = new SGService();
        service.setAppkey(appkey);
        service.setPort(port);
        service.setVersion("original");
        service.setIp(localIp);
        service.setLastUpdateTime((int) (System.currentTimeMillis() / 1000));
        service.setServerType(isThrift ? 0 : 1);
        service.setWeight(10);
        service.setFweight(10.d);
        return service;
}
```
## 2.2 服务注销
### 描述
用于注销thrift和http协议的服务节点

```
public static void unRegisterService(SGService sgService) throws TException
```

### 请求参数
参数名  | 参数类型 | 描述
------------- | ------------- | ------------
sgService | SGService  | 参见相关结构体一节

【NOTE】

注销接口不会从OCTO上删除节点。调用该接口会有以下变化：

* 如果服务器上的status不等于4(禁用)，则status的值会直接置成0，忽略调用者在sgService中设定的值。否则，status保持为4。
* 使用sgService的version，extend值覆盖服务器上的旧值
* lastUpdateTime改为当前系统的时间(sg_agent所在的机器)

### 请求示例

~~~
MnsInvoker.unRegisterService(getSGService());
// SGService的appkey，port，localIp，protocal，version等变量请自定义
private SGService getSGService() {
        SGService service = new SGService();
        service.setAppkey(appKey)
                .setPort(port).setIp(localIP)
                .setLastUpdateTime((int) (System.currentTimeMillis() / 1000))
                .setServerType(1)
                .setWeight(10).setFweight(10.d)
                .setProtocol(protocal);
        service.setStatus(fb_status.DEAD.getValue())//fb_status.DEAD值为0
                .setVersion(version);
        return service;
}
~~~
    
## 2.3 服务发现
### 描述
获取自定义协议的服务节点

~~~
public static List<SGService> getServiceList(ProtocolRequest req)
public static List<SGService> getOriginServiceList(ProtocolRequest req)
~~~

### 请求参数
参数名  | 参数类型 | 描述
------------- | ------------- | ------------
req | ProtocolRequest  | 参见相关结构体一节

### 返回值
参数类型 | 描述
------------- | ------------
List<SGService>  | 不会返回null，但会返回空列表

【NOTE】

服务节点的获取接口，在第一次获取时，会去远程获取并保存到内存中，此后的调用都是读缓存。而服务节点列表由专门的线程进行更新，读写分离，因此该接口不会堵塞。第一次从远程获取的timeout = 300毫秒，服务节点列表的更新周期在5秒内。

### 请求示例
~~~
final String consumerAppkey = "XXX";
final String remoteAppkey = "XXX";
ProtocolRequest httpReq = new ProtocolRequest()
                .setLocalAppkey(consumerAppkey)
                .setRemoteAppkey(remoteAppkey)
                .setProtocol("http");
ProtocolRequest thriftReq = new ProtocolRequest()
                .setLocalAppkey(consumerAppkey)
                .setRemoteAppkey(remoteAppkey)
                .setProtocol("thrift");
​
List<SGService> httpList = MnsInvoker.getServiceList(httpReq);
List<SGService> thriftList = MnsInvoker.getServiceList(thriftReq);
~~~
## 2.4 服务列表监听
### 描述
为自定义协议的请求添加或删除监听器，监听器的作用是当服务列表发生变化时产生回调。

~~~
public static int addServiceListener(ProtocolRequest req, IServiceListChangeListener listener)
public static int removeServiceListener(ProtocolRequest req, IServiceListChangeListener listener)
~~~
### 请求参数
参数名  | 参数类型 | 描述
------------- | ------------- | ------------
req | ProtocolRequest  | 参见相关结构体一节
listener | IServiceListChangeListener | 服务监听器
oldList | List<SGService> | 旧服务列表
newList | List<SGService> | 新服务列表
addList | List<SGService> | 新增服务节点
deletedList | List<SGService> | 被删除的服务节点
modifiedList | List<SGService> | 发生变化的服务节点
### 返回值
参数类型 | 描述
------------- | ------------
int  | 成功返回0，失败返回-1

【NOTE】

* SGService里面的appkey，ip，envir，port，protocol字段一般不会有改变，以下字段的变更会触发modifiedList：status,weight,fweight,version,role,serviceInfo
* mns-invoker对于所有的服务列表监听器只起了一个线程去触发回调callback，因此，强烈建议用户不要在监听器的chaned()方法中做有阻塞的操作，否则会阻塞所有的监听器。
* 可以对同一份服务列表添加多个监听器，但监听器必须是不同的（hashCode不同）。
* 强烈建议用户不要自行修改ProtocolRequest和IServiceListChangeListener的hashCode和equals方法。

~~~
ProtocolRequest thriftReq = new ProtocolRequest()
                .setLocalAppkey(localAppkey)
                .setRemoteAppkey(remoteAppkey)
                .setProtocol("thrift");
IServiceListChangeListener thriftListener = new MyListener();
MnsInvoker.addServiceListener(thriftReq, thriftListener);
//进行服务注册or注销逻辑
MnsInvoker.removeServiceListener(thriftReq, thriftListener);
private class MyListener implements IServiceListChangeListener {
        @Override
        public void changed(ProtocolRequest req, List<SGService> oldList, List<SGService> newList, List<SGService> addList, List<SGService> deletedList, List<SGService> modifiedList) {
            //以下为自定义的业务逻辑，本例只打印变量内容。
            System.out.println("req protocol: " + req.getProtocol());
            if (!addList.isEmpty()) {
                addCallback = true;
                print("addList:", addList);
            }
            if (!deletedList.isEmpty()) {
                deletedCallback = true;
                print("deletedList:", deletedList);
            }
​
        }
        private void print(String msg, List<SGService> list) {
            System.out.println(msg);
            for (SGService service : list) {
                System.out.println(service);
            }
        }
    }
~~~
 
# 4 工具类
## 4.1 根据ip查询idc内网信息
### 描述
根据ip列表获取对应的idc信息

~~~
public static Map<String, Idc> getIdcInfo(List<String> ips) throws MnsException
~~~

### 请求参数
参数名  | 参数类型 | 描述
------------- | ------------- | ------------
ips | List<String>  | ip列表

### 返回值
参数类型 | 描述
------------- | ------------
Map<String, Idc>  | key为传入ip，valude为Idc信息，未查询到的返回Idc(unknown, unknown, unknown)

## 4.2 全部idc内网信息获取
### 描述
查询获取对应的idc信息

~~~
public static List<Idc> getIdcs()
~~~

### 返回值
参数类型 | 描述
------------- | ------------
List<Idc>  | 所有Idc信息的列表

## 4.3 查询本机ipv4地址
### 描述
查询本机ipv4地址

~~~
public static String getLocalIpV4()
~~~

### 返回值
参数类型 | 描述
------------- | ------------
String  | 返回ipv4

## 4.4 查询本机环境
### 描述
查询本机octo.cfg配置文件里记录的环境信息

~~~
public static String getAppEnv()
~~~

### 返回值
参数类型 | 描述
------------- | ------------
String  | 环境信息，prod、stage、dev、ppe、test其中之一

## 4.5 查询本机是否为线上环境
### 描述
查询本机octo.cfg配置文件里记录的环境是否属于线上环境

~~~
public static boolean isLocalHostOnline()
~~~

### 返回值
参数类型 | 描述
------------- | ------------
boolean  | true为线上环境，即prod或stage，其他dev、ppe、test均为线下环境
## 4.6 查询Mns的zookeeper地址
### 描述
查询本机octo.cfg配置文件里面记录的服务注册中心mns的zookeeper地址

~~~
public static String getMnsZKUrl()
~~~

### 返回值
参数类型 | 描述
------------- | ------------
String  | ip:port连接的字符串，如配置文件不存在返回null。
## 4.7 查询Mnsc服务的域名
### 描述
查询本机octo.cfg配置文件记录的mnsc服务域名

~~~
public static String getMnsZKUrl()
~~~
### 返回值
参数类型 | 描述
------------- | ------------
String  | 以http://开头的域名，如配置文件不存在返回null。

## 4.8 查询idc文件的存储目录
### 描述
查询本机octo.cfg配置文件记录的idc文件位置

~~~
public static String getIdcPath()
~~~
### 返回值
参数类型 | 描述
------------- | ------------
String  | idc文件所在位置，如配置文件不存在返回null。

## 4.9 查询sg_agent代理的端口号
### 描述
查询本机octo.cfg配置文件记录的sg_agent端口号

~~~
public static int getSgagentPort()
~~~
### 返回值
参数类型 | 描述
------------- | ------------
int  | 端口号
## 4.10 查询sg_agent代理的appkey
### 描述
查询本机octo.cfg配置文件记录的sg_agent的appkey

~~~
public static String getSgagentAppkey() // sg_agent本地代理的appkey
public static String getSgsentinelAppkey() // sg_sentinel即哨兵的appkey
~~~

### 返回值
参数类型 | 描述
------------- | ------------
String  | appkey，如配置文件不存在返回null。

# 5 相关结构体
## 5.1 ProtocolRequest
~~~
struct ProtocolRequest {

1:string localAppkey;

2:string remoteAppkey;

3:string protocol;

4:string serviceName; 

}

/*

*因为是由thrift产生的类，如果该结构体的字段顺序发生变化，则对应的带参的构造方法也会随着变化，

*为了更好地兼容，因此，不建议使用带参的构造方法。

*建议构建方式如下

*/

ProtocolRequest req = new ProtocolRequest();

req.setLocalAppkey("本地appkey")

.setRemoteAppkey("服务端appkey")

.setProtocol("协议");
~~~
## 5.2 SGService
~~~
struct SGService

{

1:string appkey; //不能为空

2:string version;

3:string ip;  //不能为空

4:i32 port;  //不能为空

5:i32 weight;   //一般设置为10

6:i32 status;  //2正常状态，0未启动

7:i32 role;    //backup角色，当其他服务无法服务的时候，启用backup状态;0(主）1（备）

8:i32 envir;        //运行环境，线上：prod=3，stage=2， 线下：dev=3，ppe=2，test=1

9:i32 lastUpdateTime;  //最后修改时间

10:string extend;   //扩展

11:double fweight;   //浮点型权重 一般设置为10.0

12:i32 serverType;  //用于区分http(1)和thrift(0)

13:string protocol;    //协议

}
~~~