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

#include <sys/time.h>
#include <map>
#include "boost/lambda/lambda.hpp"
#include "route_rule.h"
#include <boost/bind.hpp>

#include "../gen-cpp/naming_common_types.h"

namespace meituan_mns {


const static std::string LIMIT_KEY = "route_limit";
const static std::string LIMIT_VALUE = "1";
const static std::string UNLIMIT_VALUE = "0";

// 启用本机房过滤的权重
const static double SameRegionMod = 0.001;
const static double DiffRegionMod = 0.000001;
const static double DoublePrecision = 0.000001;

bool RouteRule::IsMatchWeight(const SGService &service,
                              double thresholdWeight) {
  if (thresholdWeight - service.fweight >= DoublePrecision) {
    NS_LOG_DEBUG("filterWeight is 0, appkey = "
                       << service.appkey
                       << " status : " << service.status
                       << " role : " << service.role
                       << " ip : " << service.ip.c_str());
    return true;
  }
  return false;
}

int32_t RouteRule::FilterWeight(std::vector<SGService> &serviceList,
                            double thresholdWeight) {
  std::vector<SGService> serviceList_tmp;
  serviceList_tmp = serviceList;

  //删除weight = 0 的节点
  serviceList_tmp.erase(std::remove_if(serviceList_tmp.begin(),serviceList_tmp.end(),
                                       boost::bind(&RouteRule::IsMatchWeight,_1,thresholdWeight)),
                        serviceList_tmp.end());

  if (serviceList_tmp.size() != serviceList.size()
      && serviceList_tmp.size() != 0) {
    serviceList = serviceList_tmp;
    return 0;
  }

  // 表示过滤后的结果为空， 保留原值
  return -1;
}

bool RouteRule::IsHost(const SGService &service, bool *alive_flag) {
  if (0 != service.role) {
    NS_LOG_DEBUG("filterBackup, appkey = " << service.appkey
                                             << " status : " << service.status
                                             << " role : " << service.role
                                             << " ip : " << service.ip.c_str());
    return true;
  }
  if (!(*alive_flag) && fb_status::ALIVE == service.status) {
    *alive_flag = true;
  }
  return false;
}

int32_t RouteRule::FilterBackup(std::vector<SGService> &serviceList) {
  std::vector<SGService> serviceList_tmp = serviceList;
  bool is_master_alive = false;
  serviceList.erase(std::remove_if(serviceList.begin(),
                                   serviceList.end(),
                                   boost::bind(&RouteRule::IsHost, _1,
                                   &is_master_alive)),
                                   serviceList.end());

  //如果tmp中节点信息与原来节点数不同，且至少存在一个alive节点
  //则进行copy更新serviceList;
  //如果tmp节点数为0了， 或者没有alive节点
  //则返回原来所有节点，包括备份节点
  if (serviceList.empty() || !is_master_alive) {
    //所有主节点不可用是小概率case，因此代码低概率执行，避免memory copy
    serviceList = serviceList_tmp;
  }

  return 0;
}

bool RouteRule::IsUnAliveService(const SGService &service) {
  if (service.status != fb_status::ALIVE) {
    NS_LOG_INFO("filterUnAlive servicee is not alive, appkey = "
                    << service.appkey
                    << " status : "
                    << service.status
                    << " role : "
                    << service.role
                    << " ip : "
                    << service.ip.c_str());
    return true;
  }
  return false;
}

int32_t RouteRule::FilterUnAlive(std::vector<SGService> &serviceList) {
  serviceList.erase(std::remove_if(serviceList.begin(), serviceList.end(),
                                   boost::bind(&RouteRule::IsUnAliveService, _1)),
                    serviceList.end());

  return 0;
}

bool RouteRule::HasAliveNode(std::vector<SGService> &serviceList) {
  for (std::vector<SGService>::iterator iter = serviceList.begin();
       iter != serviceList.end(); ++iter) {
    if (fb_status::ALIVE == iter->status) {
      return true;
    }
  }
  return false;
}

/*
 * 对于192.168.0.196:*这类host，过滤带port的host
 */
bool RouteRule::RegexFilterPort(const std::string &regex_ip, const std::string &local_ip) {
  NS_LOG_DEBUG("RegexFilterPort input param: regex_ip: " << regex_ip << " localIp: " << local_ip);
  //对于ip地址第4位以及端口进行切分,例如： 196:8890
  std::vector<std::string> regex_ip_vec;
  SplitStringIntoVector(regex_ip.c_str(), ":", regex_ip_vec);

  if (2 != regex_ip_vec.size()) {
    NS_LOG_WARN("regex_ip =" << regex_ip << "is error");
    return false;
  }

  std::vector<std::string> local_ip_vec;
  SplitStringIntoVector(local_ip.c_str(), ":", local_ip_vec);

  if (RegexFilterIp(regex_ip_vec[0], local_ip_vec[0])) {
    if ("*" == regex_ip_vec[1] || "*\"" == regex_ip_vec[1] || regex_ip_vec[1]
        == local_ip_vec[1]) {
      NS_LOG_INFO("filter ip OK, filter port OK!");
      return true;
    }
  }

  return false;
}

bool RouteRule::IsMatchIdcRule(SGService &service,
                                    boost::shared_ptr<IDC> local_idc) {

  return !IdcUtil::IsSameIdc(local_idc, IdcUtil::GetIdc(service.ip));
}

/**
 * 同机房优先规则处理流程
 */
int32_t RouteRule::FilterSameIdc(std::vector<SGService> &serviceList,
                                  boost::shared_ptr<IDC> local_idc,
                                  bool routeLimit) {
  if (routeLimit) {
    serviceList.erase(std::remove_if(serviceList.begin(),serviceList.end(),
                                     boost::bind(&RouteRule::IsMatchIdcRule,
                                                  _1, local_idc)),
                                               serviceList.end());
  } else {
    std::vector<SGService>::iterator iter = serviceList.begin();
    for (; serviceList.end() != iter; ++iter) {
      boost::shared_ptr<IDC> idc_item = IdcUtil::GetIdc(iter->ip);
      //idc_item 智能指针的判断合法性，没有意义
      if (IdcUtil::IsSameIdc(local_idc, idc_item)) {
        continue;
      }else if (local_idc->GetRegion() == idc_item->GetRegion()) {
        iter->fweight *= SameRegionMod;
      } else {
        iter->fweight *= DiffRegionMod;
      }
    }
  }
}

bool RouteRule::IsMatchCenterRule(SGService &service,
                                       boost::shared_ptr<IDC> local_idc) {

  return !IdcUtil::IsSameCenter(local_idc, IdcUtil::GetIdc(service.ip));
}


int32_t RouteRule::FilterSameCenter(std::vector<SGService> &serviceList,
                                     boost::shared_ptr<IDC> local_idc,
                                     bool routeLimit) {
  if (routeLimit) {
    serviceList.erase(std::remove_if(serviceList.begin(),serviceList.end(),
                                     boost::bind(&RouteRule::IsMatchCenterRule,
                                                 _1, local_idc)),
                      serviceList.end());
  } else {
    std::vector<SGService>::iterator iter = serviceList.begin();
    for (; serviceList.end() != iter; ++iter) {
      boost::shared_ptr <IDC> idc_item = IdcUtil::GetIdc(iter->ip);
      //idc_item 智能指针的判断合法性，没有意义
      if (IdcUtil::IsSameCenter(local_idc, idc_item)) {
        continue;
      } else if (local_idc->GetRegion() == idc_item->GetRegion()) {
        iter->fweight *= SameRegionMod;
      } else {
        iter->fweight *= DiffRegionMod;
      }
    }
  }
}

bool RouteRule::IsMatchRegionRule(SGService &service,
                                       boost::shared_ptr<IDC> local_idc) {
  boost::shared_ptr <IDC> idc_item = IdcUtil::GetIdc(service.ip);
  return local_idc->GetRegion() != idc_item->GetRegion();
}


int32_t RouteRule::FilterSameRegion(std::vector<SGService> &serviceList,
                                boost::shared_ptr<IDC> local_idc,
                                bool routeLimit) {
  if (routeLimit) {
    serviceList.erase(std::remove_if(serviceList.begin(),serviceList.end(),
                                     boost::bind(&RouteRule::IsMatchRegionRule,
                                     _1, local_idc)),
                                     serviceList.end());
  } else {
    std::vector<SGService>::iterator iter = serviceList.begin();
    for (; serviceList.end() != iter; ++iter) {
      boost::shared_ptr <IDC> idc_item = IdcUtil::GetIdc(iter->ip);
      //无需idc_item 智能指针的判断合法性，没有意义
      if (local_idc->GetRegion() == idc_item->GetRegion()) {
        continue;
      } else {
        iter->fweight *= DiffRegionMod;
      }
    }
  }
}

bool RouteRule::FilterHost(const std::string &regex_ip,
                           const std::string &local_ip) {
  NS_LOG_DEBUG("FilterHost input param: re_ip: " << regex_ip
                                                 << " localIp: " << local_ip);
  //首先判断host格式是否为，192.168.1.196:8890
  std::vector<std::string> re_ip_vec;
  SplitStringIntoVector(regex_ip.c_str(), ":", re_ip_vec);

  //对于格式为, ip:port的host，单独过滤下port
  if (2 == re_ip_vec.size()) {
    if (RegexFilterPort(regex_ip, local_ip)) {
      NS_LOG_DEBUG("RegexFilterPort OK! filter host is: " << regex_ip
                                                          << " local host is: "
                                                          << local_ip);
      return true;
    }
  } else {
    if (RegexFilterIp(regex_ip, local_ip)) {
      NS_LOG_DEBUG("RegexFilterIp OK! filter IP is: " << regex_ip
                                                      << " local IP is: "
                                                      << local_ip);
      return true;
    }
  }

  return false;
}

bool RouteRule::RegexFilterIp(const std::string &reIp, const std::string &localIp) {
  //对于格式仅为IP的，分组ip中带*号的ip进行过滤
  std::vector<std::string> vecLocal;
  SplitStringIntoVector(localIp.c_str(), ".", vecLocal);

  std::vector<std::string> vecRe;
  SplitStringIntoVector(reIp.c_str(), ".", vecRe);

  if (vecRe.size() == 1) {
    if (vecRe[0] == "\"*\"" or vecRe[0] == "*" or vecRe[0] == "\"*")
      return true;
  } else if (vecRe.size() == 2) {
    if ((((vecRe[0] == "\"*") or (vecRe[0] == "*"))
        and (vecRe[1] == "*" or vecRe[1] == "*\""))
        or ((vecRe[0] == vecLocal[0])
            and ((vecRe[1] == "*\"") or (vecRe[1] == "*"))))
      return true;
  } else if (vecRe.size() == 3) {
    if (((vecRe[2] == "*\"" or vecRe[2] == "*")
        and (vecRe[0] == vecLocal[0])
        and (vecRe[1] == vecLocal[1]))
        or ((vecRe[2] == "*\"" or vecRe[2] == "*")
            and (vecRe[0] == vecLocal[0])
            and (vecRe[1] == "*"))
        or ((vecRe[2] == "*\"" or vecRe[2] == "*")
            and (vecRe[0] == "*" or vecRe[0] == "\"*")
            and (vecRe[1] == "*")))
      return true;
  } else if (vecRe.size() == 4) {
    if (((vecRe[3] == "*\"" or vecRe[3] == "*"
        or vecRe[3] == vecLocal[3])
        and (vecRe[0] == vecLocal[0])
        and (vecRe[1] == vecLocal[1])
        and (vecRe[2] == vecLocal[2]))
        or ((vecRe[3] == "*\"" or vecRe[3] == "*"
            or vecRe[3] == vecLocal[3])
            and (vecRe[0] == vecLocal[0])
            and (vecRe[1] == vecLocal[1])
            and (vecRe[2] == "*"))
        or ((vecRe[3] == "*\"" or vecRe[3] == "*"
            or vecRe[3] == vecLocal[3])
            and (vecRe[0] == vecLocal[0])
            and (vecRe[1] == "*")
            and (vecRe[2] == "*"))
        or ((vecRe[3] == "*\"" or vecRe[3] == "*"
            or vecRe[3] == vecLocal[3])
            and ((vecRe[0] == "*" or vecRe[0] == "\"*"))
            and (vecRe[1] == "*")
            and (vecRe[2] == "*")))
      return true;
  } else {
    NS_LOG_WARN("route consumer ip is: " << reIp.c_str()
                                           << ", localIp is: "
                                         << localIp.c_str());
    return false;
  }

  return false;
}

//consume列表过滤服务分组
bool RouteRule::IsMatchConsumer(const CRouteData &route,
                                const std::string &local_ip) {

  //一期实现基于localIp过滤consume,appkey过滤暂时不考虑
  std::vector<std::string> ips = route.consumer.ips;
  for (std::vector<std::string>::const_iterator iter = ips.begin();
       ips.end() != iter; ++iter) {
    if ((*iter == local_ip) || FilterHost(*iter, local_ip)) {
      NS_LOG_DEBUG("filterConsumer use ip = " << *iter);
      return true;
    }
  }

  return false;
}

//provide列表过滤服务分组
bool RouteRule::FilterProvider(const CRouteData &route, const SGService &oservice) {

  //自定义分组，通过ip:port格式比对
  std::string ip_port_str = "\"" + oservice.ip
      + ":" + boost::lexical_cast<std::string>(oservice.port) + "\"";

  for (std::vector<std::string>::const_iterator iter = route.provider.begin();
       route.provider.end() != iter; ++iter) {
    if ((*iter == ip_port_str) || FilterHost(*iter, ip_port_str)) {
      return true;
    }
  }
  return false;
}

/*
 * 根据服务分组列表，过滤服务列表
 * */
int32_t RouteRule::FilterRoute(std::vector<SGService> &services,
                                const std::vector<CRouteData> &sorted_routes,
                                const std::string &ip,
                                const bool route_flag,
                                const bool open_auto_route ){
  std::vector<CRouteData>::const_iterator iter = sorted_routes.begin();
  std::vector<CRouteData>::const_iterator end_iter = sorted_routes.end();
  //获取sg_agent的IP，用于过滤consumer, 初始化时已经获取到IP
  std::string local_ip = "\"" + ip + "\"";
  std::vector <CRouteData> not_exclusive_routes = sorted_routes;
  if (route_flag) {
    NS_LOG_DEBUG("active original routes'size = " << sorted_routes.size());
    std::vector <CRouteData> exclusive_routes;

    GetExclusiveRoute(not_exclusive_routes, exclusive_routes, true);

    bool is_local_ip_in_exclusive_routes = IsMatchRoutesConsumer(
        exclusive_routes, local_ip);
    if (!is_local_ip_in_exclusive_routes) {
      FilterProvidersByExclusiveRoutes(services, exclusive_routes);
    }

    iter = is_local_ip_in_exclusive_routes ?
           sorted_routes.begin() : not_exclusive_routes.begin();
    end_iter = is_local_ip_in_exclusive_routes ?
               sorted_routes.end() : not_exclusive_routes.end();
  }

  boost::shared_ptr<IDC> local_idc = IdcUtil::GetIdc(ip);
  //遍历服务分组列表，按照优先级从高到低依次遍历
  for (; iter != end_iter; ++iter) {
    bool routeLimit = IsRouteLimit(*iter);
    // 如果符合本机房过滤的条件， 进行本机房过滤
    switch (iter->category) {
      case 1:
        if (open_auto_route) {
          FilterSameIdc(services, local_idc, routeLimit);
        }
        return SUCCESS;
      case 3:
        if (open_auto_route) {
          FilterSameCenter(services, local_idc, routeLimit);
        }
        return SUCCESS;
      case 5:{
        if (open_auto_route) {
          FilterSameRegion(services, local_idc, routeLimit);
        }
        return SUCCESS;
      }
      case 4:
      default:

        // 先匹配comsumer:如果consumer都没匹配到，则应用下一条route规则
        // 否则应用该规则， 不论结果是否为空， 都跳出循环
        if (!IsMatchConsumer(*iter, local_ip)) {
          break;
        }
        NS_LOG_DEBUG("filter route OK! using route name is : "
                         << iter->name
                         << ", priority is : " << iter->priority);

        std::vector<SGService> serviceList_tmp;
        //再过滤provide,如果为空则返回所有service
        for (std::vector<SGService>::iterator vec = services.begin();
             vec != services.end(); ++vec) {
          //如果route中匹配到，则添加此provider到tmp
          if (FilterProvider(*iter, *vec)) {
            SGService service_tmp = *vec;
            serviceList_tmp.push_back(service_tmp);
          } else {
            if (!routeLimit) {
              SGService service_tmp = *vec;
              service_tmp.weight = 0;
              service_tmp.fweight *= SameRegionMod;
              serviceList_tmp.push_back(service_tmp);
            }
          }
        }

        services = serviceList_tmp;
        return SUCCESS;
    }
  }

  return SUCCESS;
}

bool RouteRule::IsAliveRoute(const CRouteData &route) {
  return 1 != route.status;
}

/*
 * 根据服务分组优先级排序，并且过滤掉下线状态的服务分组
 * */
int32_t RouteRule::SortRouteList(std::vector<CRouteData> &routeList) {
  if (routeList.empty()) {
    NS_LOG_WARN("routeList is empty");
    return -1;
  }
  //首先过滤掉status!=1的分组
  routeList.erase(std::remove_if(routeList.begin(),routeList.end(),
                                 boost::bind(&RouteRule::IsAliveRoute,_1)),
                  routeList.end());

  if (routeList.size() == 0) {
    return -1;
  }

  //对routList按照优先级逆序排列
  std::sort(routeList.begin(), routeList.end(), CompRouterData);

  return 0;
}

std::string RouteRule::GetValue(std::string src, std::string key,
                                     std::string sp, std::string sp2) {
  if (src.empty() || key.empty() || sp.empty() || sp2.empty()) {
    return "";
  }

  std::vector<std::string> kvs;
  //split(kvs, src, boost::is_any_of<std::string>(sp));
  int32_t ret = SplitStringIntoVector(src.c_str(), sp.c_str(), kvs);
  if (0 == ret) {
    return "";
  }

  std::vector<std::string> kv;
  for (std::vector<std::string>::iterator iter = kvs.begin();
       iter != kvs.end(); ++iter) {
    kv.clear();
    //split(kv, *iter, boost::is_any_of<std::string>(sp2));
    ret = SplitStringIntoVector(iter->c_str(), sp2.c_str(), kv);
    if (kv.size() == 2 && key == kv[0]) {
      return kv[1];
    }
  }

  return "";
}

/**
 * 检测是否需要保留过滤掉的结果
 */
bool RouteRule::IsRouteLimit(const CRouteData &routeData) {
  //当配置route_limit=1, 返回true; 否则返回alse
  if (!(routeData.reserved).empty()) {
    //解析reserved内容
    std::string value = GetValue(routeData.reserved, LIMIT_KEY, "|", ":");
    if (LIMIT_VALUE == value) {
      return true;
    }
  }
  return false;
}

bool RouteRule::IsMatchServiceName(const SGService &service,
                                   const std::string &serviceName) {
  return service.serviceInfo.end() == service.serviceInfo.find(serviceName);
}

int32_t RouteRule::FilterServiceName(std::vector<SGService> &serviceList,
                                      std::string serviceName) {
  serviceList.erase(std::remove_if(serviceList.begin(), serviceList.end(),
                                   boost::bind(&RouteRule::IsMatchServiceName,
                                               _1, serviceName)),
                    serviceList.end());
  return 0;
}

int32_t RouteRule::SyncFweight(std::vector<SGService> &services) {
  for (std::vector<SGService>::iterator iter = services.begin();
       iter != services.end(); ++iter) {
    if (0 == iter->fweight) {
      iter->fweight = iter->weight;
    }
  }
  return 0;
}

bool RouteRule::IsMatchRoute(const CRouteData &route,
                                  bool is_update_routes,
                                  std::vector<CRouteData> *exclusive_routes) {
  if (4 == route.category) {
    exclusive_routes->push_back(route);
    if (is_update_routes) {
      return true;
    }
  }
  return false;
}

void RouteRule::GetExclusiveRoute(std::vector<CRouteData> &routes,
                                       std::vector<CRouteData> &exclusive_routes,
                                       bool is_update_routes) {
  routes.erase(std::remove_if(routes.begin(), routes.end(),
                              boost::bind(&RouteRule::IsMatchRoute, _1,
                                          is_update_routes,&exclusive_routes)),
                                          routes.end());
}


bool RouteRule::IsMatchExclusiveService(const SGService &service,
                                             const CRouteData &route) {
  if (FilterProvider(route, service)) {
    NS_LOG_DEBUG("delete ip = " << service.ip << " port = " << service.port
                                << "from the service list");
    return true;
  }
  return false;
}
void RouteRule::FilterProvidersByExclusiveRoutes(std::vector<SGService> &services,
                                                 const std::vector<CRouteData> &exclusive_routes) {
  // delete service node while it is match the exclusive_routes
  for (std::vector<CRouteData>::const_iterator route_iter = exclusive_routes.begin();
       exclusive_routes.end() != route_iter; ++route_iter) {
    services.erase(std::remove_if(services.begin(), services.end(),
                                  boost::bind(&RouteRule::IsMatchExclusiveService,
                                              _1, *route_iter)),
                                                services.end());
  }
}
bool RouteRule::IsMatchRoutesConsumer(const std::vector<CRouteData> &routes,
                                      const std::string &ip) {
  for (std::vector<CRouteData>::const_iterator iter = routes.begin();
       routes.end() != iter; ++iter) {
    if (IsMatchConsumer(*iter, ip)) {
      return true;
    }
  }
  return false;
}

} //namespace
