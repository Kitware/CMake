/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmLocalXCodeGenerator.h"
#include "cmGlobalXCodeGenerator.h"
#include "cmSourceFile.h"
#include "cmMakefile.h"

//----------------------------------------------------------------------------
cmLocalXCodeGenerator::cmLocalXCodeGenerator()
{
  // the global generator does this, so do not
  // put these flags into the language flags
  this->EmitUniversalBinaryFlags = false;
}

//----------------------------------------------------------------------------
cmLocalXCodeGenerator::~cmLocalXCodeGenerator()
{
}

//----------------------------------------------------------------------------
std::string
cmLocalXCodeGenerator::GetTargetDirectory(cmTarget const&) const
{
  // No per-target directory for this generator (yet).
  return "";
}

//----------------------------------------------------------------------------
void cmLocalXCodeGenerator::AppendFlagEscape(std::string& flags,
                                             const char* rawFlag)
{
  cmGlobalXCodeGenerator* gg =
    static_cast<cmGlobalXCodeGenerator*>(this->GlobalGenerator);
  gg->AppendFlag(flags, rawFlag);
}

//----------------------------------------------------------------------------
void cmLocalXCodeGenerator::Generate()
{
  cmLocalGenerator::Generate();

  const cmGeneratorTargetsType &targets =
                                        this->Makefile->GetGeneratorTargets();
  for(cmGeneratorTargetsType::const_iterator iter = targets.begin();
      iter != targets.end(); ++iter)
    {
    if (l->first->IsImported())
      {
      continue;
      }
    t->HasMacOSXRpathInstallNameDir(NULL);
    }
}

//----------------------------------------------------------------------------
void cmLocalXCodeGenerator::GenerateInstallRules()
{
  cmLocalGenerator::GenerateInstallRules();

  const cmGeneratorTargetsType &targets =
                                        this->Makefile->GetGeneratorTargets();
  for(cmGeneratorTargetsType::const_iterator iter = targets.begin();
      iter != targets.end(); ++iter)
    {
    if (iter->first->IsImported())
      {
      continue;
      }
    cmGeneratorTarget* t = iter->second;
    t->HasMacOSXRpathInstallNameDir(NULL);
    }
}
