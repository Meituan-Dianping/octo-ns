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

#include "zk_client.h"
#include "inc_comm.h"
#include "base_consts.h"

using namespace meituan_mns;

const int kZkContentSize = 1024;
const int MAX_BUF_SIZE = 1024;

ZkClient *ZkClient::zk_client_ = NULL;

ZkClient::ZkClient() : zk_server_(""),
                       zk_timeout_(0),
                       rand_try_times_(0),
                       zk_retrytimes_(0),
                       zk_handle_(NULL) {

}

ZkClient::~ZkClient() {
  ZkClose();
}

void ZkClient::Destroy() {
  SAFE_DELETE(zk_client_);
}

int ZkClient::ZkInit(ZkClient *zk_client, const std::string &zk_server, const int zk_timeout, const int zk_retrytimes) {
  zk_client_ = zk_client;
  zk_server_ = zk_server;
  zk_timeout_ = zk_timeout;
  zk_retrytimes_ = zk_retrytimes;
  zoo_set_debug_level(ZOO_LOG_LEVEL_WARN);

  int ret = Connect2Zk();
  return ret;
}

int ZkClient::Connect2Zk() {
  //close old zk_connect before establish zk-connection
  ZkClose();

  int count = 0;
  //避免跟zookeeper的自定义的状态重合
  int state = 888;
  //默认初始值
  int delay_time = 1;
  do {
    NS_LOG_WARN("start to connect ZK, count : " << count);
    if (zk_handle_ == NULL) {
      RandSleep();
      zk_handle_ = zookeeper_init(zk_server_.c_str(), NULL, zk_timeout_, 0, NULL, 0);
      if (zk_handle_ == NULL || errno == EINVAL) {
        NS_LOG_ERROR("zookeeper_init failed. retrying in 1 second. "
                           "serverList: " << zk_server_
                                          << ", errno = " << errno
                                          << ", timeout = " << zk_timeout_
                                          << ", count = " << count);
      } else {
        NS_LOG_INFO("zookeeper_init success. serverList: " << zk_server_);
      }
    }
    //established，sleep 1s
    sleep(delay_time);
    delay_time = delay_time * 2;

    //check状态
    state = zoo_state(zk_handle_);
    if (state == ZOO_CONNECTING_STATE) {
      //sleep 5ms,then check zk_status
      usleep(5000);
      state = zoo_state(zk_handle_);
    }
  } while (state != ZOO_CONNECTED_STATE && count++ < zk_retrytimes_ );

  if (state != ZOO_CONNECTED_STATE) {
    NS_LOG_ERROR("zookeeper_init failed, please check zk_server. state = " << state);
  }

  return state == ZOO_CONNECTED_STATE ? 0 : -2;
}

int ZkClient::ZkClose() {
  int ret = 0;
  if (NULL != zk_handle_) {
    ret = zookeeper_close(zk_handle_);
    if (0 != ret) {
      NS_LOG_ERROR("Failed to close zk connection! ret = " << ret );
    }
    zk_handle_ = NULL;
  }
  return ret;
}

int ZkClient::CheckZkConn() {
  int count = 0;
  int ret = 0;
  int state = zoo_state(zk_handle_);
  do {
    if (ZOO_CONNECTED_STATE == state) {
      break;
    } else if (ZOO_CONNECTING_STATE == state) {
      //if connection is establishing, sleep 50ms, then check
      NS_LOG_WARN("WARN zk connection: ZOO_CONNECTING_STATE!");
      usleep(50000);
    } else {
      NS_LOG_ERROR("ERR zk connection lost! zk state = " << state);
      ret = -1;
    }
    state = zoo_state(zk_handle_);
    count++;
  } while (state != ZOO_CONNECTED_STATE && count < zk_retrytimes_);

  return ret;
}

void ZkClient::ConnWatcher(zhandle_t *zh, int type, int state,
                           const char *zk_path, void *watcher_ctx) {
  if (ZOO_CONNECTED_STATE == state) {
    NS_LOG_INFO("connWatcher() ZOO_CONNECTED_STATE = " << state
                                                         << ", type " << type);
  } else if (ZOO_AUTH_FAILED_STATE == state) {
    NS_LOG_ERROR("connWatcher() ZOO_AUTH_FAILED_STATE = " << state
                                                            << ", type " << type);
  } else if (ZOO_EXPIRED_SESSION_STATE == state) {
    NS_LOG_ERROR("connWatcher() ZOO_EXPIRED_SESSION_STATE = " << state
                                                                << ", type " << type);
    zk_client_->Reconnect2Zk();
  } else if (ZOO_CONNECTING_STATE == state) {
    NS_LOG_ERROR("connWatcher() ZOO_CONNECTING_STATE = " << state
                                                           << ", type " << type);
  } else if (ZOO_ASSOCIATING_STATE == state) {
    NS_LOG_ERROR("connWatcher() ZOO_ASSOCIATING_STATE = " << state
                                                            << ", type " << type);
  }
}

