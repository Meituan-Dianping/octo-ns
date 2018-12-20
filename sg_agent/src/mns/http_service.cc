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

#include "http_service.h"
#include "route_info.h"
#include "config_loader.h"
#include "discovery_service.h"
#include "monitor_collector.h"
using namespace meituan_mns;

muduo::MutexLock HttpService::http_service_lock_;
HttpService *HttpService::http_service_ = NULL;


HttpService *HttpService::GetInstance() {
  if (NULL == http_service_) {
    muduo::MutexLockGuard lock(http_service_lock_);
    if (NULL == http_service_) {
      http_service_ = new HttpService();
    }
  }
  return http_service_;
}
HttpService::~HttpService() {

  if (NULL != http_service_loop_) {
    http_service_loop_->quit();
    usleep(kThreadTime * http_service_loop_->queueSize());
    delete http_service_loop_;
    http_service_loop_ = NULL;
  }
}

/*
 * parse data from http
 */
int32_t HttpService::ParseServiceDataFromHttp(const char *post_data,
                                              ServicePtr &service) {
  if (NULL == post_data) {
    NS_LOG_ERROR("the http post msg is null");
    return FAILURE;
  }
  cJSON *root = cJSON_Parse(post_data);
  if (NULL == root) {
    NS_LOG_ERROR("the cJSON_Parse error");
    return FAILURE;
  }
  cJSON *pItem = cJSON_GetObjectItem(root, "remoteAppkey");
  if (NULL != pItem) {
    std::string remote_appkey = pItem->valuestring;
    boost::trim(remote_appkey);
    if (!remote_appkey.empty()) {
      service->__set_remoteAppkey(remote_appkey);
    } else {
      NS_LOG_ERROR("remoteappkey is empty.");
      cJSON_Delete(root);
      return FAILURE;
    }
  } else {
    NS_LOG_ERROR("the flush service remoteappkey is null");
    cJSON_Delete(root);
    return FAILURE;
  }
  pItem = cJSON_GetObjectItem(root, "protocol");
  if (NULL != pItem) {
    std::string protocol = pItem->valuestring;
    boost::trim(protocol);
    if (!protocol.empty()) {
      service->__set_protocol(protocol);
    } else {
      NS_LOG_ERROR("procotol is empty.");
      cJSON_Delete(root);
      return FAILURE;
    }
  } else {
    NS_LOG_ERROR("the flush service procotol is null");
    cJSON_Delete(root);
    return FAILURE;
  }
  pItem = cJSON_GetObjectItem(root, "localAppkey");
  if (NULL != pItem) {
    service->__set_localAppkey(pItem->valuestring);
  }
  pItem = cJSON_GetObjectItem(root, "version");
  if (NULL != pItem) {
    service->__set_version(pItem->valuestring);
  }
  pItem = cJSON_GetObjectItem(root, "serviceList");
  if (NULL != pItem) {
    std::vector<SGService> servicelist;
    int size = cJSON_GetArraySize(pItem);
    cJSON *item = NULL;
    for (int i = 0; i < size; ++i) {
      SGService serviceInfo;
      item = cJSON_GetArrayItem(pItem, i);
      if (SUCCESS == ParseNodeData(item, serviceInfo)) {
        serviceInfo.__set_appkey(service->remoteAppkey);
        serviceInfo.__set_protocol(service->protocol);
        serviceInfo.__set_envir(CXmlFile::GetAppenv()->GetIntEnv());//获取本地sg_agent环境
        serviceInfo.__set_lastUpdateTime(time(NULL));//获取当前时间
        servicelist.push_back(serviceInfo);
      } else {
        NS_LOG_WARN("Failed to parse the " << i << "th serviceInfo.");
      }
    }
    if (!servicelist.empty()) {
      service->__set_serviceList(servicelist);
    } else {
      NS_LOG_WARN("servlist is empty , remoteAppkey = " << service->remoteAppkey
                                                          << ", protocol = " << service->protocol);
    }
  }
  cJSON_Delete(root);
  return SUCCESS;
}
int32_t HttpService::ParseNodeData(cJSON *root, SGService &service_info) {
  cJSON *pItem = cJSON_GetObjectItem(root, "ip");
  if (NULL != pItem) {
    service_info.__set_ip(pItem->valuestring);
  } else {
    NS_LOG_ERROR("the flush service ip is null");
    return FAILURE;
  }
  pItem = cJSON_GetObjectItem(root, "port");
  if (NULL != pItem) {
    service_info.__set_port(pItem->valueint);
  } else {
    NS_LOG_ERROR("the flush port is null");
    return FAILURE;
  }
  pItem = cJSON_GetObjectItem(root, "version");
  if (NULL != pItem) {
    service_info.__set_version(pItem->valuestring);
  }
  pItem = cJSON_GetObjectItem(root, "weight");
  if (NULL != pItem) {
    service_info.__set_weight(pItem->valueint);
  }
  pItem = cJSON_GetObjectItem(root, "status");
  if (NULL != pItem) {
    service_info.__set_status(pItem->valueint);
  }
  pItem = cJSON_GetObjectItem(root, "role");
  if (NULL != pItem) {
    service_info.__set_role(pItem->valueint);
  }
  pItem = cJSON_GetObjectItem(root, "serverType");
  if (NULL != pItem) {
    service_info.__set_serverType(pItem->valueint);
  }
  pItem = cJSON_GetObjectItem(root, "heartbeatSupport");
  if (NULL != pItem) {
    service_info.__set_heartbeatSupport(pItem->valueint);
  }
  pItem = cJSON_GetObjectItem(root, "serviceInfo");
  if (pItem) {
    cJSON *svrNames = pItem;
    int size = cJSON_GetArraySize(svrNames);
    for (int i = 0; i < size; ++i) {
      cJSON *item = cJSON_GetArrayItem(svrNames, i);
      if (NULL != item) {
        std::string serviceName(item->string);
        NS_LOG_INFO("service name is:" << serviceName);
        bool unifiedProto =
            (0 != cJSON_GetObjectItem(item, "unifiedProto")->valueint);
        ServiceDetail srv;
        srv.__set_unifiedProto(unifiedProto);
        service_info.serviceInfo[serviceName] = srv;
      }
    }
  }
  NS_LOG_INFO("node flush info: appkey:"
                    << service_info.appkey << " ip: "
                    << service_info.ip << " status:"
                    << service_info.status << " last update time: "
                    << service_info.lastUpdateTime);
  return SUCCESS;
}

