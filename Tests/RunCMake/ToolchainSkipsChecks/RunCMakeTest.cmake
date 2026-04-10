include(RunCMake)

function (run_cmake_lang lang)
  if (NOT CMAKE_${lang}_COMPILER_ID)
    return ()
  endif ()
  run_cmake(${lang}-toolchain)
  set(RunCMake-check-file "check.cmake")
  run_cmake_with_options(${lang}
    # "-DCMAKE_${lang}_COMPILER_FORCED=1"
    "-DCMAKE_TOOLCHAIN_FILE=${RunCMake_BINARY_DIR}/${lang}-toolchain-build/Toolchain.cmake")
endfunction ()

run_cmake_lang(C)
run_cmake_lang(CXX)
run_cmake_lang(Fortran)
