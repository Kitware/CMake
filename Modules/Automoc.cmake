
#  AUTOMOC4_MOC_HEADERS(<target> header1.h header2.h ...)
#    Use this to add more header files to be processed with automoc4.
#
#  AUTOMOC4_ADD_EXECUTABLE(<target_NAME> src1 src2 ...)
#    This macro does the same as ADD_EXECUTABLE, but additionally
#    adds automoc4 handling for all source files.
#
# AUTOMOC4_ADD_LIBRARY(<target_NAME> src1 src2 ...)
#    This macro does the same as ADD_LIBRARY, but additionally
#    adds automoc4 handling for all source files.

# Internal helper macro, may change or be removed anytime:
# _ADD_AUTOMOC4_TARGET(<target_NAME> <SRCS_VAR>)
#
# Since version 0.9.88:
# The following two macros are only to be used for KDE4 projects
# and do something which makes sure automoc4 works for KDE. Don't
# use them anywhere else. See kdelibs/cmake/modules/KDE4Macros.cmake.
# _AUTOMOC4_KDE4_PRE_TARGET_HANDLING(<target_NAME> <SRCS_VAR>)
# _AUTOMOC4_KDE4_POST_TARGET_HANDLING(<target_NAME>)

#     Copyright (C) 2007 Matthias Kretz <kretz@kde.org>
#     Copyright (C) 2008-2009 Alexander Neundorf <neundorf@kde.org>
#
#     Redistribution and use in source and binary forms, with or without
#     modification, are permitted provided that the following conditions
#     are met:
#
#     1. Redistributions of source code must retain the above copyright
#        notice, this list of conditions and the following disclaimer.
#     2. Redistributions in binary form must reproduce the above copyright
#        notice, this list of conditions and the following disclaimer in the
#        documentation and/or other materials provided with the distribution.
#
#     THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
#     IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
#     OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
#     IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
#     INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
#     NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#     DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#     THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#     (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
#     THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


macro (AUTOMOC4_MOC_HEADERS _target_NAME)
   set (_headers_to_moc)
   foreach (_current_FILE ${ARGN})
      get_filename_component(_suffix "${_current_FILE}" EXT)
      if (".h" STREQUAL "${_suffix}" OR ".hpp" STREQUAL "${_suffix}" OR ".hxx" STREQUAL "${_suffix}" OR ".H" STREQUAL "${_suffix}")
         list(APPEND _headers_to_moc ${_current_FILE})
      else (".h" STREQUAL "${_suffix}" OR ".hpp" STREQUAL "${_suffix}" OR ".hxx" STREQUAL "${_suffix}" OR ".H" STREQUAL "${_suffix}")
         message(STATUS "AUTOMOC4_MOC_HEADERS: ignoring non-header file ${_current_FILE}")
      endif (".h" STREQUAL "${_suffix}" OR ".hpp" STREQUAL "${_suffix}" OR ".hxx" STREQUAL "${_suffix}" OR ".H" STREQUAL "${_suffix}")
   endforeach (_current_FILE)
   # need to create moc_<filename>.cpp file using automoc4
   # and add it to the target
   if(_headers_to_moc)
       set(_automoc4_headers_${_target_NAME} "${_headers_to_moc}")
   endif(_headers_to_moc)
endmacro (AUTOMOC4_MOC_HEADERS)


