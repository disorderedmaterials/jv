include_directories(
  ./
  ../
  ${CMAKE_CURRENT_BINARY_DIR} 
  ${CMAKE_CURRENT_BINARY_DIR}/../
  ${Qt5Widgets_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
)

# Meta-Objects
SET(jv_MOC_HDRS
  datainterface.h
  jv.h
  rundatawindow.h
  findwindow.h
  licensewindow.h
  logwindow.h
  messenger.hui
  plotwidget.hui
  preview.hui
  quickreport.h
  rbdata.h
  report.h
  samplereport.h
  settings.h
  treplytimeout.hui
  ttablewidgetitem.h
  ttreewidgetitem.h
)
QT5_WRAP_CPP(jv_MOC_SRCS ${jv_MOC_HDRS})

# User Interface Files
SET(jv_UIS
  datainterface.ui
  rundatawindow.ui
  jv.ui
  findwindow.ui
  licensewindow.ui
  logwindow.ui
  quickreport.ui
  report.ui
  samplereport.ui
  settings.ui
)
QT5_WRAP_UI(jv_UIS_H ${jv_UIS})

# Source Files
SET(jv_SRCS
  datainterface_funcs.cpp
  rundatawindow_funcs.cpp
  jv_funcs.cpp
  jv_cli.cpp
  jv_instrument.cpp
  jv_local.cpp
  jv_rundata.cpp
  jv_printing.cpp
  jv_settings.cpp
  findwindow_funcs.cpp
  licensewindow_funcs.cpp
  logwindow_funcs.cpp
  messenger_funcs.cpp
  plotwidget_data.cpp
  plotwidget_funcs.cpp
  plotwidget_paintevent.cpp
  preview_funcs.cpp
  quickreport_funcs.cpp
  report_funcs.cpp
  samplereport_funcs.cpp
  settings_funcs.cpp
  treplytimeout_funcs.cpp
  ttablewidgetitem_funcs.cpp
  ttreewidgetitem_funcs.cpp

  data2d.cpp
  document.cpp
  documentcommands.cpp
  enumeration.cpp
  instrument.cpp
  isis.cpp
  isis_data.cpp
  journal.cpp
  rbdata.cpp
  rundata.cpp
)

# Resources
set(jv_RES_QRC
  icons.qrc 
)
QT5_ADD_RESOURCES(jv_RES ${jv_RES_QRC})

add_dependencies(main HDF5)
# Target 'main'
add_library(main
  ${jv_RES}
  ${jv_UIS_H}
  ${jv_SRCS}
  ${jv_MOC_SRCS}
)

# Compile libget
if(NOT EXTERNAL_LIBGET)
add_subdirectory(get)
endif(NOT EXTERNAL_LIBGET)
