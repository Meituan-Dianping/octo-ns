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

#include "discovery_service.h"
#include "discovery_model.h"
#include "sds_tools.h"
#include "service_channels.h"
#include "route_rule.h"
#include "mnsc_client.h"
#include "registry_strategy.h"

namespace meituan_mns {

BufferMgr<RouteService> *DiscoveryService::route_cache_ = NULL;
BufferMgr<getservice_res_param_t> *DiscoveryService::origin_srvlist_cache_ = NULL;
BufferMgr<std::vector <SGService> > *DiscoveryService::filted_srvlist_cache_ = NULL;
BufferMgr<getservicename_res_param_t> *DiscoveryService::srvname_cache_ = NULL;
muduo::MutexLock DiscoveryService::disc_service_lock_;
DiscoveryService *DiscoveryService::disc_service_;
const int kMaxInterval = 20*1000;


int32_t DiscoveryService::Init() {
  route_cache_ = new BufferMgr<RouteService>();
  if (NULL == route_cache_) {
    NS_LOG_ERROR("failed to malloc route_cache_.");
    return -1;
  }
  origin_srvlist_cache_ = new BufferMgr<getservice_res_param_t>();
  if (NULL == origin_srvlist_cache_) {
    NS_LOG_ERROR("failed to malloc origin_srvlist_cache_.");
    return -1;
  }
  filted_srvlist_cache_ = new BufferMgr<std::vector<SGService> >();
  if (NULL == filted_srvlist_cache_) {
    NS_LOG_ERROR("failed to malloc m_filte_srvlist_cache.");
    return -1;
  }
  srvname_cache_ = new BufferMgr<getservicename_res_param_t>();
  if (NULL == srvname_cache_) {
    NS_LOG_ERROR("failed to malloc srvname_cache_.");
    return -1;
  }
  //init discoveryserver thread
  DiscoveryZkClient::GetInstance()->Init();
  return 0;
}

DiscoveryService* DiscoveryService::GetInstance() {
  if (NULL == disc_service_) {
    muduo::MutexLockGuard lock(disc_service_lock_);
    if (NULL == disc_service_) {
      disc_service_ = new DiscoveryService();
    }
  }
  return disc_service_;
}

void DiscoveryService::StartThread() {
  mns_timer_loop_ = mns_timer_thread_.startLoop();
  mns_timer_loop_->runEvery(DEFAULT_PROTOCOL_SCANTIME,
                            boost::bind(&DiscoveryService::UpdateSrvListTimer, this));
  mns_timer_loop_->runEvery(DEFAULT_PROTOCOL_MAX_UPDATETIME,
                            boost::bind(&DiscoveryService::UpdateRouteTimer, this));
  mns_timer_loop_->runEvery(DEFAULT_PROTOCOL_SCANTIME,
                            boost::bind(&DiscoveryService::UpdateSrvNameTimer, this));

}

int32_t DiscoveryService::DiscSrvlistFromCache(std::vector<SGService> &srv_list,
                                               const ProtocolRequest &req,
                                               boost::shared_ptr<ServiceChannels> &channels) {
  return SUCCESS;
}
int32_t DiscoveryService::DiscSrvlistFromMixer(std::vector<SGService> &srv_list,
                                               const ProtocolRequest &req,
                                               boost::shared_ptr<ServiceChannels> &channels) {
  return SUCCESS;
}
void DiscoveryService::UpdateSrvListTimer() {
  if (NULL == origin_srvlist_cache_) {
    NS_LOG_ERROR("ProtocolServicePlugin's ServiceBufferMgr is NULL, please init it before");
    return;
  }
  int static update_count = 0;
  const int static CountTimes = 60;
  update_count = (++update_count) % CountTimes;
  NS_LOG_DEBUG("hlb update_count = " << update_count);

  bool null_node_update = (0 == update_count);


  std::vector<std::string> keys;
  int ret = origin_srvlist_cache_->GetKeyList(keys);
  if (0 >= ret) {
    NS_LOG_ERROR("keys is empty.");
    return ;
  }
  ServicePtr svc_ptr(new getservice_res_param_t());
  ProtocolRequest req;
  std::vector<SGService> servicelist;
  for (std::vector<std::string>::iterator iter = keys.begin();
       keys.end() != iter; ++iter) {
    ret = origin_srvlist_cache_->get(*iter, *svc_ptr);
    if (SUCCESS == ret) {
      if (svc_ptr->serviceList.empty() && !null_node_update) {
        continue;
      }
      req.__set_remoteAppkey(svc_ptr->remoteAppkey);
      req.__set_protocol(svc_ptr->protocol);
      req.__set_localAppkey(svc_ptr->localAppkey);
      DiscNodesFromZk(servicelist, req);
    }
  }
}

void DiscoveryService::UpdateRouteTimer() {
  if (NULL == route_cache_) {
    NS_LOG_ERROR("ProtocolServicePlugin's ServiceNameBufferMgr is NULL, please init it before");
    return;
  }

  std::vector<std::string> keys;
  int ret = route_cache_->GetKeyList(keys);
  if (0 >= ret) {
    NS_LOG_ERROR("keys is empty.");
    return ;
  }
  RouteService route_service;
  for (std::vector<std::string>::iterator iter = keys.begin();
       keys.end() != iter; ++iter) {
    ret = route_cache_->get(*iter, route_service);
    if (SUCCESS == ret) {
      RoutePtr routePtr =
          boost::make_shared<getroute_res_param_t>(route_service.route_list);
      DiscRouteFromZk(routePtr);
    }
  }
}

void DiscoveryService::UpdateSrvNameTimer(void) {
  if (NULL == srvname_cache_) {
    NS_LOG_ERROR("ProtocolServicePlugin's ServiceNameBufferMgr is NULL, please init it before");
    return;
  }

  std::vector<std::string> keys;
  int ret = srvname_cache_->GetKeyList(keys);
  if (0 >= ret) {
    NS_LOG_ERROR("keys is empty.");
    return ;
  }
  std::vector<std::string>::iterator iter;
  ServiceNamePtr servicenamePtr(new getservicename_res_param_t());
  for (iter = keys.begin(); iter != keys.end(); ++iter) {
    ret = srvname_cache_->get(*iter, *servicenamePtr);
    if (SUCCESS == ret) {
      ret = DiscSrvNameFromZk(servicenamePtr);
      if (SUCCESS != ret) {
        NS_LOG_ERROR("failed to sendServiceMsg, serviceName = "
                           << servicenamePtr->servicename
                           << "; version = " << servicenamePtr->version
                           << "; protocol = " << servicenamePtr->protocol
                           << "; errorcode = " << ret);
      }
    } else {
      NS_LOG_ERROR("get ServiceNameBufferMgr fail! key = " << *iter
                                                             << ", ret = " << ret);
    }
  }
}

int32_t DiscoveryService::DiscGetSrvList(std::vector<SGService> &srvlist,
                                         const ProtocolRequest &req,
                                         boost::shared_ptr<ServiceChannels> &channels) {

  switch (DiscoveryModel::GetDiscoveryModelType()) {
    case kDiscZookeeper: {
      return DiscSrvlistFromZk(srvlist, req,channels);
    }
    case kDiscCached: {
      return DiscSrvlistFromCache(srvlist, req,channels);
    }
    case kDiscMixer: {
      return DiscSrvlistFromMixer(srvlist, req,channels);
    }
    default: {
      NS_LOG_ERROR("unsupported discovery method: "
                         << DiscoveryModel::GetDiscoveryModelType());
      return 0;
    }
  }
}
int32_t DiscoveryService::AdjustInOrigin(std::vector<SGService> &srv_list,
                                         const std::string &key,
                                         const ProtocolRequest &req) {

  int ret = FAILURE;
  boost::shared_ptr<getservice_res_param_t>
      service_param(new getservice_res_param_t);
  ret = origin_srvlist_cache_->get(key, *service_param);

  if (SUCCESS == ret) {
    srv_list = service_param->serviceList;
    RouteService routes;
    NS_LOG_INFO("success disc from local cache,size = "<< srv_list.size());

    if (SUCCESS == route_cache_->get(key, routes)) {
      std::vector<CRouteData> exclusive_routes;
      RouteRule::GetExclusiveRoute(routes.route_list.routeList, exclusive_routes, false);
      std::string local_ip = "\"" + CXmlFile::GetStrPara(CXmlFile::LocalIp) + "\"";

      if (!RouteRule::IsMatchRoutesConsumer(exclusive_routes, local_ip)) {
        RouteRule::FilterProvidersByExclusiveRoutes(srv_list, exclusive_routes);
      }
    }else{
      NS_LOG_WARN("Cannot find data from route cache.");
    }
    NS_LOG_INFO("success disc from local cache,size = "<<srv_list.size());
  }
  return ret;

}

int32_t DiscoveryService::AdjustListInChannels(std::vector<SGService> &srv_list,
                                               const ProtocolRequest &req,
                                               boost::shared_ptr<ServiceChannels> &channels){
  int32_t ret = FAILURE;
  std::string key = DiscTools::LocalCacheKey(req.remoteAppkey, req.protocol);
  if(channels->GetOriginChannel()){
    ret = AdjustInOrigin(srv_list,key,req);
    NS_LOG_INFO("get origin channel service list,size = "
                    <<srv_list.size()
                    <<";ret = " << ret);
  } else {
    ret = filted_srvlist_cache_->get(key,srv_list);
    NS_LOG_INFO("get filted service list,size = "
                    <<srv_list.size()
                    <<";ret = " << ret);
  }
  NS_LOG_INFO("success disc from local cache,size = "
                    <<srv_list.size()
                    <<";ret = " << ret);
  return ret;

}

int32_t DiscoveryService::UpdateSrvList(const ServicePtr &service){

    if (NULL == origin_srvlist_cache_) {
      NS_LOG_ERROR("origin_srvlist_cache_ is NULL, please init it before");
      return FAILURE;
    }
    std::string key = DiscTools::LocalCacheKey(service->remoteAppkey, service
        ->protocol);
    ServicePtr tmpServicePtr = boost::make_shared<getservice_res_param_t>();
    std::string version = service->version;
    boost::trim(version);
    if (version.empty()) {//默认为0
      tmpServicePtr->__set_version("0");
    } else {
      tmpServicePtr->__set_version(version);
    }
    int ret = origin_srvlist_cache_->get(key, *tmpServicePtr);
    NS_LOG_INFO("remoteAppkey : " << service->remoteAppkey
                                    << ", protocol = " << service->protocol
                                    << ", tmpServicePtr size ="
                                  << tmpServicePtr->serviceList.size());
    if (SUCCESS == ret) {
      bool flag = false;
      for (std::vector<SGService>::iterator piter = service->serviceList.begin();
           piter != service->serviceList.end(); ++piter) {
        flag = false;
        for (std::vector<SGService>::iterator iter = tmpServicePtr->serviceList.begin();
             iter != tmpServicePtr->serviceList.end(); ++iter) {
          if ((iter->ip == piter->ip) && (iter->port == piter->port)) {
            if (piter->status >= fb_status::DEAD && piter->status <= fb_status::WARNING) {
              iter->__set_status(piter->status);
            } else {
              NS_LOG_ERROR("status is not in conformity with the specification ,ip : " << piter->ip
                                                                                         << " , port : " << piter->port);
              flag = true;
              break;
            }
            if (("thrift" == piter->protocol && 0 == piter->serverType) ||
                ("http" == piter->protocol && 1 == piter->serverType)) {
              iter->__set_protocol(piter->protocol);
              iter->__set_serviceInfo(piter->serviceInfo);
            } else {
              NS_LOG_ERROR("thrift and serverType do not match, protocol: " << piter->protocol
                                                                              << " ,serverType: " << piter->serverType);
              flag = true;
              break;
            }
            if (piter->envir >= 1 && piter->envir <= 3) {
              iter->__set_envir(piter->envir);
            } else {
              NS_LOG_ERROR("envir is not in conformity with the specification, envir : " << piter->envir);
              flag = true;
              break;
            }
            if (piter->fweight > 0.0 && piter->fweight <= 100.0) {
              iter->__set_weight(piter->fweight);
              iter->__set_fweight(piter->fweight);
            } else if (piter->weight > 0 && piter->weight <= 100) {
              iter->__set_weight(piter->weight);
              iter->__set_fweight(piter->weight);
            } else {
              iter->__set_weight(10);
              iter->__set_fweight(10.0);
            }
            if (piter->role > 0) {
              iter->__set_role(piter->role);
            }
            if (piter->lastUpdateTime > 0) {
              iter->__set_lastUpdateTime(piter->lastUpdateTime);
            }
            iter->__set_version(piter->version);
            iter->__set_serviceInfo(piter->serviceInfo);
            iter->__set_heartbeatSupport(piter->heartbeatSupport);
            flag = true;
            break;
          }
        }
        if (!flag) {
          NS_LOG_INFO("ip or port is new, need add srvlist to origin_srvlist_cache_");
          tmpServicePtr->serviceList.push_back(*piter);
        }
      }
      origin_srvlist_cache_->insert(key, *tmpServicePtr);
    } else {
      NS_LOG_INFO("not found,insert the flush node");
      for (std::vector<SGService>::iterator iter = service->serviceList.begin();
           service->serviceList.end() != iter; ++iter) {
        tmpServicePtr->serviceList.push_back(*iter);
      }
      origin_srvlist_cache_->insert(key, *tmpServicePtr);
    }
    ret = AdjustListInRouteRule(service->localAppkey,
                                    service->remoteAppkey,
                                    service->protocol, true);
    if (SUCCESS != ret) {
      NS_LOG_ERROR("Failed to filte service list. localAppkey :" << service->localAppkey
                                                                   << ", remoteAppkey :" << service->remoteAppkey
                                                                   << ", protocol :" << service->protocol
                                                                   << ", ret :" << ret);
    }
    return ret;
}
int32_t DiscoveryService::GetSrvListAndCacheSize(ServListAndCache &list_and_cache,
                                    const std::string &protocol,
                                    const std::string &appkey) {
  if (NULL == origin_srvlist_cache_ || NULL == filted_srvlist_cache_) {
    NS_LOG_ERROR("origin_srvlist_cache_ or filted_srvlist_cache_ is NULL, please init it before");
    return ERR_SERVICE_BUFFER_NULL;
  }
  std::string key = DiscTools::LocalCacheKey(appkey, protocol);
  ServicePtr tmpServicePtr = boost::make_shared<getservice_res_param_t>();
  int ret = origin_srvlist_cache_->get(key, *tmpServicePtr);
  if (SUCCESS == ret) {
    list_and_cache.origin_servlist_size = tmpServicePtr->serviceList.size();
    list_and_cache.origin_servicelist = tmpServicePtr->serviceList;
  } else {
    NS_LOG_WARN("cannot find key in origin_srvlist_cache_, ret = " << ret);
    list_and_cache.origin_servlist_size = 0;
    list_and_cache.origin_servicelist = std::vector<SGService>();
  }
  std::vector<SGService> filte_servlist;
  ret = filted_srvlist_cache_->get(key, filte_servlist);
  if (SUCCESS == ret) {
    list_and_cache.filte_servlist_size = filte_servlist.size();
    list_and_cache.filte_servicelist = filte_servlist;
  } else {
    NS_LOG_WARN("cannot find key in m_filte_srvlist_cache, ret = " << ret);
    list_and_cache.filte_cache_size = 0;
    list_and_cache.filte_servicelist = std::vector<SGService>();
  }
  list_and_cache.origin_cache_size = origin_srvlist_cache_->size();
  list_and_cache.filte_cache_size = filted_srvlist_cache_->size();

  return SUCCESS;
}
int32_t DiscoveryService::RepalceSrvlist(const
                                         boost::shared_ptr<getservice_res_param_t>  &service){
  if (NULL == origin_srvlist_cache_) {
    NS_LOG_ERROR("m_origin_srvlist_cache is NULL, please init.");
    return ERR_SERVICE_BUFFER_NULL;
  }
  if (NULL == filted_srvlist_cache_) {
    NS_LOG_ERROR("m_filted_srvlist_cache is NULL, please init.");
    return ERR_SERVICE_BUFFER_NULL;
  }
  std::string key = DiscTools::LocalCacheKey(service->remoteAppkey, service->protocol);
  NS_LOG_INFO("remoteAppkey : " << service->remoteAppkey
                                  << "protocol : " << service->protocol
                                  << "service size is " << service->serviceList.size());
  ServicePtr tmp_service = make_shared<getservice_res_param_t>();
  if (!service->localAppkey.empty()) {
    tmp_service->__set_localAppkey(service->localAppkey);
  }
  tmp_service->__set_remoteAppkey((service->remoteAppkey));
  tmp_service->__set_protocol(service->protocol);
  std::string version = service->version;
  boost::trim(version);
  if (version.empty()) {//版本号没有填写，默认为0
    tmp_service->__set_version("0");
  } else {
    tmp_service->__set_version(version);
  }
  int ret = FAILURE;
  for (std::vector<SGService>::const_iterator iter = service->serviceList.begin();
       service->serviceList.end() != iter; ++iter) {
    SGService iservice;
    ret = CheckServiceNode(*iter, iservice);
    if (SUCCESS != ret) {
      NS_LOG_WARN("the service param is illegal, ret = " << ret);
      continue;
    } else {
      tmp_service->serviceList.push_back(iservice);
    }
  }
  if (tmp_service->serviceList.empty()) {
    NS_LOG_WARN("input servicelist are all illegal.");
  }
  origin_srvlist_cache_->insert(key, *tmp_service);
  ret = AdjustListInRouteRule(service->localAppkey,
                                  service->remoteAppkey,
                                  service->protocol, false);
  if (SUCCESS != ret) {
    NS_LOG_ERROR("Failed to filte service list. localAppkey :" << service->localAppkey
                                                                 << ", remoteAppkey :" << service->remoteAppkey
                                                                 << ", protocol :" << service->protocol
                                                                 << ", ret :" << ret);
  }
  return ret;
}
int32_t DiscoveryService::CheckServiceNode(const SGService &oservice, SGService
&iservice) {
  std::string appkey = oservice.appkey;
  boost::trim(appkey);
  if (appkey.empty()) {
    NS_LOG_ERROR("fail to register, because the appkey is empty.");
    return ERR_EMPTY_APPKEY;
  } else if (!IsAppkeyLegal(appkey)) {
    NS_LOG_ERROR("Invalid appkey in regist, appkey = " << appkey);
    return ERR_INVALIDAPPKEY;
  }
  if (!IsIpAndPortLegal(oservice.ip, oservice.port)) {
    NS_LOG_ERROR("invalid port: " << oservice.port
                                    << ", appkey: " << oservice.appkey
                                    << ", ip: " << oservice.ip
                                    << ", weight: " << oservice.weight);
    return ERR_INVALID_PORT;
  }
  if (("thrift" == oservice.protocol && 0 == oservice.serverType) ||
      ("http" == oservice.protocol && 1 == oservice.serverType)) {
    iservice.__set_protocol(oservice.protocol);
    iservice.__set_serviceInfo(oservice.serviceInfo);
  } else {
    NS_LOG_ERROR("invalid protocol : " << oservice.protocol
                                         << ", serverType : " << oservice.serverType);
    return ERR_INVALID_PROTOCOL;
  }
  if (oservice.fweight > 0.0 && oservice.fweight <= 100.0) {
    iservice.__set_weight(oservice.fweight);
    iservice.__set_fweight(oservice.fweight);
  } else if (oservice.weight > 0 && oservice.weight <= 100) {
    iservice.__set_weight(oservice.weight);
    iservice.__set_fweight(oservice.weight);
  } else {
    iservice.__set_weight(10);
    iservice.__set_fweight(10.0);
  }
  if (oservice.role > 0) {
    iservice.__set_role(oservice.role);
  }
  if (oservice.lastUpdateTime > 0) {
    iservice.__set_lastUpdateTime(oservice.lastUpdateTime);
  }
  iservice.__set_appkey(appkey);
  iservice.__set_ip(oservice.ip);
  iservice.__set_port(oservice.port);
  iservice.__set_version(oservice.version);
  iservice.__set_envir(oservice.envir);
  iservice.__set_serviceInfo(oservice.serviceInfo);
  iservice.__set_heartbeatSupport(oservice.heartbeatSupport);

  return SUCCESS;
}
int32_t DiscoveryService::DiscRouteFromZk(RoutePtr route) {
  std::string localAppkey = route->localAppkey;
  std::string remoteAppkey = route->remoteAppkey;
  std::string version = route->version;
  std::string protocol = route->protocol;
  if (remoteAppkey.empty() || protocol.empty()) {
    NS_LOG_ERROR("routeMsg's remoteAppkey is empty! localAppkey = "
                       << localAppkey
                       << ", protocol = " << protocol
                       << ", version = " << version);
    return -1;
  }
  if (NULL == route_cache_) {
    NS_LOG_ERROR("ProtocolServicePlugin's Route BufferMgr is NULL, "
                       "please init it before");
    return -2;
  }
  std::vector<CRouteData> routeList;
  //get From zk
  bool exclusive_routes_flag = false;
  int32_t ret = DiscoveryZkClient::GetInstance()->
      DiscRouteListByProtocol(routeList,
                              localAppkey,
                              remoteAppkey,
                              version,
                              protocol,
                              exclusive_routes_flag);
  if (SUCCESS == ret) {
    NS_LOG_DEBUG("succeed to get route from zk"
                       << ", remoteAppkey = " << remoteAppkey
                       << ", protocol = " << protocol
                       << ", routelist' size = " << routeList.size());

    route->__set_version(version);
    route->__set_routeList(routeList);
  } else if (ERR_NODE_NOTFIND == ret) {
    NS_LOG_WARN("can not find route node from zk"
                      << ", appkey = " << remoteAppkey
                      << ", protocol = " << protocol
                      << ", version = " << version);
    return ret;
  } else if (ret == ERR_ZK_LIST_SAME_BUFFER) {
    NS_LOG_DEBUG("ZK getRouteList is the same as buf, localAppkey : "
                       << localAppkey
                       << ", remoteAppkey is : " << remoteAppkey
                       << ", version : " << version);
    return ret;
  } else {
    NS_LOG_ERROR("getServiceList from zk fail, "
                       << ", serviceName = " << remoteAppkey
                       << ", protocol = " << protocol
                       << ", routeList' size = " << routeList.size());
    return ret;
  }
  RouteRule::SortRouteList(route->routeList);

  //write Cache
  std::string key = DiscTools::LocalCacheKey(remoteAppkey, protocol);
  do {
    RouteService route_service;
    route_service.last_update_time = time(0);
    route_service.error_code = ret;
    route_service.route_list = *route;
    route_service.enable_exclusive_routes = exclusive_routes_flag;
    muduo::MutexLockGuard lock(route_cache_lock_);
    route_cache_->insert(key, route_service);
  } while(0);

  NS_LOG_INFO("updating route cache, key = "
                    << key << ", size =" << routeList.size());
  //todo :更新缓存操作
  AdjustListInRouteRule(localAppkey,remoteAppkey,protocol,false);
  return SUCCESS;

}
int32_t DiscoveryService::DiscRouteList(const std::string &local_appkey,
                                        const std::string &remote_appkey,
                                        const std::string &protocol) {
  if (NULL == route_cache_) {
    NS_LOG_ERROR("route_cache_ is null, please init it");
    return ERR_SERVICE_BUFFER_NULL;
  }
  std::string key = DiscTools::LocalCacheKey(remote_appkey, protocol);
  RouteService res_route;
  //先取服务列表
  int ret = route_cache_->get(key, res_route);
  if (SUCCESS != ret || (SUCCESS == ret && ERR_NODE_NOTFIND == res_route.error_code
      && time(0) - res_route.last_update_time > kMaxInterval)) {
    NS_LOG_DEBUG("Cannot find data from route cache , "
                       "and try to get data from zk, ret = " << ret);
    res_route.route_list.__set_localAppkey(local_appkey);
    res_route.route_list.__set_remoteAppkey(remote_appkey);
    res_route.route_list.__set_version("");
    res_route.route_list.__set_protocol(protocol);
    RoutePtr routePtr = boost::make_shared<getroute_res_param_t>(res_route.route_list);
    ret = DiscRouteFromZk(routePtr);
    if (0 != ret) {
      NS_LOG_WARN("getRouteList ret = " << ret
                                          << ", protocol = " << protocol
                                          << ", appkey = " << remote_appkey);
      return ret;
    }
    res_route.error_code = ret;
  }
  return res_route.error_code;

}
int32_t DiscoveryService::DiscBySI(std::vector<SGService> &srv_list,
                                   const ProtocolRequest &req,
                                   boost::shared_ptr<ServiceChannels> &channels){

  if (NULL == origin_srvlist_cache_) {
    NS_LOG_ERROR("origin srvlist cache is NULL, please wait its initialization.");
    return ERR_SERVICE_BUFFER_NULL;
  }
  if (NULL == filted_srvlist_cache_) {
    NS_LOG_ERROR("filted srvlist cache is NULL, please wait its initialization.");
    return ERR_SERVICE_BUFFER_NULL;
  }

  int ret = DiscByLC(srv_list,req,channels);
  if(SUCCESS != ret){
    NS_LOG_WARN("disc from local cache failed, appkey: "
                      <<req.remoteAppkey
                      <<"; protocol:" <<req.protocol);
    ret = DiscRouteList(req.localAppkey,req.remoteAppkey,req.protocol);
    if(SUCCESS !=ret){
      NS_LOG_WARN("disc route list failed,will use origin,appkey: "
                        <<req.remoteAppkey
                        <<"; protocol: " <<req.protocol);
    }
    ret = DiscNodesFromZk(srv_list,req);
    if(SUCCESS != ret){
      NS_LOG_WARN("disc service list failed,appkey: "
                        <<req.remoteAppkey
                        <<"; protocol: " <<req.protocol);
    }
    ret = DiscByLC(srv_list,req,channels);
    if (SUCCESS != ret) {
      NS_LOG_ERROR("failed to get service from cache ,"
                         "after geting servicelist from zk. "
                         <<req.remoteAppkey
                         <<", protocol: " <<req.protocol);
    }
  }
  return ret;
}
int32_t DiscoveryService::DiscBySrvName(std::vector<SGService> &srv_list,
                                        const ProtocolRequest &req,
                                        boost::shared_ptr<ServiceChannels> &channels){
  if (NULL == srvname_cache_) {
    NS_LOG_ERROR("servicename cache is NULL, please wait its initialization.");
    return ERR_SERVICENAME_BUFFER_NULL;
  }
  std::string key = DiscTools::LocalCacheKey(req.serviceName, req.protocol);
  ServiceNamePtr resServiceNamePtr(new getservicename_res_param_t());
  int ret = srvname_cache_->get(key, *resServiceNamePtr);
  if (SUCCESS == ret) {
    ProtocolRequest req_tmp = req;
    int failure_times = 0;
    for (std::set<std::string>::iterator
             iter = resServiceNamePtr->appkeys.begin();
         resServiceNamePtr->appkeys.end() != iter; ++iter) {
      std::vector<SGService> servicelist;
      req_tmp.__set_remoteAppkey(*iter);
      ret = DiscBySI(servicelist, req_tmp, channels);
      if (SUCCESS == ret) {
        srv_list.insert(srv_list.begin(),
                        servicelist.begin(),
                        servicelist.end());
      } else {
        ++failure_times;
        NS_LOG_ERROR("Failed to get servicelist by Appkey, Appkey = "
                           << req_tmp.remoteAppkey
                           << ", ret = " << ret);
      }
    }
    if (0 != failure_times
        && failure_times == resServiceNamePtr->appkeys.size()) {
      NS_LOG_ERROR(
          "Failed to get allservilists, failure_times = " << failure_times);
      return ret;
    }
    RouteRule::FilterServiceName(srv_list, req.serviceName);
    return SUCCESS;
  } else {
    NS_LOG_WARN("Cannot find data from servicename cache, ret = " << ret);
    return ret;
  }

}
int32_t DiscoveryService::DiscByLC(std::vector<SGService> &srv_list,
                 const ProtocolRequest &req,
                 boost::shared_ptr<ServiceChannels> &channels){

  return AdjustListInChannels(srv_list,req,channels);

}

int32_t DiscoveryService::DiscNodesFromZk(std::vector<SGService> &srv_list,
                                          const ProtocolRequest &req) {


  ServicePtr service(new getservice_res_param_t());
  service->__set_localAppkey(req.localAppkey);
  service->__set_remoteAppkey(req.remoteAppkey);
  service->__set_version("");
  service->__set_protocol(req.protocol);
  int ret = DiscoveryZkClient::GetInstance()->DiscSrvListByProtocol(service);
  if (SUCCESS == ret || ERR_NODE_NOTFIND == ret) {
    if (ERR_NODE_NOTFIND == ret) {
      NS_LOG_WARN("can not find service node from zk"
                        << ", appkey = " << req.remoteAppkey
                        << ", protocol = " << req.protocol);
      // return empty list
      service->serviceList.clear();
    }
    if (SUCCESS == ret) {
      NS_LOG_INFO("succeed to get service list from zk"
                        << ", remoteAppkey = " << req.remoteAppkey
                        << ", protocol = " << req.protocol
                        << ", serviceList' size = " << service->serviceList.size());
      srv_list = service->serviceList;
    }
  } else if (ret == ERR_ZK_LIST_SAME_BUFFER) {
    // already log inside
    return -3;
  } else {
    NS_LOG_ERROR("getServiceList from zk fail, "
                       << ", remoteAppkey = " << req.remoteAppkey
                       << ", protocol = " << req.protocol
                       << ", serviceList' size = " << service->serviceList.size());
    return -4;
  }

  std::string key = req.protocol + "+" + req.remoteAppkey;

  origin_srvlist_cache_->insert(key, *service);
  NS_LOG_INFO("the service resServicePtr list size = "
                    << service->serviceList.size());

  ret = AdjustListInRouteRule(req.localAppkey,
                        req.remoteAppkey,
                        req.protocol,false);

  if (SUCCESS == ret) {
    srv_list.clear();
    ret = filted_srvlist_cache_->get(key, srv_list);
  }
  return ret;
}

int32_t DiscoveryService::AdjustListInRouteRule(const std::string &localAppkey,
                                       const std::string &remoteAppkey,
                                       const std::string &protocol,
                                       const bool &is_update_route) {

  const std::string key = DiscTools::LocalCacheKey(remoteAppkey, protocol);

  //RouteService route_service;
  boost::shared_ptr<RouteService> routeParamFilte
      (new RouteService());

  boost::shared_ptr<getservice_res_param_t> serviceParamFilte
      (new getservice_res_param_t());
  if (is_update_route) {
    DiscRouteList(localAppkey,remoteAppkey,protocol);
  }
  int32_t ret = route_cache_->get(key, *(routeParamFilte.get()));
  if (SUCCESS != ret) {
    NS_LOG_WARN("null from route buffer when update service list, key = "
                      << key << ", ret = " << ret);
    return ret;
  }
  bool route_action = (SUCCESS == ret);
  ret = origin_srvlist_cache_->get(key, *(serviceParamFilte.get()));

  if (SUCCESS != ret) {
    NS_LOG_WARN("null from route buffer when update service list, key = "
                      << key << ", ret = " << ret);
    return ret;
  }

  RouteRule::SyncFweight(serviceParamFilte->serviceList);
  if (route_action) {
    RouteRule::FilterRoute(serviceParamFilte->serviceList,
                           routeParamFilte->route_list.routeList,
                           CXmlFile::GetStrPara(CXmlFile::LocalIp),
                           routeParamFilte->enable_exclusive_routes,
                           true); //todo:从配置中获取
  }
  filted_srvlist_cache_->insert(key, serviceParamFilte->serviceList);

  NS_LOG_INFO("update filted serviceList, key = "
                    << key
                    << ", size = " << serviceParamFilte->serviceList.size());

  return SUCCESS;
}

int32_t DiscoveryService::DiscSrvlistFromZk(std::vector<SGService> &srv_list,
                                            const ProtocolRequest &req,
                                            boost::shared_ptr<ServiceChannels> &channels){
  //兼容现有sdk传入参数逻辑
  int32_t ret = FAILURE;
  if(!req.remoteAppkey.empty()){

    NS_LOG_INFO("discovery srvlist in service instance: "<<req.remoteAppkey);
    ret = DiscBySI(srv_list,req,channels);

  }else if(!req.serviceName.empty()){

    NS_LOG_INFO("discovery srvlist in service interface name: "<<req.serviceName);
    ret = DiscBySIN(srv_list,req,channels);
  }else{
    NS_LOG_WARN("discovery unsupported methods");
  }
  return ret;

}
int32_t DiscoveryService::DiscBySIN(std::vector<SGService> &srv_list,
                                    const ProtocolRequest &req,
                                    boost::shared_ptr<ServiceChannels> &channels) {
  int32_t ret = DiscBySrvName(srv_list, req, channels);
  if (SUCCESS != ret) {
    NS_LOG_DEBUG("Cannot find data from cache ,and try to get serviceName from zk.");
    ServiceNamePtr service_name = boost::make_shared<getservicename_res_param_t>();
    service_name->__set_localAppkey(req.localAppkey);
    service_name->__set_servicename(req.serviceName);
    service_name->__set_protocol(req.protocol);
    service_name->__set_version("");
    ret = DiscSrvNameFromZk(service_name);
    if (SUCCESS != ret) {
      NS_LOG_ERROR("getAppkeyByServiceNameFromWorker failed. localAppkey = "
                         << req.localAppkey
                         << ", serviceName = " << req.serviceName
                         << ", protocol = " << req.protocol
                         << ", ret = " << ret);
      return ret;
    }
    ret = DiscBySrvName(srv_list, req, channels);
  }
  return ret;
}
int32_t DiscoveryService::DiscSrvNameFromZk(ServiceNamePtr &service_name) {
  if (NULL == srvname_cache_) {
    NS_LOG_ERROR("ProtocolServicePlugin's srvname_cache_ is NULL, "
                       "please init it before");
    return FAILURE;
  }

  std::string localAppkey = service_name->localAppkey;
  std::string serviceName = service_name->servicename;
  std::string protocol = service_name->protocol;
  std::string version = service_name->version;
  NS_LOG_INFO("TO get Appkey From Worker, serviceName = "
                    << serviceName
                    << ", protocol = " << protocol);

  std::set<std::string> appkeys;
  if (serviceName.empty()) {
    NS_LOG_ERROR(
        "task which is not complete, serviceName is NULL. localAppkey = "
            << localAppkey << ", protocol = " << protocol
            << ", version = " << version);
    return -2;
  } else {
    //get From zk
    int32_t ret = DiscoveryZkClient::GetInstance()->
        DiscAppkeyByServiceName(appkeys, localAppkey, serviceName, version, protocol);
    if (SUCCESS == ret) {
      NS_LOG_DEBUG("succeed to get serviceName from zk"
                         << ", serviceName = " << serviceName
                         << ", protocol = " << protocol
                         << ", appkeys' size = " << appkeys.size());
      service_name->__set_version(version);
      service_name->__set_appkeys(appkeys);
    } else if (ERR_NODE_NOTFIND == ret) {
      NS_LOG_WARN("can not find serviceName node from zk"
                        << ", serviceName = " << serviceName
                        << ", protocol = " << protocol
                        << ", version = " << version);
      return ret;
    } else if (ERR_ZK_LIST_SAME_BUFFER == ret) {
      // already log inside
      return -3;
    } else {
      NS_LOG_ERROR("getServiceName from zk fail, "
                         << ", serviceName = " << serviceName
                         << ", protocol = " << protocol
                         << ", appkeys' size = " << appkeys.size());
      return FAILURE;
    }

    //write Cache
    std::string key = DiscTools::LocalCacheKey(serviceName, protocol);
    srvname_cache_->insert(key, *service_name);

    NS_LOG_INFO("Receive ServiceName Task in worker thread, key = "
                      << key
                      << ", appkey size:"
                      << service_name->appkeys.size());
    return SUCCESS;
  }
}

}  //  namespace meituan_mns
