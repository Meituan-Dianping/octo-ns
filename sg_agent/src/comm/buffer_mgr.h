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

#ifndef __BUFFER_MGR_H__
#define __BUFFER_MGR_H__

#include <map>
#include "inc_comm.h"
#include "../util/base_errors_consts.h"

namespace meituan_mns {

#define STAT_STOPPED 0
#define STAT_RUNNING 1
#define RETRYTIMES 2
#define CYCLE_TIME 10

template<class type>
class BufferMgr {
 public:

  BufferMgr();

  /*
  * 获取缓存信息
  * key：缓存对应的键值
  */
  int get(const std::string key, type &val);

  /*
   * 获取缓存信息
   **/
  int get(const std::string key, boost::shared_ptr<type> ptr);

  /*
  * 为更新列表插入新的key
  */
  int insert(const std::string key, const type &val);

  /*
  * 从更新列表删除key，及对应的value
  * 线程不安全，慎用
  */
  int del(const std::string key);

  /*
   * 获取size
   */
  int size();
  int GetKeyList(std::vector<std::string> &keys);

  ~BufferMgr();

 private:

  void _clear();

  std::map<std::string, type> *m_data_map;

  pthread_rwlock_t rwlock;
};

template<class type>
BufferMgr<type>::BufferMgr() {
  
	m_data_map = new std::map<std::string, type>();
  pthread_rwlock_init(&rwlock, NULL);
}

/*
 * 获取key对应的val
 * ret：0表示取得数值， -1表示没有对应key
 */
template<class type>
int BufferMgr<type>::get(const std::string key, type &val) {

	if (key.empty()) {
    return ERR_BUFFERMGR_EMPTYKEY;
  }

 if(NULL==m_data_map){
			return ERR_BUFFERMGR_EMPTYKEY;
	}	 
  int ret = 0;
  // 防止多线程不安全， iter置空的情况

  pthread_rwlock_rdlock(&rwlock);
  typename std::map<std::string, type>::iterator iter;
  if(key.empty()){
   pthread_rwlock_unlock(&rwlock);
	 return -1;
	}
	iter = m_data_map->find(key);
  if (m_data_map->end() != iter) {
    if(NULL==m_data_map){
     pthread_rwlock_unlock(&rwlock);
	 return -1;
	}
	val = m_data_map->at(key);
    ret = 0;
  } else {
    ret = -1;
  }
  pthread_rwlock_unlock(&rwlock);
  return ret;
}

template<class type>
int BufferMgr<type>::insert(const std::string key, const type &val) {
  if (NULL == m_data_map) {
    return ERR_BUFFERMGR_BUFHEAD_NULL;
  }
	pthread_rwlock_wrlock(&rwlock);
	typename std::map<std::string, type>::iterator iter;
  iter = m_data_map->find(key);
  if (m_data_map->end() != iter) {
    //如果map中已经存在，则只替换val
    iter->second = val;
  } else {
    //如果map中不存在，则insert pair
    m_data_map->insert(std::pair<std::string, type>(key, val));
  }
	pthread_rwlock_unlock(&rwlock);
  return 0;
}

template<class type>
int BufferMgr<type>::size() {
  if (NULL == m_data_map) {
    return 0;
  }
  int size = 0;
  pthread_rwlock_rdlock(&rwlock);
  size = m_data_map->size();
  pthread_rwlock_unlock(&rwlock);
  return size;
}

/**
 * return   0: succeed to delete or map don't has this key
 *          other: error
 */
template<class type>
int BufferMgr<type>::del(std::string key) {
  if (NULL == m_data_map) {
    return ERR_BUFFERMGR_BUFHEAD_NULL;
  }
  pthread_rwlock_wrlock(&rwlock);
	typename std::map<std::string, type>::iterator iter;
  iter = m_data_map->find(key);
  if (m_data_map->end() != iter) {
    m_data_map->erase(iter);
	}
  pthread_rwlock_unlock(&rwlock);
  return 0;
}

/*
 * 析构函数
 * 清理map数据
 *
 */
template<class type>
BufferMgr<type>::~BufferMgr() {
  SAFE_DELETE(m_data_map);
	pthread_rwlock_destroy(&rwlock);
}

/*
 * clear map
 */
template<class type>
void BufferMgr<type>::_clear() {
  m_data_map->clear();
}
/*
 * get keylist
 * */
template<class type>
int BufferMgr<type>::GetKeyList(std::vector<std::string> &keys) {
  if (NULL == m_data_map) {
    return ERR_BUFFERMGR_BUFHEAD_NULL;
  }
  pthread_rwlock_rdlock(&rwlock);
  typename std::map<std::string, type>::iterator iter;
  for (iter = m_data_map->begin(); iter != m_data_map->end(); iter++) {
    keys.push_back(iter->first);
  }
  pthread_rwlock_unlock(&rwlock);

  return keys.size();
}

} //namespace sg_buffermgr
#endif
