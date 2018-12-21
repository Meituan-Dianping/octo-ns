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

#include "falcon_mgr.h"
#include "log4cplus.h"

namespace meituan_mns {
muduo::detail::AtomicIntegerT<long> FalconMgr::registeQueueSize;
muduo::detail::AtomicIntegerT<long> FalconMgr::fileConfigQueueSize;
muduo::detail::AtomicIntegerT<long> FalconMgr::kvConfigQueueSize;
muduo::detail::AtomicIntegerT<long> FalconMgr::serviceListQueueSize;
muduo::detail::AtomicIntegerT<long> FalconMgr::routeListQueueSize;
muduo::detail::AtomicIntegerT<long> FalconMgr::serviceNameQueueSize;
muduo::detail::AtomicIntegerT<long> FalconMgr::commonLogQueueSize;
muduo::detail::AtomicIntegerT<long> FalconMgr::moduleInvokerQueueSize;

muduo::detail::AtomicIntegerT<int> FalconMgr::routeBufferSize;;
muduo::detail::AtomicIntegerT<int> FalconMgr::serviceBufferSize;;
muduo::detail::AtomicIntegerT<int> FalconMgr::filtedServiceBufferSize;;
muduo::detail::AtomicIntegerT<int> FalconMgr::serviceNameBufferSize;;

muduo::detail::AtomicIntegerT<int> FalconMgr::serviceListReqCount;
muduo::detail::AtomicIntegerT<int> FalconMgr::serviceListReqTime;
muduo::detail::AtomicIntegerT<int> FalconMgr::serviceListReqFailedCount;
muduo::detail::AtomicIntegerT<int> FalconMgr::serviceListReqCountStat;
muduo::detail::AtomicIntegerT<int> FalconMgr::serviceListReqTimeStat;
muduo::detail::AtomicIntegerT<int> FalconMgr::serviceListReqFailedCountStat;
double FalconMgr::serviceListReqSuccessPercent;
double FalconMgr::serviceListReqCost;

void FalconMgr::Init() {
  SetRegisteQueueSize(0);
  SetFileConfigQueueSize(0);
  SetKvConfigQueueSize(0);
  SetServiceListQueueSize(0);
  SetRouteListQueueSize(0);
  SetServiceNameQueueSize(0);

  SetRouteBufferSize(0);
  SetServiceBufferSize(0);
  SetFiltedServiceBufferSize(0);
  SetServiceNameBufferSize(0);
}

long FalconMgr::GetRegisteQueueSize() {
  return registeQueueSize.get();
}

void FalconMgr::SetRegisteQueueSize(const long size) {
  registeQueueSize.getAndSet(size);
}

long FalconMgr::GetFileConfigQueueSize() {
  return fileConfigQueueSize.get();
}

void FalconMgr::SetFileConfigQueueSize(const long size) {
  fileConfigQueueSize.getAndSet(size);
}

long FalconMgr::GetKvConfigQueueSize() {
  return kvConfigQueueSize.get();
}

void FalconMgr::SetKvConfigQueueSize(const long size) {
  kvConfigQueueSize.getAndSet(size);
}

long FalconMgr::GetServiceListQueueSize() {
  return serviceListQueueSize.get();
}

void FalconMgr::SetServiceListQueueSize(const long size) {
  serviceListQueueSize.getAndSet(size);
}

long FalconMgr::GetRouteListQueueSize() {
  return routeListQueueSize.get();
}

void FalconMgr::SetRouteListQueueSize(const long size) {
  routeListQueueSize.getAndSet(size);
}

long FalconMgr::GetServiceNameQueueSize() {
  return serviceNameQueueSize.get();
}

void FalconMgr::SetServiceNameQueueSize(const long size) {
  serviceNameQueueSize.getAndSet(size);
}

long FalconMgr::GetCommonLogQueueSize() {
  return commonLogQueueSize.get();
}

void FalconMgr::SetCommonLogQueueSize(const long size) {
  commonLogQueueSize.getAndSet(size);
}

long FalconMgr::GetModuleInvokerQueueSize() {
  return moduleInvokerQueueSize.get();
}

void FalconMgr::SetModuleInvokerQueueSize(const long size) {
  moduleInvokerQueueSize.getAndSet(size);
}

void FalconMgr::GetQueueSizeRes(std::map<int, long> *mq) {
  mq->insert(std::pair<int, long>
                 (FileConfigQueue, GetFileConfigQueueSize()));
  mq->insert(std::pair<int, long>
                 (KVConfigQueue, GetKvConfigQueueSize()));
  mq->insert(std::pair<int, long>
                 (ServiceListQueue, GetServiceListQueueSize()));
  mq->insert(std::pair<int, long>
                 (RouteListQueue, GetRouteListQueueSize()));
  mq->insert(std::pair<int, long>
                 (ServiceNameQueue, GetServiceNameQueueSize()));
  mq->insert(std::pair<int, long>
                 (CommonLogQueue, GetCommonLogQueueSize()));
  mq->insert(std::pair<int, long>
                 (ModuleInvokerQueue, GetModuleInvokerQueueSize()));
  mq->insert(std::pair<int, long>
                 (RegisteQueueSize, GetRegisteQueueSize()));
}

int FalconMgr::GetRouteBufferSize() {
  return routeBufferSize.get();
}

void FalconMgr::SetRouteBufferSize(const int size) {
  routeBufferSize.getAndSet(size);
}

int FalconMgr::GetServiceBufferSize() {
  return serviceBufferSize.get();
}

void FalconMgr::SetServiceBufferSize(const int size) {
  serviceBufferSize.getAndSet(size);
}

int FalconMgr::GetFiltedServiceBufferSize() {
  return filtedServiceBufferSize.get();
}

void FalconMgr::SetFiltedServiceBufferSize(const int size) {
  filtedServiceBufferSize.getAndSet(size);
}

int FalconMgr::GetServiceNameBufferSize() {
  return serviceNameBufferSize.get();
}

void FalconMgr::SetServiceNameBufferSize(const int size) {
  serviceNameBufferSize.getAndSet(size);
}

void FalconMgr::GetBufferSizeRes(std::map<int, int> *mp) {
  mp->insert(std::pair<int, int>
                 (RouteBuffer, GetRouteBufferSize()));
  mp->insert(std::pair<int, int>
                 (ServiceBuffer, GetServiceBufferSize()));
  mp->insert(std::pair<int, int>
                 (FiltedServiceBuffer, GetFiltedServiceBufferSize()));
  mp->insert(std::pair<int, int>
                 (ServiceNameBuffer, GetServiceNameBufferSize()));
}

int FalconMgr::GetServiceListReqCount() {
  return serviceListReqCount.get();
}

void FalconMgr::AddServiceListReqCount(const int size) {
  serviceListReqCount.getAndAdd(size);
}

int FalconMgr::GetServiceListReqTime() {
  return serviceListReqTime.get();
}

void FalconMgr::AddServiceListReqTime(const int size) {
  serviceListReqTime.getAndAdd(size);
}

int FalconMgr::GetServiceListReqFailedCount() {
  return serviceListReqFailedCount.get();
}

void FalconMgr::AddServiceListReqFailedCount(const int size) {
  serviceListReqFailedCount.getAndAdd(size);
}

int FalconMgr::GetServiceListReqCountStat() {
  return serviceListReqCountStat.get();
}

void FalconMgr::SetServiceListReqCountStat(const int size) {
  serviceListReqCountStat.getAndSet(size);
}

int FalconMgr::GetServiceListReqTimeStat() {
  return serviceListReqTimeStat.get();
}

void FalconMgr::SetServiceListReqTimeStat(const int size) {
  serviceListReqTimeStat.getAndSet(size);
}

int FalconMgr::GetServiceListReqFailedCountStat() {
  return serviceListReqFailedCountStat.get();
}

void FalconMgr::SetServiceListReqFailedCountStat(const int size) {
  serviceListReqFailedCountStat.getAndSet(size);
}

double FalconMgr::GetServiceListReqSuccessPercent() {
  return serviceListReqSuccessPercent;
}

void FalconMgr::SetServiceListReqSuccessPercent(const double size) {
  serviceListReqSuccessPercent = size;
}

double FalconMgr::GetServiceListReqCost() {
  return serviceListReqCost;
}

void FalconMgr::SetServiceListReqCost(const double size) {
  serviceListReqCost = size;
}

void FalconMgr::SetServiceListReqRes() {
  SetServiceListReqCountStat(GetServiceListReqCount());
  SetServiceListReqTimeStat(GetServiceListReqTime());
  SetServiceListReqFailedCountStat(GetServiceListReqFailedCount());
  SetServiceListReqSuccessPercent((GetServiceListReqCountStat()
      - GetServiceListReqFailedCountStat()) / GetServiceListReqCountStat());
  SetServiceListReqCost(GetServiceListReqTimeStat()
                            / GetServiceListReqCountStat());
  serviceListReqCount.getAndSet(0);
  serviceListReqTime.getAndSet(0);
  serviceListReqFailedCount.getAndSet(0);
  NS_LOG_INFO("count: " << serviceListReqCountStat.get()
                        << "; time: " << serviceListReqTimeStat.get()
                        << "; failed count: "
                        << serviceListReqFailedCountStat.get()
                        << "; success percent: " << serviceListReqSuccessPercent
                        << "; cost: " << serviceListReqCost);
}

void FalconMgr::GetServiceListReqRes(std::map<int, double> *mp) {
  mp->insert(std::pair<int, double>
                 (Count, (double) GetServiceListReqCountStat()));
  mp->insert(std::pair<int, double>
                 (Time, (double) GetServiceListReqTimeStat()));
  mp->insert(std::pair<int, double>
                 (FailedCount, (double) GetServiceListReqFailedCountStat()));
  mp->insert(std::pair<int, double>
                 (SuccessPercent, (double) GetServiceListReqSuccessPercent()));
  mp->insert(std::pair<int, double>
                 (Cost, (double) GetServiceListReqCost()));
}

}
