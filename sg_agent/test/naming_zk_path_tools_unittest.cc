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
#include "zk_path_tools.h"
#undef private
#undef protected

using namespace meituan_mns;
class AgentZkPathTest : public testing::Test {
 public:
  void SetUp() {
    m_root_path = "/octo/naming/test";
    appkey = "octo.naming.inf.tmy";

    provider = m_root_path + "/" + appkey + "/provider";
    http = m_root_path + "/" + appkey + "/provider-http";
    tair = m_root_path + "/" + appkey + "/providers/tair";
    other = m_root_path + "/" + appkey + "/providers/other";
    remoteAppkey_ = "octo.naming.inf.logCollector";
  }
 public:
  AgentZkPath agent_zk_path_;
  std::string m_root_path;
  std::string appkey;

  std::string provider;
  std::string http;
  std::string tair;
  std::string other;
  std::string remoteAppkey_;
};


TEST_F(AgentZkPathTest, GenProtocolZkPathTest) {
  std::string zk_path = "";
  std::string nodeType = "provider";
  std::string appkey = "octo.naming.inf.tmy";
  EXPECT_EQ(0, agent_zk_path_.GenProtocolZkPath(zk_path, appkey, "thrift",
  nodeType));
  appkey = "";
  EXPECT_EQ(-1, agent_zk_path_.GenProtocolZkPath(zk_path, appkey, "thrift",
  nodeType));
  EXPECT_EQ(-1, agent_zk_path_.GenProtocolZkPath(zk_path, appkey, "", nodeType));
}

TEST_F(AgentZkPathTest, GenDescZkPathTest) {
  std::string zk_path = "";
  std::string castle_appkey("octo.naming.inf.dorado.heartbeat");
  int ret =
      agent_zk_path_.GenDescZkPath(zk_path, castle_appkey);
  EXPECT_EQ(SUCCESS, ret);

  std::string zk_path_str(zk_path);

  EXPECT_FALSE(zk_path_str == "/octo/naming/test/octo.naming.inf.dorado.heartbeat"
  "/desc");

  std::string octo_tmy_appkey("octo.naming.octo.tmy");
  ret = agent_zk_path_.GenDescZkPath(zk_path, octo_tmy_appkey);
  EXPECT_EQ(SUCCESS, ret);

  std::string zk_path_str1(zk_path);
  EXPECT_TRUE(zk_path_str1 == "/octo/naming/prod/octo.naming.octo.tmy/desc");
}

TEST_F(AgentZkPathTest, GenServiceNameZkPathNodeTest) {
  std::string zk_path = "";
  std::string castle_appkey("octo.naming.inf.dorado.heartbeat");
  std::string servicename("test_service_name");
  std::string protocol("thrift");
  int ret =
      agent_zk_path_.GenServiceNameZkPathNode(zk_path, servicename, protocol,
                                              castle_appkey);
  EXPECT_EQ(SUCCESS, ret);

  std::string zk_path_str(zk_path);

  EXPECT_TRUE(zk_path_str == "/mns/service/prod/test_service_name/thrift");

  std::string octo_tmy_appkey("octo.naming.octo.tmy");
  ret = agent_zk_path_.GenServiceNameZkPathNode(zk_path, servicename, protocol,
                                                octo_tmy_appkey);
  EXPECT_EQ(SUCCESS, ret);

  std::string zk_path_str1(zk_path);
  std::cout << zk_path_str1 << std::endl;
  EXPECT_TRUE(zk_path_str1 == "/mns/service/prod/test_service_name/thrift");
}


TEST_F(AgentZkPathTest, GenRegisterZkPathTest) {
  std::string zk_path = "";
  std::string castle_appkey("octo.naming.inf.dorado.heartbeat");
  std::string protocol("thrift");
  int server_type = 1;
  int ret =
      agent_zk_path_.GenRegisterZkPath(zk_path, castle_appkey, protocol,
                                       server_type);
  EXPECT_EQ(SUCCESS, ret);

  std::string zk_path_str(zk_path);

  EXPECT_FALSE(zk_path_str == "/octo/naming/prod/octo.naming.inf.dorado.heartbeat"
  "/provider");

  std::string octo_tmy_appkey("octo.naming.octo.tmy");
  ret = agent_zk_path_.GenRegisterZkPath(zk_path, octo_tmy_appkey, protocol,
                                         server_type);
  EXPECT_EQ(SUCCESS, ret);

  std::string zk_path_str1(zk_path);
  EXPECT_TRUE(zk_path_str1 == "/octo/naming/prod/octo.naming.octo.tmy/provider");
}

