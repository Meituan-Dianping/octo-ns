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
#include "service_channels.h"
#define private public
#define protected public
#include "discovery_zk_client.h"
#undef private
#undef protected
using namespace meituan_mns;

class DiscZkClientTest : public testing::Test {
 public:
  void SetUp() {
    std::vector<SGService> service_list;
    SGService service1;
    service1.__set_ip("127.0.0.1");
    service1.__set_protocol("protocol");
    service1.__set_appkey("octo.naming.service.tmy");
    service1.__set_port(5266);
    service1.__set_version("test");
    service1.__set_weight(-1);
    service1.__set_fweight(101);
    service_list.push_back(service1);
    node = boost::make_shared<getservice_res_param_t>();
    node->__set_localAppkey("");
    node->__set_remoteAppkey("octo.naming.service.tmy");
    node->__set_serviceList(service_list);
    node->__set_protocol("thrift");
    discovery_service_ = DiscoveryZkClient::GetInstance();

    req.protocol = "thrift";
    req.remoteAppkey = "octo.naming.service.tmy";
    req.serviceName = "http://mns.octo.com";
    service_channel = boost::make_shared<ServiceChannels>();
    service_channel->SetAllChannel(false);
    service_channel->SetBankboneChannel(false);
    service_channel->SetOriginChannel(false);
    service_channel->SetSwimlaneChannel(false);
  }
 public:
  boost::shared_ptr<getservice_res_param_t> node;
  DiscoveryZkClient *discovery_service_;
  ProtocolRequest req;
  boost::shared_ptr<ServiceChannels> service_channel;
};


TEST_F(DiscZkClientTest, DiscSrvListByProtocolTest) {
  EXPECT_EQ(0, discovery_service_->DiscSrvListByProtocol(node));
  node->__set_remoteAppkey("octo.naming.service.mtconfig");
  EXPECT_EQ(0, discovery_service_->DiscSrvListByProtocol(node));
  node->__set_remoteAppkey("test");
  EXPECT_EQ(-101, discovery_service_->DiscSrvListByProtocol(node));
}

TEST_F(DiscZkClientTest, DiscRouteListByProtocolTest) {
  std::vector<CRouteData> routeList;
  std::string localAppkey = "test";
  std::string appKey = "octo.naming.service.tmy";
  std::string version = "";
  std::string protocol = "thrift";
  bool route_flag = false;
  EXPECT_EQ(0, discovery_service_->DiscRouteListByProtocol(routeList,
    localAppkey,appKey, version,protocol, route_flag));
  EXPECT_EQ(ERR_ZK_LIST_SAME_BUFFER, discovery_service_->DiscRouteListByProtocol(routeList,
      localAppkey,appKey, version,protocol, route_flag));
  appKey = "test";
  EXPECT_EQ(ERR_NODE_NOTFIND,discovery_service_->DiscRouteListByProtocol(routeList,
      localAppkey,appKey, version,protocol, route_flag));
}

TEST_F(DiscZkClientTest, DiscAppkeyByServiceNameTest) {
  std::set<std::string> appkeys;
  std::string local_appkey = "test";
  std::string service_name = "http://mns.octo.com";
  std::string version = "0";
  std::string protocol = "thrift";
  EXPECT_EQ(0, discovery_service_->DiscAppkeyByServiceName(appkeys,
      local_appkey,service_name,version, protocol));
  service_name = "test";
  EXPECT_EQ(ERR_NODE_NOTFIND, discovery_service_->DiscAppkeyByServiceName(appkeys,
    local_appkey,service_name,version, protocol));
}

TEST_F(DiscZkClientTest, DiscoveryListFromCacheTest) {
  std::vector<SGService> srvlist;
  GetMnsCacheParams params;
  params.appkey = "octo.naming.service.mtconfig";
  params.version = "octo.naming.service.mtconfig";
  params.env = "test";
  params.protocol = "thrift";
  //EXPECT_EQ(0, discovery_service_->
  // DiscoveryListFromCache(srvlist, params)) ; //todo for private

}

TEST_F(DiscZkClientTest, DivRangeDiscIndexTest) {
  int index = 1;
  int begin = 0;
  int end = 3;
  int child_count = 5;
  discovery_service_->DivRangeDiscIndex(index, begin, end, child_count);
}
