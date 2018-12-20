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

#include "route_info.h"
#include "comm/tinyxml2.h"
#include "log4cplus.h"
#include "comm/inc_comm.h"
#include <boost/make_shared.hpp>
namespace meituan_mns {


bool IdcUtil::IsSameIdc(const boost::shared_ptr<IDC> &idc1, const boost::shared_ptr<IDC> &idc2) {
  return idc1->GetIdc() == idc2->GetIdc();
}

bool IdcUtil::IsSameCenter(const boost::shared_ptr<IDC> &idc1, const boost::shared_ptr<IDC> &idc2) {
  // exclude NOCENTER
  // NOCENTER不算做一个中心来处理
  return ("NOCENTER" != idc1->GetCenter()) && (idc1->GetCenter() == idc2->GetCenter());
}
bool IdcUtil::IsInIdcs(const std::string &ip){

  NS_LOG_INFO("the local ip  = "<< ip);

  boost::shared_ptr<IDC> local_idc = IdcUtil::GetIdc(ip);
  if (NULL == local_idc.get()) {
    NS_LOG_ERROR("fail to the idc info of local ip = " << ip);
    return false;
  }
  if (local_idc->GetIdc().empty()) {
    return false;
  }
  return true;
}
boost::shared_ptr<IDC> IdcUtil::GetIdc(const std::string &ip) {

  boost::shared_ptr<std::vector<boost::shared_ptr<IDC> > > idcs_tmp;
  idcs_tmp = CXmlFile::GetIdc();
  for (std::vector<boost::shared_ptr<IDC> >::const_iterator iter = idcs_tmp->begin();
      idcs_tmp->end() != iter; ++iter) {
    if ((*iter)->IsSameIdc(ip)) {
      return *iter;
    }
  }
  return boost::make_shared<IDC>();
}

std::string IdcUtil::GetSameIdcZk(const std::string &zk_host,
                                  const std::string &ip) {
  std::vector<std::string> vec_str;
  int32_t ret = SplitStringIntoVector(zk_host.c_str(), "," , vec_str);
  if (0 >= ret) {
    NS_LOG_ERROR("failed to split zk_host to vector, zk_host: " << zk_host);
    return zk_host;
  }

  boost::shared_ptr<IDC> ip_idc = GetIdc(ip);

  std::vector<std::string> vec_res;
  if (NULL != ip_idc.get()) {
    for (std::vector<std::string>::const_iterator iter = vec_str.begin();
         iter != vec_str.end(); ++iter) {

      boost::shared_ptr<IDC> item_idc = GetIdc(*iter);
      if (IsSameIdc(item_idc, ip_idc)) {
        vec_res.push_back(*iter);
      }
    }
  } else {
    NS_LOG_ERROR("fail to get the idc info of ip = " << ip);
  }

  std::string res;
  if (vec_res.empty()) {
    res = zk_host;
  } else {
    res = vec_res[0];
    for (std::vector<std::string>::const_iterator iter = vec_res.begin() + 1;
         iter != vec_res.end(); ++iter) {
      res += "," + *iter;
    }
  }
  return res;
}

}
