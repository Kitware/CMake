
ADD_TEST_MACRO(Cuda.Complex CudaComplex)
ADD_TEST_MACRO(Cuda.ConsumeCompileFeatures CudaConsumeCompileFeatures)
ADD_TEST_MACRO(Cuda.CXXStandardSetTwice CXXStandardSetTwice)
ADD_TEST_MACRO(Cuda.ObjectLibrary CudaObjectLibrary)
ADD_TEST_MACRO(Cuda.MixedStandardLevels1 MixedStandardLevels1)
ADD_TEST_MACRO(Cuda.MixedStandardLevels2 MixedStandardLevels2)
ADD_TEST_MACRO(Cuda.MixedStandardLevels3 MixedStandardLevels3)
ADD_TEST_MACRO(Cuda.MixedStandardLevels4 MixedStandardLevels4)
ADD_TEST_MACRO(Cuda.MixedStandardLevels5 MixedStandardLevels5)
ADD_TEST_MACRO(Cuda.NotEnabled CudaNotEnabled)
ADD_TEST_MACRO(Cuda.SeparableCompCXXOnly SeparableCompCXXOnly)
ADD_TEST_MACRO(Cuda.Toolkit Toolkit)
ADD_TEST_MACRO(Cuda.IncludePathNoToolkit IncludePathNoToolkit)
ADD_TEST_MACRO(Cuda.ProperDeviceLibraries ProperDeviceLibraries)
ADD_TEST_MACRO(Cuda.ProperLinkFlags ProperLinkFlags)
ADD_TEST_MACRO(Cuda.SharedRuntimePlusToolkit SharedRuntimePlusToolkit)

# The CUDA only ships the shared version of the toolkit libraries
# on windows
if(NOT WIN32)
  ADD_TEST_MACRO(Cuda.StaticRuntimePlusToolkit StaticRuntimePlusToolkit)
endif()

ADD_TEST_MACRO(Cuda.WithC CudaWithC)
