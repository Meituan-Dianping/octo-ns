/*
 * Copyright 2018 Meituan Dianping. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

namespace java com.octo.naming.common.thrift.model
namespace cpp meituan_mns
enum UptCmd {
    RESET = 0,
    ADD = 1,
    DELETE = 2,
}
/**
 * Common status reporting mechanism across all services
 */
enum fb_status {
    DEAD = 0,//宕机，故障
    STARTING = 1,//启动中
    ALIVE = 2,//正常
    STOPPING = 3,//正在下线
    STOPPED = 4,//禁用
    WARNING = 5,//警告
}
enum HeartbeatSupportType {
    NoSupport = 0,
    P2POnly = 1,
    ScannerOnly = 2,
    BothSupport = 3,
}
struct ServiceDetail {
    1:bool unifiedProto;  //默认为false，true代表支持统一协议
}

struct SGService
{
    1:string appkey;
    2:string version;
    3:string ip;
    4:i32 port;
    5:i32 weight;
    6:i32 status;  //正常状态，重启状态，下线状态，故障状态,具体值参考fb_status
    7:i32 role;    //backup角色，当其他服务无法服务的时候，启用backup状态;0(主）1（备）
    8:i32 envir;        //运行环境，prod（线上环境3），stag（线上测试环境2），test（测试环境1）
    9:i32 lastUpdateTime;  //最后修改时间
    10:double fweight;   //浮点型权重
    11:i32 serverType;  //用于区分http(1)和thrift(0)
    12:string protocol;    //支持扩展协议
    13:map<string, ServiceDetail> serviceInfo; //serviceName 到 servcieDetail的映射
    14:byte heartbeatSupport; //0:不支持心跳， 1:仅支持端对端心跳   2:仅支持scanner心跳  3:两种心跳都支持
    15:i32 warmup; //节点预热时间，单位秒
}
/*
 * 服务节点, 主要储存服务->appkey映射
 */
struct ServiceNode
{
    1:string serviceName;
    2:set<string> appkeys; //serviceName对应的appkey
    3:i32 lastUpdateTime;  //最后修改时间
}
/*
 * 服务分组,consumer定义
 */
struct Consumer
{
    1:list<string> ips;
    2:list<string> appkeys;
}

/*
 * 服务分组定义
 */
struct CRouteData
{
    1:string id;
    2:string name;
    3:string appkey;
    4:i32 env;
    5:i32 category;
    6:i32 priority;
    7:i32 status;  //服务分组，0：禁用，1：启用
    8:Consumer consumer;
    9:list<string> provider;
    10:i32 updateTime;  //最后修改时间
    11:i32 createTime;  //最后修改时间
    12:string reserved;   //扩展 k1:v1|k2:v2...
}
/*
 * 路由分组中provider节点信息
 */
struct CProviderNode
{
    1:string appkey;
    2:i64 lastModifiedTime;
    3:i64 mtime;
    4:i64 cversion;
    5:i64 version;
}
/*
 * 路由分组中route根节点结构
 */
struct CRouteNode
{
    1:string appkey;
    2:i64 lastModifiedTime;
    3:i64 mtime;
    4:i64 cversion;
    5:i64 version;
}
struct ProtocolRequest {
    1:string localAppkey;
    2:string remoteAppkey;
    3:string protocol;
    4:string serviceName;
}
struct ProtocolResponse {
    1:i32 errcode;
    2:list<SGService> servicelist;
}
