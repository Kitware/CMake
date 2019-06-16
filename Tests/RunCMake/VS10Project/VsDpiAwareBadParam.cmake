set(CMAKE_CONFIGURATION_TYPES Debug)
enable_language(C)
enable_language(CXX)

add_executable(DPIAWARE-TGT-BADPARAM-C empty.c)
set_property(TARGET DPIAWARE-TGT-BADPARAM-C PROPERTY VS_DPI_AWARE "Foo")
add_executable(DPIAWARE-TGT-BADPARAM-CXX empty.cxx)
set_property(TARGET DPIAWARE-TGT-BADPARAM-CXX PROPERTY VS_DPI_AWARE "Bar")
