cmake_minimum_required(VERSION 2.6)
project (kvpm)
 
find_package(KDE4 REQUIRED)
include_directories( ${KDE4_INCLUDES} ${QT4_INCLUDES} )
 
set(kvpm_SRCS addmirror.cpp kvpmconfigdialog.cpp devicemodel.cpp deviceproperties.cpp 
	     devicepropertiesstack.cpp devicesizechart.cpp devicesizechartseg.cpp 
	     devicetab.cpp executablefinder.cpp deviceactionsmenu.cpp
	     fsprobe.cpp logvol.cpp lvactionsmenu.cpp lvchange.cpp lvrename.cpp
	     lvcreate.cpp lvproperties.cpp lvpropertiesstack.cpp lvreduce.cpp 
	     lvremove.cpp lvsizechart.cpp main.cpp mkfs.cpp masterlist.cpp  mount.cpp 
	     lvsizechartseg.cpp mountentry.cpp mountinfo.cpp partremove.cpp 
	     partaddgraphic.cpp partadd.cpp pvproperties.cpp  pvpropertiesstack.cpp 
	     devicetreeview.cpp physvol.cpp partmoveresize.cpp
	     nomungecheck.cpp processprogress.cpp pvchange.cpp pvcreate.cpp 
	     pvmove.cpp pvremove.cpp pvtree.cpp removemissing.cpp removemirror.cpp 
	     removemirrorleg.cpp
	     sizetostring.cpp storagepartition.cpp storagedevice.cpp topwindow.cpp 
	     tablecreate.cpp unmount.cpp 
	     vgchangealloc.cpp vgchangeavailable.cpp vgexport.cpp vgimport.cpp
	     vgchangeextent.cpp vgchangelv.cpp vgchangepv.cpp 
	     vgchangeresize.cpp vgcreate.cpp vgextend.cpp vginfolabels.cpp 
	     vgreduce.cpp vgremove.cpp vgtree.cpp volgroup.cpp vgreduceone.cpp
	     vgrename.cpp volumegrouptab.cpp)
 
kde4_add_executable(kvpm ${kvpm_SRCS})
target_link_libraries(kvpm ${KDE4_KDEUI_LIBS} ${KDE4_KFILE_LIBS} ${QT_LIBRARIES} blkid parted)
INSTALL(TARGETS kvpm DESTINATION bin)
