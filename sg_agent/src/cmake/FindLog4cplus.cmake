# FindLog4cplus
# --------
#
# Find log4cplus
#
# Find the log4cplus includes and library.  Once done this will define
#
#   LOG4CPLUS_INCLUDE_DIRS      - where to find log4cplus include, etc.
#   LOG4CPLUS_FOUND             - True if log4cplus found.
#

set(LOG4CPLUS_INCLUDE_DIR ${SGAGENT_MODULE_PATH}/logs/include/)
set(LOG4CPLUS_LIBRARY ${SGAGENT_MODULE_PATH}/logs/lib/liblog4cplus.a)


#message(${CURL_INCLUDE_DIRS})
find_path(LOG4CPLUS_INCLUDE_DIR NAMES log4cplus)
find_library(LOG4CPLUS_LIBRARY NAMES log4cplus)

mark_as_advanced(LOG4CPLUS_LIBRARY LOG4CPLUS_INCLUDE_DIR)


include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Log4cplus REQUIRED_VARS LOG4CPLUS_LIBRARY
        LOG4CPLUS_INCLUDE_DIR)

set(LOG4CPLUS_FOUND TRUE)
if(LOG4CPLUS_FOUND)
  set(LOG4CPLUS_INCLUDE_DIR ${LOG4CPLUS_INCLUDE_DIR})
  set(LOG4CPLUS_LIBRARY ${LOG4CPLUS_LIBRARY})
  message(STATUS "${LOG4CPLUS_INCLUDE_DIR}")
  message(STATUS "${LOG4CPLUS_LIBRARY}")
endif()