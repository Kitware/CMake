# Verify that:
# 1. Non-isolated files from different subdirs are combined in one Unity node
# 2. Isolated files from subdirs are in separate ObjectLists with proper subdir paths

# Check that Unity node contains the non-isolated files (main.cpp and unity_file.cpp)
# but not the isolated file. The isolated file should be compiled separately with
# its subdirectory in the output path.
set(REGEX_TO_MATCH "
  Unity\\('main_Unity_1'\\)
  {
    \\.UnityOutputPath = 'CMakeFiles/main.dir'
    \\.UnityOutputPattern = 'main_Unity_1.cpp'
    \\.UnityInputFiles =
    {
      .*main.cpp',
      .*subdir1/unity_file.cpp'
    }
  }
.*ObjectList.*
.*
    \\.CompilerOutputPath = 'CMakeFiles/main.dir/subdir2'
.*
      .*isolated_file.cpp'
")

include(${RunCMake_SOURCE_DIR}/check.cmake)