TEST_F(AgentZkPathTest, GenProtocolZkPathTest1) {
  std::string zk_path = "";
  std::string castle_appkey("octo.naming.inf.dorado.heartbeat");
  std::string protocol("thrift");
  std::string provider("provider");
  int ret =
      agent_zk_path_.GenProtocolZkPath(zk_path, castle_appkey, protocol,
                                       provider);
  EXPECT_EQ(SUCCESS, ret);

  std::string zk_path_str(zk_path);

  EXPECT_FALSE(zk_path_str == "/octo/naming/prod/octo.naming.inf.dorado."
  "heartbeat/provider");

  std::string octo_tmy_appkey("octo.naming.octo.tmy");
  ret = agent_zk_path_.GenProtocolZkPath(zk_path, octo_tmy_appkey, protocol,
                                         provider);
  EXPECT_EQ(SUCCESS, ret);

  std::string zk_path_str1(zk_path);
  EXPECT_TRUE(zk_path_str1 == "/octo/naming/prod/octo.naming.octo.tmy/provider");
}

TEST_F(AgentZkPathTest, GenRegisterZkPathTest1) {
  std::string zk_path = "";
  EXPECT_LE(0, agent_zk_path_.GenRegisterZkPath(zk_path, appkey, "thrift", 0));
  zk_path = "";
  EXPECT_LE(0, agent_zk_path_.GenRegisterZkPath(zk_path, appkey, "", 0));


  //验证serverType无关
  EXPECT_LE(0, agent_zk_path_.GenRegisterZkPath(zk_path, appkey, "thrift", 1));

  //验证serverType无关

  EXPECT_LE(0, agent_zk_path_.GenRegisterZkPath(zk_path, appkey, "http", 0));
  //EXPECT_STREQ(http.c_str(), zkPath);


  EXPECT_LE(0, agent_zk_path_.GenRegisterZkPath(zk_path, appkey, "http", 1));
  //EXPECT_STREQ(http.c_str(), zkPath_2);

  EXPECT_LE(0, agent_zk_path_.GenRegisterZkPath(zk_path, appkey, "", 1));

  EXPECT_LE(0, agent_zk_path_.GenRegisterZkPath(zk_path, appkey, "tair", 0));

  EXPECT_LE(0, agent_zk_path_.GenRegisterZkPath(zk_path, appkey, "tair", 1));
}


TEST_F(AgentZkPathTest, GenServiceNameZkPathNodeTest1) {
  std::string zk_path = "";
  std::string appkey = "octo.naming.inf.sg_agent";
  std::string serviceName = "sayHello";

  EXPECT_LE(0, agent_zk_path_.GenServiceNameZkPathNode(zk_path, serviceName,
  "thrift",appkey));
  std::cout << "zkPath = " << zk_path << std::endl;
  EXPECT_STREQ(zk_path.c_str(), "/mns/service/prod/sayHello/thrift");

  EXPECT_LE(0, agent_zk_path_.GenServiceNameZkPathNode(zk_path, serviceName,
  "http", appkey));
  EXPECT_STREQ(zk_path.c_str(), "/mns/service/prod/sayHello/http");


  appkey = "octo.naming.inf.sg_agent";
  serviceName = "sayHello";

  EXPECT_LE(0, agent_zk_path_.GenServiceNameZkPathNode(zk_path,
      serviceName,
      appkey));
  std::cout << "zkPath = " << zk_path << std::endl;
  EXPECT_STREQ(zk_path.c_str(), "/mns/service/prod/sayHello");

  EXPECT_LE(0, agent_zk_path_.GenServiceNameZkPathNode(zk_path,
      serviceName,
      appkey));
  EXPECT_STREQ(zk_path.c_str(), "/mns/service/prod/sayHello");
}

