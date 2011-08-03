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

cmProperty *cmPropertyMap::GetOrCreateProperty(const char *name)
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

void cmPropertyMap::SetProperty(const char *name, const char *value,
                                cmProperty::ScopeType scope)
{
  if (!name)
    {
    return;
    }
  if(!value)
    {
    this->erase(name);
    return;
    }
#ifdef CMAKE_STRICT
  if (!this->CMakeInstance)
    {
    cmSystemTools::Error("CMakeInstance not set on a property map!"); 
    abort();
    }
  else
    {
    this->CMakeInstance->RecordPropertyAccess(name,scope);
    }
#else
  (void)scope;
#endif

  cmProperty *prop = this->GetOrCreateProperty(name);
  prop->Set(name,value);
}

void cmPropertyMap::AppendProperty(const char* name, const char* value,
                                   cmProperty::ScopeType scope, bool asString)
{
  // Skip if nothing to append.
  if(!name || !value || !*value)
    {
    return;
    }
#ifdef CMAKE_STRICT
  if (!this->CMakeInstance)
    {
    cmSystemTools::Error("CMakeInstance not set on a property map!");
    abort();
    }
  else
    {
    this->CMakeInstance->RecordPropertyAccess(name,scope);
    }
#else
  (void)scope;
#endif

  cmProperty *prop = this->GetOrCreateProperty(name);
  prop->Append(name,value,asString);
}

const char *cmPropertyMap
::GetPropertyValue(const char *name, 
                   cmProperty::ScopeType scope, 
                   bool &chain) const
{ 
  chain = false;
  if (!name)
    {
    return 0;
    }

  // has the property been defined?
#ifdef CMAKE_STRICT
  if (!this->CMakeInstance)
    {
    cmSystemTools::Error("CMakeInstance not set on a property map!"); 
    abort();
    }
  else
    {
    this->CMakeInstance->RecordPropertyAccess(name,scope);
    }
#endif

  cmPropertyMap::const_iterator it = this->find(name);
  if (it == this->end())
    {
    // should we chain up?
    if (this->CMakeInstance)
      {
      chain = this->CMakeInstance->IsPropertyChained(name,scope);
      }
    return 0;
    }
  return it->second.GetValue();
}

