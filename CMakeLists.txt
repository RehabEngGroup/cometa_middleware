cmake_minimum_required(VERSION 2.8.6)

project(CometaDataAcquisition)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(WaveAPI REQUIRED)

find_package(Filter REQUIRED)

add_subdirectory(yarp_version)
