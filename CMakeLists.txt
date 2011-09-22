cmake_minimum_required(VERSION 2.6)
project (kvpm)
 
SET(CMAKE_INSTALL_PREFIX "/usr")

find_package(KDE4 REQUIRED)
include_directories( ${KDE4_INCLUDES} ${QT4_INCLUDES} )
add_subdirectory( kvpm )
add_subdirectory( docbook )
add_subdirectory( icons/local )
add_subdirectory( icons/app )
 
set_target_properties(kvpm PROPERTIES COMPILE_FLAGS "-std=gnu++98")

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE "RelWithDebInfo")
ENDIF(NOT CMAKE_BUILD_TYPE)
