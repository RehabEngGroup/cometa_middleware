project(cometa_emg_yarp)

find_package(YARP REQUIRED)
list(APPEND CMAKE_MODULE_PATH ${YARP_MODULE_PATH})
include_directories(${WaveAPI_INCLUDE_DIR} ${YARP_INCLUDE_DIRS})

add_subdirectory(rosmsgs)

include_directories(gen_msgs/include)
include_directories(${Filter_INCLUDE_DIRS})
add_executable(cometa_emg_yarp cometa_yarp.cpp)
target_link_libraries(cometa_emg_yarp ${WaveAPI_LIBRARY} ${YARP_LIBRARIES})
