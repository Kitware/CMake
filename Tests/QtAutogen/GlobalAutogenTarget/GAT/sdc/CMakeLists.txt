set(CMAKE_GLOBAL_AUTOGEN_TARGET_NAME "global_autogen_sdc")
set(CMAKE_GLOBAL_AUTORCC_TARGET_NAME "global_autorcc_sdc")

add_library(sdc ../item.cpp ../data.qrc sdc.cpp)
target_link_libraries(sdc ${QT_LIBRARIES})
