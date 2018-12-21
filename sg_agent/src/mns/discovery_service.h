/*
 * Copyright (c) 2011-2018, Meituan Dianping. All Rights Reserved.
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#include <muduo/base/Mutex.h>
#include <muduo/base/Atomic.h>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <comm/buffer_mgr.h>
#include <boost/make_shared.hpp>
#include "discovery_zk_client.h"
#include "base_consts.h"
#include "naming_data_types.h"

#include "service_channels.h"
#include "buffer_mgr.h"
#include "base_mns_define.h"

namespace meituan_mns {

typedef boost::shared_ptr<getservicename_res_param_t> ServiceNamePtr;
typedef boost::shared_ptr<getservice_res_param_t> ServicePtr;
typedef boost::shared_ptr<getroute_res_param_t> RoutePtr;
typedef boost::function<void(muduo::net::EventLoop*)> ThreadInitCallback;

struct RouteService {
  getroute_res_param_t route_list;
  time_t last_update_time;
  int error_code;
  bool enable_exclusive_routes;
  RouteService() : last_update_time(0), error_code(0), enable_exclusive_routes(false) {}
};

class DiscoveryService {

 public:

  /**
   * 初始化缓存等初始化信息
   * @param
   *
   * @return
   *
   */
  int32_t Init(void);

  static DiscoveryService* GetInstance(void);

  void StartThread(void);

  ~DiscoveryService() {};

  /**
   * 为server侧提供获取服务列表接口
   * @param srvlist,req,channels
   *
   * @return int
   *
   */
  int32_t DiscGetSrvList(std::vector<SGService> &srvlist,
                         const ProtocolRequest &req,
                         boost::shared_ptr<ServiceChannels> &channels);

  /**
  * 从zookeeper拉取服务节点
  * @param srv_list,key,req
  *
  * @return int
  *
  */
  int32_t DiscNodesFromZk(std::vector<SGService> &srv_list,
                          const ProtocolRequest &req);

  /**
   *
   * @param service
   * @return
   */
  int32_t UpdateSrvList(const ServicePtr &service);

  /**
   *
   * @param list_and_cache
   * @param protocol
   * @param appkey
   * @return
   */
  int32_t GetSrvListAndCacheSize(ServListAndCache &list_and_cache,
                                 const std::string &protocol,
                                 const std::string &appkey);

  /**
   *
   * @param srv_list
   * @param req
   * @param channels
   * @return
   */
  int32_t DiscBySI(std::vector<SGService> &srv_list,
                   const ProtocolRequest &req,
                   boost::shared_ptr<ServiceChannels> &channels);

  int32_t RepalceSrvlist(const boost::shared_ptr<getservice_res_param_t> &service);


 private:
  /**
   * 从注册中心=zookeeper发现服务节点
   * @param srvlist,req,channels
   *
   * @return int
   *
   */
  int32_t DiscSrvlistFromZk(std::vector<SGService> &srv_list,
                            const ProtocolRequest &req,
                            boost::shared_ptr<ServiceChannels> &channels);
  /**
    * 从注册中心=cache发现服务节点,如redis,cache(mnsc)
    * @param srvlist,req,channels
    *
    * @return int
    *
    */
  int32_t DiscSrvlistFromCache(std::vector<SGService> &srv_list,
                               const ProtocolRequest &req,
                               boost::shared_ptr<ServiceChannels> &channels);
  /**
    * 存储两种以上进行服务节点发现,如zookeeper+cached
    * @param srvlist,req,channels
    *
    * @return int
    *
    */
  int32_t DiscSrvlistFromMixer(std::vector<SGService> &srv_list,
                               const ProtocolRequest &req,
                               boost::shared_ptr<ServiceChannels> &channels);
  /**
  * 通过SI: service instance 发现服务节点
  * @param srvlist,req,channels
  *
  * @return int
  *
  */
  int32_t DiscBySrvName(std::vector<SGService> &srv_list,
                        const ProtocolRequest &req,
                        boost::shared_ptr<ServiceChannels> &channels);
  /**
  * 通过SIN:service interface name发现服务节点
  * @param srvlist,req,channels
  *
  * @return int
  *
  */
  int32_t DiscBySIN(std::vector<SGService> &srv_list,
                   const ProtocolRequest &req,
                   boost::shared_ptr<ServiceChannels> &channels);

  /**
  * 通过LC:本地缓存发现服务节点
  * @param srvlist,req,channels
  *
  * @return int
  *
  */
  int32_t DiscByLC(std::vector<SGService> &srv_list,
                    const ProtocolRequest &req,
                    boost::shared_ptr<ServiceChannels> &channels);

  /**
   *
   * @param servicename
   * @return
   */

  int32_t DiscSrvNameFromZk(ServiceNamePtr &service_name);
  /**
  * 调整服务列表预处理
  * @param srv_list,req,channels
  *
  * @return int
  *
  */

  int32_t AdjustListInChannels(std::vector<SGService> &srv_list,
                               const ProtocolRequest &req,
                               boost::shared_ptr<ServiceChannels> &channels);
  /**
  * 按泳道对服务列表进行标记处理
  * @param srv_list,req,channels
  *
  * @return int
  *
  */
  int32_t AdjustInSwimlane(std::vector<SGService> *srv_list,
                           const ProtocolRequest &req,
                           boost::shared_ptr<ServiceChannels> &channels);


  /**
  * 按原始全量服务列表处理
  * @param srv_list,key,req
  *
  * @return int
  *
  */
  int32_t AdjustInOrigin(std::vector<SGService> &srv_list,
                         const std::string& key,
                         const ProtocolRequest &req);

  /**
  * 获取路由信息
  * @param local_appkey,remote_appkey,protocol
  *
  * @return int32_t
  *
  */
  int32_t DiscRouteList(const std::string &local_appkey,
                        const std::string &remote_appkey,
                        const std::string &protocol);

  /**
  * 从zookeeper获取路由配置信息
  * @param local_appkey,remote_appkey,protocol
  *
  * @return int32_t
  *
  */
  int32_t DiscRouteFromZk(RoutePtr route);

  /**
  * 按路由规则调整服务列表
  * @param local_appkey,remote_appkey,protocol
  *
  * @return int32_t
  *
  */
  int AdjustListInRouteRule(const std::string &localAppkey,
                                                 const std::string &remoteAppkey,
                                                 const std::string &protocol,
                                                 const bool &is_update_route);


  int32_t CheckServiceNode(const SGService &oservice, SGService
  &iservice);

  /**
 * 定时更新srvlist
 */
  void UpdateSrvListTimer();
  /**
   *定时更行routelist
   */
  void UpdateRouteTimer();
  /**
   *定时更新servname
   */
  void UpdateSrvNameTimer();

 private:
  DiscoveryService() {};


  static muduo::MutexLock disc_service_lock_;
  static DiscoveryService *disc_service_;



  static BufferMgr<RouteService> *route_cache_;
  muduo::MutexLock route_cache_lock_;

  static BufferMgr<getservice_res_param_t> *origin_srvlist_cache_;
  muduo::MutexLock origin_srvlist_cache_lock_;

  // 用于存储分组后的serviceList
  static BufferMgr<std::vector<SGService> > *filted_srvlist_cache_;
  muduo::MutexLock filted_srvlist_cache_lock;

  static BufferMgr<getservicename_res_param_t> *srvname_cache_;
  muduo::MutexLock srvname_cache_lock_;

  muduo::net::EventLoop *mns_timer_loop_;
  muduo::net::EventLoopThread mns_timer_thread_;

 protected:

};

}  //  namespace meituan_mns
