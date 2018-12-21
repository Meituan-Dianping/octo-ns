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
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#define private public
#define protected public
#include "whitelist_manager.h"
#undef private
#undef protected


using namespace meituan_mns;
class WhiteListManagerTest : public testing::Test {
 public:
  void SetUp() {
  }
 public:
  WhiteListManager whitelist_manager_;
};

TEST_F(WhiteListManagerTest, IsAppkeyInWhitListTest){
  std::string not_appkey = "octo.naming.tmy";
  std::string yes_appkey = "octo.naming.inf.logCollector";
  EXPECT_TRUE(whitelist_manager_.IsAppkeyInWhitList(yes_appkey));
  EXPECT_FALSE(whitelist_manager_.IsAppkeyInWhitList(not_appkey));
}

TEST_F(WhiteListManagerTest, IsAppkeyInRegistUnlimitWhitListTest) {
  std::string yes_appkey = "octo.naming.inf.sg_agent";
  std::string no_appkey = "octo.naming.test";
  EXPECT_FALSE(whitelist_manager_.IsAppkeyInRegistUnlimitWhitList(no_appkey));

  EXPECT_TRUE(whitelist_manager_.IsAppkeyInRegistUnlimitWhitList(yes_appkey));

}

TEST_F(WhiteListManagerTest, IsAppkeyInAllEnvWhitListTest) {
  std::string yes_appkey = "octo.naming.inf.mafka.castle";
  std::string no_appkey = "octo.naming.test";
  EXPECT_TRUE(whitelist_manager_.IsAppkeyInAllEnvWhitList(yes_appkey));
  EXPECT_FALSE(whitelist_manager_.IsAppkeyInAllEnvWhitList(no_appkey));
}


TEST_F(WhiteListManagerTest, IsMacRegisterAppkeyTest) {
  std::string yes_appkey = "octo.naming.flight.basis.linkstar";
  std::string no_appkey = "octo.naming.tmy";
  EXPECT_TRUE(whitelist_manager_.IsMacRegisterAppkey(yes_appkey));
  EXPECT_FALSE(whitelist_manager_.IsMacRegisterAppkey(no_appkey));
}

TEST_F(WhiteListManagerTest, IsAppkeyContainsTest) {
  std::string appkey = "";
  UnorderedMapPtr set_ptr = boost::make_shared<boost::unordered_set<std::string> >();
  EXPECT_FALSE(whitelist_manager_.IsAppkeyContains(appkey, set_ptr));
}

TEST_F(WhiteListManagerTest, IsAppkeyListTest) {
  std::string appkey = "";
  UnorderedMapPtr set_ptr = boost::make_shared<boost::unordered_set<std::string> >();
  EXPECT_FALSE(whitelist_manager_.IsAppkeyList(appkey, set_ptr));
}
