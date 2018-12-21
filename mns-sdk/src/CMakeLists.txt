cmake_minimum_required(VERSION 2.6)

SET(MNS_SDK_SRC
        mns_sdk.cc
        mns_worker.cc
        thrift_client.cc
        util/mns_sdk_common.cc
        util/mns_log.cc
        util/mns_config.cc
        )

ADD_LIBRARY(mns_sdk STATIC ${MNS_SDK_SRC})
ADD_LIBRARY(mns_sdk_dynamic SHARED ${MNS_SDK_SRC})

SET_TARGET_PROPERTIES(mns_sdk_dynamic PROPERTIES OUTPUT_NAME "mns_sdk")

EXECUTE_PROCESS(COMMAND find ${CMAKE_CURRENT_SOURCE_DIR} -path ${CMAKE_CURRENT_SOURCE_DIR}/util -prune -o -name *.h -exec cp -t ${INCLUDE_MNS_OUTPUT_PATH} {} \;)

#DO NOT add ' OR " to the path
EXECUTE_PROCESS(COMMAND find ${CMAKE_CURRENT_SOURCE_DIR}/util -name *.h -exec cp -t ${INCLUDE_MNS_UTIL_OUTPUT_PATH} {} \;)

if (NOT CMAKE_BUILD_NO_TESTS)
    ADD_SUBDIRECTORY(test)
endif ()
