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

#ifndef OCTO_OPEN_SOURCE_ZK_CLIENT_H
#define OCTO_OPEN_SOURCE_ZK_CLIENT_H

extern "C" {
#include <zookeeper/zookeeper.h>
#include "cJSON.h"
}
#include <pthread.h>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/bind.hpp>
#include <muduo/base/Mutex.h>
#include <muduo/base/ThreadPool.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThread.h>
#include <muduo/net/EventLoopThreadPool.h>
#include <muduo/base/CountDownLatch.h>
#include "base_errors_consts.h"
#include "zk_invoke_param.h"

namespace  meituan_mns {

class ZkClient {
 public:
  ZkClient();
  ~ZkClient();
  //destroy object
  void Destroy();

  /**
   * @param server : ip:port list
   * @param zk_timeout : zookeeper_init rcv_timeout
   * @param zk_retrytimes : times of zk retry-connection
   * @return
   */
  int ZkInit(ZkClient *zk_client, const std::string &server, const int zk_timeout, const int zk_retrytimes);

  /**
   * @param request_ptr : zoo_get params
   * @param response_ptr : result
   * @return
   */
  int ZkGet(const ZkGetRequestPtr &request_ptr, ZkGetResponsePtr &response_ptr);

  /**
   * @param request_ptr : zoo_wget_children params
   * @param response_ptr
   * @return
   */
  int ZkWgetChildren(const ZkWGetChildrenRequestPtr &request_ptr, ZkWGetChildrenResponsePtr &response_ptr);

  /**
   * @param request_ptr : zk_wget params
   * @param response_ptr
   * @return
   */
  int ZkWget(const ZkWGetRequestPtr &request_ptr, ZkWGetResponsePtr &response_ptr);

  /**
   * @param request_ptr : zk_create params
   * @return
   */
  int ZkCreate(const ZkCreateRequestPtr &request_ptr);

  /**
  *创建节点
  *@param agent_service,zk_path,json,regCmd,uptCmd
  *
  *@return int
  *
  **/
  int32_t ZkPathCreateRecursivly(std::string &zk_path);

  /**
   * @param request_ptr : zk_set params
   * @return
   */
  int ZkSet(const ZkSetRequestPtr &request_ptr);

  /**
   * @param request_ptr : zk_exists params
   * @return
   */
  int ZkExists(const ZkExistsRequestPtr &request_ptr);

 private:
  int Connect2Zk();
  int CheckZkConn();
  int Reconnect2Zk();
  int ZkClose();

  static void ConnWatcher(zhandle_t *zh, int type, int state,
                          const char *zk_path, void *watcher_ctx);
  void RandSleep();

  static ZkClient *zk_client_;

  zhandle_t *zk_handle_;
  std::string zk_server_;
  int zk_timeout_;
  int zk_retrytimes_;
  unsigned int rand_try_times_;

};

}  //  namespace meituan_mns

#endif //OCTO_OPEN_SOURCE_ZK_CLIENT_H
