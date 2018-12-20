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


#ifndef SG_AGENT_HTTP_SERVICE_H
#define SG_AGENT_HTTP_SERVICE_H

#include <assert.h>
#include <string>
#include <map>
#include <list>
#include <iterator>
#include <stdio.h>
#include <evhttp.h>
#include <boost/algorithm/string/trim.hpp>
#include "cJSON.h"
#include "naming_service_types.h"
#include "inc_comm.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/EventLoopThread.h"
#include "boost/bind.hpp"
#include "json_data_tools.h"
#include "base_mns_consts.h"
#include "base_mns_define.h"
#include "naming_common_types.h"
#include "naming_data_types.h"


#define LOCAL_IP 32
#define LOCAL_MASK 64

namespace meituan_mns {

typedef boost::shared_ptr<getservice_res_param_t> ServicePtr;


class HttpService {
 private:
  HttpService() : http_service_loop_(NULL), http_service_thread_
      (boost::function<void(muduo::net::EventLoop * )>(), "sg_agent") {};
 public:
  ~HttpService();
  static HttpService *GetInstance();

  void StartService();
  void StopService() {};
  void StartHttpServer();


 private:
  /**
  * 更新缓存服务列表
  * @param service
  *
  * @return int32_t
  *
  */
  int32_t UpdateServiceInCache(const ServicePtr &service);
  /**
  * 从缓存获取服务列表
  * @param service
  *
  * @return int32_t
  *
  */
  int32_t GetServListAndCacheSize(ServListAndCache &list_and_cache,
                                  const std::string &protocol,
                                  const std::string &appkey);
  /**
  * 从缓存更新服务列表
  * @param service
  *
  * @return int32_t
  *
  */
  int32_t RepalceServlistAndCache(const ServicePtr &service);
  /**
  * 获取http请求方法类型
  * @param input_method
  *
  * @return int32_t
  *
  */
  int32_t GetServiceMethodFromHttp(const char *input_method);
  /**
  * 从url获取参数
  * @param url,params
  *
  * @return int32_t
  *
  */
  int32_t GetServiceParamFromUrl(const std::string &url,
                                 ProtocolRequest &params);
  /**
  * 从url获取剑指key
  * @param url,key,value
  *
  * @return int32_t
  *
  */
  int32_t GetKeyValueFromUrl(const std::string &url,
                             const std::string &key,
                             std::string &value);
  /**
  * get方法响应
  * @param url,buf
  *
  * @return int32_t
  *
  */
  int32_t Response2RequestByGet(const char *url, struct evbuffer *buf);
  /**
  * post方法响应
  * @param url,req_buf,buf
  *
  * @return int32_t
  *
  */
  int32_t Response2RequestByPost(const char *url, const u_char *req_buf,
                             struct evbuffer *buf);

  int32_t Service2Json(const std::vector<SGService> &servicelist,
                       cJSON *json,
                       const char *type);

  int32_t ServListAndCache2Json(const ServListAndCache &list_and_cache,
                                std::string &response);

  int32_t ServiceListActionByPost(const int serviceMethod,
                                  const u_char *req_buf,
                                  struct evbuffer *buf);

  int32_t ServiceListByProtocol(const ProtocolRequest &req,
                                ProtocolResponse &_return);

  int32_t SGService2Json(const SGService &oservice, cJSON *root);

  int32_t DecodeMonitorUrlMethod(const std::string &url);

  int32_t DecodeMnsUrlMethod(const std::string &url);

  int32_t DecodeHealthyMethod(const std::string &url);

  int32_t ProtocolResponse2Json(const ProtocolResponse &res,
                                std::string &response);

  int32_t ParseNodeData(cJSON *root, SGService &service_info);

  int32_t ParseServiceDataFromHttp(const char *post_data, ServicePtr &service);

  int32_t MonitorActionByPost(int serviceMethod, const u_char *req_buf,
                          struct evbuffer *buf);
  int32_t HealthyCheckByPost(int serviceMethod, const u_char *req_buf,
                             struct evbuffer *buf);
  int32_t EncodeHealthyInfo(std::string &res);

  static void HttpHandler(struct evhttp_request *http_request, void *info);

  int32_t IsHealthy();

  static HttpService *http_service_;

  static muduo::MutexLock http_service_lock_;

  muduo::net::EventLoopThread http_service_thread_;

  muduo::net::EventLoop *http_service_loop_;

};

}

#endif //SG_AGENT_HTTP_SERVICE_H
