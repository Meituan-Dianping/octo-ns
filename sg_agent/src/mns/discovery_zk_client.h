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

#ifndef OCTO_OPEN_SOURCE_DISCOVERY_ZK_CLIENT_H
#define OCTO_OPEN_SOURCE_DISCOVERY_ZK_CLIENT_H

#include "naming_service_types.h"
#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThread.h>
#include <muduo/net/EventLoopThreadPool.h>
#include <muduo/base/CountDownLatch.h>
#include "naming_data_types.h"
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <muduo/base/Thread.h>
#include <stdio.h>
#include "version_manager.h"
#include "zk_path_tools.h"
#include "zk_invoke_param.h"
#include "mnsc_client.h"

extern "C" {
#include <zookeeper/zookeeper.h>
#include "comm/cJSON.h"
}

namespace meituan_mns {


class DiscoveryZkClient {
 public:

  void Init(void);

  static DiscoveryZkClient *GetInstance(void);

  typedef boost::function<void(muduo::net::EventLoop *) > ThreadInitCallback;
  /**
   *
   * @param srvlist
   * @param localAppkey
   * @param appKey
   * @param protocol
   * @param is_watcher_callback
   * @return
   */
  int32_t DiscSrvListByProtocol(boost::shared_ptr<getservice_res_param_t> &service,
                                bool is_watcher_callback = false);

  /**
   *
   * @param routeList
   * @param localAppkey
   * @param appKey
   * @param version
   * @param protocol
   * @return
   */
  int32_t DiscRouteListByProtocol(std::vector<CRouteData> &routeList,
                                  const std::string &localAppkey,
                                  const std::string &appKey,
                                  std::string &version,
                                  const std::string &protocol,
                                  bool &route_flag);

  /**
   *
   * @param zh
   * @param type
   * @param state
   * @param path
   * @param watcherCtx
   */
  static void DiscByProtocolWatcher(zhandle_t *zh, int32_t type, int32_t state,
                                    const char *path, void *watcherCtx);
  /**
   *
   * @param appkeys
   * @param local_appkey
   * @param service_name
   * @param version
   * @param protocol
   * @return
   */

  int32_t DiscAppkeyByServiceName(std::set<std::string> &appkeys,
                                  const std::string &local_appkey,
                                  const std::string &service_name,
                                  std::string &version,
                                  const std::string
                                  &protocol);

 private:

  int32_t DiscoveryListFromCache(std::vector<SGService> &srvlist,
                                 GetMnsCacheParams& params);
  /**
   *
   * @param index
   * @param begin
   * @param threads
   * @param end
   * @param child_count
   */
  void DivRangeDiscIndex(const int32_t &index,
                         int32_t &begin,
                         int32_t &end,
                         const int32_t &child_count);

  int32_t GetSubSrvListFromZk(SubGetServiceResPtr &sub_res,
                              const SubGetServiceParams &sub_params,
                              muduo::CountDownLatch *p_countdown);


  DiscoveryZkClient(){};
  ~DiscoveryZkClient() {};



  static DiscoveryZkClient *disc_zk_instance_;

  /**
  *初始化注册处理线程
  *@param mns_loop,num_threads,name
  *
  *@return void type
  *
  **/
  void InitDiscThreadPool(muduo::net::EventLoop *mns_loop,
                              int32_t num_threads, const std::string &name);
  /**
   *
   * @param cb
   */
  void SetThreadInitCallback(const ThreadInitCallback &cb) { threadInitCallback_ = cb; };

  void ThreadInfo(void);


 private:

  AgentZkPath agent_zk_path_;
  VersionManager version_manager_;
  WhiteListManager whitelist_manager_;

  ThreadInitCallback threadInitCallback_;
  muduo::net::EventLoopThread disc_loop_thread_;
  muduo::net::EventLoop *disc_loop_;
  static muduo::net::EventLoopThreadPool *disc_pool_;
  std::vector<muduo::net::EventLoop *> disc_loops_;
  static muduo::MutexLock disc_zk_service_lock_;

 protected:

};

}  //  namespace meituan_mns



#endif //OCTO_OPEN_SOURCE_DISCOVERY_ZK_CLIENT_H
