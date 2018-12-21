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

#include "registry_service.h"
#include "registry_strategy.h"
#include "naming_common_types.h"
#include "base_consts.h"
#include "base_mns_consts.h"
#include "base_errors_consts.h"
#include "log4cplus.h"
#include "zk_client_pool.h"
#include "json_data_tools.h"
#include "zk_path_tools.h"
#include <boost/lexical_cast.hpp>


namespace meituan_mns {

RegistryService *RegistryService::registry_service_ = NULL;
muduo::MutexLock RegistryService::registry_service_lock_;

RegistryServiceBatch *RegistryServiceBatch::registry_service_batch_ = NULL;
muduo::MutexLock RegistryServiceBatch::registry_service_batch_lock_;



RegistryService* RegistryService::GetInstance() {
  if (NULL == registry_service_) {
    muduo::MutexLockGuard lock(registry_service_lock_);
    if (NULL == registry_service_) {
      registry_service_ = new RegistryService();
    }
  }
  return registry_service_;
}


RegistryServiceBatch* RegistryServiceBatch::GetInstance() {
  if (NULL == registry_service_batch_) {
    muduo::MutexLockGuard lock(registry_service_batch_lock_);
    if (NULL == registry_service_batch_) {
      registry_service_batch_ = new RegistryServiceBatch();
    }
  }
  return registry_service_batch_;
}

Registry* RegistryFactory::CreateRegistry(RegistryType type) {
  return RegistryService::GetInstance();
  switch (CXmlFile::GetI32Para(CXmlFile::RegistryEntry)) {
    case kAddReg: {
      return RegistryService::GetInstance();
    }
    case kBatchReg: {
      return RegistryServiceBatch::GetInstance();
    }
    default: {
      break;
    }
  }
  return NULL;
}

const uint32_t RegistryService::RegistryStart(const SGService &oService,
                                              const int32_t &reg_action) {
  int32_t ret = register_strategy_.IsAllowedRegistry(oService);
  if (SUCCESS != ret) {
    NS_LOG_ERROR("its not allowed to registry");
    return FAILURE;
  }
  SGServicePtr service_ptr = boost::make_shared<SGService>(oService);
  ret = register_strategy_.CheckAllowedProtocolRegistry(service_ptr);
  if (SUCCESS != ret) {
    return ret;
  }
  RegParamPtr regParam_ptr(new regist_req_param_t());
  regParam_ptr->sgservice = *service_ptr;
  regParam_ptr->uptCmd = reg_action;
  regParam_ptr->regCmd = RegistCmd::REGIST;
  ret = RegistryAction(regParam_ptr,reg_action);
  if (SUCCESS != ret){
    NS_LOG_ERROR("success registry, appkey: "<< service_ptr->appkey<<"; ip = "
                                               <<service_ptr->ip);
    return FAILURE;
  }
  NS_LOG_INFO("success registry, appkey: "<< service_ptr->appkey<<"; ip = "
                                            << service_ptr->ip);
  return SUCCESS;
}

const uint32_t RegistryService::UnRegistryStart(const SGService &oService,
                                                const int32_t &reg_action){

  if (SUCCESS != register_strategy_.IsAllowedRegistry(oService)) {
    NS_LOG_ERROR("its not allowed to unregistry");
    return FAILURE;
  }
  uint32_t round_index = 0;
  srand((unsigned) time(NULL));
  SGServicePtr servicePtr(new SGService(oService));
  servicePtr->status = fb_status::DEAD;
  RegParamPtr regParamPtr(new regist_req_param_t);
  regParamPtr->sgservice = *servicePtr;
  regParamPtr->uptCmd = reg_action;
  regParamPtr->regCmd = RegistCmd::UNREGIST;

  if (SUCCESS == RegistryAction(regParamPtr,reg_action)){
    NS_LOG_INFO("success un-registry, appkey: "<<servicePtr->appkey<<"; ip : "
                                              <<servicePtr->ip);
    return SUCCESS;
  } else{
    NS_LOG_ERROR("success un-registry, appkey: "<<servicePtr->appkey<<"; ip : "
                                               <<servicePtr->ip);
    return FAILURE;
  }

}

int32_t RegistryService::RegistryServiceToZk(const SGService &oservice,
                                              RegistCmd::type regCmd,
                                              int uptCmd) {
  int32_t ret = FAILURE;
  std::string zk_provider_path= "";
  ret = agent_path_.GenRegisterZkPath(zk_provider_path, oservice.appkey,
                                      oservice.protocol, oservice.serverType);
  if (SUCCESS != ret) {
    NS_LOG_INFO("_gen registry path fail! appkey = "
                      << oservice.appkey
                      << ", protocol = " << oservice.protocol
                      << ", serverType = " << oservice.serverType);
    return ret;
  }
  ZkExistsRequestPtr zk_exists_req(new ZkExistsRequest());
  zk_exists_req->path = zk_provider_path;
  zk_exists_req->watch = 0;
  ret = ZkClientPool::GetInstance()->GetZkClientForHash(kZkClientIndex)->
      ZkExists(zk_exists_req);

  if (ZNONODE == ret || ret == FAILURE) {
    NS_LOG_INFO("appkey not exist.zkpath:" << zk_provider_path
                                             << ", ip : " << oservice.ip
                                             << ", port : " << oservice.port
                                             << ", or zk handle fail ret: " << ret);
    if(0 >= ZkClientPool::GetInstance()->GetZkClientForHash(kZkClientIndex)->
        ZkPathCreateRecursivly(zk_provider_path)){
      NS_LOG_ERROR("recursivly create zk node failed,zk_path: "
                       << zk_provider_path);
      return FAILURE;
    }
  }
  std::string zk_path = "";
  zk_path = zk_provider_path + "/" + oservice.ip + ":" +
      boost::lexical_cast<std::string>(oservice.port);

  std::string strJson = "";
  zk_exists_req->path = zk_path;
  zk_exists_req->watch = 0;

  ret = ZkClientPool::GetInstance()->GetZkClientForHash(kZkClientIndex)->ZkExists(zk_exists_req);
  if (ZNONODE == ret) {
    ret =  RegistryCreateZkNode(oservice, zk_path, regCmd, uptCmd);
    if (SUCCESS != ret) {
      NS_LOG_ERROR("create zk node failed,zk_path: " << zk_path);
      return ret;
    }
  } else if (ZOK == ret) {
    ret = RegistryUpdateZkNode(oservice, zk_path, regCmd, uptCmd);
    if (SUCCESS != ret) {
      NS_LOG_ERROR("update zk node failed,zk_path: " << zk_path);
      return ret;
    }
  } else if (ZNOAUTH == ret) {
    NS_LOG_INFO("ZNOAUTH in registerService. ret = " << ret);
    return ret;
  } else {
    NS_LOG_INFO("ERR other error: " << ret << " in registerService");
    return ret;
  }
  ret = UpdateLastModifiedTime(oservice.appkey, zk_provider_path);
  if (SUCCESS != ret) {
    NS_LOG_ERROR("last modified time updated failed");
    return ret;
  }
  if (RegistCmd::REGIST == regCmd) {
    ret = RegistryServiceNameToZk(oservice, uptCmd);
    if (SUCCESS != ret) {
      NS_LOG_ERROR("failed to register servname, ret =" << ret);
      return ret;
    }
  }
  return SUCCESS;
}
int32_t RegistryService::RegistryServiceNameCreate(const std::string&
service_name,const std::string& path){
  int32_t ret = FAILURE;
  std::string service_json = "";
  ret = JsonZkMgr::ServiceNameNode2Json(service_name, service_json);
  if (0 != ret) {
    return ret;
  }
  NS_LOG_INFO("to create serviceName when node not exit"
                    << ", zkPath: " << path
                    << ", servicename : " << service_name);
  ZkCreateRequestPtr zk_create_req = boost::make_shared<ZkCreateRequest>();
  zk_create_req->path = path;
  zk_create_req->value = service_json;
  zk_create_req->value_len = service_json.size();
  ret = ZkClientPool::GetInstance()->GetZkClientForHash(kZkClientIndex)->ZkCreate(zk_create_req);
  if (ZOK != ret || ret == -1) {
    NS_LOG_ERROR("WARN zoo_create failed zkPath:" << path
                                                    << ", zValue:" << service_json.c_str()
                                                    << ", zValue size:" << service_json.size()
                                                    << ", ret:" << ret);
    return ret;
  }
  return ret;
}
int32_t RegistryService::RegistryServiceNameNodeCreate(const SGService&
oservice, const std::string& service_name,const std::string& path){
  int32_t ret = SUCCESS;
  std::string str_json = "";
  ServiceNode oTmp;
  oTmp.serviceName = service_name;
  oTmp.appkeys.insert(oservice.appkey);
  oTmp.lastUpdateTime = static_cast<int32_t>(time(0));

  JsonZkMgr::ServiceNode2Json(oTmp, str_json);

  NS_LOG_INFO("to regist when node not exit"
                    << ", serviceName: " << service_name
                    << ", appkey : " << oservice.appkey);
  ZkCreateRequestPtr zk_create_req = boost::make_shared<ZkCreateRequest>();
  zk_create_req->path = path;
  zk_create_req->value = str_json;
  zk_create_req->value_len = str_json.size();
  ret = ZkClientPool::GetInstance()->GetZkClientForHash(kZkClientIndex)->
      ZkCreate(zk_create_req);
  if (ZOK != ret || ret == -1) {
    NS_LOG_WARN("WARN zoo_create failed zkPath:" << path
                                                   << ", zValue:" << str_json.c_str()
                                                   << ", zValue size:" << str_json.size()
                                                   << ", ret:" << ret);
    return ret;
  }
  return ret;

}
int32_t RegistryService::UpdateServiceNameNodeTime(const SGService&
oservice,const std::string& path){
  int32_t ret = SUCCESS;
  int data_len = kZkContentSize;
  std::string str_org_json = "";
  struct Stat stat;

  ZkGetRequestPtr zk_get_req = boost::make_shared<ZkGetRequest>();
  ZkGetResponsePtr zk_get_res = boost::make_shared<ZkGetResponse>();
  zk_get_req->path = path;
  zk_get_req->watch = 0;
  ret = ZkClientPool::GetInstance()->GetZkClientForHash(kZkClientIndex)->
      ZkGet(zk_get_req, zk_get_res);
  if (ZOK != ret) {
    NS_LOG_WARN("zoo_get origin content fail or zk handle is null, ret: "
                      << ret
                      << ", zkPath: " << path);
    return ret;
  }
  str_org_json = zk_get_res->buffer;
  data_len = zk_get_res->buffer_len;
  stat = zk_get_res->stat;

  ServiceNode org_service;
  ret = JsonZkMgr::Json2ServiceNode(str_org_json, org_service);
  if (ret != 0) {
    NS_LOG_ERROR("Json2ServiceNode failed! strJson = " << str_org_json
                                                         << "ret = " << ret);
    return ret;
  }
  if (org_service.appkeys.end() != org_service.appkeys.find(oservice.appkey)) {//防止重复注册
    NS_LOG_WARN("appkey already exists, don't need to register appkey:"
                      << oservice.appkey);
    return SUCCESS;
  }
  //reset oService last_update_time
  org_service.lastUpdateTime = static_cast<int32_t>(time(0));
  org_service.appkeys.insert(oservice.appkey);

  str_org_json = "";
  //NOTICE: sg2Service always return 0
  if (JsonZkMgr::ServiceNode2Json(org_service, str_org_json) < 0) {
    NS_LOG_ERROR("ServiceName2Json failed");
    return -1;
  }

  NS_LOG_INFO("to regist when node exists"
                    << ", serviceName : " << org_service.serviceName
                    << ", appkey : " << oservice.appkey);
  ZkSetRequestPtr zk_set_req = boost::make_shared<ZkSetRequest>();
  zk_set_req->path = path;
  zk_set_req->buffer = str_org_json;
  zk_set_req->version = stat.version;   //set的版本号设置为get获取的版本号，不再强制覆盖
  ret = ZkClientPool::GetInstance()->GetZkClientForHash(kZkClientIndex)->
      ZkSet(zk_set_req);
  if (ZOK != ret || ret == -1) {
    NS_LOG_ERROR("ERR zoo_set failed zkPath:"
                       << path
                       << " szValue:" << str_org_json
                       << " szValue size:" << str_org_json.size()
                       << "ret:" << ret);
    return ret;
  }
  return ret;
}
int32_t RegistryService::RegistryServiceNameToZk(const SGService &oservice,
                                                int uptCmd) {
  if (0 == oservice.serviceInfo.size()) {
    NS_LOG_DEBUG("serviceinfo is empty");
    return SUCCESS;
  }
  for (std::map<std::string, ServiceDetail>::const_iterator iter =
      oservice.serviceInfo.begin(); iter != oservice.serviceInfo.end(); ++iter) {
    std::string zk_service_path = "";
    std::string service_name = iter->first;
    int32_t ret = agent_path_.GenServiceNameZkPathNode(zk_service_path,
                                                   service_name, oservice.appkey);
    if (SUCCESS != ret) {
      return ret;
    }
    NS_LOG_INFO("_gen serviceNamePath: " << zk_service_path);
    ZkExistsRequestPtr zk_exists_req =
        boost::make_shared<ZkExistsRequest>();
    zk_exists_req->path = zk_service_path;
    zk_exists_req->watch = 0;
    ret = ZkClientPool::GetInstance()->GetZkClientForHash(kZkClientIndex)->
        ZkExists(zk_exists_req);
    if (ZNONODE == ret || ret == -1) {
      NS_LOG_ERROR("servicename not exist.zkpath:"
                        << zk_service_path);
      if(SUCCESS != RegistryServiceNameCreate(service_name,
                                                  zk_service_path)){
        NS_LOG_ERROR("servicename create failed, zkpath:"
                         << zk_service_path);
      }

    } else if (ZNOAUTH == ret) {
      NS_LOG_ERROR("ZNOAUTH in serviceName. ret = " << ret);
      return ret;
    }
    std::string zkPath = "";
    ret = agent_path_.GenServiceNameZkPathNode(zkPath, service_name,
                                               oservice.protocol, oservice.appkey);
    if (SUCCESS != ret) {
      return ret;
    }
    NS_LOG_INFO("_gen serviceNamePathNode: " << zkPath);
    zk_exists_req->path = zkPath;
    zk_exists_req->watch = 0;
    ret = ZkClientPool::GetInstance()->GetZkClientForHash(kZkClientIndex)->
        ZkExists(zk_exists_req);
    if (ZNONODE == ret) {
      if(SUCCESS != RegistryServiceNameNodeCreate(oservice,service_name,
                                                  zk_service_path)){
        NS_LOG_ERROR("servicename create failed, zkpath:"
                         << zk_service_path);
        return FAILURE;
      }
    } else if (ZOK == ret) {
      if(SUCCESS != UpdateServiceNameNodeTime(oservice, zk_service_path)){
        NS_LOG_ERROR("servicename create failed, zkpath:"
                         << zk_service_path);
        return FAILURE;
      }
    } else if (ZNOAUTH == ret) {
      NS_LOG_ERROR("ZNOAUTH in registerService. ret = " << ret);
      return ret;
    } else {
      NS_LOG_ERROR("ERR other error: " << ret << " in registerService");
      return ret;
    }
  }

  return SUCCESS;
}
int32_t RegistryService::RegistryUpdateZkNode(const SGService& agent_service,
                                              const std::string &zk_path,
                                              RegistCmd::type regCmd,
                                              int uptCmd) {
  int32_t ret = FAILURE;
  int32_t data_len = kZkContentSize;
  std::string str_json = "";
  struct Stat stat;

  ZkGetRequestPtr zk_get_req(new ZkGetRequest());
  ZkGetResponsePtr zk_get_res(new ZkGetResponse());
  zk_get_req->path = zk_path;
  zk_get_req->watch = 0;
  ret = ZkClientPool::GetInstance()->GetZkClientForHash(kZkClientIndex)->
      ZkGet(zk_get_req, zk_get_res);
  if (ZOK != ret) {
    NS_LOG_INFO("zoo_get origin content fail or zk handle is null, ret: "
                    << ret << ", zkPath: " << zk_path);
    return ret;
  }
  str_json = zk_get_res->buffer;
  data_len = zk_get_res->buffer_len;
  stat = zk_get_res->stat;
  SGService org_service;
  ret = JsonZkMgr::Json2SGService(str_json, org_service);
  if (ret != 0) {
    NS_LOG_INFO("Json2SGService failed! strJson = " << str_json
                                                      << "ret = " << ret);
    return ret;
  }
  if(register_strategy_.IsRepeatedRegister(org_service,agent_service)){
    NS_LOG_WARN("Repeated to register , registerinfo : appkey = "
                      << org_service.appkey
                      << ", protocol = " << org_service.protocol
                      << ", uptCmd : " << uptCmd
                      << ", env : " << org_service.envir
                      << ", status : " << org_service.status
                      << "; fweight : " << org_service.fweight
                      << "; protocol: " << org_service.protocol
                      << ", ip : " << org_service.ip);
    return ERR_REPEATED_REGISTER;
  }
  if (UptCmd::ADD != static_cast<UptCmd::type>(uptCmd)) {
    if (fb_status::STOPPED == org_service.status
        || fb_status::STARTING == org_service.status) {
      std::string status_str =
          fb_status::STOPPED == org_service.status ? "STOPPED" : "STARTING";
      NS_LOG_INFO("the zk node status is " << status_str
                                             << ", don't change its status, "
                                                 "appkey = "
                                             << org_service.appkey
                                             << ", ip = " << agent_service.ip
                                             << ", port = "
                                             << agent_service.port);
    } else {
      NS_LOG_INFO("the zk node status(" << org_service.status <<
                                          ") is not equals to the one which is "
                                              "defined by user, use the later("
                                          << agent_service.status << ")");
      org_service.status = agent_service.status;
    }
  }

  org_service.lastUpdateTime = time(0);
  ResetRegistryServiceInfo(org_service, agent_service, regCmd, uptCmd);
  str_json = "";
  if (JsonZkMgr::SGService2Json(org_service,
                                str_json,
                                CXmlFile::GetAppenv()->GetIntEnv()) < 0) {
    NS_LOG_INFO("_SGService2Json failed");
    return FAILURE;
  }
  NS_LOG_INFO("to regist when node exists"
                    << ", uptCmd : " << uptCmd
                    << ", appkey : " << org_service.appkey
                    << ", env : " << org_service.envir
                    << ", local env : " << CXmlFile::GetAppenv()->GetIntEnv()
                    << ", status : " << org_service.status
                    << "; fweight : " << org_service.fweight
                    << "; serverType : " << org_service.serverType
                    << "; protocol: " << org_service.protocol
                    << ", ip : " << org_service.ip);
  ZkSetRequestPtr zk_set_req(new ZkSetRequest());
  zk_set_req->path = zk_path;
  zk_set_req->buffer = str_json;
  zk_set_req->version = stat.version;
  ret = ZkClientPool::GetInstance()->GetZkClientForHash(kZkClientIndex)->
      ZkSet(zk_set_req);
  if (ZOK != ret || ret == -1) {
    NS_LOG_INFO("ERR zoo_set failed zkPath:" << zk_path
                                             << " szValue:"
                                             << str_json.c_str()
                                             << " szValue size:"
                                             << str_json.size()
                                             << "ret:" << ret);
  }
  return ret;
}
int32_t RegistryService::RegistryCreateZkNode(const SGService& agent_service,
                                              const std::string &zk_path,
                                              RegistCmd::type regCmd,
                                              int uptCmd) {

  if (RegistCmd::UNREGIST == regCmd || UptCmd::DELETE == uptCmd) {
    // if the zk node don't exist, ignore unRegister
    NS_LOG_INFO("ignore unRegister,zk node don't exist"
                      << ", zkPath: " << zk_path
                      << ", ip : " << agent_service.ip
                      << ", regCmd: " << regCmd
                      << ", uptCmd: " << uptCmd);
    return ERR_NODE_NOTFIND;
  }
  int32_t ret = FAILURE;
  std::string json = "";
  SGService service_tmp = const_cast<SGService &>(agent_service);
  JsonZkMgr::SGService2Json(service_tmp, json,CXmlFile::GetAppenv()
      ->GetIntEnv());

  NS_LOG_INFO("register: create a new zk node"
                    << ", zkPath: " << zk_path
                    << ", appkey : " << service_tmp.appkey
                    << ", env : " << service_tmp.envir
                    << ", local env : " <<  CXmlFile::GetAppenv()->GetStrEnv()
                    << "; protocol : " << service_tmp.protocol
                    << ", ip : " << service_tmp.ip);
  ZkCreateRequestPtr zk_data_create(new ZkCreateRequest());
  zk_data_create->path = zk_path;
  zk_data_create->value = json;
  zk_data_create->value_len = json.size();
  ret = ZkClientPool::GetInstance()->GetZkClientForHash(kZkClientIndex)
      ->ZkCreate(zk_data_create);
  if (ZOK != ret || ret == FAILURE) {
    NS_LOG_INFO("WARN zoo_create failed zkPath:" << zk_path
                                                 << ", zValue:" << json.c_str()
                                                 << ", zValue size:"
                                                 << json.size()
                                                 << ", ret:" << ret);
    return ret;
  }
  return ret;
}
int32_t RegistryService::UpdateLastModifiedTime(const std::string &appkey,
                                                const std::string &zk_provider_path) {
  int32_t ret = FAILURE;

  CProviderNode oprovider;
  oprovider.appkey = appkey;
  oprovider.lastModifiedTime = time(NULL);

  std::string str_json = "";
  JsonZkMgr::ProviderNode2Json(oprovider, str_json);
  ZkSetRequestPtr zk_set_req = boost::shared_ptr<ZkSetRequest>(new ZkSetRequest());
  zk_set_req->path = zk_provider_path;
  zk_set_req->buffer = str_json;
  zk_set_req->version = -1;
  ret = ZkClientPool::GetInstance()->GetZkClientForHash(kZkClientIndex)
      ->ZkSet(zk_set_req);

  if (ZOK != ret || FAILURE == ret) {
    NS_LOG_INFO("ERR zoo_set provider failed zkPath:"
                      << zk_provider_path
                      << " szValue:" << str_json.c_str()
                      << " szValue size:" << str_json.size()
                      << "ret: " << ret);
    return ret;
  }
  return ret;
}
void RegistryService::ResetRegistryServiceInfo(SGService &org_service,
                                               const SGService &sg_service,
                                               RegistCmd::type reg_cmd,
                                               int upt_cmd) {
  if (RegistCmd::REGIST == reg_cmd) {
    org_service.version = sg_service.version;
    if (version_manager_.IsOldVersion(sg_service.version)){
      NS_LOG_INFO("version = " << org_service.version
                                 << ", need to clear serviceInfo");
      org_service.heartbeatSupport = HeartbeatUnSupport;
      org_service.serviceInfo.clear();
    } else{
      org_service.heartbeatSupport = sg_service.heartbeatSupport;
      ReEditServiceName(org_service, sg_service, upt_cmd);
      }
    }
    org_service.appkey = sg_service.appkey;
    org_service.ip = sg_service.ip;
    org_service.port = sg_service.port;
}
int32_t RegistryService::ReEditServiceName(SGService &desService,
                                           const SGService &srcService,
                                           int uptCmd) {
  switch (uptCmd) {
    case UptCmd::RESET:desService.serviceInfo = srcService.serviceInfo;
      break;
    case UptCmd::ADD:
      for (std::map<std::string, ServiceDetail>::const_iterator iter =
          srcService.serviceInfo.begin();
           iter != srcService.serviceInfo.end();
           ++iter) {
        desService.serviceInfo[iter->first] = iter->second;
      }
      break;
    case UptCmd::DELETE:
      for (std::map<std::string, ServiceDetail>::const_iterator iter =
          srcService.serviceInfo.begin();
           iter != srcService.serviceInfo.end();
           ++iter) {
        desService.serviceInfo.erase(iter->first);
      }
      break;
    default: NS_LOG_INFO("unknown uptCmd: " << uptCmd);
      return FAILURE;
  }
  return SUCCESS;
}
int32_t RegistryService::RegistryAction(RegParamPtr reg_service,
                                        const int32_t &reg_action) {

  switch (CXmlFile::GetI32Para(CXmlFile::RegistryCenter)){
    case kRegistryZk:{
      return RegistryActionToZk(reg_service,reg_action);
    }
    case kRegistryCache:{
      return RegistryActionToCache();
    }
    case kRegistryMixer:{
      return RegistryActionToMixer();
    }
    default:{
      NS_LOG_WARN("unsupported registry action "<<reg_action);
    }
  }
  return 0;
}
int32_t RegistryService::RegistryActionToZk(RegParamPtr reg_service,
                                            const int32_t &reg_action){

  std::string service_name = "";
  std::string unified_protocol = "";
  for (std::map<std::string, ServiceDetail>::const_iterator
           iter = reg_service->sgservice.serviceInfo.begin();
       iter != reg_service->sgservice.serviceInfo.end(); ++iter) {
    service_name += iter->first + " ";
    unified_protocol += iter->second.unifiedProto + " ";
  }
  NS_LOG_INFO("to register service to ZK"
                    << ", appkey : " << reg_service->sgservice.appkey
                    << ", ip : " << reg_service->sgservice.ip
                    << ", version : " << reg_service->sgservice.version
                    << ", port : " << reg_service->sgservice.port
                    << ", status : " << reg_service->sgservice.status
                    << "; fweight : " << reg_service->sgservice.fweight
                    << "; protocol: " << reg_service->sgservice.protocol
                    << "; serverType : " << reg_service->sgservice.serverType
                    << "; unifiedProto : " << unified_protocol
                    << "; regCmd : " << reg_action);
  int loop_times = 0;
  do {
    int32_t ret = RegistryServiceToZk(reg_service->sgservice,
                                      reg_service->regCmd,
                                      reg_service->uptCmd);
    if (SUCCESS == ret || ERR_ILLEGAL_REGISTE == ret || ERR_NODE_NOTFIND == ret) {
      return ret;
    } else {
      NS_LOG_INFO("retry to registry , appkey is : " << reg_service->sgservice.appkey);
      if (loop_times > retry_times_) {
        NS_LOG_INFO("register service to ZK fail! loop_times > reg_retry_, loop_times is : "
                          << loop_times
                          << ", appkey : " << reg_service->sgservice.appkey
                          << ", ip : " << reg_service->sgservice.ip
                          << ", port : " << reg_service->sgservice.port
                          << ", env : " << reg_service->sgservice.envir
                          << ", local env : " << CXmlFile::GetAppenv()->GetStrEnv()
                          << ", status : " << reg_service->sgservice.status
                          << "; fweight : " << reg_service->sgservice.fweight
                          << "; serverType : " << reg_service->sgservice.serverType
                          << "; protocol: " << reg_service->sgservice.protocol
                          << ", default retry times is : " << retry_times_);
        return ERR_REGIST_SERVICE_ZK_FAIL;
      }
    }
  } while (retry_times_ > loop_times++);
}

int32_t RegistryService::RegistryActionToCache(){

  return 0;
}

int32_t RegistryService::RegistryActionToMixer(){

  return 0;
}

}
