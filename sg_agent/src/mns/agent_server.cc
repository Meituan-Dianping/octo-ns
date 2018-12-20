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

#include <regex.h>
#include "agent_server.h"
#include "registry_service.h"
#include "naming_common_types.h"
#include "registry_base.h"
#include "service_channels.h"
#include "discovery_service.h"

namespace meituan_mns {

SGAgentServer::SGAgentServer() {
}

int SGAgentServer::Init() {
  return 0;
}
/**
 *@ rpc注册接口
 *
 *@param 上下线类型
 *@param SGService 注册参数
 *
 *@return int
 *
 * */

int32_t SGAgentServer::registServicewithCmd(const int32_t uptCmd,
                                            const SGService &oService) {
  NS_LOG_INFO("the registry cmd = "<< uptCmd
                                     <<"registry appkey = "<< oService.appkey
                                     <<"registry ip = "<< oService.ip);
  int ret = -1;

  boost::shared_ptr<RegistryFactory> registry_factory =
      boost::make_shared<RegistryFactory>();

  Registry *registry = registry_factory->CreateRegistry((RegistryType)uptCmd);

  ret = registry->RegistryStart(oService, uptCmd);

  return ret;
}
/**
 *@ rpc服务下线接口
 *
 *@param SGService 注册参数
 *
 *@return int
 *
 * */
int32_t SGAgentServer::unRegistService(const SGService &oService) {
  NS_LOG_INFO("the unRegistry"<< oService.appkey <<" ;registry ip = "
                                << oService.ip);
  int ret = -1;

  boost::shared_ptr<RegistryFactory> registry_factory = boost::make_shared<RegistryFactory>();

  Registry *registry = registry_factory->CreateRegistry(kAddReg);

  ret = registry->RegistryStart(oService, RegistCmd::UNREGIST);

  return ret;
}

void SGAgentServer::getServiceListByProtocol(ProtocolResponse &_return, const ProtocolRequest &req) {
  boost::shared_ptr<ServiceChannels> service_channel = boost::make_shared<ServiceChannels>();
  service_channel->SetAllChannel(false);
  service_channel->SetBankboneChannel(false);
  service_channel->SetOriginChannel(false);
  service_channel->SetSwimlaneChannel(false);

  DiscoveryService::GetInstance()->DiscGetSrvList(_return.servicelist, req, service_channel);

  NS_LOG_INFO("getOriginServiceList size = " << _return.servicelist.size()
                                               << ", remoteAppkey = " << req.remoteAppkey
                                               << ", protocol = " << req.protocol
                                               << ", errorCode = " << _return.errcode);

}

void SGAgentServer::getOriginServiceList(ProtocolResponse &_return,
                                         const ProtocolRequest &req) {
  boost::shared_ptr<ServiceChannels> service_channel =
      boost::make_shared<ServiceChannels>();
  service_channel->SetAllChannel(false);
  service_channel->SetBankboneChannel(false);
  service_channel->SetOriginChannel(true);
  service_channel->SetSwimlaneChannel(false);

  DiscoveryService::GetInstance()->DiscGetSrvList(_return.servicelist,
                                                  req, service_channel);

  NS_LOG_INFO("getOriginServiceList size = "
                  << _return.servicelist.size()
                  << ", remoteAppkey = " << req.remoteAppkey
                  << ", protocol = " << req.protocol
                  << ", errorCode = " << _return.errcode);

}

int32_t SGAgentServer::registService(const SGService &oService) {
  NS_LOG_INFO("the registry appkey = "<<oService.appkey
                                     <<"registry ip = "<<oService.ip);
  int ret = -1;

  boost::shared_ptr<RegistryFactory> registry_factory =
      boost::make_shared<RegistryFactory>();

  Registry *registry = registry_factory->CreateRegistry(kAddReg);

  ret = registry->RegistryStart(oService,UptCmd::ADD);

  return ret;
}

}  //  namespace meituan_mns
