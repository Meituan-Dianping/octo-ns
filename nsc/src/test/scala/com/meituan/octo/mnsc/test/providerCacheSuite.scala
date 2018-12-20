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
import com.meituan.octo.mnsc.model.service.CacheValue
import com.meituan.octo.mnsc.service.apiProviders
import com.octo.naming.common.thrift.model.SGService
import org.junit.runner.RunWith
import org.scalatest.junit.JUnitRunner
import org.scalatest.{BeforeAndAfter, FunSuite}


@RunWith(classOf[JUnitRunner])
class providerCacheSuite extends FunSuite with BeforeAndAfter {
  test("getPovidersCache") {
    val thrift_valid_services = appProvidersCommCache.getProviderCache("com.sankuai.inf.sg_sentinel", 3, "thrift")
    assert(thrift_valid_services.Providers.nonEmpty)
    val thrift_invalid_services = appProvidersCommCache.getProviderCache("com.sankuai.inf.sg_sentinel", 3, "http")
    assert(thrift_invalid_services.Providers.isEmpty)

    val http_valid_services = appProvidersCommCache.getProviderCache("com.sankuai.inf.msgp", 3, "thrift")
    assert(http_valid_services.Providers.isEmpty)
    val http_invalid_services = appProvidersCommCache.getProviderCache("com.sankuai.inf.msgp", 3, "http")
    assert(http_invalid_services.Providers.nonEmpty)

    val api_service = apiProviders.getProviders("com.sankuai.inf.sg_sentinel", 3, "thrift")
    assert(api_service.nonEmpty)
  }

  test("get provider cache from remote") {
    //appProviderDataCache.renewAllProviderForce(Path.provider.toString, true)
    val emptyValue = CacheValue("invalid-version", List[SGService]())

    //thrift
    val local_cache = appProviderDataCache.getProviderCache("com.sankuai.inf.sg_sentinel", "prod", false)
    assert(local_cache.getOrElse(emptyValue).version == "invalid-version")
    assert(local_cache.getOrElse(emptyValue).SGServices.isEmpty)

    val remote_cache = appProviderDataCache.getProviderCache("com.sankuai.inf.sg_sentinel", "prod", true)
    assert(remote_cache.getOrElse(emptyValue).version != "invalid-version")
    assert(remote_cache.getOrElse(emptyValue).SGServices.nonEmpty)

    //http
    val local_cache_http = appProviderHttpDataCache.getProviderHttpCache("com.sankuai.inf.msgp", "prod", false)
    assert(local_cache_http.getOrElse(emptyValue).version == "invalid-version")
    assert(local_cache_http.getOrElse(emptyValue).SGServices.isEmpty)

    val remote_cache_http = appProviderHttpDataCache.getProviderHttpCache("com.sankuai.inf.msgp", "prod", true)
    assert(remote_cache_http.getOrElse(emptyValue).version != "invalid-version")
    assert(remote_cache_http.getOrElse(emptyValue).SGServices.nonEmpty)
  }

  test("get provider cache from local") {
    val emptyValue = CacheValue("invalid-version", List[SGService]())

    appProviderDataCache.renewAllProviderForce(Path.provider.toString, true)
    val cache = appProviderDataCache.getProviderCache("com.sankuai.inf.sg_sentinel", "prod", false)
    assert(cache.getOrElse(emptyValue).version != "invalid-version")
    assert(cache.getOrElse(emptyValue).SGServices.nonEmpty)
    val invalidCache = appProviderDataCache.getProviderCache("com.sankuai.invalid.test", "prod", false)
    assert(invalidCache.getOrElse(emptyValue).version == "invalid-version")
    assert(invalidCache.getOrElse(emptyValue).SGServices.isEmpty)

    appProviderHttpDataCache.renewAllProviderForce(Path.providerHttp.toString, true)
    val cache_http = appProviderHttpDataCache.getProviderHttpCache("com.sankuai.inf.msgp", "prod", false)
    assert(cache_http.getOrElse(emptyValue).version != "invalid-version")
    assert(cache_http.getOrElse(emptyValue).SGServices.nonEmpty)

    val invalid_cache_http = appProviderHttpDataCache.getProviderHttpCache("com.sankuai.invalid.test", "prod", false)
    assert(invalid_cache_http.getOrElse(emptyValue).version == "invalid-version")
    assert(invalid_cache_http.getOrElse(emptyValue).SGServices.isEmpty)
  }
}

@RunWith(classOf[JUnitRunner])
class providerCacheScheduleSuite extends FunSuite with BeforeAndAfter {
  test("schedule job worker") {
    val emptyValue = CacheValue("invalid-version", List[SGService]())
    val cache = appProviderDataCache.getProviderCache("com.sankuai.inf.sg_sentinel", "prod", false)
    assert(cache.getOrElse(emptyValue).version == "invalid-version")

    val cache_http = appProviderHttpDataCache.getProviderHttpCache("com.sankuai.inf.msgp", "prod", false)
    assert(cache_http.getOrElse(emptyValue).version == "invalid-version")

    appProviderHttpDataCache.doRenew()
    appProviderDataCache.doRenew()
    Thread.sleep(1000*60)

    val cache_valid = appProviderDataCache.getProviderCache("com.sankuai.inf.sg_sentinel", "prod", false)
    assert(cache_valid.getOrElse(emptyValue).version != "invalid-version")

    val invalidCache = appProviderDataCache.getProviderCache("com.sankuai.invalid.test", "prod", false)
    assert(invalidCache.getOrElse(emptyValue).version == "invalid-version")

    val cache_http_valid = appProviderHttpDataCache.getProviderHttpCache("com.sankuai.inf.msgp", "prod", false)
    assert(cache_http_valid.getOrElse(emptyValue).version != "invalid-version")

    val invalid_cache_http = appProviderHttpDataCache.getProviderHttpCache("com.sankuai.invalid.test", "prod", false)
    assert(invalid_cache_http.getOrElse(emptyValue).version == "invalid-version")

    val ip_valid = "10.72.221.226"
    val ip_invalid = "invalid_ip"
    assert(appProviderDataCache.getProvidersByIP(ip_valid).nonEmpty)
    assert(appProviderDataCache.getProvidersByIP(ip_invalid).isEmpty)

    val http_ip_valid = "10.24.124.94"
    assert(appProviderHttpDataCache.getProvidersByIP(http_ip_valid).nonEmpty)
    assert(appProviderHttpDataCache.getProvidersByIP(ip_invalid).isEmpty)
  }
}