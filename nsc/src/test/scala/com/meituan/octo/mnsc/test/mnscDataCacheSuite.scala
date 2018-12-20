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

package com.meituan.octo.mnsc.test

import com.meituan.octo.mnsc.dataCache._
import com.meituan.octo.mnsc.model.Path
import com.meituan.octo.mnsc.service.mnscService
import com.octo.mnsc.idl.thrift.model.{mnsc_dataConstants, MnsRequest, Protocols}
import org.junit.runner.RunWith
import org.scalatest.junit.JUnitRunner
import org.scalatest.{BeforeAndAfter, FunSuite}


@RunWith(classOf[JUnitRunner])
class mnscServiceSuite extends FunSuite with BeforeAndAfter {
  test("test ip affiliation") {
    val apps = new java.util.ArrayList[String]()
    apps.add("com.sankuai.inf.sg_sentinel")
    apps.add("com.sankuai.inf.msgp")
    val thrift_cache = mnscService.getMNSCacheByAppkeys(apps, "thrift")
    println(thrift_cache.cache)
    assert(thrift_cache.cache.get("com.sankuai.inf.sg_sentinel").get("prod").isEmpty)
    val http_cache = mnscService.getMNSCacheByAppkeys(apps, "http")
    assert(http_cache.cache.get("com.sankuai.inf.msgp").get("prod").isEmpty)

    assert(mnscService.getAppkeyListByIP("10.72.221.226").isEmpty)
    assert(mnscService.getProvidersByIP("10.72.221.226").defaultMNSCache.isEmpty)

    appProviderDataCache.renewAllProviderForce(Path.provider.toString, true)
    appProviderHttpDataCache.renewAllProviderForce(Path.providerHttp.toString, true)

    assert(!mnscService.getAppkeyListByIP("10.72.221.226").isEmpty)
    assert(!mnscService.getProvidersByIP("10.72.221.226").defaultMNSCache.isEmpty)
  }

  test("get MNSCache service API1") {
    assert(!mnscService.getMnsc("com.sankuai.inf.sg_sentinel","0|0|1","prod").defaultMNSCache.isEmpty)
    assert(mnscService.getMnsc("com.sankuai.inf.sg_sentinel","0|0|99999999","prod").code == mnsc_dataConstants.NOT_MODIFIED)
    assert(!mnscService.getMNSCache4HLB("com.sankuai.inf.msgp","0|0|1","prod").defaultMNSCache.isEmpty)
    assert(mnscService.getMNSCache4HLB("com.sankuai.inf.msgp","0|0|99999999","prod").code == mnsc_dataConstants.NOT_MODIFIED)

    assert(mnscService.getMnsc("com.sankuai.invalid.appkey","0|0|1","prod").code == mnsc_dataConstants.NOT_FOUND)
    assert(mnscService.getMNSCache4HLB("com.sankuai.invalid.appkey","0|0|99999999","prod").code == mnsc_dataConstants.NOT_FOUND)
  }

  test("get MNSCache service API2") {
    val request = new MnsRequest()
    request.appkey = "com.sankuai.inf.sg_sentinel"
    request.env = "prod"
    request.protoctol= Protocols.THRIFT
    assert(mnscService.getMnsc(request).code == mnsc_dataConstants.SUCCESS)

    request.appkey = "com.sankuai.invalid.appkey"
    assert(mnscService.getMnsc(request).code == mnsc_dataConstants.NOT_FOUND)

    request.appkey = "com.sankuai.inf.msgp"
    request.protoctol= Protocols.HTTP
    assert(mnscService.getMnsc(request).code == mnsc_dataConstants.SUCCESS)

    request.appkey = "com.sankuai.invalid.appkey"
    assert(mnscService.getMnsc(request).code == mnsc_dataConstants.NOT_FOUND)
  }
}