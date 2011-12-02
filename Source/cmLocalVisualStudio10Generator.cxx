/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmLocalVisualStudio10Generator.h"
#include "cmTarget.h"
#include "cmMakefile.h"
#include "cmVisualStudio10TargetGenerator.h"
#include "cmGlobalVisualStudio10Generator.h"
#include <cm_expat.h>
#include "cmXMLParser.h"
class cmVS10XMLParser : public cmXMLParser
{
  public:
  virtual void EndElement(const char* /* name */)
    {
    } 
  virtual void CharacterDataHandler(const char* data, int length)
    { 
      if(this->DoGUID )
        {
        this->GUID.assign(data+1, length-2);
        this->DoGUID = false;
        }
    }
  virtual void StartElement(const char* name, const char**)
    {
      // once the GUID is found do nothing
      if(this->GUID.size())
        {
        return;
        }
      if(strcmp("ProjectGUID", name) == 0 || strcmp("ProjectGuid", name) == 0)
        {
        this->DoGUID = true;
        } 
    }
  int InitializeParser()
    {
      this->DoGUID = false;
      int ret = cmXMLParser::InitializeParser();
      if(ret == 0)
        {
        return ret;
        }
      // visual studio projects have a strange encoding, but it is 
      // really utf-8
      XML_SetEncoding(static_cast<XML_Parser>(this->Parser), "utf-8");
      return 1;
    }
  std::string GUID;
  bool DoGUID;
};


//----------------------------------------------------------------------------
cmLocalVisualStudio10Generator::cmLocalVisualStudio10Generator(VSVersion v):
  cmLocalVisualStudio7Generator(v)
{
}

cmLocalVisualStudio10Generator::~cmLocalVisualStudio10Generator()
{
}

void cmLocalVisualStudio10Generator::Generate()
{
  
  cmTargets &tgts = this->Makefile->GetTargets();
  // Create the regeneration custom rule.
  if(!this->Makefile->IsOn("CMAKE_SUPPRESS_REGENERATION"))
    {
    // Create a rule to regenerate the build system when the target
    // specification source changes.
    if(cmSourceFile* sf = this->CreateVCProjBuildRule())
      {
      // Add the rule to targets that need it.
      for(cmTargets::iterator l = tgts.begin(); l != tgts.end(); ++l)
        {
        if(l->first != CMAKE_CHECK_BUILD_SYSTEM_TARGET)
          {
          l->second.AddSourceFile(sf);
          }
        }
      }
    }

  for(cmTargets::iterator l = tgts.begin(); l != tgts.end(); ++l)
    {
    if(static_cast<cmGlobalVisualStudioGenerator*>(this->GlobalGenerator)
       ->TargetIsFortranOnly(l->second))
      {
      this->CreateSingleVCProj(l->first.c_str(),l->second);
      }
    else
      {
      cmVisualStudio10TargetGenerator tg(
        &l->second, static_cast<cmGlobalVisualStudio10Generator*>(
          this->GetGlobalGenerator()));
      tg.Generate();
      }
    }
  this->WriteStampFiles();
}


void cmLocalVisualStudio10Generator
::ReadAndStoreExternalGUID(const char* name,
                           const char* path)
{
  cmVS10XMLParser parser;
  parser.ParseFile(path); 
  std::string guidStoreName = name;
  guidStoreName += "_GUID_CMAKE";
  // save the GUID in the cache
  this->GlobalGenerator->GetCMakeInstance()->
    AddCacheEntry(guidStoreName.c_str(),
                  parser.GUID.c_str(),
                  "Stored GUID",
                  cmCacheManager::INTERNAL);
}

//----------------------------------------------------------------------------
const char* cmLocalVisualStudio10Generator::ReportErrorLabel() const
{
  return ":VCEnd";
}
