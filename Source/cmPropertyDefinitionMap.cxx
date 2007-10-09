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


void cmPropertyDefinitionMap
::DefineProperty(const char *name, cmProperty::ScopeType scope,
                 const char *ShortDescription,
                 const char *FullDescription,
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
    prop->DefineProperty(name,scope,ShortDescription, FullDescription, chain);
    }
}

void cmPropertyDefinitionMap
::GetPropertiesDocumentation(std::vector<cmDocumentationEntry>& v) const
{
  for(cmPropertyDefinitionMap::const_iterator j = this->begin();
      j != this->end(); ++j)
    {
    cmDocumentationEntry e = j->second.GetDocumentation();
    if (e.brief.size())
      {
      v.push_back(e);
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
