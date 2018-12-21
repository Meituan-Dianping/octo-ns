# - Find Thrift (a cross platform RPC lib/tool)
# This module defines
#  THRIFT_VERSION, version string of ant if found
#  THRIFT_INCLUDE_DIR, where to find THRIFT headers
#  THRIFT_CONTRIB_DIR, where contrib thrift files (e.g. fb303.thrift) are installed
#  THRIFT_LIBS, THRIFT libraries
#  THRIFT_FOUND, If false, do not try to use ant

# prefer the thrift version supplied in THRIFT_HOME
set(THRIFT_INCLUDE_DIR ${THIRD_MODULE_PATH}/thrift-0.8.0_build/include)
find_path(THRIFT_INCLUDE_DIR thrift/Thrift.h)

set(THRIFT_CONTRIB_DIR ${THIRD_MODULE_PATH}/thrift-0.8.0_build/contrib)
find_path(THRIFT_CONTRIB_DIR fb303/if/fb303.thrift)

set (THRIFT_LIB_PATHS ${THIRD_MODULE_PATH}/thrift-0.8.0_build/lib)
find_path(THRIFT_LIB_PATH libthrift.a PATHS ${THRIFT_LIB_PATHS})

set (THRIFT_STATIC_LIB_PATH ${THIRD_MODULE_PATH}/thrift-0.8.0_build/lib)
find_path(THRIFT_STATIC_LIB_PATH libthrift.a PATHS ${THRIFT_LIB_PATHS})

# prefer the thrift version supplied in THRIFT_HOME
find_library(THRIFT_LIB NAMES thrift HINTS ${THRIFT_LIB_PATHS})
find_library(THRIFTNB_LIB NAMES thriftnb HINTS ${THRIFT_LIB_PATHS})

set(THRIFT_COMPILER ${THIRD_MODULE_PATH}/thrift-0.8.0_build/bin/thrift)
find_program(THRIFT_COMPILER thrift)

if (THRIFT_LIB)
  set(THRIFT_FOUND TRUE)
  set(THRIFT_LIBS ${THIRD_MODULE_PATH}/thrift-0.8.0_build/lib/libthrift.so)
  set(THRIFT_STATIC_LIB ${THRIFT_STATIC_LIB_PATH}/libthrift.a)
  set(THRIFTNB_STATIC_LIB ${THRIFT_STATIC_LIB_PATH}/libthriftnb.a)
  exec_program(${THRIFT_COMPILER}
    ARGS -version OUTPUT_VARIABLE THRIFT_VERSION RETURN_VALUE THRIFT_RETURN)
else ()
  set(THRIFT_FOUND FALSE)
endif ()

if (THRIFT_FOUND)
  if (NOT THRIFT_FIND_QUIETLY)
    message(STATUS "${THRIFT_VERSION}")
  endif ()
else ()
  message(STATUS "Thrift compiler/libraries NOT found. "
          "Thrift support will be disabled (${THRIFT_RETURN}, "
          "${THRIFT_INCLUDE_DIR}, ${THRIFT_LIB})")
endif ()


mark_as_advanced(
  THRIFT_LIB
  THRIFT_COMPILER
  THRIFT_INCLUDE_DIR
)
