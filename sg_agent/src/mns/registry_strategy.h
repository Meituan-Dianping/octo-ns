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

#ifndef SRC_MNS_REGISTRY_STRATEGY_H_
#define SRC_MNS_REGISTRY_STRATEGY_H_

#include "ServiceAgent.h"
#include "naming_service_types.h"
#include "route_base.h"
#include "version_manager.h"
#include "whitelist_manager.h"

namespace meituan_mns {

typedef boost::shared_ptr<SGService> SGServicePtr;

class RegistryStrategy {

 public:
  RegistryStrategy() {}
  ~RegistryStrategy() {}

  /**
  * 通过注册合法检测
  * @param oservice
  *
  * @return uint
  *
  */
  int32_t IsAllowedRegistry(const SGService &oservice);

  /**
   *
   * @param orgservice
   * @param zkservice
   * @return
   */
  bool IsRepeatedRegister(const SGService& orgservice,
                          const SGService& zkservice);

  /**
   * 注册协议检测
   * @param oservice
   *
   * @return uint
   *
   */
  int32_t CheckAllowedProtocolRegistry(SGServicePtr &oservice);

 private:
  /**
   * 注册参数合法检测
   * @param 注册参数SGService oservice
   *
   * @return uint
   *
   */
  int32_t CheckArgs(const SGService &oservice);
  /**
   *
   * @param oservice
   * @return
   */
  bool IsAllowMacRegistry(const SGService &oservice);

  /**
   *
   * @param oservice
   * @return
   */
  bool RegistryFilterCheck(const SGService &oservice);
  /**
   *
   * @param service
   * @return
   */
  bool IsLimitOnZk(const SGService &service);
  /**
   *
   * @param service
   * @return
   */
  bool CheckLegalOnOps(const SGService &service);

 protected:

 private:
  WhiteListManager whitelist_manager_;
  AgentZkPath agent_path_;
};
}  //  namespace meituan_mns

#endif //  SRC_MNS_REGISTRY_STRATEGY_H_
