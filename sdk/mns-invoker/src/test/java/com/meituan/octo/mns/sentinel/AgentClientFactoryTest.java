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


import com.meituan.octo.mns.model.SGAgentClient;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;


public class AgentClientFactoryTest {
    @Before
    public void setCustomizedSgagent() {
        CustomizedManager.setCustomizedSGAgents("10.22.23.29:5266");
    }

    @Test
    public void testConnection() {
        checkConnection(SGAgentClient.ClientType.multiProto, true);
        checkConnection(SGAgentClient.ClientType.temp, false);
    }

    private void checkConnection(SGAgentClient.ClientType type, boolean isLong) {
        SGAgentClient client = AgentClientFactory.borrowClient(type);
        Assert.assertNotNull(client);
        Assert.assertNotNull(client.getTSocket());

        AgentClientFactory.returnClient(client);
        Assert.assertNotNull(client);
        if (isLong) {
            Assert.assertNotNull(client.getTSocket());
        } else {
            Assert.assertNull(client.getTSocket());
        }
        Assert.assertEquals(type, client.getType());
    }
}
