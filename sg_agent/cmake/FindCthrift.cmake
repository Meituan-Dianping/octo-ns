# FindCthrift
# --------
#
# Find Cthrift
#
# Find the Cthrift includes and library.  Once done this will define
#
#   CTHRIFT_INCLUDE_DIRS      - where to find cthrift include, etc.
#   CTHRIFT_FOUND             - True if cthrift found.
#
set(CTHRIFT_INCLUDE_DIR ${SGAGENT_MODULE_PATH}/cthrift/include)

#message(${CTHRIFT_INCLUDE_DIR})
find_path(CTHRIFT_INCLUDE_DIR NAMES cthrift)

mark_as_advanced(cthrift_INCLUDE_DIR)

# handle the QUIETLY and REQUIRED arguments and set CTHRIFT_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Cthrift REQUIRED_VARS CTHRIFT_INCLUDE_DIR)

if(CTHRIFT_FOUND)
    set(CTHRIFT_INCLUDE_DIRS ${CTHRIFT_INCLUDE_DIR})
endif()

set(CTHRIFT_LIBRARY ${SGAGENT_MODULE_PATH}/cthrift/lib/libcthrift.a)
