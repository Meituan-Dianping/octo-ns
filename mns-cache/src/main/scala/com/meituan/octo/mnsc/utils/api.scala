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

import javax.crypto.Mac
import javax.crypto.spec.SecretKeySpec

import com.fasterxml.jackson.databind.ObjectMapper
import com.fasterxml.jackson.module.scala.DefaultScalaModule
import com.fasterxml.jackson.module.scala.experimental.ScalaObjectMapper
import org.apache.commons.codec.binary.Base64

object api {
  /** ObjectMapper is thread safe */
  private val mapper = new ObjectMapper() with ScalaObjectMapper
  mapper.registerModule(DefaultScalaModule)
  def errorJsonArgInvalid(msg: AnyRef):String ={
    errorJson(400,msg)
  }

  def dataJson200(data: AnyRef):String={
    dataJson(200,data)
  }

  def dataJson(errorCode:Int,data: AnyRef):String={
    val map = Map("ret" -> errorCode, "data" -> data)
    jsonStr(map)
  }

  def dataJson(errorCode:Int, errorMsg:String, data: AnyRef):String={
    val map = Map("ret" -> errorCode, "msg" -> errorMsg, "data" -> data)
    jsonStr(map)
  }

  def errorJson(errorCode: Int, msg: AnyRef): String = {
    val map = Map("ret" -> errorCode, "msg" -> msg)
    jsonStr(map)
  }

  def dataJson(data: AnyRef): String = {
    val map = Map("data" -> data, "isSuccess" -> true)
    jsonStr(map)
  }

  def jsonStr(data: AnyRef): String = {
    new String(jsonBytes(data), "utf-8")
  }

  def jsonBytes(data: AnyRef): Array[Byte] = {
    mapper.writeValueAsBytes(data)
  }

  def toObject[T](bytes: Array[Byte], valueType: Class[T]) = {
    mapper.readValue(bytes, valueType)
  }

  def toObject[T](str: String, valueType: Class[T]) = {
    mapper.readValue(str, valueType)
  }

  def authorization(uri: String, method: String, date: String, clientId: String, secret: String): String = {
    val signKey = new SecretKeySpec(secret.getBytes("utf-8"), "HmacSHA1")
    val mac = Mac.getInstance("HmacSHA1")
    mac.init(signKey)
    val text = method + " " + uri + "\n" + date
    "MWS " + clientId + ":" + Base64.encodeBase64String(mac.doFinal(text.getBytes("utf-8")))
  }
}
