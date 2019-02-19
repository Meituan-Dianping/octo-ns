#  NSC 介绍

## 背景描述
NSC 是 OCTO 服务治理体系内的缓存服务数据子系统.

开发语言: Java

运行环境要求: JDK 1.8及以上、Maven3、ZooKeeper

组件依赖: [Dorado](https://github.com/Meituan-Dianping/octo-rpc/tree/master/dorado)

主要功能点:

1. 缓存服务注册信息，为OCTO组件提供批量获取服务列表功能
2. 提供SGAgent哨兵列表，本地SGAgent不可用时，业务服务FallBack到哨兵服务，保证OCTO健壮性


## 实现细节
### 数据同步机制
被动Notify + Push的机制保障数据实效性；周期性的Pull同步模式，确保数据完整性。

### 异步模式
采用异步模式处理Watcher触发后的逻辑，避免同步模式下大量Watcher阻塞执行，排队耗费大量等待时间。

### 过滤zk事件
网络抖动时，产生的网络事件会导致ZkClient进行Watcher实例重复注册；频繁网络抖动可能导致进程OOM。

### 队列分离
分队列处理不同优先级任务，保证核心任务快速响应

## 快速上手
### 依赖构建    
[idl-common依赖构建](https://github.com/Meituan-Dianping/octo-ns/tree/master/common/idl-mns/idl-common/Compile.md)      
[idl-mnsc依赖构建](https://github.com/Meituan-Dianping/octo-ns/tree/master/common/idl-mns/idl-mnsc/Compile.md)    
[mns-invoker依赖构建](https://github.com/Meituan-Dianping/octo-ns/tree/master/mns-invoker/docs/Compile.md)    
[Dorado依赖构建](https://github.com/Meituan-Dianping/octo-rpc/blob/master/dorado/dorado-doc/manual-developer/Compile.md) 




### 打包运行NSC


```bash
git clone git@github.com:Meituan-Dianping/octo-ns.git

cd octo-ns/mns-cache

sh ./run.sh

#需要在启动提前在octo.cfg参数中指定zookeeper地址
#日志会输出到当前路径下"/opt/logs/com.meituan.octo.mnsc/mnsc-server.log"文件

```


