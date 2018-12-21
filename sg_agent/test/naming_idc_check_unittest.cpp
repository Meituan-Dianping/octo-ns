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
#include <gtest/gtest.h>
#include "route_info.h"
#include "route_base.h"
#include "route_rule.h"

using namespace meituan_mns;
class IdcCheck : public testing::Test {
 public:
  virtual void SetUp() {
  }
  virtual void TearDown() {
  }
 protected:
};
const char* GetIdcInfo(const std::string &ip) {
  boost::shared_ptr<IDC> idc = IdcUtil::GetIdc(ip);
  std::string res = "";
  if (NULL != idc.get()) {
    res = idc->GetIdc();
  }
  return res.c_str();
}