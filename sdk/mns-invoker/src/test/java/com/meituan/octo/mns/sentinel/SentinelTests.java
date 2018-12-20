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
import com.meituan.octo.mns.util.ProcessInfoUtil;
import org.junit.Assert;
import org.junit.Test;

import java.util.List;

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

public class SentinelTests {
    @Test
    public void testGetSentinelAgentList() throws InterruptedException {
        for (int i = 0; i < 10; i++) {
            Thread thread = new Thread(new Runnable() {
                @Override
                public void run() {
                    String strSentinelList = SentinelManager.getSentinelAgentList().toString();
                    assertNotNull(strSentinelList);
                    assertTrue(strSentinelList.contains(ProcessInfoUtil.getSgsentinelAppkey()));
                }
            });
            thread.start();
        }
        Thread.sleep(100000);
    }

    @Test
    public void testCustomizedSGAgents() {
        CustomizedManager.setCustomizedSGAgents("10.1.1.1:5266,10.1.1.2:5266");
        List<AgentInfo> agentList = CustomizedManager.getCustomizedAgentList();
        Assert.assertFalse(agentList.isEmpty());

        CustomizedManager.setCustomizedSGAgents("10.1.1.1:5266, 10.1.1.2:5266");
        agentList.clear();
        agentList = CustomizedManager.getCustomizedAgentList();
        Assert.assertTrue(agentList.isEmpty());

        CustomizedManager.setCustomizedSGAgents("10.1.1.1,10.1.1.2");
        agentList.clear();
        agentList = CustomizedManager.getCustomizedAgentList();
        Assert.assertTrue(agentList.isEmpty());
    }
}
