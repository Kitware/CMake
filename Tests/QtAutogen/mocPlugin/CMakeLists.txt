cmake_minimum_required(VERSION 3.8)

if (NOT QT_TEST_VERSION STREQUAL 5)
  message(SEND_ERROR "Invalid Qt version specified.")
endif()
find_package(Qt5Widgets REQUIRED)

if(Qt5_POSITION_INDEPENDENT_CODE AND CMAKE_CXX_COMPILE_OPTIONS_PIC)
  add_definitions(${CMAKE_CXX_COMPILE_OPTIONS_PIC})
endif()

configure_file(jsonIn/StyleC.json jsonFiles/StyleC.json @ONLY)
configure_file(jsonIn/StyleD.json jsonFiles/sub/StyleD.json @ONLY)

# Enable automoc
set(CMAKE_AUTOMOC TRUE)

include_directories("${CMAKE_CURRENT_BINARY_DIR}/jsonFiles")
link_libraries(Qt5::Widgets)

add_library(PluginStyleA MODULE StyleA.cpp)
add_library(PluginStyleB MODULE StyleB.cpp)
add_library(PluginStyleC MODULE StyleC.cpp)
add_library(PluginStyleD MODULE StyleD.cpp)
add_library(PluginStyleE MODULE StyleE.cpp)
