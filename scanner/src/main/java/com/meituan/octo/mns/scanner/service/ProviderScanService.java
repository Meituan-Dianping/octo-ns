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

import com.meituan.octo.mns.scanner.model.Provider;
import org.apache.curator.framework.CuratorFramework;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Service;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Objects;
import java.util.Set;
import java.util.concurrent.atomic.AtomicLong;
import java.util.regex.Pattern;
import java.util.stream.Collectors;
import java.util.stream.Stream;

@Service
public class ProviderScanService {
    private static final Logger log = LoggerFactory.getLogger(ProviderScanService.class);
    private static final Pattern IP_PORT_PATTERN = Pattern.compile("\\d+.\\d+.\\d+.\\d+:\\d+");
    private final AtomicLong scanRoundCounter = new AtomicLong(1);
    private final String[] ENV_LIST = new String[] { "prod", "stage", "test" };
    @Value("${zookeeper.basePath}")
    private String basePath;

    @Autowired
    private CuratorFramework zkClient;

    @Autowired
    private ProviderDetectService detectService;

    @Scheduled(fixedRate = 5000)
    public void scanZk() {
        try {
            long start = System.currentTimeMillis();
            long round = scanRoundCounter.getAndIncrement();
            Set<String> appkeys = getAllAppkeys();
            appkeys.parallelStream().forEach(appkey -> {
                log.info("round={}, scan appkey:{}", round, appkey);
                try {
                    List<Provider> providers = getProviders(appkey);
                    for (Provider provider : providers) {
                        try {
                            detectService.detect(round, provider);
                        } catch (Exception e) {
                            log.error("scan provider:{} exception:{}, message:{}", provider, e.getCause(),
                                    e.getMessage());
                        }
                    }
                } catch (Exception e) {
                    log.error("scan appkey:{} exception:{}, message:{}", appkey, e.getCause(), e.getMessage());
                }
            });
            long end = System.currentTimeMillis();
            log.info("round={}, scan {} appkeys cost {} ms", round, appkeys.size(), (end - start));
        } catch (Exception e) {
            log.error("scan zk exception:{}, message:{}", e.getCause(), e.getMessage());
        }
    }

    private Set<String> getAllAppkeys() {
        return Arrays.stream(ENV_LIST).flatMap(env -> {
            try {
                return zkClient.getChildren().forPath(basePath + "/" + env).stream();
            } catch (Exception e) {
                return Stream.empty();
            }
        }).collect(Collectors.toSet());
    }

    private List<Provider> getProviders(String appkey) {
        try {
            return Arrays.stream(ENV_LIST).flatMap(env -> {
                try {
                    return zkClient.getChildren().forPath(basePath + "/" + env + "/" + appkey + "/provider").stream()
                            .map(ipPort -> {
                                if (!IP_PORT_PATTERN.matcher(ipPort).matches()) {
                                    return null;
                                }
                                String ip = ipPort.split(":")[0];
                                int port = Integer.parseInt(ipPort.split(":")[1]);
                                Provider provider = new Provider();
                                provider.setIp(ip);
                                provider.setPort(port);
                                provider.setAppkey(appkey);
                                provider.setEnv(env);
                                return provider;
                            }).filter(Objects::nonNull);
                } catch (Exception e) {
                    return null;
                }
            }).filter(Objects::nonNull).collect(Collectors.toList());
        } catch (Exception e) {
            return Collections.emptyList();
        }
    }
}
