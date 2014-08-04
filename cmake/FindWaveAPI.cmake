# Find Cometa's WaveAPI library
# Author: Elena Ceseracciu   <elena.ceseracciu@gmail.com>

find_path(WaveAPI_INCLUDE_DIR WaveAPI.h
          PATHS ENV WAVEAPI_ROOT
          PATH_SUFFIXES Include )

if(NOT WaveAPI_FIND_QUIETLY)
    message( STATUS "WaveAPI include path: ${WaveAPI_INCLUDE_DIR}")
endif()

find_library(WaveAPI_LIBRARY
             NAMES WaveAPI
             PATHS ENV WAVEAPI_ROOT
             PATH_SUFFIXES Lib)

if(NOT WaveAPI_FIND_QUIETLY)
    message( STATUS "WaveAPI library: ${WaveAPI_LIBRARY}")
endif()

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set OPENSIM_FOUND to TRUE
# if all listed variables are TRUE
# DEFAULT_MSG is predefined... change only if you need a custom msg
find_package_handle_standard_args(WaveAPI DEFAULT_MSG
                                  WaveAPI_INCLUDE_DIR WaveAPI_LIBRARY)