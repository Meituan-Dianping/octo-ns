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

import com.meituan.octo.mns.cache.MnsCacheManager;
import com.meituan.octo.mns.listener.IServiceListChangeListener;
import com.octo.naming.common.thrift.model.ProtocolRequest;
import com.octo.naming.common.thrift.model.SGService;
import org.apache.thrift.TException;

import java.util.List;

public class MnsInvoker {

    private static MnsCacheManager cacheManager = new MnsCacheManager();

    private static RegistryManager registryManager = new RegistryManager();


    public static void registServiceWithCmd(int uptCmd, SGService sgService) throws TException {
        registryManager.registerServiceWithCmd(uptCmd, sgService);
    }

    public static void unRegisterService(SGService sgService) throws TException {
        registryManager.unRegisterService(sgService);
    }


    public static int addServiceListener(ProtocolRequest req, IServiceListChangeListener listener) {
        return cacheManager.addServiceListListener(req, listener);
    }

    public static int removeServiceListener(ProtocolRequest req, IServiceListChangeListener listener) {
        return cacheManager.removeServiceListListener(req, listener);
    }

    public static List<SGService> getServiceList(ProtocolRequest req) {
        return cacheManager.getServiceList(req);
    }

    public static List<SGService> getOriginServiceList(ProtocolRequest req) {
        return cacheManager.getOriginServiceList(req);
    }

    public static int addOriginServiceListener(ProtocolRequest req, IServiceListChangeListener listener) {
        return cacheManager.addOriginServiceListListener(req, listener);
    }

    public static int removeOriginServiceListener(ProtocolRequest req, IServiceListChangeListener listener) {
        return cacheManager.removeOriginServiceListListener(req, listener);
    }

}
