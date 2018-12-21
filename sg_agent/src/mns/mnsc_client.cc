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


#include "mnsc_client.h"
#include "naming_service_types.h"
#include "config_loader.h"
#include "discovery_service.h"
#include "route_rule.h"
namespace meituan_mns {

MnscClient *MnscClient::mnsc_client_ = NULL;
muduo::MutexLock MnscClient::mnsc_client_lock_;
const int MNSC_DEFAULT_SLEEPTIME = 30000;

MnscClient::MnscClient() :
    mnsc_appkey_(CXmlFile::GetStrPara(CXmlFile::MNSCacheAppkey)),
    retry_times_(CXmlFile::GetI32Para(CXmlFile::RetryTimes)),
    timeout_(CXmlFile::GetI32Para(CXmlFile::MnscTimeOut)) {}

MnscClient::~MnscClient() {}

MnscClient *MnscClient::GetInstance() {
  if (NULL == mnsc_client_) {
    muduo::MutexLockGuard lock(mnsc_client_lock_);
    if (NULL == mnsc_client_) {
      mnsc_client_ = new MnscClient();
    }
  }
  return mnsc_client_;
}

int32_t MnscClient::GetMnscThriftClient(MnscThriftClientPtr
&mnsc_thrift_client) {
  std::vector<SGService> service_list;
  int32_t ret = GetThriftServiceList(mnsc_appkey_, service_list);
  if (SUCCESS != ret) {
    NS_LOG_WARN("GetThriftServiceList failed,ret = "<<ret<<";service_list = "
        ""<<service_list.size());
    return ret;
  }
  size_t size = service_list.size();
  int begin_index = rand() % size;
  int rand_index = begin_index;
  do {
    ret = mnsc_thrift_client->CreateThriftClient(service_list[rand_index].ip,
                                                 service_list[rand_index].port,
                                                 timeout_);
    if (SUCCESS == ret) {
      return SUCCESS;
    }
    rand_index = (rand_index + 1) % size;
  } while(rand_index != begin_index);

  return FAILURE;
}

int32_t MnscClient::GetMnscCache(std::vector<SGService> &service_list, const GetMnsCacheParams &params) {
  MnscThriftClientPtr mnsc_thrift_client =
      boost::make_shared<ThriftClient<MNSCacheServiceClient> >();
  int32_t ret = GetMnscThriftClient(mnsc_thrift_client);
  if (SUCCESS != ret) {
    NS_LOG_WARN("Failed to get mnsc thrift client.");
    return ret;
  }
  MNSResponse mnscache_res;
  try {
    //调用mnscache接口
    if ("thrift" == params.protocol) {
      mnsc_thrift_client->GetClient()->getMNSCache(mnscache_res, params.appkey, params.version, params.env);
    } else if ("http" == params.protocol) {
      mnsc_thrift_client->GetClient()->getMNSCacheHttp(mnscache_res, params.appkey, params.version, params.env);
    } else {
      NS_LOG_ERROR("invalid protocol: " << params.protocol);
    }
    //提取返回的服务列表
    if (meituan_mns::MNSC_OK == mnscache_res.code) {
      service_list = mnscache_res.defaultMNSCache;
      NS_LOG_DEBUG("getMNSCache ok! appkey : " << params.appkey
                                                 << ", serviceList size is : " << service_list.size()
                                                 << ", env is : " << params.env);
    } else {
      NS_LOG_ERROR("getMNSCache wrong errcode. appkey : " << params.appkey
                                                            << ", env is : " << params.env
                                                            << ", errCode : " << mnscache_res.code);
    }
  } catch (TException &e) {
    NS_LOG_ERROR("getMNSCache catch error! msg: " << e.what()
                                                    << ", serviceList size is : " << service_list.size()
                                                    << ", env is : " << params.env
                                                    << ", appkey : " << params.appkey);
    return ERR_MNSC_GET_MNSCACHE;
  }
  return mnscache_res.code;
}

int32_t MnscClient::GetServicelistFromMnsc(std::vector<SGService> &service_list, const GetMnsCacheParams &params) {

  timeval tvalStart;
  timeval tvalEnd;
  long deltaTime;
  gettimeofday(&tvalStart, NULL);
  uint32_t times = 0;
  int32_t ret = 0;
  do {
    //访问MNS_Cache
    ret = GetMnscCache(service_list, params);
    if (meituan_mns::MNSC_OK == ret) {
      //getCache正常返回
      break;
    } else if (meituan_mns::MNSC_UPDATING == ret) {
      //getCache返回500, 表示MNSCache正在watcher更新, sleep重试一次
      gettimeofday(&tvalEnd, NULL);
      deltaTime = (tvalEnd.tv_sec - tvalStart.tv_sec)
          * 1000000L
          + (tvalEnd.tv_usec - tvalStart.tv_usec);
      usleep(MNSC_DEFAULT_SLEEPTIME);
    } else {
      //其他错误，表示MNSCache异常，重试3次，
      //若都失败则退出循环，直连ZK再次getServerList
      ++times;
      if (times < meituan_mns::MNSC_RETRY_TIME) {
        NS_LOG_WARN("getMNSCache fail , ret code : "
                          << ret
                          << ", appkey : " << params.appkey
                          << ", need to retry, count : " << times);
        continue;
      } else {
        break;
      }
    }
  } while (retry_times_ > times);
  return ret;
}
int32_t MnscClient::GetThriftServiceList(const std::string &appkey, std::vector<SGService> &service_list) {
  boost::shared_ptr<ServiceChannels> service_channel = boost::make_shared<ServiceChannels>();
  service_channel->SetAllChannel(false);
  service_channel->SetBankboneChannel(false);
  service_channel->SetOriginChannel(false);
  service_channel->SetSwimlaneChannel(false);

  ProtocolRequest req;
  req.__set_localAppkey("sgAgent");
  req.__set_protocol("thrift");
  req.__set_remoteAppkey(appkey);

  int32_t ret = DiscoveryService::GetInstance()->DiscGetSrvList(service_list, req, service_channel);
  if (SUCCESS != ret) {
    NS_LOG_ERROR("ERR getService list failed, ret: "
                       << ret << ", appkey : " << appkey);
    return ERR_FAILEDTOGETCONFSERVLIST;
  }
  RouteRule::FilterUnAlive(service_list);
  if (service_list.empty()) {
    NS_LOG_ERROR("ERR service list return null ");
    return ERR_SERVICELIST_NULL;
  }
  ret = RouteRule::FilterWeight(service_list, IdcThresHold);
  if (SUCCESS != ret) {
    NS_LOG_DEBUG("result from IDC filte is empty");
    ret = meituan_mns::RouteRule::FilterWeight(service_list, RegionThresHold);
    if (SUCCESS != ret) {
      NS_LOG_WARN("result from Region filte is empty");
    }
  }
  return SUCCESS;
}

}  //  namespace meituan_mns
