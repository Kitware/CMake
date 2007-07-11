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
#ifndef cmPropertyDefinition_h
#define cmPropertyDefinition_h

#include "cmProperty.h"

class cmPropertyDefinition 
{
public:
  // Define this property
  void DefineProperty(const char *name, cmProperty::ScopeType scope,
                      const char *ShortDescription,
                      const char *FullDescription, 
                      bool chained);

  // get the documentation string
  cmDocumentationEntry GetDocumentation() const;

  // basic constructor 
  cmPropertyDefinition() { this->Chained = false; };

  // is it chained?
  bool IsChained() {return this->Chained; };

protected:
  std::string Name;
  std::string ShortDescription;
  std::string FullDescription;
  cmProperty::ScopeType Scope; 
  bool Chained;
};

#endif
