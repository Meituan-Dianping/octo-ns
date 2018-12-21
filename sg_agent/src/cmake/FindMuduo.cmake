# FindMuduo
# --------
#
# Find muduo
#
# Find the muduo includes and library.  Once done this will define
#
#   MUDUO_INCLUDE_DIRS      - where to find muduo include, etc.
#   MUDUO_BASE_LIBRARIES    - List of libraries when using muduo_base.
#   MUDUO_NET_LIBRARIES     - List of libraries when using muduo_net.
#   MUDUO_HTTP_LIBRARIES    - List of libraries when using muduo_http.
#   MUDUO_FOUND             - True if muduo found.
#
set(MUDUO_INCLUDE_DIR /usr/include/)
set(MUDUO_INCLUDE_BASE_DIR /usr/include/muduo/base)
set(MUDUO_INCLUDE_NET_DIR /usr/include/muduo/muduo/net)
set(MUDUO_BASE_LIBRARY  /usr/lib64/libmuduo_base.a)
set(MUDUO_NET_LIBRARY  /usr/lib64/libmuduo_net.a)
set(MUDUO_HTTP_LIBRARY /usr/lib64/libmuduo_http.a)

#message(${MUDUO_INCLUDE_DIRS})
find_path(MUDUO_INCLUDE_DIR NAMES muduo)
find_library(MUDUO_BASE_LIBRARY NAMES muduo_base)
find_library(MUDUO_NET_LIBRARY NAMES muduo_net)
find_library(MUDUO_HTTP_LIBRARY NAMES muduo_http)

mark_as_advanced(MUDUO_BASE_LIBRARY MUDUO_NET_LIBRARY
                 MUDUO_HTTP_LIBRARY MUDUO_INCLUDE_DIR)

# handle the QUIETLY and REQUIRED arguments and set MUDUO_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Muduo REQUIRED_VARS MUDUO_BASE_LIBRARY MUDUO_NET_LIBRARY
                                                      MUDUO_HTTP_LIBRARY MUDUO_INCLUDE_DIR)

if(MUDUO_FOUND)
    set(MUDUO_INCLUDE_DIRS ${MUDUO_INCLUDE_DIR})
    set(MUDUO_BASE_LIBRARIES ${MUDUO_BASE_LIBRARY} pthread rt)
    set(MUDUO_NET_LIBRARIES ${MUDUO_NET_LIBRARY} ${MUDUO_BASE_LIBRARIES})
    set(MUDUO_HTTP_LIBRARIES ${MUDUO_HTTP_LIBRARY} ${MUDUO_NET_LIBRARIES})
endif()