int ZkClient::Reconnect2Zk() {
  int retry_time = 0;
  int ret = 0;
  while (0 != CheckZkConn() && zk_retrytimes_ > retry_time++) {
    RandSleep();
    ret = Connect2Zk();
    if (0 != ret) {
      NS_LOG_ERROR("Failed to connect zk! retry_time = " << retry_time);
    }
  }
  return ret;
}

void ZkClient::RandSleep() {
  srand(time(0));
  unsigned int rand_usleep_time = 1000;
  rand_try_times_ = rand_try_times_ > 3 ? 0 : rand_try_times_;
  switch (rand_try_times_++) {
    case 0 : {
      rand_usleep_time *= rand() % 10;
      break;
    }
    case 1 : {
      rand_usleep_time *= rand() % 20;
      break;
    }
    case 2 : {
      rand_usleep_time *= rand() % 30;
    }
    default : {
      rand_usleep_time *= rand() % 50;
    }
  }
  usleep(rand_usleep_time);
}

int ZkClient::ZkGet(const ZkGetRequestPtr &request_ptr, ZkGetResponsePtr &response_ptr) {
  if (0 != CheckZkConn()) {
    return ERR_ZK_CONNECTION_LOSS;
  }
  int retry = 0;
  int ret = 0;
  struct Stat stat;
  int buffer_len = kZkContentSize;
  boost::shared_array<char> buff_ptr(new char[buffer_len]);
  while (zk_retrytimes_ > retry++) {
    ret = zoo_get(zk_handle_, request_ptr->path.c_str(), request_ptr->watch, buff_ptr.get(), &buffer_len, &stat);
    if (ZOK != ret) {
      NS_LOG_ERROR("Failed to zoo get, zk_path: " << request_ptr->path
                                                    << ", buffer_len: " << buffer_len
                                                    << ", ret = " << ret);
      return ret;
    }
    if (buffer_len != stat.dataLength && 0 != stat.dataLength) {
      NS_LOG_INFO("new larger buffer's size to get zk_get node");
      buff_ptr = boost::shared_array<char>(new char[stat.dataLength]);
      buffer_len = stat.dataLength;
      continue;
    }
    break;
  }
  response_ptr->buffer.assign(buff_ptr.get(), buffer_len);
  response_ptr->buffer_len = buffer_len;
  response_ptr->stat = stat;
  return ret;
}


int ZkClient::ZkWgetChildren(const ZkWGetChildrenRequestPtr &request_ptr, ZkWGetChildrenResponsePtr &response_ptr) {
  if (0 != CheckZkConn()) {
    return ERR_ZK_CONNECTION_LOSS;
  }
  struct String_vector stat;
  stat.count = 0;
  stat.data = 0;
  int ret = zoo_wget_children(zk_handle_, (request_ptr->path).c_str(), request_ptr->watch,
                              request_ptr->watcherCtx, &stat);
  if (ZOK == ret) {
    response_ptr->count = stat.count;
    for (int i = 0; i < stat.count; i++) {
      std::string data = stat.data[i];
      response_ptr->data.push_back(data);
    }
  }
  return ret;
}

int ZkClient::ZkWget(const ZkWGetRequestPtr &request_ptr, ZkWGetResponsePtr &response_ptr) {
  if (0 != CheckZkConn()) {
    return ERR_ZK_CONNECTION_LOSS;
  }

  int retry = 0;
  int ret = 0;
  struct Stat stat;
  int buffer_len = kZkContentSize;
  boost::shared_array<char> buff_ptr(new char[buffer_len]);
  while (zk_retrytimes_ > retry++) {
    ret = zoo_wget(zk_handle_, (request_ptr->path).c_str(), request_ptr->watch, request_ptr->watcherCtx,
                   buff_ptr.get(), &buffer_len, &stat);
    if (ZOK != ret) {
      return ret;
    }

    if (buffer_len != stat.dataLength && stat.dataLength != 0) {
      NS_LOG_INFO("buffer's size is not enough to put zk_wget node");
      buff_ptr = boost::shared_array<char>(new char[stat.dataLength]);
      buffer_len = stat.dataLength;
      continue;
    }
    break;
  }
  response_ptr->buffer.assign(buff_ptr.get(), buffer_len);
  response_ptr->buffer_len = buffer_len;
  response_ptr->stat = stat;
  return ret;
}

