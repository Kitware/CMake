set(RunCMake_TEST_NO_CMP0129 ON)
include(RunCMake)

foreach(lang C CXX Fortran)
  run_cmake(${lang})
  run_cmake_with_options(${lang} "-DSET_CMP0129=NEW")
  run_cmake_with_options(${lang} "-DSET_CMP0129=OLD")
endforeach()
