cmake_minimum_required(VERSION 3.7)

project(QtAutogen)

# Tell find_package(Qt5) where to find Qt.
if(QT_QMAKE_EXECUTABLE)
  get_filename_component(Qt_BIN_DIR "${QT_QMAKE_EXECUTABLE}" PATH)
  get_filename_component(Qt_PREFIX_DIR "${Qt_BIN_DIR}" PATH)
  set(CMAKE_PREFIX_PATH ${Qt_PREFIX_DIR})
endif()

if (QT_TEST_VERSION STREQUAL 4)
  find_package(Qt4 REQUIRED)

  # Include this directory before using the UseQt4 file.
  add_subdirectory(defines_test)

  include(UseQt4)

  set(QT_QTCORE_TARGET Qt4::QtCore)

  macro(qtx_wrap_cpp)
    qt4_wrap_cpp(${ARGN})
  endmacro()

else()
  if (NOT QT_TEST_VERSION STREQUAL 5)
    message(SEND_ERROR "Invalid Qt version specified.")
  endif()
  find_package(Qt5Widgets REQUIRED)

  set(QT_QTCORE_TARGET Qt5::Core)

  include_directories(${Qt5Widgets_INCLUDE_DIRS})
  set(QT_LIBRARIES Qt5::Widgets)

  if(Qt5_POSITION_INDEPENDENT_CODE AND CMAKE_CXX_COMPILE_OPTIONS_PIC)
    add_definitions(${CMAKE_CXX_COMPILE_OPTIONS_PIC})
  endif()

  macro(qtx_wrap_cpp)
    qt5_wrap_cpp(${ARGN})
  endmacro()

endif()

get_property(QT_COMPILE_FEATURES TARGET ${QT_QTCORE_TARGET} PROPERTY INTERFACE_COMPILE_FEATURES)

# -- Test: AUTORCC
# RCC only
add_executable(rccOnly rccOnly.cpp rccOnlyRes.qrc)
set_property(TARGET rccOnly PROPERTY AUTORCC ON)
target_link_libraries(rccOnly ${QT_QTCORE_TARGET})

# -- Test: AUTORCC
# RCC empty
add_executable(rccEmpty rccEmpty.cpp rccEmptyRes.qrc)
set_property(TARGET rccEmpty PROPERTY AUTORCC ON)
target_link_libraries(rccEmpty ${QT_QTCORE_TARGET})

# -- Test: AUTOUIC
# UIC only
qtx_wrap_cpp(uicOnlyMoc uicOnlySource/uiconly.h)
add_executable(uicOnly uicOnlySource/uiconly.cpp ${uicOnlyMoc})
set_property(TARGET uicOnly PROPERTY AUTOUIC ON)
target_link_libraries(uicOnly ${QT_LIBRARIES})

# -- Test: AUTOMOC, AUTORCC
# Add not_generated_file.qrc to the source list to get the file-level
# dependency, but don't generate a c++ file from it.  Disable the AUTORCC
# feature for this target.  This tests that qrc files in the sources don't
# have an effect on generation if AUTORCC is off.
add_library(empty STATIC empty.cpp not_generated_file.qrc)
set_target_properties(empty PROPERTIES AUTORCC OFF)
set_target_properties(empty PROPERTIES AUTOMOC TRUE)
target_link_libraries(empty no_link_language)
add_library(no_link_language STATIC empty.h)
set_target_properties(no_link_language PROPERTIES AUTOMOC TRUE)
# Pass Qt compiler features to targets that don't link against Qt
target_compile_features(no_link_language PRIVATE ${QT_COMPILE_FEATURES})
target_compile_features(empty PRIVATE ${QT_COMPILE_FEATURES})

# -- Test: AUTORCC
# When a file listed in a .qrc file changes the target must be rebuilt
try_compile(RCC_DEPENDS
  "${CMAKE_CURRENT_BINARY_DIR}/autorcc_depends"
  "${CMAKE_CURRENT_SOURCE_DIR}/autorcc_depends"
  autorcc_depends
  CMAKE_FLAGS "-DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}" "-DQT_TEST_VERSION=${QT_TEST_VERSION}"
              "-DCMAKE_PREFIX_PATH=${Qt_PREFIX_DIR}"
  OUTPUT_VARIABLE output
)
if (NOT RCC_DEPENDS)
  message(SEND_ERROR "Initial build of autorcc_depends failed. Output: ${output}")
endif()

file(STRINGS "${CMAKE_CURRENT_BINARY_DIR}/autorcc_depends/info_file.txt" qrc_files)

list(GET qrc_files 0 qrc_file1)

set(timeformat "%Y%j%H%M%S")

file(TIMESTAMP "${qrc_file1}" file1_before "${timeformat}")

execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1) # Ensure that the timestamp will change.
execute_process(COMMAND "${CMAKE_COMMAND}" -E touch "${CMAKE_CURRENT_BINARY_DIR}/autorcc_depends/res1/input.txt")

execute_process(COMMAND "${CMAKE_COMMAND}" --build .
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/autorcc_depends"
)

file(TIMESTAMP "${qrc_file1}" file1_step1 "${timeformat}")

