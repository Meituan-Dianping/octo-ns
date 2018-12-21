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

#ifndef DYNAMICLOADCONFIG_H
#define DYNAMICLOADCONFIG_H

#include <muduo/base/Mutex.h>
#include "muduo/net/EventLoop.h"
#include "muduo/net/EventLoopThread.h"
#include "config_loader.h"

namespace meituan_mns {
class DynamicLoadConfig {
 public:
  static DynamicLoadConfig* GetInstance(void);

  ~DynamicLoadConfig();

  void Init(void);

  void UpdateIdcsTimer(void);

  std::string CalculateMd5(const std::string &file_name,
                           const std::string &file_path);
 private:
  DynamicLoadConfig();

  static DynamicLoadConfig* dynamic_load_config_;
  static muduo::MutexLock dynamic_load_config_lock_;
  CXmlFile cxml_file_;

  muduo::net::EventLoopThread check_thread_;
  muduo::net::EventLoop *check_loop_;

  std::string idc_md5_;
  std::string appkeys_md5_;

};

}

#endif //OCTO_OPEN_SOURCE_上午11_17_21_DYNAMICLOADCONFIG_H
