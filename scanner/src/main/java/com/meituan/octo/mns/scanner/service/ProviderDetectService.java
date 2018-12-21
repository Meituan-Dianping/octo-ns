/*
 * Copyright 2018 Meituan Dianping. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.meituan.octo.mns.scanner.service;

import com.alibaba.fastjson.JSONObject;
import com.meituan.octo.mns.scanner.model.Provider;
import com.meituan.octo.mns.scanner.util.Constant;
import com.meituan.octo.mns.scanner.util.SocketDetector;
import com.meituan.octo.mns.scanner.util.heartbeat.HeartbeatDetector;
import org.apache.curator.framework.CuratorFramework;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;

import java.util.concurrent.Executor;
import java.util.regex.Pattern;

@Service
public class ProviderDetectService {
    private static final Logger log = LoggerFactory.getLogger(ProviderDetectService.class);
    private static final Pattern IP_PORT_PATTERN = Pattern.compile("\\d+.\\d+.\\d+.\\d+:\\d+");

    @Value("${zookeeper.basePath}")
    private String basePath;

    @Autowired
    private CuratorFramework zkClient;

    @Autowired
    private Executor aliveCheckExecutor;

    @Autowired
    private Executor deadCheckExecutor;

    @Autowired
    private ProviderUpdateService updateService;

    public void detect(long round, Provider provider) {
        if (IP_PORT_PATTERN.matcher(provider.getIpPort()).matches()) {
            JSONObject providerJson = getProviderJson(provider);
            if (providerJson != null) {
                int originalStatus = providerJson.getInteger("status");
                Executor executor = null;

                if (originalStatus == Constant.STATUS_STOPPED) {
                    log.info("round={}, detecting provider:{}, status:{}", round, provider, Constant.STATUS_STOPPED);
                    return;
                } else if (originalStatus == Constant.STATUS_ALIVE) {
                    executor = aliveCheckExecutor;
                } else {
                    executor = deadCheckExecutor;
                }

                executor.execute(() -> {
                    try {
                        Integer heartbeatType = providerJson.getInteger("heartbeatSupport");
                        if (heartbeatType == null) {
                            heartbeatType = 0;
                        }
                        int status;
                        if (Constant.HEARTBEAT_SUPPORT_SCANNER == heartbeatType
                                || Constant.HEARTBEAT_SUPPORT_BOTH == heartbeatType) {
                            status = HeartbeatDetector.detect(provider.getIp(), provider.getPort());
                        } else {
                            status = SocketDetector.detect(provider.getIp(), provider.getPort());
                        }
                        log.info("round={}, detecting provider:{}, status:{}", round, provider, status);
                        if (status != originalStatus) {
                            updateService.update(round, provider, providerJson, status);
                        }
                    } catch (Exception e) {
                        log.error("", e);
                    }
                });
            }
        } else {
            log.warn("invalid provider: {} ", provider);
        }
    }

    private JSONObject getProviderJson(Provider provider) {
        try {
            byte[] data = zkClient.getData().forPath(
                    basePath + "/" + provider.getEnv() + "/" + provider.getAppkey() + "/provider/" + provider
                            .getIpPort());
            return JSONObject.parseObject(new String(data));
        } catch (Exception e) {
            log.warn("get provider json exception:{}, message:{}", e.getClass(), e.getMessage());
        }
        return null;
    }
}
