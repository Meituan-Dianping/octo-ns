
# mns-invoker 源码编译说明

## 1.下载工程
```
  git clone https://github.com/Meituan-Dianping/octo-ns.git
```

## 2.构建Jar包
环境要求： 

- Java version >= 1.8   
- Maven version >= 3.2    

切换到mns-invoker目录

```
cd sdk/mns-invoker
```

本地install，执行后在本地仓库~/.m2/repository/com/meituan/octo/mns-invoker/（假如你的仓库位置是~/
.m2/repository）下可以找到mns-invoker的jar

```
# 切换到mns-invoker 目录下
mvn clean install -Dmaven.test.skip=true
```

```
