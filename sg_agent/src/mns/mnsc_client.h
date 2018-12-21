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


#ifndef SG_AGENT_MNSC_CLIENT_H
#define SG_AGENT_MNSC_CLIENT_H
#include "zk_invoke_param.h"
#include <muduo/base/Mutex.h>
#include "MNSCacheService.h"
#include "mnsc_data_types.h"
#include "thrift_client.h"
#include <boost/shared_ptr.hpp>

namespace meituan_mns {


typedef boost::shared_ptr<ThriftClient<MNSCacheServiceClient> > MnscThriftClientPtr;

class MnscClient {
 public:
  MnscClient();
  ~MnscClient();
  static MnscClient* GetInstance();
  /**
   * 从MNS Cache服务获取服务列表
   * @param service_list
   * @param params
   * @return
   */
  int32_t GetServicelistFromMnsc(std::vector<SGService> &service_list,
                                 const GetMnsCacheParams& params);

 private:
  /**
   * MNS Cache rpc接口
   * @param service_list, params
   * @return
   */
  int32_t GetMnscCache(std::vector<SGService> &service_list,
                       const GetMnsCacheParams& params);
  /**
   * 获取MNSC服务节点列表
   * @param service_list
   * @return
   */
  int32_t GetMnscServiceList(std::vector<SGService> &service_list);


  /**
   *
   * @return
   */
  int32_t GetMnscThriftClient(MnscThriftClientPtr &mnsc_thrift_client);


  /**
  *
  * @param appkey,service_list
  * @return int32_t
  */
  int32_t GetThriftServiceList(const std::string &appkey, std::vector<SGService> &service_list);

 private:
  uint32_t retry_times_;
  int32_t timeout_;
  std::string mnsc_appkey_;

  static MnscClient *mnsc_client_;
  static muduo::MutexLock mnsc_client_lock_;

};



}  //  namespace meituan_mns

#endif //SG_AGENT_MNSC_CLIENT_H
