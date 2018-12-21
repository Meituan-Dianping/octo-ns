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

import com.meituan.octo.mnsc.service.mnscService;
import com.meituan.octo.mnsc.web.api.ProvidersController;
import com.octo.mnsc.idl.thrift.model.mnsc_dataConstants;
import com.octo.mnsc.idl.thrift.model.MnsRequest;
import com.octo.mnsc.idl.thrift.model.Protocols;
import org.junit.Test;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.ArrayList;
import java.util.List;

public class MNSCacheServiceTest extends SpringBaseTest{
    static final Logger LOG = LoggerFactory.getLogger(ProvidersController.class);

    @Test
    public void getAppKeyByIP() throws Exception {
        long start = System.currentTimeMillis();
        assert(!mnscService.getAppkeyListByIP("10.72.221.226").isEmpty());
        System.out.println(System.currentTimeMillis() - start);
    }

    @Test
    public void getProvidersByIP() throws Exception {
        assert(!mnscService.getProvidersByIP("10.72.221.226").defaultMNSCache.isEmpty());
    }

    @Test
    public void getMNSCacheWithVersionCheck() throws Exception {
        MnsRequest request = new MnsRequest();
        request.appkey = "com.sankuai.inf.sg_sentinel";
        request.env = "prod";
        request.protoctol= Protocols.THRIFT;
        assert(mnscService.getMnsc(request).code == mnsc_dataConstants.SUCCESS);

        request.appkey = "com.sankuai.invalid.appkey";
        assert(mnscService.getMnsc(request).code == mnsc_dataConstants.NOT_FOUND);

        request.appkey = "com.sankuai.inf.msgp";
        request.protoctol= Protocols.HTTP;
        assert(mnscService.getMnsc(request).code == mnsc_dataConstants.SUCCESS);

        request.appkey = "com.sankuai.invalid.appkey";
        assert(mnscService.getMnsc(request).code == mnsc_dataConstants.NOT_FOUND);
    }

    @Test
    public void getMNSCacheByAppkeys() throws Exception {
        List<String> appkeys = new ArrayList<String>();
        appkeys.add("com.sankuai.inf.sg_sentinel");
        appkeys.add("com.sankuai.inf.msgp");
        appkeys.add("com.sankuai.invalid.appkey");
        assert(!mnscService.getMNSCacheByAppkeys(appkeys, "thrift").cache.get("com.sankuai.inf.sg_sentinel").get("prod").isEmpty());
        assert(mnscService.getMNSCacheByAppkeys(appkeys, "thrift").cache.get("com.sankuai.invalid.appkey").get("prod").isEmpty());

        assert(!mnscService.getMNSCacheByAppkeys(appkeys, "http").cache.get("com.sankuai.inf.msgp").get("prod").isEmpty());
        assert(mnscService.getMNSCacheByAppkeys(appkeys, "http").cache.get("com.sankuai.invalid.appkey").get("prod").isEmpty());
    }

    @Test
    public void getMNSCache() throws Exception {
        assert(!mnscService.getMnsc("com.sankuai.inf.sg_sentinel","0|0|1","prod").defaultMNSCache.isEmpty());
        assert(mnscService.getMnsc("com.sankuai.inf.sg_sentinel","0|0|99999999","prod").code == mnsc_dataConstants.NOT_MODIFIED);
    }

    @Test
    public void getMNSCacheHttp() throws Exception {
        assert(!mnscService.getMNSCache4HLB("com.sankuai.inf.msgp","0|0|1","prod").defaultMNSCache.isEmpty());
        assert(mnscService.getMNSCache4HLB("com.sankuai.inf.msgp","0|0|99999999","prod").code == mnsc_dataConstants.NOT_MODIFIED);

        assert(mnscService.getMnsc("com.sankuai.invalid.appkey","0|0|1","prod").code == mnsc_dataConstants.NOT_FOUND);
        assert(mnscService.getMNSCache4HLB("com.sankuai.invalid.appkey","0|0|99999999","prod").code == mnsc_dataConstants.NOT_FOUND);
    }
    /*@Test
    public void getSG_Agent() throws Exception {
        System.out.println(apiService.getServiceList("com.sankuai.inf.sg_sentinel", "test",  "10.22.16.146"));
    }*/

}
