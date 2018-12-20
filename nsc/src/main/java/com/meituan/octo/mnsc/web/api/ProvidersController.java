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

import com.meituan.octo.mnsc.dataCache.appProviderDataCache;
import com.meituan.octo.mnsc.dataCache.appProviderHttpDataCache;
import com.meituan.octo.mnsc.model.Env;
import com.meituan.octo.mnsc.service.apiProviders;
import com.meituan.octo.mnsc.utils.api;
import com.octo.naming.common.thrift.model.SGService;
import org.apache.commons.lang.StringUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.*;

import java.util.List;


@Controller
@RequestMapping("/api/providers")
public class ProvidersController {
    private static final Logger LOG = LoggerFactory.getLogger(ProvidersController.class);
    private static final String HTTP = "http";
    private static final String THRIFT = "thrift";

    @RequestMapping(value = "", method = RequestMethod.GET, produces = "application/json")
    @ResponseBody
    public String getProviders(@RequestParam("appkey") String appkey,
                               @RequestParam("env") int env,
                               @RequestParam("protocol") String protocol) {
        try {
            if (StringUtils.isEmpty(appkey)) {
                return api.errorJson(400, "appkey is not allowed to empty");
            }

            if (!Env.isValid(env)) {
                return api.errorJson(400, "env invalid");
            }

            if (StringUtils.isEmpty(protocol)) {
                return api.errorJson(400, "protocol is not allowed to empty");
            }

            if (THRIFT.equalsIgnoreCase(protocol)) {
                return api.dataJson(appProviderDataCache.getProviderCache(appkey, Env.apply(env).toString(),false));
            } else if (HTTP.equalsIgnoreCase(protocol)) {
                return api.dataJson(appProviderHttpDataCache.getProviderHttpCache(appkey, Env.apply(env).toString(),false));
            } else {
                return apiProviders.getProviders(appkey.trim(), env, protocol);
            }
        } catch (Exception e) {
            LOG.error("/api/providers error ", e);
            return api.errorJson(500, "server error");
        }
    }

    @RequestMapping(value = "", method = RequestMethod.POST, produces = "application/json")
    @ResponseBody
    public String postProviders(@RequestBody List<SGService> providers) {
        return api.jsonStr(apiProviders.postProviders(providers));
    }

    @RequestMapping(value = "", method = RequestMethod.DELETE, produces = "application/json")
    @ResponseBody
    public String deleteProviders(@RequestBody List<SGService> providers) {
        return api.jsonStr(apiProviders.deleteProviders(providers));
    }

    @RequestMapping(value = "/delete", method = RequestMethod.POST, produces = "application/json")
    @ResponseBody
    public String deletePostProviders(@RequestBody List<SGService> providers) {
        // client may be able to delete the nodes because the jdk bug, so add a new API. more detail see: http://bugs.java.com/view_bug.do?bug_id=8148558
        return api.jsonStr(apiProviders.deleteProviders(providers));
    }
}