macro(_ADD_AUTOMOC4_TARGET _target_NAME _SRCS)
   set(_moc_files)
   set(_moc_headers)

   # first list all explicitly set headers
   foreach(_header_to_moc ${_automoc4_headers_${_target_NAME}} )
      get_filename_component(_abs_header ${_header_to_moc} ABSOLUTE)
      list(APPEND _moc_headers ${_abs_header})
   endforeach(_header_to_moc)

   # now add all the sources for the automoc
   foreach (_current_FILE ${${_SRCS}})
      get_filename_component(_abs_current_FILE "${_current_FILE}" ABSOLUTE)
      get_source_file_property(_skip      "${_abs_current_FILE}" SKIP_AUTOMOC)
      get_source_file_property(_generated "${_abs_current_FILE}" GENERATED)

      if(NOT  _generated  AND NOT  _skip)
         get_filename_component(_suffix "${_current_FILE}" EXT)
         # skip every source file that's not C++
         if(_suffix STREQUAL ".cpp" OR _suffix STREQUAL ".cc" OR _suffix STREQUAL ".cxx" OR _suffix STREQUAL ".C" OR _suffix STREQUAL ".mm")
             list(APPEND _moc_files ${_abs_current_FILE})
         endif(_suffix STREQUAL ".cpp" OR _suffix STREQUAL ".cc" OR _suffix STREQUAL ".cxx" OR _suffix STREQUAL ".C" OR _suffix STREQUAL ".mm")
      endif(NOT  _generated  AND NOT  _skip)
   endforeach (_current_FILE)

   if(_moc_files OR _moc_headers)
      set(_automoc_source "${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}.cpp")
      get_directory_property(_moc_incs INCLUDE_DIRECTORIES)
      get_directory_property(_moc_defs DEFINITIONS)
      get_directory_property(_moc_cdefs COMPILE_DEFINITIONS)

      # configure_file replaces _moc_files, _moc_incs, _moc_cdefs and _moc_defs
      set(_automocTargetDir "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${_target_NAME}.dir/" )
      set(AM_TARGET_NAME ${_target_NAME})
      configure_file(${CMAKE_ROOT}/Modules/AutomocInfo.cmake.in ${_automocTargetDir}/AutomocInfo.cmake @ONLY)

      add_custom_target(${_target_NAME}
         COMMAND ${CMAKE_COMMAND} -E cmake_automoc "${_automocTargetDir}" )

      set_source_files_properties(${_automoc_source} PROPERTIES GENERATED TRUE)
      get_directory_property(_extra_clean_files ADDITIONAL_MAKE_CLEAN_FILES)
      list(APPEND _extra_clean_files "${_automoc_source}")
      set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${_extra_clean_files}")
      set(${_SRCS} ${_automoc_source} ${${_SRCS}})
   endif(_moc_files OR _moc_headers)
endmacro(_ADD_AUTOMOC4_TARGET)


macro(AUTOMOC4_ADD_EXECUTABLE _target_NAME)
   set(_SRCS ${ARGN})

   set(_add_executable_param)
   foreach(_argName "WIN32" "MACOSX_BUNDLE" "EXCLUDE_FROM_ALL")
      list(FIND _SRCS ${_argName} _index)
      if(_index GREATER -1)
         list(APPEND _add_executable_param ${_argName})
         list(REMOVE_AT _SRCS ${_index})
      endif(_index GREATER -1)
   endforeach(_argName)

   _add_automoc4_target("${_target_NAME}_automoc" _SRCS)
   add_executable(${_target_NAME} ${_add_executable_param} ${_SRCS})
   add_dependencies(${_target_NAME} "${_target_NAME}_automoc")

endmacro(AUTOMOC4_ADD_EXECUTABLE)


macro(AUTOMOC4_ADD_LIBRARY _target_NAME)
   set(_SRCS ${ARGN})

   set(_add_executable_param)
   foreach(_argName "STATIC" "SHARED" "MODULE" "EXCLUDE_FROM_ALL")
      list(FIND _SRCS ${_argName} _index)
      if(_index GREATER -1)
         list(APPEND _add_executable_param ${_argName})
         list(REMOVE_AT _SRCS ${_index})
      endif(_index GREATER -1)
   endforeach(_argName)

   _add_automoc4_target("${_target_NAME}_automoc" _SRCS)
   add_library(${_target_NAME} ${_add_executable_param} ${_SRCS})
   add_dependencies(${_target_NAME} "${_target_NAME}_automoc")
endmacro(AUTOMOC4_ADD_LIBRARY)


macro(_AUTOMOC4_KDE4_PRE_TARGET_HANDLING _target _srcs)
   _add_automoc4_target("${_target}_automoc" ${_srcs})
endmacro(_AUTOMOC4_KDE4_PRE_TARGET_HANDLING)


macro(_AUTOMOC4_KDE4_POST_TARGET_HANDLING _target)
   add_dependencies(${_target} "${_target}_automoc")
endmacro(_AUTOMOC4_KDE4_POST_TARGET_HANDLING)
