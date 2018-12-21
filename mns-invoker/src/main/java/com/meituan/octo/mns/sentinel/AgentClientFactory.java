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

import com.meituan.octo.mns.Consts;
import com.meituan.octo.mns.model.AgentInfo;
import com.meituan.octo.mns.model.SGAgentClient;
import com.meituan.octo.mns.util.ProcessInfoUtil;
import com.octo.naming.service.thrift.model.ServiceAgent;
import org.apache.thrift.protocol.TBinaryProtocol;
import org.apache.thrift.protocol.TProtocol;
import org.apache.thrift.transport.TFramedTransport;
import org.apache.thrift.transport.TSocket;
import org.apache.thrift.transport.TTransportException;
import java.util.Collections;
import java.util.List;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


public class AgentClientFactory {
    private static final Logger LOG = LoggerFactory.getLogger(AgentClientFactory.class);
    private static SGAgentClient multiProtoClient = null;

    private static final int SGAGENT_PORT = ProcessInfoUtil.getSgagentPort();


    public static SGAgentClient borrowClient(SGAgentClient.ClientType type) {
        SGAgentClient client = null;
        switch (type) {
            case temp:
                client = getSGAgentClient(type);
                break;
            case multiProto:
                if (null == multiProtoClient) {
                    multiProtoClient = getSGAgentClient(type);
                }
                client = multiProtoClient;
                break;
            default:
                LOG.debug("");

        }
        return client;

    }

    public static void returnClient(SGAgentClient client) {
        if (client == null) {
            return;
        }
        switch (client.getType()) {
            case temp:
                client.destory();
                break;
            case multiProto:
                multiProtoClient = checkHealthLocality(client);
                break;
            default:
                break;
        }
    }

    public static SGAgentClient getSGAgentClient(SGAgentClient.ClientType type) {
        SGAgentClient client = null;
        List<AgentInfo> customizedList = CustomizedManager.getCustomizedAgentList();

        if (customizedList.isEmpty()) {
            client = createAgentClient(Consts.LOCALHOST, SGAGENT_PORT);
            if (null != client) {
                client.setType(type);
                return client;
            }
            List<AgentInfo> sentinelList = SentinelManager.getSentinelAgentList();

            Collections.shuffle(sentinelList);
            for (AgentInfo agentInfo : sentinelList) {
                client = createAgentClient(agentInfo.getIp(), agentInfo.getPort());
                if (null != client) {
                    client.setType(type);
                    break;
                }
            }
            if (null == client) {
                LOG.warn("SgAgent not available, check the network."
                        + " ip: " + ProcessInfoUtil.getLocalIpv4()
                        + "|env: " + ProcessInfoUtil.getAppEnv()
                        + "|sentinels:" + sentinelList.toString());
            }
        } else {
            for (AgentInfo agentInfo : customizedList) {
                client = createAgentClient(agentInfo.getIp(), agentInfo.getPort());
                if (null != client) {
                    client.setType(type);
                    break;
                }
            }
            if (null == client) {
                LOG.warn("SgAgent not available, check the network. The customized agent list is {}", customizedList.toString());
            }
        }
        return client;
    }

    private static SGAgentClient createAgentClient(String ip, int port) {
        SGAgentClient client = null;
        TSocket socket = new TSocket(ip, port, Consts.CONNECT_TIMEOUT);
        try {
            socket.setTimeout(Consts.DEFAULT_TIMEOUT_IN_MILLS);
            socket.open();
        } catch (TTransportException e) {
            //no log. because the log may make the RDs confused
            LOG.debug("fail to open socket to sg_agent.", e);
            return null;
        }

        TFramedTransport transport = new TFramedTransport(socket, Consts.DEFALT_MAX_RESPONSE_MESSAGE_BYTES);
        TProtocol protocol = new TBinaryProtocol(transport);
        ServiceAgent.Client iface = new ServiceAgent.Client(protocol);
        client = new SGAgentClient();
        client.setIface(iface);
        client.setTSocket(socket);

        return client;

    }

    /**
     * check health & locality; if not, try to recreate client
     */
    private static SGAgentClient checkHealthLocality(SGAgentClient client) {

        SGAgentClient newClient = client;
        if (null == client) {
            // TODO 这种情况下需要如何处理?
            LOG.warn("checkHealthLocality failed, client is null");
        } else if (null == client.getTSocket()) {
            newClient = getSGAgentClient(client.getType());
        } else if (!client.isLocal() && CustomizedManager.getCustomizedAgentList().isEmpty()) {
            //If local sg_agent is OK, using it to replace sg_sentinel and destorying the connection of sg_sentinel.
            SGAgentClient localClient = createAgentClient(Consts.LOCALHOST, SGAGENT_PORT);
            if ((null != localClient) && (null != localClient.getTSocket())) {
                client.destory();
                newClient = localClient;
                newClient.setType(client.getType());
            }
        }
        return newClient;
    }


}
