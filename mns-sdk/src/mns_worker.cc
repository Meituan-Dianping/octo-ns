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

#include "mns_worker.h"

using namespace std;
using namespace muduo::net;
using namespace mns_sdk;

void ConnInfo::UptSgservice(const meituan_mns::SGService &sgservice) {
  double d_old_weight = MnsSdkCommon::FetchOctoWeight(sgservice_.fweight,
                                                      static_cast<double>(sgservice_.weight));
  double d_new_weight = MnsSdkCommon::FetchOctoWeight(sgservice.fweight,
                                                      static_cast<double>(sgservice.weight));

  MNS_LOG_DEBUG("d_old_weight " << d_old_weight << " d_new_weight "
                                << d_new_weight);

  if (!MnsSdkCommon::CheckDoubleEqual(d_old_weight, d_new_weight)) {
    MNS_LOG_DEBUG("need update weight buf");

    //real conn NOT erase, just del index
    p_map_weight_tcpclientwp_->erase(it_map_weight_tcpclientwp_index_);

    it_map_weight_tcpclientwp_index_ =
        p_map_weight_tcpclientwp_->insert(std::make_pair(
            d_new_weight,
            sp_tcpclient_));
  }

  sgservice_ = sgservice;
}

void ConnInfo::setSp_tcpclient_(const TcpClientSharedPtr &sp_tcpclient) {
  if (MNS_UNLIKELY(sp_tcpclient_.get())) {
    MNS_LOG_ERROR("client ip: " << (sp_tcpclient_->connection()
        ->peerAddress()).toIp() << " port: "
                                << (sp_tcpclient_->connection()->peerAddress()).toPort()
                                << " replace");

    p_map_weight_tcpclientwp_->erase(it_map_weight_tcpclientwp_index_);
  }

  sp_tcpclient_ = sp_tcpclient;

  double d_weight = MnsSdkCommon::FetchOctoWeight(sgservice_.fweight,
                                                  static_cast<double>(sgservice_.weight));
  MNS_LOG_DEBUG("dweight " << d_weight);

  it_map_weight_tcpclientwp_index_ =
      p_map_weight_tcpclientwp_->insert(std::make_pair(d_weight,
                                                       sp_tcpclient_));
}

MnsWorker::MnsWorker()
    : cond_avaliable_conn_ready_(mutexlock_avaliable_conn_ready_),
      i8_destructor_flag_(0),
      unzip_buf_(0),
      sentinel_url_addr_(g_config.url_port_) {  //atomic_avaliable_conn_num_
  // defalut init
  // by value
  //start real worker thread
  sp_event_thread_ =
      boost::make_shared<muduo::net::EventLoopThread>(EventLoopThread::ThreadInitCallback(),
                                                      "cthrift_cli_IO");
  p_event_loop_ = sp_event_thread_->startLoop();
  p_event_loop_->runInLoop(boost::bind(&MnsWorker::InitWorker,
                                       this)); //will use event_loop in
}

void
MnsWorker::OnConn4Sentinel(const muduo::net::TcpConnectionPtr &conn) {
  MNS_LOG_INFO(conn->localAddress().toIpPort() << " -> "
                                               << conn->peerAddress().toIpPort()
                                               << " is "
                                               << (conn->connected() ? "UP"
                                                                     : "DOWN"));

  if (conn->connected()) {
    boost::shared_ptr<HttpContext> tmp = boost::make_shared<HttpContext>();
    conn->setContext(tmp);
    conn->setTcpNoDelay(true);

    //maybe send a few duplicate request, but will stop when sentinel addrs filled, acceptable.
    if (MNS_UNLIKELY(1 >= map_ipport_spconninfo_.size())) {
      MNS_LOG_DEBUG("sgagent still NOT fill sentinel address");

      muduo::net::Buffer buf;
      buf.append(g_config.str_sentinel_http_request_);

      MNS_LOG_DEBUG("Send appkey tags buf " << buf.toStringPiece().data());

      conn->send(&buf);
    }
  } else {
    //http服务端关闭了链接后，这里的tcp链接应该close tcpclient，避免fd close_wait
    if (sp_tcpclient_sentinel_) {
      sp_tcpclient_sentinel_->disconnect();
      sp_tcpclient_sentinel_.reset();
    }
  }
}

void
MnsWorker::OnMsg4Sentinel(const muduo::net::TcpConnectionPtr &conn,
                          muduo::net::Buffer *buf,
                          muduo::Timestamp receiveTime) {
  MNS_LOG_DEBUG("OnMsg buf " << (buf->toStringPiece()).data());

  if (MNS_UNLIKELY((conn->getContext()).empty())) {
    MNS_LOG_ERROR("address: " << (conn->peerAddress()).toIpPort() << " "
        "context empty");    //NOT clear here
    return;
  }

  HttpContextSharedPtr pConnInfo;
  try {
    pConnInfo = boost::any_cast<HttpContextSharedPtr>(conn->getContext());
  } catch (boost::bad_any_cast e) {
    MNS_LOG_ERROR("bad_any_cast:" << e.what());
    return;
  }

  if (buf->readableBytes() < pConnInfo->u32_want_len) {
    MNS_LOG_DEBUG("NOT enough");
    return;
  }

  if (!MnsSdkCommon::ParseHttpRequest(&(pConnInfo->u32_want_len),
                                      buf,
                                      &(pConnInfo->http_context),
                                      muduo::Timestamp::now())) {
    MNS_LOG_ERROR("parseRequest failed");
    return;
  }

  if ((pConnInfo->http_context).gotAll()) {
    const muduo::net::HttpRequest
        &httpReq = (pConnInfo->http_context).request();
    const string &str_ori_body = httpReq.body();

    if (0 == str_ori_body.size()) {
      MNS_LOG_ERROR("No body from sentinel");
      return;
    }

    //decompress
    string str_body;
    string strCotentCode = httpReq.getHeader("Content-Encoding");
    if (string::npos != strCotentCode.find("gzip")) {
      MNS_LOG_DEBUG("strCotentCode " << strCotentCode);

      uLong ulBodyLen = 200000;    //fix
      int iRet
          =
          MnsSdkCommon::Httpgzdecompress(reinterpret_cast<unsigned char *>
                                         (const_cast<char *>(str_ori_body.c_str())),
                                         static_cast<uLong>(str_ori_body.size()),
                                         reinterpret_cast<unsigned char *>(unzip_buf_),
                                         &ulBodyLen);
      if (0 > iRet) {
        MNS_LOG_ERROR("Httpgzdecompress failed, iRet " << iRet);
        return;
      }

      MNS_LOG_DEBUG("ulBodyLen " << ulBodyLen);
      str_body.assign(unzip_buf_, ulBodyLen);
    } else {
      str_body.assign(str_ori_body);
    }

    MNS_LOG_DEBUG("str_body " << str_body);
    (pConnInfo->http_context).reset();

    vector<meituan_mns::SGService> vec_sgservice;
    if (MnsSdkCommon::ParseSentineSgagentList(str_body, &vec_sgservice)) {
      return;
    }

    if (MNS_LIKELY(!CheckLocalSgagentHealth())) {
      UpdateSvrList(vec_sgservice);
    }
  }
}

