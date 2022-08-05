include_guard()

set(CMAKE_EXECUTABLE_SUFFIX ".dxe")

macro(__compiler_adsp lang)
  set(CMAKE_${lang}_OUTPUT_EXTENSION ".doj")

  set(CMAKE_${lang}_LINKER_WRAPPER_FLAG "-flags-link" " ")
  set(CMAKE_${lang}_LINKER_WRAPPER_FLAG_SEP ",")

  set(_CMAKE_${lang}_ADSP_FLAGS "-proc=${CMAKE_ADSP_PROCESSOR}")

  set(CMAKE_${lang}_COMPILE_OBJECT
    "<CMAKE_${lang}_COMPILER> ${_CMAKE_${lang}_ADSP_FLAGS} <DEFINES> <INCLUDES> <FLAGS> -o <OBJECT> -c <SOURCE>")

  set(CMAKE_${lang}_CREATE_STATIC_LIBRARY
    "<CMAKE_${lang}_COMPILER> ${_CMAKE_${lang}_ADSP_FLAGS} -build-lib -o <TARGET> <CMAKE_${lang}_LINK_FLAGS> <OBJECTS>")

  set(CMAKE_${lang}_LINK_EXECUTABLE
    "<CMAKE_${lang}_COMPILER> ${_CMAKE_${lang}_ADSP_FLAGS} <FLAGS> <CMAKE_${lang}_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

  unset(_CMAKE_${lang}_ADSP_FLAGS)

  set(CMAKE_${lang}_CREATE_SHARED_LIBRARY)
  set(CMAKE_${lang}_CREATE_MODULE_LIBRARY)
endmacro()
