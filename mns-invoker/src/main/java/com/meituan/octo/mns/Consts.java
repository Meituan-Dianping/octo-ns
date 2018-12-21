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

import com.meituan.octo.mns.util.ProcessInfoUtil;

public class Consts {
    public static final String LOCALHOST = "127.0.0.1";
    //  public static final boolean isOnline = ProcessInfoUtil.isLocalHostOnline();

    //  private static final String mnsUrl = isOnline ? "http://mns.sankuai.com" : "http://mns.inf.test.sankuai.com";


    public static final String UNKNOWN = "unknown";
    public static final int DEFALT_MAX_RESPONSE_MESSAGE_BYTES = 10 * 1024 * 1024;
    public static final int DEFAULT_TIMEOUT_IN_MILLS = 5000;
    public static final int SERVICES_THRESHOLD = 10;
    public static final int RETRY_TIMES = 3;
    public static final int CONNECT_TIMEOUT = 500;
    public static final String GET_SERVER_LIST_API = ProcessInfoUtil.getMnsUrl() + "/api/servicelist?";

    public static final int DEFAULT_UPDATE_TIME = 5;

    public static final String CONFIG_FILE = "/data/webapps/octo.cfg";


    public static final String ENV_KEY = "env";
    public static final String IDC_PATH = "idc_path";
    public static final String SG_AGENT_PORT = "port";
    public static final String SG_SENTINEL_APPKEY = "sg_sentinel_appkey";
    public static final String SGAENT_APPKEY = "sgagent_appkey";
    public static final String MNSZK_URL = "mnszk_url";
    public static final String MNS_URL = "mns_url";

    public static final String MNS_DEFAULT_VERSION = "original";


}
