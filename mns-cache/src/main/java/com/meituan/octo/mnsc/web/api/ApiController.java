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

package com.meituan.octo.mnsc.web.api;

import com.meituan.octo.mns.util.ProcessInfoUtil;
import com.meituan.octo.mnsc.model.Env;
import com.meituan.octo.mnsc.service.apiService;
import com.meituan.octo.mnsc.service.mnscService;
import com.meituan.octo.mnsc.utils.api;
import com.meituan.octo.mnsc.utils.ipCommon;
import com.meituan.octo.mnsc.utils.mnscCommon;
import com.octo.mnsc.idl.thrift.model.AppKeyListResponse;
import org.apache.commons.lang.StringUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Controller;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import org.springframework.web.bind.annotation.*;

@Controller
@RequestMapping("/api")
public class ApiController {
    private static final Logger LOG = LoggerFactory.getLogger(ApiController.class);
    private static final String SENTINEL = ProcessInfoUtil.getSgsentinelAppkey();

    @RequestMapping(value = "/monitor/alive")
    @ResponseBody
    public Map<String, Object> monitorAlive() {
        Map<String, Object> result = new HashMap<String, Object>(1);
        result.put("status", "ok");
        return result;
    }

    @RequestMapping(value = "/servicelist", method = RequestMethod.GET, produces = "application/json")
    @ResponseBody
    public String getServiceList(@RequestParam("appkey") String appkey,
                                 @RequestParam("env") String env,
                                 @RequestParam("ip") String ip) {
        //all parameters are required.
        try {
            if (!SENTINEL.equals(appkey)) {
                return api.errorJson(400, "appkey currently only supports com.sankuai.inf.sgsentinel");

            }

            if (StringUtils.isEmpty(ip) || !(ipCommon.checkIP(ip.trim()))) {
                return api.errorJson(400, "ip invalid");
            }
            if (!Env.isValid(env.trim())) {
                return api.errorJson(400, "env invalid");
            }

            return apiService.getServiceList(appkey, env.trim(), ip.trim());
        } catch (Exception e) {
            LOG.error("/api/servicelist error ", e);
            return api.errorJson(500, "server error");
        }
    }

    @RequestMapping(value = "/{ip}/appkeylist", method = RequestMethod.GET, produces = "application/json")
    @ResponseBody
    public String getAppkeyList(@PathVariable("ip") String ip) {
        AppKeyListResponse ret = new AppKeyListResponse();
        if (StringUtils.isEmpty(ip)) {
            ret.setCode(400);
        } else {
            List<String> appkeys = mnscService.getAppkeyListByIP(ip.trim());
            ret.setCode(200).setAppKeyList(appkeys);
        }
        return api.dataJson(ret.getCode(), ret.getAppKeyList());
    }


    @RequestMapping(value = "/allappkeys", method = RequestMethod.GET, produces = "application/json")
    @ResponseBody
    public String getAppkeys() {
        return api.dataJson200(mnscCommon.allAppkeysList());
    }
}