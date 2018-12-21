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

package com.meituan.octo.mns.scanner.util;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketException;
import java.net.SocketTimeoutException;

public class SocketDetector {
    private static final Logger log = LoggerFactory.getLogger(SocketDetector.class);

    public static int detect(String ip, int port) {
        Socket socket = new Socket();
        initSocket(socket);
        try {
            socket.connect(new InetSocketAddress(ip, port), Constant.timeout);
            return Constant.STATUS_ALIVE;
        } catch (SocketTimeoutException e) {
            return detectAgain(ip, port);
        } catch (Exception e) {
            log.warn("provider:{}:{}, exception:{}", ip, port, e.getMessage());
            return Constant.STATUS_DEAD;
        } finally {
            try {
                socket.close();
            } catch (IOException e) {
                //ignored
            }
        }
    }

    private static int detectAgain(String ip, int port) {
        Socket socket = new Socket();
        initSocket(socket);
        try {
            socket.connect(new InetSocketAddress(ip, port), Constant.timeout);
            return Constant.STATUS_ALIVE;
        } catch (Exception e) {
            log.warn("provider:{}:{}, exception:{}", ip, port, e.getMessage());
            return Constant.STATUS_DEAD;
        } finally {
            try {
                socket.close();
            } catch (IOException e) {
                //ignored
            }
        }
    }

    private static void initSocket(Socket socket) {
        try {
            socket.setSoLinger(false, Constant.STATUS_DEAD);
            socket.setTcpNoDelay(true);
            socket.setSoTimeout(Constant.timeout);
        } catch (SocketException e) {
            log.error("init socket error", e);
        }
    }
}
