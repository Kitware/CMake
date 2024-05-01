enable_language(C)
set(CMAKE_PCH_EXTENSION "") # suppress cmake_pch from SOURCES

add_library(foo1 STATIC empty.c)
target_link_libraries(foo1 PRIVATE foo2 foo3)
target_include_directories(foo1 INTERFACE dir1)
target_compile_definitions(foo1 INTERFACE DEF1)
target_compile_features(foo1 INTERFACE cxx_std_11)
target_compile_options(foo1 INTERFACE -opt1)
target_precompile_headers(foo1 INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/empty1.h")
target_sources(foo1 INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/empty1.c")
set_target_properties(foo1 PROPERTIES
  INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/sys1"
  INTERFACE_AUTOMOC_MACRO_NAMES "MOC1"
  INTERFACE_AUTOUIC_OPTIONS "-uic1"
  )

add_library(foo2 STATIC empty.c)
target_include_directories(foo2 INTERFACE dir2)
target_compile_definitions(foo2 INTERFACE DEF2)
target_compile_features(foo2 INTERFACE cxx_std_14)
target_compile_options(foo2 INTERFACE -opt2)
target_precompile_headers(foo2 INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/empty2.h")
target_sources(foo2 INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/empty2.c")
set_target_properties(foo2 PROPERTIES
  INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/sys2"
  INTERFACE_AUTOMOC_MACRO_NAMES "MOC2"
  INTERFACE_AUTOUIC_OPTIONS "-uic2"
  )

add_library(foo3 STATIC empty.c)
target_include_directories(foo3 PRIVATE dir3)
target_compile_definitions(foo3 PRIVATE DEF3)
target_compile_features(foo3 PRIVATE cxx_std_17)
target_compile_options(foo3 PRIVATE -opt3)
target_precompile_headers(foo3 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/empty3.h")
target_sources(foo3 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/empty3.c")
set_target_properties(foo3 PROPERTIES
  SYSTEM_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/sys3"
  AUTOMOC_MACRO_NAMES "MOC3"
  AUTOUIC_OPTIONS "-uic3"
  )

add_executable(main main.c)
target_link_libraries(main PRIVATE foo1)
target_include_directories(main PRIVATE dirM)
target_compile_definitions(main PRIVATE DEFM)
target_compile_features(main PRIVATE cxx_std_20)
target_compile_options(main PRIVATE -optM)
target_precompile_headers(main PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/empty.h")
set_target_properties(main PROPERTIES
  AUTOMOC_MACRO_NAMES "MOCM"
  AUTOUIC_OPTIONS "-uicM"
  )

file(GENERATE OUTPUT out.txt CONTENT "# file(GENERATE) produced:
main INCLUDE_DIRECTORIES: '$<TARGET_PROPERTY:main,INCLUDE_DIRECTORIES>'
main SYSTEM_INCLUDE_DIRECTORIES: '$<TARGET_PROPERTY:main,SYSTEM_INCLUDE_DIRECTORIES>'
main COMPILE_DEFINITIONS: '$<TARGET_PROPERTY:main,COMPILE_DEFINITIONS>'
main COMPILE_FEATURES: '$<TARGET_PROPERTY:main,COMPILE_FEATURES>'
main COMPILE_OPTIONS: '$<TARGET_PROPERTY:main,COMPILE_OPTIONS>'
main PRECOMPILE_HEADERS: '$<TARGET_PROPERTY:main,PRECOMPILE_HEADERS>'
main SOURCES: '$<TARGET_PROPERTY:main,SOURCES>'
main AUTOMOC_MACRO_NAMES: '$<TARGET_PROPERTY:main,AUTOMOC_MACRO_NAMES>'
main AUTOUIC_OPTIONS: '$<TARGET_PROPERTY:main,AUTOUIC_OPTIONS>'
")
