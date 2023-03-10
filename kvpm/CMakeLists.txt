 
set(kvpm_SRCS allocationpolicy.cpp changemirror.cpp kvpmconfigdialog.cpp deviceproperties.cpp 
              devicepropertiesstack.cpp devicesizechart.cpp devicesizechartseg.cpp externalraid.cpp
	      devicetree.cpp devicetab.cpp executablefinder.cpp devicemenu.cpp deviceactions.cpp fsck.cpp 
	      fsblocksize.cpp fsdata.cpp fsextend.cpp dualselectorbox.cpp fsprobe.cpp fsreduce.cpp 
	      kvpmdialog.cpp lvactions.cpp logvol.cpp lvactionsmenu.cpp lvchange.cpp lvrename.cpp
	      lvcreate.cpp lvcreatebase.cpp lvproperties.cpp lvpropertiesstack.cpp lvreduce.cpp 
	      lvremove.cpp lvsizechart.cpp lvmconfig main.cpp maxfs.cpp mkfs.cpp masterlist.cpp
	      mount.cpp lvsizechartseg.cpp mountentry.cpp mounttables.cpp maintabwidget.cpp 
	      pvextend.cpp pvreduce.cpp partremove.cpp pvgroupbox.cpp partbase.cpp
	      partitiongraphic.cpp partadd.cpp pvproperties.cpp  pvpropertiesstack.cpp 
	      physvol.cpp partchange.cpp progressbox.cpp repairmissing.cpp partflag.cpp
	      pvactions.cpp pvactionsmenu.cpp resync.cpp
	      misc.cpp pedexceptions.cpp processprogress.cpp pvchange.cpp snapmerge.cpp
	      pvmove.cpp pvtree.cpp vgremovemissing.cpp removemirror.cpp sizeselectorbox.cpp
	      removemirrorleg.cpp storagepartition.cpp storagedevice.cpp storagebase.cpp 
	      thincreate.cpp topwindow.cpp tablecreate.cpp unmount.cpp vgchange.cpp vgexport.cpp 
	      vgimport.cpp vgcreate.cpp vgextend.cpp vginfolabels.cpp vgactions.cpp
	      vgreduce.cpp vgremove.cpp vgtree.cpp volgroup.cpp vgwarning.cpp
	      vgrename.cpp volumegrouptab.cpp vgsplit.cpp vgmerge.cpp)

add_executable(kvpm ${kvpm_SRCS})
target_link_libraries(kvpm KF5::CoreAddons KF5::I18n KF5::KDELibs4Support blkid parted lvm2app devmapper)

INSTALL( TARGETS kvpm DESTINATION ${SBIN_INSTALL_DIR} )

set_target_properties(kvpm PROPERTIES COMPILE_FLAGS "-Wall -Wno-psabi -Wno-cast-align -Wstrict-aliasing")
set_target_properties(kvpm PROPERTIES LINK_FLAGS "-Wl,--as-needed")

IF(CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_CXX_FLAGS "-std=gnu++11")
  SET(CMAKE_C_FLAGS "-std=gnu99")
ELSE()
  SET(CMAKE_CXX_FLAGS "-std=c++11")
  SET(CMAKE_C_FLAGS "-std=c99")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

SET(CMAKE_CXX_FLAGS_DEBUG "-ggdb -O0")
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-ggdb -O2")
