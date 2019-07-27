cmake_minimum_required(VERSION 3.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/)

project(red_demo)
find_package(OpenImageIO REQUIRED)
find_package(R3DSDK)


add_executable(${PROJECT_NAME} "main.cpp" "w_red_util.h" "w_red_util.cpp")

target_link_libraries(${PROJECT_NAME} ${OPENIMAGEIO_LIBRARIES})

if(NOT RED_FOUND)
    message("Skipping R3D SDK Support.")
    target_compile_definitions(${PROJECT_NAME} PUBLIC "-DUSE_RED=0")
else()
    message("Building with R3D SDK Support.")
    target_link_libraries(${PROJECT_NAME} ${RED_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PUBLIC ${RED_INCLUDE_DIR})
    target_compile_definitions(${PROJECT_NAME} PUBLIC "-DUSE_RED=1" "-DREDRUNTIME=\"${RED_RUNTIME_LIBRARIES}\"")

endif()

