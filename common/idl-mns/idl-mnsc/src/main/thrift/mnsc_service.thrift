/*
 * Copyright 2018 Meituan Dianping. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

namespace java com.octo.mnsc.idl.thrift.service
namespace cpp meituan_mns
include "mnsc_data.thrift"
typedef mnsc_data.MNSResponse MNSResponse
typedef mnsc_data.MNSBatchResponse MNSBatchResponse
typedef mnsc_data.AppKeyListResponse AppKeyListResponse
typedef mnsc_data.MnsRequest MnsRequest
typedef mnsc_data.SGService SGService
service MNSCacheService {
    /**
     * @param appkey 服务标识
     * @param version 版本
     * @param env 环境
     * @return 服务的节点列表
     * @name 获取服务的节点列表
     */
    MNSResponse getMNSCache(1:string appkey, 2:string version, 3:string env);
    /**
     * @param appkey 服务标识
     * @param version 版本
     * @param env 环境
     * @return 服务的HLB节点列表
     * @name 获取服务的HLB节点列表
     */
    MNSResponse getMNSCacheHttp(1:string appkey, 2:string version, 3:string env);
    /**
     * @param req 请求结构体
     * @name 获取服务的节点列表; 获取时, 会将缓存中的version与zk上的version进行比较,如果不相等,则重新更新缓存后,再返回数据。
     */
     MNSResponse getMNSCacheWithVersionCheck(1:MnsRequest req);
    /**
     * @param appkeys 请求的appkey列表
     * @param protocol
     */
    MNSBatchResponse getMNSCacheByAppkeys(1:list<string> appkeys, 2:string protocol);
    /**
    * 根据IP返回与该IP相关的服务节点
    * @param ip IP 地址
    */
    MNSResponse getProvidersByIP(1:string ip);
    /**
    *
    * @param ip
    * @return appkey列表
    */
    AppKeyListResponse getAppkeyListByIP(1:string ip);
}
