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

#include <algorithm>

#include <boost/algorithm/string.hpp>

#include "mns_sdk.h"

using namespace std;
using namespace mns_sdk;

namespace mns_sdk {

extern mns_sdk::MnsConfig g_config;

struct ServiceInfo {
  meituan_mns::ProtocolRequest protoreq_;
  vector<SvrListCallback> cb_list_;
  vector<UpdateSvrListCallback> update_cb_list_;
  vector<meituan_mns::SGService> sgservice_;

  boost::unordered_map<string, meituan_mns::SGService>
      map_ipport_sgservice_;

  ServiceInfo() {
  }

  void SetProtocolReq(const string &str_cli_appkey,
                      const string &str_svr_appkey,
                      const string &str_proto_type,
                      const string &str_service_name) {
    protoreq_.__set_localAppkey(str_cli_appkey);
    protoreq_.__set_remoteAppkey(str_svr_appkey);
    protoreq_.__set_protocol(str_proto_type);
    protoreq_.__set_serviceName(str_service_name);
  }

  void AddSvrListCallback(const SvrListCallback &cb) {
    cb_list_.push_back(cb);
  }

  void AddUpdateSvrListCallback(const UpdateSvrListCallback &cb) {
    update_cb_list_.push_back(cb);
  }
};

string g_local_ip_port;

int ref_count_ = 0;

muduo::MutexLock init_mutex;
muduo::net::EventLoop *mns_eventloop_p_ = NULL;

boost::shared_ptr<muduo::net::EventLoopThread> mns_worker_thread_sp_;

typedef muduo::ThreadLocalSingleton <boost::shared_ptr<MnsWorker> >
    ThreadLocalSingletonSGAgentClientSharedPtr;

typedef muduo::ThreadLocalSingleton <boost::unordered_map<string, ServiceInfo> >
    ThreadLocalSingletonMap;

void UptSvrList(const string &, ServiceInfo &);
bool FilterService(const meituan_mns::SGService &sg,
                   const string &service_name);
void GetSvrList(void);
void GetSvrList(const string &str_svr_appkey,
                const string &str_cli_appkey,
                const string &str_proto_type,
                const string &str_service_name,
                vector<meituan_mns::SGService> *res_svr_list,
                muduo::CountDownLatch *p_countdown);
void DoAddUpdateSvrListCallback(const string &str_svr_appkey,
                                const UpdateSvrListCallback &cb,
                                string *p_err_info,
                                muduo::CountDownLatch *countdown_set_callback);
void DoAddSvrListCallback(const string &str_svr_appkey,
                          const SvrListCallback &cb,
                          string *p_err_info,
                          muduo::CountDownLatch *countdown_set_callback);
void InitCthrift(void);
void InitMap(void);
void InitWorkerWithSvrListCB(const string &str_svr_appkey,
                             const string &str_cli_appkey,
                             const string &str_proto_type,
                             const string &str_service_name,
                             const SvrListCallback &cb);
void InitWorkerWithUptSvrListCB(const string &str_svr_appkey,
                                const string &str_cli_appkey,
                                const string &str_proto_type,
                                const string &str_service_name,
                                const UpdateSvrListCallback &cb);
int8_t StartSvr(const meituan_mns::SGService &sgService);

string InitLocalAddr(void);
} // namespace mns_sdk

string mns_sdk::InitLocalAddr(void) {
  string str_sgagent_default_port;
  try {
    str_sgagent_default_port =
        boost::lexical_cast<std::string>(g_config.local_chrion_port_);
  } catch (boost::bad_lexical_cast &e) {
    MNS_LOG_ERROR("boost::bad_lexical_cast :" << e.what()
                                              << " port : "
                                              << g_config.local_chrion_port_);
  }

  return g_config.str_local_ip_ + ":" + str_sgagent_default_port;
}

