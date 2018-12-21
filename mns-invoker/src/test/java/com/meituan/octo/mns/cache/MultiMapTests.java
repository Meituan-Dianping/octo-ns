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


import com.octo.naming.common.thrift.model.SGService;
import org.junit.Assert;
import org.junit.Test;

import java.util.HashMap;
import java.util.Map;

public class MultiMapTests {
    @Test
    public void testMap() {
        MultiMap map = new MultiMap<String, String, SGService>();
        Assert.assertTrue(map.rows().isEmpty());
        Assert.assertTrue(map.columns(null).isEmpty());
        Assert.assertTrue(map.columns("test").isEmpty());
        Assert.assertNull(map.get(null));
        Assert.assertNull(map.get("row"));
        Assert.assertNull(map.get(null, null));
        Assert.assertNull(map.get("row", null));
        Assert.assertNull(map.get(null, "column"));
        Assert.assertNull(map.getOrCreate(null));
        Assert.assertTrue(map.getOrCreate("row").isEmpty());
        Assert.assertFalse(map.rows().isEmpty());

        Assert.assertTrue(map.columns(null).isEmpty());
        Assert.assertTrue(map.columns("test").isEmpty());

        map.put(null, null, null);
        Assert.assertNull(map.get(null, null));

        map.put("row", null, null);
        Assert.assertNull(map.get("row", null));

        map.put(null, "column", null);
        Assert.assertNull(map.get(null, "column"));
        map.put("row", "column", null);
        Assert.assertNull(map.get("row", "column"));
        map.put("row", "column", "value");
        Assert.assertEquals("value", map.get("row", "column"));

        map.put("row", "column", null);
        Assert.assertEquals("value", map.get("row", "column"));
        Assert.assertFalse(map.rows().isEmpty());
        Assert.assertTrue(map.columns(null).isEmpty());
        Assert.assertTrue(map.columns("test").isEmpty());

        Map<String, String> hashMap = new HashMap<String, String>();
        String value = hashMap.get(null);
        Assert.assertNull(value);
    }
}
