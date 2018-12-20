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
package com.meituan.octo.mns.model;

import com.meituan.octo.mns.Consts;
import com.octo.naming.service.thrift.model.ServiceAgent;
import org.apache.thrift.transport.TSocket;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class SGAgentClient {
    private static final Logger LOG = LoggerFactory.getLogger(SGAgentClient.class);
    private TSocket tSocket;
    private ServiceAgent.Iface iface;
    private ClientType type;

    public SGAgentClient() {
        this(ClientType.temp);
    }

    public SGAgentClient(ClientType type) {
        this.type = type;
    }

    public ClientType getType() {
        return type;
    }

    public void setType(ClientType type) {
        this.type = type;
    }

    public TSocket getTSocket() {
        return tSocket;
    }

    public void setTSocket(TSocket tSocket) {
        this.tSocket = tSocket;
    }

    public ServiceAgent.Iface getIface() {
        return iface;
    }

    public void setIface(ServiceAgent.Iface iface) {
        this.iface = iface;
    }

    public void destory() {
        if (null == this.tSocket) {
            return;
        }
        LOG.debug("destroy current tSocket:" + tSocket.getSocket().toString());
        tSocket.close();
        this.tSocket = null;
    }

    public boolean isLocal() {
        boolean local = false;
        try {
            String remoteSocketAddress = this.getTSocket().getSocket().getRemoteSocketAddress().toString();
            if (remoteSocketAddress.contains(Consts.LOCALHOST)) {
                local = true;
            }
        } catch (Exception e) {
            LOG.debug(e.getMessage(), e);
        }
        return local;
    }

    public enum ClientType {
        temp, multiProto
    }

}
