cmake_minimum_required(VERSION 3.10)

message("Setting up ImageTypes parent CMake")
project("Imageypes")

add_subdirectory("red")

add_custom_target(data COMMAND dir SOURCES README.md LICENSE)


