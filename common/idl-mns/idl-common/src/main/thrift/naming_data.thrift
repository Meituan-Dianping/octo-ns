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
namespace java com.octo.data.thrift.model
namespace cpp meituan_mns

include "naming_common.thrift"
typedef naming_common.SGService SGService
typedef naming_common.CRouteData CRouteData
enum RegistCmd {
    REGIST = 0,
    UNREGIST = 1
}
struct getservice_req_param_t
{
    1:string localAppkey;
    2:string remoteAppkey;
    3:string version;
    4:string protocol;
}
struct getservice_res_param_t
{
    1:string localAppkey;
    2:string remoteAppkey;
    3:string version;
    4:list<SGService> serviceList;
    5:string protocol;
}
struct getservicename_req_param_t
{
    1:string localAppkey;
    2:string servicename;
    3:string version;
    4:string protocol;
}
struct getservicename_res_param_t
{
    1:string localAppkey;
    2:string servicename;
    3:string version;
    4:set<string> appkeys;
    5:string protocol;
}
struct getroute_req_param_t
{
    1:string localAppkey;
    2:string remoteAppkey;
    3:string version;
    4:string protocol;
}
struct getroute_res_param_t
{
    1:string localAppkey;
    2:string remoteAppkey;
    3:string version;
    4:list<CRouteData> routeList;
    5:string protocol;
}
struct regist_req_param_t
{
    1:i32 retry_times = 0;
    2:SGService sgservice;
    3:i32 uptCmd;
    4:RegistCmd regCmd;
}