void mns_sdk::UptSvrList(const string &appkey, ServiceInfo &service_info) {
  vector<meituan_mns::SGService>::const_iterator it_vec;
  boost::unordered_map<string, meituan_mns::SGService>::iterator
      it_map_sgservice;

  string str_port;

  vector<meituan_mns::SGService> vec_sgservice_add;
  vector<meituan_mns::SGService> vec_sgservice_del;
  vector<meituan_mns::SGService> vec_sgservice_chg;

  const vector<meituan_mns::SGService> &vec_sgservice = service_info.sgservice_;
  boost::unordered_map<string, meituan_mns::SGService>
      &g_map_ipport_sgservice_ =
      service_info.map_ipport_sgservice_;
  if (MNS_UNLIKELY(
      0 == g_map_ipport_sgservice_.size() && 0 == vec_sgservice.size())) {
    MNS_LOG_WARN("Init svr list but empty srvlist");
  } else if (MNS_UNLIKELY(0 == g_map_ipport_sgservice_.size())) {
    it_vec = vec_sgservice.begin();
    MNS_LOG_INFO("Init svr list for appkey: " << it_vec->appkey);

    while (it_vec != vec_sgservice.end()) {
      if (MNS_UNLIKELY(2 != it_vec->status)) {
        MNS_LOG_DEBUG("svr info: " << MnsSdkCommon::SGService2String(*it_vec)
                                   << " IGNORED");
        ++it_vec;
        continue;
      }

      try {
        str_port = boost::lexical_cast<std::string>(it_vec->port);
      } catch (boost::bad_lexical_cast &e) {
        MNS_LOG_ERROR("boost::bad_lexical_cast :"
                          << e.what() << "svr info : "
                          << MnsSdkCommon::SGService2String(
                                                      *it_vec));
        ++it_vec;
        continue;
      }

      g_map_ipport_sgservice_.insert(make_pair(it_vec->ip + ":" + str_port,
                                               *it_vec));

      vec_sgservice_add.push_back(*(it_vec++));
    }
  } else if (MNS_UNLIKELY(0 == vec_sgservice.size())) {
    MNS_LOG_WARN("vec_sgservice empty");

    it_map_sgservice = g_map_ipport_sgservice_.begin();
    while (it_map_sgservice != g_map_ipport_sgservice_.end()
        && (g_local_ip_port)
            != it_map_sgservice->first) {    //exclude local sgagent since sentinel list NOT include it
      vec_sgservice_del.push_back(it_map_sgservice->second);
      g_map_ipport_sgservice_.erase(it_map_sgservice++);
    }
  } else {
    boost::unordered_map<string, meituan_mns::SGService>
        map_tmp_locate_del(g_map_ipport_sgservice_);

    map_tmp_locate_del.erase(g_local_ip_port); //exclude local sgagent

    it_vec = vec_sgservice.begin();
    while (it_vec != vec_sgservice.end()) {
      if (MNS_UNLIKELY(2 != it_vec->status)) {
        MNS_LOG_DEBUG("svr info: " << MnsSdkCommon::SGService2String(*it_vec)
                                   << " IGNORED");
        ++it_vec;
        continue;
      }

      try {
        str_port = boost::lexical_cast<std::string>(it_vec->port);
      } catch (boost::bad_lexical_cast &e) {

        MNS_LOG_ERROR("boost::bad_lexical_cast :" << e.what()
                                                  << "svr info : "
                                                  << MnsSdkCommon::SGService2String(
                                                      *it_vec));

        ++it_vec;
        continue;
      }

      string str_key(it_vec->ip + ":" + str_port);
      it_map_sgservice = g_map_ipport_sgservice_.find(str_key);
      if (g_map_ipport_sgservice_.end() == it_map_sgservice) {
        MNS_LOG_DEBUG("ADD svr list info: "
                          << MnsSdkCommon::SGService2String(*it_vec));

        vec_sgservice_add.push_back(*it_vec);
        g_map_ipport_sgservice_.insert(make_pair(str_key, *it_vec));
      } else {
        map_tmp_locate_del.erase(str_key);

        if (it_map_sgservice->second != *it_vec) {
          MNS_LOG_DEBUG(
              "UPDATE svr list. old info: "
                  << MnsSdkCommon::SGService2String(it_map_sgservice->second)
                  << " new info: "
                  << MnsSdkCommon::SGService2String(*it_vec));

          it_map_sgservice->second = *it_vec;

          vec_sgservice_chg.push_back(*it_vec);

          map_tmp_locate_del.erase(str_key);
        }
      }

      ++it_vec;
    }

    if (map_tmp_locate_del.size()) {
      MNS_LOG_DEBUG("DEL svr list");

      it_map_sgservice = map_tmp_locate_del.begin();
      while (it_map_sgservice != map_tmp_locate_del.end()) {
        MNS_LOG_DEBUG(
            "del svr info: "
                << MnsSdkCommon::SGService2String(it_map_sgservice->second));

        vec_sgservice_del.push_back(it_map_sgservice->second);
        g_map_ipport_sgservice_.erase((it_map_sgservice++)->first);
      }
    }
  }

  if (MNS_UNLIKELY(vec_sgservice_add.size() || vec_sgservice_del.size()
                       || vec_sgservice_chg.size())) {
    for (vector<UpdateSvrListCallback>::const_iterator
             itr = service_info.update_cb_list_.begin();
         service_info.update_cb_list_.end() != itr; ++itr) {
      (*itr)(vec_sgservice_add,
             vec_sgservice_del,
             vec_sgservice_chg,
             appkey);
    }

    for (vector<SvrListCallback>::const_iterator
             itr = service_info.cb_list_.begin();
         service_info.cb_list_.end() != itr; ++itr) {
      (*itr)(vec_sgservice,
             appkey);
    }
  }
}

