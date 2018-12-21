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

#include "config_loader.h"
#include <gtest/gtest.h>
#include "base_errors_consts.h"
#include "registry_service.h"
#include "base_mns_consts.h"
#include "base_consts.h"
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include "service_channels.h"
#include "sds_tools.h"


#define private public
#define protected public
#include "../mns/http_service.h"
#undef private
#undef protected

using namespace meituan_mns;

class HttpServiceTest : public testing::Test {
 public:
  void SetUp() {
    std::vector<SGService> service_list;
    SGService service1, service2;
    service1.__set_ip("127.0.0.1");
    service1.__set_protocol("protocol");
    service1.__set_appkey("octo.naming.service.tmy");
    service1.__set_port(5266);
    service1.__set_version("dev");
    service1.__set_weight(-1);
    service1.__set_fweight(101.0);
    service1.__set_ip("10.4.245.3");
    service1.__set_protocol("protocol");
    service1.__set_appkey("octo.naming.service.tmy");
    service1.__set_port(5266);
    service1.__set_version("dev");
    service1.__set_weight(0);
    service1.__set_fweight(101);
    service_list.push_back(service1);
    service_list.push_back(service2);
    node = boost::make_shared<getservice_res_param_t>();
    node->__set_localAppkey("");
    node->__set_remoteAppkey("octo.naming.service.tmy");
    node->__set_serviceList(service_list);
    node->__set_protocol("thrift");
    http_server_ = HttpService::GetInstance();
  }
 public:
  HttpService *http_server_;
  boost::shared_ptr<getservice_res_param_t> node;
  ProtocolRequest req;
  boost::shared_ptr<ServiceChannels> service_channel;
};

TEST_F(HttpServiceTest, UpdateServiceInCacheTest) {
  EXPECT_EQ(0, http_server_->UpdateServiceInCache(node));
}

TEST_F(HttpServiceTest, GetServListAndCacheSizeTest) {
  ServListAndCache list_and_cache;
  std::string protocol = "thrift";
  std::string appkey = "octo.naming.service.tmy";
  EXPECT_EQ(0, http_server_->GetServListAndCacheSize(list_and_cache,protocol, appkey));
}

TEST_F(HttpServiceTest, RepalceServlistAndCacheTest) {
  EXPECT_EQ(0, http_server_->RepalceServlistAndCache(node));
}

TEST_F(HttpServiceTest, GetServiceMethodFromHttpTest) {
  std::string input = "/api/mns/provider/delete";
  EXPECT_EQ(0, http_server_->GetServiceMethodFromHttp(input.c_str()));
  input = "";
  EXPECT_EQ(INVALID_METHOD, http_server_->GetServiceMethodFromHttp(input.c_str()));
  input = "/api/monitor";
  EXPECT_EQ(GET_MONITOR_SERVICE, http_server_->GetServiceMethodFromHttp(input.c_str()));
  input = "/api/healthy";
  EXPECT_EQ(HEALTHY_CHECK, http_server_->GetServiceMethodFromHttp(input.c_str()));
}

