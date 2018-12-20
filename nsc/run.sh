#!/usr/bin/env bash
JVM_ARGS="-server -Dfile.encoding=UTF-8 -Dsun.jnu.encoding=UTF-8 -Djava.io.tmpdir=/tmp -Djava.net.preferIPv6Addresses=false"
JVM_GC="-XX:+DisableExplicitGC -XX:+PrintGCDetails -XX:+PrintHeapAtGC -XX:+PrintTenuringDistribution -XX:+UseConcMarkSweepGC -XX:+PrintGCTimeStamps -XX:+PrintGCDateStamps"
JVM_GC=$JVM_GC" -XX:CMSFullGCsBeforeCompaction=0 -XX:+UseCMSCompactAtFullCollection -XX:CMSInitiatingOccupancyFraction=80"
JVM_HEAP="-XX:SurvivorRatio=8 -XX:PermSize=256m -XX:MaxPermSize=256m -XX:+HeapDumpOnOutOfMemoryError -XX:ReservedCodeCacheSize=128m -XX:InitialCodeCacheSize=128m"
JVM_SIZE="-Xmx4g -Xms4g -Xmn2g"

EXEC_JAVA="java $JVM_ARGS $JVM_SIZE $JVM_HEAP $JVM_GC"
EXEC_JAVA=$EXEC_JAVA" $JAVA_ARGS"

mvn clean -U package -Dmaven.test.skip=true
cd target
$EXEC_JAVA -jar mnsc-1.0.0.jar