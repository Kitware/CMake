cmake_minimum_required(VERSION 4.5)
if(NOT CMP0234_LANGS)
  set(CMP0234_LANGS C CXX)
endif()
project(${RunCMake_TEST} ${CMP0234_LANGS})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Sources check that _MSVC_TRADITIONAL matches the expectation for their
# language: STRICT is the conformant preprocessor, EXTENSIONS the traditional.
set(sources)
if("C" IN_LIST CMP0234_LANGS)
  list(APPEND sources main42.c)
endif()
if("CXX" IN_LIST CMP0234_LANGS)
  list(APPEND sources other.cpp)
endif()
add_library(main42 STATIC ${sources})
# The Visual Studio generators compile the C and C++ sources with one set of
# target options, so the expectation of each language is set per source.
if("C" IN_LIST CMP0234_LANGS)
  set_property(SOURCE main42.c PROPERTY COMPILE_DEFINITIONS
    EXPECT_${CMP0234_EXPECT_C})
endif()
if("CXX" IN_LIST CMP0234_LANGS)
  set_property(SOURCE other.cpp PROPERTY COMPILE_DEFINITIONS
    EXPECT_${CMP0234_EXPECT_CXX})
endif()
if(CMP0234_C_STANDARD)
  set_property(TARGET main42 PROPERTY C_STANDARD ${CMP0234_C_STANDARD})
endif()
set_property(TARGET main42 PROPERTY C_EXTENSIONS ${CMP0234_C_EXTENSIONS})
if(CMP0234_CXX_STANDARD)
  set_property(TARGET main42 PROPERTY CXX_STANDARD ${CMP0234_CXX_STANDARD})
endif()
set_property(TARGET main42 PROPERTY CXX_EXTENSIONS ${CMP0234_CXX_EXTENSIONS})

# The policy is recorded when a target is created.
if(CMP0234_LATE_POLICY)
  cmake_policy(SET CMP0234 ${CMP0234_LATE_POLICY})
  # Use copies of the sources, which have their own definitions.
  set(late_sources)
  foreach(source IN LISTS sources)
    configure_file(${source} late_${source} COPYONLY)
    list(APPEND late_sources ${CMAKE_CURRENT_BINARY_DIR}/late_${source})
  endforeach()
  add_library(late STATIC ${late_sources})
  target_compile_definitions(late PRIVATE EXPECT_${CMP0234_LATE_EXPECT})
  if(CMP0234_C_STANDARD)
    set_property(TARGET late PROPERTY C_STANDARD ${CMP0234_C_STANDARD})
  endif()
  set_property(TARGET late PROPERTY C_EXTENSIONS ${CMP0234_C_EXTENSIONS})
  set_property(TARGET late PROPERTY CXX_EXTENSIONS ${CMP0234_CXX_EXTENSIONS})
  cmake_policy(SET CMP0234 ${CMP0234_POLICY})
endif()

# The policy setting is propagated to try_compile projects.  They have a
# single language, so only that language's extensions setting matters.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
if("C" IN_LIST CMP0234_LANGS)
  set(CMAKE_C_EXTENSIONS ${CMP0234_C_EXTENSIONS})
  if(CMP0234_C_STANDARD)
    set(CMAKE_C_STANDARD ${CMP0234_C_STANDARD})
    # The C11 standard option implies the conformant preprocessor, and
    # the traditional one needs /Zc:preprocessor- with extensions on.
    if(CMP0234_POLICY STREQUAL "NEW" AND CMP0234_C_EXTENSIONS)
      set(c_expect EXTENSIONS)
    else()
      set(c_expect STRICT)
    endif()
  elseif(CMP0234_POLICY STREQUAL "NEW" AND NOT CMP0234_C_EXTENSIONS)
    set(c_expect STRICT)
  else()
    set(c_expect EXTENSIONS)
  endif()
  try_compile(c_ok
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main42.c
    COMPILE_DEFINITIONS -DEXPECT_${c_expect}
    NO_CACHE)
  if(NOT c_ok)
    message(SEND_ERROR "C try_compile did not build in ${c_expect} mode")
  endif()
endif()
if("CXX" IN_LIST CMP0234_LANGS)
  set(CMAKE_CXX_EXTENSIONS ${CMP0234_CXX_EXTENSIONS})
  if(CMP0234_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD ${CMP0234_CXX_STANDARD})
  endif()
  if(CMP0234_POLICY STREQUAL "NEW" AND NOT CMP0234_CXX_EXTENSIONS)
    set(cxx_expect STRICT)
  else()
    set(cxx_expect EXTENSIONS)
  endif()
  try_compile(cxx_ok
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/other.cpp
    COMPILE_DEFINITIONS -DEXPECT_${cxx_expect}
    NO_CACHE)
  if(NOT cxx_ok)
    message(SEND_ERROR "CXX try_compile did not build in ${cxx_expect} mode")
  endif()
endif()
