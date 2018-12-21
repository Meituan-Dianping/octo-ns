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

#ifndef OCTO_OPEN_SOURCE_ZK_CLIENT_POOL_H
#define OCTO_OPEN_SOURCE_ZK_CLIENT_POOL_H
#include <vector>
#include "zk_client.h"

namespace meituan_mns {
class ZkClientPool {
 public:

  static ZkClientPool *GetInstance();
  /**
   * @param server : ip:port list
   * @param zk_client_num < 10
   * @param zk_timeout : zookeeper_init rcv_timeout
   * @param zk_retrytimes : times of zk retry-connection
   * @return
   */
  int Init(const std::string &server, const int zk_client_nums,
            const int zk_timeout, const int zk_retrytimes);

  ZkClient *GetZkClient();

  ZkClient *GetZkClientForHash(size_t hash_cade);

  bool started() const { return started_; }

 private:
  ZkClientPool();
  ~ZkClientPool();
  bool started_;
  int zk_client_nums_;
  int next_;
  std::vector<ZkClient *> zk_client_pools_;

  static ZkClientPool *zk_client_instance_;
  static muduo::MutexLock zk_client_mutex_;

};

}  //  namespace meituan_mns
#endif //OCTO_OPEN_SOURCE_ZK_CLIENT_POOL_H
