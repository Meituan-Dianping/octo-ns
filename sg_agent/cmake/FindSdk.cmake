# FindSdk
# --------
#
# Find SDK
#
set(MNS_SDK_LIBRARY ${THIRD_MODULE_PATH}/thrid/lib/libmns_sdk.a)

find_library(MNS_SDK_LIBRARY NAMES libmns_sdk.a)
set(SDK_INCLUDE_DIR ${THIRD_MODULE_PATH}/thrid/include/)
mark_as_advanced(MNS_SDK_LIBRARY MNS_SDK_INCLUDE_DIR)

# handle the QUIETLY and REQUIRED arguments and set ZOOKEEPER_FOUND to TRUE if
# all listed variables are TRUE

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(sdk REQUIRED_VARS MNS_SDK_LIBRARY)

