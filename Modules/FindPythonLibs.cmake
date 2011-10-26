# - Find python libraries
# This module finds if Python is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#
#  PYTHONLIBS_FOUND           - have the Python libs been found
#  PYTHON_LIBRARIES           - path to the python library
#  PYTHON_INCLUDE_PATH        - path to where Python.h is found (deprecated)
#  PYTHON_INCLUDE_DIRS        - path to where Python.h is found
#  PYTHON_DEBUG_LIBRARIES     - path to the debug library
#  Python_ADDITIONAL_VERSIONS - list of additional Python versions to search for

#=============================================================================
# Copyright 2001-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

include(CMakeFindFrameworks)
# Search for the python framework on Apple.
CMAKE_FIND_FRAMEWORKS(Python)

# Set up the versions we know about, in the order we will search. Always add
# the user supplied additional versions to the front.
set(_Python_VERSIONS
  ${Python_ADDITIONAL_VERSIONS}
  2.7 2.6 2.5 2.4 2.3 2.2 2.1 2.0 1.6 1.5)

foreach(_CURRENT_VERSION ${_Python_VERSIONS})
  string(REPLACE "." "" _CURRENT_VERSION_NO_DOTS ${_CURRENT_VERSION})
  if(WIN32)
    find_library(PYTHON_DEBUG_LIBRARY
      NAMES python${_CURRENT_VERSION_NO_DOTS}_d python
      PATHS
      [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\${_CURRENT_VERSION}\\InstallPath]/libs/Debug
      [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\${_CURRENT_VERSION}\\InstallPath]/libs )
  endif(WIN32)

  find_library(PYTHON_LIBRARY
    NAMES python${_CURRENT_VERSION_NO_DOTS} python${_CURRENT_VERSION}
    PATHS
      [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\${_CURRENT_VERSION}\\InstallPath]/libs
    # Avoid finding the .dll in the PATH.  We want the .lib.
    NO_SYSTEM_ENVIRONMENT_PATH
  )
  # Look for the static library in the Python config directory
  find_library(PYTHON_LIBRARY
    NAMES python${_CURRENT_VERSION_NO_DOTS} python${_CURRENT_VERSION}
    # Avoid finding the .dll in the PATH.  We want the .lib.
    NO_SYSTEM_ENVIRONMENT_PATH
    # This is where the static library is usually located
    PATH_SUFFIXES python${_CURRENT_VERSION}/config
  )

  # For backward compatibility, honour value of PYTHON_INCLUDE_PATH, if
  # PYTHON_INCLUDE_DIR is not set.
  if(DEFINED PYTHON_INCLUDE_PATH AND NOT DEFINED PYTHON_INCLUDE_DIR)
    set(PYTHON_INCLUDE_DIR "${PYTHON_INCLUDE_PATH}" CACHE PATH
      "Path to where Python.h is found" FORCE)
  endif(DEFINED PYTHON_INCLUDE_PATH AND NOT DEFINED PYTHON_INCLUDE_DIR)

  set(PYTHON_FRAMEWORK_INCLUDES)
  if(Python_FRAMEWORKS AND NOT PYTHON_INCLUDE_DIR)
    foreach(dir ${Python_FRAMEWORKS})
      set(PYTHON_FRAMEWORK_INCLUDES ${PYTHON_FRAMEWORK_INCLUDES}
        ${dir}/Versions/${_CURRENT_VERSION}/include/python${_CURRENT_VERSION})
    endforeach(dir)
  endif(Python_FRAMEWORKS AND NOT PYTHON_INCLUDE_DIR)

  find_path(PYTHON_INCLUDE_DIR
    NAMES Python.h
    PATHS
      ${PYTHON_FRAMEWORK_INCLUDES}
      [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\${_CURRENT_VERSION}\\InstallPath]/include
    PATH_SUFFIXES
      python${_CURRENT_VERSION}
  )

  # For backward compatibility, set PYTHON_INCLUDE_PATH, but make it internal.
  set(PYTHON_INCLUDE_PATH "${PYTHON_INCLUDE_DIR}" CACHE INTERNAL
    "Path to where Python.h is found (deprecated)")

endforeach(_CURRENT_VERSION)

mark_as_advanced(
  PYTHON_DEBUG_LIBRARY
  PYTHON_LIBRARY
  PYTHON_INCLUDE_DIR
)

# We use PYTHON_INCLUDE_DIR, PYTHON_LIBRARY and PYTHON_DEBUG_LIBRARY for the
# cache entries because they are meant to specify the location of a single
# library. We now set the variables listed by the documentation for this
# module.
set(PYTHON_INCLUDE_DIRS "${PYTHON_INCLUDE_DIR}")
set(PYTHON_LIBRARIES "${PYTHON_LIBRARY}")
set(PYTHON_DEBUG_LIBRARIES "${PYTHON_DEBUG_LIBRARY}")


include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PythonLibs DEFAULT_MSG PYTHON_LIBRARIES PYTHON_INCLUDE_DIRS)


