cmake_minimum_required(VERSION 3.8)


# I use this idiom to do things like finding a vcpkg toolchain file
# that is machine-specific and doesn't need to be in git.
# Don't worry about it if you don't need it.
if(EXISTS  ${CMAKE_CURRENT_SOURCE_DIR}/site.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/site.cmake)
else()
    message("You can make a 'site.cmake' file in ${CMAKE_CURRENT_SOURCE_DIR} that will get auto included if you need to override something.")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/)

project(red_demo)

# A Nonsense "data" target can be convenient if you open
# this project in an IDE, so misc. files show up in the project GUI.
add_custom_target(data COMMAND dir SOURCES README.md)



find_package(OpenImageIO REQUIRED)
find_package(R3DSDK)


add_executable(${PROJECT_NAME} "main.cpp" "w_red_util.h" "w_red_util.cpp")

target_link_libraries(${PROJECT_NAME} ${OPENIMAGEIO_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC ${OPENIMAGEIO_INCLUDE_DIR})

if(NOT RED_FOUND)
    message("Skipping R3D SDK Support.")
    target_compile_definitions(${PROJECT_NAME} PUBLIC "-DUSE_RED=0")
else()
    message("Building with R3D SDK Support.")
    target_link_libraries(${PROJECT_NAME} ${RED_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PUBLIC ${RED_INCLUDE_DIR})
    target_compile_definitions(${PROJECT_NAME} PUBLIC "-DUSE_RED=1" "-DREDRUNTIME=\"${RED_RUNTIME_LIBRARIES}\"")

endif()

