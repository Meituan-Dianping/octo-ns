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

#ifndef _whitelist_mgr_H__
#define _whitelist_mgr_H__

#include <string>
#include "config_loader.h"

namespace meituan_mns {

class WhiteListManager {
 public:
  WhiteListManager() {}
  ~WhiteListManager() {}
  /**
   *
   * @param appkey
   * @return
   */
  bool IsAppkeyInWhitList(const std::string &appkey);
  /**
   *
   * @param appkey
   * @return
   */
  bool IsAppkeyInRegistUnlimitWhitList(const std::string &appkey);
  /**
   *
   * @param appkey
   * @return
   */
  bool IsAppkeyInAllEnvWhitList(const std::string &appkey);

  /**
   *
   * @param appkey
   * @return
   */
  bool IsMacRegisterAppkey(const std::string &appkey);
 private:
  /**
   *
   * @param appkey
   * @param appkeys
   * @return
   */
  bool IsAppkeyContains(const std::string &appkey,
                      const UnorderedMapPtr &appkeys);
  /**
   *
   * @param appkey
   * @param appkeys
   * @return
   */
  bool IsAppkeyList(const std::string &appkey,
                    const UnorderedMapPtr &appkeys);
};

}

#endif
