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

#ifndef PROJECT_THRIFT_CONNECTION_H
#define PROJECT_THRIFT_CONNECTION_H


#include "log4cplus.h"
#include "naming_service_types.h"
#include "base_errors_consts.h"
#include "inc_comm.h"
#include "config_loader.h"

namespace meituan_mns {


class ThriftConn {
 public:
  ThriftConn();
  ~ThriftConn();

  int32_t CheckHandler();

  /**
   *
   * @param ip
   * @param port
   * @return
   */
  boost::shared_ptr<TProtocol> InitConnection(const std::string &ip, const int32_t port, const int32_t timeout);

 private:
  int32_t CreateConnection();

  int32_t CloseConnection();

 private:
  bool is_closed_;
  int retry_times_;
  std::string ip_;
  int port_;
  boost::shared_ptr<TSocket> socket_;
  boost::shared_ptr<TTransport> transport_;
  boost::shared_ptr<TProtocol> protocol_;

};


}

#endif //PROJECT_THRIFT_CONNECTION_H
