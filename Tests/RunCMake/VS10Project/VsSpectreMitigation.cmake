set(CMAKE_CONFIGURATION_TYPES Debug)
enable_language(C)

add_library(SpectreMitigationOn-C empty.c)
target_compile_options(SpectreMitigationOn-C PRIVATE -Qspectre)

add_library(SpectreMitigationOff-C empty.c)
target_compile_options(SpectreMitigationOff-C PRIVATE -Qspectre-)
