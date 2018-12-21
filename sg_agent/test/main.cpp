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

#include <gtest/gtest.h>
#include <muduo/base/TimeZone.h>
#include <muduo/net/EventLoop.h>
#include <cthrift/cthrift_svr.h>
#include <iostream>
#include "mns/agent_server.h"
#include "mns/agent_init.h"
#include <TProcessor.h>
#include <pthread.h>
using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace meituan_mns;
typedef void *HANDLE;

extern "C" {
int InitHost();
}

void* StartServer(void* args) {

  string str_svr_appkey("octo.naming.sg_agent"); //服务端的appkey
  uint16_t u16_port = 5266;
  bool b_single_thread = false;  //当时单线程运行时，worker thread num 只能是1
  int32_t i32_timeout_ms = 30;
  int32_t i32_max_conn_num = 20000;
  int16_t i16_worker_thread_num = 4;

  std::cout << "start server........." << std::endl;

  try {
    boost::shared_ptr<meituan_mns::SGAgentServer> handler(new meituan_mns::SGAgentServer());
    meituan_mns::ServiceAgentProcessor *processor_tmp = new
        meituan_mns::ServiceAgentProcessor(handler);
    boost::shared_ptr<apache::thrift::TProcessor> processor(processor_tmp);

    meituan_cthrift::CthriftSvr server(processor);

    if(server.Init() != 0) {
      cerr << "server init error" << endl;
      return NULL;
    }
    meituan_mns::AgentInit agent_init;
    agent_init.Init();

    sleep(6);
    server.serve();
    std::cout << "start server...exit" << std::endl;
  } catch (meituan_cthrift::TException &tx) {
    return NULL;
  } catch (int e) {
    return (void*)(&e);
  }

  return 0;
}
int InitHost(void) {

  pthread_t tid;
  tid = pthread_create(&tid, NULL, StartServer, NULL);
  sleep(20);
  if(tid < 0){
    std::cout << "create thread failed..." << std::endl;
    return -1;
  }
  return 0;
}
int main(int argc, char **argv) {
  AgentInit agent_init;
  agent_init.Init();
  int ret = InitHost();
  if (0 != ret) {
    return -1;
  }
  testing::InitGoogleTest(&argc, argv);
  sleep(10);
  RUN_ALL_TESTS();
  return 0;
}

