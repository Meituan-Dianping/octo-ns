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
import com.meituan.octo.mns.util.IpUtil;
import com.octo.naming.common.thrift.model.SGService;
import com.octo.naming.service.thrift.model.ServiceAgent;
import org.apache.commons.lang3.StringUtils;
import org.apache.thrift.TException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class RegistryManager {

    private static final Logger LOG = LoggerFactory.getLogger(RegistryManager.class);
    private final ServiceAgent.Iface tempClient = new InvokeProxy(SGAgentClient.ClientType.temp).getProxy();
    private ExecutorService executors = Executors.newFixedThreadPool(1);


    public void registerServiceWithCmd(final int cmd, final SGService sgService) throws TException {
        if (!isSGServiceValid(sgService)) {
            return;
        }
        executors.submit(new Runnable() {
            @Override
            public void run() {
                try {
                    synchronized (tempClient) {
                        LOG.info("Mns service provider registration, cmd ={} | param = {}", cmd,
                                sgService.toString());
                        tempClient.registServicewithCmd(cmd, sgService);
                    }
                } catch (Exception e) {
                    LOG.error(e.getMessage(), e);
                }
            }
        });
    }

    public void unRegisterService(final SGService sgService) throws TException {
        if (!isSGServiceValid(sgService)) {
            return;
        }
        executors.submit(new Runnable() {
            @Override
            public void run() {
                try {
                    synchronized (tempClient) {
                        LOG.info("Mns service provider unRegistration, param = {}", sgService.toString());
                        tempClient.unRegistService(sgService);
                    }
                } catch (Exception e) {
                    LOG.error(e.getMessage(), e);
                }
            }
        });
    }


    private static boolean isSGServiceValid(SGService service) {
        if (null == service) {
            LOG.error("Cannot register null.");
            return false;
        }
        int port = service.getPort();
        String appkey = service.getAppkey();
        String ip = service.getIp();
        if (port < 1) {
            LOG.error("Invalid port: port|" + port + "|appkey:" + appkey);
            return false;
        }
        if (StringUtils.isEmpty(appkey)) {
            LOG.error("Invalid appKey: appkey|" + appkey);
            return false;
        }

        if (!IpUtil.checkIP(ip)) {
            LOG.error("Invalid ip: ip|" + ip + "|appkey:" + appkey);
            return false;
        }
        return true;
    }
}
