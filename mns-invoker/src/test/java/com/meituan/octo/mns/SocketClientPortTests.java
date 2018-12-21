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

import junit.framework.Assert;
import org.apache.thrift.transport.TSocket;
import org.apache.thrift.transport.TTransportException;
import org.junit.Test;

import java.io.IOException;
import java.net.InetSocketAddress;

public class SocketClientPortTests {

    private int localPort = 40000;

    @Test
    public void clientPortConfig() throws IOException {
        TSocket socket = new TSocket("127.0.0.1", 5266, Consts.CONNECT_TIMEOUT);
        socket.getSocket().bind(new InetSocketAddress(localPort));
        try {
            socket.open();
            socket.setTimeout(Consts.DEFAULT_TIMEOUT_IN_MILLS);
            System.out.println(socket.getSocket());
            Assert.assertTrue(socket.getSocket().toString().contains(localPort + ""));
        } catch (TTransportException e) {
            e.printStackTrace();
        } finally {
            socket.close();
        }

    }
}
