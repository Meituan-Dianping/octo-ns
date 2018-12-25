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

package com.meituan.octo.mnsc.utils

import com.meituan.octo.mnsc.remote.zk
import org.apache.commons.lang3.StringUtils
import com.meituan.octo.mns.util.ProcessInfoUtil
import org.slf4j.{Logger, LoggerFactory}

import scala.collection.JavaConverters._


object mnscCommon {
  private val LOG: Logger = LoggerFactory.getLogger(mnscCommon.getClass)

  val rootPre = "/octo/nameservice"

  val zookeeperHost = "10.24.41.248:2188,10.24.41.248:2188,10.24.41.248:2188"
  //10.20.63.232:2181,10.21.128.208:2181,10.20.60.152:2181

  //provider线程第一执行delay时间模值
  val initDelay4Provider = 100
  //provider弱同步时间间隔
  val renewInterval4Provider = 20

  //provider强同步时间间隔为renewInterval4Provider * forceBorder4Provider
  val forceBorder4Provider = 120

  //provider-http弱同步时间间隔
  val renewInterval4ProviderHttp = 20
  //20秒
  //provider-http强同步时间间隔renewInterval4ProviderHttp * forceBorder4ProviderHttp
  val forceBorder4ProviderHttp = 120


  //provider线程第一执行delay时间模值
  val initDelay4HttpProperties = 3000

  //每个zk节点建立的zkClient数量, mns-zk对单台主机的连接数做了限制
  val singleHostCount4ZK = 2

  private var appkeys = List[String]()

  private val ENV = ProcessInfoUtil.getAppEnv.toLowerCase
  private val appkeysPath = s"$rootPre/$ENV"

  private def getAppFromZK() = {
    val testAppkeys = System.getProperty("mnscCacheLoadAppkeys4Test")
    val remoteAppkeys =if(StringUtils.isNotEmpty(testAppkeys)){
      testAppkeys.trim.split(",").toList
    }else{
      zk.children(appkeysPath).toList
    }

    if (remoteAppkeys.nonEmpty) {
      appkeys = remoteAppkeys
    }
    appkeys
  }

  def allApp() = {
    getAppFromZK()
  }

  def allAppkeysList() = {
    getAppFromZK().asJava
  }

  def allAppkeys(isPar: Boolean) = {
    if (isPar) {
      mnscCommon.allApp().par
    } else {
      mnscCommon.allApp()
    }
  }
}
