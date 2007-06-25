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
#include "cmPropertyDefinition.h"
#include "cmSystemTools.h"

cmDocumentationEntry cmPropertyDefinition::GetDocumentation() const
{
  cmDocumentationEntry e;
  e.name = this->LongName.c_str();
  e.brief = 
    this->ShortDescription.size() ? this->ShortDescription.c_str() : 0;
  e.full = this->FullDescription.size() ? this->FullDescription.c_str() : 0;
  return e;
}

void cmPropertyDefinition
::DefineProperty(const char *name, cmProperty::ScopeType scope,
                 const char *shortDescription,
                 const char *fullDescription,
                 bool chain)
{
  this->Name = name;
  this->Scope = scope;
  this->Chained = chain;
  if (shortDescription)
    {
    this->ShortDescription = shortDescription;
    }
  if (fullDescription)
    {
    this->FullDescription = fullDescription;
    }
  this->LongName = this->Name;
  switch (this->Scope)
    {
    case cmProperty::TARGET: this->LongName += " on a target";
      break;
    case cmProperty::SOURCE_FILE: this->LongName += " on a source file";
      break;
    case cmProperty::DIRECTORY: this->LongName += " on a directory";
      break;
    case cmProperty::GLOBAL: this->LongName += " globaly";
      break;
    case cmProperty::TEST: this->LongName += " on CTest";
      break;
    case cmProperty::VARIABLE: this->LongName += " as a variable";
      break;
    case cmProperty::CACHED_VARIABLE: this->LongName += 
                                        " as a cached variable";
      break;
    }
}

