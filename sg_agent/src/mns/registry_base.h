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

#ifndef SRC_MNS_REGISTRY_BASE_H_
#define SRC_MNS_REGISTRY_BASE_H_
#include <stdio.h>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include "base_consts.h"
#include "naming_service_types.h"

namespace meituan_mns {

class Registry {

 public:
  Registry() {}
  ~Registry() {}
  /**
   * 提供注册操作，用于异常终止及撤销
   * @param
   *
   * @return
   *
   */
  virtual const uint32_t RegistryStart(const SGService &oService,
                                       const int32_t &reg_action)=0;
  virtual const uint32_t RegistryStop(const SGService &oService)=0;

  /**
   * 提供下线操作，用于异常终止及撤销
   * @param
   *
   * @return
   *
   */
  virtual const uint32_t UnRegistryStart(const SGService &oService, const int32_t &reg_action)=0;
  virtual const uint32_t UnRegistryStop(const SGService &oService)=0;

 private:
 protected:
};
}

#endif //  SRC_MNS_REGISTRY_BASE_H_
