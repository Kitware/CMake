enable_language(CXX)
set(CMAKE_CXX_STANDARD 20)

find_package(Qt${with_qt_version} REQUIRED COMPONENTS Core)

add_library(foo STATIC ../Autogen_common/example.cpp)
set_target_properties(foo PROPERTIES
  AUTOMOC ON
  CXX_SCAN_FOR_MODULES OFF
)
