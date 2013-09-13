/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmPropertyDefinitionMap_h
#define cmPropertyDefinitionMap_h

#include "cmPropertyDefinition.h"

class cmDocumentationSection;

class cmPropertyDefinitionMap :
public std::map<cmStdString,cmPropertyDefinition>
{
public:
  // define the property
  void DefineProperty(const char *name, cmProperty::ScopeType scope,
                      const char *ShortDescription,
                      const char *FullDescription,
                      const char *DocumentaitonSection,
                      bool chain);

  // has a named property been defined
  bool IsPropertyDefined(const char *name);

  // is a named property set to chain
  bool IsPropertyChained(const char *name);
};

#endif

