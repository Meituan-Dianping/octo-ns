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

import com.meituan.octo.mns.sentinel.CustomizedManager;
import com.octo.naming.common.thrift.model.ProtocolRequest;
import com.octo.naming.common.thrift.model.SGService;
import com.octo.naming.service.thrift.model.ServiceAgent;
import org.apache.thrift.TException;
import org.apache.thrift.async.AsyncMethodCallback;
import org.apache.thrift.async.TAsyncClientManager;
import org.apache.thrift.protocol.TBinaryProtocol;
import org.apache.thrift.protocol.TProtocolFactory;
import org.apache.thrift.transport.TNonblockingSocket;
import org.junit.Before;
import org.junit.Test;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.List;

public class AsyncTests {
    private static Logger LOG = LoggerFactory.getLogger(AsyncTests.class);
    String providerAppkey = "com.sankuai.inf.dorado.server";
    String localAppkey = "com.sankuai.octo.tmy";

    @Before
    public void setCustomizedSgagent() {
        CustomizedManager.setCustomizedSGAgents("10.22.23.29:5266");
    }

    @Test
    public void testAsync() throws Exception {
        TProtocolFactory factory = new TBinaryProtocol.Factory();
        TAsyncClientManager manager = new TAsyncClientManager();
        ServiceAgent.AsyncClient.Factory clientFactory = new ServiceAgent.AsyncClient.Factory(manager, factory);

        ServiceAgent.AsyncClient client = clientFactory.getAsyncClient(
                new TNonblockingSocket("10.22.23.29", 5266, Consts.CONNECT_TIMEOUT));
        AsyncMethodCallback<ServiceAgent.AsyncClient.getOriginServiceList_call> callback1 = new ServiceListCallback();
        ProtocolRequest request = new ProtocolRequest();
        request.setProtocol("thrift")
                .setLocalAppkey(localAppkey)
                .setRemoteAppkey(providerAppkey);
        client.getOriginServiceList(request, callback1);
        Thread.sleep(10000);
    }

    class ServiceListCallback implements AsyncMethodCallback<ServiceAgent.AsyncClient.getOriginServiceList_call> {

        @Override
        public void onComplete(ServiceAgent.AsyncClient.getOriginServiceList_call getOriginServiceList_call) {
            try {
                List<SGService> list = getOriginServiceList_call.getResult().getServicelist();
                LOG.info("{}", list.size());
            } catch (TException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void onError(Exception e) {
            e.printStackTrace();
        }
    }
}
