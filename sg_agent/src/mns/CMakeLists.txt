INCLUDE_DIRECTORIES(${THRIFT_INCLUDE_DIR})
INCLUDE_DIRECTORIES(/usr/include)
INCLUDE_DIRECTORIES(${CLOG_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${BOOST_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${MUDUO_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${ZOOKEEPER_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${THRIFT_GEN_CPP_PATH})
INCLUDE_DIRECTORIES(${AGENT_SRC_PATH})
INCLUDE_DIRECTORIES(${CTHRIFT_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${UTIL_PATH})
INCLUDE_DIRECTORIES(${RULE_PATH})
INCLUDE_DIRECTORIES(${MCC_PATH})
INCLUDE_DIRECTORIES(${COMMONLOG_PATH})
INCLUDE_DIRECTORIES(${MAFKA_PATH})
INCLUDE_DIRECTORIES(${FALCON_PATH})
INCLUDE_DIRECTORIES(${MONITOR_COLLECTOR_PATH})
INCLUDE_DIRECTORIES(${LOG4CPLUS_PATH})
INCLUDE_DIRECTORIES(${LOG4CPLUS_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${CTHRIFT_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${SDK_INCLUDE_DIR})
INCLUDE_DIRECTORIES(../../thrid_party/log4cplus)
INCLUDE_DIRECTORIES(../../../common/gen-cpp)
INCLUDE_DIRECTORIES(../remote/monitor)
INCLUDE_DIRECTORIES(../util)
INCLUDE_DIRECTORIES(../comm)
INCLUDE_DIRECTORIES(../rule)
INCLUDE_DIRECTORIES(../mns)

SET(THRIFT_GEN_CPP_PATH ../../../common/gen-cpp)
MESSAGE(STATUS "THRIFT_GEN_CPP_PATH DIR ${THRIFT_GEN_CPP_PATH}")

SET(MNS_SDK_LIB thrid_party/cthrift/lib/libmns_sdk.a)

SET(THRIFT_GENCPP_SRCS
	${THRIFT_GEN_CPP_PATH}/MNSCacheService.cpp
		${THRIFT_GEN_CPP_PATH}/mnsc_data_constants.cpp
		${THRIFT_GEN_CPP_PATH}/mnsc_data_types.cpp
		${THRIFT_GEN_CPP_PATH}/mnsc_service_constants.cpp
		${THRIFT_GEN_CPP_PATH}/mnsc_service_types.cpp
		${THRIFT_GEN_CPP_PATH}/naming_common_constants.cpp
		${THRIFT_GEN_CPP_PATH}/naming_common_types.cpp
		${THRIFT_GEN_CPP_PATH}/naming_service_constants.cpp
		${THRIFT_GEN_CPP_PATH}/naming_service_types.cpp
		${THRIFT_GEN_CPP_PATH}/unified_protocol_constants.cpp
		${THRIFT_GEN_CPP_PATH}/unified_protocol_types.cpp
		${THRIFT_GEN_CPP_PATH}/ServiceAgent.cpp
	)

SET(AGENT_COMM_SRCS
        ${COMMON_PATH}/tinyxml2.cc
        ${COMMON_PATH}/md5.cc
        ${COMMON_PATH}/cJSON.c
        )

SET(AGENT_UTIL_SRCS
		${UTIL_PATH}/json_data_tools.cc
		${UTIL_PATH}/zk_path_tools.cc
		${UTIL_PATH}/version_manager.cc
		${UTIL_PATH}/thrift_connection.cc
		${UTIL_PATH}/whitelist_manager.cc
		${UTIL_PATH}/falcon_mgr.cc
		)

SET(AGENT_RULE_SRCS
		${RULE_PATH}/route_rule.cc
		${RULE_PATH}/route_base.cc
		${RULE_PATH}/route_info.cc
		)

SET(MNS_LIB_SRCS
		zk_client.cc
		zk_client_pool.cc
        registry_service.cc
		registry_strategy.cc
		discovery_service.cc
		discovery_zk_client.cc
        hot_load_config.cc
		config_loader.cc
		agent_init.cc
		agent_server.cc
		service_channels.cc
		mnsc_client.cc
		http_service.cc
		)

SET(MONITOR_COLLECTOR_SRCS
		${MONITOR_COLLECTOR_PATH}/monitor_collector.cc
		)
SET(LOG4CPLUS_SRCS
		${LOG4CPLUS_PATH}/log4cplus.cpp
		)

ADD_EXECUTABLE(ServiceAgent ${AGENT_SRC_PATH}/main.cc ${MNS_LIB_SRCS}
	${THRIFT_GENCPP_SRCS} ${AGENT_COMM_SRCS} ${MONITOR_COLLECTOR_SRCS}
	${AGENT_UTIL_SRCS} ${AGENT_RULE_SRCS} ${LOG4CPLUS_SRCS}
	)
TARGET_LINK_LIBRARIES(ServiceAgent -Xlinker "-(" ${MUDUO_BASE_LIBRARY}
		${THRIFTNB_STATIC_LIB} ${THRIFT_STATIC_LIB} ${LOG4CPLUS_LIBRARY}
		${MUDUO_NET_LIBRARY} ${MUDUO_HTTP_LIBRARY} ${MNS_SDK_LIBRARY}
	    ${CTHRIFT_LIBRARY} ${ZOOKEEPER_LIBRARY} boost_system event uuid ssl
		dl pthread rt -Xlinker "-)" )


