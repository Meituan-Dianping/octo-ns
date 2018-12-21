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

#include "route_base.h"
#include "../comm/inc_comm.h"

namespace meituan_mns {

void IDC::CalculateIpMask() {
  int_mask_ = ConvertMaskToInt();
  ip_mask_value_ = int_mask_ & GetIp4Value(ip_);
}

bool IDC::IsSameIdc(const std::string &ip) {
  return ip_mask_value_ == (int_mask_ & GetIp4Value(ip));
}

int32_t IDC::ConvertMaskToInt() {
  std::vector<std::string> vnum;
  SplitStringIntoVector(mask_.c_str(), ".", vnum);
  if (4 != vnum.size()) {
    return FAILURE;
  }

  int32_t iMask = 0;
  for (int i = 0; i < 4; ++i) {
    iMask += (atoi(vnum[i].c_str()) << ((3 - i) * 8));
  }
  return iMask;
}

int32_t IDC::GetIp4Value(const std::string &ip) {
  std::vector<std::string> vcIp;
  SplitStringIntoVector(ip.c_str(), ".", vcIp);
  if (4 != vcIp.size()) {
    return FAILURE;
  }
  int address = 0;
  int filter_num = 0xFF;
  for (int i = 0; i < 4; i++) {
    int pos = i * 8;
    int vIp = atoi(vcIp[3 - i].c_str());
    address |= ((vIp << pos) & (filter_num << pos));
  }
  return address;
}

void IDC::SetRegion(const std::string &val) {
  region_ = val;
}
std::string IDC::GetRegion() const {
  return region_;
}

void IDC::SetIdc(const std::string &val) {
  idc_ = val;
}
std::string IDC::GetIdc() const {
  return idc_;
}

void IDC::SetCenter(const std::string &val) {
  center_ = val;
}
std::string IDC::GetCenter() const {
  return center_;
}

void IDC::SetIp(const std::string &val) {
  ip_ = val;
}
std::string IDC::GetIp() const {
  return ip_;
}

void IDC::SetMask(const std::string &val) {
  mask_ = val;
}
std::string IDC::GetMask() const {
  return mask_;
}

}