int8_t MnsWorker::CheckRegion(const double &d_weight) {
  if (d_weight < MnsSdkCommon::kDSecondRegionMin) {
    return 3;
  } else if (d_weight < MnsSdkCommon::kDFirstRegionMin) {
    return 2;
  }

  return 1;
}

bool ConnInfo::CheckConnHealthy(void) const {
  if (MNS_UNLIKELY(!(sp_tcpclient_.get()))) {
    MNS_LOG_ERROR("sp_tcpconn invalid appkey: " << sgservice_.appkey
                                                << " ip:" << sgservice_.ip
                                                << " port: "
                                                << sgservice_.port);
    return false;
  }

  muduo::net::TcpConnectionPtr sp_tcpconn = sp_tcpclient_->connection();
  if (MNS_UNLIKELY(!sp_tcpconn || !(sp_tcpconn.get()))) {
    MNS_LOG_ERROR("sp_tcpconn invalid appkey: " << sgservice_.appkey
                                                << " ip:" << sgservice_.ip
                                                << " port: "
                                                << sgservice_.port);
    return false;
  }

  if (MNS_UNLIKELY(!(sp_tcpconn->connected()))) {
    MNS_LOG_DEBUG("address: " << (sp_tcpconn->peerAddress()).toIpPort()
                              << "NOT connected");
    return false;
  }

  if (MNS_UNLIKELY((sp_tcpconn->getContext()).empty())) {
    MNS_LOG_ERROR("address: " << (sp_tcpconn->peerAddress()).toIpPort() << " "
        "context empty");    //NOT clear here
    return false;
  }

  Context4WorkerSharedPtr sp_context;
  try {
    sp_context = boost::any_cast<Context4WorkerSharedPtr>
        (sp_tcpconn->getContext());
  } catch (boost::bad_any_cast e) {
    MNS_LOG_ERROR("bad_any_cast:" << e.what() << " peer address "
                                  << (sp_tcpconn->peerAddress()).toIpPort());
    return false;
  }

  if (MNS_UNLIKELY(sp_context->b_highwater || sp_context->b_occupied)) {
    MNS_LOG_WARN("address: " << (sp_tcpconn->peerAddress()).toIpPort() <<
                             " b_highwater " << sp_context->b_highwater
                             << " b_occupied "
                             << sp_context->b_occupied << " ignore");
    return false;
  }

  return true;
}

