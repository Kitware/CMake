include(Platform/Windows-Clang)
__windows_compiler_clang(CUDA)

# The MSVC platform initialization enables host IPO for Clang, but CUDA's
# compiler module does not support CUDA device IPO.
set(_CMAKE_CUDA_IPO_SUPPORTED_BY_CMAKE NO)
set(_CMAKE_CUDA_IPO_MAY_BE_SUPPORTED_BY_COMPILER NO)

if(CMAKE_CUDA_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
  # The MSVC object rule has no CUDA_COMPILE_MODE placeholder, so it would
  # omit -fgpu-rdc for separable compilation. Use -o for the final host object
  # rather than applying /Fo to device intermediates as well.
  set(CMAKE_CUDA_COMPILE_OBJECT
    "<CMAKE_CUDA_COMPILER> ${CMAKE_START_TEMP_FILE} ${CMAKE_CL_NOLOGO} ${_CMAKE_CUDA_EXTRA_FLAGS} <DEFINES> <INCLUDES> <FLAGS> ${_CMAKE_COMPILE_AS_CUDA_FLAG} <CUDA_COMPILE_MODE> -o <OBJECT> /Fd<TARGET_COMPILE_PDB> -- <SOURCE>${CMAKE_END_TEMP_FILE}")
  if(NOT CMAKE_CUDA_COMPILER_HAS_DEVICE_LINK_PHASE)
    # link.exe's incremental padding breaks the contiguous offload-entry
    # table traversed by clang-linker-wrapper's registration constructor.
    __windows_compiler_clang_msvc_offload_link(CUDA "${_CMAKE_CUDA_RDC_LINK_FLAG}" "/INCREMENTAL:NO")
  endif()
  set(_CUDA_LIBRARY_PATH_FLAG "-libpath:")
  set(CMAKE_CUDA_VERBOSE_FLAG "-clang:-v")
else()
  set(_CUDA_LIBRARY_PATH_FLAG "-L")
endif()

# Tell Clang where to find the CUDA libraries.
set(__IMPLICIT_LINKS)
foreach(dir ${CMAKE_CUDA_HOST_IMPLICIT_LINK_DIRECTORIES})
  string(APPEND __IMPLICIT_LINKS " ${_CUDA_LIBRARY_PATH_FLAG}\"${dir}\"")
endforeach()
string(APPEND CMAKE_CUDA_LINK_EXECUTABLE "${__IMPLICIT_LINKS}")
string(APPEND CMAKE_CUDA_CREATE_SHARED_LIBRARY "${__IMPLICIT_LINKS}")
string(APPEND CMAKE_CUDA_CREATE_SHARED_MODULE "${__IMPLICIT_LINKS}")
unset(__IMPLICIT_LINKS)
unset(_CUDA_LIBRARY_PATH_FLAG)

# Device linking is just regular linking so these are the same.
set(CMAKE_CUDA_DEVICE_LINKER_WRAPPER_FLAG ${CMAKE_CUDA_LINKER_WRAPPER_FLAG})
set(CMAKE_CUDA_DEVICE_LINKER_WRAPPER_FLAG_SEP ${CMAKE_CUDA_LINKER_WRAPPER_FLAG_SEP})
