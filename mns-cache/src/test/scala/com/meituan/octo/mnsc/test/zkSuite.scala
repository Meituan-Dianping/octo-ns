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

package com.meituan.octo.mnsc.test

import com.meituan.octo.mnsc.model.Path
import com.meituan.octo.mnsc.remote.zk
import com.meituan.octo.mnsc.service.apiProviders
import com.meituan.octo.mnsc.utils.mnscCommon
import com.meituan.octo.mnsc.zkWatcher.appProviderWatcher
import com.octo.naming.common.thrift.model.SGService
import org.apache.curator.framework.api.CuratorWatcher
import org.apache.zookeeper.WatchedEvent
import org.junit.runner.RunWith
import org.scalatest.{BeforeAndAfter, FunSuite}
import org.scalatest.junit.JUnitRunner

@RunWith(classOf[JUnitRunner])
class zkSuite extends FunSuite with BeforeAndAfter with CuratorWatcher {
  val env = "prod"
  val appkey = "com.sankuai.octo.tmy"
  val path = s"${mnscCommon.rootPre}/$env/$appkey/${Path.provider}"

  val service = new SGService()
  service.setAppkey(appkey)
    .setVersion("thrift")
    .setIp("10.4.229.159")
    .setPort(8830)
    .setWeight(10)
    .setStatus(4)
    .setRole(0)
    .setEnvir(3)
    .setLastUpdateTime(1543395127)
    .setFweight(10.0)
    .setServerType(0)
    .setProtocol("thrift")
    .setHeartbeatSupport(0)

  test("test zk addDataWatcher") {
    zk.addDataWatcher(path, this)
    val services = new java.util.ArrayList[SGService]()
    services.add(service)
    Thread.sleep(1000*5)
    apiProviders.postProviders(services)
    Thread.sleep(1000*5)
  }

  override def process(event: WatchedEvent): Unit = {
    assert(event.getPath == path)
    println(zk.getData(event.getPath))
    zk.addDataWatcher(path, this)
  }


  test ("test zk addChildrenWatcher") {
    val services = new java.util.ArrayList[SGService]()
    services.add(service)
    apiProviders.deleteProviders(services)

    zk.addChildrenWatcher(path,this)
    Thread.sleep(1000*5)
    apiProviders.postProviders(services)
    Thread.sleep(1000*5)
  }

  test ("test version check") {
    assert(zk.versionCompare("0|0|1", "0|0|2", false, (arg1: Long, arg2: Long) => arg1 <= arg2))
    assert(!zk.versionCompare("0|0", "0|0|2", false, (arg1: Long, arg2: Long) => arg1 <= arg2))
    assert(!zk.versionCompare("0|0|1", "0|0", false, (arg1: Long, arg2: Long) => arg1 <= arg2))
    assert(!zk.versionCompare("0|0|3", "0|0|2", true, (arg1: Long, arg2: Long) => arg1 <= arg2))
    assert(zk.versionCompare("0|0", "0|0|2", true, (arg1: Long, arg2: Long) => arg1 <= arg2))
    assert(zk.versionCompare("0|0|3", "0|0", true, (arg1: Long, arg2: Long) => arg1 <= arg2))
  }
}

@RunWith(classOf[JUnitRunner])
class zkProviderSuite extends FunSuite with BeforeAndAfter {
  val appkey = "com.sankuai.octo.tmy"

  val service = new SGService()
  service.setAppkey(appkey)
    .setVersion("thrift")
    .setIp("10.4.229.159")
    .setPort(8830)
    .setWeight(10)
    .setStatus(4)
    .setRole(0)
    .setEnvir(3)
    .setLastUpdateTime(1543395127)
    .setFweight(10.0)
    .setServerType(0)
    .setProtocol("thrift")
    .setHeartbeatSupport(0)

  test ("init watcher for app") {
    appProviderWatcher.initWatcher()

    val services = new java.util.ArrayList[SGService]()
    services.add(service)
    apiProviders.postProviders(services)
    Thread.sleep(1000*3)

    apiProviders.postProviders(services)
    Thread.sleep(1000*2)
  }

  service.setProtocol("http")
  test ("init watcher for http app") {
    appProviderWatcher.initWatcher()

    val services = new java.util.ArrayList[SGService]()
    services.add(service)
    apiProviders.postProviders(services)
    Thread.sleep(1000*3)

    apiProviders.postProviders(services)
    Thread.sleep(1000*2)
  }

  //TODO:watch是否有appKey的新增或删除
}