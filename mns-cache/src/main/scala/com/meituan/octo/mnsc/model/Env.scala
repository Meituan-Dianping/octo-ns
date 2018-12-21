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

package com.meituan.octo.mnsc.model

import com.meituan.octo.mns.util.ProcessInfoUtil
import org.apache.commons.lang.StringUtils

object Env extends Enumeration {
  type Env = Value
  val test = Value(1)
  val stage = Value(2)
  val prod = Value(3)

  def isValid(env: String) = {
    if (StringUtils.isEmpty(env)) {
      false
    } else {
      if (ProcessInfoUtil.isLocalHostOnline) {
        onlineEnv.contains(env)
      } else {
        offlineEnv.contains(env)
      }
    }
  }

  def strConvertEnum(env:String)={
   val envEnum= if (ProcessInfoUtil.isLocalHostOnline){
      onlineEnv.get(env)
    }else{
      offlineEnv.get(env)
    }
    envEnum.get
  }

  private val offlineEnv = Map(
    "ppe" -> stage,
    "dev" -> prod,
    "test" -> test,
    "prod" -> prod,
    "stage" -> stage,
    "beta" -> stage,
    "1" -> test,
    "2" -> stage,
    "3" -> prod
  )
  private val onlineEnv = Map(
    "prod" -> prod,
    "staging" -> stage,
    "stage" -> stage,
    "test" -> test,
    "1" -> test,
    "2" -> stage,
    "3" -> prod
  )

  def isValid(env: Int) = values.map(_.id).contains(env)
}
