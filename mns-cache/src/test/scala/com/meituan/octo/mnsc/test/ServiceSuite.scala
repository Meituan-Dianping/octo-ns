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

package com.meituan.octo.mnsc.test

import com.octo.mnsc.idl.thrift.service.MNSCacheService
import org.apache.thrift.protocol.{TBinaryProtocol, TProtocol}
import org.apache.thrift.transport.{TFramedTransport, TSocket}
import org.joda.time.DateTime
import org.junit.runner.RunWith
import org.scalatest.junit.JUnitRunner
import org.scalatest.{BeforeAndAfter, FunSuite}

import scala.collection.parallel.ForkJoinTaskSupport

@RunWith(classOf[JUnitRunner])
class ServiceSuite extends FunSuite with BeforeAndAfter {

  test("service") {
    val timeout = 3000
    val ip = "127.0.0.1"
    val port = 8091
    val transport = new TFramedTransport(new TSocket(ip, port, timeout), 16384000)
    val protocol: TProtocol = new TBinaryProtocol(transport)
    val mnsc = new MNSCacheService.Client(protocol)
    val appkey = "com.sankuai.cos.mtconfi"
    val env = "prod"
    //    val path = List("/mns/sankuai", env, appkey, Path.provider).mkString("/")
    //    val version = zk.getNodeVersion(path)
    transport.open
    while (true) {
      val start = new DateTime().getMillis
      println(mnsc.getMNSCache(appkey, "0", env))
      val end = new DateTime().getMillis
      println(s"cost ${end - start}")
      Thread.sleep(2000)
    }
  }

  test("par") {
    val a = (0 to 30).toList.par
    a.tasksupport = new ForkJoinTaskSupport(new scala.concurrent.forkjoin.ForkJoinPool(1000))
    val b = (0 to 100000).toList.par
    b.tasksupport = new ForkJoinTaskSupport(new scala.concurrent.forkjoin.ForkJoinPool(1000))
    val result = a.map {
      x =>
        val at = Thread.currentThread().getName
        println(b.map(x => Thread.currentThread().getName).distinct.length)
        at
    }.distinct.length
    println("ss" + result)
  }
}
