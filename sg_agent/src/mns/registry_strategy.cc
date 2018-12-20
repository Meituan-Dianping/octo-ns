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

#include "inc_comm.h"
#include "mnsc_client.h"
#include "base_errors_consts.h"
#include "config_loader.h"
#include "route_info.h"
#include "registry_strategy.h"
#include "base_consts.h"
#include "base_mns_consts.h"
#include "zk_client_pool.h"
#include <vector>


namespace meituan_mns {


int32_t RegistryStrategy::CheckArgs(const SGService &oservice) {
  std::string appkey = oservice.appkey;
  if (appkey.empty()) {
    NS_LOG_ERROR("fail to register, because the appkey is empty.");
    return ERR_EMPTY_APPKEY;
  } else if (!IsAppkeyLegal(appkey)) {
    NS_LOG_ERROR("Invalid appkey in regist, appkey = " << appkey);
    return ERR_INVALIDAPPKEY;
  }

  if (!(0 <= oservice.weight && 100 >= oservice.weight)) {
    NS_LOG_ERROR("invalid weight: " << oservice.weight
                                      << ", appkey: " << oservice.appkey
                                      << ", ip: " << oservice.ip
                                      << ", port: " << oservice.port);
    return ERR_INVALID_WEIGHT;
  }

  if (!(0.0001 <= oservice.fweight && 100.0 - oservice.fweight >= 0.1)) {
    NS_LOG_ERROR("invalid fweight: " << oservice.fweight
                                      << ", appkey: " << oservice.appkey
                                      << ", ip: " << oservice.ip
                                      << ", port: " << oservice.port);
    return ERR_INVALID_WEIGHT;
  }

  if (!IsIpAndPortLegal(oservice.ip, oservice.port)) {
    NS_LOG_ERROR("invalid port: " << oservice.port
                                    << ", appkey: " << oservice.appkey
                                    << ", ip: " << oservice.ip
                                    << ", weight: " << oservice.weight);
    return ERR_INVALID_PORT;
  }
  return SUCCESS;
}

bool RegistryStrategy::IsAllowMacRegistry(const SGService &oservice) {
  NS_LOG_INFO("RegistryStrategy the registry appkey is = "
                    << oservice.appkey << ";env = "
                    << CXmlFile::GetAppenv()->GetTypeEnv());

  if (PPE != CXmlFile::GetAppenv()->GetTypeEnv() &&
      TEST != CXmlFile::GetAppenv()->GetTypeEnv()) {
    return true;
  }
  if (!whitelist_manager_.IsMacRegisterAppkey(oservice.appkey)) {
    return true;
  }
  std::vector<boost::shared_ptr<IDC> >
      ::const_iterator iter = CXmlFile::GetRegisterIdc()->begin();
  for (; CXmlFile::GetRegisterIdc()->end() != iter; ++iter) {
    if ((*iter)->IsSameIdc(oservice.ip)) {
      return true;
    }
  }
  return false;
}

int32_t RegistryStrategy::CheckAllowedProtocolRegistry(SGServicePtr &oservice) {
  if (oservice->protocol.empty()) {
    if (THRIFT_TYPE == oservice->serverType) {
      oservice->protocol = "thrift";
    } else if (HTTP_TYPE == oservice->serverType) {
      oservice->protocol = "http";
    } else {
      NS_LOG_ERROR("Appkey: " << oservice->appkey
                                << " serverType: " << oservice->serverType
                                << " is wrong!");
      return FAILURE;
    }
    NS_LOG_WARN("the original protocol defined by user is empty, "
                    "now identify the protocol="
                      << oservice->protocol
                      << " based on serverType="
                      << oservice->serverType);
  }
  return SUCCESS;

}
int32_t RegistryStrategy::IsAllowedRegistry(const SGService &oService) {
  int32_t ret = CheckArgs(oService);
  if (SUCCESS != ret) {
    NS_LOG_INFO("its not allowed to registry");
    return ret;
  }
  if (!IsAllowMacRegistry(oService)) {
    NS_LOG_INFO("hotel appkey and the ip is not in the special idc,"
                      " not allow to register service to ZK , appkey = "
                      << oService.appkey
                      << "; version = " << oService.version
                      << "; ip = " << oService.ip
                      << "; port = " << oService.port
                      << "; weight = " << oService.weight
                      << "; status = " << oService.status
                      << "; role = " << oService.role
                      << "; envir = " << oService.envir
                      << "; fweight = " << oService.fweight
                      << "; lastUpdateTime = " << oService.lastUpdateTime
                      << "; protocol = " << oService.protocol
                      << "; serverType = " << oService.serverType);

    return FAILURE;
  }
  if (!RegistryFilterCheck(oService)) {
    NS_LOG_ERROR("appkey is not legal to regist in this ip, appkey = "
                       << oService.appkey);
    return ERR_ILLEGAL_REGISTE;
  }
  return ret;
}

bool RegistryStrategy::RegistryFilterCheck(const SGService &oservice){
  if (whitelist_manager_.IsAppkeyInRegistUnlimitWhitList(oservice.appkey)) {
    NS_LOG_INFO("appkey: " << oservice.appkey
                             << " is in regist unlimit white list");
    return true;
  }
  if (IsLimitOnZk(oservice)) {
    return CheckLegalOnOps(oservice);
  }
  return true;

}

bool RegistryStrategy::CheckLegalOnOps(const SGService &service) {
  std::ifstream fi(kServiceAppkeyPath.c_str());
  if (!fi.is_open()) {
    NS_LOG_INFO("faild to open file: " << kServiceAppkeyPath);
    fi.close();
    return true;
  }

  std::string appkey;
  while (std::getline(fi, appkey)) {
    NS_LOG_INFO("appkey: " << appkey);
    if (appkey == service.appkey) {
      fi.close();
      return true;
    }
  }

  fi.close();
  NS_LOG_INFO(service.appkey << " is not in appkeys");
  return false;

}
bool RegistryStrategy::IsLimitOnZk(const SGService &service) {
  bool res = false;
  std::string zk_desc_path = "";
  int ret = agent_path_.GenDescZkPath(zk_desc_path, service.appkey);

  std::string str_json = "";
  ZkExistsRequestPtr zk_exists_req(new ZkExistsRequest());
  zk_exists_req->path = zk_desc_path;
  zk_exists_req->watch = 0;
  ret = ZkClientPool::GetInstance()->GetZkClientForHash(kZkClientIndex)->
      ZkExists(zk_exists_req);

  if (ZNONODE == ret || ret == -1) {
    NS_LOG_ERROR("appkey desc is not exists, appkey = "
                       << service.appkey
                       << ", env = " << CXmlFile::GetAppenv()->GetStrEnv());
    return false; //默认反馈合法
  }else if (ZOK == ret) { // 节点存在， 不需要进行合法验证
    int datalen = kZkContentSize;
    struct Stat stat;
    ZkGetRequestPtr zk_get_req = boost::shared_ptr<ZkGetRequest>(new ZkGetRequest());
    ZkGetResponsePtr zk_get_res = boost::shared_ptr<ZkGetResponse>(new ZkGetResponse());
    zk_get_req->path = zk_desc_path;
    zk_get_req->watch = 0;
    ret = ZkClientPool::GetInstance()->GetZkClientForHash(kZkClientIndex)->
        ZkGet(zk_get_req, zk_get_res);
    if (ZOK != ret) {
      NS_LOG_INFO("zoo_get origin content fail or zk handle is null, ret: "
                        << ret
                        << ", zkPath: " << zk_desc_path);
      return ret;
    }
    str_json = zk_get_res->buffer;
    datalen = zk_get_res->buffer_len;
    stat = zk_get_res->stat;

    cJSON *root = cJSON_Parse(str_json.c_str());
    if (NULL == root) {
      NS_LOG_ERROR("failed to parse json: " << str_json);
      return false;
    }

    cJSON *pItem = cJSON_GetObjectItem(root, "regLimit");
    if (pItem) {
      int reg_limit = pItem->valueint;
      if (reg_limit == REG_LIMIT_LEGAL) {
        res = true;
      }
    }
    cJSON_Delete(root);
  }
  NS_LOG_INFO("check appkey: " << service.appkey << " is reglimit: " << res);
  return res;
}
//  去除lastupdatetime的判断
bool RegistryStrategy::IsRepeatedRegister(const SGService& orgservice,
                                          const SGService& zkservice) {
  if (!(orgservice.appkey == zkservice.appkey))
    return false;
  if (!(orgservice.version == zkservice.version))
    return false;
  if (!(orgservice.ip == zkservice.ip))
    return false;
  if (!(orgservice.port == zkservice.port))
    return false;
  if (!(orgservice.weight == zkservice.weight))
    return false;
  if (!(orgservice.status == zkservice.status))
    return false;
  if (!(orgservice.role == zkservice.role))
    return false;
  if (!(orgservice.envir == zkservice.envir))
    return false;
  if (!(orgservice.fweight == zkservice.fweight))
    return false;
  if (!(orgservice.serverType == zkservice.serverType))
    return false;
  if (!(orgservice.protocol == zkservice.protocol))
    return false;
  if (!(orgservice.serviceInfo == zkservice.serviceInfo))
    return false;
  if (!(orgservice.heartbeatSupport == zkservice.heartbeatSupport))
    return false;
  return true;
}

}