# PYTHON_ADD_MODULE(<name> src1 src2 ... srcN) is used to build modules for python.
# PYTHON_WRITE_MODULES_HEADER(<filename>) writes a header file you can include
# in your sources to initialize the static python modules
function(PYTHON_ADD_MODULE _NAME )
  get_property(_TARGET_SUPPORTS_SHARED_LIBS
    GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS)
  option(PYTHON_ENABLE_MODULE_${_NAME} "Add module ${_NAME}" TRUE)
  option(PYTHON_MODULE_${_NAME}_BUILD_SHARED
    "Add module ${_NAME} shared" ${_TARGET_SUPPORTS_SHARED_LIBS})

  # Mark these options as advanced
  mark_as_advanced(PYTHON_ENABLE_MODULE_${_NAME}
    PYTHON_MODULE_${_NAME}_BUILD_SHARED)

  if(PYTHON_ENABLE_MODULE_${_NAME})
    if(PYTHON_MODULE_${_NAME}_BUILD_SHARED)
      set(PY_MODULE_TYPE MODULE)
    else(PYTHON_MODULE_${_NAME}_BUILD_SHARED)
      set(PY_MODULE_TYPE STATIC)
      set_property(GLOBAL  APPEND  PROPERTY  PY_STATIC_MODULES_LIST ${_NAME})
    endif(PYTHON_MODULE_${_NAME}_BUILD_SHARED)

    set_property(GLOBAL  APPEND  PROPERTY  PY_MODULES_LIST ${_NAME})
    add_library(${_NAME} ${PY_MODULE_TYPE} ${ARGN})
#    target_link_libraries(${_NAME} ${PYTHON_LIBRARIES})

    if(PYTHON_MODULE_${_NAME}_BUILD_SHARED)
      set_target_properties(${_NAME} PROPERTIES PREFIX "${PYTHON_MODULE_PREFIX}")
      if(WIN32 AND NOT CYGWIN)
        set_target_properties(${_NAME} PROPERTIES SUFFIX ".pyd")
      endif(WIN32 AND NOT CYGWIN)
    endif(PYTHON_MODULE_${_NAME}_BUILD_SHARED)

  endif(PYTHON_ENABLE_MODULE_${_NAME})
endfunction(PYTHON_ADD_MODULE)

function(PYTHON_WRITE_MODULES_HEADER _filename)

  get_property(PY_STATIC_MODULES_LIST  GLOBAL  PROPERTY PY_STATIC_MODULES_LIST)

  get_filename_component(_name "${_filename}" NAME)
  string(REPLACE "." "_" _name "${_name}")
  string(TOUPPER ${_name} _nameUpper)
  set(_filename ${CMAKE_CURRENT_BINARY_DIR}/${_filename})

  set(_filenameTmp "${_filename}.in")
  file(WRITE ${_filenameTmp} "/*Created by cmake, do not edit, changes will be lost*/\n")
  file(APPEND ${_filenameTmp}
"#ifndef ${_nameUpper}
#define ${_nameUpper}

#include <Python.h>

#ifdef __cplusplus
extern \"C\" {
#endif /* __cplusplus */

")

  foreach(_currentModule ${PY_STATIC_MODULES_LIST})
    file(APPEND ${_filenameTmp} "extern void init${PYTHON_MODULE_PREFIX}${_currentModule}(void);\n\n")
  endforeach(_currentModule ${PY_STATIC_MODULES_LIST})

  file(APPEND ${_filenameTmp}
"#ifdef __cplusplus
}
#endif /* __cplusplus */

")


  foreach(_currentModule ${PY_STATIC_MODULES_LIST})
    file(APPEND ${_filenameTmp} "int ${_name}_${_currentModule}(void) \n{\n  static char name[]=\"${PYTHON_MODULE_PREFIX}${_currentModule}\"; return PyImport_AppendInittab(name, init${PYTHON_MODULE_PREFIX}${_currentModule});\n}\n\n")
  endforeach(_currentModule ${PY_STATIC_MODULES_LIST})

  file(APPEND ${_filenameTmp} "void ${_name}_LoadAllPythonModules(void)\n{\n")
  foreach(_currentModule ${PY_STATIC_MODULES_LIST})
    file(APPEND ${_filenameTmp} "  ${_name}_${_currentModule}();\n")
  endforeach(_currentModule ${PY_STATIC_MODULES_LIST})
  file(APPEND ${_filenameTmp} "}\n\n")
  file(APPEND ${_filenameTmp} "#ifndef EXCLUDE_LOAD_ALL_FUNCTION\nvoid CMakeLoadAllPythonModules(void)\n{\n  ${_name}_LoadAllPythonModules();\n}\n#endif\n\n#endif\n")

# with configure_file() cmake complains that you may not use a file created using file(WRITE) as input file for configure_file()
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_filenameTmp}" "${_filename}" OUTPUT_QUIET ERROR_QUIET)

endfunction(PYTHON_WRITE_MODULES_HEADER)
