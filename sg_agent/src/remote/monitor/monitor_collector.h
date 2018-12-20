/**
 * xxx is
 * Copyright (C) 20xx THL A29 Limited, a xxx company. All rights reserved.
 *
 * Licensed under the  xxx
 *
 * Unless required by applicable law or agreed to in writing, software distributed
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 */

#ifndef OCTO_SRC_MONITOR_COLLECTOR_H
#define OCTO_SRC_MONITOR_COLLECTOR_H
#include <assert.h>
#include <string>
#include <map>
#include <list>
#include <iterator>
#include <stdio.h>
#include <evhttp.h>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThread.h>
#include <muduo/net/EventLoopThreadPool.h>
#include <boost/algorithm/string/trim.hpp>
#include "cJSON.h"
#include "config_loader.h"
#include "falcon_mgr.h"
#include "inc_comm.h"


namespace meituan_mns {

typedef struct cpu_util{

  unsigned long total_cpu_delta;
  unsigned long proc_cpu_delta;
  cpu_util(){total_cpu_delta = 0.0;proc_cpu_delta = 0.0;};
}cpu_util_t;

class MonitorCollector {

 private:
  MonitorCollector() : has_init_(false),
                         end_point_(""),
                         agent_pid_(""),
                         monitor_data_(boost::unordered_map<std::string, int>()) {};
 public:
  ~MonitorCollector() {};
  static MonitorCollector *GetInstance();
  int DoInitMonitorInfo();
  int GetCollectorMonitorInfo(std::string &mInfo);
  int CollectorInfo2Json(cJSON *json, cJSON *json_arrary, int type);
  float CalcuProcCpuUtil(const int& pid);

 private:
  void SetValueByType(cJSON *root,int type);
  void GetEndPoint(std::string &value);
  int64_t GetTimeStamp();


  static MonitorCollector *monitor_collector_;
  static muduo::MutexLock monitor_collector_lock_;
  std::map<int, std::string> metric_value_;
  bool has_init_;
  std::string end_point_;
  std::string agent_pid_;
  cpu_util_t cpu_jiff_value_;
  boost::unordered_map<std::string, int> monitor_data_;
};

}
#endif //OCTO_SRC_MONITOR_COLLECTOR_H
