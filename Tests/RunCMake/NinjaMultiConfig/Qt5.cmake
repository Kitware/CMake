enable_language(CXX)

find_package(Qt5Core REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOMOC_COMPILER_PREDEFINES OFF)

add_executable(exe qt5.cxx)
target_link_libraries(exe PRIVATE Qt5::Core)

include(${CMAKE_CURRENT_LIST_DIR}/Common.cmake)
generate_output_files(exe)

set(moc_writes_depfiles 0)
if(Qt5Core_VERSION VERSION_GREATER_EQUAL "5.15.0")
  set(moc_writes_depfiles 1)
endif()

set(autogen_files "${CMAKE_BINARY_DIR}/exe_autogen/mocs_compilation.cpp")
if(moc_writes_depfiles)
  list(APPEND autogen_files "${CMAKE_BINARY_DIR}/exe_autogen/deps")
  list(APPEND autogen_files "${CMAKE_BINARY_DIR}/exe_autogen/timestamp")
endif()
foreach(c IN LISTS CMAKE_CONFIGURATION_TYPES)
  list(APPEND autogen_files "${CMAKE_BINARY_DIR}/exe_autogen/include_${c}/moc_qt5.cpp")
  if(moc_writes_depfiles)
    list(APPEND autogen_files "${CMAKE_BINARY_DIR}/exe_autogen/include_${c}/moc_qt5.cpp.d")
  endif()
endforeach()
file(APPEND "${CMAKE_BINARY_DIR}/target_files.cmake" "set(AUTOGEN_FILES [==[${autogen_files}]==])\n")
