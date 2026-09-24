set(CMAKE_CXX_SCANDEP_SOURCE "")
enable_language(CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MSVC_RUNTIME_LIBRARY_DEFAULT "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
unset(CMAKE_MSVC_RUNTIME_LIBRARY)

add_library(runtime-module INTERFACE IMPORTED)
target_sources(runtime-module INTERFACE
  FILE_SET modules TYPE CXX_MODULES FILES sources/module.cxx)
set_property(TARGET runtime-module PROPERTY MSVC_RUNTIME_LIBRARY MultiThreadedDLL)

foreach(runtime IN ITEMS MultiThreaded MultiThreadedDLL MultiThreadedDebug MultiThreadedDebugDLL)
  set(CMAKE_CXX_COMPILE_OPTIONS_MSVC_RUNTIME_LIBRARY_${runtime} "-runtime-${runtime}")
  set(CMAKE_MSVC_RUNTIME_LIBRARY "${runtime}")
  add_library(${runtime} OBJECT main-no-use.cxx)
  target_link_libraries(${runtime} PRIVATE runtime-module)
endforeach()

set(CMAKE_MSVC_RUNTIME_LIBRARY "")
add_library(empty-runtime OBJECT main-no-use.cxx)
target_link_libraries(empty-runtime PRIVATE runtime-module)

unset(CMAKE_MSVC_RUNTIME_LIBRARY)
add_library(default-runtime OBJECT main-no-use.cxx)
target_link_libraries(default-runtime PRIVATE runtime-module)

add_library(genex-runtime OBJECT main-no-use.cxx)
set_target_properties(genex-runtime PROPERTIES
  CUSTOM_RUNTIME "MultiThreaded"
  MSVC_RUNTIME_LIBRARY "$<TARGET_PROPERTY:CUSTOM_RUNTIME>$<$<CONFIG:Debug>:Debug>")
target_link_libraries(genex-runtime PRIVATE runtime-module)
