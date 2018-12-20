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

import com.meituan.octo.mns.model.IDC;
import com.octo.idc.model.Idc;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;


public class IpUtilTest {
    private static final String rootPath = IpUtilTest.class.getResource("/").getFile().toString();
    private static String UNKNOWN = "unknown";
    private static String SHANGHAI = "shanghai";
    private static String BEIJING = "beijing";
    private static String YF = "YF";
    private static String DX = "DX";
    private static String GQ = "GQ";

    private static String BJ1 = "BJ1";
    private static String BJ2 = "BJ2";
    private static String SH = "SH";

    private class CheckInfo {
        String ip;
        String region;
        String idc;
        String center;

        public CheckInfo(String ip, String region, String idc, String center) {
            this.ip = ip;
            this.region = region;
            this.idc = idc;
            this.center = center;
        }
    }

    private static List<CheckInfo> infoList = new ArrayList<CheckInfo>();
    private static List<String> ips = new ArrayList<String>();

    @Before
    public void initBeforeClass() {

        infoList.clear();
        ips.clear();
        infoList.add(new CheckInfo("10.4.245.3", BEIJING, YF, BJ2));
        infoList.add(new CheckInfo("10.32.245.34", BEIJING, DX, BJ1));
        infoList.add(new CheckInfo("10.4.2433333333.121", UNKNOWN, UNKNOWN, UNKNOWN));

        infoList.add(new CheckInfo("10.4.2s.121", UNKNOWN, UNKNOWN, UNKNOWN));
        infoList.add(new CheckInfo("10.69.23.43", SHANGHAI, GQ, SH));

        infoList.add(new CheckInfo(null, UNKNOWN, UNKNOWN, UNKNOWN));
        infoList.add(new CheckInfo("", UNKNOWN, UNKNOWN, UNKNOWN));

        for (CheckInfo item : infoList) {
            ips.add(item.ip);
        }
    }


    @Test
    public void testGetIdcInfoFromLocal() throws Exception{
        List<IDC> idcTemp = new ArrayList<IDC>();
        new DefaultIdcParser().initIdcXml(ProcessInfoUtil.getIdcPath(), idcTemp);

        Map<String, Idc> idcs = IpUtil.getIdcInfoFromLocal(ips);
        check(idcs, true);


        ExecutorService executor = Executors.newFixedThreadPool(10);
        for (int i = 0; i < 10; i++) {
            executor.submit(new Runnable() {
                @Override
                public void run() {
                    List<Idc> allIdcs = IpUtil.getAllIdcs();
                    Assert.assertTrue(allIdcs != null && !allIdcs.isEmpty());
                }
            });
        }

        Thread.sleep(10000);
    }

    @Test
    public void testCheckAndloadIdcXmlIfNecessary() {
        Assert.assertTrue(IpUtil.checkAndloadIdcXmlIfNecessary());
    }

    @Test
    public void testCheckIP() {
        Assert.assertFalse(IpUtil.checkIP("100.2.2.1q"));
        Assert.assertFalse(IpUtil.checkIP("257.2.2.10"));
        Assert.assertFalse(IpUtil.checkIP("1-1.2.2.1"));
        Assert.assertFalse(IpUtil.checkIP("100.2.2"));
        Assert.assertTrue(IpUtil.checkIP("100.2.2.1"));
    }

    private void check(Map<String, Idc> map, boolean isCheckCenter) {
        for (CheckInfo item : infoList) {
            Idc idc = map.get(item.ip);
            if (null == item.ip || null == idc) {
                continue;
            }
            Assert.assertEquals(item.region, idc.getRegion());
            if (isCheckCenter) {
                Assert.assertEquals(item.center, idc.getCenter());
            }
            Assert.assertEquals(item.idc, idc.getIdc());
        }

    }
}
