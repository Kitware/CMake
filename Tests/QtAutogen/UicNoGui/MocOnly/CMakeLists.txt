# Library uses QtCore only (no Widgets)
add_library(mocOnly main.cpp)
target_link_libraries(mocOnly ${QT_QTCORE_TARGET})
