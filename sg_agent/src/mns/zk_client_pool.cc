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

#include "zk_client_pool.h"
#include "inc_comm.h"
#include "log4cplus.h"

namespace meituan_mns {
muduo::MutexLock ZkClientPool::zk_client_mutex_;
ZkClientPool *ZkClientPool::zk_client_instance_ = NULL;

const int kMaxZkClientNums = 10;

ZkClientPool::ZkClientPool() : started_(false),
                               next_(0),
                               zk_client_nums_(0) {

}
ZkClientPool *ZkClientPool::GetInstance() {

  if (NULL == zk_client_instance_) {
    muduo::MutexLockGuard lock(zk_client_mutex_);
    if (NULL == zk_client_instance_) {
      zk_client_instance_ = new ZkClientPool();
    }
  }
  return zk_client_instance_;
}

ZkClientPool::~ZkClientPool() {
  assert(started_);
  for (int i = 0; i < zk_client_pools_.size(); ++i) {
    SAFE_DELETE(zk_client_pools_[i]);
  }
}

int ZkClientPool::Init(const std::string &server, const int zk_client_nums,
                        const int zk_timeout, const int zk_retrytimes) {
  //assert(!started_);
  if (server.empty()) {
    NS_LOG_ERROR("zk_server list is empty.");
    return -1;
  }
  zk_client_nums_ = zk_client_nums > kMaxZkClientNums ?
                    kMaxZkClientNums : zk_client_nums;
  for (int i = 0; i < zk_client_nums_; ++i) {
    ZkClient *zk_client = new ZkClient();
    int ret = zk_client->ZkInit(zk_client, server, zk_timeout, zk_retrytimes);
    if (0 != ret) {
      NS_LOG_ERROR("failed to init zk.");
      continue;
    }
    zk_client_pools_.push_back(zk_client);
  }
  if (zk_client_pools_.empty()) {
    NS_LOG_ERROR("zk_client_pools is empty, server = "
                       << server << " , zk_client num = "
                       << zk_client_nums);
    return -1;
  }
  started_ = true;
  return 0;
}

ZkClient *ZkClientPool::GetZkClient() {
  assert(started_);
  ZkClient *zk_client = NULL;
  if (!zk_client_pools_.empty()) {
    zk_client = zk_client_pools_[next_];
    ++next_;
    next_ = static_cast<size_t>(next_) >= zk_client_pools_.size() ? 0 : next_;
  }
  return zk_client;
}

ZkClient *ZkClientPool::GetZkClientForHash(size_t hash_code) {
  assert(started_);
  ZkClient *zk_client = NULL;
  if (!zk_client_pools_.empty()) {
    zk_client = zk_client_pools_[hash_code % zk_client_pools_.size()];
  }
  return zk_client;
}
}  //  namespace meituan_mns
