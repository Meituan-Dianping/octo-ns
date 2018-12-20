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
import org.junit.Assert;
import org.junit.Test;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.util.Properties;

import static com.meituan.octo.mns.util.ProcessConfig.VALUE_MAP_HOST_ENV;


public class ProcessConfigTests {
    @Test
    public void isFileExist() {
        InputStream in = null;
        try {
            in = new FileInputStream(Consts.CONFIG_FILE);
            Assert.assertTrue(ProcessConfig.isConfigIsExist());
        } catch (FileNotFoundException e) {
            Assert.assertFalse(ProcessConfig.isConfigIsExist());
        } finally {
            if (in != null) {
                try {
                    in.close();
                } catch (Exception e) {
                }
            }
        }
    }

    @Test
    public void checkEnv() {
        Properties config = ProcessConfig.getConfig();
        Assert.assertNotNull(config);

        HostEnv hostEnv = ProcessConfig.getHostEnv();
        Assert.assertTrue(hostEnv.equals(HostEnv.DEV) || hostEnv.equals(HostEnv.PPE) || hostEnv.equals(HostEnv.TEST)
                || hostEnv.equals(HostEnv.PROD) || hostEnv.equals(HostEnv.STAGE));

        if (!StringUtils.isEmpty(config.getProperty(Consts.ENV_KEY))) {
            Assert.assertTrue(VALUE_MAP_HOST_ENV.get(config.getProperty(Consts.ENV_KEY)) == ProcessConfig.getHostEnv());

        } else {
            Assert.assertTrue(ProcessConfig.getHostEnv() == HostEnv.DEV);
        }
    }
}
