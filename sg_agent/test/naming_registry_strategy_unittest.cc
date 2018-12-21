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


#define private public
#define protected public
#include "../mns/registry_strategy.h"
#undef private
#undef protected

using namespace meituan_mns;

class RegistryStrategyTest : public testing::Test {

 public:
  void SetUp() {
    node.__set_appkey("octo.naming.service.yangjie");
    node.__set_protocol("thrift");
    node.__set_envir(1);
    node.__set_version("chrion_test");
    node.__set_fweight(10.0);
    node.__set_weight(10);
    node.__set_status(2);

    node1.__set_appkey("octo.naming.service.yangjie");
    node1.__set_protocol("thrift");
    node1.__set_envir(1);
    node1.__set_version("chrion_test");
    node1.__set_fweight(10.0);
    node1.__set_weight(10);
    node1.__set_status(2);
    register_service_ = RegistryService::GetInstance();
  }
 public:
  SGService node;
  SGService node1;
  CXmlFile cxml_file_;
  RegistryService *register_service_;
  RegistryStrategy register_strategy_;

};

#if 0
TEST_F(RegistryStrategyTest, zkNodeNotExist) {

}
//测试加载
TEST_F(RegistryStrategyTest, registryInfoLoad) {
  cxml_file_.CXmlFileInit();
}
//测试
TEST_F(RegistryStrategyTest, hotelRegistryTest) {
  int port = 10086;
  std::string cell = "test";
  node.__set_ip("127.0.0.1);
  node.__set_port(port);
  std::cout<<"the test registry"<<std::endl;

  EXPECT_EQ(SUCCESS, register_service_->RegistryStart(node, 1));
}

TEST_F(RegistryStrategyTest, IsAllowedRegistryTest) {
  EXPECT_EQ(0, register_strategy_.IsAllowedRegistry(node));
  node.__set_appkey("");
  EXPECT_EQ(ERR_EMPTY_APPKEY, register_strategy_.IsAllowedRegistry(node));
  node.__set_appkey("octo.naming.travel.dsg.crmtag");
  EXPECT_EQ(-1, register_strategy_.IsAllowedRegistry(node));
  node.__set_appkey("test-benchmark-web");//双框架appkey
  EXPECT_EQ(-1, register_strategy_.IsAllowedRegistry(node));
}

TEST_F(RegistryStrategyTest, IsRepeatedRegisterTest) {
  EXPECT_TRUE(register_strategy_.IsRepeatedRegister(node,node1,UptCmd::ADD));
}

TEST_F(RegistryStrategyTest, CheckAllowedProtocolRegistryTest) {
  node.__set_protocol("thrift");
  EXPECT_EQ(0, register_strategy_.CheckAllowedProtocolRegistry(node));
  node.__set_protocol("http");
  EXPECT_EQ(0, register_strategy_.CheckAllowedProtocolRegistry(node));
  node.__set_protocol("unknown");
  EXPECT_EQ(-1, register_strategy_.CheckAllowedProtocolRegistry(node));
}


TEST_F(RegistryStrategyTest, CheckArgsTest) {
  node._set_appkey("");
  EXPECT_EQ(-1, register_strategy_.CheckArgs(node));
  node.__set_appkey("octo.naming.~.)");//非法字符的appkey
  EXPECT_EQ(-1, register_strategy_.CheckArgs(node));


}
TEST_F(RegistryStrategyTest, CheckArgsTest1) {
  node.__set_weight(-1);
  EXPECT_EQ(ERR_INVALID_WEIGHT, register_strategy_.CheckArgs(node));
}

TEST_F(RegistryStrategyTest, CheckArgsTest2) {
  node.__set_fweight(-1);
  EXPECT_EQ(ERR_INVALID_WEIGHT, register_strategy_.CheckArgs(node));
}

TEST_F(RegistryStrategyTest, CheckArgsTest3) {
  node.__set_ip("10.4");
  EXPECT_EQ(ERR_INVALID_PORT, register_strategy_.CheckArgs(node));
}

TEST_F(RegistryStrategyTest, CheckArgsTest3) {
  node.__set_port(-1);
  EXPECT_EQ(ERR_INVALID_PORT, register_strategy_.CheckArgs(node));
}

TEST_F(RegistryStrategyTest, IsAllowMacRegisterTest) {
  CXmlFile::GetAppenv()->SetTypeEnv(Appenv::DEV);
  node.__set_appkey("octo.naming.travel.dsg.trace");
  EXPECT_TRUE(register_strategy_.IsAllowMacRegister(node));


}
TEST_F(RegistryStrategyTest, IsAllowMacRegisterTest1) {
  CXmlFile::GetAppenv()->SetTypeEnv(Appenv::TEST);
  EXPECT_TRUE(register_strategy_.IsAllowMacRegister(node));

}

TEST_F(RegistryStrategyTest, IsAllowMacRegisterTest2) {
  CXmlFile::GetAppenv()->SetTypeEnv(Appenv::TEST);
  node.__set_appkey("octo.naming.travel.dsg.trace");
  EXPECT_TRUE(register_strategy_.IsAllowMacRegister(node));
}

TEST_F(RegistryStrategyTest, RegistryFilterByCacheTest) {
  EXPECT_EQ(0, register_strategy_.RegistryFilterByCache(node));
  node.__set_appkey("test-benchmark-web");//双框架appkey
  EXPECT_EQ(-1, register_strategy_.RegistryFilterByCache(node));
}

TEST_F(RegistryStrategyTest,RegistryFilterCheckTest) {
  node.__set_appkey("octo.naming.inf.sg_agent");
  EXPECT_TRUE(register_strategy_.RegistryFilterCheck(node));
  node.__set_appkey("octo.naming.service.yangjie");//开启强制
  EXPECT_TRUE(register_strategy_.RegistryFilterCheck(node));
}

TEST_F(RegistryStrategyTest, IsLimitOnZkTest){
  node.__set_appkey("octo.naming.service.yangjie");//开启强制
  EXPECT_TRUE(register_strategy_.IsLimitOnZk(node));
  node.__set_appkey("test");
  EXPECT_FALSE(register_strategy_.IsLimitOnZk(node));
}

TEST_F(RegistryStrategyTest, CheckLegalOnOpsTest) {
  node.__set_appkey("test");
  EXPECT_FALSE(register_strategy_.CheckLegalOnOps(node));
  node.__set_appkey("octo.naming.service.tmy");
  EXPECT_TRUE(register_strategy_.CheckLegalOnOps(node));
}
#endif
