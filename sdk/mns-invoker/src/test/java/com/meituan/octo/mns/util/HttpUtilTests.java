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

import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.node.ObjectNode;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.*;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.List;


public class HttpUtilTests {
    private static final Logger LOG = LoggerFactory.getLogger(HttpUtil.class);

    public static void delete(String appkey, String protocol, String ip, int port) throws InterruptedException {
        List<String> ipPorts = new ArrayList<String>();
        ipPorts.add(ip + ":" + port);
        String url = ProcessInfoUtil.getMnsUrl() + "/api/providers/delete";
        String data = getDeletedProviderJson(appkey, protocol, ipPorts, 1);
        HttpUtilTests.delete(url, data);
        Thread.sleep(6000);
    }

    public static String getDeletedProviderJson(String appkey, String protocol, List<String> ipPorts, int env) {
        List<String> jsonList = new ArrayList<String>();
        for (String ipPort : ipPorts) {
            String[] ipport = ipPort.split(":");
            jsonList.add(getJson(appkey, protocol, ipport[0], ipport[1], env));
        }
        return "[" + listToString(jsonList) + "]";
    }

    public static String getJson(String appkey, String protocol, String ip, String port, int env) {
        ObjectMapper mapper = new ObjectMapper();
        ObjectNode node = mapper.createObjectNode();
        node.put("appkey", appkey);
        node.put("protocol", protocol);
        node.put("ip", ip);
        node.put("port", Integer.valueOf(port));
        node.put("envir", env);
        String ret = node.toString();
        return ret;
    }

    public static String listToString(List<String> stringList) {

        StringBuilder result = new StringBuilder();
        boolean flag = false;
        for (String string : stringList) {
            if (flag) {
                result.append(",");
            } else {
                flag = true;
            }
            result.append(string);
        }
        return result.toString();
    }

    public static String delete(String url, String content) {
        HttpURLConnection connection = null;
        String response = null;
        int code = -1;
        try {
            connection = (HttpURLConnection) new URL(url).openConnection();
            if (connection != null) {
                connection.setRequestMethod("POST");
                connection.setRequestProperty("Content-Type", "application/json; charset=UTF-8");
                writeContent(connection, content);
                response = readStream(connection);
                code = connection.getResponseCode();
                LOG.debug("delete {} {} {}", new Object[]{url, connection.getResponseCode(), response});
            } else {
                LOG.debug("can't connect to {}" + url);
            }
        } catch (Exception e) {
            LOG.error("delete {} failed {} {}", new Object[]{url, code, e});
        } finally {
            if (connection != null) {
                try {
                    connection.disconnect();
                } catch (Exception e) {
                    LOG.debug("close connection failed... " + url, e);
                }
            }
        }
        return response;
    }

    private static void writeContent(HttpURLConnection connection, String content) {
        OutputStreamWriter out = null;
        try {
            connection.setDoOutput(true);
            out = new OutputStreamWriter(connection.getOutputStream(), Charset.forName("UTF-8"));
            out.write(content);
            out.flush();
            out.close();
        } catch (Exception e) {
            LOG.debug("write content to {} failed {}", new Object[]{connection.getURL(), e});
        } finally {
            if (out != null) {
                try {
                    out.close();
                } catch (IOException e) {
                    LOG.debug("close connection failed... " + connection.getURL(), e);
                }
            }
        }
    }

    private static String readStream(HttpURLConnection connection) throws IOException {
        connection.setConnectTimeout(10000);
        connection.setReadTimeout(10000);

        String result = null;
        StringBuilder sb = new StringBuilder();
        InputStream is = null;
        try {
            is = new BufferedInputStream(connection.getInputStream());
            BufferedReader br = new BufferedReader(new InputStreamReader(is, Charset.forName("UTF-8")));
            String inputLine = "";
            while ((inputLine = br.readLine()) != null) {
                sb.append(inputLine);
            }
            result = sb.toString();
        } catch (Exception e) {
            LOG.debug("read connection failed... " + connection.getURL(), e);
        } finally {
            if (is != null) {
                try {
                    is.close();
                } catch (IOException e) {
                    LOG.debug("close connection failed... " + connection.getURL(), e);
                }
            }
        }
        return result;
    }
}
