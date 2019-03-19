
ADD_TEST_MACRO(CudaOnly.CircularLinkLine CudaOnlyCircularLinkLine)
ADD_TEST_MACRO(CudaOnly.EnableStandard CudaOnlyEnableStandard)
ADD_TEST_MACRO(CudaOnly.ExportPTX CudaOnlyExportPTX)
ADD_TEST_MACRO(CudaOnly.GPUDebugFlag CudaOnlyGPUDebugFlag)
ADD_TEST_MACRO(CudaOnly.ResolveDeviceSymbols CudaOnlyResolveDeviceSymbols)
ADD_TEST_MACRO(CudaOnly.SeparateCompilation CudaOnlySeparateCompilation)
ADD_TEST_MACRO(CudaOnly.WithDefs CudaOnlyWithDefs)

add_test(NAME CudaOnly.DontResolveDeviceSymbols COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMAKE_CURRENT_SOURCE_DIR}/DontResolveDeviceSymbols/"
  "${CMAKE_CURRENT_BINARY_DIR}/DontResolveDeviceSymbols/"
  ${build_generator_args}
  --build-project DontResolveDeviceSymbols
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )

if(MSVC)
  ADD_TEST_MACRO(CudaOnly.PDB CudaOnlyPDB)
endif()
