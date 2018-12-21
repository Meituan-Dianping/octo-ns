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

#include "hot_load_config.h"
#include "inc_comm.h"
#include "base_consts.h"
#include "md5.h"
#include <boost/bind.hpp>

namespace  meituan_mns {


const int kPeriodTime = 5*60;//5min
const int kSleepTime = 20*1000;//50ms
const std::string idc_file_name = "idc.xml";


DynamicLoadConfig* DynamicLoadConfig::dynamic_load_config_ = NULL;
muduo::MutexLock DynamicLoadConfig::dynamic_load_config_lock_;

DynamicLoadConfig::DynamicLoadConfig() :
    idc_md5_(""),
    appkeys_md5_(""),
    check_loop_ (NULL),
    check_thread_ (boost::function<void(muduo::net::EventLoop*)>(), "sg_agent") {}

DynamicLoadConfig::~DynamicLoadConfig() {
  if (NULL != check_loop_) {
    check_loop_->quit();
    // sleep for avoiding race condition
    usleep(kSleepTime);
    delete check_loop_;
    check_loop_ = NULL;
  }
  SAFE_DELETE(dynamic_load_config_);
}

DynamicLoadConfig* DynamicLoadConfig::GetInstance() {
  if (NULL == dynamic_load_config_) {
    muduo::MutexLockGuard lock(dynamic_load_config_lock_);
    if (NULL == dynamic_load_config_) {
      dynamic_load_config_ = new DynamicLoadConfig();
    }
  }
  return dynamic_load_config_;
}
void DynamicLoadConfig::Init() {
  idc_md5_ = CalculateMd5(idc_file_name, AGENT_CONF_PATH);
  check_loop_ = check_thread_.startLoop();
  check_loop_->runEvery(kPeriodTime, boost::bind(&DynamicLoadConfig::UpdateIdcsTimer,
                                                 this));
}

void DynamicLoadConfig::UpdateIdcsTimer(void) {
  std::string idc_md5_tmp = CalculateMd5(idc_file_name, AGENT_CONF_PATH);
  if (idc_md5_tmp.empty() ||
      0 == idc_md5_tmp.compare(idc_md5_)) {
    return ;
  }
  cxml_file_.UpdateIdcs();
}



std::string DynamicLoadConfig::CalculateMd5(const std::string &file_name,
                                            const std::string &file_path) {
  std::string context = "";
  int32_t ret = loadFile(context, file_name, file_path);
  if (SUCCESS != ret) {
    NS_LOG_ERROR("failed to load idc.");
    return "";
  }
  MD5 md5_string(context);
  return md5_string.md5();
}


}