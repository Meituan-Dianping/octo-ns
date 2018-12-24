# octo-ns

## 背景及概念

 octo-ns
 是美团OCTO服务治理体系进行服务注册发现的套件，其基于zookeeper并由多个组件和服务组成，包括sdk
 (C++/Java)、
 服务治理代理SGAgent、服务缓存NSC、健康检查服务Scanner
 ，用户可搭配人性化的运营管理平台[octo-portal](https://github
 .com/Meituan-Dianping/octo-portal)实现可视化服务操作。

 octo-ns
 可帮助业务进行服务注册发现，自定义路由分组策略、健康检查等功能，通过它可快速的实现基本的服务治理功能，让用户专注于业务逻辑实现。octo-ns总体架构图如下:

<div align=center> <img src="./docs/image/ns.png" width="500"> </div align=center>

* 服务Service

  通过自定义接口提供给消费端的软件功能。octo目前支持RPC、HTTP服务。

* 服务名 ServiceName

  服务标识，可映射到指定服务。octo体系中通过appkey字段表示服务名。

* 服务提供者 Service Provider

  是指提供可被调用服务的一方。

* 服务消费者 Service Consumer

  是指发起对某个服务调用的一方。

* 注册中心

  注册中心存储服务描述信息来实现服务的自动注册与发现。服务在启动时将服务信息上报到注册中心，服务消费者从注册中心获取关注的服务提供者信息。健康检查组件会根据对服务节点的检测结果更新注册中心服务状态信息。

* 服务注册(Service Registry)

  服务提供者启用后将服务信息上报到注册中心的流程

* 服务发现(Service Discovery)

  服务消费者获取服务提供者信息的流程

* 权重

  浮点数，实例粒度的路由流量指标。通过路由规则进行权重调整，实现流量分发。

* 服务分组(Service Group)

  是指按预定义或自定义约束的服务提供者和服务消费者的调用关系划分

* 健康检查(Health Check)

  服务节点的端口/进程存活状态的检查。octo-ns体系中通过scanner执行，并将结果信息上报给注册中心。

## 架构特点
* 去中心模块化架构

  通信框架(Whale、Dorado)、服务治理代理(SGAgent)、健康检查服务(scanner)、服务治理平台(ocot-portal)

* 轻量化框架及Sdk

  服务治理策略实现下移到octo-ns层，简化框架及SDK功能

 <div align=center> <img src="./docs/image/ns_trait.png" width="500"> </div align=center>


* 支持水平扩展的分布式服务治理代理(SGAgent)

    分布式服务治理代理从部署上分为本地和哨兵模式两种，当本地SDK到SGAgent不可用时可切换到哨兵集群，保证服务平滑扩展.
如特点(2)中描述，SGAgent实现了服务治理如注册、发现、路由、策略等重要功能，简化服务消费者和服务提供者使用的SDK。
* 统一服务状态检查服务(Scanner)

   Scanner会从端口连通性检查、网络超时检查以及系统负载的角度检查服务的健康状态，将状态更新到存储中心ZooKeeper，当服务恢复时将状态更新

* 容灾特点

   调用链容灾：octo-ns各组件的本地缓存能力可保证在调用链上的某个服务或组件在发生故障时进行服务降级，保证服务可用性

   哨兵模式：当服务消费者或服务提供者连接本地SG_Agent不可用时，可通过连接到哨兵集群接入到NS

##  架构介绍

<div align=center> <img src="./docs/image/ns_components.png" width="500"> </div align=center>

   OCTO NS由SGAgent、NSC、注册中心、Scanner组成，其中部分管理端Portal页面组成NS功能，如更改服务提供者状态、调整权重. 服务提供者通过通信框架及SDK接入NS，进行服务注册和服务发现。

### 模块介绍
* 服务治理代理SGAgent

  服务治理完成服务注册、服务发现的两个主要流程，注册支持按增量和非增量方式，增量方式是指一个服务实例可以包含多种ServiceName接口类型，通过在注册中心建立ServiceName和服务实例的双向映射，可满足按ServiceName和服务实例两种方式进行服务发现。

  **服务分组**：服务分组本质影响服务节点权重(权重的调整可按照用户进行自定义)，服务分组只有在注册中心配置了路由策略且生效才会最终影响到权重过滤，目前支持的几种策略如下

  a.自定义路由分组:在服务端配置该策略后，可定向指定上游打入该服务的服务范围

  b.同机房优先分组: 开启则优先同机房服务节点

  c.同中心优先分组: 开启则优先同中心服务节点

  d.同城市路由分组: 开启则优先同城市服务节点

  **标签过滤**: 对于服务提供者自定义了标签属性的服务发现流程，可按照传入标签返回对应感兴趣服务提供者信息

##  缓存服务NSC

 ZooKeeper数据快速缓存服务，一方面提供批量服务数据缓存与预处理功能，提升服务发现性能;另一方面增强服务治理链路的fallback能力，本地agent不可用的场景下该模块通过http形式提供远程agent哨兵服务信息。
##  服务健康Scanner

  健康检查模块负责实时检查ZooKeeper中的服务提供者的健康状况，一旦发现服务提供者不可以就会将其置为未启动状态，及时摘除流量；当服务提供者恢复正常时，Scanner会及时将服务提供者的状态置为正常状态，恢复流量。整个服务的生命周期中不需要手动管理服务的状态，实现自动的服务摘除和恢复。为了提高检查的准确性，Scanner支持OCTO自定的应用层心跳协议来进行应用层检查，OCTO体系内的通讯框架目前都已接入，无需额外配置。

### 功能流程

* 服务注册

   octo-ns支持使用NSInvoker(Java SDK)
   和Mns-SDK发起服务注册，可依赖本地或哨兵SGAgent进行，在使用哨兵SG_Agent发起注册时，SDK可通过
   NSC服务获取远程SGAgent哨兵机器列表信息，通过获得的哨兵机器节点发起服务注册流程，服务提供者注册数据会被写入到ZooKeeper中，从管理端Portal服务提供者页面可查看到不同环境注册的服务提供者信息。


<div align=center> <img src="./docs/image/registry_sucess.png" width="500"> </div align=center>

* 服务发现

  octo-ns支持基于HTTP和RPC的服务发现，服务消费者可通过调用SGAgent服务发现接口发起获取服务提供者列表流程，SGAgent会优先从NSC
  进行服务发现，从NSC服务发现失败时会降级从ZooKeeper获取服务提供者数据。

* 健康检查

  scanner运行时会不断扫描Zookeeper中注册的服务节点，然后依次检查每个服务提供者是否正常

<div align=center> <img src="./docs/image/scanner_traits.png" width="400"> </div align=center>

##  使用文档

* [快速开始](https://github.com/smartlife1127/octo-ns/blob/master/docs/ns-quick-start.md)


##  未来规划

* 注册中心AP化

   以NSC为核心，将目前基于ZK的CP系统演进为AP系统，在保证最终一致性的前提下以可用性为优先目标



**Copyright and License**

[Apache 2.0 License](https://github.com/dianping/cat/blob/master/LICENSE)


#####  联系我们

 * Mail: octo@meituan.com 
- [**Issues**](https://github.com/Meituan-Dianping/octo-ns/issues)


 
