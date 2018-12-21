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

#include "mns_config.h"

using namespace mns_sdk;

MnsConfig mns_sdk::g_config;

MnsConfig::MnsConfig() :
    url_port_(80),local_chrion_port_(-1), b_use_remote_(false) {
}

int MnsConfig::InitAppnev(const std::map<std::string, std::string>
                          &config_map) {
  string env;
  std::map<std::string, std::string>::const_iterator it = config_map.find
      ("env");
  if (it != config_map.end()) {
    env = it->second;
  } else {
    MNS_LOG_ERROR("have no env config.");
    return -1;
  }

  Appenv mAppenv;
  // 优先解释env字段，无法解释时，再解释deployenv。请勿优化以下if语句
  if ("prod" == env) {
    mAppenv = PROD;
  } else if ("staging" == env) {
    mAppenv = STAGING;
  } else if ("dev" == env) {
    mAppenv = DEV;
  } else if ("ppe" == env) {
    mAppenv = PPE;
  } else if ("test" == env) {
    mAppenv = TEST;
  } else {
    MNS_LOG_ERROR(
        "str_env_ is empty, fetch from appenv file failed, Please contact with SRE to handle this problem. ");
    return -1;
  }

  switch (mAppenv) {
    case PROD:str_env_ = "prod";
      str_octo_env_ = "prod";
      enum_onoffline_env_ = ONLINE;
      MNS_LOG_INFO("success to init host env = prod (online)");
      break;
    case STAGING:str_env_ = "stage";
      str_octo_env_ = "stage";
      enum_onoffline_env_ = ONLINE;
      MNS_LOG_INFO("success to init host env = staging (online)");
      break;
    case DEV:str_env_ = "prod";
      str_octo_env_ = "dev";
      enum_onoffline_env_ = OFFLINE;
      MNS_LOG_INFO("success to init host env = dev (offline)");
      break;
    case PPE:str_env_ = "stage";
      str_octo_env_ = "ppe";
      enum_onoffline_env_ = OFFLINE;
      MNS_LOG_INFO("success to init host env = ppe (offline)");
      break;
    case TEST:str_env_ = "test";
      str_octo_env_ = "test";
      enum_onoffline_env_ = OFFLINE;
      MNS_LOG_INFO("success to init host env = test (offline)");
      break;
    default: MNS_LOG_ERROR("fail to init host env.");
      return -1;
  }

  MNS_LOG_DEBUG("enum_onoffline_env_ " << enum_onoffline_env_);
  return 0;
}

void MnsConfig::GetHostIPInfo(void) {
  char ip[INET_ADDRSTRLEN] = {0};

  MnsSdkCommon::IntranetIp(ip);
  if (MNS_UNLIKELY(0 == strlen(ip))) {
    MNS_LOG_WARN("Cannot get local ip, wait 5 secs");
    muduo::CurrentThread::sleepUsec(5000 * 1000);

    MnsSdkCommon::IntranetIp(ip);
  }

  if (MNS_UNLIKELY(0 == strlen(ip))) {
    MNS_LOG_WARN("After wait 5 secs, still cannot get local ip, set to be 127"
                     ".0.0.1");
    str_local_ip_.assign("127.0.0.1");
  } else {
    str_local_ip_.assign(ip);
    MNS_LOG_INFO("local ip " << str_local_ip_);
  }

  char hostCMD[64] = {0};
  strncpy(hostCMD, "host ", 5);
  strncpy(hostCMD + 5, ip, INET_ADDRSTRLEN);

  FILE *fp = popen(hostCMD, "r");
  char hostInfo[256] = {0};

  if (MNS_LIKELY(!fgets(hostInfo, 256, fp))) {
    int iRet = ferror(fp);
    if (MNS_UNLIKELY(iRet)) {
      MNS_LOG_ERROR("fgets error, iRet " << iRet);
      pclose(fp);
      return;
    }
  }
  hostInfo[strlen(hostInfo) - 1] = '\0';

  str_host_.assign(hostInfo);
  str_host_.assign(MnsSdkCommon::strToLower(str_host_));
  MnsSdkCommon::replace_all_distinct(" ", "%20", &str_host_);
  MNS_LOG_INFO("host info: " << hostInfo);

  pclose(fp);
  memset(hostCMD, 0, sizeof(hostCMD));

  strncpy(hostCMD, "hostname ", 9);
  fp = popen(hostCMD, "r");
  char hostname[256] = {0};
  if (MNS_LIKELY(!fgets(hostname, 256, fp))) {
    int iRet = ferror(fp);
    if (MNS_UNLIKELY(iRet)) {
      MNS_LOG_ERROR("fgets error, iRet " << iRet);
      pclose(fp);
      return;
    }
  }

  hostname[strlen(hostname) - 1] = '\0';  //del line token

  str_hostname_.assign(hostname);
  str_hostname_.assign(MnsSdkCommon::strToLower(str_hostname_));
  MnsSdkCommon::replace_all_distinct(" ", "%20", &str_hostname_);
  MNS_LOG_INFO("host name: " << hostname);
  pclose(fp);
}

