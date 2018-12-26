
# idl-mnsc 源码编译说明

## 1.下载工程
```
  git clone https://github.com/Meituan-Dianping/octo-ns.git
```

## 2.构建Jar包
环境要求： 

- Java version >= 1.7    
- Maven version >= 3.0    

切换到idl-mnsc目录

```
cd common/idl-mns/idl-mnsc
```

本地install，执行后在本地仓库~/.m2/repository/com/meituan/octo/idl-mnsc/（假如你的仓库位置是~/.m2/repository）下可以找到idl-mnsc的jar

```
# common/idl-mns/idl-mnsc 目录下
mvn clean install -Dmaven.test.skip=true
```

```