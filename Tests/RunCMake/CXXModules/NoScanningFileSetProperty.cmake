enable_language(CXX)
unset(CMAKE_CXX_SCANDEP_SOURCE)

add_executable(noscanning-fs-property)
target_sources(noscanning-fs-property PRIVATE
  FILE_SET SOURCES FILES sources/module-use.cxx)
set_target_properties(noscanning-fs-property
  PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_SCAN_FOR_MODULES 0)
set_source_files_properties(sources/module-use.cxx
  PROPERTIES
    CXX_SCAN_FOR_MODULES 0)
set_property(FILE_SET SOURCES TARGET noscanning-fs-property
             PROPERTY CXX_SCAN_FOR_MODULES 1)
