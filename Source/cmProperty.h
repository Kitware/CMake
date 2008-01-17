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
#ifndef cmProperty_h
#define cmProperty_h

#include "cmStandardIncludes.h"

class cmProperty 
{
public:
  enum ScopeType { TARGET, SOURCE_FILE, DIRECTORY, GLOBAL, 
                   TEST, VARIABLE, CACHED_VARIABLE };

  // set this property
  void Set(const char *name, const char *value);

  // append to this property
  void Append(const char *name, const char *value);

  // get the value
  const char *GetValue() const;

  // construct with the value not set
  cmProperty() { this->ValueHasBeenSet = false; };

protected:
  std::string Name;
  std::string Value;
  bool ValueHasBeenSet;
};

#endif
