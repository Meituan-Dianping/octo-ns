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

#include "zk_path_tools.h"
#include "base_consts.h"
#include "base_errors_consts.h"
#include "base_mns_consts.h"
#include "log4cplus.h"
#include <boost/algorithm/string/trim.hpp>
#include "config_loader.h"

namespace meituan_mns {

const std::string provider_appkey = "/octo/nameservice";
const std::string provider_srvname = "/octo/service/";
const std::string ProtocolProviderThriftTail = "provider";
const std::string ProtocolRouteThriftTail = "route";
const std::string ProtocolProviderHttpTail = "provider-http";
const std::string ProtocolRouteHttpTail = "route-http";


int32_t AgentZkPath::GenNodeType(std::string &nodeType,
                                 const std::string &protocol) {
  //protocol变量
  if (protocol.empty()) {
    NS_LOG_ERROR("protocol cannot be empty.");
    return FAILURE;
  } else {
    //新接口访问,thrift或者http服务
    if ("http" == protocol) {
      nodeType += "-http";
    } else if ("thrift" == protocol) {
      NS_LOG_DEBUG("nodeType: " << nodeType << " not changed");
    } else {
      nodeType = nodeType + "s/" + protocol;
    }
    NS_LOG_DEBUG("nodeType in newInterface: " << nodeType);
  }
  return SUCCESS;
}

int32_t AgentZkPath::GenProtocolZkPath(std::string &path,
                                       const std::string &appkey,
                                       const std::string &protocol,
                                       std::string &node_type) {
  std::string current_env(CXmlFile::GetAppenv()->GetStrEnv());
  int32_t ret = GetEnvByAppkeyWhiteList(current_env, appkey);
  if (SUCCESS != ret) {
    return ret;
  }
  ret = GenNodeType(node_type, protocol);
  if (SUCCESS != ret) {
    return ret;
  }
  path = provider_appkey + "/" +current_env + "/" + appkey + "/" + node_type;
  return SUCCESS;
}

int32_t AgentZkPath::GenRegisterZkPath(std::string &path,
                                       const std::string &appkey,
                                       const std::string &protocol,
                                       const int serverType) {
  std::string current_env(CXmlFile::GetAppenv()->GetStrEnv());
  int ret = GetEnvByAppkeyWhiteList(current_env, appkey);
  if (SUCCESS != ret) {
    return ret;
  }
  //获取nodeType
  std::string providerType = "provider";
  if ((!protocol.empty())) {
    if ("http" == protocol) {
      providerType += "-http";
    } else if ("thrift" == protocol) {
      NS_LOG_DEBUG("thrift in newInterface: " << providerType);
    } else {
      providerType = providerType + "s/" + protocol;
    }
    NS_LOG_INFO("provider in newInterface: " << providerType);
  } else {
    //为了应对前端未修改protocol的情况
    if (HTTP_TYPE == serverType) {
      providerType += "-http";
    }
    NS_LOG_DEBUG("provider in oldInterface: " << providerType);
  }
  NS_LOG_INFO("zkPath provider prefix: " << providerType);

  path = provider_appkey  + "/" +current_env + "/" + appkey + "/" +
      providerType;

  return SUCCESS;
}

int32_t AgentZkPath::GenServiceNameZkPathNode(std::string &path,
                                          const std::string &service_name,
                                          const std::string &protocol,
                                          const std::string &appkey) {
  std::string current_env(CXmlFile::GetAppenv()->GetStrEnv());
  GetEnvByAppkeyWhiteList(current_env, appkey);
  std::string new_servicename = ReplaceHttpServiceName(service_name);

  //拼接zkPath
  path = provider_srvname + current_env + "/" + new_servicename + "/" + protocol;

  return SUCCESS;
}

std::string AgentZkPath::ReplaceHttpServiceName(const std::string &servicename) {
  std::string result;
  for (int i = 0; i < servicename.size(); ++i) {
    // replace the / with ^
    result.push_back('/' == servicename[i] ? '^' : servicename[i]);
  }
  return result;
}

int32_t AgentZkPath::GenDescZkPath(std::string &path,
                                   const std::string &appkey) {

  std::string current_env(CXmlFile::GetAppenv()->GetStrEnv());// 因为GetEnvByAppkeyWhiteList会修改第一个参数，因此必须使用临时变量
  int32_t ret = GetEnvByAppkeyWhiteList(current_env, appkey);
  if (SUCCESS != ret) {
    return ret;
  }

  path = provider_appkey + "/" + appkey + "/desc";

  return ret;
}
int32_t AgentZkPath::DeGenZkPath(const char *zkPath,
                                 std::string &appkey,
                                 std::string &protocol) {
  std::vector<std::string> pathList;
  int32_t size = SplitStringIntoVector(zkPath, "/", pathList);
  if (0 < size) {
    int length = pathList.size();
    //这里必须保证zkPath符合 /octo/naming/环境/appkey/(nodeType + protocol)
    if(4 < length) {
      //appkey在数组的下标是4，是因为pathList[0]是""
      appkey = pathList[4];
      if(appkey.empty()) {
        NS_LOG_ERROR("deGenZkPath appkey is empty! zkPath" << zkPath);
        return -3;
      }
      //protocol取最后一个，有可能是cellar, provider, provider-http
      protocol = pathList[length - 1];
      if(protocol.empty()) {
        NS_LOG_ERROR("deGenZkPath protocol is empty! zkPath" << zkPath);
        return -3;
      }
    }
    else {
      NS_LOG_ERROR("zkPath is not complete! zkPath: " << zkPath
                                                        << ", appkey: " << appkey
                                                        << ", length: " << length);
      return -2;
    }
  } else {
    NS_LOG_ERROR("zkPath is wroing! zkPath: " << zkPath << ", appkey:" << appkey);
    return -1;
  }
  int32_t ret = DeGenNoDeGenNodeType(protocol);
  if(SUCCESS != ret) {
    NS_LOG_ERROR("deGenNodeType is wrong!  protocol:" << protocol);
  }

  return ret;
}

int32_t AgentZkPath::DeGenNoDeGenNodeType(std::string &protocol) {
  if(protocol.empty()) {
    NS_LOG_ERROR("protocol in deGenNodeType is empty! ");
    return FAILURE;
  }
  if(0 == protocol.compare(ProtocolProviderHttpTail)
      || 0 == protocol.compare(ProtocolRouteHttpTail)) {
    protocol = "http";
  } else if (0 == protocol.compare(ProtocolProviderThriftTail)
      || 0 == protocol.compare(ProtocolRouteThriftTail)) {
    protocol = "thrift";
  } else {
    NS_LOG_INFO("when watcher trigger, in deGenNodeType, protocol:" << protocol);
  }
  return SUCCESS;
}


int32_t AgentZkPath::GetEnvByAppkeyWhiteList(std::string &env_str,
                                             const std::string &appkey) {
  if (appkey.empty()) {
    NS_LOG_WARN("appkey cannot be empty.");
    return FAILURE;
  }

  if (whitelist_manager_.IsAppkeyInAllEnvWhitList(appkey)) {
    //在白名单内的appkey, 环境强制改成prod
    NS_LOG_INFO("the appkey: " << appkey << ", current the env is: " << env_str << " change to prod");
    env_str = "prod";
  }
  return SUCCESS;
}

std::string AgentZkPath::GenRoutePath(const std::string &zk_path,
                                      const std::string &data_path) {
  return zk_path + "/" + data_path;
}
}  //  namespace meituan_mns