void HttpService::StartService() {

  char local_ip[LOCAL_IP], local_mask[LOCAL_MASK];
  memset(local_ip, '0', LOCAL_IP);
  memset(local_mask, '0', LOCAL_MASK);

  if (getIntranet(local_ip, local_mask) < 0) {
    NS_LOG_ERROR("getIntranet failed");
    return;
  } else {
    NS_LOG_INFO(
        "getIntranet ip and mask are " << local_ip << "," << local_mask);
  }

  struct event_base *base;
  struct evhttp *http;
  base = event_base_new();
  if (!base) {
    NS_LOG_ERROR("event_base_new() failed");
    return;
  }
  http = evhttp_new(base);
  if (!http) {
    NS_LOG_ERROR("evhttp_new() http server failed");
    event_base_free(base);
    return;
  }
  if (evhttp_bind_socket(http, listen_ip.c_str(), http_port)) {
    NS_LOG_ERROR("http bind socket failed");
    evhttp_free(http);
    event_base_free(base);
    return;
  }

  evhttp_set_gencb(http, HttpHandler, this);
  event_base_dispatch(base);
  evhttp_free(http);
  event_base_free(base);

  return;
}

int32_t HttpService::UpdateServiceInCache(const ServicePtr &service) {

  NS_LOG_INFO("http service:update service ");
  int32_t ret = DiscoveryService::GetInstance()->UpdateSrvList(service);
  if (SUCCESS != ret) {
    NS_LOG_ERROR("Run mns update srv list failed, ret = " << ret);
  }
  return ret;
}

int32_t HttpService::GetServListAndCacheSize(ServListAndCache &list_and_cache,
                                             const std::string &protocol,
                                             const std::string &appkey) {
  NS_LOG_INFO("http service:get service, protocol = " << protocol << " ,appkey = " << appkey);
  int32_t ret = DiscoveryService::GetInstance()->GetSrvListAndCacheSize(list_and_cache,
                                                        protocol, appkey);
  if (SUCCESS != ret) {
    NS_LOG_ERROR("Run mns get srvlist and buffer failed, ret = " << ret);
  }
  return ret;
}

