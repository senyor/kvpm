cmake_minimum_required(VERSION 2.6)
project (kvpm)
 
find_package(KDE4 REQUIRED)
include_directories( ${KDE4_INCLUDES} ${QT4_INCLUDES} )
 
set(kvpm_SRCS addmirror.cpp kvpmconfigdialog.cpp devicemodel.cpp deviceproperties.cpp 
              devicepropertiesstack.cpp devicesizechart.cpp devicesizechartseg.cpp 
	      devicetab.cpp executablefinder.cpp deviceactionsmenu.cpp fsck.cpp
	      fsblocksize.cpp fsextend.cpp
	      fsprobe.cpp fsreduce.cpp  kvpmsetup.cpp
	      logvol.cpp lvactionsmenu.cpp lvchange.cpp lvrename.cpp
	      lvcreate.cpp lvproperties.cpp lvpropertiesstack.cpp lvreduce.cpp 
	      lvremove.cpp lvsizechart.cpp main.cpp mkfs.cpp masterlist.cpp  mount.cpp 
	      lvsizechartseg.cpp mountentry.cpp mountinfo.cpp maintabwidget.cpp 
	      pvextend.cpp pvreduce.cpp partremove.cpp
	      partaddgraphic.cpp partadd.cpp pvproperties.cpp  pvpropertiesstack.cpp 
	      devicetreeview.cpp physvol.cpp partmoveresize.cpp removefs.cpp
	      misc.cpp processprogress.cpp pvchange.cpp 
	      pvmove.cpp pvtree.cpp removemissing.cpp removemirror.cpp 
	      removemirrorleg.cpp storagepartition.cpp storagedevice.cpp topwindow.cpp  
	      tablecreate.cpp unmount.cpp 
	      vgchangealloc.cpp vgchangeavailable.cpp vgexport.cpp vgimport.cpp
	      vgchangeextent.cpp vgchangelv.cpp vgchangepv.cpp 
	      vgchangeresize.cpp vgcreate.cpp vgextend.cpp vginfolabels.cpp 
	      vgreduce.cpp vgremove.cpp vgtree.cpp volgroup.cpp vgreduceone.cpp
	      vgrename.cpp volumegrouptab.cpp vgsplit.cpp vgmerge.cpp)
 
kde4_add_executable(kvpm ${kvpm_SRCS})
target_link_libraries(kvpm ${KDE4_KDEUI_LIBS} ${KDE4_KFILE_LIBS} ${QT_LIBRARIES} blkid parted lvm2app)
INSTALL(TARGETS kvpm DESTINATION bin)
set_target_properties(kvpm PROPERTIES COMPILE_FLAGS "-std=gnu++98")

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)