void mns_sdk::GetSvrList(void) {
  typedef boost::unordered_map<string, ServiceInfo>::iterator MAP_IT;
  meituan_mns::ProtocolResponse rsp;
  rsp.__set_errcode(-1);
  for (MAP_IT iter = (ThreadLocalSingletonMap::instance()).begin();
       iter != (ThreadLocalSingletonMap::instance()).end(); ++iter) {
    MNS_LOG_INFO("appkey: " << iter->first << " Do getsvrlist");
    try {
      (ThreadLocalSingletonSGAgentClientSharedPtr::instance())
          ->GetServiceList(rsp, iter->second.protoreq_);
    } catch (apache::thrift::TException &tx) {
      MNS_LOG_ERROR("appkey: " << iter->first << " service getsvrlist error "
                               << tx.what());
      return;
    }

    if (0 != rsp.errcode) {
      MNS_LOG_ERROR(
          "appkey: " << iter->first << " meituan_mns::ProtocolResponse errcode "
                     << rsp.errcode);
      return;
    }

    //filter service name
    const string &tmp_service = iter->second.protoreq_.serviceName;
    if (!tmp_service.empty()) {
      MNS_LOG_INFO(
          "Get service list with serviceName filter: " << tmp_service);
      rsp.servicelist.erase(remove_if(rsp.servicelist.begin(),
                                      rsp.servicelist.end(),
                                      boost::bind(mns_sdk::FilterService,
                                                  _1,
                                                  tmp_service)),
                            rsp.servicelist.end());
    }
    (iter->second.sgservice_).assign(rsp.servicelist.begin(),
                                     rsp.servicelist.end());

    MNS_LOG_DEBUG("appkey: " << iter->first << " recv vec_sgservice.size "
                             << (iter->second.sgservice_).size());
    for (int i = 0; i < static_cast<int>((iter->second.sgservice_).size());
         i++) {
      MNS_LOG_DEBUG(
          "[" << i << "]" << ": "
              << MnsSdkCommon::SGService2String((iter->second.sgservice_)[i]));
    }

    UptSvrList(iter->first, iter->second);
  }
}

