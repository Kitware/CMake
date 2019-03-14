set(CMAKE_GLOBAL_AUTOGEN_TARGET_NAME "global_autogen_sdb")
set(CMAKE_GLOBAL_AUTORCC_TARGET_NAME "global_autorcc_sdb")

add_library(sdb ../item.cpp ../data.qrc sdb.cpp)
target_link_libraries(sdb ${QT_LIBRARIES})
