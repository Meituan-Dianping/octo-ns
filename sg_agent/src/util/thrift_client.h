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

#ifndef SG_AGENT_THRIFT_CLIENT_H
#define SG_AGENT_THRIFT_CLIENT_H
#include "log4cplus.h"
#include "naming_service_types.h"
#include "base_errors_consts.h"
#include "base_consts.h"
#include "inc_comm.h"
#include "config_loader.h"
#include "thrift_connection.h"
#include "mnsc_data_types.h"
namespace meituan_mns {
template<typename T>
class ThriftClient {
 public:
  ThriftClient() : thrift_client_(NULL) {}
  ~ThriftClient() {
    SAFE_DELETE(thrift_client_);
  }
  //get specific client
  T *GetClient() {
    return thrift_client_;
  }

  int32_t CreateThriftClient(const std::string &ip,const int32_t port,const int32_t timeout) {
    boost::shared_ptr<TProtocol> tmp_protocol =
        thrift_conn_.InitConnection(ip, port, timeout);
    if (NULL != tmp_protocol.get()) {
      thrift_client_ = new T(tmp_protocol);
      return SUCCESS;
    }
    NS_LOG_ERROR("thrift handler init failed! ip = " << ip
    << ", port = " << port);
    return FAILURE;
  }

  int32_t CheckConn() {
    return thrift_conn_.CheckHandler();
  }

 private:
  ThriftConn thrift_conn_;
  T *thrift_client_;

};
}

#endif //SG_AGENT_THRIFT_COMMON_CLIENT_H
