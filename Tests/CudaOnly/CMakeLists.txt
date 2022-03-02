macro (add_cuda_test_macro name)
  add_test_macro("${name}" ${ARGN})
  set_property(TEST "${name}" APPEND
    PROPERTY LABELS "CUDA")
endmacro ()

add_cuda_test_macro(CudaOnly.Architecture Architecture)
add_cuda_test_macro(CudaOnly.ArchSpecial CudaOnlyArchSpecial)
add_cuda_test_macro(CudaOnly.CompileFlags CudaOnlyCompileFlags)

add_cuda_test_macro(CudaOnly.EnableStandard CudaOnlyEnableStandard)
add_cuda_test_macro(CudaOnly.ExportPTX CudaOnlyExportPTX)
add_cuda_test_macro(CudaOnly.SharedRuntimePlusToolkit CudaOnlySharedRuntimePlusToolkit)
add_cuda_test_macro(CudaOnly.Standard98 CudaOnlyStandard98)
add_cuda_test_macro(CudaOnly.Toolkit CudaOnlyToolkit)
add_cuda_test_macro(CudaOnly.ToolkitBeforeLang CudaOnlyToolkitBeforeLang)
add_cuda_test_macro(CudaOnly.WithDefs CudaOnlyWithDefs)
add_cuda_test_macro(CudaOnly.CircularLinkLine CudaOnlyCircularLinkLine)
add_cuda_test_macro(CudaOnly.ResolveDeviceSymbols CudaOnlyResolveDeviceSymbols)
add_cuda_test_macro(CudaOnly.SeparateCompilation main/CudaOnlySeparateCompilation)
add_cuda_test_macro(CudaOnly.SeparateCompilationPTX CudaOnlySeparateCompilationPTX)

if(CMake_TEST_CUDA AND NOT CMake_TEST_CUDA STREQUAL "Clang")
  # Clang doesn't have flags for selecting the runtime.
  add_cuda_test_macro(CudaOnly.SharedRuntimeViaCUDAFlags CudaOnlySharedRuntimeViaCUDAFlags)

  # Only NVCC defines __CUDACC_DEBUG__ when compiling in debug mode.
  add_cuda_test_macro(CudaOnly.GPUDebugFlag CudaOnlyGPUDebugFlag)
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
set_property(TEST "CudaOnly.DontResolveDeviceSymbols" APPEND
  PROPERTY LABELS "CUDA")

# The CUDA only ships the shared version of the toolkit libraries
# on windows
if(NOT WIN32)
  add_cuda_test_macro(CudaOnly.StaticRuntimePlusToolkit CudaOnlyStaticRuntimePlusToolkit)
endif()

if(MSVC)
  add_cuda_test_macro(CudaOnly.PDB CudaOnlyPDB)
endif()

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
set_property(TEST "CudaOnly.RuntimeControls" APPEND
  PROPERTY LABELS "CUDA")
