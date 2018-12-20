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

#ifndef SRC_MNS_REGISTRY_SERVICE_H_
#define SRC_MNS_REGISTRY_SERVICE_H_

#include "registry_base.h"
#include "naming_data_types.h"
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/bind.hpp>
#include <muduo/base/Mutex.h>
#include <muduo/base/ThreadPool.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThread.h>
#include <muduo/net/EventLoopThreadPool.h>
#include <muduo/base/CountDownLatch.h>
#include "config_loader.h"
#include "version_manager.h"
#include "registry_strategy.h"
namespace meituan_mns {
class RegistryFactory {

 public:

  /**
    *注册创建实现
    *@param type 注册的类型:增量注册、批量注册
    *
    *@return 注册实体类型
    *
 **/
  Registry* CreateRegistry(RegistryType type);

  RegistryFactory() {}
  ~RegistryFactory() {}

 private:
 protected:
};

class RegistryService : public Registry {

 public:
  typedef boost::shared_ptr<SGService> SGServicePtr;
  typedef boost::shared_ptr<regist_req_param_t> RegParamPtr;

  static RegistryService* GetInstance();

  ~RegistryService(){}

  void RegistryHealthycheck() {}
  /**
  *注册实现
  *@param
  *
  *@return int
  *
  **/
  const uint32_t RegistryStart(const SGService &oService,
                               const int32_t &reg_action);
  const uint32_t RegistryStop(const SGService &oService) {}

  /**
  *服务下线实现
  *@param
  *
  *@return int
  *
  **/
  const uint32_t UnRegistryStart(const SGService &oService,
                                 const int32_t &reg_action);
  const uint32_t UnRegistryStop(const SGService &oService) {}

 private:

  RegistryService(){};
  /**
  *实际注册流程处理
  *@param
  *
  *@return void type
  *
  **/
  int32_t RegistryAction(RegParamPtr reg_service, const int32_t &reg_action);

  int32_t RegistryActionToZk(RegParamPtr reg_service, const int32_t &reg_action);

  int32_t RegistryActionToCache();

  int32_t RegistryActionToMixer();

  /**
  *写入方式取决注册中心类型，使用zookeeper
  *@param oservice,regCmd,uptCmd
  *
  *@return uint32_t type
  *
  **/
  int32_t RegistryServiceToZk(const SGService &oservice,
                               RegistCmd::type regCmd = RegistCmd::REGIST,
                               int uptCmd = UptCmd::RESET);

  /**
  *重新处理serviceinfo数据，如servicename
  *@param org_service,sg_service,regCmd
  *
  *@return void
  *
  **/
  void ResetRegistryServiceInfo(SGService &org_service,
                                const SGService &sg_service,
                                RegistCmd::type regCmd, int upt_cmd);
  /**
  *更新节点最后更改的时间
  *@param appkey,zk_provider_path,json
  *
  *@return int
  *
  **/
  int32_t UpdateLastModifiedTime(const std::string &appkey,
                                 const std::string &zk_provider_path);
  /**
  *创建节点
  *@param agent_service,zk_path,json,regCmd,uptCmd
  *
  *@return int
  *
  **/
  int32_t RegistryCreateZkNode(const SGService& agent_service,
                               const std::string &zk_path,
                               RegistCmd::type regCmd, int uptCmd);
  /**
  *创建servicename
  *@param oservice,uptCmd
  *
  *@return int
  *
  **/
  int32_t RegistryServiceNameToZk(const SGService &oservice,
                                                  int uptCmd);
  /**
  *更新节点数据
  *@param agent_service,zk_path,json,regCmd,uptCmd
  *
  *@return int
  *
  **/
  int32_t RegistryUpdateZkNode(const SGService& agent_service,
                               const std::string &zk_path,
                               RegistCmd::type regCmd, int uptCmd);
  /**
  *重新编辑servicename数据
  *@param agent_service,zk_path,json,regCmd,uptCmd
  *
  *@return int
  *
  **/
  int32_t ReEditServiceName(SGService &desService,
                            const SGService &srcService,
                            int uptCmd);

  int32_t RegistryServiceNameCreate(const std::string&
  service_name,const std::string& path);

  int32_t RegistryServiceNameNodeCreate(const SGService&
  oservice, const std::string& service_name,const std::string& path);

  int32_t UpdateServiceNameNodeTime(const SGService&
  oservice,const std::string& path);

 private:

  static RegistryService* registry_service_;
  static muduo::MutexLock registry_service_lock_;

  VersionManager version_manager_;

  AgentZkPath agent_path_;

  RegistryStrategy register_strategy_;

 protected:

};
class RegistryServiceBatch : public Registry {
 public:
  RegistryServiceBatch() {}
  ~RegistryServiceBatch() {}
  static RegistryServiceBatch* GetInstance();
  /**
  *注册实现
  *@param oService,reg_action
  *
  *@return int
  *
  **/
  const uint32_t RegistryStart(const SGService &oService,
                               const int32_t &reg_action) {}
  const uint32_t RegistryStop(const SGService &oService) {}

  const uint32_t UnRegistryStart(const SGService &oService,
                                 const int32_t &reg_action) {}
  const uint32_t UnRegistryStop(const SGService &oService) {}

 private:
  static RegistryServiceBatch * registry_service_batch_;
  static muduo::MutexLock registry_service_batch_lock_;

  RegistryStrategy register_strategy_;
  AgentZkPath agent_path_;
 protected:
};

}

#endif //  namespace  meituan_mns

