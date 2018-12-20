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

package com.meituan.octo.mnsc.zkWatcher

import com.meituan.octo.mnsc.dataCache.{appProviderDataCache, appProviderHttpDataCache}
import com.meituan.octo.mnsc.model.{Env, Path}
import com.meituan.octo.mnsc.remote.zk
import com.meituan.octo.mnsc.utils.mnscCommon
import org.apache.curator.framework.api.CuratorWatcher
import org.apache.zookeeper.WatchedEvent
import org.apache.zookeeper.Watcher.Event.{EventType, KeeperState}
import org.slf4j.{Logger, LoggerFactory}

object appProviderWatcher {
  private val LOG: Logger = LoggerFactory.getLogger(appProviderWatcher.getClass)
  private val pre = mnscCommon.rootPre
  private val prodPath = s"$pre/${Env.prod}"
  private var apps = mnscCommon.allApp()

  //watch provider节点下的服务节点是否有变更
  class ProviderWatcher(appkey: String, env: String) extends CuratorWatcher {
    def process(event: WatchedEvent): Unit = {
      LOG.info(s"ProviderWatcher event $event for appKey|env = $appkey|$env")
      val watcherPath = s"$pre/$env/$appkey/${Path.provider}"

      event.getType match {
        case EventType.NodeDataChanged =>
          val watcherPath = event.getPath
          val (_, mtime) = zk.getNodeVersion(watcherPath)
          appProviderDataCache.mnscWatcherAction(appkey, env, Path.provider.toString)

          val interval = System.currentTimeMillis - mtime
          LOG.debug(s"ProviderWatcher process for appKey|env = $appkey|$env -->  cost ${interval} ms")
        case _ => //do nothing
      }

      /*避免reconnect时watcher instance 指数级增加，导致的OOM问题
        同时，加快新session中Watcher的playback速度，提高网络抖动后自愈的速度
       */
      if(EventType.NodeDataChanged == event.getType || KeeperState.Expired == event.getState){
        zk.addDataWatcher(watcherPath, new ProviderWatcher(appkey, env))
      }
    }
  }

  //watch provider-http节点下的服务节点是否有变更
  class ProviderHttpWatcher(appkey: String, env: String) extends CuratorWatcher {
    def process(event: WatchedEvent): Unit = {
      LOG.info(s"ProviderHttpWatcher event $event for appKey|env = $appkey|$env")

      val watcherPath = s"$pre/$env/$appkey/${Path.providerHttp}"

      event.getType match {
        case EventType.NodeDataChanged =>
          val watcherPath = event.getPath
          val (_, mtime) = zk.getNodeVersion(watcherPath)
          appProviderHttpDataCache.mnscWatcherAction(appkey, env, Path.providerHttp.toString)

          val interval = System.currentTimeMillis - mtime
          LOG.debug(s"ProviderHttpWatcher process for appKey|env = $appkey|$env -->  cost ${interval} ms")
        case _ => //do nothing
      }

      /*避免reconnect时watcher instance 指数级增加，导致的OOM问题
        同时，加快新session中Watcher的playback速度，提高网络抖动后自愈的速度
       */
      if(EventType.NodeDataChanged == event.getType || KeeperState.Expired == event.getState){
        zk.addDataWatcher(watcherPath, new ProviderHttpWatcher(appkey, env))
      }
    }
  }

  //watch是否有appKey的新增或删除
  class appWatcher(path: String) extends CuratorWatcher {
    def process(event: WatchedEvent): Unit = {
      LOG.info(s"appWatcher event $event for path=$path")

      event.getType match {
        case EventType.NodeChildrenChanged =>
          val newApps = mnscCommon.allApp()
          LOG.info(s"new apps $newApps")
          newApps.filter(!apps.contains(_)).foreach {
            appkey => registryProviderWatcher4AllEnv(appkey)
          }

          appProviderDataCache.deleteNonexistentAppKey()
          appProviderHttpDataCache.deleteNonexistentAppKey()

          apps = newApps

        case _ => //do nothing
      }

      if(EventType.NodeDataChanged == event.getType ||
        EventType.NodeChildrenChanged == event.getType ||
        KeeperState.Expired == event.getState) {
        zk.addChildrenWatcher(path, new appWatcher(path))
      }

    }
  }

  def registryProviderWatcher4AllEnv(appkey: String) = {
    Env.values.foreach {
      env =>
        val providerWatcherPath = s"$pre/$env/$appkey/${Path.provider}"
        zk.addDataWatcher(providerWatcherPath, new ProviderWatcher(appkey, env.toString))
        val providerHTTPWatcherPath = s"$pre/$env/$appkey/${Path.providerHttp}"
        zk.addDataWatcher(providerHTTPWatcherPath, new ProviderHttpWatcher(appkey, env.toString))
    }
  }

  def initWatcherApp() = {
    val watcherPath = prodPath
    zk.addChildrenWatcher(watcherPath, new appWatcher(prodPath))
  }

  def initWatcherProvider() = {
    LOG.info("init provider watcher")
    apps.par.foreach {
      appkey =>
        registryProviderWatcher4AllEnv(appkey)
    }
  }

  def initWatcher() = {
    initWatcherApp()
    initWatcherProvider()
  }
}
