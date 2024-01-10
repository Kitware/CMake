cmake_minimum_required(VERSION 3.24...3.28)
project(scan_properties CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

set(scanning_control 1)
if (CMAKE_GENERATOR MATCHES "Visual Studio")
  set(scanning_control 0)
endif ()

# To detect that not-to-be scanned sources are not scanned, add a `-D` to the
# scan flags so that the files can detect whether scanning happened and error
# if not.
string(APPEND CMAKE_CXX_MODULE_MAP_FLAG
  " -DCMAKE_SCANNED_THIS_SOURCE")
string(REPLACE "<DEFINES>" "<DEFINES> -DCMAKE_SCANNED_THIS_SOURCE"
  CMAKE_CXX_SCANDEP_SOURCE "${CMAKE_CXX_SCANDEP_SOURCE}")

set_property(SOURCE always_scan.cxx
  PROPERTY CXX_SCAN_FOR_MODULES 1)
set_property(SOURCE never_scan.ixx
  PROPERTY CXX_SCAN_FOR_MODULES 0)
set_property(SOURCE never_scan.ixx
  PROPERTY LANGUAGE CXX)

add_executable(scans_everything)
target_sources(scans_everything
  PRIVATE
    main.cxx
    join.cxx
    always_scan.cxx
    never_scan.ixx
  PRIVATE
    FILE_SET CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        module.cxx)
target_compile_features(scans_everything PRIVATE cxx_std_20)
target_compile_definitions(scans_everything PRIVATE SCAN_AT_TARGET_LEVEL=1)
target_compile_definitions(scans_everything PRIVATE "SCANNING_CONTROL=${scanning_control}")

set(CMAKE_CXX_SCAN_FOR_MODULES 0)

add_executable(no_scan_everything)
target_sources(no_scan_everything
  PRIVATE
    main.cxx
    join.cxx
    always_scan.cxx
    never_scan.ixx
  PRIVATE
    FILE_SET CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        module.cxx)
target_compile_features(no_scan_everything PRIVATE cxx_std_20)
target_compile_definitions(no_scan_everything PRIVATE SCAN_AT_TARGET_LEVEL=0)
target_compile_definitions(no_scan_everything PRIVATE "SCANNING_CONTROL=${scanning_control}")

add_test(NAME scanned COMMAND scans_everything)
add_test(NAME unscanned COMMAND no_scan_everything)
