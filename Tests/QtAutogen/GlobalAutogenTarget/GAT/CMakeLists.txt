cmake_minimum_required(VERSION 3.12)
project(GAT)
include("../../AutogenCoreTest.cmake")

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Enable AUTOMOC/UIC/RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
# Disable ORIGN_DEPENDS and enable AUTOGEN global targets
set(CMAKE_AUTOGEN_ORIGIN_DEPENDS OFF)
set(CMAKE_GLOBAL_AUTOGEN_TARGET ON)
set(CMAKE_GLOBAL_AUTORCC_TARGET ON)

add_subdirectory(sda)
add_subdirectory(sdb)
add_subdirectory(sdc)

# Add custom target that depends on all autogen/autorcc targets
add_custom_target(all_autogen DEPENDS autogen global_autogen_sdb global_autogen_sdc)
add_custom_target(all_autorcc DEPENDS autorcc global_autorcc_sdb global_autorcc_sdc)

# Main target
add_executable(gat data.qrc item.cpp main.cpp)
target_link_libraries(gat ${QT_LIBRARIES})
target_link_libraries(gat sda sdb sdc)
