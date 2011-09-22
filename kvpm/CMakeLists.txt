 
set(kvpm_SRCS addmirror.cpp kvpmconfigdialog.cpp deviceproperties.cpp 
              devicepropertiesstack.cpp devicesizechart.cpp devicesizechartseg.cpp 
	      devicetab.cpp executablefinder.cpp deviceactionsmenu.cpp fsck.cpp
	      fsblocksize.cpp fsdata.cpp fsextend.cpp dualselectorbox.cpp
	      fsprobe.cpp fsreduce.cpp  kvpmsetup.cpp
	      logvol.cpp lvactionsmenu.cpp lvchange.cpp lvrename.cpp
	      lvcreate.cpp lvproperties.cpp lvpropertiesstack.cpp lvreduce.cpp 
	      lvremove.cpp lvsizechart.cpp main.cpp maxfs.cpp mkfs.cpp masterlist.cpp  mount.cpp 
	      lvsizechartseg.cpp mountentry.cpp mountinfo.cpp mountinfolist.cpp maintabwidget.cpp 
	      pvextend.cpp pvreduce.cpp partremove.cpp pvcheckbox.cpp
	      partaddgraphic.cpp partadd.cpp pvproperties.cpp  pvpropertiesstack.cpp 
	      devicetreeview.cpp physvol.cpp partmoveresize.cpp removefs.cpp
	      misc.cpp pedexceptions.cpp processprogress.cpp pvchange.cpp snapmerge.cpp
	      pvmove.cpp pvtree.cpp removemissing.cpp removemirror.cpp sizeselectorbox.cpp
	      removemirrorleg.cpp storagepartition.cpp storagedevice.cpp storageitem.cpp storagemodel.cpp
	      topwindow.cpp tablecreate.cpp unmount.cpp vgchange.cpp vgexport.cpp vgimport.cpp
	      vgcreate.cpp vgextend.cpp vginfolabels.cpp 
	      vgreduce.cpp vgremove.cpp vgtree.cpp volgroup.cpp vgreduceone.cpp
	      vgrename.cpp volumegrouptab.cpp vgsplit.cpp vgmerge.cpp)
 
kde4_add_executable(kvpm ${kvpm_SRCS})
target_link_libraries(kvpm ${KDE4_KDEUI_LIBS} ${KDE4_KFILE_LIBS} ${QT_LIBRARIES} blkid parted lvm2app)

INSTALL( TARGETS kvpm DESTINATION ${BIN_INSTALL_DIR} )


set_target_properties(kvpm PROPERTIES COMPILE_FLAGS "-std=gnu++98")

