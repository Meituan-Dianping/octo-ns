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
import com.meituan.octo.mns.util.HttpUtilTests;
import com.meituan.octo.mns.util.ProcessInfoUtil;
import com.octo.naming.common.thrift.model.ProtocolRequest;
import com.octo.naming.common.thrift.model.SGService;
import com.octo.naming.common.thrift.model.ServiceDetail;
import org.apache.thrift.TException;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class RegisterTests {
    private static Logger LOG = LoggerFactory.getLogger(RegisterTests.class);
    String providerAppkey = "com.sankuai.inf.dorado.server";
    String localAppkey = "com.sankuai.octo.tmy";
    int port = 9002;
    private static String localIP = ProcessInfoUtil.getLocalIpv4();

    @Test
    public void register() throws TException, InterruptedException {
        SGService service = SGServiceUtilTests.getDefaultSGService(providerAppkey, 5198, true);
        service.setProtocol("thrift");
        MnsInvoker.registServiceWithCmd(0, service);
        Thread.sleep(5000);
    }

    @Test
    public void testAddRegister() throws InterruptedException, TException {
        ProtocolRequest request = new ProtocolRequest();
        request.setProtocol("thrift")
                .setLocalAppkey(localAppkey)
                .setRemoteAppkey(providerAppkey);
        List<SGService> list = MnsInvoker.getServiceList(request);
        int size1 = list.size();

        SGService service = SGServiceUtilTests.getDefaultSGService(providerAppkey, port, true);
        Map<String, ServiceDetail> serviceNames = new HashMap<String, ServiceDetail>();
        ServiceDetail detail = new ServiceDetail();
        detail.setUnifiedProto(true);
        serviceNames.put("octo.service.base_a", detail);
        service.setServiceInfo(serviceNames);
        MnsInvoker.registServiceWithCmd(1, service);
        Thread.sleep(5000);

        list = MnsInvoker.getServiceList(request);

        service.getServiceInfo().put("octo.service.base_b", detail);
        MnsInvoker.registServiceWithCmd(1, service);

        list = MnsInvoker.getServiceList(request);
        LOG.info("{}", list.size());
    }

    @Test
    public void testServiceName() throws InterruptedException, TException {
        int port = 10002;
        String serviceName = "octo.service.base_c";
        HttpUtilTests.delete(providerAppkey, "thrift", localIP, port);
        SGService service = SGServiceUtilTests.getDefaultSGService(providerAppkey, port, true);
        Map<String, ServiceDetail> serviceNames = new HashMap<String, ServiceDetail>();
        ServiceDetail detail = new ServiceDetail();
        detail.setUnifiedProto(true);
        serviceNames.put(serviceName, detail);
        service.setServiceInfo(serviceNames);
        MnsInvoker.registServiceWithCmd(0, service);
        Thread.sleep(10000);
        ProtocolRequest request = new ProtocolRequest();
        request.setProtocol("thrift")
                .setLocalAppkey(localAppkey)
                .setServiceName(serviceName);
        List<SGService> list = MnsInvoker.getServiceList(request);
        boolean isExist = false;
        for (SGService item : list) {
            if (localIP.equals(item.getIp()) && port == item.getPort() && item.getServiceInfo().containsKey(serviceName)) {
                isExist = true;
            }
        }
        Assert.assertTrue(isExist);
    }
}
