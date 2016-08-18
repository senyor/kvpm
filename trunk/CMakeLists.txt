cmake_minimum_required(VERSION 3.1)
cmake_policy(SET CMP0059 NEW)
project (kvpm)
 
SET(CMAKE_INSTALL_PREFIX "/usr")

set(QT_MIN_VERSION "5.6.1")
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
  Core
  Gui
  Widgets
)

find_package(ECM 5.23 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_MODULE_PATH})
include(ECMInstallIcons)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)

# Load the frameworks we need
find_package(KF5 5.23 REQUIRED
  CoreAddons 
  DocTools
  I18n
  KDELibs4Support
)

# find_package(KDE4 REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS} ${BLKID_INCLUDE_DIRS})
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

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