int32_t HttpService::RepalceServlistAndCache(const ServicePtr &service) {
  NS_LOG_INFO("http service:replace service.");
  int32_t ret = DiscoveryService::GetInstance()->RepalceSrvlist(service);
  if (SUCCESS != ret) {
    NS_LOG_ERROR("Run mns replace srvlist and cache failed, ret = " << ret);
  }
  return ret;
}

int32_t HttpService::ServiceListByProtocol(const ProtocolRequest &req,
                                           ProtocolResponse &_return) {
  NS_LOG_INFO("http service GET servicelist.");

  bool enable_swimlane2 = false;

  boost::shared_ptr<ServiceChannels> service_channel = boost::make_shared<ServiceChannels>();
  service_channel->SetAllChannel(false);
  service_channel->SetBankboneChannel(false);
  service_channel->SetOriginChannel(false);
  service_channel->SetSwimlaneChannel(enable_swimlane2);

  int32_t ret = DiscoveryService::GetInstance()->DiscGetSrvList(_return
                                                                    .servicelist, req,service_channel);
  NS_LOG_INFO("Http ServiceListByProtocol size = " << _return.servicelist.size
      ());
  _return.errcode = ret;
  if (SUCCESS != ret) {
    NS_LOG_ERROR("Http ServiceListByProtocol failed, and ret = " << ret);
  }
  return ret;
}

void HttpService::HttpHandler(struct evhttp_request *http_request, void *info) {

  if (NULL == http_request || NULL == info) {
    NS_LOG_ERROR("the http handler para is null");
    return;
  }
  u_char *req_buf;
  req_buf = EVBUFFER_DATA(http_request->input_buffer);
  const char *decode_uri = evhttp_request_uri(http_request);
  char *url = evhttp_decode_uri(decode_uri);
  NS_LOG_DEBUG("the remote host is: " << http_request->remote_host
                                        << ", input url method = " << url
                                        << ",the req data = " << req_buf);

  struct evbuffer *buf = evbuffer_new();//response buffer
  if (NULL == buf) {
    NS_LOG_ERROR("Failed to create response buffer");
    return;
  }

  HttpService *httpService = (HttpService *) info;
  int http_err_code = HTTP_OK;
  if (EVHTTP_REQ_GET == http_request->type) {
    http_err_code = httpService->Response2RequestByGet(url, buf);
  } else if (EVHTTP_REQ_POST == http_request->type) {
    http_err_code = httpService->Response2RequestByPost(url, req_buf, buf);
  }
  evhttp_send_reply(http_request,
                    http_err_code,
                    "decode the json data ok!",
                    buf);
  if (NULL != buf) {
    evbuffer_free(buf);
  }
  SAFE_FREE(url);
}

int32_t HttpService::ServiceListActionByPost(const int service_method,
                                             const u_char *req_buf,
                                             struct evbuffer *buf) {
  int http_err_code = HTTP_RESPONSE_OK;
  ServicePtr service(new getservice_res_param_t());
  int ret_code = ParseServiceDataFromHttp((const char *) req_buf, service);
  if (SUCCESS != ret_code) {
    NS_LOG_ERROR("Failed to parse data,  ret = " << ret_code);
    evbuffer_add_printf(buf, "request param error");
    return HTTP_PARAM_ERROR;
  }

  switch (service_method) {
    case ADD_SERVICE: {
      NS_LOG_INFO("http add method");
      break;
    }
    case UPDATE_SERVICE: {
      if (SUCCESS != UpdateServiceInCache(service)) {
        NS_LOG_ERROR("http-update in agent failed");
        http_err_code = HTTP_INNER_ERROR;
      }
      break;
    }
    case DELETE_SERVICE: {
      NS_LOG_INFO("http delete method");
      http_err_code = HTTP_NOT_SUPPORT;
      break;
    }
    case GET_SERVICE: {
      ServListAndCache list_and_cache;
      if (SUCCESS != GetServListAndCacheSize(list_and_cache,
                                             service->protocol,
                                             service->remoteAppkey)) {
        NS_LOG_ERROR("http-get in agent failed");
        http_err_code = HTTP_INNER_ERROR;
      } else {
        std::string response = "";
        int ret = ServListAndCache2Json(list_and_cache, response);
        if (SUCCESS != ret) {
          NS_LOG_ERROR("ServListAndCache(response) to str failed, ret = " << ret);
          http_err_code = HTTP_INNER_ERROR;
        } else {
          evbuffer_add_printf(buf, response.c_str());
        }
      }
      break;
    }
    case REPLACE_SERVICE: {
      if (SUCCESS != RepalceServlistAndCache(service)) {
        NS_LOG_ERROR("http-replace in agent failed");
        http_err_code = HTTP_INNER_ERROR;
      }
      break;
    }
    default: {
      http_err_code = HTTP_NOT_SUPPORT;
      NS_LOG_ERROR("unkown service method, disgardless");
      break;
    }
  }
  return http_err_code;

}