int ZkClient::ZkCreate(const ZkCreateRequestPtr &request_ptr) {
  if (0 != CheckZkConn()) {
    return ERR_ZK_CONNECTION_LOSS;
  }
  char path_buffer[MAX_BUF_SIZE] = {0};
  int path_buffer_len = MAX_BUF_SIZE;
  int ret = zoo_create(zk_handle_,
                       request_ptr->path.c_str(),
                       request_ptr->value.c_str(),
                       request_ptr->value_len,
                       &ZOO_OPEN_ACL_UNSAFE /* use ACL of parent */,
                       0 /* persistent node*/,
                       path_buffer,
                       path_buffer_len);
  NS_LOG_INFO("zoo_create ret = "
                    << ret
                    << ", path = "
                    << request_ptr->path);
  return ret;
}

int ZkClient::ZkSet(const ZkSetRequestPtr &request_ptr) {
  if (0 != CheckZkConn()) {
    return ERR_ZK_CONNECTION_LOSS;
  }
  int ret = zoo_set(zk_handle_,
                    request_ptr->path.c_str(),
                    request_ptr->buffer.c_str(),
                    static_cast<int>(request_ptr->buffer.size()),
                    request_ptr->version);
  NS_LOG_INFO("zoo_set ret = "
                    << ret
                    << ", path = "
                    << request_ptr->path);
  return ret;
}

int ZkClient::ZkExists(const ZkExistsRequestPtr &request_ptr) {
  if (0 != CheckZkConn()) {
    return ERR_ZK_CONNECTION_LOSS;
  }
  struct Stat stat;
  int ret = zoo_exists(zk_handle_,
                       request_ptr->path.c_str(),
                       request_ptr->watch,
                       &stat);
  NS_LOG_INFO("zoo_exists ret = "
                    << ret
                    << ", path = "
                    << request_ptr->path);
  return ret;
}
int ZkClient::ZkPathCreateRecursivly(std::string &zk_path) {
  if (0 != CheckZkConn()) {
    return ERR_ZK_CONNECTION_LOSS;
  }
  int32_t ret = FAILURE;
	boost::trim(zk_path);
  ZkExistsRequestPtr zk_exists_req(new ZkExistsRequest());
  zk_exists_req->watch = 0;

  ZkCreateRequestPtr zk_data_create(new ZkCreateRequest());
  zk_data_create->path = zk_path;
  zk_data_create->value = "test zk path create";
  zk_data_create->value_len = strlen(zk_data_create->value.c_str());
  std::vector<std::string> path_list;
  int32_t path_len = SplitStringIntoVector(zk_path.c_str(), "/", path_list);
  if(0 >= path_len || path_len >kMaxZkPathDepth){
    NS_LOG_INFO("create path failed, zk_path: "<< zk_path
                                               <<"; path_len:"<<path_len);
    return FAILURE;
  }
  char path_buffer[MAX_BUF_SIZE] = {0};
  int32_t path_buffer_len = MAX_BUF_SIZE;
  int32_t path_pos = 1;
  std::string create_path = "";
  std::string create_path_tmp = "";
  while(path_pos < path_len){
    create_path = create_path_tmp + "/" +  path_list[path_pos];
    zk_exists_req->path = create_path;
    zk_data_create->path = create_path;
    NS_LOG_INFO("create_path zk_path: "<< create_path);
    struct Stat stat;
    if(ZNONODE == zoo_exists(zk_handle_,
                         zk_exists_req->path.c_str(),
                         zk_exists_req->watch,
                         &stat)){

      if(ZOK == zoo_create(zk_handle_,
                       zk_data_create->path.c_str(),
                       zk_data_create->value.c_str(),
                       zk_data_create->value_len,
                       &ZOO_OPEN_ACL_UNSAFE /* use ACL of parent */,
                       0 /* persistent node*/,
                       path_buffer,
                       path_buffer_len)){
        create_path_tmp = create_path;
        NS_LOG_INFO("zoo_create ret = "
                        << ret
                        << ", create_path = "
                        << create_path<<"; create_path_tmp:"<<create_path_tmp);
        path_pos++;
      }else{
        NS_LOG_ERROR("zoo_create failed create_path_tmp = "<<create_path_tmp);
        return FAILURE;
      }

    }else{
      create_path_tmp = create_path;
      path_pos++;
    }
  }
  return ret;
}
