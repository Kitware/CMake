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
#include "cmGeneratorExpression.h"
#include "cmGeneratorExpressionDAGChecker.h"

//----------------------------------------------------------------------------
cmGeneratorTarget::cmGeneratorTarget(cmTarget* t): Target(t)
{
  this->Makefile = this->Target->GetMakefile();
  this->LocalGenerator = this->Makefile->GetLocalGenerator();
  this->GlobalGenerator = this->LocalGenerator->GetGlobalGenerator();
  this->ClassifySources();
  this->LookupObjectLibraries();
}

//----------------------------------------------------------------------------
int cmGeneratorTarget::GetType() const
{
  return this->Target->GetType();
}

//----------------------------------------------------------------------------
const char *cmGeneratorTarget::GetName() const
{
  return this->Target->GetName();
}

//----------------------------------------------------------------------------
const char *cmGeneratorTarget::GetProperty(const char *prop)
{
  return this->Target->GetProperty(prop);
}

//----------------------------------------------------------------------------
bool cmGeneratorTarget::IsSystemIncludeDirectory(const char *dir,
                                                 const char *config)
{
  for (std::set<cmStdString>::const_iterator
      it = this->Target->GetSystemIncludeDirectories().begin();
      it != this->Target->GetSystemIncludeDirectories().end(); ++it)
    {
    cmListFileBacktrace lfbt;
    cmGeneratorExpression ge(lfbt);

    std::vector<std::string> incs;
    cmSystemTools::ExpandListArgument(ge.Parse(*it)
                                       ->Evaluate(this->Makefile,
                                                  config, false), incs);
    if (std::find(incs.begin(), incs.end(), dir) != incs.end())
      {
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
bool cmGeneratorTarget::GetPropertyAsBool(const char *prop)
{
  return this->Target->GetPropertyAsBool(prop);
}

//----------------------------------------------------------------------------
std::vector<cmSourceFile*> const& cmGeneratorTarget::GetSourceFiles()
{
  return this->Target->GetSourceFiles();
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::ClassifySources()
{
  cmsys::RegularExpression header(CM_HEADER_REGEX);
  bool isObjLib = this->Target->GetType() == cmTarget::OBJECT_LIBRARY;
  std::vector<cmSourceFile*> badObjLib;
  std::vector<cmSourceFile*> const& sources = this->Target->GetSourceFiles();
  for(std::vector<cmSourceFile*>::const_iterator si = sources.begin();
      si != sources.end(); ++si)
    {
    cmSourceFile* sf = *si;
    std::string ext = cmSystemTools::LowerCase(sf->GetExtension());
    if(sf->GetCustomCommand())
      {
      this->CustomCommands.push_back(sf);
      }
    else if(sf->GetPropertyAsBool("HEADER_FILE_ONLY"))
      {
      this->HeaderSources.push_back(sf);
      }
    else if(sf->GetPropertyAsBool("EXTERNAL_OBJECT"))
      {
      this->ExternalObjects.push_back(sf);
      if(isObjLib) { badObjLib.push_back(sf); }
      }
    else if(sf->GetLanguage())
      {
      this->ObjectSources.push_back(sf);
      }
    else if(ext == "def")
      {
      this->ModuleDefinitionFile = sf->GetFullPath();
      if(isObjLib) { badObjLib.push_back(sf); }
      }
    else if(ext == "idl")
      {
      this->IDLSources.push_back(sf);
      if(isObjLib) { badObjLib.push_back(sf); }
      }
    else if(ext == "resx")
      {
      // Build and save the name of the corresponding .h file
      // This relationship will be used later when building the project files.
      // Both names would have been auto generated from Visual Studio
      // where the user supplied the file name and Visual Studio
      // appended the suffix.
      std::string resx = sf->GetFullPath();
      std::string hFileName = resx.substr(0, resx.find_last_of(".")) + ".h";
      this->ExpectedResxHeaders.insert(hFileName);
      this->ResxSources.push_back(sf);
      }
    else if(header.find(sf->GetFullPath().c_str()))
      {
      this->HeaderSources.push_back(sf);
      }
    else if(this->GlobalGenerator->IgnoreFile(sf->GetExtension().c_str()))
      {
      // We only get here if a source file is not an external object
      // and has an extension that is listed as an ignored file type.
      // No message or diagnosis should be given.
      this->ExtraSources.push_back(sf);
      }
    else
      {
      this->ExtraSources.push_back(sf);
      if(isObjLib && ext != "txt")
        {
        badObjLib.push_back(sf);
        }
      }
    }

  if(!badObjLib.empty())
    {
    cmOStringStream e;
    e << "OBJECT library \"" << this->Target->GetName() << "\" contains:\n";
    for(std::vector<cmSourceFile*>::iterator i = badObjLib.begin();
        i != badObjLib.end(); ++i)
      {
      e << "  " << (*i)->GetLocation().GetName() << "\n";
      }
    e << "but may contain only headers and sources that compile.";
    this->GlobalGenerator->GetCMakeInstance()
      ->IssueMessage(cmake::FATAL_ERROR, e.str(),
                     this->Target->GetBacktrace());
    }
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::LookupObjectLibraries()
{
  std::vector<std::string> const& objLibs =
    this->Target->GetObjectLibraries();
  for(std::vector<std::string>::const_iterator oli = objLibs.begin();
      oli != objLibs.end(); ++oli)
    {
    std::string const& objLibName = *oli;
    if(cmTarget* objLib = this->Makefile->FindTargetToUse(objLibName.c_str()))
      {
      if(objLib->GetType() == cmTarget::OBJECT_LIBRARY)
        {
        if(this->Target->GetType() != cmTarget::EXECUTABLE &&
           this->Target->GetType() != cmTarget::STATIC_LIBRARY &&
           this->Target->GetType() != cmTarget::SHARED_LIBRARY &&
           this->Target->GetType() != cmTarget::MODULE_LIBRARY)
          {
          this->GlobalGenerator->GetCMakeInstance()
            ->IssueMessage(cmake::FATAL_ERROR,
                           "Only executables and non-OBJECT libraries may "
                           "reference target objects.",
                           this->Target->GetBacktrace());
          return;
          }
        this->Target->AddUtility(objLib->GetName());
        this->ObjectLibraries.push_back(objLib);
        }
      else
        {
        cmOStringStream e;
        e << "Objects of target \"" << objLibName
          << "\" referenced but is not an OBJECT library.";
        this->GlobalGenerator->GetCMakeInstance()
          ->IssueMessage(cmake::FATAL_ERROR, e.str(),
                         this->Target->GetBacktrace());
        return;
        }
      }
    else
      {
      cmOStringStream e;
      e << "Objects of target \"" << objLibName
        << "\" referenced but no such target exists.";
      this->GlobalGenerator->GetCMakeInstance()
        ->IssueMessage(cmake::FATAL_ERROR, e.str(),
                       this->Target->GetBacktrace());
      return;
      }
    }
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::UseObjectLibraries(std::vector<std::string>& objs)
{
  for(std::vector<cmTarget*>::const_iterator
        ti = this->ObjectLibraries.begin();
      ti != this->ObjectLibraries.end(); ++ti)
    {
    cmTarget* objLib = *ti;
    cmGeneratorTarget* ogt =
      this->GlobalGenerator->GetGeneratorTarget(objLib);
    for(std::vector<cmSourceFile*>::const_iterator
          si = ogt->ObjectSources.begin();
        si != ogt->ObjectSources.end(); ++si)
      {
      std::string obj = ogt->ObjectDirectory;
      obj += ogt->Objects[*si];
      objs.push_back(obj);
      }
    }
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::GetAppleArchs(const char* config,
                             std::vector<std::string>& archVec)
{
  const char* archs = 0;
  if(config && *config)
    {
    std::string defVarName = "OSX_ARCHITECTURES_";
    defVarName += cmSystemTools::UpperCase(config);
    archs = this->Target->GetProperty(defVarName.c_str());
    }
  if(!archs)
    {
    archs = this->Target->GetProperty("OSX_ARCHITECTURES");
    }
  if(archs)
    {
    cmSystemTools::ExpandListArgument(std::string(archs), archVec);
    }
}

//----------------------------------------------------------------------------
const char* cmGeneratorTarget::GetCreateRuleVariable()
{
  switch(this->GetType())
    {
    case cmTarget::STATIC_LIBRARY:
      return "_CREATE_STATIC_LIBRARY";
    case cmTarget::SHARED_LIBRARY:
      return "_CREATE_SHARED_LIBRARY";
    case cmTarget::MODULE_LIBRARY:
      return "_CREATE_SHARED_MODULE";
    case cmTarget::EXECUTABLE:
      return "_LINK_EXECUTABLE";
    default:
      break;
    }
  return "";
}

//----------------------------------------------------------------------------
std::vector<std::string> cmGeneratorTarget::GetIncludeDirectories(
                                                          const char *config)
{
  return this->Target->GetIncludeDirectories(config);
}
