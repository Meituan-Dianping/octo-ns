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

#ifndef SRC_MNS_AGENT_SERVER_H_
#define SRC_MNS_AGENT_SERVER_H_
#include <string>
#include "comm/tinyxml2.h"
#include "ServiceAgent.h"
#include "naming_service_types.h"
#include "log4cplus.h"


#define HANDLERNAME "sg_agent"

using namespace __gnu_cxx;
namespace meituan_mns {
const size_t NMATCH = 10;

class SGAgentServer : virtual public ServiceAgentIf {
 public:
  SGAgentServer();

  int Init();

  /**
   * 按协议类型获取服务列表
   * @param _return,req
   *
   * @return void
   *
   * */
  void getServiceListByProtocol(ProtocolResponse &_return, const ProtocolRequest &req);
  /**
   * 按权重获取服务列表
    * @param _return,req
    *
    * @return void
    *
    * */
  void getOriginServiceList(ProtocolResponse &_return, const ProtocolRequest &req);
  /**
    * 服务注册
    * @param oService
    *
    * @return int32_t
    *
    * */
  int32_t registService(const SGService &oService);

  int32_t registServicewithCmd(const int32_t uptCmd, const SGService &oService);
  /**
   * 服务下线
    * @param oService
    *
    * @return int32_t
    *
  * */
  int32_t unRegistService(const SGService &oService);


};

}  //  namespace meituan_mns

#endif
