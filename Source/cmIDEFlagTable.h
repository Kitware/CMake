/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmIDEFlagTable_h
#define cmIDEFlagTable_h

// This is a table mapping XML tag IDE names to command line options
struct cmIDEFlagTable
{
  const char* IDEName;  // name used in the IDE xml file
  const char* commandFlag; // command line flag
  const char* comment;     // comment
  const char* value; // string value
  unsigned int special; // flags for special handling requests
  enum
  {
    UserValue    = (1<<0), // flag contains a user-specified value
    UserIgnored  = (1<<1), // ignore any user value
    UserRequired = (1<<2), // match only when user value is non-empty
    Continue     = (1<<3), // continue looking for matching entries
    SemicolonAppendable = (1<<4), // a flag that if specified multiple times
                                  // should have its value appended to the
                                  // old value with semicolons (e.g.
                                  // /NODEFAULTLIB: =>
                                  // IgnoreDefaultLibraryNames)

    UserValueIgnored  = UserValue | UserIgnored,
    UserValueRequired = UserValue | UserRequired
  };
};

#endif
