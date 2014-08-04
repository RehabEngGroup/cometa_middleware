
include(YarpIDL)
set(generated_libs_dir "${CMAKE_CURRENT_SOURCE_DIR}/../gen_msgs")
yarp_idl_to_dir(ceinms_msgs/EmgData.msg ${generated_libs_dir})