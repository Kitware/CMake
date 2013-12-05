
find_package(Qt4 REQUIRED)

set(QT_CORE_TARGET Qt4::QtCore)
set(QT_GUI_TARGET Qt4::QtGui)

add_library(KI18n INTERFACE)
set_property(TARGET KI18n APPEND PROPERTY
  INTERFACE_AUTOUIC_OPTIONS -tr ki18n
)

add_library(OtherI18n INTERFACE)
set_property(TARGET OtherI18n APPEND PROPERTY
  INTERFACE_AUTOUIC_OPTIONS -tr otheri18n
)

add_library(LibWidget empty.cpp)
target_link_libraries(LibWidget KI18n OtherI18n ${QT_GUI_TARGET})
