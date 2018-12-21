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

#ifndef SG_AGENT_MNS_COMM_H
#define SG_AGENT_MNS_COMM_H

namespace meituan_mns {
struct ServListAndCache {
  int origin_servlist_size;
  int filte_servlist_size;
  int origin_cache_size;
  int filte_cache_size;
  std::vector<SGService> origin_servicelist;
  std::vector<SGService> filte_servicelist;
  ServListAndCache() :
      origin_servlist_size(0),
      filte_servlist_size(0),
      origin_cache_size(0),
      filte_cache_size(0),
      origin_servicelist(std::vector<SGService>()),
      filte_servicelist(std::vector<SGService>()){}
};
struct SgCollectorMonitorInfo {
  int pid;
  int vmRss;
  int cpu;
  int zkConnections;
  int mtConfigConnections;
  int logCollectorConnections;
  int fileConfigQueueLen;
  int kvConfigQueueLen;
  int serviceListQueueLen;
  int routeListQueueLen;
  int serviceNameQueueLen;
  int commonLogQueueLen;
  int moduleInvokerQueueLen;
  int registeQueueSizeLen;
  std::string extend;
  SgCollectorMonitorInfo():
      pid(0),
      vmRss(0),
      cpu(0),
      zkConnections(0),
      mtConfigConnections(0),
      logCollectorConnections(0),
      fileConfigQueueLen(0),
      kvConfigQueueLen(0),
      serviceListQueueLen(0),
      routeListQueueLen(0),
      serviceNameQueueLen(0),
      commonLogQueueLen(0),
      moduleInvokerQueueLen(0),
      registeQueueSizeLen(0),
      extend("") {}
};
}

#endif //SG_AGENT_MNS_COMM_H
