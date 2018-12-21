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

#include <unistd.h>
#include <map>
#include <sstream>

#include "json_data_tools.h"
#include "comm/inc_comm.h"

namespace meituan_mns {

#define CHECK_JSON(args) \
    if(!((args.c_str()[0] == '{' && args.c_str()[args.size() - 1] == '}') || (args.c_str()[0] == '[' && args.c_str()[args.size() -1] == ']')))\
    { NS_LOG_ERROR( "json formate not correct, should start with { and end with }");return -1;}\

int JsonZkMgr::ProviderNode2Json(const CProviderNode &oprovider, std::string &strJson) {
  cJSON *root;
  char *out;
  root = cJSON_CreateObject();
  cJSON_AddItemToObject(root, "appkey", cJSON_CreateString(oprovider.appkey.c_str()));
  cJSON_AddNumberToObject(root, "lastUpdateTime", oprovider.lastModifiedTime);

  out = cJSON_Print(root);
  strJson = out;

  SAFE_FREE(out);
  cJSON_Delete(root);

  return 0;
}

int JsonZkMgr::ServiceNameNode2Json(
    const std::string &serviceName,
    std::string &strJson) {
  cJSON *root;
  char *out;
  root = cJSON_CreateObject();
  if (NULL == root) {
    NS_LOG_ERROR("failed to cJSON Create Object");
    return 0;
  }

  cJSON_AddItemToObject(root, "serviceName", cJSON_CreateString(serviceName.c_str()));
  cJSON_AddNumberToObject(root, "lastUpdateTime", time(0));

  out = cJSON_Print(root);
  strJson = out;

  SAFE_FREE(out);
  cJSON_Delete(root);

  return 0;
}

int JsonZkMgr::Json2ProviderNode(const std::string &strJson,
                                 unsigned long mtime,
                                 unsigned long version,
                                 unsigned long cversion,
                                 CProviderNode &oprovider) {
  CHECK_JSON(strJson)
  cJSON *root = cJSON_Parse(strJson.c_str());
  if (NULL == root) {
    return ERR_JSON_TO_DATA_FAIL;
  }
  cJSON *pProviderAppkeyItem = cJSON_GetObjectItem(root, "appkey");
  if (NULL != pProviderAppkeyItem) {
    oprovider.appkey = pProviderAppkeyItem->valuestring;
  } else {
    NS_LOG_WARN("the provider node appkey is null");
  }
  oprovider.mtime = mtime;
  oprovider.cversion = cversion;
  oprovider.version = version;
  cJSON *pModifiedTimeItem = cJSON_GetObjectItem(root, "lastUpdateTime");
  if (NULL != pModifiedTimeItem) {
    oprovider.lastModifiedTime = pModifiedTimeItem->valueint;
  } else {
    NS_LOG_WARN("the provider node lastUpdateTime is null");
  }

  cJSON_Delete(root);
  return 0;
}

int JsonZkMgr::Json2RouteNode(const std::string &strJson,
                              unsigned long mtime,
                              unsigned long version,
                              unsigned long cversion,
                              CRouteNode &oroute) {
  CHECK_JSON(strJson)
  cJSON *root = cJSON_Parse(strJson.c_str());
  if (NULL == root) {
    return ERR_JSON_TO_DATA_FAIL;
  }
  cJSON *pAppkeyItem = cJSON_GetObjectItem(root, "appkey");
  if (NULL != pAppkeyItem) {
    oroute.appkey = pAppkeyItem->valuestring;
  } else {
    NS_LOG_WARN("the route node appkey is null");
  }
  oroute.mtime = mtime;
  oroute.cversion = cversion;
  oroute.version = version;
  cJSON *pModifiedTimeItem = cJSON_GetObjectItem(root, "lastUpdateTime");

  if (NULL != pModifiedTimeItem) {
    oroute.lastModifiedTime = pModifiedTimeItem->valueint;
  } else {
    NS_LOG_WARN("the route node lastModifiedTime is null");
  }
  cJSON_Delete(root);
  return 0;
}

int JsonZkMgr::SGService2Json(const SGService &oservice, std::string &strJson, const int env_int) {
  cJSON *root;
  char *out;
  root = cJSON_CreateObject();
  if (NULL == root) {
    NS_LOG_ERROR("failed to create root Object");
    return FAILURE;
  }

  cJSON_AddItemToObject(root, "appkey", cJSON_CreateString(oservice.appkey.c_str()));
  cJSON_AddItemToObject(root, "version", cJSON_CreateString(oservice.version.c_str()));
  cJSON_AddItemToObject(root, "ip", cJSON_CreateString(oservice.ip.c_str()));
  cJSON_AddNumberToObject(root, "port", oservice.port);
  cJSON_AddNumberToObject(root, "weight", oservice.weight);
  cJSON_AddNumberToObject(root, "status", oservice.status);
  cJSON_AddNumberToObject(root, "role", oservice.role);
  cJSON_AddNumberToObject(root, "env", env_int);
  cJSON_AddNumberToObject(root, "lastUpdateTime", oservice.lastUpdateTime);
  cJSON_AddNumberToObject(root, "fweight", oservice.fweight);
  cJSON_AddNumberToObject(root, "serverType", oservice.serverType);
  cJSON_AddNumberToObject(root, "warmup", oservice.warmup);

  int32_t heartbeatSupport = oservice.heartbeatSupport;
  cJSON_AddNumberToObject(root, "heartbeatSupport", heartbeatSupport);
  cJSON_AddItemToObject(root, "protocol", cJSON_CreateString(oservice.protocol.c_str()));

  cJSON *item = cJSON_CreateObject();
  if (NULL == item) {
    NS_LOG_ERROR("cJson failed to CreateObject. Item is serviceInfo");
    return FAILURE;
  }
  int ret = cJson_AddServiceObject(oservice.serviceInfo, root,
                                   item, std::string("serviceInfo"));
  if (0 != ret) {
    NS_LOG_ERROR("failed to add serviceName to root");
    return FAILURE;
  }


  //cJSON_AddItemToObject(root, "unifiedProto", cJSON_CreateBool(oservice.unifiedProto));

  out = cJSON_Print(root);
  strJson = out;
  NS_LOG_INFO("out: " << strJson);

  SAFE_FREE(out);
  cJSON_Delete(root);

  return SUCCESS;
}

int JsonZkMgr::ServiceNode2Json(const ServiceNode &oservice, std::string &strJson) {
  cJSON *root;
  char *out;
  root = cJSON_CreateObject();
  if (NULL == root) {
    NS_LOG_ERROR("failed to create root Object");
    return -1;
  }

  cJSON_AddItemToObject(root, "serviceName", cJSON_CreateString(oservice.serviceName.c_str()));
  cJSON_AddNumberToObject(root, "lastUpdateTime", oservice.lastUpdateTime);

  cJSON *arrayItem = cJSON_CreateArray();
  int ret = cJson_AddStringArray(oservice.appkeys, root,
                                 arrayItem, std::string("appkey"));
  if (0 != ret) {
    NS_LOG_ERROR("failed to add appkey to root");
    return -1;
  }

  out = cJSON_Print(root);
  strJson = out;
  NS_LOG_INFO("out: " << strJson);

  SAFE_FREE(out);
  cJSON_Delete(root);

  return 0;
}

int JsonZkMgr::Json2ServiceNode(const std::string &strJson,
                                ServiceNode &oservice) {
  cJSON *root = cJSON_Parse(strJson.c_str());
  if (NULL == root) {
    return ERR_JSON_TO_DATA_FAIL;
  }

  cJSON *pItem = cJSON_GetObjectItem(root, "serviceName");
  if (pItem) {
    oservice.serviceName = pItem->valuestring;
  }
  pItem = cJSON_GetObjectItem(root, "lastUpdateTime");
  if (pItem) {
    oservice.lastUpdateTime = pItem->valueint;
  }
  pItem = cJSON_GetObjectItem(root, "appkey");
  if (pItem) {
    cJSON *appkeys = pItem;
    //解析serviceName
    std::string name;
    int nCount = cJSON_GetArraySize(appkeys);
    for (int i = 0; i < nCount; i++) {
      cJSON *pAppkeyItem = cJSON_GetArrayItem(appkeys, i);
      if (NULL != pAppkeyItem) {
        name = pAppkeyItem->valuestring;
        oservice.appkeys.insert(name);
      } else {
        NS_LOG_WARN("json2service node, the appkey is null");
      }
    }
  }

  cJSON_Delete(root);
  return 0;
}
int JsonZkMgr::Json2RouteData(const std::string &strJson, CRouteData &orouteData) {
  cJSON *root = cJSON_Parse(strJson.c_str());
  if (NULL == root) {
    return ERR_JSON_TO_DATA_FAIL;
  }

  cJSON *pItem = cJSON_GetObjectItem(root, "id");
  if (pItem) {
    orouteData.id = pItem->valuestring;
  }
  pItem = cJSON_GetObjectItem(root, "name");
  if (pItem) {
    orouteData.name = pItem->valuestring;
  }
  pItem = cJSON_GetObjectItem(root, "appkey");
  if (pItem) {
    orouteData.appkey = pItem->valuestring;
  }
  pItem = cJSON_GetObjectItem(root, "env");
  if (pItem) {
    orouteData.env = pItem->valueint;
  }
  pItem = cJSON_GetObjectItem(root, "category");
  if (pItem) {
    orouteData.category = pItem->valueint;
  }
  pItem = cJSON_GetObjectItem(root, "priority");
  if (pItem) {
    orouteData.priority = pItem->valueint;
  }
  pItem = cJSON_GetObjectItem(root, "status");
  if (pItem) {
    orouteData.status = pItem->valueint;
  }
  pItem = cJSON_GetObjectItem(root, "updateTime");
  if (pItem) {
    orouteData.updateTime = pItem->valueint;
  }
  pItem = cJSON_GetObjectItem(root, "createTime");
  if (pItem) {
    orouteData.createTime = pItem->valueint;
  }
  pItem = cJSON_GetObjectItem(root, "reserved");
  if (pItem) {
    orouteData.reserved = pItem->valuestring;
  }

  //解析consumer
  int i = 0;
  int nCount = 0;
  cJSON *consumer = cJSON_GetObjectItem(root, "consumer");
  if (consumer) {
    cJSON *ips = cJSON_GetObjectItem(consumer, "ips");
    cJSON *appkeys = cJSON_GetObjectItem(consumer, "appkeys");
    //解析consumer ips
    std::vector<std::string> ipList;
    std::string oip;
    nCount = cJSON_GetArraySize(ips);
    for (i = 0; i < nCount; i++) {
      cJSON *item = cJSON_GetArrayItem(ips, i);
      char *ip = cJSON_Print(item);
      oip = ip;
      ipList.push_back(oip);
      SAFE_FREE(ip);
    }

    //解析consumer appkeys
    std::vector<std::string> appkeyList;
    std::string oappkey;
    nCount = cJSON_GetArraySize(appkeys);
    for (i = 0; i < nCount; i++) {
      cJSON *item = cJSON_GetArrayItem(appkeys, i);
      char *appkey = cJSON_Print(item);
      oappkey = appkey;
      appkeyList.push_back(oappkey);
      SAFE_FREE(appkey);

    }
    //组装解析consumer
    Consumer newConsumer;
    newConsumer.ips = ipList;
    newConsumer.appkeys = appkeyList;
    orouteData.consumer = newConsumer;
  }

  //解析provider
  std::vector<std::string> pList;
  std::string oprovider;
  cJSON *provider = cJSON_GetObjectItem(root, "provider");
  nCount = cJSON_GetArraySize(provider);
  for (i = 0; i < nCount; i++) {
    cJSON *item = cJSON_GetArrayItem(provider, i);
    char *ip = cJSON_Print(item);
    oprovider = ip;
    pList.push_back(oprovider);
    SAFE_FREE(ip);
  }
  orouteData.provider = pList;

  cJSON_Delete(root);
  return 0;
}

int JsonZkMgr::Json2SGService(const std::string &strJson, SGService &oservice) {
  cJSON *root = cJSON_Parse(strJson.c_str());
  if (NULL == root) {
    return ERR_JSON_TO_DATA_FAIL;
  }

  cJSON *pItem = cJSON_GetObjectItem(root, "appkey");
  if (pItem) {
    oservice.appkey = pItem->valuestring;
  }
  pItem = cJSON_GetObjectItem(root, "version");
  if (pItem) {
    oservice.version = pItem->valuestring;
  }
  pItem = cJSON_GetObjectItem(root, "ip");
  if (pItem) {
    oservice.ip = pItem->valuestring;
  }
  pItem = cJSON_GetObjectItem(root, "port");
  if (pItem) {
    oservice.port = pItem->valueint;
  }
  pItem = cJSON_GetObjectItem(root, "weight");
  if (pItem) {
    oservice.weight = pItem->valueint;
  }
  pItem = cJSON_GetObjectItem(root, "status");
  if (pItem) {
    oservice.status = pItem->valueint;
  }
  pItem = cJSON_GetObjectItem(root, "role");
  if (pItem) {
    oservice.role = pItem->valueint;
  }
  pItem = cJSON_GetObjectItem(root, "env");
  if (pItem) {
    oservice.envir = pItem->valueint;
  }
  pItem = cJSON_GetObjectItem(root, "lastUpdateTime");
  if (pItem) {
    oservice.lastUpdateTime = pItem->valueint;
  }
  pItem = cJSON_GetObjectItem(root, "heartbeatSupport");
  if (pItem) {
    oservice.heartbeatSupport = static_cast<int8_t>(pItem->valueint);
  }

  //后续添加字段
  pItem = cJSON_GetObjectItem(root, "fweight");
  if (pItem) {
    oservice.fweight = pItem->valuedouble;
  }

  pItem = cJSON_GetObjectItem(root, "serverType");
  if (pItem) {
    oservice.serverType = pItem->valueint;
  }

  pItem = cJSON_GetObjectItem(root, "warmup");
  if (pItem) {
    oservice.serverType = pItem->valueint;
  }

  pItem = cJSON_GetObjectItem(root, "protocol");
  if (pItem) {
    oservice.protocol = pItem->valuestring;
  }
  pItem = cJSON_GetObjectItem(root, "serviceInfo");
  if (pItem) {
    cJSON *svrNames = pItem;
    for (int i = 0; i < cJSON_GetArraySize(svrNames); ++i) {
      cJSON *item = cJSON_GetArrayItem(svrNames, i);
      if (NULL != item) {
        std::string serviceName(item->string);
        cJSON *pUnifiedItem = cJSON_GetObjectItem(item, "unifiedProto");
        if (NULL != pUnifiedItem) {
          bool unifiedProto = (0 != pUnifiedItem->valueint);
          ServiceDetail srv;
          srv.unifiedProto = unifiedProto;
          oservice.serviceInfo[serviceName] = srv;
        } else {
          NS_LOG_WARN("service info unifiedProto is null");
        }
      }
    }
  }
  cJSON_Delete(root);
  return 0;
}

int JsonZkMgr::cJson_AddStringArray(const std::set<std::string> &vecSvrName,
                                    cJSON *root, cJSON *item,
                                    const std::string &itemName) {
  if (NULL == item) {
    NS_LOG_ERROR("failed to cJson_CreateArray");
    return -1;
  }
  for (std::set<std::string>::iterator iter = vecSvrName.begin();
       iter != vecSvrName.end(); ++iter) {
    const char *ptr = (*iter).c_str();
    cJSON_AddItemToArray(item, cJSON_CreateString(ptr));
  }
  cJSON_AddItemToObject(root, itemName.c_str(),
                        item);
  return 0;
}

int JsonZkMgr::cJson_AddServiceObject(const std::map<std::string, ServiceDetail> &vecSvrName,
                                      cJSON *root, cJSON *item,
                                      const std::string &itemName) {
  if (NULL == item) {
    NS_LOG_ERROR("failed to cJSON_CreateObject");
    return -1;
  }
  for (std::map<std::string, ServiceDetail>::const_iterator iter = vecSvrName.begin();
       iter != vecSvrName.end(); ++iter) {
    cJSON *ele = cJSON_CreateObject();
    cJSON_AddNumberToObject(ele, "unifiedProto",
                            iter->second.unifiedProto);

    cJSON_AddItemToObject(item, iter->first.c_str(),
                          ele);
  }
  cJSON_AddItemToObject(root, itemName.c_str(),
                        item);
  return 0;
}


cJSON *JsonZkMgr::GetObjectItem(cJSON *item, const char *name) {
  cJSON *pItem = cJSON_GetObjectItem(item, name);
  if (!pItem) {
    NS_LOG_ERROR("failed to get item " << name);
  }
  return pItem;
}
}  //  namespace meituan_mns