int32_t HttpService::IsHealthy() {
  if (IdcUtil::IsInIdcs(CXmlFile::GetStrPara(CXmlFile::LocalIp))) {
    return HTTP_RESPONSE_OK;
  }
  return HTTP_INNER_ERROR;

}
int32_t HttpService::EncodeHealthyInfo(std::string &res) {

  char *out;
  int ret = HTTP_RESPONSE_OK;
  int ret_json = -1;
  cJSON *json = cJSON_CreateObject();
  if (!json) {
    NS_LOG_ERROR("json is NULL, create json_object failed.");
    return FAILURE;
  }
  ret = IsHealthy();
  cJSON_AddNumberToObject(json, "ret", ret);
  cJSON_AddStringToObject(json, "retMsg", "success");
  cJSON_AddStringToObject(json, "endpoint", CXmlFile::GetAppenv()->GetStrEnv().c_str());
  out = cJSON_Print(json);
  if(NULL!= out){
    res = out;
    SAFE_FREE(out);
    cJSON_Delete(json);
    NS_LOG_INFO("EncodeHealthyInfo success = " 
                      << res << "local ip = " << CXmlFile::GetAppenv()->GetStrEnv());
    return SUCCESS;
  }else{
    cJSON_Delete(json);
    res = HTTP_RESPONSE_NULL;
    NS_LOG_ERROR("EncodeHealthyInfo failed = " 
                       << res << "local ip = " 
                       << CXmlFile::GetAppenv()->GetStrEnv());
    return FAILURE;
  }
}

//获取监控信息reqBuf无需做空判断处理
int32_t HttpService::MonitorActionByPost(int service_method,
                                         const u_char *req_buf,
                                         struct evbuffer *buf) {
  int http_err_code = HTTP_RESPONSE_OK;
  switch (service_method) {
    case GET_MONITOR_SERVICE: {
      std::string response = "";
      int32_t ret = MonitorCollector::GetInstance()->GetCollectorMonitorInfo(response);
      if (SUCCESS != ret) {
        NS_LOG_ERROR("Get collectorMonitorInfo failed, ret = " << ret);
        http_err_code = HTTP_INNER_ERROR;
      } else {
        evbuffer_add_printf(buf, response.c_str());
      }
      break;
    }
    default:http_err_code = HTTP_INNER_ERROR;
      break;
  }
  return http_err_code;
}

/*
 *  response to HTTP requests
 *  缺省为服务列表操作，后续扩展其它
 */
int32_t HttpService::Response2RequestByPost(const char *url,
                                            const u_char *req_buf,
                                            struct evbuffer *buf) {

  int serviceMethod = GetServiceMethodFromHttp(url);
  switch (serviceMethod) {

    case GET_MONITOR_SERVICE: {
      return MonitorActionByPost(serviceMethod, req_buf, buf);
    }
    case HEALTHY_CHECK: {
      return HealthyCheckByPost(serviceMethod, req_buf, buf);
    }
    default: {
      return ServiceListActionByPost(serviceMethod, req_buf, buf);
    }
  }
  return HTTP_RESPONSE_OK;
}

/*
 * ServListAndCache struct to Json
 */
