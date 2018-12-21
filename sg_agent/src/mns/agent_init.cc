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
#include "agent_init.h"
#include "zk_client_pool.h"
#include "log4cplus.h"
#include "base_consts.h"
#include <boost/algorithm/string/trim.hpp>
#include "http_service.h"
#include "hot_load_config.h"
#include "discovery_service.h"
#include "registry_service.h"
namespace meituan_mns {

AgentInit::AgentInit(): is_zk_init_(false),
                        is_loadfile_init_(false),
                        is_disc_init_(false),
                        latch(1),
                        healthy_interval_secs(HealthCheckInterval) {}

AgentInit::~AgentInit() {}

void AgentInit::SetLocolServiceNode(SGService& service){
  service.appkey = CXmlFile::GetStrPara(CXmlFile::AgentAppKey);
  service.version = CXmlFile::GetStrPara(CXmlFile::AgentVersion);
  service.ip = CXmlFile::GetStrPara(CXmlFile::LocalIp);
  service.port = CXmlFile::GetI32Para(CXmlFile::AgentPort);
  service.weight = kDefaultWeight;
  service.fweight = kDefaultFweight;
  service.status = fb_status::ALIVE;
  service.protocol = kDefaultStrProtocol;
  service.envir = CXmlFile::GetAppenv()->GetIntEnv();
  service.lastUpdateTime = time(0);
  service.role = 0;
  service.warmup = 0;
  NS_LOG_INFO(" service.appkey = " << service.appkey << ", service.port = "
                           << service.port << ", service.fweigit = "
                           << service.fweight);
}

int32_t AgentInit::PreloadLogConfigPath(std::string& log_path){
  log_path = cxml_file_.GetLogPath();
  std::cout <<"agent log path:" <<log_path<<std::endl;
  if(log_path.empty()){
    return -1;
  }
  return 0;
}

void AgentInit::Init(void) {
  NS_LOG_INFO("OCTO Service Agent Start...");
  healthy_loop_ = healthy_loop_thread_.startLoop();
  HealthCheck();
  latch.wait();

  HttpService::GetInstance()->StartHttpServer();
  NS_LOG_INFO("OCTO Service Agent End ...");

  SGService oservice;
  SetLocolServiceNode(oservice);
  int ret = RegistryService::GetInstance()->RegistryStart(oservice, UptCmd::ADD);
  if (SUCCESS != ret) {
    NS_LOG_ERROR("failed to regist sg_agent to registry center, ret = "
                       << ret);
  }
  NS_LOG_INFO("succeed to regist self to mns ret:" << ret);
}

void AgentInit::HealthCheckFailed() {
  if (healthy_interval_secs >= HealthCheckMaxInterval) {
    healthy_interval_secs = HealthCheckMaxInterval;
  }
  healthy_loop_->runAfter(HealthCheckInterval,
                          boost::bind(&AgentInit::HealthCheck, this));
  healthy_interval_secs += HealthCheckInterval;
}

int32_t AgentInit::HealthCheck() {
  int32_t ret = 0;
  if (!is_loadfile_init_) {
    ret = cxml_file_.CXmlFileInit();
    if (0 != ret) {
      HealthCheckFailed();
      return ret;
    }
    is_loadfile_init_ = true;
  }
  if (!is_zk_init_) {
    std::string zk_lists = cxml_file_.GetStrPara(CXmlFile::ZkLists);
    boost::trim(zk_lists);
    ret =
        ZkClientPool::GetInstance()->Init(CXmlFile::GetStrPara(CXmlFile::ZkLists),
                                          CXmlFile::GetI32Para(CXmlFile::ZkClientNum),
                                          CXmlFile::GetI32Para(CXmlFile::ZkTimeout),
                                          CXmlFile::GetI32Para(CXmlFile::RetryTimes));
    if (SUCCESS != ret) {
      NS_LOG_ERROR("failed to create zkclient pool ,ret = " << ret);
      HealthCheckFailed();
      return ret;
    }
    is_zk_init_ = true;
  }

  if (!is_disc_init_) {
    ret = DiscoveryService::GetInstance()->Init();
    if (SUCCESS != ret) {
      NS_LOG_INFO("failed to init discovery service.");
      HealthCheckFailed();
      return ret;
    }
    is_disc_init_ = true;
  }
  NS_LOG_INFO("Healthy check success");
  if (1 == latch.getCount()) {
    latch.countDown();
  }
}

}  // namespace meituan_mns
