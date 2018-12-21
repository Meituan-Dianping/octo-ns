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

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import com.meituan.octo.mns.scanner.model.Provider;
import org.apache.curator.framework.CuratorFramework;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;

@Service
public class ProviderUpdateService {
    private static final Logger log = LoggerFactory.getLogger(ProviderUpdateService.class);
    @Value("${zookeeper.basePath}")
    private String basePath;

    @Autowired
    private CuratorFramework zkClient;

    public void update(long round, Provider provider, JSONObject providerJson, int status) {
        log.info("round={}, updating provider:{}, status:{}", round, provider, status);
        try {
            long lastUpdateTime = System.currentTimeMillis() / 1000;
            providerJson.put("status", status);
            providerJson.put("lastUpdateTime", lastUpdateTime);
            String providerPath =
                    basePath + "/" + provider.getEnv() + "/" + provider.getAppkey() + "/provider/" + provider
                            .getIpPort();
            byte[] data = JSON.toJSONBytes(providerJson);
            zkClient.setData().forPath(providerPath, data);
            String serverPath = basePath + "/" + provider.getEnv() + "/" + provider.getAppkey() + "/provider";
            byte[] serverPathData = zkClient.getData().forPath(serverPath);
            JSONObject serverPathJson = JSON.parseObject(new String(serverPathData));
            serverPathJson.put("lastUpdateTime", lastUpdateTime);
            zkClient.setData().forPath(serverPath, serverPathData);
        } catch (Exception e) {
            log.error("update provider exception:{}, message:{}", e.getClass(), e.getMessage());
        }
    }
}
