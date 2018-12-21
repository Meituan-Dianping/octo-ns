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

#ifndef OCTO_OPEN_SOURCE_ZK_PATH_TOOLS_H
#define OCTO_OPEN_SOURCE_ZK_PATH_TOOLS_H

#include "naming_common_types.h"
#include "base_mns_consts.h"
#include "inc_comm.h"
#include "whitelist_manager.h"

namespace meituan_mns {


class AgentZkPath {
 public:
  AgentZkPath() {}
  ~AgentZkPath() {}
  /**
   *
   * @param provider_path
   * @param appkey
   * @param protocol
   * @param nodeType
   * @return
   */
  int32_t GenProtocolZkPath(std::string &provider_path,
                            const std::string &appkey,
                            const std::string &protocol,
                            std::string &nodeType);
  /**
   *
   * @param provider_path
   * @param appkey
   * @param protocol
   * @param serverType
   * @return
   */
  int32_t GenRegisterZkPath(std::string &provider_path,
                            const std::string &appkey,
                            const std::string &protocol,
                            const int serverType);
  /**
   *
   * @param provider_path
   * @param serviceName
   * @param protocol
   * @param appkey
   * @return
   */
  int32_t GenServiceNameZkPathNode(std::string &provider_path,
                               const std::string &serviceName,
                               const std::string &protocol,
                               const std::string &appkey = "");
  /**
   *
   * @param provider_path
   * @param appkey
   * @return
   */
  int32_t GenDescZkPath(std::string &provider_path,
                        const std::string &appkey);
  /**
   *
   * @param env_str
   * @param appkey
   * @return
   */
  int32_t GetEnvByAppkeyWhiteList(std::string &env_str,
                                  const std::string &appkey);
  /**
   *
   * @param nodeType
   * @param protocol
   * @return
   */
  int32_t GenNodeType(std::string &nodeType,
                      const std::string &protocol);

  /**
   *
   * @param servicename
   * @return
   */
  std::string ReplaceHttpServiceName(const std::string &servicename);
  /**
   *
   * @param protocol
   * @return
   */
  static int32_t DeGenNoDeGenNodeType(std::string &protocol);

  /**
   *
   * @param zk_path
   * @param data_path
   * @return
   */
  std::string GenRoutePath(const std::string &zk_path,
                           const std::string &data_path);


  /**
   *
   * @param zkPath
   * @param appkey
   * @param protocol
   * @return
   */
  static int32_t DeGenZkPath(const char* zkPath,
                             std::string &appkey,
                             std::string &protocol);

 private:
  WhiteListManager whitelist_manager_;

};
}  //  namespace meituan_mns

#endif //OCTO_OPEN_SOURCE_ZK_PATH_TOOLS_H
