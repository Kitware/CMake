add_executable(main main.cxx)
target_link_libraries(main PRIVATE cuda_lib)

if(APPLE)
  set_property(TARGET main PROPERTY BUILD_RPATH ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
endif()
