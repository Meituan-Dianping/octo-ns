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

#include "thrift_connection.h"
#include "config_loader.h"
namespace meituan_mns {

ThriftConn::ThriftConn():retry_times_(CXmlFile::GetI32Para(CXmlFile::RetryTimes)) {}

ThriftConn::~ThriftConn() {
  CloseConnection();
}


int32_t ThriftConn::CheckHandler() {
  if (!transport_->isOpen()) {
    int32_t ret = CloseConnection();
    if (SUCCESS != ret) {
      NS_LOG_ERROR("Failed to close connection , ret = " << ret);
      return ret;
    }
    //重新建立连接
    ret = CreateConnection();
    if (SUCCESS != ret) {
      NS_LOG_WARN("Failed to create connection! ret = " << ret);
      return ret;
    }
  }
  return SUCCESS;
}

boost::shared_ptr<TProtocol> ThriftConn::InitConnection(const std::string &ip, const int32_t port, const int32_t timeout) {
  ip_ = ip;
  port_ = port;
  socket_ = boost::shared_ptr<TSocket>(new TSocket(ip, port));
  transport_ = boost::shared_ptr<TFramedTransport>(new TFramedTransport(socket_));
  protocol_ = boost::shared_ptr<TBinaryProtocol>(new TBinaryProtocol(transport_));
  socket_->setConnTimeout(timeout);
  socket_->setSendTimeout(timeout);
  socket_->setRecvTimeout(timeout);
  int32_t ret = CreateConnection();
  if (SUCCESS != ret) {
    NS_LOG_ERROR("init create connection fail! ret = " << ret);
    return boost::shared_ptr<TProtocol>();
  }
  return protocol_;
}

int32_t ThriftConn::CreateConnection() {
  int count = 0;
  while (!transport_->isOpen() && count < retry_times_) {
    ++count;
    try {
      transport_->open();
      NS_LOG_INFO("succeed to create connection, ip = " << ip_ << " port = " << port_);
    } catch (TException &e) {
      NS_LOG_WARN("Failed to create connection , error : " << e.what());
    }
  }
  return transport_->isOpen() ? SUCCESS : FAILURE;
}

int32_t ThriftConn::CloseConnection() {
  try {
    if (NULL != transport_) {
      transport_->close();
      NS_LOG_INFO("succeed to close connection, ip = " << ip_ << ", port = " << port_);
    } else {
      NS_LOG_ERROR("transport is NULL");
    }
  } catch (TException &e) {
    NS_LOG_ERROR("ERR, close connection fail! error : " << e.what());
    return ERR_CLOSE_CONNECTION;
  }
  return SUCCESS;
}

}
