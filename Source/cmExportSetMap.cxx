/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2012 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmExportSetMap.h"
#include "cmExportSet.h"

cmExportSet* cmExportSetMap::operator[](const std::string &name)
{
  std::map<std::string, cmExportSet*>::iterator it = this->find(name);
  if (it == this->end()) // Export set not found
    {
    it = this->insert(std::make_pair(name, new cmExportSet(name))).first;
    }
  return it->second;
}

void cmExportSetMap::clear()
{
  for(std::map<std::string, cmExportSet*>::iterator it = this->begin();
      it != this->end();
      ++ it)
    {
    delete it->second;
    }
  this->derived::clear();
}

cmExportSetMap::~cmExportSetMap()
{
  this->clear();
}
