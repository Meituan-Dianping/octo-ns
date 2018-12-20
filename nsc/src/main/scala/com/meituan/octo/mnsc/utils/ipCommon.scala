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

import org.apache.commons.lang.StringUtils

object ipCommon {
  private final val ipRegex = "^(1\\d{2}|2[0-4]\\d|25[0-5]|[1-9]\\d|[1-9])\\." +
    "(1\\d{2}|2[0-4]\\d|25[0-5]|[1-9]\\d|\\d)\\." +
    "(1\\d{2}|2[0-4]\\d|25[0-5]|[1-9]\\d|\\d)\\." +
    "(1\\d{2}|2[0-4]\\d|25[0-5]|[1-9]\\d|\\d)$"


  def getPrefixOfIP(ip: String) = {
    val ips = ip.split("\\.")
    val prefix = List(ips(0), ips(1)).mkString(".")
    s"$prefix."
  }

  def checkIP(ip: String) = {
    if (StringUtils.isBlank(ip)) {
      false
    } else {
      ip.trim.matches(ipRegex)
    }
  }

}
