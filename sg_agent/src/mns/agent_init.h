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

#ifndef SRC_MNS_AGENT_INIT_H_
#define SRC_MNS_AGENT_INIT_H_
#include "config_loader.h"
#include <muduo/net/EventLoop.h>
#include <muduo/base/CountDownLatch.h>
#include <muduo/net/EventLoopThread.h>
#include "naming_common_types.h"
#include <string>
namespace meituan_mns {

class AgentInit {
 public:
  AgentInit();
  ~AgentInit();
  void Init(void);
  int32_t HealthCheck(void);
  void HealthCheckFailed(void);
  int32_t PreloadLogConfigPath(std::string& log_path);
 protected:
 private:
  void SetLocolServiceNode(SGService& service);

  CXmlFile cxml_file_;
  bool is_zk_init_;
  bool is_loadfile_init_;
  bool is_disc_init_;
  muduo::CountDownLatch latch;
  int32_t healthy_interval_secs;
  muduo::net::EventLoopThread healthy_loop_thread_;
  muduo::net::EventLoop *healthy_loop_;
};
}   //  namespace meituan_mns

#endif   //  SRC_MNS_AGENT_INIT_H_
