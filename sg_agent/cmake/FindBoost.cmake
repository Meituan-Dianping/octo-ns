# FindBoost
# --------
#
# Find boost
#
# Find the boost includes and library.  Once done this will define
#
#   BOOST_INCLUDE_DIRS      - where to find boost include, etc.
#   BOOST_FOUND             - True if boost found.
#
set(BOOST_INCLUDE_DIR ${THIRD_MODULE_PATH}/boost_1_59_0/)

#message(${BOOST_INCLUDE_DIRS})
find_path(BOOST_INCLUDE_DIR NAMES boost)

mark_as_advanced(BOOST_INCLUDE_DIR)

# handle the QUIETLY and REQUIRED arguments and set BOOST_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Boost REQUIRED_VARS BOOST_INCLUDE_DIR)

if(BOOST_FOUND)
    set(BOOST_INCLUDE_DIRS ${BOOST_INCLUDE_DIR})
endif()

