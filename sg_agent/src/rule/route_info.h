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

#ifndef PROJECT_ROUTE_INFO_H
#define PROJECT_ROUTE_INFO_H
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <muduo/base/Mutex.h>
#include "route_base.h"
#include "../comm/inc_comm.h"
#include <boost/shared_ptr.hpp>
#include "../mns/config_loader.h"

namespace meituan_mns {

class IdcUtil {
 public:
  static boost::shared_ptr<IDC> GetIdc(const std::string &ip);
  /**
   *
   * @param idc1
   * @param idc2
   * @return
   */
  static bool IsSameIdc(const boost::shared_ptr<IDC> &idc1,
                        const boost::shared_ptr<IDC> &idc2);
  /**
   *
   * @param idc1
   * @param idc2
   * @return
   */
  static bool IsSameCenter(const boost::shared_ptr<IDC> &idc1,
                           const boost::shared_ptr<IDC> &idc2);

  static bool IsInIdcs(const std::string &ip);

  /**
   *
   * @param zk_host
   * @param ip
   * @return
   */
  static std::string GetSameIdcZk(const std::string &zk_host,
                                  const std::string &ip);


};

}



#endif //PROJECT_ROUTE_INFO_H
