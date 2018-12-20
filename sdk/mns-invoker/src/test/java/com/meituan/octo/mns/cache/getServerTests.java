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

import com.meituan.octo.mns.MnsInvoker;
import com.meituan.octo.mns.listener.ServiceListListenerTest;
import com.meituan.octo.mns.sentinel.CustomizedManager;
import com.meituan.octo.mns.util.HttpUtilTests;
import com.octo.naming.common.thrift.model.ProtocolRequest;
import com.octo.naming.common.thrift.model.SGService;
import org.apache.thrift.TException;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.ArrayList;
import java.util.List;

public class getServerTests {
    private static Logger LOG = LoggerFactory.getLogger(ServiceListListenerTest.class);

    final String consumerAppkey = "com.sankuai.octo.tmy";
    final String remoteAppkey = "com.sankuai.inf.dorado.server";

    /*@Before
    public void setCustomizedSgagent() {
        CustomizedManager.setCustomizedSGAgents("10.22.23.29:5266");
    }*/

    @Test
    public void testProtocolServiceList() throws TException, InterruptedException {
        System.out.println("【testGetServiceListByProtocol】thrift http redis cunsumerAppkey =" + consumerAppkey + " and remoteAppkey = " + remoteAppkey);

        ProtocolRequest httpReq = new ProtocolRequest()
                .setLocalAppkey(consumerAppkey)
                .setRemoteAppkey(remoteAppkey)
                .setProtocol("http");
        ProtocolRequest thriftReq = new ProtocolRequest()
                .setLocalAppkey(consumerAppkey)
                .setRemoteAppkey(remoteAppkey)
                .setProtocol("thrift");


        List<SGService> httpList = MnsInvoker.getServiceList(httpReq);
        List<SGService> thriftList = MnsInvoker.getServiceList(thriftReq);
        LOG.info("{}", httpList.size());
        LOG.info("{}", thriftList.size());

    }


    @Test
    public void multiThread() throws InterruptedException {
        for (int i = 1; i < 3; i++) {
            new Thread() {
                @Override
                public void run() {
                    try {
                        try {
                            testProtocolServiceList();
                        } catch (TException e) {
                            Assert.assertTrue(false);
                        }

                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }.start();
        }
        Thread.sleep(1000);
    }


    public static void delete(String remoteAppkey, String protocol, String ipPort) throws InterruptedException {
        List<String> ipPorts = new ArrayList<String>();
        ipPorts.add(ipPort);
        String url = "http://mns.test.sankuai.info/api/providers/delete";
        String data = HttpUtilTests.getDeletedProviderJson(remoteAppkey, protocol, ipPorts, 3);
        HttpUtilTests.delete(url, data);
        Thread.sleep(5000);
    }


}
