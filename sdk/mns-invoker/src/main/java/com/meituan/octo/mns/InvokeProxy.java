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
package com.meituan.octo.mns;

import com.meituan.octo.mns.model.SGAgentClient;
import com.meituan.octo.mns.sentinel.AgentClientFactory;
import com.octo.naming.service.thrift.model.ServiceAgent;
import org.apache.thrift.transport.TTransportException;
import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;
import java.util.Arrays;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class InvokeProxy implements InvocationHandler {
    private static final Logger LOG = LoggerFactory.getLogger(InvokeProxy.class);
    private final SGAgentClient.ClientType type;
    private static boolean isMock = false;
    private static Object mockValue;

    //lock
    private static final Object MULTI_PROTO_LOCK = new Object();

    public static void setIsMock(boolean isMock) {
        InvokeProxy.isMock = isMock;
    }

    public static void setMockValue(Object mock) {
        mockValue = mock;
    }

    public InvokeProxy(SGAgentClient.ClientType clientType) {
        this.type = clientType;
    }

    public SGAgentClient.ClientType getType() {
        return type;
    }

    public ServiceAgent.Iface getProxy() {
        return (ServiceAgent.Iface) Proxy.newProxyInstance(ServiceAgent.class.getClassLoader(),
                ServiceAgent.Client.class.getInterfaces(), this);
    }

    @Override
    public Object invoke(Object proxy, Method method, Object[] args) {
        Object result = null;

        // to ensure proper use of locks, do not optimized the redundant case codes.
        switch (type) {
            case multiProto:
                synchronized (MULTI_PROTO_LOCK) {
                    result = doInvoker(proxy, method, args);
                }
                break;
            default:
                // no lock for temp sg_agent connection.
                result = doInvoker(proxy, method, args);
                break;
        }


        return result;
    }

    private Object doInvoker(Object proxy, Method method, Object[] args) {
        Object result = null;

        // get the sg_agent connection
        SGAgentClient client = AgentClientFactory.borrowClient(type);

        try {
            result = isMock ? mockValue : method.invoke(client.getIface(), args);

        } catch (Exception e) {
            LOG.debug("Invoker Exception, method: {} args: {}", method.getName(), Arrays.toString(args));
            LOG.debug(e.getMessage(), e);

            if (e.getCause() instanceof TTransportException) {
                client.destory();
            }

        } finally {
            // return the sg_agent connection
            AgentClientFactory.returnClient(client);
        }
        return result;
    }


}