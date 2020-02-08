cmake_minimum_required(VERSION 3.1)
project(acdc-daq)
set(acdc-daq_VERSION_MAJOR 2)
set(acdc-daq VERSION_MINOR 1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_SOURCE_DIR "${CMAKE_SOURCE_DIR}/lib")
set(EXECUTABLE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")

include(ExternalProject)
ExternalProject_Add(project_yaml_cpp
        GIT_REPOSITORY "https://github.com/jbeder/yaml-cpp"
        GIT_TAG "yaml-cpp-0.6.2"
        INSTALL_COMMAND ""
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/yaml-cpp)
ExternalProject_Get_Property(project_yaml_cpp source_dir)
ExternalProject_Get_Property(project_yaml_cpp binary_dir)

add_library(yaml-cpp SHARED IMPORTED)
add_dependencies(yaml-cpp project_yaml_cpp)
set_property(TARGET yaml-cpp PROPERTY IMPORTED_LOCATION ${binary_dir}/libyaml-cpp.a)


include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${source_dir}/include)


add_compile_options(-g -Wall -Wextra -Wpedantic -fstack-protector)

add_library(ACC
        ${LIBRARY_SOURCE_DIR}/ACC.cpp)

add_library(ACDC
        ${LIBRARY_SOURCE_DIR}/ACDC.cpp)


add_library(myusb
        ${LIBRARY_SOURCE_DIR}/stdUSB.cpp)

target_link_libraries(myusb PRIVATE usb)

target_link_libraries(ACC myusb ACDC yaml-cpp)

add_executable(readACDC ${EXECUTABLE_SOURCE_DIR}/readACDC.cpp)



target_link_libraries(readACDC PUBLIC ACC)

