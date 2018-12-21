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

#ifndef OCTO_OPEN_SOURCE_SERVICE_CHANNELS_H
#define OCTO_OPEN_SOURCE_SERVICE_CHANNELS_H

namespace meituan_mns {
class ServiceChannels {

 public:

  ServiceChannels();
  ~ServiceChannels() {};
  /**
  * Set服务列表归属的channel标识
  * @param
  *
  * @return void
  *
  */
  void SetBankboneChannel(bool type) { is_filter_bankbone_channel = type; };
  void SetSwimlaneChannel(bool type) { is_filter_swimlane_channel = type; };
  void SetAllChannel(bool type) { is_all_channels = type; };
  void SetOriginChannel(bool type) { is_origin_cache = type; };

  /**
  * Get服务列表归属的channel标识
  * @param
  *
  * @return void
  *
  */
  bool GetBankboneChannel() { return is_filter_bankbone_channel; };
  bool GetSwimlaneChannel() { return is_filter_swimlane_channel; };
  bool GetAllChannel() { return is_all_channels; };
  bool GetOriginChannel() { return is_origin_cache; };

 private:
  bool is_filter_bankbone_channel;
  bool is_filter_swimlane_channel;
  bool is_all_channels;
  bool is_origin_cache;

 protected:

};

}  //  namespace meituan_mns




#endif //OCTO_OPEN_SOURCE_SERVICE_CHANNELS_H
