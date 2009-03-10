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
#include "cmPropertyDefinitionMap.h"
#include "cmSystemTools.h"
#include "cmDocumentationSection.h"

void cmPropertyDefinitionMap
::DefineProperty(const char *name, cmProperty::ScopeType scope,
                 const char *ShortDescription,
                 const char *FullDescription,
                 const char *DocumentationSection,
                 bool chain)
{
  if (!name)
    {
    return;
    }

  cmPropertyDefinitionMap::iterator it = this->find(name);
  cmPropertyDefinition *prop;
  if (it == this->end())
    {
    prop = &(*this)[name];
    prop->DefineProperty(name,scope,ShortDescription, FullDescription, 
                         DocumentationSection, chain);
    }
}

void cmPropertyDefinitionMap
::GetPropertiesDocumentation(std::map<std::string,
                             cmDocumentationSection *>& v) const
{
  for(cmPropertyDefinitionMap::const_iterator j = this->begin();
      j != this->end(); ++j)
    {
    // add a section if needed
    std::string secName = j->second.GetDocumentationSection();
    // if a section was not specified then use the scope
    if (!secName.size())
      {
      switch (j->second.GetScope())
        {
        case cmProperty::GLOBAL: 
          secName = "Properties of Global Scope";
          break;
        case cmProperty::TARGET: 
          secName = "Properties on Targets";
          break;
        case cmProperty::SOURCE_FILE:
          secName = "Properties on Source Files";
          break;
        case cmProperty::DIRECTORY:
          secName = "Properties on Directories";
          break;
        case cmProperty::TEST:
          secName = "Properties on Tests";
          break;
        case cmProperty::CACHE:
          secName = "Properties on Cache Entries";
          break;
        case cmProperty::VARIABLE:
          secName = "Variables";
          break;
        case cmProperty::CACHED_VARIABLE:
          secName = "Cached Variables";
          break;
        default:
          secName = "Properties of Unknown Scope";
          break;
        }
      }
    if (!v[secName])
      {
      v[secName] = new 
        cmDocumentationSection(secName.c_str(),
                               cmSystemTools::UpperCase(secName).c_str());
      }
    cmDocumentationEntry e = j->second.GetDocumentation();
    if (e.Brief.size() || e.Full.size())
      {
      v[secName]->Append(e);
      }
    }
}

bool cmPropertyDefinitionMap::IsPropertyDefined(const char *name)
{
  if (!name)
    {
    return false;
    }

  cmPropertyDefinitionMap::iterator it = this->find(name);
  if (it == this->end())
    {
    return false;
    }

  return true;
}

bool cmPropertyDefinitionMap::IsPropertyChained(const char *name)
{
  if (!name)
    {
    return false;
    }

  cmPropertyDefinitionMap::iterator it = this->find(name);
  if (it == this->end())
    {
    return false;
    }

  return it->second.IsChained();
}
