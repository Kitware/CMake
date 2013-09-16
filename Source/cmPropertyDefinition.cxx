/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmPropertyDefinition.h"
#include "cmSystemTools.h"

cmDocumentationEntry cmPropertyDefinition::GetDocumentation() const
{
  cmDocumentationEntry e;
  e.Name = this->Name;
  e.Brief = this->ShortDescription;
  return e;
}

void cmPropertyDefinition
::DefineProperty(const char *name, cmProperty::ScopeType scope,
                 const char *shortDescription,
                 const char *fullDescription,
                 const char *sec,
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
  if (sec)
    {
    this->DocumentationSection = sec;
    }
}

