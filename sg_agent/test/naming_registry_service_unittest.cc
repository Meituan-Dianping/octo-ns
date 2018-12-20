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
#include "registry_service.h"
#undef private
#undef protected


using namespace meituan_mns;

class RegistryZkTest : public testing::Test {

 public:
  void SetUp() {
    node.__set_appkey("octo.naming.service.yangjie");
    node.__set_protocol("thrift");
    node.__set_envir(1);
    node.__set_version("chrion_test");
    node.__set_fweight(10.0);
    node.__set_weight(10);
    node.__set_status(2);

    node_a.__set_appkey("octo.naming.service.yangjie");
    node_a.__set_protocol("thrift");
    node_a.__set_envir(1);
    node_a.__set_version("chrion_test");
    node_a.__set_fweight(10.0);
    node_a.__set_weight(10);
    node_a.__set_status(2);

    node_b.__set_appkey("octo.naming.service.yangjie");
    node_b.__set_protocol("thrift");
    node_b.__set_envir(1);
    node_b.__set_version("chrion_test");
    node_b.__set_fweight(10.0);
    node_b.__set_weight(10);
    node_b.__set_status(2);
    registry_service_ = RegistryService::GetInstance();
  }
 public:
  SGService node;
  SGService node_a;
  SGService node_b;
  RegistryService *registry_service_;

};

TEST_F(RegistryZkTest, zkNodeNotExist) {
  std::map<std::string, ServiceDetail>  serviceInfo;
  std::map<std::string, ServiceDetail>  serviceInfo_a;
  std::map<std::string, ServiceDetail>  serviceInfo_b;
  int port = 10086;
  std::string cell = "test";
  node.__set_ip("127.0.0.1");
  node.__set_port(port);
  node_a.__set_ip("127.0.0.1");
  node_a.__set_port(port);
  node_b.__set_ip("127.0.0.1");
  node_b.__set_port(port);
  std::cout<<"the test registry"<<std::endl;

  ServiceDetail srv;
  srv.__set_unifiedProto(0);
  serviceInfo["this is the test registry"] = srv;
  node.__set_serviceInfo(serviceInfo);

  ServiceDetail srv_a;
  srv.__set_unifiedProto(0);
  serviceInfo_a["this is the test registry a"] = srv_a;
  node_a.__set_serviceInfo(serviceInfo_a);

  ServiceDetail srv_b;
  srv.__set_unifiedProto(0);
  serviceInfo_b["this is the test registry b"] = srv_b;
  node_b.__set_serviceInfo(serviceInfo_b);
  EXPECT_EQ(SUCCESS, registry_service_->RegistryStart(node, 1));
  EXPECT_EQ(SUCCESS, registry_service_->RegistryStart(node_a, 1));
  EXPECT_EQ(SUCCESS, registry_service_->RegistryStart(node_b, 1));

}