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
package com.meituan.octo.mns.cache;

import com.meituan.octo.mns.util.ProcessInfoUtil;
import com.octo.naming.common.thrift.model.SGService;
import org.junit.Test;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

public class MnsCacheTests {
    private static final Logger LOG = LoggerFactory.getLogger(MnsCacheTests.class);

    @Test
    public void test() {

        MnsCache cache = new MnsCache<String, String, List<SGService>>(
                new CacheLoader<String, String, List<SGService>>() {
                    @Override
                    public List<SGService> reload(String row, String column) {
                        return null;
                    }
                });
        LOG.info("{}", cache.get("local", "remote"));
        cache.put("local", "remote", Arrays.asList(randService()));
        LOG.info("{}", cache.get("local", "remote"));
        cache.updateAll();
        cache.put("local", "remote", Collections.emptyList());
        LOG.info("{}", cache.get("local", "remote"));
        cache.put("", "test", "test");
        LOG.info("{}", cache.get("", "test"));
        cache.updateAll();
    }

    private SGService randService() {
        SGService sgService = new SGService();
        sgService.setAppkey("test");
        sgService.setIp(ProcessInfoUtil.getLocalIpv4());
        sgService.setPort(111);
        sgService.setLastUpdateTime((int) (System.currentTimeMillis() / 1000));
        sgService.setVersion("original");
        sgService.setWeight(10);
        sgService.setFweight(10.d);

        return sgService;
    }
}
