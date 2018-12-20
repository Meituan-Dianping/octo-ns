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
#define private public
#define protected public
#include "version_manager.h"
#undef private
#undef protected

using namespace meituan_mns;

class ZKVersionCheck : public testing::Test {
 public:
  VersionManager version_;
};

TEST_F(ZKVersionCheck, CheckZkVersion) {
  std::string appkey = "test1";
  std::string zk_version = "1.2";
  std::string version = "2.3";

  EXPECT_EQ(0, version_.CheckZkVersion(appkey, zk_version, version));
  usleep(100);
  EXPECT_EQ(ERR_ZK_LIST_SAME_BUFFER, version_.CheckZkVersion(appkey, zk_version, version));

}

TEST_F(ZKVersionCheck, GetVersionTest) {
  const std::string version = "dorado-v1.7.0";
  std::vector<int> test;
  version_.GetVersion(version, &test);
  EXPECT_EQ(3, test.size());
}

TEST_F(ZKVersionCheck, IsOldVersionTest) {
  const std::string version = "dorado-v1.7.0";
  EXPECT_FALSE(version_.IsOldVersion(version));

  const std::string version_2 = "dorado-v1.7.0-SNAPSHOT";
  EXPECT_FALSE(version_.IsOldVersion(version_2));

  std::string version_3 = "dorado-v1.7.3";
  EXPECT_FALSE(version_.IsOldVersion(version_3));

  version_3 = "dorado-v1.17.3";
  EXPECT_FALSE(version_.IsOldVersion(version_3));
}

TEST_F(ZKVersionCheck, OldVersion) {
  std::string version = "dorado-v1.6.4-SNAPSHOT";
  EXPECT_TRUE(version_.IsOldVersion(version));

  version = "dorado-v1.6.3-SNAPSHOT";
  EXPECT_TRUE(version_.IsOldVersion(version));

  version = "dorado-v1.5.7";
  EXPECT_TRUE(version_.IsOldVersion(version));
}

TEST_F(ZKVersionCheck, dorado) {
  EXPECT_TRUE(version_.IsOldVersion("2.7.9"));
  EXPECT_TRUE(version_.IsOldVersion("dorado-v2.7.9"));
  EXPECT_TRUE(version_.IsOldVersion("1.1.0"));
  EXPECT_TRUE(version_.IsOldVersion("dorado-v1.1.0"));

  EXPECT_FALSE(version_.IsOldVersion("2.8.0"));
  EXPECT_FALSE(version_.IsOldVersion("dorado-v2.8.0"));
  EXPECT_FALSE(version_.IsOldVersion("2.8.1"));
  EXPECT_FALSE(version_.IsOldVersion("2.10.0"));
  EXPECT_FALSE(version_.IsOldVersion("dorado-v2.8.1"));
  EXPECT_FALSE(version_.IsOldVersion("dorado-v2.10.1"));
}

TEST_F(ZKVersionCheck, other) {
  std::string version = "cthrift";
  EXPECT_FALSE(version_.IsOldVersion(version));

  version = "pthrift";
  EXPECT_FALSE(version_.IsOldVersion(version));

  version = "";
  EXPECT_TRUE(version_.IsOldVersion(version));
}
