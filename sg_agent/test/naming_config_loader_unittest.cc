
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
#include <boost/make_shared.hpp>
#define private public
#define protected public
#include "config_loader.h"
#undef private
#undef protected
#include "base_consts.h"
#include "base_errors_consts.h"

using namespace meituan_mns;

class ConfigLoaderTest : public testing::Test {

 public:
  void SetUp() {
  }
 public:
  CXmlFile cxml_file_;

};

TEST_F(ConfigLoaderTest, CXmlFileInitTest) {

  EXPECT_EQ(0,cxml_file_.CXmlFileInit());
}


TEST_F(ConfigLoaderTest, InitEnvTest) {
  std::string env_str = "test";
  std::string deployenv_str = "test";
  EXPECT_EQ(0, cxml_file_.InitEnv(env_str, deployenv_str));

  env_str = "unknown";
  EXPECT_EQ(0, cxml_file_.InitEnv(env_str, deployenv_str));
  deployenv_str = "unknown";
  EXPECT_EQ(-102010, cxml_file_.InitEnv(env_str, deployenv_str));
}

TEST_F(ConfigLoaderTest, LoadMutableFileTest) {
  EXPECT_EQ(0, cxml_file_.LoadMutableFile());

}

TEST_F(ConfigLoaderTest, LoadWhiteFileTest) {
  EXPECT_EQ(0, cxml_file_.LoadWhiteFile());

}

TEST_F(ConfigLoaderTest ,InitAddrInfo) {
  EXPECT_EQ(0, cxml_file_.InitAddrInfo());
  EXPECT_STREQ("127.0.0.1", CXmlFile::GetStrPara(CXmlFile::LocalIp).c_str());
  EXPECT_STREQ("255.255.255.0",CXmlFile::GetStrPara(CXmlFile::LocalMask).c_str());
}

TEST_F(ConfigLoaderTest, LoadWhiteAppkeysTest) {
  tinyxml2::XMLDocument agent_whitelist;
  const tinyxml2::XMLError agent_xml_ret =
      agent_whitelist.LoadFile(AGENT_WHITELIST_CONF.c_str());

  if (tinyxml2::XML_SUCCESS == agent_xml_ret) {
    const tinyxml2::XMLElement *agent_xml_conf =
        agent_whitelist.FirstChildElement("SGAgentWhiteListConf");
    if (NULL != agent_xml_conf) {
      EXPECT_EQ(0, cxml_file_.LoadWhiteAppkeys(agent_xml_conf,
          CXmlFile::MacRegisterUnlimitAppkeys));
      EXPECT_EQ(0, cxml_file_.LoadWhiteAppkeys(agent_xml_conf,
          CXmlFile::AllEnvWhiteLists));
      EXPECT_EQ(0, cxml_file_.LoadWhiteAppkeys(agent_xml_conf,
          CXmlFile::RegisteUnlimitWhiteList));
      EXPECT_EQ(0, cxml_file_.LoadWhiteAppkeys(agent_xml_conf,
          CXmlFile::NoWatcherWhiteLists));
      CXmlFile::Type type = static_cast<CXmlFile::Type>(-1);
      EXPECT_EQ(-1, cxml_file_.LoadWhiteAppkeys(agent_xml_conf, type));
    }
  }
}

TEST_F(ConfigLoaderTest, LoadAgentFuncFlagTest) {

  tinyxml2::XMLDocument agent_mutable;
  const tinyxml2::XMLError agent_xml_ret =
      agent_mutable.LoadFile(AGENT_MUTABLE_CONF.c_str());
  if (tinyxml2::XML_SUCCESS == agent_xml_ret) {
    const tinyxml2::XMLElement *agent_xml_conf =
        agent_mutable.FirstChildElement("SGAgentMutableConf");
    if (NULL != agent_xml_conf) {
      EXPECT_EQ(0, cxml_file_.LoadAgentFuncFlag(agent_xml_conf));
    }
  }

}

TEST_F(ConfigLoaderTest, LoadRegisterWhitelistIdcTest) {
  tinyxml2::XMLDocument agent_whitelist;
  const tinyxml2::XMLError agent_xml_ret =
      agent_whitelist.LoadFile(AGENT_WHITELIST_CONF.c_str());

  if (tinyxml2::XML_SUCCESS == agent_xml_ret) {
  const tinyxml2::XMLElement *agent_xml_conf =
      agent_whitelist.FirstChildElement("SGAgentWhiteListConf");
    if (NULL != agent_xml_conf) {
      EXPECT_EQ(0, cxml_file_.LoadRegisterWhitelistIdc(agent_xml_conf));
    }
  }
}


TEST_F(ConfigLoaderTest, LoadZkConfigTest) {
  tinyxml2::XMLDocument agent_mutable;
  const tinyxml2::XMLError agent_xml_ret =
      agent_mutable.LoadFile(AGENT_MUTABLE_CONF.c_str());
  if (tinyxml2::XML_SUCCESS == agent_xml_ret) {
    const tinyxml2::XMLElement *agent_xml_conf =
        agent_mutable.FirstChildElement("SGAgentMutableConf");
    if (NULL != agent_xml_conf) {
      EXPECT_EQ(0, cxml_file_.LoadZkConfig(agent_xml_conf));
    }
  }

}

TEST_F(ConfigLoaderTest, LoadIdcFileTest) {
  IdcsPtr idcs = boost::make_shared<std::vector<boost::shared_ptr<IDC> > >();
  EXPECT_EQ(0, cxml_file_.LoadIdcFile(idcs));
  if (idcs.get() != NULL) {
    EXPECT_EQ(0,0);
  }
}
