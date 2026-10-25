add_executable(main main.cpp)
target_compile_definitions(main PRIVATE EXPECT_${CMP0231_EXPECT})
# MSVC does not support modules in non-permissive mode.
set_property(TARGET main PROPERTY CXX_SCAN_FOR_MODULES OFF)
if(CMP0231_STANDARD)
  set_property(TARGET main PROPERTY CXX_STANDARD ${CMP0231_STANDARD})
endif()
set_property(TARGET main PROPERTY CXX_EXTENSIONS ${CMP0231_EXTENSIONS})
