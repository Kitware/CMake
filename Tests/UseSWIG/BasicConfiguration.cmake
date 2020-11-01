
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

# Path separator
if (WIN32)
  set (PS "$<SEMICOLON>")
else()
  set (PS ":")
endif()

unset(SWIG_LANG_TYPE)
unset(SWIG_LANG_INCLUDE_DIRECTORIES)
unset(SWIG_LANG_DEFINITIONS)
unset(SWIG_LANG_OPTIONS)
unset(SWIG_LANG_LIBRARIES)

if(${language} MATCHES csharp)
  set(SWIG_LANG_TYPE TYPE SHARED)
endif()
if(${language} MATCHES fortran)
  set(SWIG_LANG_TYPE TYPE SHARED)
endif()
if(${language} MATCHES python)
  find_package(Python REQUIRED COMPONENTS Interpreter Development)
  set(SWIG_LANG_INCLUDE_DIRECTORIES ${Python_INCLUDE_DIRS})
  set(SWIG_LANG_LIBRARIES ${Python_LIBRARIES})
endif()
if(${language} MATCHES perl)
  find_package(Perl REQUIRED)
  find_package(PerlLibs REQUIRED)
  set(SWIG_LANG_INCLUDE_DIRECTORIES ${PERL_INCLUDE_PATH})
  separate_arguments(c_flags UNIX_COMMAND "${PERL_EXTRA_C_FLAGS}")
  set(SWIG_LANG_OPTIONS ${c_flags})
  set(SWIG_LANG_LIBRARIES ${PERL_LIBRARY})
endif()
if(${language} MATCHES tcl)
  find_package(TCL REQUIRED)
  set(SWIG_LANG_INCLUDE_DIRECTORIES ${TCL_INCLUDE_PATH})
  set(SWIG_LANG_LIBRARIES ${TCL_LIBRARY})
endif()
if(${language} MATCHES ruby)
  find_package(Ruby REQUIRED)
  set(SWIG_LANG_INCLUDE_DIRECTORIES ${RUBY_INCLUDE_PATH})
  set(SWIG_LANG_LIBRARIES ${RUBY_LIBRARY})
endif()
if(${language} MATCHES php4)
  find_package(PHP4 REQUIRED)
  set(SWIG_LANG_INCLUDE_DIRECTORIES ${PHP4_INCLUDE_PATH})
  set(SWIG_LANG_LIBRARIES ${PHP4_LIBRARY})
endif()
if(${language} MATCHES pike)
  find_package(Pike REQUIRED)
  set(SWIG_LANG_INCLUDE_DIRECTORIES ${PIKE_INCLUDE_PATH})
  set(SWIG_LANG_LIBRARIES ${PIKE_LIBRARY})
endif()
if(${language} MATCHES lua)
  find_package(Lua REQUIRED)
  set(SWIG_LANG_INCLUDE_DIRECTORIES ${LUA_INCLUDE_DIR})
  set(SWIG_LANG_TYPE TYPE SHARED)
  set(SWIG_LANG_LIBRARIES ${LUA_LIBRARIES})
endif()

unset(CMAKE_SWIG_FLAGS)

set (CMAKE_INCLUDE_CURRENT_DIR ON)

set_property(SOURCE "${CMAKE_CURRENT_LIST_DIR}/example.i" PROPERTY CPLUSPLUS ON)
set_property(SOURCE "${CMAKE_CURRENT_LIST_DIR}/example.i" PROPERTY COMPILE_OPTIONS -includeall)

set_property(SOURCE "${CMAKE_CURRENT_LIST_DIR}/example.i"
  PROPERTY GENERATED_INCLUDE_DIRECTORIES ${SWIG_LANG_INCLUDE_DIRECTORIES}
                                         "${CMAKE_CURRENT_LIST_DIR}")
set_property(SOURCE "${CMAKE_CURRENT_LIST_DIR}/example.i"
  PROPERTY GENERATED_COMPILE_DEFINITIONS ${SWIG_LANG_DEFINITIONS})
set_property(SOURCE "${CMAKE_CURRENT_LIST_DIR}/example.i"
  PROPERTY GENERATED_COMPILE_OPTIONS ${SWIG_LANG_OPTIONS})


SWIG_ADD_LIBRARY(example
                 LANGUAGE "${language}"
                 ${SWIG_LANG_TYPE}
                 SOURCES "${CMAKE_CURRENT_LIST_DIR}/example.i"
                         "${CMAKE_CURRENT_LIST_DIR}/example.cxx")
TARGET_INCLUDE_DIRECTORIES(example PUBLIC ${CMAKE_CURRENT_LIST_DIR})
TARGET_LINK_LIBRARIES(example PRIVATE ${SWIG_LANG_LIBRARIES})