int32_t HttpService::ServListAndCache2Json(const ServListAndCache &list_and_cache,
                                           std::string &response) {
  cJSON *json = cJSON_CreateObject();
  char *out;
  if (!json) {
    NS_LOG_ERROR("json is NULL, create json_object failed.");
    return FAILURE;
  }
  cJSON_AddNumberToObject(json, "origin_servlist_size", list_and_cache.origin_servlist_size);
  cJSON_AddNumberToObject(json, "filte_servlist_size", list_and_cache.filte_servlist_size);
  cJSON_AddNumberToObject(json, "origin_cache_size", list_and_cache.origin_cache_size);
  cJSON_AddNumberToObject(json, "filte_cache_size", list_and_cache.filte_cache_size);
  int ret = Service2Json(list_and_cache.origin_servicelist, json, "origin_servicelist");
  if (SUCCESS != ret) {
    NS_LOG_ERROR("Failed to change origin_servicelist to json, ret = " << ret);
    cJSON_Delete(json);
    return FAILURE;
  }
  ret = Service2Json(list_and_cache.filte_servicelist, json, "filte_servicelist");
  if (SUCCESS != ret) {
    NS_LOG_ERROR("Failed to change filte_servicelist to json, ret = " << ret);
    cJSON_Delete(json);
    return FAILURE;
  }
  out = cJSON_Print(json);
  response = out;
  SAFE_FREE(out);
  cJSON_Delete(json);
  boost::trim(response);
  if (response.empty()) {
    NS_LOG_ERROR("json to str failed, the response json is empty.");
    return FAILURE;
  }
  return SUCCESS;
}
/*
 * SGService to json
 */
int32_t HttpService::Service2Json(const std::vector<SGService> &servicelist,
                                  cJSON *json,
                                  const char *type) {
  cJSON *all_srvlist_json = cJSON_CreateArray();
  if (!all_srvlist_json) {
    NS_LOG_ERROR("all_srvlist_json is NULL, create json_object failed.");
    return FAILURE;
  }
  for (std::vector<SGService>::const_iterator iter = servicelist.begin();
       iter != servicelist.end(); ++iter) {
    cJSON *item = cJSON_CreateObject();
    int ret = SGService2Json(*iter, item);
    if (SUCCESS != ret) {
      NS_LOG_ERROR("SGService2Json failed, ret = " << ret);
      return FAILURE;
    }
    cJSON_AddItemToArray(all_srvlist_json, item);
  }
  cJSON_AddItemToObject(json, type, all_srvlist_json);
  return SUCCESS;
}

int32_t HttpService::SGService2Json(const SGService &oservice, cJSON *root) {
  cJSON_AddItemToObject(root, "appkey", cJSON_CreateString(oservice.appkey.c_str()));
  cJSON_AddItemToObject(root, "version", cJSON_CreateString(oservice.version.c_str()));
  cJSON_AddItemToObject(root, "ip", cJSON_CreateString(oservice.ip.c_str()));
  cJSON_AddNumberToObject(root, "port", oservice.port);
  cJSON_AddNumberToObject(root, "weight", oservice.weight);
  cJSON_AddNumberToObject(root, "status", oservice.status);
  cJSON_AddNumberToObject(root, "role", oservice.role);
  cJSON_AddNumberToObject(root, "env", CXmlFile::GetAppenv()->GetIntEnv());
  cJSON_AddNumberToObject(root, "lastUpdateTime", oservice.lastUpdateTime);
  //后续添加,注意有可能没有
  cJSON_AddNumberToObject(root, "fweight", oservice.fweight);
  cJSON_AddNumberToObject(root, "serverType", oservice.serverType);

  int32_t heartbeatSupport = oservice.heartbeatSupport;
  cJSON_AddNumberToObject(root, "heartbeatSupport", heartbeatSupport);
  cJSON_AddItemToObject(root, "protocol", cJSON_CreateString(oservice.protocol.c_str()));

  cJSON *item = cJSON_CreateObject();
  if (NULL == item) {
    NS_LOG_ERROR("cJson failed to CreateObject. Item is serviceInfo");
    return FAILURE;
  }
  int ret = JsonZkMgr::cJson_AddServiceObject(oservice.serviceInfo, root,
                                              item, std::string("serviceInfo"));
  if (0 != ret) {
    NS_LOG_ERROR("failed to add serviceName to root");
    return FAILURE;
  }

  return SUCCESS;
}