if (NOT file1_step1 GREATER file1_before)
  message(SEND_ERROR "file1 (${qrc_file1}) should have changed in the first step!")
endif()

# -- Test: AUTOMOC
# Ensure a repeated build succeeds when a header containing a QObject changes
try_compile(MOC_RERUN
  "${CMAKE_CURRENT_BINARY_DIR}/automoc_rerun"
  "${CMAKE_CURRENT_SOURCE_DIR}/automoc_rerun"
  automoc_rerun
  CMAKE_FLAGS "-DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}" "-DQT_TEST_VERSION=${QT_TEST_VERSION}"
              "-DCMAKE_PREFIX_PATH=${Qt_PREFIX_DIR}"
  OUTPUT_VARIABLE output
)
if (NOT MOC_RERUN)
  message(SEND_ERROR "Initial build of automoc_rerun failed. Output: ${output}")
endif()

configure_file(automoc_rerun/test1.h.in2 automoc_rerun/test1.h COPYONLY)

execute_process(COMMAND "${CMAKE_COMMAND}" --build .
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/automoc_rerun"
  RESULT_VARIABLE automoc_rerun_result
  )
if (automoc_rerun_result)
  message(SEND_ERROR "Second build of automoc_rerun failed.")
endif()

# -- Test: AUTOMOC, SKIP_AUTOMOC
# Test for SKIP_AUTOMOC and SKIP_AUTOGEN on an AUTOMOC enabled target
qtx_wrap_cpp(skipMocWrapMoc
  skipSource/qItemA.hpp
  skipSource/qItemB.hpp)
set(skipMocSources
  skipMoc.cpp
  skipSource/qItemA.cpp
  skipSource/qItemB.cpp
  skipSource/qItemC.cpp)
set_property(SOURCE skipSource/qItemA.cpp PROPERTY SKIP_AUTOMOC ON)
set_property(SOURCE skipSource/qItemB.cpp PROPERTY SKIP_AUTOGEN ON)
# AUTOMOC enabled only
add_executable(skipMocA ${skipMocSources} ${skipMocWrapMoc})
set_property(TARGET skipMocA PROPERTY AUTOMOC ON)
target_link_libraries(skipMocA ${QT_LIBRARIES})
# AUTOMOC and AUTOUIC enabled
add_executable(skipMocB ${skipMocSources} ${skipMocWrapMoc})
set_property(TARGET skipMocB PROPERTY AUTOMOC ON)
set_property(TARGET skipMocB PROPERTY AUTOUIC ON)
target_link_libraries(skipMocB ${QT_LIBRARIES})

# -- Test: AUTOUIC, SKIP_AUTOUIC
# Test for SKIP_AUTOUIC and SKIP_AUTOGEN on an AUTOUIC enabled target
set(skipUicSources
  skipUic.cpp
  skipSource/skipUicGen.cpp
  skipSource/skipUicNoGen1.cpp
  skipSource/skipUicNoGen2.cpp
)
set_property(SOURCE skipSource/skipUicNoGen1.cpp PROPERTY SKIP_AUTOUIC ON)
set_property(SOURCE skipSource/skipUicNoGen2.cpp PROPERTY SKIP_AUTOGEN ON)
# AUTOUIC enabled
add_executable(skipUicA ${skipUicSources})
set_property(TARGET skipUicA PROPERTY AUTOUIC ON)
target_link_libraries(skipUicA ${QT_LIBRARIES})
# AUTOUIC and AUTOMOC enabled
add_executable(skipUicB ${skipUicSources})
set_property(TARGET skipUicB PROPERTY AUTOUIC ON)
set_property(TARGET skipUicB PROPERTY AUTOMOC ON)
target_link_libraries(skipUicB ${QT_LIBRARIES})

# -- Test: AUTORCC, SKIP_AUTORCC
# Test for SKIP_AUTORCC and SKIP_AUTOGEN on an AUTORCC enabled target
set(skipRccSources
  skipRcc.cpp
  skipSource/skipRccBad1.qrc
  skipSource/skipRccBad2.qrc
  skipSource/skipRccGood.qrc
)
set_property(SOURCE skipSource/skipRccBad1.qrc PROPERTY SKIP_AUTORCC ON)
set_property(SOURCE skipSource/skipRccBad2.qrc PROPERTY SKIP_AUTOGEN ON)
# AUTORCC enabled
add_executable(skipRccA ${skipRccSources})
set_property(TARGET skipRccA PROPERTY AUTORCC ON)
target_link_libraries(skipRccA ${QT_LIBRARIES})
# AUTORCC, AUTOUIC and AUTOMOC enabled
add_executable(skipRccB ${skipRccSources})
set_property(TARGET skipRccB PROPERTY AUTORCC ON)
set_property(TARGET skipRccB PROPERTY AUTOUIC ON)
set_property(TARGET skipRccB PROPERTY AUTOMOC ON)
target_link_libraries(skipRccB ${QT_LIBRARIES})

# -- Test: AUTOMOC AUTORCC
# Source files with the same basename in different subdirectories
add_subdirectory(sameName)

# -- Test: AUTOMOC AUTORCC AUTOUIC
# Complex test case
add_subdirectory(complex)
