set(REGEX_TO_MATCH "
.*ObjectList.*
.*
    \.CompilerInputFiles =
    {
      '.*main.cpp',
      '.*some_source_file_1.cpp'
    }
")
include(${RunCMake_SOURCE_DIR}/check.cmake)