int32_t HttpService::GetKeyValueFromUrl(const std::string &url,
                                        const std::string &key,
                                        std::string &value) {
  int len = url.length();
  size_t pos = url.find(key);
  if (std::string::npos != pos) {
    for (int i = pos + key.length(); i < len && '&' != url[i]; ++i) {
      value += url[i];
    }
    if (value.empty()) {
      NS_LOG_DEBUG("key: " << key << " is empty.");
      return FAILURE;
    }
  } else {
    NS_LOG_DEBUG("key: " << key << " does not exist");
    return FAILURE;
  }
  return SUCCESS;
}

int32_t HttpService::GetServiceParamFromUrl(const std::string &url,
                                            ProtocolRequest &params) {
  NS_LOG_DEBUG("the MNS url is: " << url);
  std::string tmp_url = url;
  boost::trim(tmp_url);
  size_t pos = tmp_url.find("/api/servicelist?");
  size_t appkey_pos = tmp_url.find("appkey=");
  size_t protocol_pos = tmp_url.find("protocol=");
  if (std::string::npos != pos && std::string::npos != appkey_pos
      && std::string::npos != protocol_pos) {
    std::string env = "";
    int ret = GetKeyValueFromUrl(tmp_url, "env=", env);
    if (SUCCESS == ret) {
      if (env == CXmlFile::GetAppenv()->GetStrEnv()) {
        NS_LOG_DEBUG("env is " << env);
      } else {
        NS_LOG_ERROR("env:" << env << " is different from local env:" << CXmlFile::GetAppenv()->GetStrEnv());
        return ERR_INVALID_ENV;
      }
    } else {
      NS_LOG_WARN("env error, ret = " << ret);
    }
    std::string appkey = "";
    ret = GetKeyValueFromUrl(tmp_url, "appkey=", appkey);
    if (SUCCESS == ret) {
      params.remoteAppkey = appkey;
    } else {
      NS_LOG_ERROR("appkey error, ret = " << ret);
      return HTTP_PARAM_ERROR;
    }
    std::string protocol = "";
    ret = GetKeyValueFromUrl(tmp_url, "protocol=", protocol);
    if (SUCCESS == ret) {
      params.protocol = protocol;
    } else {
      NS_LOG_ERROR("protocol error, ret = " << ret);
      return HTTP_PARAM_ERROR;
    }
    std::string hostname = "";
    ret = GetKeyValueFromUrl(tmp_url, "hostname=", hostname);
    if (SUCCESS == ret) {
      NS_LOG_INFO("hostname is " << hostname);
    } else {
      NS_LOG_WARN("hostname error, ret = " << ret);
    }
    std::string localip = "";
    ret = GetKeyValueFromUrl(tmp_url, "localip=", localip);
    if (SUCCESS == ret) {
      NS_LOG_INFO("localip is " << localip);
    } else {
      NS_LOG_WARN("localip error ,ret = " << ret);
    }
  } else {
    NS_LOG_ERROR("params are empty.");
    return HTTP_PARAM_ERROR;
  }
  return HTTP_OK;
}

int32_t HttpService::Response2RequestByGet(const char *url,
                                           struct evbuffer *buf) {
  ProtocolRequest req;
  int http_code = GetServiceParamFromUrl(url, req);
  if (HTTP_OK != http_code) {
    NS_LOG_ERROR("http response failed, err_code = " << http_code);
    return http_code;
  }
  std::vector<SGService> result;
  req.__set_remoteAppkey(req.remoteAppkey);
  req.__set_protocol(req.protocol);
  ProtocolResponse _return;
  int ret = ServiceListByProtocol(req, _return);
  if (SUCCESS != ret) {
    NS_LOG_ERROR("Failed to get servicelist by http, ret = " << ret);
    return HTTP_INNER_ERROR;
  }
  std::string response = "";
  ret = ProtocolResponse2Json(_return, response);
  if (SUCCESS != ret) {
    NS_LOG_ERROR("Failed to ProtocolResponse2Json, ret = " << ret);
    return HTTP_INNER_ERROR;
  }
  evbuffer_add_printf(buf, response.c_str());
  //NS_LOG_INFO("response : " << response);
  return http_code;
}

int32_t HttpService::HealthyCheckByPost(int serviceMethod, const u_char
                                        *req_buf, struct evbuffer *buf) {

  NS_LOG_INFO("the healthy check by post");
  int http_err_code = HTTP_RESPONSE_OK;
  std::string response = "";
  if (SUCCESS != EncodeHealthyInfo(response)) {
    NS_LOG_ERROR("Get collectorMonitorInfo failed, ret = ");
    http_err_code = HTTP_INNER_ERROR;
  } else {
    evbuffer_add_printf(buf, response.c_str());
  }
  return http_err_code;
}

