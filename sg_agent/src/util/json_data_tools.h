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

#ifndef OCTO_OPEN_SOURCE_JSONDATAMGR_H
#define OCTO_OPEN_SOURCE_JSONDATAMGR_H

#include <iostream>
#include <map>
#include <set>
#include <list>
#include "naming_service_types.h"

extern "C" {
#include "comm/cJSON.h"
}
using namespace __gnu_cxx;
namespace meituan_mns {
class JsonZkMgr {
 public:

  static int ProviderNode2Json(const CProviderNode &oprovider,
                               std::string &strJson);
  static int ServiceNameNode2Json(const std::string &serviceName,
                                  std::string &strJson);
  static int Json2ProviderNode(
      const std::string &strJson,
      unsigned long mtime,
      unsigned long version,
      unsigned long cversion,
      CProviderNode &oprovider);

  static int SGService2Json(const SGService &oservice,
                            std::string &strJson,
                            const int env_int);
  static int Json2SGService(const std::string &strJson, SGService &oservice);
  static int ServiceNode2Json(const ServiceNode &oservice,
                              std::string &strJson);
  static int Json2ServiceNode(const std::string &strJson,
                              ServiceNode &oservice);
  static int Json2RouteData(const std::string &strJson, CRouteData &orouteData);
  static int Json2RouteNode(const std::string &strJson,
                            unsigned long mtime,
                            unsigned long version,
                            unsigned long cversion,
                            CRouteNode &oroute);

  static int cJson_AddStringArray(
      const std::set<std::string> &vecSvrname,
      cJSON *root, cJSON *item,
      const std::string &itemName);

  static int cJson_AddServiceObject(
      const std::map<std::string, ServiceDetail> &vecSvrname,
      cJSON *root, cJSON *item,
      const std::string &itemName);

  static cJSON *GetObjectItem(cJSON *item, const char *name);

};
}  //  namespace meituan_mns

#endif //OCTO_OPEN_SOURCE_JSONDATAMGR_H