TEST_F(AgentZkPathTest, parse_xml) {
  std::string char_with_empty = "        ";
  boost::trim(char_with_empty);
  EXPECT_TRUE("" == char_with_empty);
  std::string char_with_empty1 = "    s    ";
  boost::trim(char_with_empty1);
  EXPECT_TRUE("s" == char_with_empty1);
}

TEST_F(AgentZkPathTest, thrift_provider) {
  std::string zk_path = "/octo/naming/prod/octo.naming.inf.logCollector/provider";
  std::string appkey = "";
  std::string protocol = "";
  EXPECT_EQ(0, AgentZkPath::DeGenZkPath(zk_path.c_str(), appkey, protocol));

  EXPECT_STREQ(appkey.c_str(), appkey.c_str());
  EXPECT_STREQ("thrift", protocol.c_str());
}

TEST_F(AgentZkPathTest, http_provider) {
  std::string zk_path = "/octo/naming/prod/octo.naming.inf.logCollector/"
      "provider-http";
  std::string appkey = "";
  std::string protocol = "";
  EXPECT_EQ(0, AgentZkPath::DeGenZkPath(zk_path.c_str(), appkey, protocol));

  EXPECT_STREQ(remoteAppkey_.c_str(), appkey.c_str());
  EXPECT_STREQ("http", protocol.c_str());
}

TEST_F(AgentZkPathTest, cellar_provider) {
  std::string zk_path = "/octo/naming/prod/octo.naming.inf.logCollector/"
      "providers"
      "/cellar";
  std::string appkey = "";
  std::string protocol = "";
  EXPECT_EQ(0, AgentZkPath::DeGenZkPath(zk_path.c_str(), appkey, protocol));

  EXPECT_STREQ(remoteAppkey_.c_str(), appkey.c_str());
  EXPECT_STREQ("cellar", protocol.c_str());
}

TEST_F(AgentZkPathTest, thrift_route) {
  std::string zk_path = "/octo/naming/prod/octo.naming.inf.logCollector/route";
  std::string appkey = "";
  std::string protocol = "";
  EXPECT_EQ(0, AgentZkPath::DeGenZkPath(zk_path.c_str(), appkey, protocol));

  EXPECT_STREQ(remoteAppkey_.c_str(), appkey.c_str());
  EXPECT_STREQ("thrift", protocol.c_str());
}

TEST_F(AgentZkPathTest, http_route) {
  std::string zk_path = "/octo/naming/prod/octo.naming.inf.logCollector"
      "/route-http";
  std::string appkey = "";
  std::string protocol = "";
  EXPECT_EQ(0, AgentZkPath::DeGenZkPath(zk_path.c_str(), appkey, protocol));

  EXPECT_STREQ(remoteAppkey_.c_str(), appkey.c_str());
  EXPECT_STREQ("http", protocol.c_str());
}

TEST_F(AgentZkPathTest, cellar_route) {
  std::string zk_path = "/octo/naming/prod/octo.naming.inf.logCollector"
      "/providers/cellar";
  std::string appkey = "";
  std::string protocol = "";
  EXPECT_EQ(0, AgentZkPath::DeGenZkPath(zk_path.c_str(), appkey, protocol));

  EXPECT_STREQ(remoteAppkey_.c_str(), appkey.c_str());
  EXPECT_STREQ("cellar", protocol.c_str());
}

TEST_F(AgentZkPathTest, invalid_path) {
  std::string appkey = "";
  std::string protocol = "";
  //empty
  std::string zk_path= "";
  EXPECT_EQ(-1, AgentZkPath::DeGenZkPath(zk_path.c_str(), appkey, protocol));

  //not complete
  zk_path = "/octo/naming/prod";
  EXPECT_EQ(-2, AgentZkPath::DeGenZkPath(zk_path.c_str(), appkey, protocol));

  zk_path = "/octo/naming/prod////";
  EXPECT_EQ(-3, AgentZkPath::DeGenZkPath(zk_path.c_str(), appkey, protocol));
}


