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

#ifndef OCTO_OPEN_SOURCE_ZKTOOLS_H
#define OCTO_OPEN_SOURCE_ZKTOOLS_H
#include <string.h>

#include "zk_path_tools.h"
#include "base_consts.h"
#include "buffer_mgr.h"
namespace meituan_mns {

class VersionManager {
 public:
  VersionManager();
  ~VersionManager() {}
  /**
   *
   * @param appkey
   * @param zk_version
   * @param version
   * @return
   */
  int CheckZkVersion(const std::string &appkey,
                     const std::string &zk_version,
                     std::string &version);
  /**
   *
   * @param version
   * @return
   */
  bool IsOldVersion(const std::string &version);


 private:

  //mthrift < 1.7.0
  bool CompareMTVersion(const std::string &version);
  //piegon < 2.8
  bool ComparePigeonVersion(const std::string &version);
  //cthrift < 2.6.0
  bool CompareCthriftVersion(const std::string &version);
  /**
   * 字符串中的版本号信息提取到vector
   * @param version_str
   * @param p_vec_version
   */
  void GetVersion(const std::string &version_str,
                  std::vector<int> *p_vec_version);

  /**
 * 暂不支持两个长度不等的判断
 * @param a
 * @param b
 * @return
 */
  int32_t CompareVector(const std::vector<int> &a,
                        const std::vector<int> &b);

 private:

  int32_t zk_update_time_;
  int32_t zk_update_rand_time_;
  boost::shared_ptr <BufferMgr<int> > key_buffer_;

  std::vector<int> old_mtthrift_version_;
  std::vector<int> old_pigeon_version_;
  std::vector<int> old_cthrift_version_;

};

}  //  namespace meituan_mns


#endif //OCTO_OPEN_SOURCE_ZKTOOLS_H
