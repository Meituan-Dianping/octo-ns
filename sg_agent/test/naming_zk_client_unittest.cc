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
#include "zk_client.h"
#include "zk_client_pool.h"
#undef private
#undef protected

using namespace meituan_mns;

class ZkClientTest : public testing::Test {
 public:
  void SetUp() {
    zk_client_ = new ZkClient();
    zk_client_pool_ = ZkClientPool::GetInstance();
    zk_list = "127.0.0.1:2181";
    zk_timeout = 300000;
    zk_retrytimes = 3;
  }
 public:
  ZkClient *zk_client_;
  ZkClientPool *zk_client_pool_;
  std::string zk_list;
  int zk_timeout;
  int zk_retrytimes;
};

TEST_F(ZkClientTest, ZkInitTest) {
  EXPECT_EQ(0, zk_client_->ZkInit(zk_client_, zk_list, zk_timeout, zk_retrytimes));
  EXPECT_EQ(0, zk_client_->ZkClose());
}

TEST_F(ZkClientTest, Connect2ZkTest) {
  EXPECT_EQ(0, zk_client_->Reconnect2Zk());
  EXPECT_EQ(0, zk_client_->ZkClose());
}

TEST_F(ZkClientTest, CheckZkConnTest) {
  EXPECT_EQ(0, zk_client_->CheckZkConn());
  EXPECT_EQ(0, zk_client_->ZkClose());
}

TEST_F(ZkClientTest, InitTest) {
  EXPECT_EQ(-1, zk_client_pool_->Init(zk_list, 0, zk_timeout, zk_retrytimes));
  EXPECT_EQ(-1, zk_client_pool_->Init(zk_list, 1, zk_timeout, zk_retrytimes));
}

TEST_F(ZkClientTest, GetZkClientForHashTest) {
  ZkClient *zk = zk_client_pool_->GetZkClientForHash(0);
  if (zk == NULL) {
    EXPECT_EQ(0,-1);
  } else {
    EXPECT_EQ(0,0);
  }
  EXPECT_TRUE(zk_client_pool_->started());
}

