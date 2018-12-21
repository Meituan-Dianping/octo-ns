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
package com.meituan.octo.mns.listener;

import com.meituan.octo.mns.MnsInvoker;
import com.meituan.octo.mns.SGServiceUtilTests;
import com.meituan.octo.mns.sentinel.CustomizedManager;
import com.meituan.octo.mns.util.HttpUtilTests;
import com.meituan.octo.mns.util.ProcessInfoUtil;
import com.octo.naming.common.thrift.model.ProtocolRequest;
import com.octo.naming.common.thrift.model.SGService;
import org.apache.thrift.TException;
import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.List;

public class ServiceListListenerTest {
    private static Logger LOG = LoggerFactory.getLogger(ServiceListListenerTest.class);
    String remoteAppkey = "com.sankuai.inf.dorado.server";
    String localAppkey = "com.sankuai.octo.tmy";
    int port = 9010;
    String localIP = ProcessInfoUtil.getLocalIpv4();
    boolean addCallback = false, deletedCallback = false;

    private enum ServiceType {
        PROTOCOL, ORIGIN
    }


    private class MyListener implements IServiceListChangeListener {
        @Override
        public void changed(ProtocolRequest req, List<SGService> oldList, List<SGService> newList, List<SGService> addList, List<SGService> deletedList, List<SGService> modifiedList) {
            System.out.println("req protocol: " + req.getProtocol());
            if (!addList.isEmpty()) {
                addCallback = true;
                print("addList:", addList);
            }

            if (!deletedList.isEmpty()) {
                deletedCallback = true;
                print("deletedList:", deletedList);
            }

        }

        private void print(String msg, List<SGService> list) {
            System.out.println(msg);
            for (SGService service : list) {
                System.out.println(service);
            }
        }
    }

    @Test
    public void serviceListListenerTest() throws TException, InterruptedException {
        testListener(ServiceType.ORIGIN);
        testListener(ServiceType.PROTOCOL);
    }

    private void testListener(ServiceType type) throws TException, InterruptedException {
        ProtocolRequest httpReq = new ProtocolRequest()
                .setLocalAppkey(localAppkey)
                .setRemoteAppkey(remoteAppkey)
                .setProtocol("http");
        ProtocolRequest thriftReq = new ProtocolRequest()
                .setLocalAppkey(localAppkey)
                .setRemoteAppkey(remoteAppkey)
                .setProtocol("thrift");

        IServiceListChangeListener thriftListener = new MyListener();
        IServiceListChangeListener httpListener = new MyListener();
        delete();

        addCallback = false;
        deletedCallback = false;
        switch (type) {
            case PROTOCOL:
                Assert.assertEquals(0, MnsInvoker.addServiceListener(thriftReq, thriftListener));
                Assert.assertEquals(0, MnsInvoker.addServiceListener(httpReq, httpListener));
                break;
            case ORIGIN:
                Assert.assertEquals(0, MnsInvoker.addOriginServiceListener(thriftReq, thriftListener));
                Assert.assertEquals(0, MnsInvoker.addOriginServiceListener(httpReq, httpListener));
                break;
        }


        LOG.info("Now start to test listener");
        register();
        Assert.assertTrue(addCallback);
        delete();
        Assert.assertTrue(deletedCallback);
        LOG.info("Now start to test remove listener");

        switch (type) {
            case PROTOCOL:
                Assert.assertEquals(0, MnsInvoker.removeServiceListener(thriftReq, thriftListener));
                Assert.assertEquals(0, MnsInvoker.removeServiceListener(httpReq, httpListener));
                break;
            case ORIGIN:
                Assert.assertEquals(0, MnsInvoker.removeOriginServiceListener(thriftReq, thriftListener));
                Assert.assertEquals(0, MnsInvoker.removeOriginServiceListener(httpReq, httpListener));
                break;
        }

        addCallback = false;
        deletedCallback = false;
        register();
        delete();
        Assert.assertFalse(addCallback || deletedCallback);
    }


    private void delete() throws InterruptedException {
        HttpUtilTests.delete(remoteAppkey, "thrift", localIP, 10000);
        HttpUtilTests.delete(remoteAppkey, "thrift", localIP, 10001);

        HttpUtilTests.delete(remoteAppkey, "http", localIP, 10000);
        HttpUtilTests.delete(remoteAppkey, "http", localIP, 10001);
        Thread.sleep(10000);

    }

    private void register() throws TException, InterruptedException {
        int weigth = 10;
        registerHandle("thrift", localIP, 10000, weigth);
        registerHandle("thrift", localIP, 10001, weigth);

        registerHandle("http", localIP, 10000, weigth);
        registerHandle("http", localIP, 10001, weigth);
        Thread.sleep(10000);

    }

    private void registerHandle(String protocol, String ip, int port, int weigth) throws TException, InterruptedException {
        SGService service = SGServiceUtilTests.getDefaultSGService(remoteAppkey, port, true);
        service.setIp(ip);
        service.setProtocol(protocol);
        service.setWeight(weigth).setFweight(weigth);
        MnsInvoker.registServiceWithCmd(0, service);
    }

    @After
    public void destroy() throws InterruptedException {
        delete();
    }
}
