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
import com.meituan.octo.mnsc.model.service.CacheValue
import com.meituan.octo.mnsc.remote.zk
import com.meituan.octo.mnsc.utils.mnscCommon
import org.joda.time.DateTime
import org.slf4j.{Logger, LoggerFactory}

import scala.collection.JavaConverters._
import scala.collection.concurrent.Map

object appProviderHttpDataCache extends providerBaseCache {
  private val LOG: Logger = LoggerFactory.getLogger(appProviderHttpDataCache.getClass)
  private val pre = mnscCommon.rootPre

  //服务的provider-http缓存，key为"appkey|env"
  private val providerHttpCache = new ConcurrentHashMap[String, CacheValue]() asScala

  private val ipInfoCache = new ConcurrentHashMap[String, scala.collection.Set[String]]() asScala

  private var pullCount4ProviderHttp = 0

  private val scheduler = Executors.newScheduledThreadPool(1)

  //周期进行强同步和弱同步
  def doRenew() = {
    val now = System.currentTimeMillis() / mnscCommon.initDelay4HttpProperties
    val init = 60 - (now % 60)
    LOG.info(s"init doRenew on $now with delay $init")

    //provider-http定时同步任务
    scheduler.scheduleAtFixedRate(new Runnable {
      def run(): Unit = {
        try {
          if (0 == pullCount4ProviderHttp) {
            //For provider-http
            LOG.info("renewAllProviderHTTPForce start")
            val startProviderHTTP = new DateTime().getMillis
            renewAllProviderForce(Path.providerHttp.toString, true)
            val endProviderHTTP = new DateTime().getMillis
            LOG.info(s"renewAllProviderHTTPForce cost ${endProviderHTTP - startProviderHTTP}")
            //逐出被删除的appKey
            deleteNonexistentAppKey()
          } else {
            LOG.info("renewAllProviderHTTP start")
            val startProviderHTTP = new DateTime().getMillis
            renewAllProvider(providerHttpCache, Path.providerHttp.toString)
            val endProviderHTTP = new DateTime().getMillis
            LOG.info(s"renewAllProviderHTTP cost ${endProviderHTTP - startProviderHTTP}")
          }
          pullCount4ProviderHttp = (pullCount4ProviderHttp + 1) % mnscCommon.forceBorder4ProviderHttp
        } catch {
          case e: Exception => LOG.error(s"renew localCache fail.", e)
        }
      }
    }, init, mnscCommon.renewInterval4ProviderHttp * 6, TimeUnit.SECONDS)

    scheduler.scheduleWithFixedDelay(new Runnable {
      def run(): Unit = {
        try {
          LOG.info(s"renewHTTP ip to appkeys map start.")
          val startProvider = System.currentTimeMillis()

          ipInfoCache.foreach {
            item =>
              val removeAppkey = scala.collection.mutable.ArrayBuffer[String]()
              item._2.foreach {
                app =>
                  val isExists = Env.values.exists{
                    env =>
                      val key = s"$app|$env"
                      providerHttpCache.keySet.contains(key) && providerHttpCache(key).SGServices.exists(_.ip == item._1)
                  }
                  if (!isExists) {
                    removeAppkey += app
                  }
              }
              val newCache = item._2 -- removeAppkey
              ipInfoCache.update(item._1, newCache)
          }
          val endProvider = System.currentTimeMillis()
          LOG.info(s"renewHTTP ip to appkeys map start cost ${endProvider - startProvider}")
        } catch {
          case e: Exception => LOG.error(s"renewHTTP ip to appkeys map start fail.", e)
        }
      }
    }, init+1, 10, TimeUnit.MINUTES)
  }

  //获取provider-http cache
  def getProviderHttpCache(appkey: String, env: String, isSearchZK: Boolean) = {
    val providers = providerHttpCache.get(getCacheKey(appkey, env))
    if (providers.isEmpty && isSearchZK) {
      LOG.debug(s"providerHttpCache don't exist $appkey|$env")
      val (version, _) = zk.getNodeVersion(s"${mnscCommon.rootPre}/$env/$appkey/${Path.providerHttp.toString}")
      updateProviderCache(version, appkey, env, Path.providerHttp.toString)
      providerHttpCache.get(s"$appkey|$env")
    } else {
      providers
    }
  }

  def getProvidersByIP(ip: String) = {
    val list = providerHttpCache.flatMap(_._2.SGServices.filter(_.ip.equals(ip)))
    list.foreach(_.setServerType(ServerType.http.id))
    list.toList
  }

  override protected def getCache(appkey: String): Map[String, CacheValue] = providerHttpCache

  override protected def getCache() = providerHttpCache

  override protected def getIpInfoCache(): Map[String, scala.collection.Set[String]] = ipInfoCache
}


