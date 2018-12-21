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
package com.meituan.octo.mns;


import com.meituan.octo.mns.model.SGAgentClient;
import com.meituan.octo.mns.sentinel.CustomizedManager;
import com.octo.naming.common.thrift.model.ProtocolRequest;
import com.octo.naming.common.thrift.model.SGService;
import com.octo.naming.service.thrift.model.ServiceAgent;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.List;

public class InvokeProxyTests {
    private static Logger LOG = LoggerFactory.getLogger(InvokeProxyTests.class);
    String remoteAppkey = "com.sankuai.inf.dorado.server";
    String localAppkey = "com.sankuai.octo.tmy";

    @Before
    public void setCustomizedSgagent() {
        CustomizedManager.setCustomizedSGAgents("10.22.23.29:5266");
    }

    @Test
    public void testProxy() {
        InvokeProxy invoker = new InvokeProxy(SGAgentClient.ClientType.temp);
        try {
            ServiceAgent.Iface proxy = invoker.getProxy();
            ProtocolRequest thriftReq = new ProtocolRequest()
                    .setLocalAppkey(localAppkey)
                    .setRemoteAppkey(remoteAppkey)
                    .setProtocol("thrift");
            List<SGService> serviceList = proxy.getServiceListByProtocol(thriftReq).getServicelist();
            LOG.info("{}", serviceList);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }


    // please use jprofiler to run this test case. be careful the memory of the byte[] and full gc.
    @Test
    public void multiThreadProxy() throws Exception {
        final ServiceAgent.Iface multiProtocolClient = new InvokeProxy(SGAgentClient.ClientType.multiProto).getProxy();
        for (int i = 0; i < 5; ++i) {


            Thread th2 = new Thread(new Runnable() {
                @Override
                public void run() {

                    ProtocolRequest req = new ProtocolRequest();
                    req.setProtocol("thrift")
                            .setRemoteAppkey("com.sankuai.inf.mnsc");
                    while (true) {
                        try {
                            multiProtocolClient.getServiceListByProtocol(req);
                            Thread.sleep(10);
                        } catch (Exception e) {
                            Assert.assertTrue(false);
                        }

                    }


                }
            });

            th2.start();
        }

        Thread.sleep(1000 * 120);
    }
}
