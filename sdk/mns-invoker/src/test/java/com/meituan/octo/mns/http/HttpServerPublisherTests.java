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
package com.meituan.octo.mns.http;

import com.meituan.octo.mns.MnsInvoker;
import com.meituan.octo.mns.sentinel.CustomizedManager;
import com.meituan.octo.mns.util.ProcessInfoUtil;
import com.octo.naming.common.thrift.model.ProtocolRequest;
import com.octo.naming.common.thrift.model.SGService;
import org.junit.Assert;
import org.junit.Test;

import java.util.List;


public class HttpServerPublisherTests {

    @Test
    public void testRegist() throws Exception {
        //CustomizedManager.setCustomizedSGAgents("10.22.23.29:5266");
        String remoteAppkey = "com.sankuai.inf.dorado.server";
        int port = 5110;

        HttpServerPublisher httpServerPublisher = new HttpServerPublisher();
        httpServerPublisher.setAppKey(remoteAppkey);
        httpServerPublisher.setPort(port);
        httpServerPublisher.setVersion("original");
        httpServerPublisher.publish();

        Thread.sleep(3000);

        boolean suc = false;
        ProtocolRequest request = new ProtocolRequest();
        request.setProtocol("http")
                .setRemoteAppkey(remoteAppkey);
        List<SGService> services = MnsInvoker.getServiceList(request);
        for (SGService sgService : services) {
            if (sgService.getIp().equals(ProcessInfoUtil.getLocalIpv4()) && sgService.getPort() == port) {
                suc = true;
                break;
            }
        }
        Assert.assertTrue(suc);
    }
}
