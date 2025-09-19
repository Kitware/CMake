set(REGEX_TO_MATCH "
  Unity\\('main_Unity_Group_TestGroup_2'\\)
.*
    .UnityOutputPattern = 'main_Unity_Group_TestGroup_2.cpp'
    .UnityInputFiles =
    {
      '.*some_source_file_1.cpp',
      '.*some_source_file_2.cpp'
    }
.*
  ObjectList.*
.*
    \.CompilerInputUnity =
    {
      'main_Unity_Group_TestGroup_2'
    }
    \.CompilerInputFiles =
    {
      '.*main.cpp'
    }
")

include(${RunCMake_SOURCE_DIR}/check.cmake)
