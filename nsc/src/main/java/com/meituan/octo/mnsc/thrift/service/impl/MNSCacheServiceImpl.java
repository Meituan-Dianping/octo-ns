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

package com.meituan.octo.mnsc.thrift.service.impl;

import com.octo.mnsc.idl.thrift.model.*;
import com.octo.mnsc.idl.thrift.service.MNSCacheService;
import com.meituan.octo.mnsc.model.Env;
import com.meituan.octo.mnsc.service.mnscService;
import org.apache.commons.lang.StringUtils;
import org.apache.thrift.TException;
import org.springframework.util.Assert;

import java.util.List;

public class MNSCacheServiceImpl implements MNSCacheService.Iface {
    @Override
    public MNSResponse getMNSCache(String appkey, String version, String env) throws TException {
        if (!Env.isValid(env) || org.apache.commons.lang3.StringUtils.isEmpty(appkey)) {
            MNSResponse ret = new MNSResponse();
            ret.setCode(mnsc_dataConstants.ILLEGAL_ARGUMENT);
            return ret;
        }
        return mnscService.getMnsc(appkey, version, env);
    }

    @Override
    public MNSResponse getMNSCacheHttp(String appkey, String version, String env) throws TException {
        if (!Env.isValid(env) || org.apache.commons.lang3.StringUtils.isEmpty(appkey)) {
            MNSResponse ret = new MNSResponse();
            ret.setCode(mnsc_dataConstants.ILLEGAL_ARGUMENT);
            return ret;
        }
        return mnscService.getMNSCache4HLB(appkey, version, env);
    }

    @Override
    public MNSBatchResponse getMNSCacheByAppkeys(List<String> appkeys, String protocol) throws TException {
        if (null == appkeys || StringUtils.isEmpty(protocol)) {
            return new MNSBatchResponse().setCode(mnsc_dataConstants.ILLEGAL_ARGUMENT);
        }
        return mnscService.getMNSCacheByAppkeys(appkeys, protocol);
    }

    @Override
    public MNSResponse getProvidersByIP(String ip) throws TException {
        Assert.hasText(ip, "ip不能为空");
        return mnscService.getProvidersByIP(ip);
    }

    @Override
    public MNSResponse getMNSCacheWithVersionCheck(MnsRequest mnsRequest) throws TException {
        if (null == mnsRequest || StringUtils.isEmpty(mnsRequest.getAppkey()) || !Env.isValid(mnsRequest.getEnv())) {
            MNSResponse invalidResponse = new MNSResponse();
            invalidResponse.setCode(mnsc_dataConstants.ILLEGAL_ARGUMENT);
            return invalidResponse;
        }
        return mnscService.getMnsc(mnsRequest);
    }

    @Override
    public AppKeyListResponse getAppkeyListByIP(String ip) throws TException {
        AppKeyListResponse ret = new AppKeyListResponse();
        if (StringUtils.isEmpty(ip)) {
            ret.setCode(400);
        } else {
            List<String> appkeys = mnscService.getAppkeyListByIP(ip.trim());
            ret.setCode(200).setAppKeyList(appkeys);
        }
        return ret;
    }
}
