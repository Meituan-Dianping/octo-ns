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
namespace java com.octo.naming.service.thrift.model
namespace cpp meituan_mns

include "naming_common.thrift"
typedef naming_common.SGService SGService
typedef naming_common.ProtocolRequest ProtocolRequest
typedef naming_common.ProtocolResponse ProtocolResponse
/**
 * Common status reporting mechanism across all services
 */
service ServiceAgent
{
    /*
     * 新增协议获取服务列表的接口
     */
    ProtocolResponse getServiceListByProtocol(1:ProtocolRequest req);
    // 获取zk全部节点的接口
    ProtocolResponse getOriginServiceList(1:ProtocolRequest req);
     /*
      *服务注册
      */
    i32 registService(1:SGService oService);

    /*
     * 注册服务
     * uptCmd:0,重置(代表后面的serviceName list就是该应用支持的全量接口);
     * 1，增加(代表后面的serviceName list是该应用新增的接口);
     * 2，减少(代表后面的serviceName list是该应用删除的接口)。
     */
    i32 registServicewithCmd(1:i32 uptCmd, 2:SGService oService);
    /*
     *服务注销
     */
    i32 unRegistService(1:SGService oService);
}
