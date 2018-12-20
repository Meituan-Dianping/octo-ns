# Find the muduo includes and library.  Once done this will define
#
#   MUDUO_INCLUDE_DIRS      - where to find muduo include, etc.
#   MUDUO_BASE_LIBRARIES    - List of libraries when using muduo_base.
#   MUDUO_NET_LIBRARIES     - List of libraries when using muduo_net.
#   MUDUO_HTTP_LIBRARIES    - List of libraries when using muduo_http.
#   MUDUO_FOUND             - True if muduo found.
#
set(MUDUO_INCLUDE_DIR /usr/local/include/)
set(MUDUO_BASE_LIBRARY  /usr/local/lib)


find_path(MUDUO_INCLUDE_DIR NAMES muduo PATHS ${MUDUO_INCLUDE_DIR})
find_library(MUDUO_BASE_LIBRARY NAMES muduo_base PATHS ${MUDUO_BASE_LIBRARY})


mark_as_advanced(MUDUO_BASE_LIBRARY MUDUO_INCLUDE_DIR)

# handle the QUIETLY and REQUIRED arguments and set MUDUO_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MUDUO REQUIRED_VARS MUDUO_BASE_LIBRARY
        MUDUO_INCLUDE_DIR)

MESSAGE("MUDUO_INCLUDE_DIR  ${MUDUO_INCLUDE_DIR}")
MESSAGE("MUDUO_BASE_LIBRARY  ${MUDUO_BASE_LIBRARY}")

include(DonwloadMuduo)

if(MUDUO_FOUND)
    set(MUDUO_INCLUDE_DIRS ${MUDUO_INCLUDE_DIR})
    set(MUDUO_BASE_LIBRARIES ${MUDUO_BASE_LIBRARY} pthread rt)
    MESSAGE("MUDUO_FOUND")
else()
    MESSAGE("MUDUO_FOUND NO FOUND Download and compile")
    include(DonwloadMuduo)
endif()

