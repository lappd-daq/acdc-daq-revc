cmake_minimum_required(VERSION 3.1)
project(acdc-daq-revc)
set(acdc-daq-revc_VERSION_MAJOR 1)
set(acdc-daq-revc VERSION_MINOR 1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_SOURCE_DIR "${CMAKE_SOURCE_DIR}/lib")
set(EXECUTABLE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")

#include(ExternalProject)
#ExternalProject_Add(project_yaml_cpp
#        GIT_REPOSITORY "https://github.com/jbeder/yaml-cpp"
#        GIT_TAG "yaml-cpp-0.6.3"
#        INSTALL_COMMAND ""
#        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/yaml-cpp)
#ExternalProject_Get_Property(project_yaml_cpp source_dir)
#ExternalProject_Get_Property(project_yaml_cpp binary_dir)

#add_library(yaml-cpp SHARED IMPORTED)
#add_dependencies(yaml-cpp project_yaml_cpp)
#set_property(TARGET yaml-cpp PROPERTY IMPORTED_LOCATION ${binary_dir}/libyaml-cpp.a)


include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${source_dir}/include)
include_directories(${source_dir}/local/include)


add_compile_options(-g -Wall -Wextra -Wpedantic -Wno-psabi -fstack-protector -Wno-unused-variable) 

add_library(ACC
        ${LIBRARY_SOURCE_DIR}/ACC.cpp)

add_library(ACDC
        ${LIBRARY_SOURCE_DIR}/ACDC.cpp)

add_library(myusb
        ${LIBRARY_SOURCE_DIR}/stdUSB.cpp)

add_library(Metadata
        ${LIBRARY_SOURCE_DIR}/Metadata.cpp)

#add_library(Config
#        ${LIBRARY_SOURCE_DIR}/Config.cpp)
#add_library(Scope
#	${LIBRARY_SOURCE_DIR}/Scope.cpp)

target_link_libraries(ACC myusb ACDC)
target_link_libraries(ACDC Metadata)
#target_link_libraries(Config ACC myusb yaml-cpp)
target_link_libraries(myusb PUBLIC usb-1.0)
#target_link_libraries(Scope ACC ACDC)




add_executable(connectedBoards ${EXECUTABLE_SOURCE_DIR}/connectedBoards.cpp)
add_executable(calibratePed ${EXECUTABLE_SOURCE_DIR}/calibratePed.cpp)
add_executable(setPed ${EXECUTABLE_SOURCE_DIR}/setPed.cpp)
add_executable(debug ${EXECUTABLE_SOURCE_DIR}/debug.cpp)
add_executable(listenForData ${EXECUTABLE_SOURCE_DIR}/listenForData.cpp)
#add_executable(onlyListen ${EXECUTABLE_SOURCE_DIR}/onlyListen.cpp)
#add_executable(onlySetup ${EXECUTABLE_SOURCE_DIR}/onlySetup.cpp)
#add_executable(DataCalibToConfig ${EXECUTABLE_SOURCE_DIR}/DataCalibToConfig.cpp)

target_link_libraries(connectedBoards PUBLIC myusb)
target_link_libraries(calibratePed PUBLIC ACC)
target_link_libraries(setPed PUBLIC ACC myusb)
target_link_libraries(debug PUBLIC myusb)
target_link_libraries(listenForData PUBLIC ACC)
#target_link_libraries(onlyListen PUBLIC ACC Scope)
#target_link_libraries(onlySetup PUBLIC ACC)
#target_link_libraries(DataCalibToConfig PUBLIC ACC)


