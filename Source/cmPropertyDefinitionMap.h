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
#ifndef cmPropertyDefinitionMap_h
#define cmPropertyDefinitionMap_h

#include "cmPropertyDefinition.h"

class cmPropertyDefinitionMap : 
public std::map<cmStdString,cmPropertyDefinition>
{
public:
  // define the property
  void DefineProperty(const char *name, cmProperty::ScopeType scope,
                      const char *ShortDescription,
                      const char *FullDescription,
                      bool chain);

  // has a named property been defined
  bool IsPropertyDefined(const char *name);

  // is a named property set to chain
  bool IsPropertyChained(const char *name);

  void GetPropertiesDocumentation(std::vector<cmDocumentationEntry>& v) const;
};

#endif