int32_t HttpService::ProtocolResponse2Json(const ProtocolResponse &res,
                                           std::string &response) {
  cJSON *parent = cJSON_CreateObject();
  char *out;
  if (NULL == parent) {
    NS_LOG_ERROR("Failed to create json object:parent.");
    return FAILURE;
  }
  cJSON *child = cJSON_CreateObject();
  if (NULL == child) {
    NS_LOG_ERROR("Failed to create json object:child.");
    return FAILURE;
  }
  cJSON_AddNumberToObject(parent, "ret", HTTP_RESPONSE_OK);
  cJSON_AddStringToObject(parent, "retMsg", "success");
  int ret = Service2Json(res.servicelist, child, "serviceList");
  if (SUCCESS != ret) {
    NS_LOG_ERROR("Failed to parse service, ret = " << ret);
    cJSON_Delete(parent);
    return FAILURE;
  }

  cJSON_AddItemToObject(parent, "data", child);
  out = cJSON_Print(parent);
  response = out;
//	NS_LOG_INFO("the response service list is "<< response);
  boost::trim(response);
  if (response.empty()) {
    NS_LOG_ERROR("json to str failed, the response json is empty.");
    SAFE_FREE(out);
    cJSON_Delete(parent);
    return FAILURE;
  }
  SAFE_FREE(out);
  cJSON_Delete(parent);
  return SUCCESS;
}

//todo:monitor子方法后续细分后解析
int32_t HttpService::DecodeMonitorUrlMethod(const std::string &url) {
  NS_LOG_INFO("the monitor url is:" << url);

  return GET_MONITOR_SERVICE;

}

int32_t HttpService::DecodeMnsUrlMethod(const std::string &url) {

  NS_LOG_INFO("the Mns Url is:" << url);
  std::string strtmp = "";
  strtmp.assign(url, strlen("/api/mns/provider/"), url.length());
  NS_LOG_INFO("find method:" << strtmp);
  if (SUCCESS == strtmp.compare("add")) {
    return ADD_SERVICE;
  } else if (SUCCESS == strtmp.compare("delete")) {
    return DELETE_SERVICE;
  } else if (SUCCESS == strtmp.compare("update")) {
    return UPDATE_SERVICE;
  } else if (SUCCESS == strtmp.compare("get")) {
    return GET_SERVICE;
  } else if (SUCCESS == strtmp.compare("monitorinfo")) {//名字修改monitor
    return GET_MONITOR_SERVICE;
  } else if (SUCCESS == strtmp.compare("replace")) {//强制替换列表
    return REPLACE_SERVICE;
  } else {
    NS_LOG_ERROR("unkown http sub method");
  }
  return INVALID_METHOD;

}
int32_t HttpService::DecodeHealthyMethod(const std::string &url) {
  NS_LOG_INFO("the monitor url is:" << url);

  return HEALTHY_CHECK;

}

int32_t HttpService::GetServiceMethodFromHttp(const char *input_method) {
  if (NULL == input_method) {
    NS_LOG_ERROR("the inputMethod is null");
    return INVALID_METHOD;
  }
  std::string input_str = input_method;
  NS_LOG_INFO("Input str = " << input_str);
  std::size_t pos_mns = input_str.find("/api/mns/provider/");
  if (std::string::npos != pos_mns && 0 == pos_mns) {
    return DecodeMnsUrlMethod(input_str);
  }
  std::size_t pos_mon = input_str.find("/api/monitor");
  if (std::string::npos != pos_mon && 0 == pos_mon) {
    return DecodeMonitorUrlMethod(input_str);
  }
  std::size_t pos_hlh = input_str.find("/api/healthy");
  if (std::string::npos != pos_hlh && 0 == pos_hlh) {
    return DecodeHealthyMethod(input_str);
  }
  return INVALID_METHOD;
}
void HttpService::StartHttpServer() {

  NS_LOG_INFO("http server init");
  http_service_loop_ = http_service_thread_.startLoop();
  http_service_loop_->runInLoop(boost::bind(&HttpService::StartService, this));

}