void mns_sdk::GetSvrList(const string &str_svr_appkey,
                         const string &str_cli_appkey,
                         const string &str_proto_type,
                         const string &str_service_name,
                         vector<meituan_mns::SGService> *p_res_svr_list,
                         muduo::CountDownLatch *p_countdown_get_svr_list) {
  meituan_mns::ProtocolRequest protoreq;
  protoreq.__set_localAppkey(str_cli_appkey);
  protoreq.__set_remoteAppkey(str_svr_appkey);
  protoreq.__set_protocol(str_proto_type);
  protoreq.__set_serviceName(str_service_name);

  meituan_mns::ProtocolResponse rsp;
  rsp.__set_errcode(-1);
  try {
    (ThreadLocalSingletonSGAgentClientSharedPtr::instance())
        ->GetServiceList(rsp, protoreq);
  } catch (apache::thrift::TException &tx) {
    MNS_LOG_ERROR("appkey: " << str_svr_appkey << " service getsvrlist error "
                             << tx.what());
    p_countdown_get_svr_list->countDown();
    return;
  }

  if (0 != rsp.errcode) {
    MNS_LOG_ERROR("appkey: " << str_svr_appkey
                             << " meituan_mns::ProtocolResponse errcode "
                             << rsp.errcode);
    p_countdown_get_svr_list->countDown();
    return;
  }

  //filter service name
  if (!str_service_name.empty()) {
    MNS_LOG_INFO(
        "Get service list with serviceName filter: " << str_service_name);
    rsp.servicelist.erase(remove_if(rsp.servicelist.begin(),
                                    rsp.servicelist.end(),
                                    boost::bind(mns_sdk::FilterService,
                                                _1,
                                                str_service_name)),
                          rsp.servicelist.end());
  }
  p_res_svr_list->assign(rsp.servicelist.begin(), rsp.servicelist.end());
  MNS_LOG_DEBUG("appkey: " << str_svr_appkey << " recv vec_sgservice.size "
                           << p_res_svr_list->size());
  p_countdown_get_svr_list->countDown();
  //no need to update status.
}

void mns_sdk::InitMap() {
  MNS_LOG_DEBUG("begin to init global map.");
  ThreadLocalSingletonMap::instance() =
      boost::unordered_map<string, ServiceInfo>();
}

int mns_sdk::InitMNS(const std::string &mns_path, const double &sec, const
double &timeout) {

  if (g_config.LoadConfig(mns_path) != 0) {
    MNS_LOG_DEBUG("load config file failed." << mns_path);
    return -1;
  }

  muduo::MutexLockGuard lock(init_mutex);

  if (NULL == mns_eventloop_p_) {
    mns_worker_thread_sp_ = boost::make_shared<muduo::net::EventLoopThread>(
        muduo::net::EventLoopThread::ThreadInitCallback(),
        "mns_sdk");

    mns_eventloop_p_ = mns_worker_thread_sp_->startLoop();

    mns_eventloop_p_->runInLoop(mns_sdk::InitCthrift);

    mns_eventloop_p_->runInLoop(mns_sdk::InitMap);

    mns_eventloop_p_->runEvery(sec,
                               boost::bind(&GetSvrList));

  }
  ref_count_++;

  return 0;
}

void mns_sdk::InitCthrift(void) {
  MNS_LOG_DEBUG("Init Cthrift & sg_agent client");

  if (!ThreadLocalSingletonSGAgentClientSharedPtr::instance()) {
    ThreadLocalSingletonSGAgentClientSharedPtr::instance() =
        boost::make_shared<MnsWorker>();

    ThreadLocalSingletonSGAgentClientSharedPtr::instance()->Init();
    g_local_ip_port = InitLocalAddr();
  }
}

void mns_sdk::InitWorkerWithSvrListCB(const string &str_svr_appkey,
                                      const string &str_cli_appkey,
                                      const string &str_proto_type,
                                      const string &str_service_name,
                                      const SvrListCallback &cb) {
  ServiceInfo &service_info =
      (ThreadLocalSingletonMap::instance())[str_svr_appkey];
  service_info.SetProtocolReq(str_cli_appkey, str_svr_appkey,
                              str_proto_type, str_service_name);
  service_info.AddSvrListCallback(cb);

  GetSvrList();
}

