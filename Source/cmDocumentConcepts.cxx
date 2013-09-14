/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2013 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmDocumentConcepts.h"

void cmDocumentConcepts::GetDocumentation(std::vector<cmDocumentationEntry>&v)
{
  v.push_back(cmDocumentationEntry("policies",
    "Policies and minimum version settings",
    "Policies in CMake are used to preserve backward compatible behavior "
    "across multiple releases.  When a new policy is introduced, newer CMake "
    "versions will begin to warn about the backward compatible behavior.  It "
    "is possible to disable the warning by explicitly requesting the OLD, or"
    "backward compatible behavior using the cmake_policy command.  It is "
    "also possible to request NEW, or non-backward compatible behavior for a "
    "policy."
    "\n"
    "The cmake_minimum_required() command does more than report an error if "
    "a too-old version of CMake is used to build a project.  It also sets "
    "all policies introduced in that CMake version or earlier to NEW "
    "behavior."
    "\n"
    "The CMAKE_MINIMUM_REQUIRED_VERSION variable may also be used to "
    "determine whether to report an error on use of deprecated macros or "
    "functions."
  ));
}
