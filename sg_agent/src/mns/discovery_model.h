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

#ifndef OCTO_OPEN_SOURCE_DISCOVERY_MODEL_H
#define OCTO_OPEN_SOURCE_DISCOVERY_MODEL_H
/**
 *服务发现: 1.直接从注册中心(如zk) ;2.直接走缓存服务；
 *         3.混合模式:从缓存服务获取&&注册中心
 *@param
 *@return
 *
 **/
#include <string>
#include <stdint.h>
#include "discovery_model.h"
#include "config_loader.h"

namespace meituan_mns{

class DiscoveryModel {
 public:
  DiscoveryModel() {};
  ~DiscoveryModel() {};
  /**
 * 进行服务发现的模式类型,zk，cached(mnsc)，mixer(cached firstly zk serconde)
 * @param void
 *
 * @return int
 *
 */
  static int32_t GetDiscoveryModelType(){return CXmlFile::GetI32Para
        (CXmlFile::DiscoveryType);};

 private:
  int32_t is_disc_zookeeper;
  int32_t is_disc_cached;
  int32_t is_disc_mixer;
  int32_t is_disc_extend;
};

}  //  namespace meituan_mns





#endif //OCTO_OPEN_SOURCE_DISCOVERY_MODEL_H
