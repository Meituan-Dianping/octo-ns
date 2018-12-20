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

#ifndef OCTO_OPEN_SOURCE_CONFIG_LOADER_H
#define OCTO_OPEN_SOURCE_CONFIG_LOADER_H
#include "tinyxml2.h"
#include "route_base.h"
#include <string>
#include <vector>
#include <muduo/base/Mutex.h>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
namespace meituan_mns {

typedef enum {
  PROD, STAGING, DEV, PPE, TEST
} Appenv;


typedef boost::shared_ptr<boost::unordered_set<std::string> > UnorderedMapPtr;
typedef boost::shared_ptr<boost::unordered_map<std::string, std::string> >
    MccClusterAppkeyPtr;

class AppEnv {
 public:

  int32_t GetIntEnv(void) { return int_env; }
  std::string GetStrEnv(void) { return str_env; }
  Appenv GetTypeEnv(void) { return type_env; }
  bool GetEnvFlag(void) { return is_online; }

  void SetIntEnv(const int32_t env) { int_env = env;}
  void SetStrEnv(const std::string &env) { str_env = env;}
  void SetTypeEnv(Appenv &env) { type_env = env;}
  void SetEnvFlag(const bool flag) {is_online = flag;}

 private:
  int32_t int_env;
  std::string str_env;
  Appenv type_env;
  bool is_online;
};

typedef boost::shared_ptr<AppEnv> AppEnvPtr;
typedef boost::shared_ptr<std::vector<boost::shared_ptr<IDC> > > IdcsPtr;
class CXmlFile {

 public:

  enum Type {
    AgentAppKey = 0,
    AgentVersion,
    AgentPort,
    DefaultMnsPath,
    RegistryThreads,
    ClientNum,
    ClientThreads,
    WatcherPullThreads,
    WatcherDispatchThreads,
    FastGetThreads,
    ZkTimeout,
    ZkClientNum,
    RetryTimes,
    RegistryCenter,
    CacheDiscSwitch,
    ZkLists,
    MNSCacheAppkey,
    MnscTimeOut,
    RegistryEntry,
    DiscoveryType,
    LocalIp,
    LocalMask,
    AllEnvWhiteLists,
    NoWatcherWhiteLists,
    RegisteUnlimitWhiteList,
    MacRegisterUnlimitAppkeys,
    OpenMNSCache,
    OpenAutoRoute,
    AgentLog,
    AgentLogPath
  };



  CXmlFile();
  ~CXmlFile() {}

  /**
   * 初始化组件依赖的配置项
   * @param void
   *
   * @return int32_t
   *
   * */
  int32_t CXmlFileInit();

  /**
   * 根据配置索引获取Int型配置参数
   * @param index
   *
   * @return int
   *
   * */
  static int32_t GetI32Para(const CXmlFile::Type type);

  /**
   * 获取字符串类型配置
   * @param void
   *
   * @return string
   *
   * */
  static const std::string GetStrPara(const CXmlFile::Type type);
  /**
   * 获取日志路径
   * @param void
   *
   * @return string
   *
   * */
  const std::string GetLogPath();
  /**
   *
   * @param p_xml_elem
   * @param str_key
   * @return
   */
  std::string FetchElemValByTinyXML(const tinyxml2::XMLElement *p_xml_elem,
                                    const std::string &str_key);

  /** 获取降级开关标示 */
  static bool GetAgentFunFlag(const CXmlFile::Type type);
  /** 获取对应appkeys列表 */
  static UnorderedMapPtr GetWhiteAppkeys(const CXmlFile::Type type);
  /** 获取当前机器环境 */
  static AppEnvPtr GetAppenv(void) { return env_;}
  /** 获取mac注册限制的网段*/
  static IdcsPtr GetRegisterIdc(void) { return register_whitelist_idcs_; }
  /** 获取idc文件内容 */
  static IdcsPtr GetIdc(void);

  void UpdateIdcsTimer(void);

	void UpdateIdcs();

 private:

  /**
   * 初始化机器环境
   * @return
   */
  int32_t InitAppenv(void);
  int32_t InitAddrInfo(void);
  /**
   *
   * @param env_str
   * @param deployenv_str
   * @return
   */
  int32_t InitEnv(const std::string &env_str, const std::string &deployenv_str);
  /** 加载mutable文件 */
  int32_t LoadMutableFile(void);
  /** 加载white白名单文件 */
  int32_t LoadWhiteFile(void);

  /**
   *
   * @param agent_xml_conf
   * @param type
   * @return
   */
  int32_t LoadWhiteAppkeys(const tinyxml2::XMLElement *agent_xml_conf,
                           CXmlFile::Type type);
  /**
   *
   * @param agent_xml_conf
   * @return
   */
  int32_t LoadAgentFuncFlag(const tinyxml2::XMLElement *agent_xml_conf);

  /**
   *
   * @param sg_agent_idc
   * @return
   */
  int32_t LoadRegisterWhitelistIdc(const tinyxml2::XMLElement *sg_agent_idc);
  /**
   *
   * @return
   */
  int32_t LoadIdcFile(IdcsPtr &idcs);
  /**
   *
   * @param sg_agent_conf
   * @return
   */
  int32_t LoadZkConfig(const tinyxml2::XMLElement *sg_agent_conf);

 private:
  CXmlFile(const CXmlFile &);
  CXmlFile &operator=(const CXmlFile &);



  static boost::unordered_map<CXmlFile::Type, int32_t> global_int_params_;
  static boost::unordered_map<CXmlFile::Type, std::string> global_str_params_;
  static boost::unordered_map<CXmlFile::Type, UnorderedMapPtr> white_lists_;
  static boost::unordered_map<CXmlFile::Type, bool> sg_agent_fun_;

  static IdcsPtr register_whitelist_idcs_;
  static IdcsPtr idcs_;

  static AppEnvPtr env_;

  static muduo::MutexLock update_idc_lock_;

  std::string idc_md5_;

 protected:
};
}  //  namespace meituan_mns

#endif  //  OCTO_OPEN_SOURCE_CONFIG_LOADER_H
