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

#include <queue>

//----------------------------------------------------------------------------
cmGeneratorTarget::cmGeneratorTarget(cmTarget* t): Target(t)
{
  this->Makefile = this->Target->GetMakefile();
  this->LocalGenerator = this->Makefile->GetLocalGenerator();
  this->GlobalGenerator = this->LocalGenerator->GetGlobalGenerator();
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
std::vector<cmSourceFile*> const*
cmGeneratorTarget::GetSourceDepends(cmSourceFile* sf)
{
  SourceEntriesType::iterator i = this->SourceEntries.find(sf);
  if(i != this->SourceEntries.end())
    {
    return &i->second.Depends;
    }
  return 0;
}

//----------------------------------------------------------------------------
bool cmGeneratorTarget::IsSystemIncludeDirectory(const char *dir,
                                                 const char *config)
{
  std::string config_upper;
  if(config && *config)
    {
    config_upper = cmSystemTools::UpperCase(config);
    }

  typedef std::map<std::string, std::vector<std::string> > IncludeCacheType;
  IncludeCacheType::iterator iter =
      this->SystemIncludesCache.find(config_upper);

  if (iter == this->SystemIncludesCache.end())
    {
    std::vector<std::string> result;
    for (std::set<cmStdString>::const_iterator
        it = this->Target->GetSystemIncludeDirectories().begin();
        it != this->Target->GetSystemIncludeDirectories().end(); ++it)
      {
      cmListFileBacktrace lfbt;
      cmGeneratorExpression ge(lfbt);

      cmGeneratorExpressionDAGChecker dagChecker(lfbt,
                                                this->GetName(),
                                "INTERFACE_SYSTEM_INCLUDE_DIRECTORIES", 0, 0);

      cmSystemTools::ExpandListArgument(ge.Parse(*it)
                                        ->Evaluate(this->Makefile,
                                        config, false, this->Target,
                                        &dagChecker), result);
      }
    for(std::vector<std::string>::iterator li = result.begin();
        li != result.end(); ++li)
      {
      cmSystemTools::ConvertToUnixSlashes(*li);
      }

    IncludeCacheType::value_type entry(config_upper, result);
    iter = this->SystemIncludesCache.insert(entry).first;
    }

  if (std::find(iter->second.begin(),
                iter->second.end(), dir) != iter->second.end())
    {
    return true;
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
class cmTargetTraceDependencies
{
public:
  cmTargetTraceDependencies(cmGeneratorTarget* target);
  void Trace();
private:
  cmTarget* Target;
  cmGeneratorTarget* GeneratorTarget;
  cmMakefile* Makefile;
  cmGlobalGenerator* GlobalGenerator;
  typedef cmGeneratorTarget::SourceEntry SourceEntry;
  SourceEntry* CurrentEntry;
  std::queue<cmSourceFile*> SourceQueue;
  std::set<cmSourceFile*> SourcesQueued;
  typedef std::map<cmStdString, cmSourceFile*> NameMapType;
  NameMapType NameMap;

  void QueueSource(cmSourceFile* sf);
  void FollowName(std::string const& name);
  void FollowNames(std::vector<std::string> const& names);
  bool IsUtility(std::string const& dep);
  void CheckCustomCommand(cmCustomCommand const& cc);
  void CheckCustomCommands(const std::vector<cmCustomCommand>& commands);
};

//----------------------------------------------------------------------------
cmTargetTraceDependencies
::cmTargetTraceDependencies(cmGeneratorTarget* target):
  Target(target->Target), GeneratorTarget(target)
{
  // Convenience.
  this->Makefile = this->Target->GetMakefile();
  this->GlobalGenerator =
    this->Makefile->GetLocalGenerator()->GetGlobalGenerator();
  this->CurrentEntry = 0;

  // Queue all the source files already specified for the target.
  std::vector<cmSourceFile*> const& sources = this->Target->GetSourceFiles();
  for(std::vector<cmSourceFile*>::const_iterator si = sources.begin();
      si != sources.end(); ++si)
    {
    this->QueueSource(*si);
    }

  // Queue pre-build, pre-link, and post-build rule dependencies.
  this->CheckCustomCommands(this->Target->GetPreBuildCommands());
  this->CheckCustomCommands(this->Target->GetPreLinkCommands());
  this->CheckCustomCommands(this->Target->GetPostBuildCommands());
}

//----------------------------------------------------------------------------
void cmTargetTraceDependencies::Trace()
{
  // Process one dependency at a time until the queue is empty.
  while(!this->SourceQueue.empty())
    {
    // Get the next source from the queue.
    cmSourceFile* sf = this->SourceQueue.front();
    this->SourceQueue.pop();
    this->CurrentEntry = &this->GeneratorTarget->SourceEntries[sf];

    // Queue dependencies added explicitly by the user.
    if(const char* additionalDeps = sf->GetProperty("OBJECT_DEPENDS"))
      {
      std::vector<std::string> objDeps;
      cmSystemTools::ExpandListArgument(additionalDeps, objDeps);
      this->FollowNames(objDeps);
      }

    // Queue the source needed to generate this file, if any.
    this->FollowName(sf->GetFullPath());

    // Queue dependencies added programatically by commands.
    this->FollowNames(sf->GetDepends());

    // Queue custom command dependencies.
    if(cmCustomCommand const* cc = sf->GetCustomCommand())
      {
      this->CheckCustomCommand(*cc);
      }
    }
  this->CurrentEntry = 0;
}

//----------------------------------------------------------------------------
void cmTargetTraceDependencies::QueueSource(cmSourceFile* sf)
{
  if(this->SourcesQueued.insert(sf).second)
    {
    this->SourceQueue.push(sf);

    // Make sure this file is in the target.
    this->Target->AddSourceFile(sf);
    }
}

//----------------------------------------------------------------------------
void cmTargetTraceDependencies::FollowName(std::string const& name)
{
  NameMapType::iterator i = this->NameMap.find(name);
  if(i == this->NameMap.end())
    {
    // Check if we know how to generate this file.
    cmSourceFile* sf = this->Makefile->GetSourceFileWithOutput(name.c_str());
    NameMapType::value_type entry(name, sf);
    i = this->NameMap.insert(entry).first;
    }
  if(cmSourceFile* sf = i->second)
    {
    // Record the dependency we just followed.
    if(this->CurrentEntry)
      {
      this->CurrentEntry->Depends.push_back(sf);
      }

    this->QueueSource(sf);
    }
}

//----------------------------------------------------------------------------
void
cmTargetTraceDependencies::FollowNames(std::vector<std::string> const& names)
{
  for(std::vector<std::string>::const_iterator i = names.begin();
      i != names.end(); ++i)
    {
    this->FollowName(*i);
    }
}

//----------------------------------------------------------------------------
bool cmTargetTraceDependencies::IsUtility(std::string const& dep)
{
  // Dependencies on targets (utilities) are supposed to be named by
  // just the target name.  However for compatibility we support
  // naming the output file generated by the target (assuming there is
  // no output-name property which old code would not have set).  In
  // that case the target name will be the file basename of the
  // dependency.
  std::string util = cmSystemTools::GetFilenameName(dep);
  if(cmSystemTools::GetFilenameLastExtension(util) == ".exe")
    {
    util = cmSystemTools::GetFilenameWithoutLastExtension(util);
    }

  // Check for a target with this name.
  if(cmTarget* t = this->Makefile->FindTargetToUse(util.c_str()))
    {
    // If we find the target and the dep was given as a full path,
    // then make sure it was not a full path to something else, and
    // the fact that the name matched a target was just a coincidence.
    if(cmSystemTools::FileIsFullPath(dep.c_str()))
      {
      if(t->GetType() >= cmTarget::EXECUTABLE &&
         t->GetType() <= cmTarget::MODULE_LIBRARY)
        {
        // This is really only for compatibility so we do not need to
        // worry about configuration names and output names.
        std::string tLocation = t->GetLocation(0);
        tLocation = cmSystemTools::GetFilenamePath(tLocation);
        std::string depLocation = cmSystemTools::GetFilenamePath(dep);
        depLocation = cmSystemTools::CollapseFullPath(depLocation.c_str());
        tLocation = cmSystemTools::CollapseFullPath(tLocation.c_str());
        if(depLocation == tLocation)
          {
          this->Target->AddUtility(util.c_str());
          return true;
          }
        }
      }
    else
      {
      // The original name of the dependency was not a full path.  It
      // must name a target, so add the target-level dependency.
      this->Target->AddUtility(util.c_str());
      return true;
      }
    }

  // The dependency does not name a target built in this project.
  return false;
}

//----------------------------------------------------------------------------
void
cmTargetTraceDependencies
::CheckCustomCommand(cmCustomCommand const& cc)
{
  // Transform command names that reference targets built in this
  // project to corresponding target-level dependencies.
  cmGeneratorExpression ge(cc.GetBacktrace());

  // Add target-level dependencies referenced by generator expressions.
  std::set<cmTarget*> targets;

  for(cmCustomCommandLines::const_iterator cit = cc.GetCommandLines().begin();
      cit != cc.GetCommandLines().end(); ++cit)
    {
    std::string const& command = *cit->begin();
    // Check for a target with this name.
    if(cmTarget* t = this->Makefile->FindTargetToUse(command.c_str()))
      {
      if(t->GetType() == cmTarget::EXECUTABLE)
        {
        // The command refers to an executable target built in
        // this project.  Add the target-level dependency to make
        // sure the executable is up to date before this custom
        // command possibly runs.
        this->Target->AddUtility(command.c_str());
        }
      }

    // Check for target references in generator expressions.
    for(cmCustomCommandLine::const_iterator cli = cit->begin();
        cli != cit->end(); ++cli)
      {
      const cmsys::auto_ptr<cmCompiledGeneratorExpression> cge
                                                              = ge.Parse(*cli);
      cge->Evaluate(this->Makefile, 0, true);
      std::set<cmTarget*> geTargets = cge->GetTargets();
      for(std::set<cmTarget*>::const_iterator it = geTargets.begin();
          it != geTargets.end(); ++it)
        {
        targets.insert(*it);
        }
      }
    }

  for(std::set<cmTarget*>::iterator ti = targets.begin();
      ti != targets.end(); ++ti)
    {
    this->Target->AddUtility((*ti)->GetName());
    }

  // Queue the custom command dependencies.
  std::vector<std::string> const& depends = cc.GetDepends();
  for(std::vector<std::string>::const_iterator di = depends.begin();
      di != depends.end(); ++di)
    {
    std::string const& dep = *di;
    if(!this->IsUtility(dep))
      {
      // The dependency does not name a target and may be a file we
      // know how to generate.  Queue it.
      this->FollowName(dep);
      }
    }
}

//----------------------------------------------------------------------------
void
cmTargetTraceDependencies
::CheckCustomCommands(const std::vector<cmCustomCommand>& commands)
{
  for(std::vector<cmCustomCommand>::const_iterator cli = commands.begin();
      cli != commands.end(); ++cli)
    {
    this->CheckCustomCommand(*cli);
    }
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::TraceDependencies()
{
  // CMake-generated targets have no dependencies to trace.  Normally tracing
  // would find nothing anyway, but when building CMake itself the "install"
  // target command ends up referencing the "cmake" target but we do not
  // really want the dependency because "install" depend on "all" anyway.
  if(this->GetType() == cmTarget::GLOBAL_TARGET)
    {
    return;
    }

  // Use a helper object to trace the dependencies.
  cmTargetTraceDependencies tracer(this);
  tracer.Trace();
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

//----------------------------------------------------------------------------
void cmGeneratorTarget::GenerateTargetManifest(const char* config)
{
  if (this->Target->IsImported())
    {
    return;
    }
  cmMakefile* mf = this->Target->GetMakefile();
  cmLocalGenerator* lg = mf->GetLocalGenerator();
  cmGlobalGenerator* gg = lg->GetGlobalGenerator();

  // Get the names.
  std::string name;
  std::string soName;
  std::string realName;
  std::string impName;
  std::string pdbName;
  if(this->GetType() == cmTarget::EXECUTABLE)
    {
    this->Target->GetExecutableNames(name, realName, impName, pdbName,
                                     config);
    }
  else if(this->GetType() == cmTarget::STATIC_LIBRARY ||
          this->GetType() == cmTarget::SHARED_LIBRARY ||
          this->GetType() == cmTarget::MODULE_LIBRARY)
    {
    this->Target->GetLibraryNames(name, soName, realName, impName, pdbName,
                                  config);
    }
  else
    {
    return;
    }

  // Get the directory.
  std::string dir = this->Target->GetDirectory(config, false);

  // Add each name.
  std::string f;
  if(!name.empty())
    {
    f = dir;
    f += "/";
    f += name;
    gg->AddToManifest(config? config:"", f);
    }
  if(!soName.empty())
    {
    f = dir;
    f += "/";
    f += soName;
    gg->AddToManifest(config? config:"", f);
    }
  if(!realName.empty())
    {
    f = dir;
    f += "/";
    f += realName;
    gg->AddToManifest(config? config:"", f);
    }
  if(!pdbName.empty())
    {
    f = dir;
    f += "/";
    f += pdbName;
    gg->AddToManifest(config? config:"", f);
    }
  if(!impName.empty())
    {
    f = this->Target->GetDirectory(config, true);
    f += "/";
    f += impName;
    gg->AddToManifest(config? config:"", f);
    }
}
