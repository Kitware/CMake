/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2012 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmGeneratorTarget.h"

#include "cmTarget.h"
#include "cmMakefile.h"
#include "cmLocalGenerator.h"
#include "cmGlobalGenerator.h"
#include "cmSourceFile.h"

//----------------------------------------------------------------------------
cmGeneratorTarget::cmGeneratorTarget(cmTarget* t): Target(t)
{
  this->Makefile = this->Target->GetMakefile();
  this->LocalGenerator = this->Makefile->GetLocalGenerator();
  this->GlobalGenerator = this->LocalGenerator->GetGlobalGenerator();
  this->ClassifySources();
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::ClassifySources()
{
  std::vector<cmSourceFile*> const& sources = this->Target->GetSourceFiles();
  for(std::vector<cmSourceFile*>::const_iterator si = sources.begin();
      si != sources.end(); ++si)
    {
    cmSourceFile* sf = *si;
    cmTarget::SourceFileFlags tsFlags =
      this->Target->GetTargetSourceFileFlags(sf);
    if(sf->GetCustomCommand())
      {
      this->CustomCommands.push_back(sf);
      }
    else if(tsFlags.Type != cmTarget::SourceFileTypeNormal)
      {
      this->OSXContent.push_back(sf);
      }
    else if(sf->GetPropertyAsBool("HEADER_FILE_ONLY"))
      {
      this->HeaderSources.push_back(sf);
      }
    else if(sf->GetPropertyAsBool("EXTERNAL_OBJECT"))
      {
      this->ExternalObjects.push_back(sf);
      }
    else if(cmSystemTools::LowerCase(sf->GetExtension()) == "def")
      {
      this->ModuleDefinitionFile = sf->GetFullPath();
      }
    else if(this->GlobalGenerator->IgnoreFile(sf->GetExtension().c_str()))
      {
      // We only get here if a source file is not an external object
      // and has an extension that is listed as an ignored file type.
      // No message or diagnosis should be given.
      }
    else if(sf->GetLanguage())
      {
      this->ObjectSources.push_back(sf);
      }
    else
      {
      this->ExtraSources.push_back(sf);
      }
    }
}