void mns_sdk::InitWorkerWithUptSvrListCB(const string &str_svr_appkey,
                                         const string &str_cli_appkey,
                                         const string &str_proto_type,
                                         const string &str_service_name,
                                         const UpdateSvrListCallback &cb) {
  ServiceInfo &service_info =
      (ThreadLocalSingletonMap::instance())[str_svr_appkey];
  service_info.SetProtocolReq(str_cli_appkey, str_svr_appkey,
                              str_proto_type, str_service_name);
  service_info.AddUpdateSvrListCallback(cb);

  GetSvrList();
}

int8_t mns_sdk::getSvrList(const string &str_svr_appkey,
                           const string &str_cli_appkey,
                           const string &str_proto_type,
                           const string &str_service_name,
                           vector<meituan_mns::SGService> *p_svr_list) {
  if (str_svr_appkey.empty() || NULL == p_svr_list) {
    MNS_LOG_ERROR(
        "Input server appkey can not empty, and result point of "
            "meituan_mns::SGService list should not NULL");
    return -1;
  }

  if (MNS_LIKELY(mns_eventloop_p_)) {

    muduo::CountDownLatch countdown_get_svr_list(1);
    mns_eventloop_p_->runInLoop(boost::bind(&GetSvrList,
                                            str_svr_appkey,
                                            str_cli_appkey,
                                            str_proto_type,
                                            str_service_name,
                                            p_svr_list,
                                            &countdown_get_svr_list));

    countdown_get_svr_list.wait();
  } else {
    MNS_LOG_ERROR("Please init mns_sdk first");
    return -1;
  }
  return 0;
}

void mns_sdk::DoAddSvrListCallback(const string &str_svr_appkey,
                                   const SvrListCallback &cb,
                                   string *p_err_info,
                                   muduo::CountDownLatch *countdown_set_callback) {
  boost::unordered_map<string, ServiceInfo>
      &map_serviceInfo = ThreadLocalSingletonMap::instance();
  if (map_serviceInfo.end() == map_serviceInfo.find(str_svr_appkey)) {
    p_err_info->assign("Has not set protocol type for " + str_svr_appkey
                           + ". Please use StartClient set first.");
    MNS_LOG_DEBUG(*p_err_info);
    countdown_set_callback->countDown();
    return;
  }
  map_serviceInfo[str_svr_appkey].AddSvrListCallback(cb);
  countdown_set_callback->countDown();
}

int8_t mns_sdk::AddSvrListCallback(const string &str_svr_appkey,
                                   const SvrListCallback &cb,
                                   string *p_err_info) {
  bool appkey_empty = str_svr_appkey.empty();
  bool cb_empty = cb.empty();
  bool p_err_info_null = (NULL == p_err_info);
  if (MNS_UNLIKELY(appkey_empty || cb_empty || p_err_info_null)) {
    string str_err_info("input appkey empty " + (appkey_empty ? string("true;")
                                                              : string("false;"))
                            +
                                " input callback func empty "
                            + (cb_empty ? string("true;") : string("false;")) +
        " input err info pointer null "
                            + (p_err_info_null ? string("true.") : string(
                                "false.")));
    MNS_LOG_ERROR(str_err_info);
    if (p_err_info) {
      p_err_info->assign(str_err_info);
    }
    return -1;
  }
  if (MNS_LIKELY(mns_eventloop_p_)) {
    muduo::CountDownLatch countdown_set_callback(1);
    mns_eventloop_p_->runInLoop(boost::bind(&mns_sdk::DoAddSvrListCallback,
                                            str_svr_appkey,
                                            cb,
                                            p_err_info,
                                            &countdown_set_callback));
    countdown_set_callback.wait();
  } else {
    MNS_LOG_ERROR("Please init mns_sdk first");
    return -1;
  }
  return p_err_info->empty() ? 0 : -1;
}

