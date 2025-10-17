enable_language(CXX)
set(CMAKE_CXX_PVS_STUDIO "$<1:${PSEUDO_PVS}>")
add_executable(main main.cxx)
