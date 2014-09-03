cmake_minimum_required(VERSION 2.6)
project (kvpm)
 
SET(CMAKE_INSTALL_PREFIX "/usr")

find_package(KDE4 REQUIRED)
include_directories( ${KDE4_INCLUDES} ${QT4_INCLUDES} )
add_subdirectory( kvpm )
add_subdirectory( docbook )
add_subdirectory( images )
add_subdirectory( icons/local )
add_subdirectory( icons/app )
 

IF(CMAKE_BUILD_TYPE EQUAL "Debian")
  SET(CMAKE_BUILD_TYPE "RelWithDebInfo")
ENDIF()

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE "RelWithDebInfo")
ENDIF(NOT CMAKE_BUILD_TYPE)

install(FILES kvpm.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES "kvpm.1" DESTINATION ${MAN_INSTALL_DIR}/man1 COMPONENT doc)