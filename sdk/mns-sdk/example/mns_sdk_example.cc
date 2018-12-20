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

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/unordered/unordered_map.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/random.hpp>

#include <mns_sdk/mns_sdk.h>

using namespace std;
using namespace mns_sdk;

void Job(const vector<meituan_mns::SGService> &vec_add,
         const vector<meituan_mns::SGService> &vec_del,
         const vector<meituan_mns::SGService> &vec_chg,
         const string &appkey) {

  cout << "=========GetServerList success=========" << endl;

  cout << "recv add.size " << vec_add.size() << endl;
  for (int i = 0; i < static_cast<int>(vec_add.size()); i++) {
    cout << "[" << i << "]" << ": "
         << MnsSdkCommon::SGService2String(vec_add[i]) << endl;
  }

  cout << "recv del.size " << vec_del.size() << endl;
  for (size_t i = 0; i < vec_del.size(); i++) {
    cout << "[" << i << "]" << ": "
         << MnsSdkCommon::SGService2String(vec_del[i]) << endl;
  }

  cout << "recv chg.size " << vec_chg.size() << endl;
  for (size_t i = 0; i < vec_chg.size(); i++) {
    cout << "[" << i << "]" << ": "
         << MnsSdkCommon::SGService2String(vec_chg[i]) << endl;
  }

  cout << "appkey " << appkey << endl;
}

void JobList(const vector<meituan_mns::SGService> &vec_add,
             const vector<meituan_mns::SGService> &vec_del,
             const vector<meituan_mns::SGService> &vec_chg,
             const string &appkey) {
  cout << "AddUpdateSvrListCallback appkey " << appkey << endl;
}

int main(void) {

  //初始化mns_sdk
  log4cplus::PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT("log4cplus.conf"));

  InitMNS("/opt/octo.cfg", 10);

  if (MNS_UNLIKELY(StartSvr("com.sankuai.inf.newct",
                            7776,
                            0))) {    //只是注册了一个ip:port,如果该网络地址未真实存在,将在OCTO管理界面上显示未启动,但注册本身没有问题.
    cerr << "=========registService failed=========" << endl;
  } else {
    cout << "=========registService success=========" << endl;
  }

  vector<string> service_list;
  service_list.push_back("name1");
  service_list.push_back("name2");
  if (MNS_UNLIKELY(StartSvr("com.sankuai.inf.newct",
                            service_list,
                            17776,
                            0,
                            "thrift"))) {    //只是注册了一个ip:port,如果该网络地址未真实存在,将在OCTO管理界面上显示未启动,但注册本身没有问题.
    cerr << "=========registService failed=========" << endl;
  } else {
    cout << "=========registService success=========" << endl;
  }

  boost::function<void(
      const vector<meituan_mns::SGService> &vec_add,
      const vector<meituan_mns::SGService> &vec_del,
      const vector<meituan_mns::SGService> &vec_chg,
      const string &appkey)> job(boost::bind(&Job, _1, _2, _3, _4));

  if (MNS_UNLIKELY(StartClient("com.sankuai.inf.newct",
                               "com.sankuai.inf.newct.client",
                               "thrift",
                               "",
                               job))) {
    cerr << "=========start client failed=========" << endl;
  } else {
    cout << "=========start client success=========" << endl;
  }

  boost::function<void(
      const vector<meituan_mns::SGService> &vec_add,
      const vector<meituan_mns::SGService> &vec_del,
      const vector<meituan_mns::SGService> &vec_chg,
      const string &appkey)> job1(boost::bind(&JobList, _1, _2, _3, _4));
  string err_info = "";
  if (MNS_UNLIKELY(AddUpdateSvrListCallback("com.sankuai.inf.newct",
                                            job1,
                                            &err_info))) {
    cout << "=========AddUpdateSvrListCallback client failed=========" << err_info << endl;
  } else {
    cout << "=========AddUpdateSvrListCallback client success=========" <<
                                                                         endl;
  }
  sleep(120);

  DestroyMNS();
}
