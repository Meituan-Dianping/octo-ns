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

import com.meituan.octo.mns.util.IpUtil;
import com.octo.idc.model.Idc;

public class IDC {
    private String ip;
    private String mask;
    private Idc idcinfo;

    private int intMask;
    private int ipMaskValue;
    private boolean isInit = false;
    //private Object lock = new Object();

    public void init() {
        if (!isInit) {
            synchronized (this) {
                if (!isInit) {
                    intMask = convertMaskToInt(mask);
                    ipMaskValue = intMask & IpUtil.getIpv4Value(ip);
                    isInit = true;
                }
            }
        }
    }

    int convertMaskToInt(String mask) {
        String[] vnum = mask.split("\\.");
        if (4 != vnum.length) {
            return -1;
        }

        int iMask = 0;
        for (int i = 0; i < vnum.length; ++i) {
            iMask += (Integer.parseInt(vnum[i]) << ((3 - i) * 8));
        }
        return iMask;
    }

    public Idc getIdcinfo() {
        return idcinfo;
    }

    public void setIdcinfo(Idc idcinfo) {
        this.idcinfo = idcinfo;
    }

    public String getIp() {
        return ip;
    }

    public void setIp(String ip) {
        this.ip = ip;
    }

    public void setMask(String mask) {
        this.mask = mask;
    }

    public int getIntMask() {
        return intMask;
    }

    public int getIpMaskValue() {
        return ipMaskValue;
    }
}
