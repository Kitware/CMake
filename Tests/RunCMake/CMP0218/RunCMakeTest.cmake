include(RunCMake)

run_cmake_script(CMP0218-WARN)
run_cmake_script(CMP0218-OLD)
run_cmake_script(CMP0218-NEW)

run_cmake_with_options(ExpectIgnore -DCMAKE_WARN_DEPRECATED=OFF)
run_cmake_with_options(ExpectError -DCMAKE_ERROR_DEPRECATED=ON)
