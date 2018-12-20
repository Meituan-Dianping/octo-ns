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

#include "whitelist_manager.h"
#include "log4cplus.h"

namespace meituan_mns {
bool WhiteListManager::IsAppkeyInWhitList(const std::string &appkey) {
  UnorderedMapPtr set_ptr =
      CXmlFile::GetWhiteAppkeys(CXmlFile::NoWatcherWhiteLists);
  return IsAppkeyList(appkey, set_ptr);
}

bool WhiteListManager::IsAppkeyInRegistUnlimitWhitList(const std::string &appkey) {
  UnorderedMapPtr set_ptr =
      CXmlFile::GetWhiteAppkeys(CXmlFile::RegisteUnlimitWhiteList);
  return IsAppkeyList(appkey, set_ptr);
}

bool WhiteListManager::IsAppkeyInAllEnvWhitList(const std::string &appkey) {
  UnorderedMapPtr set_ptr =
      CXmlFile::GetWhiteAppkeys(CXmlFile::AllEnvWhiteLists);
  return IsAppkeyList(appkey,set_ptr);
}

bool WhiteListManager::IsMacRegisterAppkey(const std::string &appkey) {
  UnorderedMapPtr set_ptr =
      CXmlFile::GetWhiteAppkeys(CXmlFile::MacRegisterUnlimitAppkeys);
  return IsAppkeyList(appkey, set_ptr);
}


bool WhiteListManager::IsAppkeyContains(const std::string &appkey,
                                        const UnorderedMapPtr &appkeys) {
  if (appkey.empty()) {
    NS_LOG_WARN("fileconfig req appkey is empty!");
    return false;
  }
  for (boost::unordered_set<std::string>::const_iterator iter = appkeys->begin();
       appkeys->end() != iter; ++iter) {
    if (std::string::npos != appkey.find(*iter)) {
      NS_LOG_INFO(appkey << " is in the appkey white list : " << *iter);
      return true;
    }
  }
  return false;
}

bool WhiteListManager::IsAppkeyList(const std::string &appkey,
                                    const UnorderedMapPtr &appkeys) {
  if (appkey.empty()) {
    NS_LOG_WARN("fileconfig req appkey is empty!");
    return false;
  }
  for (boost::unordered_set<std::string>::const_iterator iter = appkeys->begin();
       appkeys->end() != iter; ++iter) {
    if (std::string::npos != appkey.find(*iter)) {
      NS_LOG_INFO(appkey << " is in the appkey white list : " << *iter);
      return true;
    }
  }
  return false;
}
}