int8_t MnsWorker::ChooseNextReadyConn(TcpClientWeakPtr *p_wp_tcpcli) {
  if (MNS_UNLIKELY(0 == p_multimap_weight_wptcpcli_->size())) {
    MNS_LOG_ERROR("multimap_weight_wptcpcli_ empty");
    return -1;
  }

  string str_default_sgagent_port;

  try {
    str_default_sgagent_port =
        boost::lexical_cast<std::string>(g_config.local_chrion_port_);
  } catch (boost::bad_lexical_cast &e) {

    MNS_LOG_ERROR("boost::bad_lexical_cast :" << e.what()
                                              << "SgagentPort "
                                              << g_config.local_chrion_port_);
  }

  UnorderedMapIpPort2ConnInfoSP iter_ipport_spconninfo;

  iter_ipport_spconninfo = map_ipport_spconninfo_.find
      (g_config.str_local_ip_ + ":" +   //NOT be "127.0.0.1" in normal
          // case, but be when cannot fetch local ip
          str_default_sgagent_port);

  if (map_ipport_spconninfo_.end() != iter_ipport_spconninfo
      && iter_ipport_spconninfo->second->CheckConnHealthy()) {
    MNS_LOG_DEBUG("sgagent rpc, local agent work, use it");

    TcpClientWeakPtr wp_tcpcli
        (iter_ipport_spconninfo->second->getSp_tcpclient_());
    *p_wp_tcpcli = wp_tcpcli;

    return 0;
  }

  MNS_LOG_INFO("sgagent rpc, local agent NOT work, use sentinel");

  boost::unordered_map<double, vector<TcpClientWeakPtr> > map_weight_vec;
  vector<double> vec_weight;

  double d_last_weight = -1.0;
  double d_total_weight = 0.0;
  int8_t i8_stop_region = 2;  //init not necessary, but for safe
  string str_port;

  muduo::net::TcpConnectionPtr sp_tcpconn;

  MultiMapIter iter = p_multimap_weight_wptcpcli_->begin();
  while (p_multimap_weight_wptcpcli_->end() != iter) {
    TcpClientSharedPtr sp_tcpcli((iter->second).lock());
    if (MNS_UNLIKELY(!sp_tcpcli || !(sp_tcpcli.get()))) {
      MNS_LOG_ERROR("tcpclient NOT avaliable");

      p_multimap_weight_wptcpcli_->erase(iter++);
      continue;
    }

    sp_tcpconn = sp_tcpcli->connection();
    if (MNS_UNLIKELY(!sp_tcpconn)) {
      MNS_LOG_INFO("NOT connected yet");
      ++iter;
      continue;
    }

    MNS_LOG_DEBUG("Address: " << (sp_tcpconn->peerAddress()).toIpPort() << " "
        "weight " << iter->first);

    try {
      str_port =
          boost::lexical_cast<std::string>((sp_tcpconn->peerAddress()).toPort());
    } catch (boost::bad_lexical_cast &e) {

      MNS_LOG_ERROR("boost::bad_lexical_cast :" << e.what()
                                                << "tcp connnect peer port : "
                                                << (sp_tcpconn->peerAddress()).toPort());

      ++iter;
      continue;
    }

    iter_ipport_spconninfo = map_ipport_spconninfo_.find(
        (sp_tcpconn->peerAddress()).toIp() + ":" + str_port);
    if (MNS_UNLIKELY(
        iter_ipport_spconninfo == map_ipport_spconninfo_.end())) {
      MNS_LOG_ERROR("Not find ip:"
                        << (sp_tcpconn->peerAddress()).toIp() << " port:"
                        << str_port << " in map_ipport_spconninfo_");

      p_multimap_weight_wptcpcli_->erase(iter++);
      continue;
    }

    if (!(iter_ipport_spconninfo->second->CheckConnHealthy())) {
      ++iter;
      continue;
    }

    //weight random choose algorithm
    //1. sum all weight(one weight = single weight * same weight conn num)
    //2. random total_weight,get a random_weight
    //3. choose random_weight region from all weight.
    if (!MnsSdkCommon::CheckDoubleEqual(d_last_weight,
                                        iter->first)) { //new weight
      if (MNS_LIKELY(
          !MnsSdkCommon::CheckDoubleEqual(d_last_weight, -1.0))) { //NOT init
        if (i8_stop_region <= CheckRegion(iter->first)) {  //if already get
          // conn and next region reach, stop
          MNS_LOG_DEBUG("stop region " << i8_stop_region << " "
              "iter->first " << iter->first);
          break;
        }

        d_total_weight += d_last_weight * static_cast<double>
        (map_weight_vec[d_last_weight]
                .size
                    ());
      } else {
        i8_stop_region =
            static_cast<int8_t>(CheckRegion(iter->first)
                + 1); //set stop region by the first weight

        MNS_LOG_DEBUG("i8_stop_region set to be " << i8_stop_region);
      }

      vec_weight.push_back(iter->first);
      d_last_weight = iter->first;
    }

    map_weight_vec[iter->first].push_back(iter->second);
    ++iter;
  }

  if (MNS_UNLIKELY(0 == vec_weight.size())) {
    MNS_LOG_INFO("Not avaliable conn can be choosed, maybe all occupied");
    return 1;
  }
  //将所有候选节点的权重进行求和：d_total_weight
  //将权重看作一条线段，d_total_weight是线段长度
  //不同权重的节点占据这条线段的不同部分
  //产生0~d_total_weight一个随机数，落入的权重线段某个区域; 选择该区域归属的节点列表；从这个节点列表中随机选择一个节点。
  d_total_weight +=
      d_last_weight * static_cast<double>(map_weight_vec[d_last_weight].size
          ());

  MNS_LOG_DEBUG("d_total_weight " << d_total_weight);
  //伪随机数在小范围下不均匀（0~1)，放大1000倍解决该问题
  d_total_weight *= static_cast<double>(1000.0);

  double d_choose = 0.0;
  double d_tmp = 0.0;
  //产生的随机数落入权重线段某个区域，后面的while其实是在找这个“区域”
  double d_random_weight = fmod(static_cast<double>(rand()), d_total_weight);
  vector<double>::iterator it_vec = vec_weight.begin();
  while (vec_weight.end() != it_vec) {
    //伪随机数在小范围下不均匀（0~1)，放大1000倍进行平滑处理
    d_tmp += (*it_vec) * static_cast<double>(map_weight_vec[*it_vec].size())
        * static_cast<double>(1000.0);
    if (d_tmp > d_random_weight) {
      d_choose = *it_vec;
      break;
    }

    ++it_vec;
  }

  boost::unordered_map<double, vector<TcpClientWeakPtr> >::iterator
      it_map = map_weight_vec.find(d_choose);
  if (MNS_UNLIKELY(it_map == map_weight_vec.end())) {
    MNS_LOG_ERROR("not find weight " << d_choose);
    return -1;
  }

  if (1 == (it_map->second).size()) {
    *p_wp_tcpcli = *((it_map->second).begin());
  } else {
    MNS_LOG_DEBUG((it_map->second).size() << " conn need be choose one "
        "equally");

    *p_wp_tcpcli = (it_map->second)[rand() % ((it_map->second).size())];
  }

  return 0;
}

