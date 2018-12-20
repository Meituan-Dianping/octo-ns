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

#include <muduo/base/Atomic.h>
#include <map>
namespace meituan_mns {
enum EventLoopQueue {
  FileConfigQueue,
  KVConfigQueue,
  ServiceListQueue,
  RouteListQueue,
  ServiceNameQueue,
  CommonLogQueue,
  ModuleInvokerQueue,
  RegisteQueueSize,
};

enum BufferSize {
  RouteBuffer,
  ServiceBuffer,
  FiltedServiceBuffer,
  ServiceNameBuffer,
};

enum ServiceListReq {
  Count,
  Time,
  FailedCount,
  SuccessPercent,
  Cost,
};

class FalconMgr {
 public:
  static void Init();
  static long GetRegisteQueueSize();
  static void SetRegisteQueueSize(const long size);
  static long GetFileConfigQueueSize();
  static void SetFileConfigQueueSize(const long size);
  static long GetKvConfigQueueSize();
  static void SetKvConfigQueueSize(const long size);
  static long GetServiceListQueueSize();
  static void SetServiceListQueueSize(const long size);
  static long GetRouteListQueueSize();
  static void SetRouteListQueueSize(const long size);
  static long GetServiceNameQueueSize();
  static void SetServiceNameQueueSize(const long size);
  static long GetCommonLogQueueSize();
  static void SetCommonLogQueueSize(const long size);
  static long GetModuleInvokerQueueSize();
  static void SetModuleInvokerQueueSize(const long size);
  static void GetQueueSizeRes(std::map<int, long> *mq);

  static int GetRouteBufferSize();
  static void SetRouteBufferSize(const int size);
  static int GetServiceBufferSize();
  static void SetServiceBufferSize(const int size);
  static int GetFiltedServiceBufferSize();
  static void SetFiltedServiceBufferSize(const int size);
  static int GetServiceNameBufferSize();
  static void SetServiceNameBufferSize(const int size);
  static void GetBufferSizeRes(std::map<int, int> *mp);

  static int GetServiceListReqCount();
  static void AddServiceListReqCount(const int size = 1);
  static int GetServiceListReqTime();
  static void AddServiceListReqTime(const int size);
  static int GetServiceListReqFailedCount();
  static void AddServiceListReqFailedCount(const int size = 1);
  static int GetServiceListReqCountStat();
  static void SetServiceListReqCountStat(const int size);
  static int GetServiceListReqTimeStat();
  static void SetServiceListReqTimeStat(const int size);
  static int GetServiceListReqFailedCountStat();
  static void SetServiceListReqFailedCountStat(const int size);
  static double GetServiceListReqSuccessPercent();
  static void SetServiceListReqSuccessPercent(const double size);
  static double GetServiceListReqCost();
  static void SetServiceListReqCost(const double size);
  static void SetServiceListReqRes();
  static void GetServiceListReqRes(std::map<int, double> *mp);

 private:
  static muduo::detail::AtomicIntegerT<long> registeQueueSize;
  static muduo::detail::AtomicIntegerT<long> fileConfigQueueSize;
  static muduo::detail::AtomicIntegerT<long> kvConfigQueueSize;
  static muduo::detail::AtomicIntegerT<long> serviceListQueueSize;
  static muduo::detail::AtomicIntegerT<long> routeListQueueSize;
  static muduo::detail::AtomicIntegerT<long> serviceNameQueueSize;
  static muduo::detail::AtomicIntegerT<long> commonLogQueueSize;
  static muduo::detail::AtomicIntegerT<long> moduleInvokerQueueSize;

  // buffer key size
  static muduo::detail::AtomicIntegerT<int> routeBufferSize;
  static muduo::detail::AtomicIntegerT<int> serviceBufferSize;
  static muduo::detail::AtomicIntegerT<int> filtedServiceBufferSize;
  static muduo::detail::AtomicIntegerT<int> serviceNameBufferSize;

  // req count
  static muduo::detail::AtomicIntegerT<int> serviceListReqCount;
  static muduo::detail::AtomicIntegerT<int> serviceListReqTime;
  static muduo::detail::AtomicIntegerT<int> serviceListReqFailedCount;
  static muduo::detail::AtomicIntegerT<int> serviceListReqCountStat;
  static muduo::detail::AtomicIntegerT<int> serviceListReqTimeStat;
  static muduo::detail::AtomicIntegerT<int> serviceListReqFailedCountStat;
  static double serviceListReqSuccessPercent;
  static double serviceListReqCost;
};

}
