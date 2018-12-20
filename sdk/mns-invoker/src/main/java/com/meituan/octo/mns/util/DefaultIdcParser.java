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
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import java.io.File;
import java.util.List;

public class DefaultIdcParser implements AbractIdcParser {
    private static final Logger LOG = LoggerFactory.getLogger(DefaultIdcParser.class);

    @Override
    public boolean initIdcXml(String filePath, List<IDC> idcResult) {
        boolean ret = false;
        try {
            // make sure that the cache is empty.
            idcResult.clear();
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            factory.setIgnoringElementContentWhitespace(true);
            DocumentBuilder builder = factory.newDocumentBuilder();
            Document xml = builder.parse(new File(filePath));
            NodeList regionList = xml.getElementsByTagName("Region");

            for (int regionIter = 0; regionIter < regionList.getLength(); ++regionIter) {
                Element region = (Element) regionList.item(regionIter);

                NodeList regionNames = region.getElementsByTagName("RegionName");
                String regionName = regionNames.getLength() > 0 ? regionNames.item(0).getFirstChild().getNodeValue() : Consts.UNKNOWN;

                NodeList idcs = region.getElementsByTagName("IDC");
                for (int idcIter = 0; idcIter < idcs.getLength(); ++idcIter) {
                    Element idc = (Element) idcs.item(idcIter);

                    NodeList idcNames = idc.getElementsByTagName("IDCName");
                    String idcName = idcNames.getLength() > 0 ? idcNames.item(0).getFirstChild().getNodeValue() : Consts.UNKNOWN;

                    NodeList centerNames = idc.getElementsByTagName("CenterName");
                    String centerName = centerNames.getLength() > 0 ? centerNames.item(0).getFirstChild().getNodeValue() : Consts.UNKNOWN;

                    NodeList items = idc.getElementsByTagName("Item");
                    for (int itemIter = 0; itemIter < items.getLength(); ++itemIter) {
                        Element item = (Element) items.item(itemIter);
                        NodeList ips = item.getElementsByTagName("IP");
                        NodeList masks = item.getElementsByTagName("MASK");
                        if (ips.getLength() <= 0 || masks.getLength() <= 0) {
                            //idc.xml is error
                            continue;
                        }

                        String idcIp = ips.item(0).getFirstChild().getNodeValue();
                        String idcMask = masks.item(0).getFirstChild().getNodeValue();
                        IDC idcItem = new IDC();
                        idcItem.setIp(idcIp);
                        idcItem.setMask(idcMask);

                        com.octo.idc.model.Idc idcinfo = new Idc();
                        idcinfo.setIdc(idcName);
                        idcinfo.setRegion(regionName);
                        idcinfo.setCenter(centerName);
                        idcItem.setIdcinfo(idcinfo);

                        idcItem.init();
                        idcResult.add(idcItem);
                    }
                }
            }
            ret = true;
        } catch (Exception e) {
            LOG.warn("failed to read local idc file {}", filePath, e);
            ret = false;
        } finally {
            IpUtil.setIsInitSGAgentIDCXml(true);
        }
        return ret;
    }
}
