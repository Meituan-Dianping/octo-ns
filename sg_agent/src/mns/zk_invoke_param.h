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

#ifndef OCTO_OPEN_SOURCE_ZK_PARAM_H
#define OCTO_OPEN_SOURCE_ZK_PARAM_H
#include <vector>
#include <boost/shared_array.hpp>
#include "naming_service_types.h"
#include "unified_protocol_types.h"

extern "C" {
#include <zookeeper/zookeeper.h>
#include "cJSON.h"
}
#include <string.h>
#include <boost/shared_ptr.hpp>
namespace meituan_mns {
typedef struct ZkGetRequest {
  std::string path;
  int watch;
  ZkGetRequest() : path(""), watch(0) {}
} ZkGetRequest;

typedef boost::shared_ptr<ZkGetRequest> ZkGetRequestPtr;

typedef struct ZkGetResponse {
  std::string buffer;
  int buffer_len;
  struct Stat stat;
  ZkGetResponse() : buffer(""), buffer_len(0) {}
} ZkGetResponse;

typedef boost::shared_ptr<ZkGetResponse> ZkGetResponsePtr;

typedef struct ZkWGetRequest {
  std::string path;
  watcher_fn watch;
  void *watcherCtx;
} ZkWGetRequest;

typedef boost::shared_ptr<ZkWGetRequest> ZkWGetRequestPtr;

typedef struct ZkWGetResponse {
  std::string buffer;
  int buffer_len;
  struct Stat stat;
} ZkWGetResponse;

typedef boost::shared_ptr<ZkWGetResponse> ZkWGetResponsePtr;

typedef struct ZkWGetChildrenRequest {
  std::string path;
  watcher_fn watch;
  void *watcherCtx;
} ZkWGetChildrenRequest;

typedef boost::shared_ptr<ZkWGetChildrenRequest> ZkWGetChildrenRequestPtr;

typedef struct ZkWGetChildrenResponse {
  int count;
  std::vector<std::string> data;
} ZkWGetChildrenResponse;

typedef boost::shared_ptr<ZkWGetChildrenResponse> ZkWGetChildrenResponsePtr;

typedef struct ZkCreateRequest {
  std::string path;
  std::string value;
  int value_len;
} ZkCreateRequest;

typedef boost::shared_ptr<ZkCreateRequest> ZkCreateRequestPtr;

typedef struct ZkSetRequest {
  std::string path;
  std::string buffer;
  int version;
} ZkSetRequest;

typedef boost::shared_ptr<ZkSetRequest> ZkSetRequestPtr;

typedef struct ZkExistsRequest {
  std::string path;
  int watch;
} ZkExistsRequest;

typedef boost::shared_ptr<ZkExistsRequest> ZkExistsRequestPtr;

typedef struct ZkCreateInvokeParams {
  ZkCreateRequest zk_create_request;
  int zk_create_response;
} ZkCreateInvokeParams;

typedef struct ZkGetInvokeParams {
  ZkGetRequest zk_get_request;
  ZkGetResponse zk_get_response;
} ZkGetInvokeParams;

typedef struct GetMnsCacheParams {
  std::string appkey;
  std::string version;
  std::string env;
  std::string protocol;
} GetMnsCacheParams;


typedef struct SubGetServiceParams {
  ZkWGetChildrenResponsePtr wg_child_params;
  int32_t index;
  std::string provider_path;
  std::string remote_appkey;
} SubGetServiceParams;

typedef struct SubGetServiceRes {
  std::vector<SGService> srvlist;
  bool is_ok;
  SubGetServiceRes() : is_ok(true) {}
} SubGetServiceRes;
typedef boost::shared_ptr<SubGetServiceRes> SubGetServiceResPtr;
}
#endif  //  OCTO_OPEN_SOURCE_ZK_PARAM_H
