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

package com.meituan.octo.mnsc.service

import java.util.concurrent.ConcurrentHashMap

import com.meituan.octo.mnsc.dataCache.{appProviderDataCache, appProviderHttpDataCache}
import com.meituan.octo.mnsc.model.{Env, Path, service}
import com.meituan.octo.mnsc.remote.zk
import com.meituan.octo.mnsc.utils.mnscCommon
import com.meituan.octo.mnsc.model.service.CacheValue
import com.octo.mnsc.idl.thrift.model._
import com.octo.naming.common.thrift.model.SGService
import org.slf4j.{Logger, LoggerFactory}

import scala.collection.JavaConverters._

object mnscService {
  private val LOG: Logger = LoggerFactory.getLogger(mnscService.getClass)

  def getMnsc(req: MnsRequest) = {
    req.getProtoctol match {
      case Protocols.THRIFT =>
        getCache(req, Path.provider.toString, appProviderDataCache.getProviderCache, appProviderDataCache.updateProviderCache)
      case Protocols.HTTP =>
        getCache(req, Path.providerHttp.toString, appProviderHttpDataCache.getProviderHttpCache, appProviderHttpDataCache.updateProviderCache)
      case _ =>
        val result = new MNSResponse
        result.setCode(Constants.ILLEGAL_ARGUMENT)
        result
    }
  }

  private def getCache(req: MnsRequest, providerPath: String, fCache: (String, String, Boolean) => Option[service.CacheValue], fpar: (String, String, String, String, Boolean) => service.CacheValue) = {
    val result = new MNSResponse
    val cacheProviders = fCache(req.appkey, req.env, false)
    val path = s"${mnscCommon.rootPre}/${req.getEnv}/${req.getAppkey}/$providerPath"
    val (zkVersion, _) = zk.getNodeVersion(path)
    cacheProviders match {
      case Some(item) =>
        result.setCode(Constants.SUCCESS)
        if (zk.versionCompare(item.version, zkVersion, true, (arg1: Long, arg2: Long) => arg1 == arg2)) {
          result.setDefaultMNSCache(item.SGServices.asJava)
            .setVersion(item.version)
        } else {
          val (version,_) = zk.getNodeVersion(path)
          val providersUpdate = fpar(version, req.getAppkey, req.getEnv, providerPath, false)
          if (null == providersUpdate) {
            result.setDefaultMNSCache(item.SGServices.asJava)
              .setVersion(item.version)
          } else {
            result.setDefaultMNSCache(providersUpdate.SGServices.asJava)
              .setVersion(providersUpdate.version)
          }
        }
      case None =>
        result.setCode(Constants.NOT_FOUND)
    }
    result
  }

  def getMnsc(appkey: String, version: String, env: String) = {
    val providers = appProviderDataCache.getProviderCache(appkey, env, false)
    getThriftOrHLBCache(providers, appkey, version, env)
  }


  def getMNSCache4HLB(appkey: String, version: String, env: String) = {
    val providers = appProviderHttpDataCache.getProviderHttpCache(appkey, env, false)
    getThriftOrHLBCache(providers, appkey, version, env)
  }

  private def getThriftOrHLBCache(providers: Option[service.CacheValue], appkey: String, version: String, env: String) = {
    val res = new MNSResponse
    providers match {
      case Some(value) =>
        // if input version is smaller than or equal to cache version,
        if (zk.versionCompare(version, value.version, true, (arg1: Long, arg2: Long) => arg1 <= arg2)) {
          res.setCode(Constants.SUCCESS)
            .setDefaultMNSCache(value.SGServices.asJava)
            .setVersion(value.version)
        } else {
          res.setCode(Constants.NOT_MODIFIED)
        }
      case None =>
        LOG.debug(s"localCache don't exist $appkey|$env")
        res.setCodeConstants.NOT_FOUND)

    }
    res
  }

  def getMNSCacheByAppkeys(appkeys: java.util.List[String], protocol: String) = {
    val ret = new MNSBatchResponse()
    ret.cache = new ConcurrentHashMap[String, java.util.Map[String, java.util.List[SGService]]]()
    appkeys.asScala.foreach {
      appkey =>
        Env.values.foreach {
          env =>
            val service = if ("thrift".equalsIgnoreCase(protocol)) {
              appProviderDataCache.getProviderCache(appkey, env.toString, false)
            } else if ("http".equalsIgnoreCase(protocol)) {
              appProviderHttpDataCache.getProviderHttpCache(appkey, env.toString, false)
            } else {
              None
            }
            val serviceItem = service.getOrElse(new CacheValue(s"$appkey|$env", List())).SGServices.asJava
            //            ret.cache.synchronized {
            if (null == ret.cache.get(appkey)) {
              ret.cache.put(appkey, new ConcurrentHashMap[String, java.util.List[SGService]]())
            }
            //            }
            ret.cache.get(appkey).put(env.toString, serviceItem)
        }
    }
    ret
  }

  def getProvidersByIP(ip: String) = {
    LOG.debug(s"[getProviderByIP] Input--> ip=$ip")
    val ret = new MNSResponse()
    try {
      val thriftProviders = appProviderDataCache.getProvidersByIP(ip)
      val httpProviders = appProviderHttpDataCache.getProvidersByIP(ip)
      val list = thriftProviders ::: httpProviders
      ret.setCode(200)
      ret.setDefaultMNSCache(list.asJava)
    } catch {
      case e: Exception =>
        ret.setCode(500)
    }
    ret
  }

  def getAppkeyListByIP(ip: String) = {
    val appkeys = appProviderDataCache.getAppkeysByIP(ip) ++ appProviderHttpDataCache.getAppkeysByIP(ip)
    appkeys.toList.asJava
  }

}
