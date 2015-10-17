/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2012 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmExportSet.h"
#include "cmTargetExport.h"
#include "cmAlgorithms.h"
#include "cmLocalGenerator.h"

cmExportSet::~cmExportSet()
{
  cmDeleteAll(this->TargetExports);
}

void cmExportSet::Compute(cmLocalGenerator* lg)
{
  for (std::vector<cmTargetExport*>::iterator it = this->TargetExports.begin();
       it != this->TargetExports.end(); ++it)
    {
    (*it)->Target = lg->FindGeneratorTargetToUse((*it)->TargetName);
    }
}

void cmExportSet::AddTargetExport(cmTargetExport* te)
{
  this->TargetExports.push_back(te);
}

void cmExportSet::AddInstallation(cmInstallExportGenerator const* installation)
{
  this->Installations.push_back(installation);
}
