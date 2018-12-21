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

import com.meituan.octo.mns.Consts;
import com.meituan.octo.mns.listener.IServiceListChangeListener;
import com.meituan.octo.mns.util.ScheduleTaskFactory;
import com.octo.naming.common.thrift.model.ProtocolRequest;
import com.octo.naming.common.thrift.model.SGService;
import org.apache.commons.lang3.StringUtils;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class MnsCacheManager {
    private static final Logger LOG = LoggerFactory.getLogger(MnsCacheManager.class);

    private ScheduledExecutorService scheduler = Executors.newScheduledThreadPool(1, new ScheduleTaskFactory("MnsCacheManager-Schedule"));

    private ServiceCache protocolServiceCache = new ServiceCache();

    private ServiceCache originServiceCache = new OriginServiceCache();


    private enum ServiceCacheType {
        PROTOCOL, ORIGIN
    }


    public MnsCacheManager() {
        LOG.info("init MnsCacheManager, start update scheduler");
        scheduler.scheduleWithFixedDelay(new Runnable() {
            @Override
            public void run() {
                try {
                    protocolServiceCache.updateAll();
                    originServiceCache.updateAll();
                } catch (Exception e) {
                    LOG.debug("update mns cache exception, " + e.getMessage(), e);
                }
            }
        }, 1, Consts.DEFAULT_UPDATE_TIME, TimeUnit.SECONDS);
    }

    private boolean isValid(ProtocolRequest req) {
        boolean ret = (null != req)
                && !(StringUtils.isEmpty(req.getRemoteAppkey()) && StringUtils.isEmpty(req.getServiceName()))
                && !StringUtils.isEmpty(req.getProtocol());
        if (!ret) {
            LOG.error("Invalid ProtocolRequest, protocol and remoteAppkey/serviceName cannot be empty.");
        }
        return ret;
    }

    private String handleLocalAppkey(String localAppkey) {
        if (null == localAppkey) {
            LOG.debug("localAppkey is null, probably not set, please check the settings!");
            localAppkey = "";
        }
        return localAppkey;
    }

    private void pretreatServiceListRequest(ProtocolRequest req) {
        String curLocalAppkey = handleLocalAppkey(req.getLocalAppkey());

        req.setLocalAppkey(StringUtils.trim(curLocalAppkey))
                .setRemoteAppkey(StringUtils.trim(req.getRemoteAppkey()))
                .setServiceName(StringUtils.trim(req.getServiceName()))
                .setProtocol(StringUtils.trim(req.getProtocol()));
    }

    public List<SGService> getServiceList(ProtocolRequest req) {
        return getServiceListByType(req, ServiceCacheType.PROTOCOL);
    }

    public List<SGService> getOriginServiceList(ProtocolRequest req) {
        return getServiceListByType(req, ServiceCacheType.ORIGIN);
    }

    private List<SGService> getServiceListByType(ProtocolRequest req, ServiceCacheType type) {
        if (!isValid(req)) {
            return Collections.emptyList();
        }
        pretreatServiceListRequest(req);
        switch (type) {
            case PROTOCOL:
                return protocolServiceCache.get(req);
            case ORIGIN:
                return originServiceCache.get(req);
            default:
                LOG.error("unknown ServiceCacheType = {}", type);
                return Collections.emptyList();
        }
    }


    public int addServiceListListener(ProtocolRequest req, IServiceListChangeListener listener) {
        return handleListener(protocolServiceCache, req, listener, true);
    }


    public int removeServiceListListener(ProtocolRequest req, IServiceListChangeListener listener) {
        return handleListener(protocolServiceCache, req, listener, false);
    }

    public int addOriginServiceListListener(ProtocolRequest req, IServiceListChangeListener listener) {
        return handleListener(originServiceCache, req, listener, true);
    }

    public int removeOriginServiceListListener(ProtocolRequest req, IServiceListChangeListener listener) {
        return handleListener(originServiceCache, req, listener, false);

    }


    void clearCache() {
        protocolServiceCache.clearCache();
        originServiceCache.clearCache();
    }


    private int handleListener(ServiceCache cache, ProtocolRequest req, IServiceListChangeListener listener, boolean isAdd) {
        if (!isValid(req) || null == listener) {
            return -1;
        }
        pretreatServiceListRequest(req);

        if (isAdd) {
            cache.addListener(req, listener);
        } else {
            return cache.removeListener(req, listener);
        }
        return 0;
    }
}
