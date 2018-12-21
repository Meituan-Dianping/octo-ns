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

#include <string.h>
#include "thrift_client.h"

namespace mns_sdk {

ThriftClientHandler::ThriftClientHandler()
    : m_client(NULL) {
}
ThriftClientHandler::~ThriftClientHandler() {
  if (m_client) {

    closeConnection();

    if (SG_AEGNT_TYPE == type) {
      delete static_cast<meituan_mns::ServiceAgentClient *>(m_client);
    } else {
      MNS_LOG_ERROR("Wrong Type: " << type);
    }
  }
}
int ThriftClientHandler::init(const std::string &host,
                              int port,
                              ProcType proc_type,
                              bool local) {
  m_host = host;
  m_port = port;
  type = proc_type;
  m_socket =
      boost::shared_ptr<apache::thrift::transport::TSocket>(new apache::thrift::transport::TSocket(
          m_host,
          m_port));
  m_transport =
      boost::shared_ptr<apache::thrift::transport::TFramedTransport>(new apache::thrift::transport::TFramedTransport(
          m_socket));
  m_protocol =
      boost::shared_ptr<apache::thrift::protocol::TBinaryProtocol>(new apache::thrift::protocol::TBinaryProtocol(
          m_transport));
  int ret = createConnection();
  if (ret != 0) {
    MNS_LOG_ERROR("init create connection fail! ret = " << ret);
    return ret;
  }
  //创建成功，置连接标示为未删除
  m_closed = false;
  int32_t time_out = 0;
  if (NULL == m_client) {
    if (SG_AEGNT_TYPE == type) {
      if (local) {
        time_out = SG_LOCAL_SERVER_CONN_TIMEOUT;
        m_socket->setConnTimeout(SG_LOCAL_SERVER_CONN_TIMEOUT);
        m_socket->setSendTimeout(SG_LOCAL_SERVER_WRITE_TIMEOUT);
        m_socket->setRecvTimeout(SG_LOCAL_SERVER_READ_TIMEOUT);
      } else {
        time_out = SG_SERVER_CONN_TIMEOUT;
        m_socket->setConnTimeout(SG_SERVER_CONN_TIMEOUT);
        m_socket->setSendTimeout(SG_SERVER_WRITE_TIMEOUT);
        m_socket->setRecvTimeout(SG_SERVER_READ_TIMEOUT);
      }

      m_client =
          static_cast<void *>(new meituan_mns::ServiceAgentClient(m_protocol));
    } else {
      m_client = NULL;
    }
  }
  MNS_LOG_INFO("connect to ThriftClientHandler ip: " << m_host
                                                     << " port is : " << m_port
                                                     << " type is : "
                                                     << proc_type
                                                     << ", timeout = "
                                                     << time_out);
  srand(static_cast<unsigned int>(time(0)));
  return 0;
}
//check连接
int ThriftClientHandler::checkConnection() {
  if (!m_transport->isOpen()) {
    return ERR_CHECK_CONNECTION;
  }
  return 0;
}
//创建连接
int ThriftClientHandler::createConnection() {
  int count = 0;
  bool flag = true;
  //先check连接是否可用
  int ret = checkConnection();
  //如果连接不可用，则重新建立连接
  while ((ret != 0) && count < CONNECTION_RETRY_TIMES) {
    count++;
    try {
      m_transport->open();
      flag = true;
      MNS_LOG_INFO("reconnect to  ok! ip: " << m_host.c_str()
                                            << " port is : " << m_port);
    }
    catch (apache::thrift::TException &e) {
      flag = false;
      MNS_LOG_ERROR("reconnect to  failed ip: " << m_host.c_str()
                                                << " port is : " << m_port
                                                << ", error : " << e.what());
    }
    //获取连接状态
    ret = checkConnection();
  }
  if (flag && ret == 0) {
    return 0;
  } else {
    MNS_LOG_ERROR("ThriftClientHandler failed! ret = "
                      << ret << "flag = " << flag);
    return ERR_CREATE_CONNECTION;
  }
}
//关闭连接
int ThriftClientHandler::closeConnection() {
  if (!m_closed) {
    //置连接关闭标示为true，防止多次close
    m_closed = true;
    try {
      MNS_LOG_INFO("begin close connection !");
      if (NULL != m_transport) {
        m_transport->close();
      } else {
        MNS_LOG_ERROR("m_transport is NULL when to close");
      }
    }
    catch (apache::thrift::TException &e) {
      MNS_LOG_ERROR("ERR, close connection fail! error : " << e.what());
      return ERR_CLOSE_CONNECTION;
    }
  }
  return 0;
}
//check handler对应的连接是否可用
int ThriftClientHandler::checkHandler() {
  //先check连接
  int ret = checkConnection();
  if (ret != 0) {
    MNS_LOG_ERROR(" connection lost, begin close! ret = " << ret);
    //关闭连接
    ret = closeConnection();
    if (ret != 0) {
      MNS_LOG_ERROR(" connection lost, close fail! ret = " << ret);
      return ret;
    }
    //重新创建连接
    ret = createConnection();
    if (ret != 0) {
      MNS_LOG_ERROR(" re-create connection fail! ret = " << ret);
      return ret;
    }
    //创建成功，修改连接标示为连接未被关闭
    m_closed = false;
  }
  return 0;
}
}