int MnsConfig::LoadConfig(const std::string &str_file_path) {

  std::map<std::string, std::string> str_str_map;
  string deployenv_str;
  string env_str;
  ifstream confg_fin;
  try {
    confg_fin.open(str_file_path.c_str(), std::ios::in);
    if (!confg_fin.is_open()) {
      MNS_LOG_ERROR("Failed to open appenv file, Maybe file is not exist"
                        << str_file_path);
      return -1;
    } else {
      string buffer_str;
      while (getline(confg_fin, buffer_str)
          && (env_str.empty() || deployenv_str.empty())) {
        size_t pos = buffer_str.find_first_of("=");
        if (string::npos != pos) {
          string key = buffer_str.substr(0, pos);
          string value = buffer_str.substr(pos + 1);
          boost::trim(key);
          boost::trim(value);
          str_str_map[key] = value;
        }
        buffer_str.clear();
      }
    }
    confg_fin.close();
  } catch (exception &e) {
    confg_fin.close();
    MNS_LOG_ERROR("fail to load "
                      << str_file_path
                      << "OR fetch deployenv/appenv failed, reason: "
                      << e.what());
    return -1;
  }

  if (InitAppnev(str_str_map) != 0) {
    MNS_LOG_ERROR("InitAppnev failed ");
  }

  if (str_str_map.find("mns_url") != str_str_map.end()) {
    url_ = str_str_map["mns_url"];

    size_t pos = url_.find("http://");
    if(pos != std::string::npos){
      url_ = url_.substr(pos + 7);
    }

    pos = url_.find(":");
    if(pos != std::string::npos){
      std::string temp = url_.substr(pos +1);
      size_t pos2 = temp.find("/");
      temp = temp.substr(0, pos2);


      try {
        url_port_ = boost::lexical_cast<uint16_t>(temp);
      } catch (boost::bad_lexical_cast &e) {

        MNS_LOG_ERROR("boost::bad_lexical_cast :"
                          << e.what() << "url_: " << temp);
        url_port_ = 80;
      }

      url_ = url_.substr(0, pos);
    }

    MNS_LOG_INFO("url host:" << url_ << " port:" << url_port_);
  } else {
    MNS_LOG_ERROR("have no mns_url config.");
  }

  if (str_str_map.find("sgagent_appkey") != str_str_map.end()) {
    chrion_appkey_ = str_str_map["sgagent_appkey"];
  } else {
    MNS_LOG_ERROR("have no chrion_appkey config.");
  }

  if (str_str_map.find("sg_sentinel_appkey") != str_str_map.end()) {
    chrion_sentienl_appkey_ = str_str_map["sg_sentinel_appkey"];
  } else {
    MNS_LOG_ERROR("have no sentienl_appkey config.");
  }

  if (str_str_map.find("idc_path") != str_str_map.end()) {
    str_idc_path_ = str_str_map["idc_path"];
  } else {
    MNS_LOG_ERROR("have no idc_path config.");
  }

  if (str_str_map.find("port") != str_str_map.end()) {
    local_chrion_port_ = atoi(str_str_map["port"].c_str());
  } else {
    MNS_LOG_ERROR("have no port config.");
  }

  GetHostIPInfo();

  if (str_env_.empty() || url_.empty() || str_octo_env_.empty() ||
      chrion_sentienl_appkey_.empty()) {
    MNS_LOG_ERROR("b_use_remote_. false");
    b_use_remote_ = false;
  } else {
    MNS_LOG_INFO("b_use_remote_. true");
    str_sentinel_http_request_.assign(
        "GET /api/servicelist?appkey=" + g_config.chrion_sentienl_appkey_ +
            "&env=" + g_config.str_env_ +
            "&host=" + g_config.str_host_ + "&hostname="
            + g_config.str_hostname_ + "&ip=" + g_config.str_local_ip_ + " "
            "HTTP/1.1\r\nHost: " + g_config.url_ + "\r\n\r\n");
    b_use_remote_ = true;
  }

  if (local_chrion_port_ == -1) {
    MNS_LOG_INFO("port is unknow. failed");
    return -1;
  }

  str_local_chrion_port_ = boost::lexical_cast<string>(local_chrion_port_);
  return 0;
}
