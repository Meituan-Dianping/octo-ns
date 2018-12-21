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

#include "monitor_collector.h"
#include "base_consts.h"

using namespace meituan_mns;


MonitorCollector *MonitorCollector::monitor_collector_ = NULL;
muduo::MutexLock MonitorCollector::monitor_collector_lock_;

MonitorCollector *MonitorCollector::GetInstance() {
  if (NULL == monitor_collector_) {
    muduo::MutexLockGuard lock(monitor_collector_lock_);
    if (NULL == monitor_collector_) {
      monitor_collector_ = new MonitorCollector();
    }
  }
  return monitor_collector_;
}

float MonitorCollector::CalcuProcCpuUtil(const int& pid) {

  float proc_cpu_util = 0.0;
  unsigned long total_cpu_delta = GetTotalCpuTime() - cpu_jiff_value_.total_cpu_delta;
  unsigned long proc_cpu_delta = GetProcCpuTime(pid) - cpu_jiff_value_.proc_cpu_delta;
  NS_LOG_INFO("proc_cpu_delta: " << proc_cpu_delta << "total_cpu_delta: " << total_cpu_delta);
  std::string test_cpu = "";
  if (0 != total_cpu_delta) {
    proc_cpu_util = (float) (100 * sysconf(_SC_NPROCESSORS_CONF) * proc_cpu_delta) / (float) total_cpu_delta;
    cpu_jiff_value_.total_cpu_delta = GetTotalCpuTime();
    cpu_jiff_value_.proc_cpu_delta = GetProcCpuTime(pid);
    NS_LOG_INFO("the sg_agent proc cpu util: " << proc_cpu_util);
  } else {
    NS_LOG_ERROR("it is first to calcu proc cpu util,value is empty");
  }

  NS_LOG_INFO("the sg_agent proc cpu util: " << Round(proc_cpu_util, 2));
  return Round(proc_cpu_util, 2);
}
int MonitorCollector::DoInitMonitorInfo() {

  if (!has_init_) {
    metric_value_[PID_ITEM] = "sg_agent.pid";
    metric_value_[VMRSS_ITEM] = "sg_agent.vmRss";
    metric_value_[VERSION_ITEM] = "sg_agent.version";
    metric_value_[CPU_ITEM] = "sg_agent.cpu";
    metric_value_[FILECONFIG_ITEM] = "sg_agent.fileConfigQueueLen";
    metric_value_[KVCONFIG_ITEM] = "sg_agent.kvConfigQueueLen";
    metric_value_[COMMON_LOG_ITEM] = "sg_agent.logQueueLen";
    metric_value_[ROUTE_LIST_ITEM] = "sg_agent.routeListQueueLen";
    metric_value_[SERVICE_LIST_ITEM] = "sg_agent.serviceListQueueLen";
    metric_value_[MODULE_INVOKER_ITEM] = "sg_agent.invokerQueueLen";
    metric_value_[REGISTER_ITEM] = "sg_agent.registerQueueLen";
    metric_value_[MCC_FCONFIG_ALLREQ_ITEM] = "sg_agent.mccFileConfigAllReq";
    metric_value_[MCC_FCONFIG_SUCCESSREQ_ITEM] = "sg_agent.mccFileConfigSuccessReq";
    metric_value_[MCC_CONFIG_ALLREQ_ITEM] = "sg_agent.mccAllReq";
    metric_value_[MCC_CONFIG_SUCCESSREQ_ITEM] = "sg_agent.mccSucessReq";
    metric_value_[MNSC_ALLREQ_ITEM] = "sg_agent.mnscAllReq";
    metric_value_[MNSC_SUCCESSREQ_ITEM] = "sg_agent.mnscSuccessReq";
    cpu_jiff_value_.proc_cpu_delta = GetProcCpuTime(getpid());
    cpu_jiff_value_.total_cpu_delta = GetTotalCpuTime();

    GetEndPoint(end_point_);
    has_init_ = true;
  }
  return SUCCESS;
}
int MonitorCollector::GetCollectorMonitorInfo(std::string &mInfo) {

  DoInitMonitorInfo();

  char *out;
  cJSON *json = cJSON_CreateObject();
  if (!json) {
    NS_LOG_ERROR("json is NULL, create json_object failed.");
    return FAILURE;
  }
  cJSON *all_info_json = cJSON_CreateArray();
  if (!all_info_json) {
    NS_LOG_ERROR("all_srvlist_json is NULL, create json_object failed.");
    cJSON_AddNumberToObject(json, "ret", HTTP_INNER_ERROR);
    mInfo = cJSON_Print(json);
    cJSON_Delete(json);
    return FAILURE;
  }
  cJSON_AddNumberToObject(json, "ret", HTTP_RESPONSE_OK);
  cJSON_AddStringToObject(json, "retMsg", "success");

  //CountRequest::GetInstance()->GetReqData(monitor_data_);
  for (int iter = 0; iter <= MAX_MONITOER_ITEM; iter++) {
    CollectorInfo2Json(json, all_info_json, iter);
  }
  cJSON_AddItemToObject(json, "data", all_info_json);
  out = cJSON_Print(json);
  mInfo = out;
  SAFE_FREE(out);
  cJSON_Delete(json);
  NS_LOG_INFO("CollectorInfo2Json success");

  return SUCCESS;
}
int64_t MonitorCollector::GetTimeStamp() {

  timeval cur_time;
  gettimeofday(&cur_time, NULL);

  return ((int64_t) cur_time.tv_sec);

}
void MonitorCollector::GetEndPoint(std::string &end_point) {

  char host_name[256] = {0};
  gethostname(host_name, sizeof(host_name));
  std::string host_name_str(host_name);
  NS_LOG_INFO("end point" << host_name_str);
  end_point = (std::string::npos != host_name_str.find(".octo.com")
      || std::string::npos != host_name_str.find(".office.mos")) ? host_name_str.substr(0, host_name_str.find("."))
                                                                 : host_name_str;
  if (end_point.empty() || "unknown" == end_point) {
    NS_LOG_WARN("fail to init monitor endpoint");
  }
}
int MonitorCollector::CollectorInfo2Json(cJSON *json, cJSON *json_arrary, int type) {

  if (NULL == json || NULL == json_arrary) {
    NS_LOG_ERROR("the json object is null");
    return FAILURE;
  }
  std::string mon_info = "";
  cJSON *root;
  char *out;
  root = cJSON_CreateObject();
  if (NULL == root) {
    NS_LOG_ERROR("the create json object is failed");
    return FAILURE;
  }
  cJSON_AddStringToObject(root, "endpoint", end_point_.c_str());
  cJSON_AddStringToObject(root, "metric", metric_value_.at(type).c_str());
  cJSON_AddNumberToObject(root, "timestamp", GetTimeStamp());
  cJSON_AddNumberToObject(root, "step", 60);
  SetValueByType(root, type);
  cJSON_AddStringToObject(root, "counterType", "GAUGE");
  cJSON_AddStringToObject(root, "tags", "sg_agent");
  out = cJSON_Print(root);
  mon_info = out;
  SAFE_FREE(out);
  cJSON_AddItemToArray(json_arrary, root);

  return SUCCESS;

}
void MonitorCollector::SetValueByType(cJSON *root, int type) {

  pid_t sg_pid = getpid();
  switch (type) {
    case PID_ITEM: {
      cJSON_AddNumberToObject(root, "value", getpid());
      break;
    }
    case VMRSS_ITEM: {
      cJSON_AddNumberToObject(root, "value", GetProcMemUtil(sg_pid));
      break;
    }
    case VERSION_ITEM: {
      cJSON_AddStringToObject(root, "value", CXmlFile::GetStrPara
          (CXmlFile::AgentVersion).c_str());
      break;
    }
    case CPU_ITEM: {
      cJSON_AddNumberToObject(root, "value", CalcuProcCpuUtil(sg_pid));
      break;
    }
    case FILECONFIG_ITEM: {
      cJSON_AddNumberToObject(root, "value", FalconMgr::GetFileConfigQueueSize());
      break;
    }
    case KVCONFIG_ITEM: {
      cJSON_AddNumberToObject(root, "value", FalconMgr::GetKvConfigQueueSize());
      break;
    }
    case COMMON_LOG_ITEM: {
      cJSON_AddNumberToObject(root, "value", FalconMgr::GetCommonLogQueueSize());
      break;
    }
    case ROUTE_LIST_ITEM: {
      cJSON_AddNumberToObject(root, "value", FalconMgr::GetRouteListQueueSize());
      break;
    }
    case SERVICE_LIST_ITEM: {
      cJSON_AddNumberToObject(root, "value", FalconMgr::GetServiceListQueueSize());
      break;
    }
    case MODULE_INVOKER_ITEM: {
      cJSON_AddNumberToObject(root, "value", FalconMgr::GetModuleInvokerQueueSize());
      break;
    }
    case REGISTER_ITEM: {
      cJSON_AddNumberToObject(root, "value", FalconMgr::GetRegisteQueueSize());
      break;
    }
    case MCC_FCONFIG_ALLREQ_ITEM: {
      cJSON_AddNumberToObject(root, "value", monitor_data_.at("allfconfig"));
      break;
    }
    case MCC_FCONFIG_SUCCESSREQ_ITEM: {
      cJSON_AddNumberToObject(root, "value", monitor_data_.at("fconfig"));
      break;
    }
    case MCC_CONFIG_ALLREQ_ITEM: {
      cJSON_AddNumberToObject(root, "value", monitor_data_.at("allconfig"));
      break;
    }
    case MCC_CONFIG_SUCCESSREQ_ITEM: {
      cJSON_AddNumberToObject(root, "value", monitor_data_.at("config"));
      break;
    }
    case MNSC_ALLREQ_ITEM: {
      cJSON_AddNumberToObject(root, "value", monitor_data_.at("allmnsc"));
      break;
    }
    case MNSC_SUCCESSREQ_ITEM: {
      cJSON_AddNumberToObject(root, "value", monitor_data_.at("mnsc"));
      break;
    }
    default: {
      NS_LOG_INFO("unkown collector type");
    }
  }
}

