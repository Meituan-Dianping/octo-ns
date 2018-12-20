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

#ifndef OCTO_OPEN_SOURCE_BASE_MNS_CONSTS_H
#define OCTO_OPEN_SOURCE_BASE_MNS_CONSTS_H

namespace meituan_mns {
const int kMaxBuffSize = 1024;
const int kZkContentSize = 1024;
//协议类型
const int THRIFT_TYPE = 0;
const int HTTP_TYPE = 1;

const int REG_LIMIT_LEGAL = 1;

const int DEFAULT_SLEEPTIME = 10000;

const int DEFAULT_SPIN_NUM = 10;
}
#endif //OCTO_OPEN_SOURCE_BASE_MNS_CONSTS_H
