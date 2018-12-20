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

package com.meituan.octo.mnsc;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.meituan.octo.mnsc.service.apiProviders;
import com.meituan.octo.mnsc.utils.mnscCommon;
import com.meituan.octo.mnsc.service.apiService;
import org.junit.Test;
import play.libs.Json;

import java.util.HashMap;

public class MNSCacheApiTest extends SpringBaseTest {
    @Test
    public void getProviders() throws Exception {
        assert(Json.parse(apiProviders.getProviders("com.sankuai.inf.sg_sentinel", 3, "thrift")).get("ret").asInt() == 200);
        assert(Json.parse(apiProviders.getProviders("com.sankuai.inf.msgp", 3, "http")).get("ret").asInt() == 200);
        assert(Json.parse(apiProviders.getProviders("com.sankuai.invalid.appkey", 3, "thrift")).get("ret").asInt() == 404);
    }

    @Test
    public void getAllAppkeys() throws Exception {
        assert(!mnscCommon.allAppkeysList().isEmpty());
    }

    @Test
    public void getSentinels() throws Exception {
        String ip = "10.20.50.135";
        String env = "prod";
        String appkey = "com.sankuai.inf.sg_sentinel";
        HashMap<String,Object> result = new ObjectMapper().readValue(apiService.getServiceList(appkey.trim(), env.trim(), ip.trim()), HashMap.class);
        System.out.println(result.get("data"));
        assert(result.get("ret").equals(200));
    }
}