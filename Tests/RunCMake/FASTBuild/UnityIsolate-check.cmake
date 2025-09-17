set(REGEX_TO_MATCH "
  Unity\\('main_Unity_1'\\)
  {
.*
    .UnityInputFiles =
    {
      '.*main.cpp',
      '.*some_source_file_1.cpp',
      '.*some_source_file_2.cpp',
      '.*some_source_file_3.cpp',
      '.*some_source_file_4.cpp'
    }
    .UnityInputIsolatedFiles =
    {
      '.*some_source_file_1.cpp',
      '.*some_source_file_4.cpp'
    }
  }
.*ObjectList.*
.*
    .CompilerInputUnity =
    {
      'main_Unity_1'
    }
")

include(${RunCMake_SOURCE_DIR}/check.cmake)
