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

#include <string>
#include <boost/make_shared.hpp>
#include "config_loader.h"
#include "base_consts.h"
#include "inc_comm.h"
#include "route_info.h"
#include "registry_strategy.h"

namespace meituan_mns {

boost::unordered_map<CXmlFile::Type, int32_t> CXmlFile::global_int_params_;
boost::unordered_map<CXmlFile::Type, std::string> CXmlFile::global_str_params_;
boost::unordered_map<CXmlFile::Type, bool> CXmlFile::sg_agent_fun_;
boost::shared_ptr<std::vector<boost::shared_ptr<IDC> > > CXmlFile::register_whitelist_idcs_;
boost::shared_ptr<std::vector<boost::shared_ptr<IDC> > > CXmlFile::idcs_;
boost::unordered_map<CXmlFile::Type, UnorderedMapPtr> CXmlFile::white_lists_;
AppEnvPtr CXmlFile::env_;

muduo::MutexLock CXmlFile::update_idc_lock_;

CXmlFile::CXmlFile():idc_md5_("") {
}
std::string CXmlFile::FetchElemValByTinyXML(const tinyxml2::XMLElement *p_xml_elem,
                                            const std::string &str_key) {
  const tinyxml2::XMLElement *p_xml_elem_key =
      p_xml_elem->FirstChildElement(str_key.c_str());
  if (NULL == p_xml_elem_key) {
    NS_LOG_ERROR(str_key << " not find.");
    return "";
  }
  NS_LOG_DEBUG("Fetch " << str_key << " value: " << p_xml_elem_key->GetText());

  return std::string(p_xml_elem_key->GetText());
}

int32_t CXmlFile::CXmlFileInit() {
  register_whitelist_idcs_ =
      boost::make_shared<std::vector<boost::shared_ptr<IDC> > >();
  idcs_ = boost::make_shared<std::vector<boost::shared_ptr<IDC> > >();
  env_ = boost::make_shared<AppEnv>();

  int32_t ret = InitAppenv();
  if (SUCCESS != ret) {
    NS_LOG_ERROR("failed to load appenv, ret = " << ret);
    return ret;
  }
  ret = InitAddrInfo();
  if (SUCCESS != ret) {
    NS_LOG_ERROR("failed to get local ip.");
    return ret;
  }
  ret = LoadIdcFile(idcs_);
  if (SUCCESS != ret) {
    NS_LOG_ERROR("failed to load idc.");
    return ret;
  }
  ret = LoadMutableFile();
  if (SUCCESS != ret) {
    NS_LOG_ERROR("failed to load mutable.");
    return ret;
  }
  ret = LoadWhiteFile();
  if (SUCCESS != ret) {
    NS_LOG_ERROR("failed to load whitelist.");
    return ret;
  }
  return ret;
}


int32_t CXmlFile::InitAppenv() {

  std::ifstream appenv_fin;
  std::string str_agent_env_ = "";
  std::string str_agent_deployenv_ = "";

  try {
    appenv_fin.open(APPENV_FILE.c_str(), std::ios::in);
    if (!appenv_fin.is_open()) {
      NS_LOG_ERROR("failed to init appenv, there is not"
                         << APPENV_FILE);
      return ERR_INVALID_ENV;
    }

    std::string env_str;
    while (std::getline(appenv_fin, env_str) &&
      (str_agent_env_.empty() || str_agent_deployenv_.empty())) {
      std::size_t pos = env_str.find_first_of("=");
      if (std::string::npos != pos) {
        std::string key = env_str.substr(0, pos);
        std::string value = env_str.substr(pos + 1);
        boost::trim(key);
        boost::trim(value);
        if ("env" == key) {
          str_agent_env_ = value;
          NS_LOG_INFO("parsing env:" << str_agent_env_);
        } else if ("deployenv" == key) {
          str_agent_deployenv_ = value;
          NS_LOG_INFO("parsing deployenv:" << str_agent_deployenv_);
        }
      }
      env_str.clear();
    }
    appenv_fin.close();

  } catch (std::exception &e) {
    appenv_fin.close();
    NS_LOG_ERROR("fail to load " << APPENV_FILE
                                   << "OR fetch deployenv/appenv failed, reason: "
                                   << e.what());
    return ERR_INVALID_ENV;
  }
  transform(str_agent_deployenv_.begin(),
      str_agent_deployenv_.end(),
      str_agent_deployenv_.begin(), ::tolower);

  transform(str_agent_env_.begin(),
      str_agent_env_.end(),
      str_agent_env_.begin(), ::tolower);

  NS_LOG_INFO("get env = " << str_agent_env_
                             << ", deployenv = " << str_agent_deployenv_
                             << " from " << meituan_mns::APPENV_FILE);
  if(InitEnv(str_agent_env_,str_agent_deployenv_) < 0){
    NS_LOG_ERROR("the env info error, str_agent_env_: "
                       <<str_agent_env_
                       << "; str_agent_deployenv_"
                       <<str_agent_deployenv_);
    return -1;
  }
  return 0;
}
// 优先解释env字段，无法解释时，再解释deployenv。请勿优化以下if语句
int32_t CXmlFile::InitEnv(const std::string &env_str,
                          const std::string &deployenv_str) {
  NS_LOG_INFO("start to parse the host env, env = "
                    << env_str << ", deployenv = "
                    << deployenv_str);
  Appenv g_appenv = DEV;
  int32_t local_env_int= 0;
  std::string local_env_str = "";
  bool is_online = false;

  if ("prod" == env_str) {
    g_appenv = PROD;
  } else if ("staging" == env_str) {
    g_appenv = STAGING;
  } else if ("dev" == env_str) {
    g_appenv = DEV;
  } else if ("ppe" == env_str) {
    g_appenv = PPE;
  } else if ("test" == env_str) {
    g_appenv = TEST;
  } else if ("product" == deployenv_str || "prod" == deployenv_str) {
    g_appenv = PROD;
  } else if ("staging" == deployenv_str) {
    g_appenv = STAGING;
  } else if ("dev" == deployenv_str || "alpha" == deployenv_str) {
    g_appenv = DEV;
  } else if ("ppe" == deployenv_str || "prelease" == deployenv_str) {
    g_appenv = PPE;
  } else if ("qa" == deployenv_str || "test" == deployenv_str) {
    g_appenv = TEST;
  } else {
    NS_LOG_ERROR("fail to parse the host env, invalid appenv.");
    return ERR_INVALID_ENV;
  }
  switch (g_appenv) {
    case PROD:
      local_env_str = "prod";
      local_env_int = kInt32EnvProd;
      is_online = true;
      NS_LOG_INFO("success to init host env = prod (online)");
      break;
    case STAGING:
      local_env_str = "stage";
      local_env_int = kInt32EnvStage;
      is_online = true;
      NS_LOG_INFO("success to init host env = staging (online)");
      break;
    case DEV:
      local_env_str = "prod";
      local_env_int = kInt32EnvProd;
      is_online = false;
      NS_LOG_INFO("success to init host env = dev (offline)");
      break;
    case PPE:
      local_env_str = "stage";
      local_env_int = kInt32EnvStage;
      is_online = false;
      NS_LOG_INFO("success to init host env = ppe (offline)");
      break;
    case TEST:
      local_env_str = "test";
      local_env_int = kInt32EnvTest;
      is_online = false;
      NS_LOG_INFO("success to init host env = test (offline)");
      break;
    default:
      NS_LOG_ERROR("fail to init host env.");
      return ERR_INVALID_ENV;
  }
  env_->SetIntEnv(local_env_int);
  env_->SetEnvFlag(is_online);
  env_->SetStrEnv(local_env_str);
  env_->SetTypeEnv(g_appenv);
  NS_LOG_INFO("Local env : \n\t\t\t int_env : "
                    << local_env_int
                    << "\n\t\t\t str_env : " << local_env_str
                    << "\n\t\t\t type_env : " << g_appenv
                    << "\n\t\t\t is_online : " << is_online);
  return SUCCESS;
}

int32_t CXmlFile::InitAddrInfo(void) {
  char ip[INET_ADDRSTRLEN] = {0};
  char mask[INET_ADDRSTRLEN] = {0};

  std::string local_ip = "";
  std::string local_mask = "";
  int iRet = getIntranet(ip, mask); // mask default = 255.255.0.0
  if (SUCCESS != iRet) {
    NS_LOG_ERROR("failed to get IP, Mask  by getIntranet, ret = " << iRet);

    // 使用host方式获取IP
    iRet = getHost(local_ip);
    if (SUCCESS != iRet) {
      NS_LOG_ERROR("failed to get IP by getHost, ret = " << iRet);
      return FAILURE;
    }
    NS_LOG_INFO("get local IP = " << local_ip);
  } else {
    local_ip = std::string(ip);
    local_mask = std::string(mask);
    NS_LOG_INFO("get local IP = " << local_ip
                                    << ", Mask = " << mask);
  }
  global_str_params_.insert(
      std::make_pair<CXmlFile::Type ,
                     std::string>(CXmlFile::LocalIp, local_ip));
  global_str_params_.insert(
      std::make_pair<CXmlFile::Type ,
                     std::string>(CXmlFile::LocalMask, local_mask));
  return SUCCESS;
}


int32_t CXmlFile::LoadWhiteAppkeys(const tinyxml2::XMLElement *agent_xml_conf,
                                   CXmlFile::Type type) {
  std::string key = "";
  switch (type) {
    case CXmlFile::AllEnvWhiteLists: {
      key = "AllEnvWhiteLists";
      break;
    }
    case CXmlFile::NoWatcherWhiteLists : {
      key = "NoWatcherWhiteLists";
      break;
    }
    case CXmlFile::RegisteUnlimitWhiteList : {
      key = "RegisteUnlimitWhiteList";
      break;
    }
    case CXmlFile::MacRegisterUnlimitAppkeys : {
      key = "MacRegisterUnlimitAppkeys";
      break;
    }
    default: {
      NS_LOG_ERROR("known type = " << type);
      return FAILURE;
    }
  }

  const tinyxml2::XMLElement *appkeys = agent_xml_conf->FirstChildElement(key.c_str());
  if (NULL == appkeys) {
    NS_LOG_ERROR("Failed to load appkey, type = " << key);
    return FAILURE;
  }
  const tinyxml2::XMLElement *appkey_item = appkeys->FirstChildElement("Item");
  UnorderedMapPtr map_ptr(new boost::unordered_set<std::string>());
  NS_LOG_INFO(key << ":");
  while (NULL != appkey_item) {
    std::string appkey_str(appkey_item->GetText());
    boost::trim(appkey_str);
    if (!appkey_str.empty()) {
      map_ptr->insert(appkey_str);
      NS_LOG_INFO( "\t " << appkey_str);
    }
    appkey_item = appkey_item->NextSiblingElement("Item");
  }
  white_lists_.insert(std::make_pair(type, map_ptr));
  return SUCCESS;
}


int32_t CXmlFile::LoadMutableFile() {
  tinyxml2::XMLDocument agent_mutable;
  const tinyxml2::XMLError agent_xml_ret =
      agent_mutable.LoadFile(AGENT_MUTABLE_CONF.c_str());
  if (tinyxml2::XML_SUCCESS != agent_xml_ret) {
    NS_LOG_ERROR("fail to load " << AGENT_MUTABLE_CONF << ", ret = "
                                   << agent_xml_ret);
    return FAILURE;
  }
  const tinyxml2::XMLElement *agent_xml_conf =
      agent_mutable.FirstChildElement("SGAgentMutableConf");
  if (NULL == agent_xml_conf) {
    NS_LOG_ERROR(
        "can't find SGAgentMutableConf in " << AGENT_MUTABLE_CONF);
    return FAILURE;
  }
  try {
    std::string str_reg_threads
        (FetchElemValByTinyXML(agent_xml_conf, "RegistryThreads"));
    int32_t i32_registry_threads = atoi(str_reg_threads.c_str());
    global_int_params_.insert(std::make_pair(CXmlFile::RegistryThreads,
                                             i32_registry_threads));
    NS_LOG_INFO("RegistryThreads : " << i32_registry_threads);

    std::string
        str_client_num(FetchElemValByTinyXML(agent_xml_conf, "ClientNum"));
    int32_t i32_client_num = atoi(str_client_num.c_str());
    global_int_params_.insert(std::make_pair(CXmlFile::ClientNum,
                                             i32_client_num));
    NS_LOG_INFO("ClientNum : " << i32_client_num);
    std::string str_client_threads
        (FetchElemValByTinyXML(agent_xml_conf, "ClientThreads"));
    int32_t i32_client_threads = atoi(str_client_threads.c_str());
    global_int_params_.insert(std::make_pair(CXmlFile::ClientThreads,
                                             i32_client_threads));
    NS_LOG_INFO("ClientThreads : " << i32_client_threads);

    std::string str_watcher_pull_threads
        (FetchElemValByTinyXML(agent_xml_conf, "WatcherPullThreads"));
    int32_t i32_watcher_pull_threads = atoi(str_watcher_pull_threads.c_str());
    global_int_params_.insert(std::make_pair(CXmlFile::WatcherDispatchThreads,
                                             i32_watcher_pull_threads));
    NS_LOG_INFO("WatcherPullThreads : " << i32_watcher_pull_threads);

    std::string str_watcher_dis_threads
        (FetchElemValByTinyXML(agent_xml_conf, "WatcherDispatchThreads"));
    int32_t
        i32_watcher_dispatch_threads = atoi(str_watcher_dis_threads.c_str());
    global_int_params_.insert(std::make_pair(CXmlFile::WatcherDispatchThreads,
                                             i32_watcher_dispatch_threads));
    NS_LOG_INFO("WatcherDispatchThreads : " << i32_watcher_dispatch_threads);

    std::string str_fast_get_threads
        (FetchElemValByTinyXML(agent_xml_conf, "FastGetThreads"));
    int32_t i32_fastget_threads = atoi(str_fast_get_threads.c_str());
    global_int_params_.insert(std::make_pair(CXmlFile::FastGetThreads,
                                             i32_fastget_threads));
    NS_LOG_INFO("FastGetThreads : " << i32_fastget_threads);

    std::string
        str_retrytimes(FetchElemValByTinyXML(agent_xml_conf, "RetryTimes"));
    int32_t i32_retrytimes_ = atoi(str_retrytimes.c_str());
    global_int_params_.insert(std::make_pair(CXmlFile::RetryTimes,
                                             i32_retrytimes_));
    NS_LOG_INFO("RetryTimes : " << i32_retrytimes_);

    std::string str_registry_action
        (FetchElemValByTinyXML(agent_xml_conf, "RegistryCenter"));
    int32_t i32_registry_action = atoi(str_registry_action.c_str());
    global_int_params_.insert(std::make_pair(CXmlFile::RegistryCenter,
                                             i32_registry_action));
    NS_LOG_INFO("RegistryCenter : " << i32_registry_action);


    std::string
        str_mnsc_timeout(FetchElemValByTinyXML(agent_xml_conf, "MnscTimeOut"));
    int32_t i32_mnsc_timeout_action = atoi(str_mnsc_timeout.c_str());
    global_int_params_.insert(std::make_pair(CXmlFile::MnscTimeOut,
                                             i32_mnsc_timeout_action));
    NS_LOG_INFO("MnscTimeOut : " << i32_mnsc_timeout_action);

    std::string
        str_registry_entry(FetchElemValByTinyXML(agent_xml_conf,
                                                 "RegistryEntry"));
    int32_t i32_registry_entry = atoi(str_registry_entry.c_str());
    global_int_params_.insert(std::make_pair(CXmlFile::RegistryEntry,
                                             i32_registry_entry));
    NS_LOG_INFO("RegistryEntry : " << i32_registry_entry);

    std::string
        str_discovery_type(FetchElemValByTinyXML(agent_xml_conf,
                                                 "DiscoveryType"));
    int32_t i32_discovery_entry = atoi(str_discovery_type.c_str());
    global_int_params_.insert(std::make_pair(CXmlFile::DiscoveryType,
                                             i32_discovery_entry));
    NS_LOG_INFO("DiscoveryType : " << i32_discovery_entry);

    std::string
        str_agent_port(FetchElemValByTinyXML(agent_xml_conf,
                                                 "AgentPort"));
    int32_t i32_agent_port = atoi(str_agent_port.c_str());
    global_int_params_.insert(std::make_pair(CXmlFile::AgentPort,
                                             i32_agent_port));
    NS_LOG_INFO("AgentPort  : " << i32_agent_port);

    std::string
        str_agent_appkey = FetchElemValByTinyXML(agent_xml_conf, "AgentAppKey");
    std::string str_agent_version =
        FetchElemValByTinyXML(agent_xml_conf, "AgentVersion");
    std::string str_mnsccache_appkey =
        FetchElemValByTinyXML(agent_xml_conf, "MNSCacheAppkey");
    std::string str_agent_log =
        FetchElemValByTinyXML(agent_xml_conf, "AgentLog");

    NS_LOG_INFO("AgentAppKey : "
                      << str_agent_appkey
                      << "\n\t\t\t AgentVersion : " << str_agent_version
                      << "\n\t\t\t MNSCacheAppkey : " << str_mnsccache_appkey);
    global_str_params_.insert(std::make_pair(CXmlFile::AgentAppKey,
                                             str_agent_appkey));
    global_str_params_.insert(std::make_pair(CXmlFile::AgentVersion,
                                             str_agent_version));
    global_str_params_.insert(std::make_pair(CXmlFile::MNSCacheAppkey,
                                             str_mnsccache_appkey));
    global_str_params_.insert(std::make_pair(CXmlFile::AgentLog,
                                             str_agent_log));

    LoadAgentFuncFlag(agent_xml_conf);
    int32_t ret = LoadZkConfig(agent_xml_conf);
    if (SUCCESS != ret) {
      NS_LOG_ERROR("failed to load zk config.");
      return ret;
    }

  } catch (boost::exception &e) {
    NS_LOG_ERROR("Failed to load mutable");
    return FAILURE;
  }
  return SUCCESS;
}

int32_t CXmlFile::LoadWhiteFile() {
  tinyxml2::XMLDocument agent_whitelist;
  const tinyxml2::XMLError agent_xml_ret =
      agent_whitelist.LoadFile(AGENT_WHITELIST_CONF.c_str());

  if (tinyxml2::XML_SUCCESS != agent_xml_ret) {
    NS_LOG_ERROR("fail to load " << AGENT_WHITELIST_CONF << ", ret = "
                                   << agent_xml_ret);
    return FAILURE;
  }
  const tinyxml2::XMLElement *agent_xml_conf =
      agent_whitelist.FirstChildElement("SGAgentWhiteListConf");
  if (NULL == agent_xml_conf) {
    NS_LOG_ERROR(
        "can't find SGAgentMutableConf in " << AGENT_WHITELIST_CONF);
    return FAILURE;
  }

  int32_t ret = LoadWhiteAppkeys(agent_xml_conf,
                                 CXmlFile::MacRegisterUnlimitAppkeys);
  if (SUCCESS != ret) {
    return ret;
  }
  ret = LoadWhiteAppkeys(agent_xml_conf,
                         CXmlFile::AllEnvWhiteLists);
  if (SUCCESS != ret) {
    return ret;
  }
  ret = LoadWhiteAppkeys(agent_xml_conf,
                         CXmlFile::RegisteUnlimitWhiteList);
  if (SUCCESS != ret) {
    return ret;
  }
  ret = LoadWhiteAppkeys(agent_xml_conf,
                         CXmlFile::NoWatcherWhiteLists);
  if (SUCCESS != ret) {
    return ret;
  }
  ret = LoadRegisterWhitelistIdc(agent_xml_conf);
  if (SUCCESS != ret) {
    return ret;
  }
  return ret;
}
const std::string CXmlFile::GetLogPath(){
  std::string default_path = "/octo/ns/sg_agent/log4cplus.conf";
  tinyxml2::XMLDocument agent_mutable;
  const tinyxml2::XMLError agent_xml_ret =
      agent_mutable.LoadFile(AGENT_MUTABLE_CONF.c_str());
  if (tinyxml2::XML_SUCCESS != agent_xml_ret) {
    std::cout<<"fail to load " << AGENT_MUTABLE_CONF << std::endl;
    return default_path;
  }
  const tinyxml2::XMLElement *agent_xml_conf =
      agent_mutable.FirstChildElement("SGAgentMutableConf");
  if (NULL == agent_xml_conf) {
    std::cout<< "can't find SGAgentMutableConf" << std::endl;
    return default_path;
  }
  std::string
      str_log_path = FetchElemValByTinyXML(agent_xml_conf, "AgentLogPath");

  return str_log_path;
}

int32_t CXmlFile::LoadAgentFuncFlag(const tinyxml2::XMLElement *agent_xml_conf) {
  std::string res = FetchElemValByTinyXML(agent_xml_conf, "OpenMNSCache");
  sg_agent_fun_.insert(std::make_pair(CXmlFile::OpenMNSCache,
                                      0 != res.compare("open") ? false : true));
  NS_LOG_INFO("OpenMNSCache : " << res);
  res = FetchElemValByTinyXML(agent_xml_conf, "OpenAutoRoute");
  sg_agent_fun_.insert(std::make_pair(CXmlFile::OpenAutoRoute,
                                      0 != res.compare("open") ? false : true));
  NS_LOG_INFO("OpenAutoRoute : " << res);

  return SUCCESS;
}


int32_t CXmlFile::LoadRegisterWhitelistIdc(const tinyxml2::XMLElement *sg_agent_idc) {
  const tinyxml2::XMLElement *idcs =
      sg_agent_idc->FirstChildElement("MacRegisterUnlimitAppkeysIDC");
  if (NULL == idcs) {
    NS_LOG_ERROR("can not find the xml element HotelTravelRegisterIDC.");
    return FAILURE;
  }
  NS_LOG_INFO("MacRegisterUnlimitAppkeysIDC : ");
  const tinyxml2::XMLElement *idc_item = idcs->FirstChildElement("Item");
  NS_LOG_INFO("IDC : ");
  while (NULL != idc_item) {
    const tinyxml2::XMLElement *ip_ptr = idc_item->FirstChildElement("IP");
    const tinyxml2::XMLElement *mask_ptr = idc_item->FirstChildElement("MASK");
    if (NULL != ip_ptr && NULL != mask_ptr) {
      std::string ip_str(ip_ptr->GetText());
      std::string mask_str(mask_ptr->GetText());
      boost::trim(ip_str);
      boost::trim(mask_str);
      if ((!ip_str.empty()) && (!mask_str.empty())) {
        boost::shared_ptr<IDC> idc(new IDC());
        idc->SetIp(ip_str);
        idc->SetMask(mask_str);
        idc->CalculateIpMask();
        NS_LOG_INFO("\t IP = " << ip_str << ", MASK = " << mask_str);
        register_whitelist_idcs_->push_back(idc);
      } else {
        NS_LOG_WARN("IP or MASK is empty");
      }
    } else {
      NS_LOG_WARN("IP or MASK miss.");
    }
    idc_item = idc_item->NextSiblingElement("Item");
  }
  return false;
}


int32_t CXmlFile::LoadIdcFile(IdcsPtr &idcs) {
  NS_LOG_INFO("loading " << kStrIDCFileFullPath);
  tinyxml2::XMLDocument conf_regions;
  tinyxml2::XMLError eResult =
      conf_regions.LoadFile(kStrIDCFileFullPath.c_str());

  if (tinyxml2::XML_SUCCESS != eResult) {
    NS_LOG_ERROR("failed to load config: " << kStrIDCFileFullPath
                                             << ", errorCode = " << eResult);
    return FAILURE;
  }
  tinyxml2::XMLElement *xmlRegion =
      conf_regions.FirstChildElement("SGAgent")->FirstChildElement("Region");

  while (NULL != xmlRegion) {
    tinyxml2::XMLElement *xmlRegionName =
        xmlRegion->FirstChildElement("RegionName");

    tinyxml2::XMLElement *xmlIDC = xmlRegion->FirstChildElement("IDC");
    while (NULL != xmlIDC) {
      tinyxml2::XMLElement *xmlItem = xmlIDC->FirstChildElement("Item");
      tinyxml2::XMLElement *idc_ptr = xmlIDC->FirstChildElement("IDCName");
      tinyxml2::XMLElement *center_ptr = xmlIDC->FirstChildElement("CenterName");
      while (NULL != xmlItem) {
        boost::shared_ptr<IDC> idc(new IDC());
        std::string ip_str = FetchElemValByTinyXML(xmlItem, "IP");
        std::string mask_str = FetchElemValByTinyXML(xmlItem, "MASK");
        if ((!ip_str.empty()) && (!mask_str.empty())) {
          std::string region_name =
              NULL != xmlRegionName ? xmlRegionName->GetText() : "unknown" ;
          std::string center_name =
              NULL != center_ptr ? center_ptr->GetText() : "unknown";
          std::string idc_name =
              NULL != idc_ptr ? idc_ptr->GetText() : "unknown";
          boost::shared_ptr<IDC> idc(new IDC());
          idc->SetRegion(region_name);
          idc->SetIp(ip_str);
          idc->SetMask(mask_str);
          idc->SetIdc(idc_name);
          idc->SetCenter(center_name);
          idc->CalculateIpMask();
          NS_LOG_INFO("region = " << region_name
                                    << " idc = " << idc_name
                                    << " center = " << center_name
                                    << " ip = " << ip_str
                                    << " mask = " << mask_str);
          idcs->push_back(idc);
        } else {
          NS_LOG_WARN("IP or MASK is empty");
        }
        xmlItem = xmlItem->NextSiblingElement("Item");
      }
      xmlIDC = xmlIDC->NextSiblingElement("IDC");
    }
    xmlRegion = xmlRegion->NextSiblingElement("Region");
  }
  return SUCCESS;

}

int32_t CXmlFile::LoadZkConfig(const tinyxml2::XMLElement *sg_agent_conf) {
  boost::shared_ptr<IDC> local_ip_idc = IdcUtil::GetIdc(env_->GetStrEnv());
  std::string region_name = NULL != local_ip_idc.get() ?
      local_ip_idc->GetRegion() : "beijing";
  if ("unknown" == region_name || region_name.empty()) {
    region_name = "beijing";
  }
  NS_LOG_INFO("identify the region name is " << region_name);

  std::string str_zk_client_nums
      (FetchElemValByTinyXML(sg_agent_conf, "ZkClientNum"));
  int32_t i32_zk_client_nums_ = atoi(str_zk_client_nums.c_str());
  global_int_params_.insert(std::make_pair(CXmlFile::ZkClientNum,
                                           i32_zk_client_nums_));

  std::string
      str_zk_timeout(FetchElemValByTinyXML(sg_agent_conf, "ZkTimeout"));
  int32_t i32_zk_timeout_ = atoi(str_zk_timeout.c_str());
  global_int_params_.insert(std::make_pair(CXmlFile::ZkTimeout,
                                           i32_zk_timeout_));


  //初始化ZkClient进行连接管理
  const tinyxml2::XMLElement *xml_mns_host = sg_agent_conf->FirstChildElement("MnsHost");
  if (NULL == xml_mns_host) {
    NS_LOG_ERROR("miss mns host in config mutable xml");
    return ERR_CONFIG_PARAM_MISS;
  }
  const tinyxml2::XMLElement *is_online_element =
      xml_mns_host->FirstChildElement(env_->GetEnvFlag() ? "online" : "offline");
  if (NULL == is_online_element) {
    NS_LOG_ERROR("miss online or offline element in sg_agent_mutable.xml");
    return ERR_CONFIG_PARAM_MISS;
  }

  std::string zk_lists = FetchElemValByTinyXML(is_online_element, region_name);
  if (zk_lists.empty()) {
    NS_LOG_ERROR("zk list is empty.");
    return FAILURE;
  }
  std::string idc_zk_lists = IdcUtil::GetSameIdcZk(zk_lists, env_->GetStrEnv());
  boost::trim(idc_zk_lists);
  NS_LOG_INFO("zk list is " << idc_zk_lists);
  global_str_params_.insert(std::make_pair(CXmlFile::ZkLists, idc_zk_lists));
  return SUCCESS;
}

bool CXmlFile::GetAgentFunFlag(const CXmlFile::Type type) {
  if (sg_agent_fun_.end() == sg_agent_fun_.find(type)) {
    NS_LOG_ERROR("cannot find type " << type);
    return false;
  }
  return sg_agent_fun_.at(type);
}

UnorderedMapPtr CXmlFile::GetWhiteAppkeys(const CXmlFile::Type type) {
  if (white_lists_.end() == white_lists_.find(type)) {
    NS_LOG_ERROR("cannot find type " << type);
    return boost::make_shared<boost::unordered_set<std::string> >();
  }
  return white_lists_.at(type);
}

const std::string CXmlFile::GetStrPara(const CXmlFile::Type type) {
  if (global_str_params_.end() == global_str_params_.find(type)) {
    NS_LOG_ERROR("cannot find type " << type);
    return "";
  }
  return global_str_params_.at(type);
}

int32_t CXmlFile::GetI32Para(const CXmlFile::Type type) {
  if (global_int_params_.end() == global_int_params_.find(type)) {
    NS_LOG_ERROR("cannot find type " << type);
    return 0;
  }
  return global_int_params_.at(type);
}

void CXmlFile::UpdateIdcs() {
	IdcsPtr new_idcs = boost::make_shared<std::vector<boost::shared_ptr<IDC> > >();
	int32_t ret = LoadIdcFile(new_idcs);
	if (ret != SUCCESS) {
		NS_LOG_ERROR("failed to update idc.");
		return;
	}
	muduo::MutexLockGuard lock(update_idc_lock_);
	idcs_ = new_idcs;
}
void CXmlFile::UpdateIdcsTimer() {
  std::string idc_md5_tmp = CalculateMd5(IDC_FILE_NAME, AGENT_CONF_PATH);
  if (idc_md5_tmp.empty() ||
      0 == idc_md5_tmp.compare(idc_md5_)) {
    return ;
  }
  IdcsPtr new_idcs = boost::make_shared<std::vector<boost::shared_ptr<IDC> > >();
  int32_t ret = LoadIdcFile(new_idcs);
  if (ret != SUCCESS) {
    NS_LOG_ERROR("failed to update idc.");
    return;
  }
  idc_md5_ = idc_md5_tmp;
  muduo::MutexLockGuard lock(update_idc_lock_);
  idcs_ = new_idcs;
}

IdcsPtr CXmlFile::GetIdc(void) {
  muduo::MutexLockGuard lock(update_idc_lock_);
  return idcs_;
}

}  //  namespace meituan_mns
