set(REGEX_TO_MATCH "
  Unity\\('main_Unity_1'\\)
  {
    \.UnityOutputPath = 'CMakeFiles/main.dir'
    \.UnityOutputPattern = 'main_Unity_1.cpp'
    \.UnityInputFiles =
    {
      .*main.cpp',
      .*some_source_file_1.cpp'
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