void mns_sdk::DoAddUpdateSvrListCallback(const string &str_svr_appkey,
                                         const UpdateSvrListCallback &cb,
                                         string *p_err_info,
                                         muduo::CountDownLatch *countdown_set_callback) {
  boost::unordered_map<string, ServiceInfo>
      &map_serviceInfo = ThreadLocalSingletonMap::instance();
  if (map_serviceInfo.end() == map_serviceInfo.find(str_svr_appkey)) {
    p_err_info->assign("Has not set protocol type for " + str_svr_appkey
                           + ". Please use StartClient set first.");
    MNS_LOG_DEBUG(*p_err_info);
    countdown_set_callback->countDown();
    return;
  }
  map_serviceInfo[str_svr_appkey].AddUpdateSvrListCallback(cb);
  countdown_set_callback->countDown();
}

int8_t mns_sdk::AddUpdateSvrListCallback(const string &str_svr_appkey,
                                         const UpdateSvrListCallback &cb,
                                         string *p_err_info) {
  bool appkey_empty = str_svr_appkey.empty();
  bool cb_empty = cb.empty();
  bool p_err_info_null = (NULL == p_err_info);
  if (MNS_UNLIKELY(appkey_empty || cb_empty || p_err_info_null)) {
    string str_err_info("input appkey empty " + (appkey_empty ? string("true;")
                                                              : string("false;"))
                            +
                                " input callback func empty "
                            + (cb_empty ? string("true;") : string("false;")) +
        " input err info pointer null "
                            + (p_err_info_null ? string("true.") : string(
                                "false.")));
    MNS_LOG_ERROR(str_err_info);
    if (p_err_info) {
      p_err_info->assign(str_err_info);
    }
    return -1;
  }
  if (MNS_LIKELY(mns_eventloop_p_)) {
    muduo::CountDownLatch countdown_set_callback(1);
    mns_eventloop_p_->runInLoop(boost::bind(&mns_sdk::DoAddUpdateSvrListCallback,
                                            str_svr_appkey,
                                            cb,
                                            p_err_info,
                                            &countdown_set_callback));
    countdown_set_callback.wait();
  } else {
    MNS_LOG_ERROR("Please init mns_sdk first");
  }
  return p_err_info->empty() ? 0 : -1;
}

int8_t mns_sdk::StartClient(const string &str_svr_appkey,
                            const string &str_cli_appkey,
                            const string &str_proto_type,
                            const string &str_service_name,
                            const SvrListCallback &cb) {
  if (str_svr_appkey.empty() || str_cli_appkey.empty()) {
    MNS_LOG_ERROR("Input appkeys should not empty");
    return -1;
  }
  MNS_LOG_DEBUG("str_svr_appkey " << str_svr_appkey << " str_client_appkey "
                                  << str_cli_appkey << " str_proto_type "
                                  << str_proto_type << " "
                                      "str_service_name " << str_service_name);

  if (MNS_LIKELY(mns_eventloop_p_)) {

    mns_eventloop_p_->runInLoop(boost::bind(&mns_sdk::InitWorkerWithSvrListCB,
                                            str_svr_appkey,
                                            str_cli_appkey,
                                            str_proto_type,
                                            str_service_name,
                                            cb));
  } else {
    MNS_LOG_ERROR("Please init mns_sdk first");
    return -1;
  }

  return 0;
}

int8_t mns_sdk::StartClient(const string &str_svr_appkey,
                            const string &str_cli_appkey,
                            const string &str_proto_type,
                            const string &str_service_name,
                            const UpdateSvrListCallback &cb) {
  if (str_svr_appkey.empty() || str_cli_appkey.empty()) {
    MNS_LOG_ERROR("Input appkeys should not empty");
    return -1;
  }
  MNS_LOG_DEBUG("str_svr_appkey " << str_svr_appkey << " str_client_appkey "
                                  << str_cli_appkey << " str_proto_type "
                                  << str_proto_type << " "
                                      "str_service_name " << str_service_name);

  if (MNS_LIKELY(mns_eventloop_p_)) {
    mns_eventloop_p_->runInLoop(boost::bind(&mns_sdk::InitWorkerWithUptSvrListCB,
                                            str_svr_appkey,
                                            str_cli_appkey,
                                            str_proto_type,
                                            str_service_name,
                                            cb));
  } else {
    MNS_LOG_ERROR("Please  mns_sdk first");
  }

  return 0;
}

