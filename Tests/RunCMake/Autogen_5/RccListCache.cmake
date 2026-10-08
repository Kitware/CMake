enable_language(CXX)

find_package(Qt${with_qt_version} REQUIRED COMPONENTS Core)

add_library(dummy STATIC ../Autogen_common/example.cpp "${RCC_DATA_DIR}/res.qrc")
target_link_libraries(dummy Qt${with_qt_version}::Core)
set_target_properties(dummy PROPERTIES AUTORCC ON)
