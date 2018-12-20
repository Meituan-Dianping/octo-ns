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

#ifndef OCTO_OPEN_SOURCE_BASE_CONSTS_H
#define OCTO_OPEN_SOURCE_BASE_CONSTS_H
#include <string>
#include <stdint.h>
namespace meituan_mns {
const std::string SG_AGENT_CONF = "/octo/nameservice/sg_agent/sg_agent_env.xml";
const std::string AGENT_MUTABLE_CONF = "/octo/ns/sg_agent/sg_agent_mutable.xml";
const std::string AGENT_WHITELIST_CONF = "/octo/ns/sg_agent/sg_agent_whitelist.xml";
const std::string AGENT_SWITCH_FILE = "/octo/ns/sg_agent/agent_switch.xml";
const std::string APPENV_FILE = "/data/webapps/octo.cfg";
const std::string kStrIDCFileFullPath = "/octo/ns/sg_agent/idc.xml";
const std::string kServiceAppkeyPath = "/data/webapps/appkeys";

const std::string AGENT_CONF_PATH = "/octo/ns/sg_agent/";
const std::string IDC_FILE_NAME = "idc.xml";

const int32_t kMaxRegistryThreads = 4;
const int32_t kMaxDiscThreads = 1;

const int32_t kRegistryThreads = 0;
const int32_t kNodeIp = 1;
const int32_t kClientNum = 2;
const int32_t kClientThreads = 3;
const int32_t kPullThreads = 4;
const int32_t kDispatchThreads = 5;
const int32_t kFastGetThreads = 6;
const int32_t kUnifiedAppkey = 7;
const int32_t kUnifiedMtthrift = 8;
const int32_t kNodeEnv = 9;
const int32_t kRegistryType = 10;
const int32_t kZkTimeout = 11;
const int32_t kRetryTimes = 12;
const int32_t kRegisryCenterType =13;
const int32_t kCacheDiscSwitch = 14;
const int32_t kZkClientNum = 15;

const int32_t kZkClientIndex = 0;
static const int32_t HeartbeatUnSupport = 0;
static const int32_t DefaultRetry = 5;

const int32_t kConfigInitRetryTimes = 3;

const int32_t kDiscZookeeper = 1;
const int32_t kDiscCached = 2;
const int32_t kDiscMixer = 3;
const int32_t kExtendCenter = 4;

const int32_t kRegistryZk = 0;
const int32_t kRegistryCache = 1;
const int32_t kRegistryMixer = 2;

const int32_t retry_times_ = 3;
const int32_t kRetrySleepTime = 50000;
const int32_t kDefaultWeight = 10;
const int32_t kDefaultFweight = 10.0;
const std::string kDefaultStrProtocol = "thrift";

const int32_t MTCONFIG_OK = 200;
const int32_t MTCONFIG_NOT_CHANGE = 302;

const int32_t MNSC_OK = 200;
const int MNSC_UPDATING = 500;
const int MNSC_RETRY_TIME = 3;

const int32_t DEFAULT_EXTIME = 300;
const int32_t kMaxZkPathDepth = 8;

const int32_t kInt32EnvTest = 1;
const int32_t kInt32EnvStage = 2;
const int32_t kInt32EnvProd = 3;

//上报监控item
const int PID_ITEM = 0;
const int VMRSS_ITEM = 1;
const int VERSION_ITEM = 2;
const int CPU_ITEM = 3;
const int FILECONFIG_ITEM = 4;
const int KVCONFIG_ITEM = 5;
const int COMMON_LOG_ITEM = 6;
const int ROUTE_LIST_ITEM = 7;
const int SERVICE_LIST_ITEM = 8;
const int MODULE_INVOKER_ITEM = 9;
const int REGISTER_ITEM = 10;
const int MCC_FCONFIG_ALLREQ_ITEM = 11;
const int MCC_FCONFIG_SUCCESSREQ_ITEM = 12;
const int MCC_CONFIG_ALLREQ_ITEM = 13;
const int MCC_CONFIG_SUCCESSREQ_ITEM = 14;
const int MNSC_ALLREQ_ITEM = 15;
const int MNSC_SUCCESSREQ_ITEM = 16;
const int MAX_MONITOER_ITEM = 16;
const int DEFAULT_PROTOCOL_SCANTIME = 20;
const int DEFAULT_PROTOCOL_MAX_UPDATETIME = 60; // 1min

//http
const std::string HTTP_RESPONSE_NULL = "nothing availble http description msg";
const std::string listen_ip = "0.0.0.0";

const int http_port = 5267;
const int INVALID_METHOD = -1;
const int ADD_SERVICE = 0;
const int UPDATE_SERVICE = 1;
const int DELETE_SERVICE = 2;
const int GET_SERVICE = 3;
const int GET_MONITOR_SERVICE = 4;
const int REPLACE_SERVICE = 5;
const int HTTP_MNS_URL = 6;
const int HTTP_MONITOR_URL = 7;
const int HEALTHY_CHECK = 8;
const unsigned int kThreadTime = 20*1000;//20ms


// 地域过滤阈值
const double RegionThresHold = 0.0001;
// IDC过滤阈值
const double IdcThresHold = 0.1;

const int HealthCheckInterval = 60;//60s
const int HealthCheckMaxInterval = 5*60; //5min

enum RegistryType{
    kAddReg = 0,
    kBatchReg,
    kExtendReg
};


enum RegistryActionType {
  REGIST = 0,
  UNREGIST = 1
};



}




#endif //OCTO_OPEN_SOURCE_BASE_CONSTS_H