int8_t mns_sdk::StartSvr(const string &str_appkey,
                         const vector<string> &service_list,
                         const int16_t &i16_port,
                         const int32_t &i32_svr_type,
                         const string &str_proto_type,
                         const bool &b_is_uniform) {
  meituan_mns::SGService sgService;
  sgService.appkey.assign(str_appkey);
  sgService.version.assign("mns_sdk-1.3.0");
  sgService.ip.assign(g_config.str_local_ip_);
  sgService.port = i16_port;
  sgService.weight = 10;
  //状态从2-->0，对齐scanner状态图
  sgService.status = 2;
  sgService.lastUpdateTime = static_cast<int32_t>(time(0));
  sgService.fweight = 10.0;
  sgService.serverType = i32_svr_type;
  if (!service_list.empty()) {
    meituan_mns::ServiceDetail detail;
    detail.__set_unifiedProto(b_is_uniform);
    map<string, meituan_mns::ServiceDetail> serviceInfo;

    for (vector<string>::const_iterator iter = service_list.begin();
         iter != service_list.end(); ++iter) {
      serviceInfo.insert(make_pair(*iter, detail));
    }
    sgService.__set_serviceInfo(serviceInfo);
  }

  string tmp;

  switch (i32_svr_type) {
    case 0:sgService.protocol.assign("thrift");
      break;
    case 1:sgService.protocol.assign("http");
      break;
    case 2:tmp.assign(str_proto_type);
      transform(tmp.begin(),
                tmp.end(),
                tmp.begin(),
                ::tolower);
      if ("thrift" == tmp || "http" == tmp) {
        MNS_LOG_ERROR("sgService.serverType 2 but"
                          " str_proto_type " << str_proto_type);
        return -1;
      }

      sgService.protocol.assign(str_proto_type);
      break;
    default: MNS_LOG_ERROR(
        "unkown sgService.serverType " << sgService.serverType);
      return -1;
  }

  MNS_LOG_DEBUG(
      "create sgservice " << MnsSdkCommon::SGService2String(sgService));

  return StartSvr(sgService);
}

int8_t mns_sdk::StartSvr(const string &str_appkey,
                         const int16_t &i16_port,
                         const int32_t &i32_svr_type,
                         const string &str_proto_type) {
  vector<string> service_list;
  return StartSvr(str_appkey,
                  service_list,
                  i16_port,
                  i32_svr_type,
                  str_proto_type);
}

int8_t mns_sdk::StartSvr(const meituan_mns::SGService &sgService) {
  InitCthrift();

  try {
    int32_t i32_ret =
        (ThreadLocalSingletonSGAgentClientSharedPtr::instance())->RegistService(
            sgService);
    if (MNS_UNLIKELY(i32_ret)) {
      MNS_LOG_ERROR("registService failed: " << i32_ret);
      return -1;
    } else {
      MNS_LOG_INFO("reg svr done");
    }
  } catch (apache::thrift::TException &tx) {
    MNS_LOG_ERROR("registService failed: " << tx.what());
    return -1;
  }

  return 0;
}

void mns_sdk::DestroyMNS(void) {
  //仿造智能指针，使用引用计数管理资源释放。
  muduo::MutexLockGuard lock(init_mutex);
  if (0 == (--ref_count_) && mns_eventloop_p_) {
    mns_eventloop_p_->quit();
    usleep(500 * 1000);
    mns_eventloop_p_ = NULL;
  }
}

bool mns_sdk::FilterService(const meituan_mns::SGService &sg,
                            const string &service_name) {
  return sg.serviceInfo.find(service_name) == sg.serviceInfo.end();
}
