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
  e.Name = this->Name;
  e.Brief = this->ShortDescription;
  e.Full = this->FullDescription;
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

