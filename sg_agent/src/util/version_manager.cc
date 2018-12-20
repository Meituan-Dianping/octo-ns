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

#include "version_manager.h"
#include "log4cplus.h"
namespace meituan_mns {

VersionManager::VersionManager(){

  zk_update_time_ = 1800;
  zk_update_rand_time_ = 0;
  key_buffer_ = boost::shared_ptr < BufferMgr < int > > (new BufferMgr<int>());

  srand(time(0));
  zk_update_rand_time_ = rand() % 100;
}

//本地version与ZK上相同，如果超过半小时都没更新，则主动更新一次
int32_t VersionManager::CheckZkVersion(const std::string &key,
                                   const std::string &zk_version,
                                   std::string &version) {
  int last_time = 0;
  int hasKey = key_buffer_->get(key, last_time);

  int curr_time = time(0);
  if ((version == zk_version) && 0 == hasKey) {
    if ((curr_time - last_time) > (zk_update_time_ + zk_update_rand_time_)) {
      NS_LOG_INFO("start to update zk data! appkey :  " << key);
    } else {
      NS_LOG_DEBUG("the data is the same as buf, key : " << key
                                                           << ", version : " << version);
      return ERR_ZK_LIST_SAME_BUFFER;
    }
  }
  key_buffer_->insert(key, curr_time);
  NS_LOG_INFO("version is different from zk_version,need to update!");
  version = zk_version;

  return 0;
}
bool VersionManager::IsOldVersion(const std::string &version) {
  if (version.empty()) {
    return true;
  }
  std::string mtthrift_str("mtthrift");
  std::string cthrift_str("cthrift");
  std::string pigeon_str("pigeon");
  if (std::string::npos != version.find(mtthrift_str)) {
    NS_LOG_INFO("version is mtthfit: " << version);
    return CompareMTVersion(version);
  } else if (('0' <= version[0] && '9' >= version[0]) ||
      (std::string::npos != version.find(pigeon_str))) {
    NS_LOG_INFO("version is pigeon: " << version);
    return ComparePigeonVersion(version);
  } else if (std::string::npos != version.find(cthrift_str)) {
    NS_LOG_INFO("version is cthrift: " << version);
    return CompareCthriftVersion(version);
  } else {
    NS_LOG_INFO("not need to check the version : " << version);
    return false;
  }
}
bool VersionManager::CompareCthriftVersion(const std::string &version) {
  std::vector<int> c;
  GetVersion(version, &c);
  int ret = CompareVector(c, old_cthrift_version_);
  return (0 > ret);
}

bool VersionManager::ComparePigeonVersion(const std::string &version) {
  std::vector<int> p;
  GetVersion(version, &p);
  int ret = CompareVector(p, old_pigeon_version_);
  return (0 > ret);
}
bool VersionManager::CompareMTVersion(const std::string &version) {
  std::vector<int> mt;
  GetVersion(version, &mt);
  int ret = CompareVector(mt, old_mtthrift_version_);
  return (0 > ret);
}
int32_t VersionManager::CompareVector(const std::vector<int> &a,
                                      const std::vector<int> &b) {
  int i_min_size = a.size() > b.size() ? b.size() : a.size();
  for (int i = 0; i < i_min_size; i++) {
    if (a[i] > b[i]) {
      return 1;
    } else if (a[i] < b[i]) {
      return -1;
    }
  }
  return 0;
}
void VersionManager::GetVersion(const std::string &strVersion,
                                std::vector<int> *p_vec_version) {
  std::string strLast;
  std::stringstream ss;
  int i_val;
  for (int i = 0; i < strVersion.size(); i++) {
    if ('.' == strVersion[i]) {
      //字符串转换成整型
      ss << strLast;
      ss >> i_val;
      ss.str("");
      if (ss.eof()) {
        ss.clear();
      }
      p_vec_version->push_back(i_val);
      strLast.clear();
    } else if ('0' <= strVersion[i] && '9' >= strVersion[i]) {
      strLast += strVersion[i];
    }
  }

  ss << strLast;
  ss >> i_val;
  ss.str("");
  p_vec_version->push_back(i_val);
}


}  //  namespace meituan_mns
