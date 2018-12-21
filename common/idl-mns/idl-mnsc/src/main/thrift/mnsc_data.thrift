/**
 * xxx is
 * Copyright (C) 20xx THL A29 Limited, a xxx company. All rights reserved.
 *
 * Licensed under the  xxx
 *
 * Unless required by applicable law or agreed to in writing, software distributed
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 */
namespace java com.octo.mnsc.idl.thrift.model
namespace cpp meituan_mns
include  "../../../../idl-common/src/main/thrift/naming_common.thrift"
typedef naming_common.SGService SGService
struct MNSResponse {
    /*
     * 状态码，200: 正常返回200；如果version不同，MNSCache正在更新时，先直接返回500; 400: 访问MNS请求超时
     */
    1 : required i32 code = 200;
    /*
     * 节点列表，code = 200时，必须返回的字段
     */
    2 : optional list<SGService> defaultMNSCache;
    /*
     * 版本信息
     */
    3 : optional string version; //正常返回时，附带上version信息
}
// for scanner
struct MNSBatchResponse {
    /*
     * 状态码，200: 正常返回200；如果version不同，MNSCache正在更新时，先直接返回500; 400: 访问MNS请求超时
     */
    1 : required i32 code = 200;
    /*
     * appkey->(env->detail)
     */
    2 : optional map<string, map<string, list<SGService>>> cache;
}
struct AppKeyListResponse {
    /*
     * 状态码，200: 正常返回；400: 访问MNS请求超时；
     */
    1 : required i32 code = 200;
    /*
     * 服务appkey列表
     */
    2 : optional list<string> appKeyList;
}
enum Protocols{
    THRIFT,
    HTTP
}
struct MnsRequest{
    1: Protocols protoctol;
    2: string appkey;
    3: string env;
}
//环境的名字
const string PROD = "prod";
const string STAGE = "stage";
const string TEST = "test";
/**
* 异常错误定义
**/
/**
* 正常服务
**/
const i32 SUCCESS = 200;
/**
* 如果version不同，MNSCache正在更新时，先直接返回500
**/
const i32 MNSCache_UPDATE = 500;
/**
* 超时异常
**/
const i32 TIMEOUT_ERROR = 400;
//参数错误
const i32 ILLEGAL_ARGUMENT = 400;
//请求的缓存不存在
const i32 NOT_FOUND = 404
//缓存没有变化
const i32 NOT_MODIFIED = 304
