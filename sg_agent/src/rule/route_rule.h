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

#ifndef __sgagent_filter_H__
#define __sgagent_filter_H__

#include <string>
#include <vector>
#include <set>
#include "route_info.h"
#include "../gen-cpp/naming_service_types.h"
#include <pthread.h>

namespace meituan_mns {

// routeData排序比较函数，基于priority大小
static bool CompRouterData(const CRouteData &r1,
                           const CRouteData &r2) {
  return r1.priority > r2.priority;
}

class RouteRule {
 public:
  /**
   * 过滤掉权重为零的服务节点
   * @param serviceList
   * @param thresholdWeight
   * @return
   */
  static int32_t FilterWeight(std::vector<SGService> &serviceList,
                          double thresholdWeight = 0);

  /**
   * 根据服务分组优先级排序，并且过滤掉下线状态的服务分组
   * @param routeList
   * @return
   */
  static int32_t SortRouteList(std::vector<CRouteData> &routeList);

  /**
   * 过滤服务分组,根据服务分组信息过滤返回的服务列表
   * @param serviceList
   * @param sorted_routes
   * @param ip
   * @param route_flag
   * @param open_auto_route
   * @return
   */
  static int32_t FilterRoute(std::vector<SGService> &serviceList,
                         const std::vector<CRouteData> &sorted_routes,
                         const std::string &ip,
                         const bool route_flag,
                         const bool open_auto_route = true);

  /**
   * 过滤backup节点,正常情况只返回nomal角色的服务列表，
   * 只有所有服务不可用时，才返回backup节点
   */
  static int32_t FilterBackup(std::vector<SGService> &serviceList);

  /**
   * 过滤非ALIVE节点
   */
  static int32_t FilterUnAlive(std::vector<SGService> &serviceList);

  /**
   * 检测是否有alive状态服务
   */
  static bool HasAliveNode(std::vector<SGService> &serviceList);

  /**
   * 过滤非ALIVE节点
   */
  static int32_t FilterServiceName(std::vector<SGService> &serviceList,
                               std::string serviceName);

  /**
   * 获取CRoute的reserved扩展字段route_enforce值
   * reserved格式k1:v1|k2:v2..., 此处K为“route_enforce", 0:空返回全部， 1:空返回空
   */
  static std::string GetValue(std::string src, std::string key,
                              std::string sp, std::string sp2);

  /**
   *
   * @param routes
   * @param ip
   * @return
   */
  static bool IsMatchRoutesConsumer(const std::vector<CRouteData> &routes,
                                    const std::string &ip);
  /**
   * 从routes中过滤排他性路由分组，category=4
   * @param routes
   * @param exclusive_routes
   * @param is_update_routes true: 将会更新routes
   */
  static void GetExclusiveRoute(std::vector<CRouteData> &routes,
                                std::vector<CRouteData> &exclusive_routes,
                                bool is_update_routes);

  /**
   * delete the service node while it is match the exclusive_routes
   * @param serivces
   * @param exclusive_routes
   */
  static void FilterProvidersByExclusiveRoutes(std::vector<SGService> &serivces,
                                               const std::vector<CRouteData> &exclusive_routes);

/**
 * 同步fweight，将空fweight置为weight值
 */
  static int32_t SyncFweight(std::vector<SGService> &service);
 private:
  //是否可用节点
  static bool IsUnAliveService(const SGService &service);
  //主备机
  static bool IsHost(const SGService &service, bool *alive_flag);
  //权重是否符合
  static bool IsMatchWeight(const SGService &service, double thresholdWeight);
  //路由规则是否启用
  static bool IsAliveRoute(const CRouteData &route);
  //匹配servicenane
  static bool IsMatchServiceName(const SGService &service,
                                 const std::string &serviceName);
  //过滤route
  static bool IsMatchRoute(const CRouteData &route,
                           bool is_update_routes,
                           std::vector<CRouteData> *exclusive_routes);
  //强制路由过滤
  static bool IsMatchExclusiveService(const SGService &service,
                                      const CRouteData &route);
  /**
   * 通过IP信息过滤服务消费者
   * @param route
   * @param local_ip
   * @return
   */
  static bool IsMatchConsumer(const CRouteData &route,
                              const std::string &local_ip);
  /**
   * 通过服务分组过滤服务提供列表
   * @param route
   * @param oservice
   * @return
   */
  static bool FilterProvider(const CRouteData &route,
                             const SGService &oservice);
  /**
   * 对ip:port主机进行过滤,判断是否匹配上服务分组列表中的ip:port信息
   * @param regex_ip
   * @param local_ip
   * @return
   */
  static bool FilterHost(const std::string &regex_ip,
                         const std::string &local_ip);
  /**
   *
   * @param regex_ip
   * @param local_ip
   * @return
   */
  static bool RegexFilterIp(const std::string &regex_ip,
                            const std::string &local_ip);
  /**
   *
   * @param regex_ip
   * @param local_ip
   * @return
   */
  static bool RegexFilterPort(const std::string &regex_ip,
                              const std::string &local_ip);
  /**
   *
   * @param route_limit
   * @param is_idc
   * @param is_region
   * @return
   */
  static int32_t FilterIdcOrCenter(std::vector<SGService> &, const std::string &,
                               bool route_limit, bool is_idc, bool is_region);

  /**
   * 检测是否需要保留过滤掉的结果
   * true: 强制过滤， 不保留； false： 保留
   */
  static bool IsRouteLimit(const CRouteData &routeData);

  /**
   * 应用同机房路由规则
   * 如果开启强制路由，只保留同idc节点；未开启对非idc节点进行权重降级
   */
  static int32_t FilterSameIdc(std::vector<SGService> &serviceList,
                           boost::shared_ptr<IDC> local_idc, bool routeLimit);
  /**
   *
   * @param service
   * @param local_idc
   * @return
   */
  static bool IsMatchIdcRule(SGService &service, boost::shared_ptr<IDC> local_idc);

  /**
   * 应用同中心路由规则
   * 如果开启强制路由，只保留同中心节点；未开启对跨中心节点进行权重降级
   */
  static int32_t FilterSameCenter(std::vector<SGService> &serviceList,
                              boost::shared_ptr<IDC> local_idc, bool routeLimit);

  static bool IsMatchCenterRule(SGService &service, boost::shared_ptr<IDC> local_idc);
  /**
   * 应用同机房路由规则
   * 如果开启强制路由，只保留同城节点；未开启对跨城节点进行权重降级
   */
  static int32_t FilterSameRegion(std::vector<SGService> &serviceList,
                              boost::shared_ptr<IDC> local_idc, bool routeLimit);

  /**
   *
   * @param service
   * @param local_idc
   * @return
   */
  static bool IsMatchRegionRule(SGService &service, boost::shared_ptr<IDC> local_idc);
};

} // namespace


#endif

