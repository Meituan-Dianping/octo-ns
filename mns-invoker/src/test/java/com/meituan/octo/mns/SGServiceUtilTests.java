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

import com.meituan.octo.mns.util.ProcessInfoUtil;
import com.octo.naming.common.thrift.model.SGService;


public class SGServiceUtilTests {
    private static String localIp = ProcessInfoUtil.getLocalIpv4();

    public static SGService getDefaultSGService(final String appkey, final int port, final boolean isThrift) {
        String protocol = isThrift ? "thrift" : "http";
        SGService service = new SGService();
        service.setAppkey(appkey);
        service.setPort(port);
        service.setVersion("original");
        service.setIp(localIp);
        service.setLastUpdateTime((int) (System.currentTimeMillis() / 1000));
        service.setServerType(isThrift ? 0 : 1);
        service.setWeight(10);
        service.setFweight(10.d);
        service.setProtocol(protocol);
        return service;
    }
}
