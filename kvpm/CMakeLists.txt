 
set(kvpm_SRCS changemirror.cpp kvpmconfigdialog.cpp deviceproperties.cpp 
              devicepropertiesstack.cpp devicesizechart.cpp devicesizechartseg.cpp 
	      devicetree.cpp devicetab.cpp executablefinder.cpp deviceactionsmenu.cpp fsck.cpp
	      fsblocksize.cpp fsdata.cpp fsextend.cpp dualselectorbox.cpp
	      fsprobe.cpp fsreduce.cpp logvol.cpp lvactionsmenu.cpp lvchange.cpp lvrename.cpp
	      lvcreate.cpp lvcreatebase.cpp lvproperties.cpp lvpropertiesstack.cpp lvreduce.cpp 
	      lvremove.cpp lvsizechart.cpp main.cpp maxfs.cpp mkfs.cpp masterlist.cpp  mount.cpp 
	      lvsizechartseg.cpp mountentry.cpp mounttables.cpp maintabwidget.cpp 
	      pvextend.cpp pvreduce.cpp partremove.cpp pvgroupbox.cpp
	      partitiongraphic.cpp partadd.cpp pvproperties.cpp  pvpropertiesstack.cpp 
	      physvol.cpp partchange.cpp removefs.cpp progressbox.cpp
	      misc.cpp pedexceptions.cpp processprogress.cpp pvchange.cpp snapmerge.cpp
	      pvmove.cpp pvtree.cpp removemissing.cpp removemirror.cpp sizeselectorbox.cpp
	      removemirrorleg.cpp storagepartition.cpp storagedevice.cpp thincreate.cpp
	      topwindow.cpp tablecreate.cpp unmount.cpp vgchange.cpp vgexport.cpp vgimport.cpp
	      vgcreate.cpp vgextend.cpp vginfolabels.cpp 
	      vgreduce.cpp vgremove.cpp vgtree.cpp volgroup.cpp vgreduceone.cpp
	      vgrename.cpp volumegrouptab.cpp vgsplit.cpp vgmerge.cpp)
 
kde4_add_executable(kvpm ${kvpm_SRCS})
target_link_libraries(kvpm ${KDE4_KDEUI_LIBS} ${KDE4_KFILE_LIBS} ${QT_LIBRARIES} blkid parted lvm2app)

INSTALL( TARGETS kvpm DESTINATION ${SBIN_INSTALL_DIR} )


IF (CMAKE_COMPILER_IS_GNUCXX)
set_target_properties(kvpm PROPERTIES COMPILE_FLAGS "-std=gnu++98 -Wno-psabi -Wno-cast-align")
set_target_properties(kvpm PROPERTIES LINK_FLAGS "-Wl,--as-needed")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

SET(CMAKE_CXX_FLAGS_DEBUG "-ggdb")