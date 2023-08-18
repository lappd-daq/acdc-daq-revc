cmake_minimum_required(VERSION 3.1)
project(acdc-daq-revc)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_SOURCE_DIR "${CMAKE_SOURCE_DIR}/lib")
set(EXECUTABLE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${source_dir}/include)
include_directories(${source_dir}/local/include)

add_compile_options(-Wall -Wextra -Wpedantic -Wno-psabi -fstack-protector -Wno-unused-variable -Wunused-parameter -Wformat=0)

add_library(ACC_USB ${LIBRARY_SOURCE_DIR}/ACC_USB.cpp)
add_library(ACC_ETH ${LIBRARY_SOURCE_DIR}/ACC_ETH.cpp)
add_library(DataParser ${LIBRARY_SOURCE_DIR}/DataParser.cpp)
add_library(USB ${LIBRARY_SOURCE_DIR}/stdUSB.cpp)
add_library(Ethernet ${LIBRARY_SOURCE_DIR}/Ethernet.cpp)

target_link_libraries(ACC_ETH DataParser Ethernet)
target_link_libraries(ACC_USB USB DataParser)
target_link_libraries(USB PUBLIC usb-1.0)

add_executable(TestConnection ${EXECUTABLE_SOURCE_DIR}/TestConnection.cpp)
add_executable(Command ${EXECUTABLE_SOURCE_DIR}/Command.cpp)
add_executable(ConnectedBoards ${EXECUTABLE_SOURCE_DIR}/ConnectedBoards.cpp)
add_executable(SpeedTest ${EXECUTABLE_SOURCE_DIR}/SpeedTest.cpp)

target_link_libraries(TestConnection PUBLIC Ethernet)
target_link_libraries(Command PUBLIC Ethernet)
target_link_libraries(ConnectedBoards PUBLIC Ethernet USB ACC_USB ACC_ETH)
target_link_libraries(SpeedTest PUBLIC Ethernet USB ACC_USB ACC_ETH)



