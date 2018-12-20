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
package com.meituan.octo.mns.sentinel;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.meituan.octo.mns.Consts;
import com.meituan.octo.mns.model.AgentInfo;
import com.meituan.octo.mns.util.HttpUtil;
import com.meituan.octo.mns.util.ProcessInfoUtil;
import org.apache.commons.lang3.StringUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

public class SentinelManager {
    private static final Logger LOG = LoggerFactory.getLogger(SentinelManager.class);
    private static List<AgentInfo> sentinelAgentList = new ArrayList<AgentInfo>();
    private static final Object LOCK = new Object();

    private SentinelManager() {

    }

    public static void initSentinels() {
        String env = ProcessInfoUtil.getOctoEnv();
        String ip = ProcessInfoUtil.getLocalIpv4();
        StringBuilder sb = new StringBuilder(Consts.GET_SERVER_LIST_API);
        sb.append("ip=" + ip)
                .append("&env=" + env)
                .append("&appkey=" + ProcessInfoUtil.getSgsentinelAppkey());
        String result = HttpUtil.get(sb.toString());
        if (StringUtils.isEmpty(result)) {
            LOG.warn("failed to get sg_sentinel from {}", Consts.GET_SERVER_LIST_API);
            return;
        }

        try {
            ObjectMapper mapper = new ObjectMapper();
            JsonNode rootNode = mapper.readTree(result);
            JsonNode serviceList = rootNode.path("data").path("serviceList");
            if (null != serviceList) {
                sentinelAgentList.clear();
                Iterator<JsonNode> iter = serviceList.elements();
                while (iter.hasNext()) {
                    JsonNode serviceNode = iter.next();
                    sentinelAgentList.add(new AgentInfo(serviceNode.path("ip").textValue(),
                            serviceNode.path("port").intValue(), ProcessInfoUtil.getSgsentinelAppkey(), env));
                }
            }
        } catch (Exception e) {
            LOG.warn(e.getMessage(), e);
        }
    }

    public static List<AgentInfo> getSentinelAgentList() {
        if (sentinelAgentList.isEmpty()) {
            synchronized (LOCK) {
                if (sentinelAgentList.isEmpty()) {
                    initSentinels();
                }
            }
        }
        return sentinelAgentList;
    }
}
