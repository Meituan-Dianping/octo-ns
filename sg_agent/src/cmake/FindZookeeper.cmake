# FindZookeeper
# --------
#
# Find zookeeper
#
# Find the zookeeper includes and library.  Once done this will define
#
#   ZOOKEEPER_LIBRARIES    - List of libraries when using zookeeper_base.
#   ZOOKEEPER_FOUND             - True if zookeeper found.
#
set(ZOOKEEPER_LIBRARY ${THIRD_MODULE_PATH}/zookeeper/libzookeeper_mt.a)

find_library(ZOOKEEPER_LIBRARY NAMES zookeeper_mt)

set(ZOOKEEPER_INCLUDE_DIR ${THIRD_MODULE_PATH}/zookeeper/include)

mark_as_advanced(ZOOKEEPER_LIBRARY ZOOKEEPER_INCLUDE_DIR) 

# handle the QUIETLY and REQUIRED arguments and set ZOOKEEPER_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Zookeeper REQUIRED_VARS ZOOKEEPER_LIBRARY)

if(ZOOKEEPER_FOUND)
    set(ZOOKEEPER_LIBRARIES ${ZOOKEEPER_LIBRARY})
endif()

