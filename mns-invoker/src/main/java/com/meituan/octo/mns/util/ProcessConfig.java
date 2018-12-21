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
import com.meituan.octo.mns.model.HostEnv;
import org.apache.commons.lang3.StringUtils;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

class ProcessConfig {

    private static final Logger LOG = LoggerFactory.getLogger(ProcessConfig.class);

    static final Map<String, HostEnv> VALUE_MAP_HOST_ENV = initValueMapHostEnv();

    private static boolean configIsExist = false;

    private static final Properties CONFIG = loadConfigFromFile();

    private static final boolean VALID_CONFIG = checkEnv(Consts.ENV_KEY);

    private static final HostEnv HOST_ENV = handleHostEnv();

    private static final String MNS_ZK_URL = getValueFromConfig(Consts.MNSZK_URL);

    private static final String MNS_URL = getValueFromConfig(Consts.MNS_URL);

    private static final String IDC_PATH = getValueFromConfig(Consts.IDC_PATH);

    private static final String SGAGENT_PORT = getValueFromConfig(Consts.SG_AGENT_PORT);

    private static final String SGAGENT_APPKEY = getValueFromConfig(Consts.SGAENT_APPKEY);

    private static final String SGSENTINEL_APPKEY = getValueFromConfig(Consts.SG_SENTINEL_APPKEY);


    private static Map<String, HostEnv> initValueMapHostEnv() {
        Map<String, HostEnv> ret = new HashMap<String, HostEnv>(5);
        ret.put("prod", HostEnv.PROD);
        ret.put("stage", HostEnv.STAGE);
        ret.put("dev", HostEnv.DEV);
        ret.put("ppe", HostEnv.PPE);
        ret.put("test", HostEnv.TEST);
        return ret;
    }


    private static Properties loadConfigFromFile() {
        Properties props = new Properties();
        InputStream in = null;
        try {
            in = new FileInputStream(Consts.CONFIG_FILE);
            props.load(in);
            configIsExist = true;
        } catch (FileNotFoundException e) {
            configIsExist = false;
            LOG.error(Consts.CONFIG_FILE + " does not exist", e);
        } catch (IOException e) {
            LOG.error("Failed to load CONFIG from" + Consts.CONFIG_FILE, e);
        } finally {
            if (in != null) {
                try {
                    in.close();
                } catch (Exception e) {
                    LOG.error("Failed to close " + Consts.CONFIG_FILE, e);
                }
            }
        }
        return props;
    }

    private static String getValueFromConfig(String key) {
        return StringUtils.trim(CONFIG.getProperty(key));
    }

    private static HostEnv handleHostEnv() {
        if (isValidConfig()) {
            return strToHostEnv(StringUtils.trim(CONFIG.getProperty(Consts.ENV_KEY)));
        } else {
            LOG.warn("{} does not exist or is incomplete.", Consts.CONFIG_FILE);
        }
        return HostEnv.DEV;
    }

    private static boolean checkEnv(String key) {
        boolean ret = false;
        if (isConfigIsExist()) {
            String value = StringUtils.trim(CONFIG.getProperty(key));
            ret = !StringUtils.isEmpty(value) && isEnvValid(value);
        } else {
            ret = false;
        }

        return ret;
    }

    static Properties getConfig() {
        return CONFIG;
    }

    static HostEnv getHostEnv() {
        return HOST_ENV;
    }

    static boolean isValidConfig() {
        return VALID_CONFIG;
    }

    private static boolean isEnvValid(String envStr) {
        return VALUE_MAP_HOST_ENV.containsKey(envStr);
    }

    private static HostEnv strToHostEnv(String envStr) {
        return isEnvValid(envStr) ? VALUE_MAP_HOST_ENV.get(envStr) : HostEnv.DEV;
    }

    static boolean isConfigIsExist() {
        return configIsExist;
    }

    static String getMnsZkUrl() {
        return MNS_ZK_URL;
    }

    static String getMnsUrl() {
        return MNS_URL;
    }

    static String getIdcPath() {
        return IDC_PATH;
    }

    static int getSgagentPort() {
        return SGAGENT_PORT == null ? 5266 : Integer.parseInt(SGAGENT_PORT);
    }

    static String getSgagentAppkey() {
        return SGAGENT_APPKEY;
    }

    static String getSgsentinelAppkey() {
        return SGSENTINEL_APPKEY;
    }
}
