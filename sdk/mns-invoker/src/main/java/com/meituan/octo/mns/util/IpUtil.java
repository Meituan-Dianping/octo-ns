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

import com.meituan.octo.mns.Consts;
import com.meituan.octo.mns.model.IDC;
import com.octo.idc.model.Idc;
import org.apache.commons.lang3.StringUtils;
import org.apache.commons.lang3.math.NumberUtils;
import java.io.File;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.ReentrantReadWriteLock;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class IpUtil {
    private static final Logger LOG = LoggerFactory.getLogger(IpUtil.class);
    private static final String IP_REGEX = "([1-9]|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])(\\.(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])){3}";


    private static final Object LOCK = new Object();
    private static final ReentrantReadWriteLock IDC_XML_LOCK = new ReentrantReadWriteLock();

    private static boolean isInitSGAgentIDCXml = false;
    private static boolean idcXmlValid = false;

    private static long idcLastModifiedTime = 0;
    // the exposed idc class in idl-common
    private static List<Idc> idcxml = new ArrayList<Idc>();

    private static Map<Integer, IDC> idcMap = null;
    private static Set<Integer> maskSet = null;

    private static DefaultIdcParser idcParser = new DefaultIdcParser();

    public static ScheduledExecutorService mnsCommonSchedule = Executors.newSingleThreadScheduledExecutor(new ScheduleTaskFactory("MnsInvoker-Schedule"));

    static {
        mnsCommonSchedule.scheduleWithFixedDelay(new Runnable() {
            @Override
            public void run() {
                loadIdcTask();
            }
        }, 0, 10, TimeUnit.MINUTES);
    }

    IpUtil() {
    }

    private static void loadIdcTask() {
        String idcPath = ProcessInfoUtil.getIdcPath();
        File file = new File(idcPath);
        if (file.exists() && idcLastModifiedTime != file.lastModified()) {
            List<IDC> idcs = new ArrayList<IDC>();
            boolean ret = idcParser.initIdcXml(idcPath, idcs);
            if (ret) {
                if (!idcs.isEmpty()) {
                    refreshIdcData(idcs);
                    LOG.info("success to reload {}", idcPath);
                    for (IDC idc : idcs) {
                        LOG.info("region = {}, center = {}, idc = {}, ip = {}", idc.getIdcinfo().getRegion(), idc.getIdcinfo().getCenter(), idc.getIdcinfo().getIdc(), idc.getIp());
                    }
                    idcLastModifiedTime = file.lastModified();
                    idcXmlValid = true;
                } else {
                    LOG.info("the {} is empty, not as expected, ignore it", idcPath);
                }
            } else {
                LOG.error("fail to parse {}", idcPath);
            }
        }
    }

    private static void refreshIdcData(List<IDC> idcs) {
        List<Idc> idcXmlNew = new ArrayList<Idc>();
        Map<Integer, IDC> idcTmp = new ConcurrentHashMap<Integer, IDC>();
        Set<Integer> maskTmp = new HashSet<Integer>();

        for (IDC idc : idcs) {
            Idc idcXmlItem = new Idc();
            idcXmlItem.setRegion(idc.getIdcinfo().getRegion())
                    .setCenter(idc.getIdcinfo().getCenter())
                    .setIdc(idc.getIdcinfo().getIdc());
            idcXmlNew.add(idcXmlItem);

            idcTmp.put(idc.getIpMaskValue(), idc);
            maskTmp.add(idc.getIntMask());
        }

        IDC_XML_LOCK.writeLock().lock();
        idcMap = idcTmp;
        maskSet = maskTmp;
        idcxml = idcXmlNew;
        IDC_XML_LOCK.writeLock().unlock();
    }

    public static void setIsInitSGAgentIDCXml(boolean isInitSGAgentIDCXml) {
        IpUtil.isInitSGAgentIDCXml = isInitSGAgentIDCXml;
    }


    static List<Idc> getAllIdcs() {
        List<Idc> ret = null;

        if (checkAndloadIdcXmlIfNecessary()) {
            // local has the idc.xml, try to reload cache from local.
            IDC_XML_LOCK.readLock().lock();
            ret = idcxml;
            IDC_XML_LOCK.readLock().unlock();
        }
        return ret;
    }


    static boolean checkAndloadIdcXmlIfNecessary() {
        if (!isInitSGAgentIDCXml) {
            synchronized (LOCK) {
                if (!isInitSGAgentIDCXml) {
                    List<IDC> idcs = new ArrayList<IDC>();
                    idcXmlValid = idcParser.initIdcXml(ProcessInfoUtil.getIdcPath(), idcs);
                    if (idcXmlValid) {
                        refreshIdcData(idcs);
                    }
                }
            }
        }
        return idcXmlValid;
    }


    public static int getIpv4Value(String ip) {
        if (StringUtils.isEmpty(ip)) {
            return -1;
        }
        String[] vcIp = ip.split("\\.");
        if (4 != vcIp.length) {
            return -1;
        }

        int address = 0;
        int filteNum = 0xFF;
        for (int i = 0; i < 4; ++i) {

            int pos = i * 8;
            if (!NumberUtils.isDigits(vcIp[3 - i])) {
                return -1;
            }
            int vIp = -1;
            try {
                vIp = Integer.parseInt(vcIp[3 - i]);
            } catch (NumberFormatException e) {
                //invalid num.
                return -1;
            }

            if (vIp > 255 || vIp < 0) {
                return -1;
            }
            address |= ((vIp << pos) & (filteNum << pos));
        }
        return address;
    }

    private static Idc handleIdcInfoFromLocal(String ip) {
        try {
            IDC_XML_LOCK.readLock().lock();
            for (Integer mask : maskSet) {
                IDC curIDC = idcMap.get(mask & getIpv4Value(ip));
                if (curIDC != null && curIDC.getIntMask() == mask) {
                    return curIDC.getIdcinfo();
                }
            }
            Idc resIdc = new Idc();
            resIdc.setIdc(Consts.UNKNOWN).setRegion(Consts.UNKNOWN).setCenter(Consts.UNKNOWN);
            return resIdc;
        } catch (Exception e) {
            LOG.error("failed to handleIdcInfoFromLocal, ip: {}", ip, e);
            Idc resIdc = new Idc();
            resIdc.setIdc(Consts.UNKNOWN).setRegion(Consts.UNKNOWN).setCenter(Consts.UNKNOWN);
            return resIdc;
        } finally {
            IDC_XML_LOCK.readLock().unlock();
        }
    }

    public static Map<String, Idc> getIdcInfoFromLocal(List<String> ips) {
        Map<String, Idc> resIdcInfo = new HashMap<String, Idc>();
        if (!checkAndloadIdcXmlIfNecessary() || null == ips) {
            return resIdcInfo;
        }

        for (String ip : ips) {
            if (null != resIdcInfo.get(ip)) {
                //if this ip was already parsed, ignore.
                continue;
            }
            Idc idc = IpUtil.handleIdcInfoFromLocal(ip);
            if (null != idc) {
                resIdcInfo.put(ip, idc);
            }
        }
        return resIdcInfo;
    }

    //because of the use of regular expression matching, be careful of its performance while you call it with high concurrency.
    public static boolean checkIP(String ip) {
        return StringUtils.isNotEmpty(ip) && ip.matches(IP_REGEX);
    }

}
