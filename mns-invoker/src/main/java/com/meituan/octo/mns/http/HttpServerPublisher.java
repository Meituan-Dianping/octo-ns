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

import com.meituan.octo.mns.Consts;
import com.meituan.octo.mns.MnsInvoker;
import com.meituan.octo.mns.util.ProcessInfoUtil;
import com.octo.naming.common.thrift.model.SGService;
import com.octo.naming.common.thrift.model.fb_status;
import org.apache.commons.lang3.StringUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class HttpServerPublisher {
    private static final Logger LOG = LoggerFactory.getLogger(HttpServerPublisher.class);

    private String appKey;
    private int port = -1;
    private String version;
    private static String localIP = ProcessInfoUtil.getLocalIpv4();


    public String getVersion() {
        return version;
    }

    public void setVersion(String version) {
        this.version = version;
    }

    public String getAppKey() {
        return appKey;
    }

    public void setAppKey(String appKey) {
        this.appKey = appKey;
    }

    public int getPort() {
        return port;
    }

    public void setPort(int port) {
        this.port = port;
    }

    private boolean isValid() {
        if (StringUtils.isEmpty(appKey)) {
            LOG.error("Http publisher error, appKey cannot be empty.");
            return false;
        }
        if (port < 1) {
            LOG.error("Http publisher error, invalid port {}.", port);
            return false;
        }
        return true;
    }

    public void publish() {
        if (!isValid()) {
            return;
        }
        try {
            LOG.info("Http registration appKey = {}, port = {}", appKey, port);
            MnsInvoker.registServiceWithCmd(1, getSGService());
        } catch (Exception e) {
            LOG.debug("Http registration error.", e);
        }
    }

    public void destroy() {
        if (!isValid()) {
            return;
        }
        try {
            LOG.info("Http unRegistration appKey = {}, port = {}", appKey, port);
            MnsInvoker.unRegisterService(getSGService());
            Thread.sleep(1000L);
        } catch (Exception e) {
            LOG.debug("Http unRegistration error.", e);
        }
    }


    private SGService getSGService() {
        SGService service = new SGService();

        service.setAppkey(appKey)
                .setPort(port).setIp(localIP)
                .setLastUpdateTime((int) (System.currentTimeMillis() / 1000))
                .setServerType(1)
                .setWeight(10).setFweight(10.d)
                .setProtocol("http");

        if (StringUtils.isEmpty(version)) {
            setVersion(Consts.MNS_DEFAULT_VERSION);
        }
        service.setStatus(fb_status.DEAD.getValue())
                .setVersion(getVersion());

        return service;
    }
}