void
MnsWorker::UpdateSvrList(const vector<meituan_mns::SGService> &vec_sgservice) {
  vector<meituan_mns::SGService>::const_iterator it_vec;
  boost::unordered_map<string, meituan_mns::SGService>::iterator
      it_map_sgservice;

  string str_port;
  string str_sgagent_default_port;

  try {
    str_sgagent_default_port =
        boost::lexical_cast<std::string>(g_config.local_chrion_port_);
  } catch (boost::bad_lexical_cast &e) {

    MNS_LOG_ERROR("boost::bad_lexical_cast :" << e.what()
                                              << "SgagentPort "
                                              << g_config.local_chrion_port_);

    return;
  }

  vector<meituan_mns::SGService> vec_sgservice_add;
  vector<meituan_mns::SGService> vec_sgservice_del;
  vector<meituan_mns::SGService> vec_sgservice_chg;

  if (MNS_UNLIKELY(
      0 == map_ipport_sgservice_.size() && 0 == vec_sgservice.size())) {
    MNS_LOG_WARN("Init svr list but empty srvlist");
  } else if (MNS_UNLIKELY(0 == map_ipport_sgservice_.size())) {
    it_vec = vec_sgservice.begin();
    MNS_LOG_INFO("Init svr list for appkey " << it_vec->appkey);

    while (it_vec != vec_sgservice.end()) {
      if (MNS_UNLIKELY(2 != it_vec->status)) {
        MNS_LOG_DEBUG("svr info: "
                          << MnsSdkCommon::SGService2String(*it_vec)
                          << " IGNORED");
        ++it_vec;
        continue;
      }

      try {
        str_port = boost::lexical_cast<std::string>(it_vec->port);
      } catch (boost::bad_lexical_cast &e) {

        MNS_LOG_ERROR("boost::bad_lexical_cast :" << e.what()
                                                  << "it_vec->port "
                                                  << it_vec->port);

        ++it_vec;
        continue;
      }

      map_ipport_sgservice_.insert(make_pair(it_vec->ip + ":" + str_port,
                                             *it_vec));

      vec_sgservice_add.push_back(*(it_vec++));
    }
  } else if (MNS_UNLIKELY(0 == vec_sgservice.size())) {
    MNS_LOG_WARN("vec_sgservice empty");

    it_map_sgservice = map_ipport_sgservice_.begin();
    while (it_map_sgservice
        != map_ipport_sgservice_.end()) {    //exclude local sgagent since sentinel list NOT include it
      if (g_config.str_local_ip_ + ":" + g_config.str_local_chrion_port_
          != it_map_sgservice->first) {
        vec_sgservice_del.push_back(it_map_sgservice->second);
        map_ipport_sgservice_.erase(it_map_sgservice++);
      } else {
        it_map_sgservice++;
      }
    }
  } else {
    boost::unordered_map<string, meituan_mns::SGService>
        map_tmp_locate_del(map_ipport_sgservice_);
    map_tmp_locate_del.erase(g_config.str_local_ip_ + ":"
                                 + g_config.str_local_chrion_port_); //exclude local sgagent

    it_vec = vec_sgservice.begin();
    while (it_vec != vec_sgservice.end()) {
      if (MNS_UNLIKELY(2 != it_vec->status)) {
        MNS_LOG_DEBUG("svr info: "
                          << MnsSdkCommon::SGService2String(*it_vec)
                          << " IGNORED");
        ++it_vec;
        continue;
      }

      try {
        str_port = boost::lexical_cast<std::string>(it_vec->port);
      } catch (boost::bad_lexical_cast &e) {

        MNS_LOG_DEBUG("boost::bad_lexical_cast :" << e.what()
                                                  << "it_vec->port "
                                                  << it_vec->port);

        ++it_vec;
        continue;
      }

      string str_key(it_vec->ip + ":" + str_port);
      it_map_sgservice = map_ipport_sgservice_.find(str_key);
      if (map_ipport_sgservice_.end() == it_map_sgservice) {
        MNS_LOG_DEBUG("ADD svr list info: "
                          << MnsSdkCommon::SGService2String(*it_vec));

        vec_sgservice_add.push_back(*it_vec);
        map_ipport_sgservice_.insert(make_pair(str_key, *it_vec));
      } else {
        map_tmp_locate_del.erase(str_key);

        if (it_map_sgservice->second != *it_vec) {
          MNS_LOG_DEBUG("UPDATE svr list. old info: "
                            << MnsSdkCommon::SGService2String(it_map_sgservice->second));
          MNS_LOG_DEBUG(" new info: "
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
        MNS_LOG_DEBUG("del svr info: "
                          << MnsSdkCommon::SGService2String(it_map_sgservice->second));

        vec_sgservice_del.push_back(it_map_sgservice->second);
        map_ipport_sgservice_.erase((it_map_sgservice++)->first);
      }
    }
  }

  AddSrv(vec_sgservice_add);
  DelSrv(vec_sgservice_del);
  ChgSrv(vec_sgservice_chg);
}

void MnsWorker::InitSentinel(void) {
  //conn to sentinel
  sp_tcpclient_sentinel_ = boost::make_shared<muduo::net::TcpClient>(
      p_event_loop_,
      sentinel_url_addr_,
      "sentinel tcpclient");

  sp_tcpclient_sentinel_->setConnectionCallback(boost::bind(&MnsWorker::OnConn4Sentinel,
                                                            this,
                                                            _1));
  sp_tcpclient_sentinel_->setMessageCallback(boost::bind(&MnsWorker::OnMsg4Sentinel,
                                                         this,
                                                         _1,
                                                         _2,
                                                         _3));
  sp_tcpclient_sentinel_->enableRetry();
  sp_tcpclient_sentinel_->connect();
}

void MnsWorker::UnInitSentinel(void) {

  if (MNS_LIKELY(CheckLocalSgagentHealth())) {
    MNS_LOG_DEBUG("sgagent rpc, local agent work, use it, remove sentinel");

    if (sp_tcpclient_sentinel_) {
      sp_tcpclient_sentinel_->disconnect();
      sp_tcpclient_sentinel_.reset();
    }

    vector<meituan_mns::SGService> vec_sgservice;
    UpdateSvrList(vec_sgservice);
    return;
  }
}

bool MnsWorker::CheckLocalSgagentHealth(void) {
  string str_default_sgagent_port;

  try {
    str_default_sgagent_port =
        boost::lexical_cast<std::string>(g_config.local_chrion_port_);
  } catch (boost::bad_lexical_cast &e) {

    MNS_LOG_ERROR("boost::bad_lexical_cast :" << e.what()
                                              << "SgagentPort "
                                              << g_config.local_chrion_port_);
  }

  UnorderedMapIpPort2ConnInfoSP iter_ipport_spconninfo;

  iter_ipport_spconninfo = map_ipport_spconninfo_.find
      (g_config.str_local_ip_ + ":" + g_config.str_local_chrion_port_);

  if (map_ipport_spconninfo_.end() != iter_ipport_spconninfo
      && iter_ipport_spconninfo->second->CheckConnHealthy()) {
    MNS_LOG_DEBUG("sgagent rpc, local agent work, use it");

    return true;
  }

  return false;
}

void MnsWorker::CheckLocalSgagent(void) {

  if (MNS_LIKELY(CheckLocalSgagentHealth())) {
    MNS_LOG_DEBUG("sgagent rpc, local agent work, return");

    if (map_ipport_spconninfo_.size() > 1) {
      p_event_loop_->runAfter(kDRetryIntervalSec,
                              boost::bind(&MnsWorker::UnInitSentinel, this));
    }

    return;
  }

  // to init InitSentinel
  MNS_LOG_WARN("sgagent rpc, local agent NOT work, init sentinel");

  if (map_ipport_spconninfo_.size() <= 1 && 0 == i8_destructor_flag_) {
    InitSentinel();
  }
}

void MnsWorker::InitWorker(void) {
  p_multimap_weight_wptcpcli_ =
      new multimap<double, TcpClientWeakPtr, WeightSort>;//exit del, safe

  unzip_buf_ = new char[200000]; //200k for sentinel content

  meituan_mns::SGService sgservice;
  MnsSdkCommon::PackDefaultSgservice(g_config.chrion_appkey_,
                                     g_config.str_local_ip_,
                                     static_cast<uint16_t >(g_config
                                         .local_chrion_port_),
                                     &sgservice);
  vector<meituan_mns::SGService> vec_sgservice;
  vec_sgservice.push_back(sgservice);

  UpdateSvrList(vec_sgservice);

  if (g_config.b_use_remote_) {
    if (!muduo::net::InetAddress::resolve(g_config.url_,
                                          &sentinel_url_addr_)) {   //ONLY fill
      // IP address
      MNS_LOG_WARN("resolve " << g_config.url_ << " failed");

    } else {
      //启动时，本地sg可用50ms后开始初始化哨兵；避免初始化哨兵占用work线程，造成第一次拉取服务列表超时.
      MNS_LOG_DEBUG("CheckLocalSgagent after 0.05s");
      p_event_loop_->runAfter(0.05,
                              boost::bind(&MnsWorker::CheckLocalSgagent, this));

    }
  }
}

void MnsWorker::AddSrv(const vector<meituan_mns::SGService> &vec_add_sgservice) {
  string str_port;
  vector<meituan_mns::SGService> vec_chg_sgservice;
  MultiMapIter it_multimap;

  vector<meituan_mns::SGService>::const_iterator
      it_sgservice = vec_add_sgservice
      .begin();
  while (it_sgservice != vec_add_sgservice.end()) {
    const meituan_mns::SGService &sgservice = *it_sgservice;

    try {
      str_port = boost::lexical_cast<std::string>(sgservice.port);
    } catch (boost::bad_lexical_cast &e) {

      MNS_LOG_ERROR("boost::bad_lexical_cast :" << e.what()
                                                << "sgservice.port "
                                                << sgservice.port);

      ++it_sgservice;
      continue;
    }

    ConnInfoSharedPtr
        &sp_conninfo = map_ipport_spconninfo_[sgservice.ip + ":" + str_port];

    if (MNS_UNLIKELY(sp_conninfo.get())) {
      MNS_LOG_WARN("svr " << MnsSdkCommon::SGService2String(sgservice)
                          << " already exist in map_ipport_sptcpcli, just change it");

      vec_chg_sgservice.push_back(sgservice);
      ++it_sgservice;

      continue;
    }

    sp_conninfo = boost::make_shared<ConnInfo>(sgservice,
                                               p_multimap_weight_wptcpcli_);

    boost::shared_ptr<muduo::net::TcpClient>
        sp_tcp_cli_tmp = boost::make_shared<muduo::net::TcpClient>(
        p_event_loop_,
        muduo::net::InetAddress(
            sgservice.ip,
            static_cast<uint16_t>(sgservice.port)),
        "client worker for appkey "
            + sgservice.appkey);

    sp_conninfo->setSp_tcpclient_(sp_tcp_cli_tmp);//will set weight buf inside

    TcpClientSharedPtr &sp_tcpcli = sp_conninfo->getSp_tcpclient_();

    sp_tcpcli->setConnectionCallback(boost::bind(&MnsWorker::OnConn,
                                                 this,
                                                 _1));

    sp_tcpcli->setMessageCallback(boost::bind(&MnsWorker::OnMsg,
                                              this,
                                              _1,
                                              _2,
                                              _3));

    sp_tcpcli->enableRetry();
    sp_tcpcli->connect();

    ++it_sgservice;
  }

  if (vec_chg_sgservice.size()) {
    MNS_LOG_ERROR("Add trans to Chg");
    ChgSrv(vec_chg_sgservice);
  }
}

void MnsWorker::DelSrv(const vector<meituan_mns::SGService> &vec_del_sgservice) {
  string str_port;

  vector<meituan_mns::SGService>::const_iterator
      it_sgservice = vec_del_sgservice.begin();
  while (it_sgservice != vec_del_sgservice.end()) {
    const meituan_mns::SGService &sgservice = *it_sgservice;

    try {
      str_port = boost::lexical_cast<std::string>(sgservice.port);
    } catch (boost::bad_lexical_cast &e) {

      MNS_LOG_ERROR("boost::bad_lexical_cast :" << e.what()
                                                << "sgservice.port "
                                                << sgservice.port);

      ++it_sgservice;
      continue;
    }

    //TODO grace exit??
    //tcpclient exit will close connection, conninfo exit will clear weight buf
    map_ipport_spconninfo_.erase(sgservice.ip + ":" + str_port);

    ++it_sgservice;
  }
}

void MnsWorker::ChgSrv(const vector<meituan_mns::SGService> &vec_chg_sgservice) {
  string str_port;
  string str_key;

  vector<meituan_mns::SGService> vec_add_sgservice;

  vector<meituan_mns::SGService>::const_iterator
      it_sgservice = vec_chg_sgservice.begin();
  while (it_sgservice != vec_chg_sgservice.end()) {
    const meituan_mns::SGService &sgservice = *it_sgservice;

    try {
      str_port = boost::lexical_cast<std::string>(sgservice.port);
    } catch (boost::bad_lexical_cast &e) {

      MNS_LOG_ERROR("boost::bad_lexical_cast :" << e.what()
                                                << "sgservice.port "
                                                << sgservice.port);

      ++it_sgservice;
      continue;
    }

    str_key.assign(sgservice.ip + ":" + str_port);
    UnorderedMapStr2SpConnInfoIter
        it_map = map_ipport_spconninfo_.find(str_key);
    if (it_map == map_ipport_spconninfo_.end()) {
      MNS_LOG_WARN("Not find " << str_key << " for appkey "
                               << sgservice.appkey
                               << " in map_ipport_spconninfo_, readd it");

      vec_add_sgservice.push_back(sgservice);
    } else {
      it_map->second->UptSgservice(sgservice);
    }

    ++it_sgservice;
  }

  if (vec_add_sgservice.size()) {
    MNS_LOG_ERROR("Chg trans to Add");
    AddSrv(vec_add_sgservice);
  }
}

void MnsWorker::OnConn(const muduo::net::TcpConnectionPtr &conn) {
  MNS_LOG_INFO(conn->localAddress().toIpPort() << " -> "
                                               << conn->peerAddress().toIpPort()
                                               << " is "
                                               << (conn->connected() ? "UP"
                                                                     : "DOWN"));

  if (conn->connected()) {

    string str_port;

    try {
      str_port =
          boost::lexical_cast<std::string>((conn->peerAddress()).toPort());
    } catch (boost::bad_lexical_cast &e) {

      MNS_LOG_ERROR("boost::bad_lexical_cast :" << e.what()
                                                << "toPort "
                                                << (conn->peerAddress()).toPort()
                                                << "conn peerAddr "
                                                << (conn->peerAddress()).toIpPort());

      conn->shutdown();
      return;
    }

    //check in map
    UnorderedMapIpPort2ConnInfoSP unordered_map_iter =
        map_ipport_spconninfo_.find(
            (conn->peerAddress()).toIp() + ":" + str_port);
    if (MNS_UNLIKELY(
        unordered_map_iter == map_ipport_spconninfo_.end())) {
      MNS_LOG_ERROR("conn peerAddr " << (conn->peerAddress()).toIpPort()
                                     << " localaddr "
                                     << (conn->localAddress()).toIpPort()
                                     << " NOT find key in map_ipport_spconninfo_");

      conn->shutdown();
      return;
    }

    conn->setTcpNoDelay(true);

    boost::shared_ptr<ConnContext4Worker> conn_context_ptr =
        boost::make_shared<ConnContext4Worker>(unordered_map_iter->second);
    conn->setContext(conn_context_ptr);

    Context4WorkerSharedPtr tmp;
    try {
      tmp = boost::any_cast<Context4WorkerSharedPtr>(conn->getContext());
    } catch (boost::bad_any_cast e) {
      MNS_LOG_ERROR("bad_any_cast:" << e.what());
      return;
    }

    //tmp->t_last_conn_time_ = time(0);

    if (MNS_UNLIKELY(1 == atomic_avaliable_conn_num_.incrementAndGet())) {
      muduo::MutexLockGuard lock(mutexlock_avaliable_conn_ready_);
      cond_avaliable_conn_ready_.notifyAll();
    }
  } else {
    if (MNS_UNLIKELY((conn->getContext()).empty())) {
      MNS_LOG_WARN("conn context empty, maybe shutdown when conn");
    } else {
      Context4WorkerSharedPtr sp_context;
      try {
        sp_context =
            boost::any_cast<Context4WorkerSharedPtr>(conn->getContext());
      } catch (boost::bad_any_cast e) {
        MNS_LOG_ERROR("bad_any_cast:" << e.what());
        return;
      }

      if (!(sp_context->b_highwater) && !(sp_context->b_occupied)
          && (0 >= (atomic_avaliable_conn_num_.decrementAndGet()))) {
        atomic_avaliable_conn_num_.getAndSet(0); //adjust for safe

        MNS_LOG_WARN("atomic_avaliable_conn_num_ 0");
      }
    }
  }

  if (g_config.b_use_remote_) {
    CheckLocalSgagent();
  }
}

void MnsWorker::OnMsg(const muduo::net::TcpConnectionPtr &conn,
                      muduo::net::Buffer *buffer,
                      muduo::Timestamp receiveTime) {
  MNS_LOG_DEBUG((conn->peerAddress()).toIpPort() << " msg received "
                                                 << (buffer->toStringPiece()).data()
                                                 << " len "
                                                 << buffer->readableBytes());

  if (MNS_UNLIKELY((conn->getContext()).empty())) {
    MNS_LOG_ERROR("peer address " << conn->peerAddress().toIpPort()
                                  << " context empty");
    conn->shutdown();
    return;
  }

  Context4WorkerSharedPtr sp_context_worker;
  try {
    sp_context_worker =
        boost::any_cast<Context4WorkerSharedPtr>(conn->getContext());
  } catch (boost::bad_any_cast e) {
    MNS_LOG_ERROR("bad_any_cast:" << e.what());
    return;
  }
}

TcpClientSharedPtr MnsWorker::GetAvailableConn() {

  TcpClientSharedPtr empty;

  TcpClientWeakPtr wp_tcpcli;
  if (ChooseNextReadyConn(&wp_tcpcli)) {
    MNS_LOG_ERROR("No candidate connection to send packet");
    return empty;
  }

  TcpClientSharedPtr
      sp_tcpcli(wp_tcpcli.lock());  //already check valid in ChooseNextReadyConn
  string str_port;

  try {
    str_port =
        boost::lexical_cast<std::string>((sp_tcpcli->connection()->peerAddress()).toPort());
  } catch (boost::bad_lexical_cast &e) {
    MNS_LOG_ERROR("boost::bad_lexical_cast :" << e.what()
                                              << " ip:"
                                              << (sp_tcpcli->connection()->peerAddress()).toIp()
                                              << " port:"
                                              << (sp_tcpcli->connection()->peerAddress()).toPort());
    return empty;
  }

  UnorderedMapIpPort2ConnInfoSP
      iter_ipport_spconninfo = map_ipport_spconninfo_.find(
      (sp_tcpcli->connection()->peerAddress()).toIp() + ":" + str_port);
  if (MNS_UNLIKELY(
      iter_ipport_spconninfo == map_ipport_spconninfo_.end())) {
    MNS_LOG_ERROR("Not find ip:"
                      << (sp_tcpcli->connection()->peerAddress()).toIp()
                      << " port:"
                      << str_port << " in map_ipport_spconninfo_");

    return empty;
  }

  return sp_tcpcli;
}

int32_t MnsWorker::GetServiceList(meituan_mns::ProtocolResponse &rsp, const
meituan_mns::ProtocolRequest &request) {
  ServerListParamsPtr
      params = boost::make_shared<ServerListParams>(rsp, request);

  size_t sz_queue_size = p_event_loop_->queueSize();

  if (MNS_UNLIKELY(10 <= sz_queue_size)) {
    MNS_LOG_WARN("worker queue size " << sz_queue_size);
  } else {
    MNS_LOG_DEBUG("worker queue size " << sz_queue_size);
  }

  p_event_loop_->runInLoop(boost::bind
                               (&MnsWorker::OnGetServiceList, this, params));
  //NOT sp_shared_worker_transport_ itself

  bool b_timeout;
  double d_wait_secs = 0.0;
  const double d_timeout_secs =
      static_cast<double>(kI32DefaultTimeoutForReuestMS) / 1000;
  muduo::Timestamp timestamp_start = muduo::Timestamp::now();

  do {
    muduo::MutexLockGuard lock(params->mutexlock_conn_ready_);
    b_timeout = params->cond_ready_read_.waitForSeconds(d_timeout_secs);
  } while (0);

  if (MnsSdkCommon::CheckOverTime(timestamp_start, d_timeout_secs,
                                  &d_wait_secs) &&
      params->ret_ == -1) {

    params->timeout_ = true;
    MNS_LOG_ERROR("GetServiceList time out ");
    return -1;
  }

  rsp = params->rsp_;
  MNS_LOG_INFO("GetServiceList ret:" << params->ret_)
  return params->ret_;

}

int32_t MnsWorker::RegistService(const meituan_mns::SGService &sgService) {
  RegisterParamsPtr params = boost::make_shared<RegisterParams>(sgService);

  size_t sz_queue_size = p_event_loop_->queueSize();

  if (MNS_UNLIKELY(10 <= sz_queue_size)) {
    MNS_LOG_WARN("worker queue size " << sz_queue_size);
  } else {
    MNS_LOG_DEBUG("worker queue size " << sz_queue_size);
  }

  p_event_loop_->runInLoop(boost::bind
                               (&MnsWorker::OnRegistService, this, params));

  bool b_timeout;
  double d_wait_secs = 0.0;
  const double d_timeout_secs =
      static_cast<double>(kI32DefaultTimeoutForReuestMS) / 1000;
  muduo::Timestamp timestamp_start = muduo::Timestamp::now();

  do {
    muduo::MutexLockGuard lock(params->mutexlock_conn_ready_);
    b_timeout = params->cond_ready_read_.waitForSeconds(d_timeout_secs);
  } while (0);

  if (MnsSdkCommon::CheckOverTime(timestamp_start, d_timeout_secs,
                                  &d_wait_secs) &&
      params->ret_ == -1) {

    params->timeout_ = true;
    MNS_LOG_ERROR("RegistService time out ");
    return -1;
  }

  MNS_LOG_INFO("RegistService ret:" << params->ret_);
  return params->ret_;
}

void MnsWorker::OnGetServiceList(ServerListParamsPtr params) {
  MNS_LOG_DEBUG("getServiceListByProtocol begin");

  if (params->timeout_) {
    MNS_LOG_WARN("before get server list timeout just return");
    NotifyGetServiceList(params);
    return;
  }

  TcpClientSharedPtr conn = GetAvailableConn();
  if (!conn) {
    params->ret_ = -1;
    MNS_LOG_WARN("get server list No candidate connection to send packet "
                     "just return");
    NotifyGetServiceList(params);
    return;
  }

  std::string ip = (conn->connection()->peerAddress()).toIp();
  int16_t port = (conn->connection()->peerAddress()).toPort();

  ThriftClientHandler *p_handle = InitHandler(ip, port);
  if (!p_handle) {
    params->ret_ = -1;
    MNS_LOG_ERROR("OnGetServiceList failed, get handler fail.");
    NotifyGetServiceList(params);
    return;
  }

  meituan_mns::ServiceAgentClient *thrift_client =
      static_cast<meituan_mns::ServiceAgentClient *>(p_handle->getClient());

  if (thrift_client == NULL) {
    MNS_LOG_WARN("static cast error");
    params->ret_ = -1;
    NotifyGetServiceList(params);
    return;
  }

  try {
    MNS_LOG_DEBUG("getServiceListByProtocol rpc begin");
    thrift_client->getServiceListByProtocol(params->rsp_,
                                            params->resuest_);
    params->ret_ = 0;
    MNS_LOG_DEBUG("getServiceListByProtocol rpc end && error code="
                      << params->rsp_.errcode);
  } catch (apache::thrift::TException &tx) {
    MNS_LOG_ERROR("getServiceListByProtocol error " << tx.what());
    //异常关闭连接
    p_handle->closeConnection();
    //释放内存
    MNS_SAFE_DELETE(p_handle);
    params->ret_ = -1;
    NotifyGetServiceList(params);
    return;
  }
  //使用完成，关闭连接
  p_handle->closeConnection();
  //释放内存
  MNS_SAFE_DELETE(p_handle);
  NotifyGetServiceList(params);
  MNS_LOG_DEBUG("getServiceListByProtocol begin");
  return;
}

void MnsWorker::NotifyRegistService(RegisterParamsPtr params) {
  muduo::MutexLockGuard lock(params->mutexlock_conn_ready_);
  params->cond_ready_read_.notifyAll();
}

void MnsWorker::NotifyGetServiceList(ServerListParamsPtr params) {
  muduo::MutexLockGuard lock(params->mutexlock_conn_ready_);
  params->cond_ready_read_.notifyAll();
}

void MnsWorker::OnRegistService(RegisterParamsPtr params) {
  MNS_LOG_DEBUG("OnRegistService  begin");
  if (params->timeout_) {
    MNS_LOG_WARN("before get server list timeout just return");
    NotifyRegistService(params);
    return;
  }

  TcpClientSharedPtr conn = GetAvailableConn();

  if (!conn) {
    params->ret_ = -1;
    MNS_LOG_WARN("OnRegistService No candidate connection to send packet "
                     "just return");
    NotifyRegistService(params);
    return;
  }

  std::string ip = (conn->connection()->peerAddress()).toIp();
  int16_t port = (conn->connection()->peerAddress()).toPort();

  ThriftClientHandler *p_handle = InitHandler(ip, port);
  if (!p_handle) {
    MNS_LOG_ERROR("OnRegistService failed, get handler fail.");
    params->ret_ = -1;
    NotifyRegistService(params);
    return;
  }

  meituan_mns::ServiceAgentClient *thrift_client =
      static_cast<meituan_mns::ServiceAgentClient *>(p_handle->getClient());

  if (thrift_client == NULL) {
    MNS_LOG_WARN("static cast error");
    params->ret_ = -1;
    NotifyRegistService(params);
    return;
  }

  try {
    MNS_LOG_DEBUG("OnRegistService  rpc begin");
    int ret = thrift_client->registService(params->sgService_);
    params->ret_ = ret;
    MNS_LOG_DEBUG("OnRegistService  rpc end");
  } catch (apache::thrift::TException &tx) {
    MNS_LOG_ERROR("OnRegistService error " << tx.what());
    //异常关闭连接
    p_handle->closeConnection();
    //释放内存
    MNS_SAFE_DELETE(p_handle);
    params->ret_ = -1;
    NotifyRegistService(params);
    return;
  }
  //使用完成，关闭连接
  p_handle->closeConnection();
  //释放内存
  MNS_SAFE_DELETE(p_handle);

  MNS_LOG_DEBUG("OnRegistService  end");
  NotifyRegistService(params);
  return;
}

ThriftClientHandler *MnsWorker::InitHandler(const std::string &ip,
                                            const int port) {
  ThriftClientHandler *p_handle = new ThriftClientHandler();
  if (p_handle != NULL) {
    int ret =
        p_handle->init(ip, port, SG_AEGNT_TYPE, false);
    if ((ret == 0) && p_handle->m_transport->isOpen()) {
      return p_handle;
    } else {
      MNS_LOG_ERROR("GetHandler init failed! "
                        << ", ip = " << ip
                        << ", port = " << port);
      MNS_SAFE_DELETE(p_handle);
      return NULL;
    }
  } else {
    return NULL;
  }
}

void MnsWorker::Init() {
  muduo::Timestamp timestamp_start = muduo::Timestamp::now();

  muduo::Condition
      &cond = getCond_avaliable_conn_ready_();
  muduo::MutexLock &mutexlock =
      getMutexlock_avaliable_conn_ready_();

  const double
      d_default_timeout_secs = static_cast<double>(kI32DefaultTimeoutMS) /
      1000;
  double d_left_time_sec = 0.0;
  bool b_timeout = false;

  while (0 >= getAtomic_avaliable_conn_num_()) {//while, NOT if
    MNS_LOG_DEBUG("No good conn for  from worker, wait");

    if (!MnsSdkCommon::CheckOverTime(timestamp_start,
                                     d_default_timeout_secs,
                                     &d_left_time_sec)) {
      do {
        muduo::MutexLockGuard lock(mutexlock);
        b_timeout = cond.waitForSeconds(d_left_time_sec);
      } while (0);

      if (b_timeout) {
        if (MNS_UNLIKELY(0 < getAtomic_avaliable_conn_num_())) {
          MNS_LOG_DEBUG("miss notify, but already get");
        } else {
          MNS_LOG_WARN("wait " << d_left_time_sec
                               << " secs for good conn for timeout, maybe "
                                   "need more time");
        }
        return;
      }

      if (MNS_UNLIKELY(MnsSdkCommon::CheckOverTime(timestamp_start,
                                                   d_default_timeout_secs,
                                                   0))) {
        MNS_LOG_WARN(d_default_timeout_secs
                         << "secs countdown to 0, but no good conn ready, maybe need more time");
        return;
      }
    }
  }

  MNS_LOG_DEBUG(
      "wait done, avaliable conn num " << getAtomic_avaliable_conn_num_());
}
