# 1 概述说明  
MnsSDK是OCTONS的C++侧轻量级SDK，提供服务注册与注销，服务发现，基础工具类等功能。
# 2 服务注册和发现 
## 2.1 初始化  
进行SDK的初始化。

```
／*
* param   mns_path 使用mns做服务注册发现时的配置路径
* param   sec      内部缓存更新时间间隔
* param   timeout  内部获取列表的超时时间 
* ret     返回值 成功返回0 不成功返回-1 
*／ 
int InitMNS(const std::string &mns_path, const double &sec,
            const double &timeout = 0.5);//初始化mns_sdk客户端，mns_path 环境配置文件,
```
## 2.2 服务注册
### 描述
用于注册Thrift 或 HTTP协议的服务节点。

```
／*
* param   str_appkey 要注册的app名称
* param   i16_port   要注册的端口号      
* param   i32_svr_type    服务类型 0:thrift, 1:http, 2:other
* param   str_proto_type  协议类型 默认时“thrift”
* ret     返回值 成功返回0 不成功返回-1 
*／ 
int8_t StartSvr(const std::string &str_appkey,
                const int16_t &i16_port,
                const int32_t &i32_svr_type,  
                const std::string &str_proto_type = "thrift");

／*
* param   str_appkey        要注册的app名称
* param   service_name_list 要注册的服务服务，用途为单端口多服务 
* param   i16_port          要注册的端口号      
* param   i32_svr_type      服务类型 0:thrift, 1:http, 2:other
* param   str_proto_type    协议类型 默认时“thrift”
* param   b_is_uniform      是否是octo统一协议默认是false
* ret     返回值 成功返回0 不成功返回-1 
*／                 
int8_t StartSvr(const std::string &str_appkey,
                const std::vector<std::string> &service_name_list,
                const int16_t &i16_port,
                const int32_t &i32_svr_type,  
                const std::string &str_proto_type,
                const bool &b_is_uniform = false);                
                
```

### 请求示例
注册一个AppKey名称为"com.test.appkey"，端口号是16888，server type是0（thrift），协议类型为thrift。

```
  const std::string appkey = "com.test.appkey"；
  const int16_t port = 16888;
  cosnt int32_t i32_svr_type = 0;  
  const std::string str_proto_type = "thrift";
  int ret = StartSvr(appkey, port, i32_svr_type, str_proto_type))) ;  
  if (ret != 0){
      printf("start failed");
  }else{
      printf("start success");
  }
```
   
## 2.3 服务发现
### 描述 
返回服务列表，存入用户传入的res_svr_list中。

```
／*
* param      str_svr_appkey    要获取的app名称
* param      str_cli_appkey    本地的app名称
* param      str_proto_type    要获取的协议类型      
* param      i32_svr_type      服务类型 0:thrift, 1:http, 2:other
* param      str_service_name  按照服务名进行过滤
* param  out p_svr_list        返回的服务列表
* ret     返回值 成功返回0 不成功返回-1 
*／     

int8_t getSvrList(const std::string &str_svr_appkey,
                  const std::string &str_cli_appkey,
                  const std::string &str_proto_type,
                  const std::string &str_service_name,
                  std::vector<meituan_mns::SGService> *p_svr_list);
```


## 2.4 服务列表监听
### 描述
为自定义协议的请求添加或删除监听器，监听器的作用是当服务列表发生变化时产生回调。   
获取所有类型服务节点，推荐，支持在一条业务线程中多次绑定不同的appkey。

~~~
／*
* param      str_svr_appkey    要获取的app名称
* param      str_cli_appkey    本地的app名称
* param      str_proto_type    要获取的协议类型      
* param      i32_svr_type      服务类型 0:thrift, 1:http, 2:other
* param      str_service_name  IDL文件中的service名字,可按这个名字来过滤服务节点,可填空串返回全部服务节点

* param      cb                回调函数，当有数据更新时通知的函数，全量更新
* ret                          返回值 成功返回0 不成功返回-1 
*／     
int8_t StartClient(const std::string &str_svr_appkey,
                   const std::string &str_cli_appkey,
                   const std::string &str_proto_type,
                   const std::string &str_service_name, 
                   const SvrListCallback &cb);

／*
* param      str_svr_appkey    要获取的app名称
* param      str_cli_appkey    本地的app名称
* param      str_proto_type    要获取的协议类型      
* param      i32_svr_type      服务类型 0:thrift, 1:http, 2:other
* param      str_service_name  IDL文件中的service名字,可按这个名字来过滤服务节点,可填空串返回全部服务节点

* param      cb                回调函数，当有数据更新时通知的函数，与上函数不同的是这个回调区分更新的类型
* ret                          返回值 成功返回0 不成功返回-1 
*／     
int8_t StartClient(const std::string &str_svr_appkey,
                   const std::string &str_cli_appkey,
                   const std::string &str_proto_type, 
                   const std::string &str_service_name, 
                   const UpdateSvrListCallback &cb); 
~~~

添加对appkey的回调通知，可以支持多个通知对象，实现观察者模式。

~~~
／*
* param      str_svr_appkey    要获取的app名称
* param      cb                接收回调函数，全量更新
* ret                          返回值 成功返回0 不成功返回-1 
*／   
int8_t AddSvrListCallback(const std::string &str_svr_appkey,
                          const SvrListCallback &cb,
                          std::string *p_err_info);

／*
* param      str_svr_appkey    要获取的app名称
* param      cb                接收回调函数，与上不同的是分类更新
* ret                          返回值 成功返回0 不成功返回-1 
*／   
int8_t AddUpdateSvrListCallback(const std::string &str_svr_appkey,
                                const UpdateSvrListCallback &cb,
                                std::string *p_err_info);
~~~
### 请求示例
注册一个AppKey名称为"com.test.appkey"，端口号是16888，server type是0（thrift），协议类型为thrift。

```
 void JobList(const vector<meituan_mns::SGService> &vec_add,
             const vector<meituan_mns::SGService> &vec_del,
             const vector<meituan_mns::SGService> &vec_chg,
             const string &appkey) {
  cout << "AddUpdateSvrListCallback appkey " << appkey << endl;
}


 boost::function<void(
      const vector<meituan_mns::SGService> &vec_add,
      const vector<meituan_mns::SGService> &vec_del,
      const vector<meituan_mns::SGService> &vec_chg,
      const string &appkey)> job1(boost::bind(&JobList, _1, _2, _3, _4));
  string err_info = "";
  if (MNS_UNLIKELY(AddUpdateSvrListCallback("com.sankuai.inf.newct",
                                            job1,
                                            &err_info))) {
    cout << "AddUpdateSvrListCallback failed: " << err_info << endl;
  } else {
    cout << "start client success" << endl;
  }
```


### NOTE

> - SGService里面的appkey，ip，envir，port，protocol字段一般不会有改变，以下字段的变更会触发modifiedList：status，weight，fweight，version，role，serviceInfo。
> - MnsSDK对于所有的服务列表监听器只起了一个线程去触发回调CallBack，因此，强烈建议用户不要在监听器的chaned()方法中做有阻塞的操作，否则会阻塞所有的监听器。
> - 可以对同一份服务列表添加多个监听器。
 


# 3 相关结构体
## 3.1 SGService
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

8:i32 envir;        //运行环境，prod（线上环境3），stag（线上测试环境2），test（测试环境1）

9:i32 lastUpdateTime;  //最后修改时间

10:string extend;   //扩展

11:double fweight;   //浮点型权重 一般设置为10.0d

12:i32 serverType;  //用于区分http(1)和thrift(0)

13:string protocol;    //支持tair, sql等其他协议的服务

}
~~~