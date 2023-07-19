macro (add_cuda_test_macro name)
  add_test_macro("${name}" ${ARGN})
  set_property(TEST "${name}" APPEND
    PROPERTY LABELS "CUDA")
endmacro ()

add_cuda_test_macro(Cuda.CXXStandardSetTwice CXXStandardSetTwice)
add_cuda_test_macro(Cuda.ObjectLibrary CudaObjectLibrary)
add_cuda_test_macro(Cuda.MixedStandardLevels1 MixedStandardLevels1)
add_cuda_test_macro(Cuda.MixedStandardLevels2 MixedStandardLevels2)
add_cuda_test_macro(Cuda.MixedStandardLevels3 MixedStandardLevels3)
add_cuda_test_macro(Cuda.MixedStandardLevels4 MixedStandardLevels4)
add_cuda_test_macro(Cuda.MixedStandardLevels5 MixedStandardLevels5)
add_cuda_test_macro(Cuda.NotEnabled CudaNotEnabled)
add_cuda_test_macro(Cuda.SeparableCompCXXOnly SeparableCompCXXOnly)
add_cuda_test_macro(Cuda.StubRPATH StubRPATH)
add_cuda_test_macro(Cuda.Toolkit Toolkit)
add_cuda_test_macro(Cuda.IncludePathNoToolkit IncludePathNoToolkit)
add_cuda_test_macro(Cuda.SharedRuntimePlusToolkit SharedRuntimePlusToolkit)
add_cuda_test_macro(Cuda.StaticRuntimePlusToolkit StaticRuntimePlusToolkit)
add_cuda_test_macro(Cuda.Complex CudaComplex)
add_cuda_test_macro(Cuda.ProperLinkFlags ProperLinkFlags)

if(CMake_TEST_CUDA AND NOT CMake_TEST_CUDA STREQUAL "Clang")
  # Clang lacks __CUDACC_VER*__ defines.
  add_cuda_test_macro(Cuda.ProperDeviceLibraries ProperDeviceLibraries)
endif()

add_cuda_test_macro(Cuda.WithC CudaWithC)
