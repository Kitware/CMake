enable_language(C)
enable_language(CXX)
enable_testing()

if(NOT DEFINED matlab_required)
  set(matlab_required REQUIRED)
endif()

if(NOT DEFINED Matlab_ROOT_DIR AND NOT "${matlab_root}" STREQUAL "")
  set(Matlab_ROOT_DIR ${matlab_root})
endif()

find_package(Matlab ${matlab_required} COMPONENTS MX_LIBRARY)
