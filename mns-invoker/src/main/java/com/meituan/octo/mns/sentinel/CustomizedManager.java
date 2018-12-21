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

import com.meituan.octo.mns.model.AgentInfo;
import com.meituan.octo.mns.util.IpUtil;
import com.meituan.octo.mns.util.ProcessInfoUtil;
import org.apache.commons.lang3.StringUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class CustomizedManager {
    private static final Logger LOG = LoggerFactory.getLogger(CustomizedManager.class);
    private static List<AgentInfo> customizedAgentList = new ArrayList<AgentInfo>();

    private CustomizedManager() {
    }

    public static List<AgentInfo> getCustomizedAgentList() {
        return customizedAgentList;
    }

    //not recommended to use
    public static void setCustomizedSGAgents(String customizedAgentStr) {
        customizedAgentList.clear();
        if (StringUtils.isEmpty(customizedAgentStr)) {
            LOG.warn("The customized agent list is empty. customizedAgentStr={}", customizedAgentStr);
            return;
        }
        String[] strArray = customizedAgentStr.split(",");
        List<String> errorAgentList = new ArrayList<String>();
        for (String item : strArray) {
            handleAgentIpPort(item, errorAgentList);
        }
        if (!errorAgentList.isEmpty()) {
            LOG.error("Invalid customized agent list, {}", errorAgentList.toString());
        }

        Collections.shuffle(customizedAgentList);
    }

    private static void handleAgentIpPort(String ipPortStr, List<String> errorList) {
        if (!ipPortStr.contains(":")) {
            errorList.add(ipPortStr);
            return;
        }
        String[] ipPort = ipPortStr.split(":");
        if (2 != ipPort.length) {
            errorList.add(ipPortStr);
            return;
        }
        String cusIp = ipPort[0];
        if (!IpUtil.checkIP(cusIp)) {
            errorList.add(ipPortStr);
            return;
        }
        int port = Integer.parseInt(ipPort[1]);
        if (port < Math.pow(2, 10) || port > Math.pow(2, 16)) {
            errorList.add(ipPortStr);
            return;
        }
        boolean isExist = false;
        for (AgentInfo info : customizedAgentList) {
            if (info.getIp().equals(cusIp) && info.getPort() == port) {
                isExist = true;
                break;
            }
        }
        if (!isExist) {
            AgentInfo item = new AgentInfo(cusIp, port, ProcessInfoUtil.getSgagentAppkey(), "");
            customizedAgentList.add(item);
        }
    }
}
