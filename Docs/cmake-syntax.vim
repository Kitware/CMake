" =============================================================================
" 
"   Program:   CMake - Cross-Platform Makefile Generator
"   Module:    $RCSfile$
"   Language:  C++
"   Date:      $Date$
"   Version:   $Revision$
" 
"   Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
"   See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.
" 
"      This software is distributed WITHOUT ANY WARRANTY"  without even 
"      the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
"      PURPOSE.  See the above copyright notices for more information.
" 
" =============================================================================

" Vim syntax file
" Language:     CMake
" Maintainer:   Andy Cedilnik <andy.cedilnik@kitware.com>
" Last Change:  Fri Mar 21 22:33:55 EST 2003

" For version 5.x: Clear all syntax items
" For version 6.x: Quit when a syntax file was already loaded
if version < 600
  syntax clear
elseif exists("b:current_syntax")
  finish
endif



syn case match
syn match cmakeComment /#.*$/
syn region cmakeRegistry start=/\[/ end=/\]/ skip=/\\[\[\]]/
            \ contained
syn match cmakeArgument /[^()"]+/
            \ contained
syn match cmakeVariableValue /\${[^}]*}/
            \ contained oneline
syn match cmakeEnvironment /\$ENV{.*}/
            \ contained 
syn keyword cmakeSystemVariables WIN32 UNIX APPLE
syn keyword cmakeOperators MATCHES AND OR EXISTS CACHE STRING BOOL PROGRAM NAMES INTERNAL PATHS DOC PATH NOT NAME NAME_WE FALSE TRUE ON OFF
"            \ contained
syn region cmakeString start=/"/ end=/"/ skip=/\\"/
            \ contains=cmakeVariableValue,cmakeRegistry keepend oneline
syn region cmakeArguments start=/\s*(/ end=/)/
           \ contains=cmakeEnvironment,cmakeRegistry,cmakeVariableValue,cmakeString,cmakeArgument,cmakeOperators keepend
syn keyword cmakeDeprecated ABSTRACT_FILES BUILD_NAME SOURCE_FILES SOURCE_FILES_REMOVE VTK_MAKE_INSTANTIATOR VTK_WRAP_JAVA VTK_WRAP_PYTHON VTK_WRAP_TCL WRAP_EXCLUDE_FILES
           \ nextgroup=cmakeArgument
syn keyword cmakeStatement 
           \ ADD_CUSTOM_COMMAND ADD_CUSTOM_TARGET ADD_DEFINITIONS ADD_DEPENDENCIES ADD_EXECUTABLE ADD_LIBRARY ADD_TEST AUX_SOURCE_DIRECTORY BUILD_COMMAND CMAKE_MINIMUM_REQUIRED CONFIGURE_FILE CREATE_TEST_SOURCELIST ENABLE_TESTING FOREACH ENDFOREACH IF ELSE ENDIF EXPORT_LIBRARY_DEPENDENCIES FIND_FILE FIND_LIBRARY FIND_PACKAGE FIND_PATH FIND_PROGRAM FLTK_WRAP_UI GET_CMAKE_PROPERTY GET_FILENAME_COMPONENT GET_SOURCE_FILE_PROPERTY GET_TARGET_PROPERTY INCLUDE INCLUDE_DIRECTORIES INCLUDE_EXTERNAL_MSPROJECT INCLUDE_REGULAR_EXPRESSION INSTALL_FILES INSTALL_PROGRAMS INSTALL_TARGETS ITK_WRAP_TCL LINK_DIRECTORIES LINK_LIBRARIES LOAD_CACHE LOAD_COMMAND MACRO ENDMACRO MAKE_DIRECTORY MARK_AS_ADVANCED MESSAGE OPTION OUTPUT_REQUIRED_FILES PROJECT QT_WRAP_CPP QT_WRAP_UI SEPARATE_ARGUMENTS REMOVE SET SET_SOURCE_FILES_PROPERTIES SET_TARGET_PROPERTIES SITE_NAME SOURCE_GROUP STRING SUBDIRS SUBDIR_DEPENDS TARGET_LINK_LIBRARIES TRY_COMPILE TRY_RUN USE_MANGLED_MESA UTILITY_SOURCE VARIABLE_REQUIRES WRITE_FILE
           \ nextgroup=cmakeArgumnts

syn match cmakeMacro /[A-Z_]+/ nextgroup=cmakeArgumnts

" Define the default highlighting.
" For version 5.7 and earlier: only when not done already
" For version 5.8 and later: only when an item doesn't have highlighting yet
if version >= 508 || !exists("did_cmake_syntax_inits")
  if version < 508
    let did_cmake_syntax_inits = 1
    command -nargs=+ HiLink hi link <args>
  else
    command -nargs=+ HiLink hi def link <args>
  endif


  HiLink cmakeStatement Statement
  HiLink cmakeComment Comment
  HiLink cmakeString String
  HiLink cmakeVariableValue Type
  HiLink cmakeRegistry Underlined
  HiLink cmakeArguments Identifier
  HiLink cmakeArgument Constant
  HiLink cmakeEnvironment Special
  HiLink cmakeOperators Operator
  HiLink cmakeMacro PreProc

  delcommand HiLink
endif

let b:current_syntax = "cmake"

"EOF"
