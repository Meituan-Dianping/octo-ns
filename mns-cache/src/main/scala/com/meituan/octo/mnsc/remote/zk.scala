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

package com.meituan.octo.mnsc.remote

import java.util

import com.meituan.octo.mns.util.ProcessInfoUtil
import com.meituan.octo.mnsc.utils.mnscCommon
import org.apache.curator.framework.CuratorFrameworkFactory
import org.apache.curator.framework.api.CuratorWatcher
import org.apache.curator.retry.RetryUntilElapsed
import org.apache.zookeeper.data._
import org.slf4j.{Logger, LoggerFactory}

import scala.collection.JavaConverters._
import scala.util.control.Breaks


object zk {
  private val LOG: Logger = LoggerFactory.getLogger(zk.getClass)
  private val singleHostCount = mnscCommon.singleHostCount4ZK
  private var INDEX = 0
  private val zkList = (1 to singleHostCount).flatMap(x => getZk).toList
  private val zkWatcher = getWatcherZk
  private val zkCount = zkList.length
  private val zkClientLock = new Object()

  private def getZk = {
    getURLCn2.map {
      host =>
        val superClient = CuratorFrameworkFactory.builder.connectString(host).retryPolicy(new RetryUntilElapsed(3000, 2000)).build()
        LOG.info("start zk client with " + host)
        superClient.start()
        superClient
    }
  }

  private def getWatcherZk = {
      val superClient = CuratorFrameworkFactory.builder.connectString(url).retryPolicy(new RetryUntilElapsed(3000, 2000)).build()
      superClient.start()
      superClient
  }

  private def getURLCn2 = {
    val urlsArr = url.split(",")
    val ret = new util.ArrayList[String]
    var x = 0
    while (x < urlsArr.length) {
      var y = x + 1
      while (y < urlsArr.length) {
        ret.add(s"${urlsArr(x)},${urlsArr(y)}")
        y += 1
      }
      x += 1
    }
    if (ret.isEmpty) {
      ret.add(url)
    }
    ret.asScala
  }

  def client() = {
    val loop = new Breaks
    var curIndex = getZkIndex()
    var ret = zkList(curIndex)
    loop.breakable {
      for (i <- 1 to zkCount) {
        ret = zkList(curIndex)
        if (!ret.getZookeeperClient.isConnected) {
          curIndex = getNextIndex(curIndex)
        } else {
          loop.break()
        }
      }
    }
    ret
  }

  private def getNextIndex(curIndex: Int) = (curIndex + 1) % zkCount

  private def getZkIndex() = zkClientLock.synchronized {
    INDEX = getNextIndex(INDEX)
    INDEX
  }

  private def url: String = {
    val uri = ProcessInfoUtil.getMnsZKUrl
    if (uri.isEmpty) {
      LOG.error("Please set zk url in octo.cfg")
      mnscCommon.zookeeperHost
    } else uri
  }

  def getData(path: String): java.lang.String = {
    try {
      val data = client.getData.forPath(path)
      if (data == null) "" else new String(data, "utf-8")
    } catch {
      case e: Exception => {
        LOG.error(s"function zk.getData exception.", e)
        ""
      }
    }
  }

  def children(path: String) = {
    try {
        client.getChildren.forPath(path).asScala
    } catch {
      case e: Exception => {
        LOG.error("function zk.children exception.", e)
        List[String]()
      }
    }
  }

  def exist(path: String): Boolean = {
    try {
      client.checkExists().forPath(path) != null
    } catch {
      case e: Exception => {
        LOG.error("function zk.exist exception.", e)
        false
      }
    }
  }

  private def addWatcher(path: String, watcher: CuratorWatcher, isData: Boolean) = {
      try {
        if (isData) {
          zkWatcher.getData.usingWatcher(watcher).inBackground().forPath(path)
        } else {
          zkWatcher.getChildren.usingWatcher(watcher).inBackground().forPath(path)
        }
        true
      } catch {
        case e: Exception =>
          LOG.error("fail to add watcher.", e)
          false
      }
  }

  def addDataWatcher(path: String, watcher: CuratorWatcher) = {
    addWatcher(path, watcher, true)
  }

  def addChildrenWatcher(path: String, watcher: CuratorWatcher) = {
    addWatcher(path, watcher, false)
  }

  def getNodeState(path: String) = {
    try {
      client.checkExists().forPath(path)
    } catch {
      case e: Exception => {
        LOG.error(s"function zk.getNodeState exception.", e)
        null
      }
    }
  }

  def getNodeVersion(path: String) = {
    val nodeState = getNodeState(path)
    if (null != nodeState) {
      (s"${nodeState.getMtime}|${nodeState.getCversion}|${nodeState.getVersion}", nodeState.getMtime)
    } else {

      (null, -101l)
    }
  }

  def getNodeVersion(nodeState: Stat) = {
    if (null != nodeState) {
      s"${nodeState.getMtime}|${nodeState.getCversion}|${nodeState.getVersion}"
    } else {
      null
    }
  }

  def versionCompare(inputVersion: String, cacheVersion: String, defaultValue: Boolean, f: (Long, Long) => Boolean) = {
    if (null == inputVersion || null == cacheVersion) {
      defaultValue
    } else {
      val inputVersionArray = inputVersion.split("\\|")
      val cacheVersionArray = cacheVersion.split("\\|")
      if (inputVersionArray.length == 3 && cacheVersionArray.length == 3) {
        f(inputVersionArray(2).toLong, cacheVersionArray(2).toLong)
      } else {
        defaultValue
      }
    }

  }
}
