set(REGEX_TO_MATCH "
  Unity\\('main_Unity_1'\\)
  {
.*
    .UnityOutputPattern = 'main_Unity_1.cpp'
    .UnityInputFiles =
    {
      '.*main.cpp',
      '.*some_source_file_1.cpp'
    }
  }
  Unity\\('main_Unity_2'\\)
  {
.*
    .UnityOutputPattern = 'main_Unity_2.cpp'
    .UnityInputFiles =
    {
      '.*some_source_file_2.cpp',
      '.*some_source_file_3.cpp'
    }
  }
.*ObjectLis.*
.*
    .CompilerInputUnity =
    {
      'main_Unity_1',
      'main_Unity_2'
    }
")

include(${RunCMake_SOURCE_DIR}/check.cmake)
