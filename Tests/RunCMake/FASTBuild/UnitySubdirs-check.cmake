# Verify that files from different subdirectories are combined into a single Unity node.

set(REGEX_TO_MATCH "
  Unity\\('main_Unity_1'\\)
  {
    \\.UnityOutputPath = 'CMakeFiles/main.dir'
    \\.UnityOutputPattern = 'main_Unity_1.cpp'
    \\.UnityInputFiles =
    {
      .*main.cpp',
      .*subdir1/file1.cpp',
      .*subdir2/file2.cpp'
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
