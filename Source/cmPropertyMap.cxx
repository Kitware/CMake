/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmPropertyMap.h"
#include "cmSystemTools.h"
#include "cmake.h"
#include "cmState.h"

#include <assert.h>

cmProperty *cmPropertyMap::GetOrCreateProperty(const std::string& name)
{
  cmPropertyMap::iterator it = this->find(name);
  cmProperty *prop;
  if (it == this->end())
    {
    prop = &(*this)[name];
    }
  else
    {
    prop = &(it->second);
    }
  return prop;
}

void cmPropertyMap::SetProperty(const std::string& name, const char *value)
{
  if(!value)
    {
    this->erase(name);
    return;
    }

  cmProperty *prop = this->GetOrCreateProperty(name);
  prop->Set(value);
}

void cmPropertyMap::AppendProperty(const std::string& name, const char* value,
                                   bool asString)
{
  // Skip if nothing to append.
  if(!value || !*value)
    {
    return;
    }

  cmProperty *prop = this->GetOrCreateProperty(name);
  prop->Append(value,asString);
}

const char *cmPropertyMap
::GetPropertyValue(const std::string& name,
                   cmProperty::ScopeType scope,
                   bool &chain) const
{
  chain = false;
  assert(!name.empty());

  cmPropertyMap::const_iterator it = this->find(name);
  if (it == this->end())
    {
    // should we chain up?
    if (this->CMakeInstance)
      {
      chain = this->CMakeInstance->GetState()->
                    IsPropertyChained(name,scope);
      }
    return 0;
    }
  return it->second.GetValue();
}

