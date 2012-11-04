/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2011 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmDocumentLocationUndefined_h
#define cmDocumentLocationUndefined_h

#define CM_LOCATION_UNDEFINED_BEHAVIOR(action) \
  "\n" \
  "Do not set properties that affect the location of a target after " \
  action ".  These include properties whose names match " \
  "\"(RUNTIME|LIBRARY|ARCHIVE)_OUTPUT_(NAME|DIRECTORY)(_<CONFIG>)?\", " \
  "\"(IMPLIB_)?(PREFIX|SUFFIX)\", or \"LINKER_LANGUAGE\".  " \
  "Failure to follow this rule is not diagnosed and leaves the location " \
  "of the target undefined."

#endif
