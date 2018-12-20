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

package com.meituan.octo.mnsc.utils

import org.slf4j.{Logger, LoggerFactory}

import scala.collection.JavaConverters._

object common {
  private final val LOG: Logger = LoggerFactory.getLogger(common.getClass)

  def toList(enum: Enumeration) = enum.values.map {
    x => Map("name" -> x.toString, "value" -> x.id).asJava
  }.toList.sortBy(_.get("value").asInstanceOf[Int]).asJava

  def toPairList(enum: Enumeration) = enum.values.map(x => Pair(x.toString, x.id)).toList.asJava

  def toStringList(enum: Enumeration) = enum.values.map(x => '"' + x.toString + ',' + x.id + '"').toList.asJava

  def toMap(enum: Enumeration) = enum.values.map(x => (x.toString -> x.id)).toMap.asJava

  def toMapById(enum: Enumeration) = enum.values.map(x => (x.id -> x.toString)).toMap.asJava

  def toMap(cc: AnyRef) = {
    (Map[String, Any]() /: cc.getClass.getDeclaredFields) {
      (a, f) =>
        f.setAccessible(true)
        a + (f.getName -> f.get(cc))
    }.toMap
  }

  def toJavaMap(cc: AnyRef) = {
    toMap(cc).asJava
  }

  def notEmpty(s: String) = s != null && !s.trim.isEmpty

  def notNull(s: String) = s != null

}
