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
package com.meituan.octo.mns.util;

import com.meituan.octo.mns.exception.MnsException;
import com.meituan.octo.mns.model.HostEnv;
import com.octo.idc.model.Idc;
import org.apache.commons.lang3.StringUtils;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.*;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class ProcessInfoUtil {

    private static final Logger LOG = LoggerFactory.getLogger(ProcessInfoUtil.class);

    private static final String LOCAL_IPV4 = getIpV4();

    private static final HostEnv APP_ENV = ProcessConfig.getHostEnv();
    // octo inner env
    private static final String OCTO_ENV_STR = parseOctoEnv();

    public static String getAppEnv() {
        return APP_ENV.name();
    }

    public static String getOctoEnv() {
        return OCTO_ENV_STR;
    }

    private static String parseOctoEnv() {
        String env = "prod";
        if (ProcessConfig.isValidConfig()) {
            switch (APP_ENV) {
                case PROD:
                    env = "prod";
                    break;
                case STAGE:
                    env = "stage";
                    break;
                case PPE:
                    env = "stage";
                    break;
                case DEV:
                    env = "prod";
                    break;
                case TEST:
                    env = "test";
                    break;
                default:
                    env = "prod";
                    break;

            }
        }
        return env;
    }

    private static String getIpV4() {
        String ip = "";
        Enumeration<NetworkInterface> networkInterface;
        try {
            networkInterface = NetworkInterface.getNetworkInterfaces();
        } catch (SocketException e) {
            LOG.error("fail to get network interface information.", e);
            return ip;
        }
        Set<String> ips = new HashSet<String>();
        while (networkInterface.hasMoreElements()) {
            NetworkInterface ni = networkInterface.nextElement();

            Enumeration<InetAddress> inetAddress = null;
            try {
                if (null != ni) {
                    inetAddress = ni.getInetAddresses();
                }
            } catch (Exception e) {
                LOG.debug("fail to get ip information.", e);
            }
            while (null != inetAddress && inetAddress.hasMoreElements()) {
                InetAddress ia = inetAddress.nextElement();
                if (ia instanceof Inet6Address) {
                    continue; // ignore ipv6
                }
                String thisIp = ia.getHostAddress();
                // 排除 回送地址
                if (!ia.isLoopbackAddress() && !thisIp.contains(":") && !"127.0.0.1".equals(thisIp)) {
                    ips.add(thisIp);
                    if (StringUtils.isEmpty(ip)) {
                        ip = thisIp;
                    }
                }
            }
        }

        if (ips.size() >= 2) {
            ip = (String) ips.toArray()[0];
        }

        if (StringUtils.isEmpty(ip)) {
            LOG.error("cannot get local ip.");
            ip = "";
        }

        return ip;
    }

    public static String getLocalIpv4() {
        return LOCAL_IPV4;
    }

    public static boolean isLocalHostOnline() {
        return HostEnv.PROD == APP_ENV || HostEnv.STAGE == APP_ENV;
    }

    public static List<Idc> getIdcs() {
        return IpUtil.getAllIdcs();
    }

    public static Map<String, Idc> getIdcInfo(List<String> ips) throws MnsException {
        Map<String, Idc> resIdcInfo = new HashMap<String, Idc>();
        if (null == ips || ips.isEmpty()) {
            return resIdcInfo;
        }

        if (IpUtil.checkAndloadIdcXmlIfNecessary()) {
            resIdcInfo.putAll(IpUtil.getIdcInfoFromLocal(ips));
        }

        return resIdcInfo;
    }

    public static String getMnsZKUrl() {
        return ProcessConfig.getMnsZkUrl();
    }

    public static String getMnsUrl() {
        return ProcessConfig.getMnsUrl();
    }

    public static String getIdcPath() {
        return ProcessConfig.getIdcPath();
    }

    public static int getSgagentPort() {
        return ProcessConfig.getSgagentPort();
    }

    public static String getSgagentAppkey() {
        return ProcessConfig.getSgagentAppkey();
    }

    public static String getSgsentinelAppkey() {
        return ProcessConfig.getSgsentinelAppkey();
    }

}