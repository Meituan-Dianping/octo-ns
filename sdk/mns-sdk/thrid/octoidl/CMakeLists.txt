
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/octoidl)

SET(THRIFT_GEN_CPP_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../../../common/gen-cpp)

SET(OCTOIDL_SRC
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
        ${THRIFT_GEN_CPP_PATH}/naming_common_types.cpp
        ${THRIFT_GEN_CPP_PATH}/naming_data_types.cpp
        )

MESSAGE("OCTOIDL_SRC ${OCTOIDL_SRC}")

ADD_LIBRARY(octoidl STATIC ${OCTOIDL_SRC})
ADD_LIBRARY(octoidl_dynamic SHARED ${OCTOIDL_SRC})

SET_TARGET_PROPERTIES(octoidl_dynamic PROPERTIES OUTPUT_NAME "octoidl")

#DO NOT add ' OR " to the path
EXECUTE_PROCESS(COMMAND find ${THRIFT_GEN_CPP_PATH}  -name *.h -print -exec cp -t ${INCLUDE_OCTOIDL_OUTPUT_PATH} {} \;)


