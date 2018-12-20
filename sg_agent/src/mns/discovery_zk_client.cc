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

#include "discovery_zk_client.h"
#include "discovery_service.h"
#include "log4cplus.h"
#include "zk_client.h"
#include "zk_client_pool.h"
#include "json_data_tools.h"
#include "base_mns_consts.h"
#include "base_errors_consts.h"
#include "base_consts.h"
#include "sds_tools.h"
#include "zk_path_tools.h"
#include "inc_comm.h"

namespace  meituan_mns {

DiscoveryZkClient *DiscoveryZkClient::disc_zk_instance_ = NULL;
muduo::net::EventLoopThreadPool *DiscoveryZkClient::disc_pool_ = NULL;
muduo::MutexLock DiscoveryZkClient::disc_zk_service_lock_;

void DiscoveryZkClient::ThreadInfo(void) {
  NS_LOG_INFO("registry thread info,pid = " << getpid()
                                              << "; tid = "
                                              << muduo::CurrentThread::tid());
}

DiscoveryZkClient *DiscoveryZkClient::GetInstance() {

  if (NULL == disc_zk_instance_) {
    muduo::MutexLockGuard lock(disc_zk_service_lock_);
    if (NULL == disc_zk_instance_) {
      disc_zk_instance_ = new DiscoveryZkClient();
    }
  }
  return disc_zk_instance_;
}

void DiscoveryZkClient::Init() {

  disc_loop_ = disc_loop_thread_.startLoop();
  disc_loop_->runInLoop(boost::bind(&DiscoveryZkClient::InitDiscThreadPool,
                                    this, disc_loop_,
                                    CXmlFile::GetI32Para(CXmlFile::WatcherDispatchThreads),
                                    "disc-num"));
}

void DiscoveryZkClient::InitDiscThreadPool(muduo::net::EventLoop *mns_loop,
                                           int32_t num_threads,
                                           const std::string &name) {

  NS_LOG_INFO("Init discovery threads num: " << num_threads);
  if (0 == num_threads || num_threads > kMaxDiscThreads) {
    NS_LOG_WARN("the discovery thread is " << num_threads);
    num_threads = kMaxDiscThreads;
  }
  disc_pool_ = new muduo::net::EventLoopThreadPool(mns_loop, name);
  disc_pool_->setThreadNum(num_threads);
  SetThreadInitCallback(boost::bind(&DiscoveryZkClient::ThreadInfo, this));
  disc_pool_->start(threadInitCallback_);
  disc_loops_ = disc_pool_->getAllLoops();
}

int32_t DiscoveryZkClient::DiscAppkeyByServiceName(std::set<std::string> &appkeys,
                                                   const std::string &local_appkey,
                                                   const std::string &service_name,
                                                   std::string &version,
                                                   const std::string &protocol) {
  std::string zk_path = "";
  int32_t ret = agent_zk_path_.GenServiceNameZkPathNode(zk_path, service_name, protocol);
  if (SUCCESS != ret) {
    NS_LOG_ERROR("_gen genServiceNameZkPathNode fail! protocol is empty! zkPath: "
                       << zk_path);
    return ret;
  }

  std::string strJson;
  ZkGetRequestPtr request = boost::make_shared<ZkGetRequest>();
  request->path = zk_path;
  request->watch = 0;
  ZkGetResponsePtr response = boost::make_shared<ZkGetResponse>();
  ret = ZkClientPool::GetInstance()->GetZkClientForHash(kZkClientIndex)->
      ZkGet(request, response);
  if (ZOK != ret) {
    // the zkPath contains necessary info
    NS_LOG_ERROR(
        "fail to getAppkeyByServiceName, path = "
            << zk_path << ", err_code = " << ret);
    return ret;
  }
  strJson = response->buffer;
  ServiceNode oserviceNode;
  ret = JsonZkMgr::Json2ServiceNode(strJson, oserviceNode);
  if (SUCCESS != ret) {
    NS_LOG_ERROR("getAppkeyByServiceName _Json2ServiceNode fail, Json:  "
                       << strJson
                       << ", ret = " << ret);
    return ret;
  }
  if (!version.empty() &&
      response->stat.version <= atoi(version.c_str())) {
    NS_LOG_INFO("srvname: "<< service_name <<" buff version has no change.");
    return ERR_ZK_LIST_SAME_BUFFER;
  }
  ret = ConvertInt2String(response->stat.version, version);
  if (SUCCESS != ret) {
    NS_LOG_ERROR("version : " << response->stat.version
                                << " Failed to ConvertInt2String.");
  }
  appkeys = oserviceNode.appkeys;
  return SUCCESS;
}

void DiscoveryZkClient::DiscByProtocolWatcher(zhandle_t *zh,
                                              int32_t type,
                                              int32_t state,
                                              const char *path,
                                              void *watcherCtx) {

  NS_LOG_INFO("rcv the watcher from the ZK server by protocol,path"
                    << path << "type" << type);
  if (strlen(path) == 0 || type == -1) {
    NS_LOG_ERROR("get event serviceByProtocolWatcher,  "
                       "that ZK server may down! state = " << state
                                                           << ", type = " << type
                                                           << ", path = " << path);
    return;
  } else {
    std::string path_str(path);
    NS_LOG_INFO("zk watch trigger: path = " << path_str);
  }

  std::string appkey = "";
  std::string protocol = "";
  int32_t ret = AgentZkPath::DeGenZkPath(path, appkey, protocol);

  if (SUCCESS != ret) {
    NS_LOG_ERROR("deGenZkPath is serviceByProtocolWatcher is wrong! path:"
                       << path << ", appkey:" << appkey
                       << ", protocol:" << protocol);
    return;
  }
  DiscoveryService *disc = static_cast<DiscoveryService *>(watcherCtx);
  std::vector<SGService> service_list;
  ProtocolRequest request;
  request.__set_remoteAppkey(appkey);
  request.__set_protocol(protocol);
  request.__set_localAppkey("sg_agent_protocol_watcher");
  disc->DiscNodesFromZk(service_list, request);

}
int32_t DiscoveryZkClient::DiscSrvListByProtocol(ServicePtr &service,
                                                 bool is_watcher_callback) {

  service->serviceList.clear();
  // generate the zk path of provider
  std::string provider_path = "";
  std::string node_type = "provider";
  int32_t ret = agent_zk_path_.GenProtocolZkPath(provider_path,
                                                 service->remoteAppkey,
                                                 service->protocol,
                                                 node_type);
  if (SUCCESS != ret) {
    return ret;
  }
  // wget the data of provider path
  ZkWGetRequestPtr zk_wget_req(new ZkWGetRequest);
  zk_wget_req->path = provider_path;
  if (whitelist_manager_.IsAppkeyInWhitList(service->remoteAppkey)) {
    zk_wget_req->watch = NULL;
  } else {
    zk_wget_req->watch = DiscByProtocolWatcher;
    zk_wget_req->watcherCtx = DiscoveryService::GetInstance();
  }

  ZkWGetResponsePtr zk_wget_res(new ZkWGetResponse());
  ret = ZkClientPool::GetInstance()->GetZkClientForHash(kZkClientIndex)->
      ZkWget(zk_wget_req,zk_wget_res);
  if(ZOK != ret){
    NS_LOG_ERROR("fail to zk wget. ret = "
                       << ret << ", path = " << provider_path);
    return ret;
  }
  struct Stat stat = zk_wget_res->stat;
  std::string version = service->version;
  GetMnsCacheParams params;
  params.appkey = service->remoteAppkey;
  params.protocol = service->protocol;
  params.env = CXmlFile::GetAppenv()->GetStrEnv();
  params.version = GetVersion(stat.mtime, stat.cversion, stat.version);
  ret = version_manager_.CheckZkVersion(service->remoteAppkey + service->protocol,
                                        params.version, version);

  if (0 != params.appkey.compare(CXmlFile::GetStrPara
                                     (CXmlFile::MNSCacheAppkey))) {
    ret =
        MnscClient::GetInstance()->GetServicelistFromMnsc(service->serviceList,
                                                          params);
    if (SUCCESS == ret) {
      NS_LOG_INFO("register watcher for parent node");
      return SUCCESS;
    }
  }
  NS_LOG_INFO("register watcher for parent node");
  ZkWGetChildrenRequestPtr zk_wget_child_req =
      boost::shared_ptr<ZkWGetChildrenRequest>(new (ZkWGetChildrenRequest));
  ZkWGetChildrenResponsePtr zk_wget_child_res =
      boost::shared_ptr<ZkWGetChildrenResponse>(new (ZkWGetChildrenResponse));

  zk_wget_child_req->path = provider_path;
  zk_wget_child_req->watch = DiscByProtocolWatcher;
  zk_wget_child_req->watcherCtx = 0;

  ret = ZkClientPool::GetInstance()->GetZkClientForHash(kZkClientIndex)->
      ZkWgetChildren(zk_wget_child_req, zk_wget_child_res);
  if (ZOK != ret) {
    NS_LOG_ERROR("fail to get children nodes from zk. ret = "
                       << ret << ", path = " << provider_path);
    return ret;
  }
  muduo::CountDownLatch thread_finish_countdown(kMaxDiscThreads);
  SubGetServiceParams sub_params;
  sub_params.provider_path = provider_path;
  sub_params.remote_appkey = service->remoteAppkey;
  sub_params.wg_child_params = zk_wget_child_res;

  std::vector<SubGetServiceResPtr> all_res;
  for (int32_t i = 0; i < kMaxDiscThreads; ++i) {
    NS_LOG_INFO("the watcher zk threads get loops length = "
                      << disc_loops_[i]->queueSize());
    SubGetServiceResPtr sub_res(new SubGetServiceRes());
    all_res.push_back(sub_res);
    sub_params.index = i;
    disc_loops_[i]->runInLoop(boost::bind(&DiscoveryZkClient::GetSubSrvListFromZk,
                                          this,
                                          sub_res,
                                          sub_params,
                                          &thread_finish_countdown));
  }
  thread_finish_countdown.wait();
  for (std::vector<SubGetServiceResPtr>::iterator iter = all_res.begin();
       all_res.end() != iter; ++iter) {
    if (!(*iter)->is_ok) {
      return FAILURE;
    }
    service->serviceList.insert(service->serviceList.begin(),
                                (*iter)->srvlist.begin(), (*iter)->srvlist.end());
  }
  NS_LOG_INFO("watcher get svrlist from the zk, size is "
                    << service->serviceList.size()
                    << ", thread num = " << kMaxDiscThreads);

  // 服务节点下线或者反序列化失败
  if (zk_wget_child_res->count != service->serviceList.size()) {
    NS_LOG_WARN("srvlist size is " << service->serviceList.size()
                                     << ", childnode num is "
                                     << zk_wget_child_res->count
                                     << ". Json failed or nodes have been deleted.");
  }
  return SUCCESS;

}

void DiscoveryZkClient::DivRangeDiscIndex(const int32_t &index,
                                          int32_t &begin,
                                          int32_t &end,
                                          const int32_t &child_count) {
  begin = index * child_count / kMaxDiscThreads;
  end = (index + 1) * child_count / kMaxDiscThreads;

  NS_LOG_INFO("the index " << index << ", begin "
                             << begin << ", end "
                             << end << ", child_count "
                             << child_count);
}
int32_t DiscoveryZkClient::GetSubSrvListFromZk(SubGetServiceResPtr &sub_res,
                                               const SubGetServiceParams &sub_params,
                                               muduo::CountDownLatch *p_countdown){
  int32_t ret = -1;
  sub_res->srvlist.clear();
  int32_t begin = 0, end = 0;
  NS_LOG_INFO("the remote appkey is "
                    << sub_params.remote_appkey
                    << ", cout = " << sub_params.wg_child_params->count);

  DivRangeDiscIndex(sub_params.index, begin,
                    end, sub_params.wg_child_params->count);
  for (int32_t i = begin; i < end; ++i){
    std::string zk_node_path = sub_params.provider_path + "/" + sub_params.wg_child_params->data[i];
    ZkGetRequestPtr zk_get_req = boost::shared_ptr<ZkGetRequest>(new ZkGetRequest());
    ZkGetResponsePtr zk_get_res = boost::shared_ptr<ZkGetResponse>(new ZkGetResponse());
    zk_get_req->path = zk_node_path;
    zk_get_req->watch = 0;
    ret = ZkClientPool::GetInstance()->GetZkClientForHash(kZkClientIndex)->ZkGet(zk_get_req, zk_get_res);

    if (ERR_NODE_NOTFIND == ret) {
      NS_LOG_WARN("the service node has been deleted, "
                        "ignore it and continue updating service list. path = "
                        << zk_node_path);
      continue;
    } else if (ZOK != ret){
      NS_LOG_ERROR("fail to get zk data. ret = "
                         << ret << ", path = : " << zk_node_path);
      sub_res->is_ok = false;
      p_countdown->countDown();
      return ret;
    }
    std::string node_json_str = zk_get_res->buffer;
    NS_LOG_INFO("succeed to zoo_get, json: " << node_json_str);
    SGService oservice;
    ret = JsonZkMgr::Json2SGService(node_json_str, oservice);
    if (ret != 0) {
      NS_LOG_WARN("fail to parse node json str. "
                        << ", ret = " << ret
                        << ", path = " << zk_node_path
                        << ", json = " << node_json_str);
      continue;
    }
    if (oservice.appkey != sub_params.remote_appkey) {
      NS_LOG_WARN("expected appkey: " << sub_params.remote_appkey
                                        << ", but node.appky = " << oservice.appkey
                                        << ", path = " << zk_node_path);
      continue;
    }
    //update srvlist
    sub_res->srvlist.push_back(oservice);
    NS_LOG_INFO("srvlist size : " << sub_res->srvlist.size());
  }
  p_countdown->countDown();
}

int32_t DiscoveryZkClient::DiscRouteListByProtocol(std::vector<CRouteData> &routeList,
                                const std::string &localAppkey,
                                const std::string &appKey,
                                std::string &version,
                                const std::string &protocol,
                                bool &route_flag){
  std::string zk_path = "";
  std::string nodeType = "route";
  int32_t ret = agent_zk_path_.GenProtocolZkPath(zk_path, appKey, protocol, nodeType);
  if (0 != ret) {
    NS_LOG_ERROR("gen zk route path failed, zk path:"<<zk_path);
    return ret;
  }
  NS_LOG_DEBUG("_gen getRoute zkPath: " << zk_path << ", nodeType: " << nodeType);

  int32_t data_len = kZkContentSize;
  std::string str_json;
  struct Stat stat;
  struct Stat stat_child;

  ZkGetRequestPtr zk_get_req = boost::shared_ptr<ZkGetRequest>(new ZkGetRequest());
  ZkGetResponsePtr zk_get_res = boost::shared_ptr<ZkGetResponse>(new ZkGetResponse());
  zk_get_req->path = zk_path;
  zk_get_req->watch = 0;
  ret = ZkClientPool::GetInstance()->GetZkClientForHash(kZkClientIndex)->ZkGet(zk_get_req, zk_get_res);
  if(ZOK != ret){
    NS_LOG_WARN("fail to get route data, ret = " << ret << ", path = "
                                                   << zk_get_req->path);
    return ret;
  }else if(ERR_NODE_NOTFIND == ret) {
    NS_LOG_WARN("fail to get route data, because the zk node don't exist. ret = " << ret
                                                                                << ", path = "
                                                                                << zk_get_req->path);
    return ERR_NODE_NOTFIND;
  }
  str_json = zk_get_res->buffer;
  data_len = zk_get_res->buffer_len;
  stat = zk_get_res->stat;
  if (str_json.empty()) {
    NS_LOG_ERROR("strJson is empty , cannot Json2RouteNode.");
    return ERR_DATA_TO_JSON_FAIL;
  }
  CRouteNode oroute;
  ret = JsonZkMgr::Json2RouteNode(str_json, stat.mtime, stat.version, stat.cversion, oroute);
  if (ret != 0) {
    NS_LOG_ERROR("_Json2RouteNode fail ret = " << ret
                                                 << ", json is: " << str_json.c_str());
    return ERR_DATA_TO_JSON_FAIL;
  }
  std::string zk_version = GetVersion(oroute.mtime, oroute.cversion, oroute.version);
  NS_LOG_INFO("the route info, key: "<<appKey + protocol<<"; zk_version: "<<zk_version<<"; version:"<<version);
  ret = version_manager_.CheckZkVersion(appKey + protocol, zk_version, version);
  if (0 != ret) {
    NS_LOG_DEBUG("ZK serviceList is the same as buf, key : " << appKey
                                                               << ", version : " << version);
    return ret;
  }

  ZkWGetChildrenRequestPtr zk_wget_child_req =
      boost::shared_ptr<ZkWGetChildrenRequest>(new (ZkWGetChildrenRequest));
  ZkWGetChildrenResponsePtr zk_wget_child_res =
      boost::shared_ptr<ZkWGetChildrenResponse>(new (ZkWGetChildrenResponse));

  zk_wget_child_req->path = zk_path;
  zk_wget_child_req->watch = 0;

  ret = ZkClientPool::GetInstance()->GetZkClientForHash(kZkClientIndex)->ZkWgetChildren(zk_wget_child_req,
                                                                                        zk_wget_child_res);

  if (ZOK != ret) {
    NS_LOG_WARN("getRoute.zoo_wget_children fail, zkPath: " << zk_path
                                                              << ", ret = " << ret);
    return ret;
  } else if (zk_wget_child_res->count == 0) {
    NS_LOG_WARN("getRouteList strings.count == 0 !");
    return 0;
  }

  //如果子节点非空，则遍历所有子节点，构造返回服务列表

  std::string route_node_path = "";
  for (int32_t i = 0; i < zk_wget_child_res->count; i++) {
    route_node_path = agent_zk_path_.GenRoutePath(zk_path,
                                                  zk_wget_child_res->data[i]);

    //获取子节点信息时，去掉watcher
    ZkGetRequestPtr zk_get_req(new ZkGetRequest());
    ZkGetResponsePtr zk_get_res(new ZkGetResponse());
    zk_get_req->path = route_node_path;
    zk_get_req->watch = 0;
    ret = ZkClientPool::GetInstance()->GetZkClientForHash(kZkClientIndex)->
        ZkGet(zk_get_req, zk_get_res);
    if (ZOK != ret) {
      NS_LOG_ERROR("getRouteList.zoo_wget children "
                         << i << ", fail ret: " << ret
                         << " zkpath is : " << route_node_path);
      return ret;
    }

    str_json.assign(zk_get_res->buffer);
    data_len = zk_get_res->buffer_len;
    stat_child = zk_get_res->stat;

    if (str_json.empty() || 0 == data_len) {
      continue;
    }

    CRouteData routeData;
    ret = JsonZkMgr::Json2RouteData(str_json, routeData);
    if (SUCCESS != ret) {
      NS_LOG_ERROR("json2routeData fail, strJson is:" << str_json.c_str()
                                                        << ", ret = " << ret);
      continue;
    }

    /// double check
    if (routeData.appkey != appKey) {
      NS_LOG_ERROR("unexpected exception! appkey " << appKey.c_str()
                                                     << "routeData.appKey "
                                                     << routeData.appkey.c_str()
                                                     << "datalen " << data_len);
      continue;
    }
    //将数据插入到返回列表中
    routeList.push_back(routeData);
    if (!route_flag && 4 == routeData.category) {
      route_flag = true;
    }
  }

  return 0;
}

}  //  namespace meituan_mns
