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
package com.meituan.octo.mns.cache;

import com.meituan.octo.mns.Consts;
import com.meituan.octo.mns.InvokeProxy;
import com.meituan.octo.mns.listener.IServiceListChangeListener;
import com.meituan.octo.mns.model.SGAgentClient;
import com.meituan.octo.mns.util.ScheduleTaskFactory;
import com.octo.naming.common.thrift.model.ProtocolRequest;
import com.octo.naming.common.thrift.model.ProtocolResponse;
import com.octo.naming.common.thrift.model.SGService;
import com.octo.naming.common.thrift.model.ServiceDetail;
import com.octo.naming.service.thrift.model.ServiceAgent;
import org.apache.commons.lang3.StringUtils;
import org.apache.thrift.TException;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class ServiceCache {
    private static final Logger LOG = LoggerFactory.getLogger(ServiceCache.class);

    private Map<ProtocolRequest, List<SGService>> requestCache = new ConcurrentHashMap<ProtocolRequest, List<SGService>>();
    private Map<ProtocolRequest, List<SGService>> sgagentCache = new ConcurrentHashMap<ProtocolRequest, List<SGService>>();

    private Map<ProtocolRequest, ProtocolRequest> requestToCacheKey = new ConcurrentHashMap<ProtocolRequest, ProtocolRequest>();
    private Map<ProtocolRequest, ProtocolRequest> requestCacheToSgagentCache = new HashMap<ProtocolRequest, ProtocolRequest>();

    private ServiceAgent.Iface sgAgent = new InvokeProxy(SGAgentClient.ClientType.multiProto).getProxy();

    private ServiceListListener listeners = new ServiceListListener();

    private final Object requestKeyLock = new Object();

    private static final ExecutorService LISTENER_THREAD_POOL_EXECUTOR = Executors.newFixedThreadPool(2, new ScheduleTaskFactory("MnsInvoker-ServiceList-Listener"));


    private final int retrySleepTime = 500;
    // only for test case
    private static boolean runUpdateAll = true;
    private static boolean saveNullList = true;

    private static final int SUCCESS = 0;
    private static final int ERR_NODE_NOTFIND = -101;

    public List<SGService> get(ProtocolRequest req) {
        ProtocolRequest request = getCacheKey(req);
        List<SGService> list = requestCache.get(request);
        return (null == list) ? getServiceFromAgentAndSave(request) : list;
    }

    private ProtocolRequest getCacheKey(ProtocolRequest req) {
        ProtocolRequest request = requestToCacheKey.get(req);
        if (null == request) {
            synchronized (requestKeyLock) {
                request = requestToCacheKey.get(req);
                if (null == request) {
                    request = handleServiceName(req);
                    requestToCacheKey.put(req, request);
                }
            }
        }
        return request;
    }

    private List<SGService> getServiceFromAgentAndSave(ProtocolRequest req) {


        // get the service list for the first time. retry up to three times
        List<SGService> list = null;
        for (int i = 0; i < Consts.RETRY_TIMES; ++i) {
            try {
                ProtocolResponse resp = doGetServiceList(sgAgent, req);
                if (null != resp && SUCCESS == resp.getErrcode()) {
                    //success to get service list.
                    list = resp.getServicelist();
                    break;
                } else if (null != resp && ERR_NODE_NOTFIND == resp.getErrcode()) {
                    LOG.warn("Fail to get service list, the remoteAppkey or serviceName doesn't exist. param = {}", req.toString());
                    break;
                } else {
                    if (i < 2) {
                        Thread.sleep(retrySleepTime);
                    }
                }

            } catch (Exception e) {
                LOG.error("Fail to get service list, now try to get it again. time = " + i, e);
            }
        }

        if (null == list) {
            if (!saveNullList) {
                // only for test case, using mock data.
                return list;
            }
            list = Collections.emptyList();
        }

        // Do not adjust the order of the following codes
        sgagentCache.put(req, list);

        ProtocolRequest requestKey = isIgnoreServiceName(req) ? new ProtocolRequest(req) : req;
        List<SGService> filterList = filterWithServiceName(list, requestKey);
        requestCache.put(requestKey, filterList);

        requestCacheToSgagentCache.put(requestKey, req);

        return filterList;
    }

    void clearCache() {
        requestCacheToSgagentCache.clear();
        requestCache.clear();
        sgagentCache.clear();
        requestCacheToSgagentCache.clear();
    }

    static void setSaveNullList(boolean saveNullList) {
        ServiceCache.saveNullList = saveNullList;
    }

    private boolean isIgnoreServiceName(ProtocolRequest req) {
        return "thrift".equals(req.getProtocol())
                && !StringUtils.isEmpty(req.getRemoteAppkey())
                && !StringUtils.isEmpty(req.getServiceName());
    }

    private ProtocolRequest handleServiceName(ProtocolRequest req) {
        ProtocolRequest ret = new ProtocolRequest(req);
        if (isIgnoreServiceName(req)) {
            ret.unsetServiceName();
        }
        return ret;
    }

    private List<SGService> filterWithServiceName(List<SGService> allList, ProtocolRequest req) {
        List<SGService> retList = null;
        if (isIgnoreServiceName(req)) {
            retList = new ArrayList<SGService>();
            for (SGService service : allList) {
                if (null != service.getServiceInfo() && service.getServiceInfo().containsKey(req.getServiceName())) {
                    retList.add(service);
                }
            }
        } else {
            retList = allList;
        }

        return retList;
    }

    public void addListener(ProtocolRequest req, IServiceListChangeListener listener) {
        listeners.put(req, listener);
    }

    public int removeListener(ProtocolRequest req, IServiceListChangeListener listener) {
        return listeners.remove(req, listener);
    }

    protected ProtocolResponse doGetServiceList(ServiceAgent.Iface client, com.octo.naming.common.thrift.model.ProtocolRequest req) throws TException {
        return client.getServiceListByProtocol(req);
    }

    private List<SGService> getServiceFromAgent(ServiceAgent.Iface client, ProtocolRequest req) {
        List<SGService> list = null;
        try {
            ProtocolResponse resp = doGetServiceList(client, req);
            if (0 == resp.getErrcode()) {
                list = resp.getServicelist();
            } else {
                LOG.debug("Fail to get service list, errorcode = {} | params = {}", resp.getErrcode(), req.toString());


            }
        } catch (Exception e) {
            LOG.debug(e.getMessage(), e);
        }


        return list;
    }

    static void setRunUpdateAll(boolean runUpdateAll) {
        ServiceCache.runUpdateAll = runUpdateAll;
    }

    void updateAll() {
        if (!runUpdateAll) {
            // only for test case.
            return;
        }
        for (Map.Entry<ProtocolRequest, List<SGService>> item : sgagentCache.entrySet()) {
            ProtocolRequest req = item.getKey();
            List<SGService> list = getServiceFromAgent(sgAgent, req);
            if (null != list) {
                sgagentCache.put(req, list);
            }
        }

        for (Map.Entry<ProtocolRequest, List<SGService>> item : requestCache.entrySet()) {
            ProtocolRequest req = item.getKey();
            ProtocolRequest sgagentCacheKey = requestCacheToSgagentCache.get(req);
            if (null != sgagentCacheKey) {
                List<SGService> allList = sgagentCache.get(sgagentCacheKey);
                if (null != allList) {
                    List<SGService> filterList = filterWithServiceName(allList, req);
                    requestCache.put(req, filterList);
                    listeners.callIfDiff(req, item.getValue(), filterList);
                }
            }
        }
    }

    class ServiceListListener {
        private Map<ProtocolRequest, Set<IServiceListChangeListener>> listeners = new ConcurrentHashMap<ProtocolRequest, Set<IServiceListChangeListener>>();
        private final Object listenerLock = new Object();

        private void initKeyListener(ProtocolRequest req) {
            get(req);
        }

        public void put(final ProtocolRequest req, final IServiceListChangeListener listener) {
            initKeyListener(req);
            ProtocolRequest request = getCacheKey(req);
            if (!listeners.containsKey(request)) {
                synchronized (listenerLock) {
                    if (!listeners.containsKey(request)) {
                        listeners.put(request, new HashSet<IServiceListChangeListener>());
                    }
                }
            }
            listeners.get(request).add(listener);
        }

        public int remove(final ProtocolRequest req, final IServiceListChangeListener listener) {
            int ret = 0;
            ProtocolRequest request = getCacheKey(req);
            Set<IServiceListChangeListener> listenerSet = listeners.get(request);
            if (null != listenerSet) {
                listenerSet.remove(listener);
            }
            return ret;
        }

        public void callIfDiff(final ProtocolRequest req, final List<SGService> oldList, final List<SGService> newList) {
            final Set<IServiceListChangeListener> listenerSet = listeners.get(req);
            if (null == listenerSet) {
                return;
            }

            final List<SGService> addList = new ArrayList<SGService>(),
                    deletedList = new ArrayList<SGService>(),
                    modifiedList = new ArrayList<SGService>();
            // if oldlist is small, just go through every item of newlist and oldlist
            if (oldList.size() < Consts.SERVICES_THRESHOLD) {
                for (SGService newItem : newList) {
                    boolean isExist = false;
                    for (SGService oldItem : oldList) {
                        //the intersection of new and old，check if the value is modified
                        if (isSameNode(newItem, oldItem)) {
                            if (isModified(newItem, oldItem)) {
                                modifiedList.add(newItem);
                            }
                            isExist = true;
                        }
                    }
                    if (!isExist) {
                        //the difference set in the new list
                        addList.add(newItem);
                    }
                }
                //the difference set in the old list
                for (SGService oldItem : oldList) {
                    boolean isExist = false;
                    for (SGService newItem : newList) {
                        if (isSameNode(newItem, oldItem)) {
                            isExist = true;
                            break;
                        }
                    }
                    if (!isExist) {
                        deletedList.add(oldItem);
                    }
                }
            } else {

                // if oldlist is big, we turn oldlist to map
                Map<String, SGService> oldMap = new HashMap<String, SGService>();
                for (SGService item : oldList) {
                    String sid = item.getIp() + "_" + item.getPort() + "_" + item.getAppkey();
                    oldMap.put(sid, item);
                }

                for (SGService item : newList) {
                    String key = item.getIp() + "_" + item.getPort() + "_" + item.getAppkey();

                    //the intersection of new and old，check if the value is modified
                    if (oldMap.containsKey(key)) {
                        if (isModified(item, oldMap.get(key))) {
                            modifiedList.add(item);
                        }
                    } else {
                        //the difference set in the new list
                        addList.add(item);
                    }
                    //the deletedList
                    oldMap.remove(key);
                }

                //the difference set in the old list
                for (Map.Entry<String, SGService> e : oldMap.entrySet()) {
                    deletedList.add(e.getValue());
                }

            }
            //modifiedList came first because modifiedList is more often changed
            if ((!modifiedList.isEmpty() || !deletedList.isEmpty() || !addList.isEmpty())) {
                LISTENER_THREAD_POOL_EXECUTOR.execute(new Runnable() {
                    @Override
                    public void run() {
                        for (IServiceListChangeListener listener : listenerSet) {
                            try {
                                listener.changed(req, oldList, newList, addList, deletedList, modifiedList);
                            } catch (Exception e) {
                                LOG.error("Failed to execute callback function of mns listener.", e);
                            }
                        }
                    }
                });
            }
        }

        // appkey for pigeon
        private boolean isSameNode(SGService a, SGService b) {
            return StringUtils.equals(a.getIp(), b.getIp())
                    && a.getPort() == b.getPort()
                    && StringUtils.equals(a.getAppkey(), b.getAppkey());
        }

        boolean isModified(SGService a, SGService b) {
            boolean isServiceInfoDiff = false;
            try {
                if (a.isSetServiceInfo() && b.isSetServiceInfo()) {
                    Map<String, ServiceDetail> serviceInfoA = a.getServiceInfo();
                    Map<String, ServiceDetail> serviceInfoB = b.getServiceInfo();
                    if (serviceInfoA.size() == serviceInfoB.size()) {
                        for (Map.Entry<String, ServiceDetail> oldItem : serviceInfoA.entrySet()) {
                            if (!(serviceInfoB.containsKey(oldItem.getKey()) && isServiceDetailEqual(oldItem.getValue(), serviceInfoB.get(oldItem.getKey())))) {
                                isServiceInfoDiff = true;
                                break;
                            }
                        }
                    } else {
                        isServiceInfoDiff = true;
                    }
                } else if (a.isSetServiceInfo() ^ b.isSetServiceInfo()) {
                    isServiceInfoDiff = true;
                }
            } catch (Exception e) {
                LOG.warn("Fail to compare serviceInfos , simply ignore the error.", e);
            }

            boolean isSame = (a.getStatus() == b.getStatus())
                    && (a.getWeight() == b.getWeight())
                    && (Math.abs(a.getFweight() - b.getFweight()) < 0.0000001d)
                    && StringUtils.equals(a.getVersion(), b.getVersion())
                    && (a.getRole() == b.getRole())
                    && !isServiceInfoDiff;
            return !isSame;
        }

        private boolean isServiceDetailEqual(ServiceDetail detailA, ServiceDetail detailB) {
            return (null == detailA && null == detailB) || (null != detailA && detailA.equals(detailB));
        }
    }
}
