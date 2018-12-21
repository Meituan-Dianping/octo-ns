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

package com.meituan.octo.mnsc.dataCache

import java.util.concurrent.{ConcurrentHashMap, Executors, TimeUnit}

import com.meituan.octo.mnsc.model.{Env, Path, ServerType}
import com.meituan.octo.mnsc.remote.zk
import com.meituan.octo.mnsc.utils.mnscCommon
import com.meituan.octo.mnsc.model.service.CacheValue
import org.slf4j.LoggerFactory

import scala.collection.JavaConverters._
import scala.collection.concurrent.Map

/**
  * ZK provider节点下数据缓存
  */
object appProviderDataCache extends providerBaseCache {
  private val LOG = LoggerFactory.getLogger(appProviderDataCache.getClass)

  //服务的provider缓存，key为"appkey|env"
  private val providerCache = new ConcurrentHashMap[String, CacheValue]() asScala

  private val ipInfoCache = new ConcurrentHashMap[String, scala.collection.Set[String]]() asScala

  private var pullCount4Provider = 0

  private val scheduler = Executors.newScheduledThreadPool(2)

  //获取provider cache
  def getProviderCache(appkey: String, env: String, isSearchZK: Boolean) = {
    val cache = getCache(appkey)
    val providers = cache.get(getCacheKey(appkey, env))
    if (providers.isEmpty && isSearchZK) {
      LOG.debug(s"providerCache don't exist $appkey|$env")
      val (version, _) = zk.getNodeVersion(s"${mnscCommon.rootPre}/$env/$appkey/${Path.provider.toString}")
      updateProviderCache(version, appkey, env, Path.provider.toString)
      cache.get(getCacheKey(appkey, env))

    } else {
      providers
    }
  }

  override protected def getCache(appkey: String) = {
      providerCache
  }

  override protected def getCache(): Map[String, CacheValue] = providerCache

  override protected def getIpInfoCache(): Map[String, scala.collection.Set[String]] = ipInfoCache

  //周期进行强同步和弱同步
  def doRenew() = {
    val now = System.currentTimeMillis() / mnscCommon.initDelay4Provider
    val init = 60 - (now % 60)
    LOG.info(s"init doRenew on $now with delay $init")
    //provider定时同步任务
    scheduler.scheduleAtFixedRate(new Runnable {
      def run(): Unit = {
        try {
          if (0 == pullCount4Provider) {
            LOG.info("renewAllProviderForce start. count = {}", pullCount4Provider + 1)
            val startProvider = System.currentTimeMillis()
            renewAllProviderForce(Path.provider.toString, true)
            val endProvider = System.currentTimeMillis()
            LOG.info("renewAllProviderForce cost {}", endProvider - startProvider)
            //逐出被删除的appKey
            deleteNonexistentAppKey()
          } else {
            LOG.info(s"renewAllProvider start. count = {}", pullCount4Provider + 1)
            val startProvider = System.currentTimeMillis()
            renewAllProvider(providerCache, Path.provider.toString)
            val endProvider = System.currentTimeMillis()
            LOG.info(s"renewAllProvider cost {}", endProvider - startProvider)
          }
          pullCount4Provider = (pullCount4Provider + 1) % mnscCommon.forceBorder4Provider
        } catch {
          case e: Exception => LOG.error(s"renew localCache fail.", e)
        }
      }
    }, init, mnscCommon.renewInterval4Provider * 6, TimeUnit.SECONDS)

    scheduler.scheduleWithFixedDelay(new Runnable {
      def run(): Unit = {
        try {
          LOG.info(s"renew ip to appkeys map start.")
          val startProvider = System.currentTimeMillis()

          ipInfoCache.foreach {
            item =>
              val removeAppkey = scala.collection.mutable.ArrayBuffer[String]()
              item._2.foreach {
                app =>
                  val isExists = Env.values.exists{
                    env =>
                      val key = s"$app|$env"
                      providerCache.keySet.contains(key) && providerCache(key).SGServices.exists(_.ip == item._1)
                  }
                  if (!isExists) {
                    removeAppkey += app
                  }
              }
              val newCache = item._2 -- removeAppkey
              ipInfoCache.update(item._1, newCache)
          }
          val endProvider = System.currentTimeMillis()
          LOG.info(s"renew ip to appkeys map start cost ${endProvider - startProvider}")
        } catch {
          case e: Exception => LOG.error(s"renew ip to appkeys map start fail.", e)
        }
      }
    }, init, 10, TimeUnit.MINUTES)
  }


  def getProvidersByIP(ip: String) = {
    val list = providerCache.flatMap(_._2.SGServices.filter(_.ip.equals(ip)))
    list.foreach(_.setServerType(ServerType.thrift.id))
    list.toList
  }
}