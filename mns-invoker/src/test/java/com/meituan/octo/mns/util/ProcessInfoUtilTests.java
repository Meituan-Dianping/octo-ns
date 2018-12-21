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
import com.octo.idc.model.Idc;
import org.junit.Assert;
import org.junit.Test;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import static org.junit.Assert.*;

public class ProcessInfoUtilTests {
    private static Logger LOG = LoggerFactory.getLogger(ProcessInfoUtilTests.class);

    @Test
    public void getIdcs() {

        List<Idc> idcs = ProcessInfoUtil.getIdcs();
        Assert.assertTrue(!idcs.isEmpty());
        List<IDCCheck> idcChecks = new ArrayList<IDCCheck>();

        idcChecks.add(new IDCCheck("DX", "BJ1", "beijing", false));
        idcChecks.add(new IDCCheck("YF", "BJ2", "beijing", false));
        idcChecks.add(new IDCCheck("CQ", "BJ1", "beijing", false));
        idcChecks.add(new IDCCheck("GQ", "SH", "shanghai", false));
        idcChecks.add(new IDCCheck("GH", "BJ1", "beijing", false));

        if (!idcs.isEmpty()) {
            for (Idc idc : idcs) {
                for (IDCCheck check : idcChecks) {
                    if (check.getIdcName().equalsIgnoreCase(idc.getIdc())
                            && check.getCenter().equalsIgnoreCase(idc.getCenter())
                            && check.getRegion().equalsIgnoreCase(idc.getRegion())) {
                        check.setExist(true);
                    }
                }
            }
            for (IDCCheck check : idcChecks) {
                Assert.assertTrue(check.isExist());
            }
        }


        // re-check
        idcs = ProcessInfoUtil.getIdcs();
        Assert.assertTrue(!idcs.isEmpty());

    }

    @Test
    public void getIp() {
        Assert.assertTrue(IpUtil.checkIP(ProcessInfoUtil.getLocalIpv4()));
    }

    @Test
    public void getHostInfo() {
        String ip = ProcessInfoUtil.getLocalIpv4(); // 获取本地内网 IP
        boolean isLocalOnline = ProcessInfoUtil.isLocalHostOnline(); //无需参数, 直接判断本机是否 Online
    }


    @Test
    public void testIdcInfoYF() throws IOException {
        List<String> ips = new ArrayList<String>();
        String IP = "10.4.243.121";
        ips.add(IP);
        try {
            Map<String, Idc> idcs = ProcessInfoUtil.getIdcInfo(ips);
            assertEquals(1, idcs.size());
            Idc idc = idcs.get(IP);
            assertEquals("YF", idc.idc);
            assertEquals("beijing", idc.region);
        } catch (MnsException e) {
            e.printStackTrace();
        }
    }


    @Test
    public void testIdcInfoGQ() throws IOException {
        List<String> ips = new ArrayList<String>();
        String IP = "10.67.243.121";
        ips.add(IP);
        ips.add(IP);
        try {
            Map<String, Idc> idcs = ProcessInfoUtil.getIdcInfo(ips);
            assertEquals(1, idcs.size());
            Idc idc = idcs.get(IP);
            assertEquals("GQ", idc.idc);
            assertEquals("shanghai", idc.region);
        } catch (MnsException e) {
            e.printStackTrace();
        }

    }

    @Test
    public void testIdcInfoUnkown() throws IOException {
        String IP = "10.44132.243.1212312";
        List<String> ips = new ArrayList<String>();
        ips.add(IP);
        try {
            Map<String, Idc> idcs = ProcessInfoUtil.getIdcInfo(ips);
            assertEquals(1, idcs.size());
        } catch (MnsException e) {
            e.printStackTrace();
        }
    }


    @Test
    public void testIP() {
        assertTrue(IpUtil.checkIP("10.4.245.3"));
        assertFalse(IpUtil.checkIP("0.4.245.3"));
        assertFalse(IpUtil.checkIP(".4.245.3"));
        assertFalse(IpUtil.checkIP("10.4.245."));
        assertFalse(IpUtil.checkIP("10.4.2453"));
        assertFalse(IpUtil.checkIP("10.4.s.1"));
        assertFalse(IpUtil.checkIP("10.4.4,1"));
        assertFalse(IpUtil.checkIP("10.4.245.2223"));
        assertFalse(IpUtil.checkIP(null));
        assertFalse(IpUtil.checkIP(""));
        System.out.println("IpUtil.checkIP pass.");
    }

    @Test
    public void testOctoCfg() throws IOException {
        System.out.println(ProcessInfoUtil.getIdcPath());
        System.out.println(ProcessInfoUtil.getSgagentPort());
        System.out.println(ProcessInfoUtil.getSgagentAppkey());
        System.out.println(ProcessInfoUtil.getSgsentinelAppkey());
        System.out.println(ProcessInfoUtil.getMnsUrl());
        System.out.println(ProcessInfoUtil.getMnsZKUrl());
        System.out.println(ProcessInfoUtil.getAppEnv());
        /*assertEquals(ProcessInfoUtil.getIdcPath(), "/octo/namingservice/chrion/idc.xml");
        assertEquals(ProcessInfoUtil.getSgagentPort(), 5266);
        assertEquals(ProcessInfoUtil.getSgagentAppkey(), "com.sankuai.inf.sgagent");
        assertEquals(ProcessInfoUtil.getSgsentinelAppkey(), "com.sankuai.inf.sg_sentinel");*/
    }

    static class IDCCheck {
        private String idcName;
        private String center;
        private String region;
        private boolean exist;


        public IDCCheck(String idcName, String center, String region, boolean exist) {
            this.idcName = idcName;
            this.center = center;
            this.region = region;
            this.exist = exist;
        }

        public String getIdcName() {
            return idcName;
        }

        public void setIdcName(String idcName) {
            this.idcName = idcName;
        }

        public boolean isExist() {
            return exist;
        }

        public void setExist(boolean exist) {
            this.exist = exist;
        }

        public String getCenter() {
            return center;
        }

        public void setCenter(String center) {
            this.center = center;
        }

        public String getRegion() {
            return region;
        }

        public void setRegion(String region) {
            this.region = region;
        }
    }
}
