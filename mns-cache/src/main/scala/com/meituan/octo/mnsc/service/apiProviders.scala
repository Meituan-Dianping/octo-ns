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

import scala.collection.JavaConverters._
import com.meituan.octo.mnsc.dataCache.appProvidersCommCache
import com.meituan.octo.mnsc.model.{Env, service}
import com.meituan.octo.mnsc.model.service.{Provider, ProviderNode}
import com.meituan.octo.mnsc.remote.zk
import com.meituan.octo.mnsc.utils.{api, zkCommon}
import com.octo.naming.common.thrift.model.SGService
import org.slf4j.{Logger, LoggerFactory}
import play.api.libs.json.Json

object apiProviders {
  private val LOG: Logger = LoggerFactory.getLogger(apiProviders.getClass)


  def getProviders(appkey: String, env: Int, protocol: String) = {
    val services = appProvidersCommCache.getProviderCache(appkey, env, protocol)
    if (services.Providers.isEmpty) api.errorJson(404, s"can't find appkey:$appkey env:$env protocol:$protocol") else api.dataJson(200, Map {
      "serviceList" -> services.Providers
    })
  }

  def postProviders(providers: java.util.List[SGService]) = {
    providers.asScala.map {
      svr =>
        if (null == svr.getVersion) {
          // version cannot be null
          svr.setVersion("")
        }
        val item = service.SGService2ProviderNode(svr)
        val protocol = getProtocol(item)
        val providerPath = zkCommon.getProtocolPath(item.appkey, Env(item.env).toString, protocol)
        val nodePath = s"${providerPath}/${item.ip}:${item.port}"
        val nodeData = Json.prettyPrint(Json.toJson(item))
        val provider = Provider(item.appkey, item.lastUpdateTime)
        val providerData = Json.prettyPrint(Json.toJson(provider))
        if (!zk.exist(providerPath)) {
          val msg = Map("appkey" -> item.appkey, "ip" -> item.ip, "port" -> item.port)
          Map("ret" -> 404, "msg" -> msg)
        }
        else if (zk.exist(nodePath)) {
          zk.client.inTransaction().setData().forPath(nodePath, nodeData.getBytes("utf-8")).and().setData().forPath(providerPath, providerData.getBytes("utf-8")).and().commit()
          Map("ret" -> 200, "msg" -> "success")
        }
        else {
          zk.client.inTransaction().create().forPath(nodePath, nodeData.getBytes("utf-8")).and().setData().forPath(providerPath, providerData.getBytes("utf-8")).and().commit()
          Map("ret" -> 200, "msg" -> "success")
        }
    }
  }


  def deleteProviders(providers: java.util.List[SGService]) = {
    providers.asScala.map {
      svr =>
        val item = service.SGService2ProviderNode(svr)
        val protocol = getProtocol(item)
        val providerPath = zkCommon.getProtocolPath(item.appkey, Env(item.env).toString, protocol)
        val nodePath = s"${providerPath}/${item.ip}:${item.port}"
        val provider = Provider(item.appkey, item.lastUpdateTime)
        val providerData = Json.prettyPrint(Json.toJson(provider))
        if (!zk.exist(providerPath) || !zk.exist(nodePath)) {
          val msg = Map("appkey" -> item.appkey, "ip" -> item.ip, "port" -> item.port)
          Map("ret" -> 404, "msg" -> msg)
        }
        else {
          zk.client.inTransaction().delete().forPath(nodePath).and().setData().forPath(providerPath, providerData.getBytes("utf-8")).and().commit()
          Map("ret" -> 200, "msg" -> "success")
        }
    }
  }

  def getProtocol(item: ProviderNode) = {
    item.protocol.getOrElse(if (Some(1) == item.serverType) "http" else "thrift")
  }

}
