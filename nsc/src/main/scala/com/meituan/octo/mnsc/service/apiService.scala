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

import java.text.SimpleDateFormat
import java.util.Date

import com.meituan.octo.mns.util.IpUtil
import com.meituan.octo.mnsc.dataCache.appProviderDataCache
import com.meituan.octo.mnsc.model.service
import com.meituan.octo.mnsc.model.service.CacheValue
import com.meituan.octo.mnsc.utils.api
import com.octo.naming.common.thrift.model.{SGService, fb_status}
import com.octo.idc.model.Idc
import org.slf4j.{Logger, LoggerFactory}

import scala.collection.JavaConverters._
object apiService {
  private val LOG: Logger = LoggerFactory.getLogger(apiService.getClass)
  private def getTimeMillis(time: String) = {
    val ret = try {
      val dateFormat = new SimpleDateFormat("yy-MM-dd HH:mm:ss");
      val dayFormat = new SimpleDateFormat("yy-MM-dd");
      val curDate = dateFormat.parse(dayFormat.format(new Date()) + " " + time);
      curDate.getTime()
    } catch {
      case e: Exception =>
        //ignore
        0l
    }
    ret
  }
  private def getSamePrefixIP(list: List[SGService], ipIdc: String) = {
    val defaultIdc = new Idc()
    defaultIdc.setIdc("")
    if (ipIdc.nonEmpty) {
      list.filter(x => (IpUtil.getIdcInfoFromLocal(List(x.ip).asJava)).asScala.getOrElse(x.ip, defaultIdc).getIdc == ipIdc)
    } else {
      List()
    }
  }
  private def isListAlive(list: List[SGService]) = {
    list.foldLeft(false) {
      (ret, item) =>
        ret || (item.status == fb_status.ALIVE.getValue)
    }
  }
  private def handleServiceList(list: List[SGService], ip: String) = {
    val defaultIdc = new Idc()
    defaultIdc.setIdc("")
    val localIdc = IpUtil.getIdcInfoFromLocal(List(ip).asJava).asScala.getOrElse(ip, defaultIdc)
    val regionList = list.filter(x => (IpUtil.getIdcInfoFromLocal(List(x.ip).asJava)).asScala.getOrElse(x.ip, defaultIdc).getRegion == localIdc.getRegion)
    val sameIDCList = getSamePrefixIP(regionList, localIdc.getIdc)
    if (isListAlive(sameIDCList)) {
      sameIDCList
    } else if (isListAlive(regionList)) {
      regionList
    } else {
      list
    }
  }
  def getServiceList(appkey: String, env: String, ip: String) = {
    val services = appProviderDataCache.getProviderCache(appkey, env, false).getOrElse(CacheValue("", List()))
    val list = handleServiceList(services.SGServices, ip)
    val retList = if (list.isEmpty) {
      List()
    } else {
      list.map {
        item =>
          service.SGService2ProviderNode(item)
      }
    }
    val retCode = if (retList.isEmpty) 404 else 200
    api.dataJson(retCode, Map("serviceList" -> retList))
  }
}