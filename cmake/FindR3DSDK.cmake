# - Try to find R3D SDK
#
# Inputs:
#
#  Set R3DSDK_ROOT Environment variable to root of the SDK
#    Otherwise it will try to guess some plausible paths.
#  Set R3D_LIB_NAME if it is guessing the wrong library and causing linker errors.
#
# Outputs - Once done, this will define:
#
#  R3DSDK_FOUND - system has R3D SDK
#  R3DSDK_INCLUDE_DIR - the include directory
#  R3DSDK_LIBRARIES - link these at build time.
#  R3DSDK_RUNTIME_LIBRARIES - Use this path (or copy the contents to a path)
#          for R3DSDK::InitializeSdk() at runtime.

include (FindPackageHandleStandardArgs)

# Assuming only C++11 and 64 bit support is needed with new-ish compiler.
# Set R3D_LIB_NAME before invoking this file to override.
if (NOT DEFINED R3D_LIB_NAME)
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        set(R3D_LIB_NAME R3DSDKPIC-cpp11)
    else()
        # It's rumored that there was once someone who completely understood
        # the differences between the various MSVCRT MT, MD, MDd, etc. options.
        # But if such a person ever did exist, they certainly can't be found here.

        # It seems likely that it's more reliable to just tell you to set RED_LIB_NAME
        # if this is wrong, rather than make it ocmplicated enough to try fully
        # parsing out all the combinations of compiler, build mode, etc.

        # message(${CMAKE_BUILD_TYPE})
        if(CMAKE_BUILD_TYPE MATCHES Debug)
            set(R3D_LIB_NAME R3DSDK-2017MDd)
        else()
            set(R3D_LIB_NAME R3DSDK-2017MD)
        endif()
    endif()
endif()



if(DEFINED ENV{R3DSDK_ROOT})
    set(R3DSDK_ROOT $ENV{R3DSDK_ROOT})
    # message("Using R3D SDK Root: " ${R3DSDK_ROOT})
else()
    # message("Looking for R3D SDK")

    # The R3D SDK is supplied as a zip file that you can unpack anywhere.
    # So try some plausible locations and hope for the best.
    # Use a glob to try and avoid dealing with exact SDK version numbers.

    file(GLOB R3DSDK_PATHS "/opt/R3DSDKv*"
        "C:/R3DSDKv*"
        ${CMAKE_CURRENT_SOURCE_DIR}/../R3DSDKv*
        ${CMAKE_CURRENT_SOURCE_DIR}/../../R3DSDKv*
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../R3DSDKv*
        )
    message("Looking in " ${R3DSDK_PATHS})
    find_path(R3DSDK_ROOT Include/R3DSDK.h
              PATHS ${R3DSDK_PATHS}
             )
endif()

set(R3DSDK_INCLUDE_DIR ${R3DSDK_ROOT}/Include)
find_library(R3D_LIBRARY NAMES ${R3D_LIB_NAME}
             PATHS ${R3DSDK_ROOT}/Lib/linux64/
                   ${R3DSDK_ROOT}/Lib/win64/
             NO_DEFAULT_PATH
            )
                   
# handle the QUIETLY and REQUIRED arguments and set RED_FOUND to TRUE if
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(RED DEFAULT_MSG R3D_LIBRARY R3DSDK_INCLUDE_DIR)


# TODO : Ask somebody with a Mac to add support.
if(R3DSDK_FOUND)
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        set(R3D_LIBRARIES ${RED_LIBRARY} dl)
        set(R3DSDK_RUNTIME_LIBRARIES ${RED_ROOT}/Redistributable/linux)
    else()
        set(R3D_LIBRARIES ${RED_LIBRARY})
        set(R3DSDK_RUNTIME_LIBRARIES ${R3DSDK_ROOT}/Redistributable/win)
    endif()
endif()

# message("R3D SDK: " ${RED_INCLUDE_DIR} "  " ${RED_LIBRARIES} "  " ${RED_RUNTIME_LIBRARIES})

mark_as_advanced(R3D_LIBRARY R3DSDK_INCLUDE_DIR)

