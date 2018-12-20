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

#include <gtest/gtest.h>
#include "base_consts.h"
#include "base_mns_consts.h"
#include "base_errors_consts.h"
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#define private public
#define protected public
#include "mnsc_client.h"
#undef private
#undef protected


using namespace meituan_mns;

class MnscServiceTest : public testing::Test {

 public:
  void SetUp() {
    node.__set_appkey("octo.naming.service.tmy");
    node.__set_ip("127.0.0.1");
    node.__set_port(5266);
    node.__set_protocol("thrift");
    node.__set_envir(1);
    node.__set_version("chrion_test");
    node.__set_fweight(10.0);
    node.__set_weight(10);
    node.__set_status(2);
    mnsc_client_ = MnscClient::GetInstance();
  }
 public:
  SGService node;
  MnscClient *mnsc_client_;

};


TEST_F(MnscServiceTest,GetThriftServiceListTest) {
  std::string appkey = "test";
  std::vector<SGService> service_list;
  EXPECT_EQ(ERR_FAILEDTOGETCONFSERVLIST, mnsc_client_->GetThriftServiceList(appkey, service_list));
  appkey = "octo.naming.service.tmy";//找个节点都是不可用状态的服务
  EXPECT_EQ(ERR_SERVICELIST_NULL, mnsc_client_->GetThriftServiceList(appkey, service_list));

}

TEST_F(MnscServiceTest , GetServicelistFromMnscTest) {

  std::vector<SGService> serviceList;
  std::string appkey = "octo.naming.cos.mtconfig";
  std::string version = "";
  std::string env = "test";
  std::string protocol = "thrift";
  boost::unordered_map<std::string, int> count;
  GetMnsCacheParams params;
  params.appkey = "octo.naming.cos.mtconfig";
  params.version = "octo.naming.cos.mtconfig";
  params.env = "test";
  params.protocol = "thrift";

  mnsc_client_->GetServicelistFromMnsc(serviceList,params);
  std::cout<<"the servicelist size from mnsc "<<serviceList.size()<<std::endl;

}

TEST_F(MnscServiceTest, GetMnscCacheTest) {
  std::vector<SGService> service_list;
  GetMnsCacheParams params;
  params.appkey = "octo.naming.service.tmy";
  params.version = "test-verison";
  params.env = "test";
  params.protocol = "thrift";
  EXPECT_EQ(404, mnsc_client_->GetMnscCache(service_list, params));
  params.protocol = "http";
  EXPECT_EQ(200, mnsc_client_->GetMnscCache(service_list, params));
  params.protocol = "test";
  EXPECT_EQ(200, mnsc_client_->GetMnscCache(service_list, params));
}

