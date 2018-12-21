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

import com.meituan.octo.mnsc.model
import com.meituan.octo.mnsc.model.Env
import com.meituan.octo.mnsc.remote.zk
import com.meituan.octo.mnsc.utils.mnscCommon
import com.meituan.octo.mnsc.model.service.{CacheValue, ProviderNode}
import com.octo.naming.common.thrift.model.SGService
import org.apache.commons.lang.StringUtils
import org.joda.time.DateTime
import org.slf4j.LoggerFactory
import play.api.libs.json.Json

import scala.collection.JavaConverters._


abstract class providerBaseCache {
  private val LOG = LoggerFactory.getLogger(this.getClass)

  def updateProviderCache(providerVersion: String, appkey: String, env: String, providerPath: String, isPar: Boolean = false) = {
    val path = s"${mnscCommon.rootPre}/$env/$appkey/$providerPath"
    if (StringUtils.isNotEmpty(providerVersion)) {
      try {
        val nodeList = zk.client().getChildren.forPath(path).asScala
        val result = scala.collection.mutable.ArrayBuffer[SGService]()
        val parList = if (isPar) {
          nodeList.par
        } else {
          nodeList
        }

        val ipToAppkey = getIpInfoCache()
        parList.foreach(
          node => {
            val nodePath = s"$path/$node"
            val data = zk.client.getData.forPath(nodePath)
            val dataUTF8 = if (null == data) "" else new String(data, "utf-8")
            if (dataUTF8.nonEmpty) {
              Json.parse(dataUTF8).validate[ProviderNode].asOpt match {
                case Some(x) =>
                  val service = model.service.ProviderNode2SGService(x)
                  result.synchronized {
                    result += service
                  }

                  if (service.ip.nonEmpty) {
                    val v = if (ipToAppkey.keySet.contains(service.ip)) {
                      ipToAppkey(service.ip) + service.appkey
                    } else {
                      scala.collection.Set[String](service.appkey)
                    }

                    ipToAppkey.update(service.ip, v)
                  }
                case _ => //do nothing
              }
            }
          }
        )
        val currentCache = getCache(appkey)
        val cacheValue = CacheValue(providerVersion, result.toList)
        currentCache.update(getCacheKey(appkey, env), cacheValue)
        cacheValue
      } catch {
        case e: Exception =>
          LOG.error(s"fail to update cache, appkey=$appkey env=$env", e)
          null
      }
    } else {
      null
    }
  }

  protected def getCache(appkey: String): scala.collection.concurrent.Map[String, CacheValue]

  protected def getCache(): scala.collection.concurrent.Map[String, CacheValue]

  protected def getIpInfoCache(): scala.collection.concurrent.Map[String, scala.collection.Set[String]]

  protected def getCacheKey(appkey: String, env: String) = s"$appkey|$env"

  //若在providerCache中发现不存在的appkey，删除之
  def deleteNonexistentAppKey() = {
    val cache = getCache()
    val newAppkeys = mnscCommon.allApp()
    val cacheAppkeys = getAppkeysByCacheKeySet(cache.keySet)
    LOG.info(s"[deleteNonexistentAppKey] providerCache=${cache.keySet.size} cacheAppkeys=${cacheAppkeys.size} newAppkeys=${newAppkeys.size}")

    cacheAppkeys.toList.filter(!newAppkeys.contains(_)).foreach {
      appkey =>
        Env.values.foreach {
          env =>
            cache.remove(s"$appkey|$env")
            LOG.info(s"[deleteNonexistentAppKey] providerCache delete $appkey|$env")
        }
    }
  }

  //定时任务强同步pull provider
  def renewAllProviderForce(providerPathStr: String, isPar: Boolean) = {
    val apps = mnscCommon.allAppkeys(isPar)
    val start = new DateTime().getMillis
    Env.values.foreach {
      env =>
        apps.foreach {
          appkey =>
            val path = s"${mnscCommon.rootPre}/$env/$appkey/$providerPathStr"
            val (version, _) = zk.getNodeVersion(path)
            if (null == version) {
              LOG.error(s"renewAllProviderForce failed, $path don't exist")
            } else {
              updateProviderCache(version, appkey, env.toString, providerPathStr)
            }
        }
    }
    val end = new DateTime().getMillis
    LOG.info(s"renewProviderForce--> path=${providerPathStr} apps.length=${apps.length}  cost ${end - start}")

  }

  private def getAppkeysByCacheKeySet(appkeySets: scala.collection.Set[String]) = appkeySets.map(x => x.stripSuffix("|prod").stripSuffix("|stage").stripSuffix("|test"))

  //定时任务弱同步pull provider
  protected def renewAllProvider(cache: scala.collection.concurrent.Map[String, CacheValue], providerPathStr: String) = {
    val apps = mnscCommon.allApp().par
    apps.foreach {
      appkey =>
        Env.values.foreach {
          env =>
            val providers = cache.get(getCacheKey(appkey, env.toString))
            providers match {
              case Some(value) =>
                val path = s"${mnscCommon.rootPre}/$env/$appkey/$providerPathStr"
                val (version, _) = zk.getNodeVersion(path)
                if (null == version) {
                  LOG.warn(s"renewAllProvider failed, $path don't exist")
                } else {
                  if (version != value.version) {
                    updateProviderCache(version, appkey, env.toString, providerPathStr)
                  }
                }
              case None =>
                LOG.debug(s"reload providerCache is empty: $appkey|$env")
            }
        }
    }
  }


  //watcher触发执行动作
  def mnscWatcherAction(appkey: String, env: String, providerPathStr: String) = {
    val providers = getCache(appkey).get(getCacheKey(appkey, env))
    val path = s"${mnscCommon.rootPre}/$env/$appkey/$providerPathStr"
    val (version, _) = zk.getNodeVersion(path)
    providers match {
      case Some(value) =>
        if (version != value.version) {
          updateProviderCache(version, appkey, env, providerPathStr)
        }
      case None =>
        updateProviderCache(version, appkey, env, providerPathStr)
    }
  }

  def getAppkeyListByIP(ip: String) = {
    val cache = getCache()
    val keySet = cache.filter(_._2.SGServices.foldLeft(false) { (result, item) =>
      result || StringUtils.equals(ip, item.ip)
    }).keySet
    getAppkeysByCacheKeySet(keySet)
  }

  def getAppkeysByIP(ip: String) = {
    getIpInfoCache().getOrElse(ip, Nil)
  }
}
