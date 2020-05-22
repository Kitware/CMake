
ADD_TEST_MACRO(CudaOnly.CircularLinkLine CudaOnlyCircularLinkLine)
ADD_TEST_MACRO(CudaOnly.EnableStandard CudaOnlyEnableStandard)
ADD_TEST_MACRO(CudaOnly.ExportPTX CudaOnlyExportPTX)
ADD_TEST_MACRO(CudaOnly.GPUDebugFlag CudaOnlyGPUDebugFlag)
ADD_TEST_MACRO(CudaOnly.ResolveDeviceSymbols CudaOnlyResolveDeviceSymbols)
ADD_TEST_MACRO(CudaOnly.SeparateCompilation CudaOnlySeparateCompilation)
ADD_TEST_MACRO(CudaOnly.SharedRuntimePlusToolkit CudaOnlySharedRuntimePlusToolkit)
ADD_TEST_MACRO(CudaOnly.SharedRuntimeViaCUDAFlags CudaOnlySharedRuntimeViaCUDAFlags)
ADD_TEST_MACRO(CudaOnly.Standard98 CudaOnlyStandard98)
ADD_TEST_MACRO(CudaOnly.Toolkit CudaOnlyToolkit)
ADD_TEST_MACRO(CudaOnly.WithDefs CudaOnlyWithDefs)

# The CUDA only ships the shared version of the toolkit libraries
# on windows
if(NOT WIN32)
  ADD_TEST_MACRO(Cuda.StaticRuntimePlusToolkit StaticRuntimePlusToolkit)
endif()

if(MSVC)
  ADD_TEST_MACRO(CudaOnly.PDB CudaOnlyPDB)
endif()

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

add_test(NAME CudaOnly.RuntimeControls COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMAKE_CURRENT_SOURCE_DIR}/RuntimeControls/"
  "${CMAKE_CURRENT_BINARY_DIR}/RuntimeControls/"
  --build-two-config
    ${build_generator_args}
  --build-project RuntimeControls
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
