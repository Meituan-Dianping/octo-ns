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

public class MnsCache<R, C, V> {
    private MultiMap<R, C, V> cache = MultiMap.create();
    private CacheLoader<R, C, V> loader;

    public MnsCache() {
    }

    public MnsCache(CacheLoader<R, C, V> loader) {
        this.loader = loader;
    }

    public static <R, C, V> MnsCache<R, C, V> create() {
        MnsCache<R, C, V> instance = new MnsCache<R, C, V>();
        return instance;
    }

    public static <R, C, V> MnsCache<R, C, V> create(
            CacheLoader<R, C, V> loader) {
        MnsCache<R, C, V> instance = new MnsCache<R, C, V>(loader);
        return instance;
    }

    public void put(R row, C column, V value) {
        cache.put(row, column, value);
    }

    public V get(R row, C column) {
        return cache.get(row, column);
    }

    public boolean contains(R row, C column) {
        return cache.contains(row, column);
    }

    public void updateAll() {
        if (loader != null) {
            for (R row : cache.rows()) {
                for (C column : cache.columns(row)) {
                    V value = loader.reload(row, column);
                    if (null != value) {
                        put(row, column, value);
                    }
                }
            }
        }
    }
}
