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

package com.meituan.octo.mnsc.model

import com.octo.naming.common.thrift.model.SGService
import org.apache.commons.lang3.StringUtils
import play.api.libs.json.Json.JsValueWrapper
import play.api.libs.json._
import scala.collection.JavaConverters._


object service {

  case class ServiceDetail(unifiedProto: Int)

  // enabled 0 启用 1 停用   trace 0 关闭 1 开启
  case class ProviderNode(appkey: String, version: String, ip: String, port: Int,
                          weight: Int, fweight: Option[Double], status: Int, role: Int, env: Int,
                          lastUpdateTime: Long, serverType: Option[Int] = Some(0), protocol: Option[String] = Some(""),
                          serviceInfo: Option[Map[String, ServiceDetail]], heartbeatSupport: Option[Int] = Some(0)) {
    override def toString = try {
      val envir = Env(env)
      s"""{"appkey":"${appkey}","version":"${version}","ip":"${ip},"port":${port},"weight":${weight},"fweight":$fweight,"status:":${status},"role":$role,"envir":"${envir}","lastUpdateTime":${lastUpdateTime},"serverType":$serverType,"protocol":"${protocol},"serviceInfo":"${serviceInfo}","heartbeatSupport":"${heartbeatSupport}"}"""
    } catch {
      case e: Exception => ""
    }
  }

  case class AppkeyTs(appkey: String, lastUpdateTime: Long)

  implicit val ApppkeyTsReads = Json.reads[AppkeyTs]
  implicit val ApppkeyTsWrites = Json.writes[AppkeyTs]

  implicit val serviceDetailRepads = Json.reads[ServiceDetail]
  implicit val serviceDetailWrites = Json.writes[ServiceDetail]


  implicit val mapRepads: Reads[Map[String, ServiceDetail]] = new Reads[Map[String, ServiceDetail]] {
    override def reads(json: JsValue): JsResult[Map[String, ServiceDetail]] = JsSuccess {
      json.as[JsObject].value.map {
        case (k, v) => (k, ServiceDetail(
          (v \ "unifiedProto").as[Int]
        ))
      }.toMap
    }
  }
  implicit val mapWrites: Writes[Map[String, ServiceDetail]] = new Writes[Map[String, ServiceDetail]] {
    def writes(map: Map[String, ServiceDetail]): JsValue =
      Json.obj(map.map { case (s, o) =>
        val ret: (String, JsValueWrapper) = s -> Json.toJson(o)
        ret
      }.toSeq: _*)
  }


  case class ProviderDel(appkey: String, protocol: String, ip: String, prot: Int, env: Int)

  case class Provider(appkey: String, lastUpdateTime: Long)

  implicit val providerNodeReads = Json.reads[ProviderNode]
  implicit val providerNodeWrites = Json.writes[ProviderNode]

  implicit val providerReads = Json.reads[Provider]
  implicit val providerWrites = Json.writes[Provider]

  implicit val providerDelReads = Json.reads[ProviderDel]
  implicit val providerDelWrites = Json.writes[ProviderDel]


  case class NodeState(mtime: Long, cversion: Long, version: Long)

  implicit val nodeStateR = Json.reads[NodeState]
  implicit val nodeStateW = Json.writes[NodeState]

  case class CacheValue(version: String, SGServices: List[SGService])

  case class CacheData(version: String, Providers: List[ProviderNode], lastGetTime: Long = System.currentTimeMillis() / 1000)

  // version mtime|cversion|version

  def ProviderNode2SGService(node: ProviderNode) = {

    val service = new SGService()
    service.setAppkey(node.appkey)
      .setVersion(node.version)
      .setIp(node.ip)
      .setPort(node.port)
      .setWeight(node.weight)
      .setStatus(node.status)
      .setRole(node.role)
      .setEnvir(node.env)
      .setLastUpdateTime(node.lastUpdateTime.toInt)
      .setFweight(node.fweight.getOrElse(0))
      .setServerType(node.serverType.getOrElse(0))
      .setProtocol(node.protocol.getOrElse(""))
      .setHeartbeatSupport(node.heartbeatSupport.getOrElse(0).toByte)
      .setServiceInfo(ProviderInfo2Info(node).asJava)
  }


  def SGService2ProviderNode(service: SGService) = {
    val serviceInfo = if (null == service.serviceInfo) {
      Some(Map[String, ServiceDetail]())
    } else {
      Some(SGServiceInfo2Info(service))
    }
    val protocolTemp = if (StringUtils.isEmpty(service.protocol)) None else Some(service.protocol)

    ProviderNode(service.appkey,
      service.version,
      service.ip,
      service.port,
      service.weight,
      Some(service.fweight),
      service.status,
      service.role,
      service.envir,
      service.lastUpdateTime,
      Some(service.serverType),
      protocolTemp,
      serviceInfo = serviceInfo,
      heartbeatSupport = Some(service.heartbeatSupport & 0xff)
    )
  }

  private def ProviderInfo2Info(node: ProviderNode) = {
    val info = node.serviceInfo.getOrElse(Map[String, ServiceDetail]())
    info.map {
      item =>
        val value = new com.octo.naming.common.thrift.model.ServiceDetail()
        value.setUnifiedProto(1 == item._2.unifiedProto)
        (item._1 -> value)
    }
  }

  private def SGServiceInfo2Info(service: SGService) = {
    val serviceInfo = service.serviceInfo.asScala
    val info = scala.collection.mutable.Map[String, ServiceDetail]()
    serviceInfo.foreach { e =>
      info.put(e._1, ServiceDetail(if (e._2.isUnifiedProto) 1 else 0))
    }
    info.toMap
  }
}
