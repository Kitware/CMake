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
#include "cmComputeLinkInformation.h"
#include "cmGlobalGenerator.h"
#include "cmSourceFile.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorExpressionDAGChecker.h"
#include "cmComputeLinkInformation.h"

#include <queue>
#include <assert.h>
#include <errno.h>

//----------------------------------------------------------------------------
cmTargetLinkInformationMap
::cmTargetLinkInformationMap(cmTargetLinkInformationMap const& r): derived()
{
  // Ideally cmTarget instances should never be copied.  However until
  // we can make a sweep to remove that, this copy constructor avoids
  // allowing the resources (LinkInformation) from getting copied.  In
  // the worst case this will lead to extra cmComputeLinkInformation
  // instances.  We also enforce in debug mode that the map be emptied
  // when copied.
  static_cast<void>(r);
  assert(r.empty());
}

//----------------------------------------------------------------------------
cmTargetLinkInformationMap::~cmTargetLinkInformationMap()
{
  for(derived::iterator i = this->begin(); i != this->end(); ++i)
    {
    delete i->second;
    }
}

#include "assert.h"

//----------------------------------------------------------------------------
cmGeneratorTarget::cmGeneratorTarget(cmTarget* t): Target(t)
{
  this->Makefile = this->Target->GetMakefile();
  this->LocalGenerator = this->Makefile->GetLocalGenerator();
  this->GlobalGenerator = this->LocalGenerator->GetGlobalGenerator();

  this->DebugIncludesDone = false;
  this->PolicyWarnedCMP0022 = false;
}

//----------------------------------------------------------------------------
void deleteAndClear2(
      std::vector<cmGeneratorTarget::TargetPropertyEntry*> &entries)
{
  for (std::vector<cmGeneratorTarget::TargetPropertyEntry*>::const_iterator
      it = entries.begin(),
      end = entries.end();
      it != end; ++it)
    {
      delete *it;
    }
  entries.clear();
}

//----------------------------------------------------------------------------
void deleteAndClear2(
  std::map<std::string,
          std::vector<cmGeneratorTarget::TargetPropertyEntry*> > &entries)
{
  for (std::map<std::string,
          std::vector<cmGeneratorTarget::TargetPropertyEntry*> >::iterator
        it = entries.begin(), end = entries.end(); it != end; ++it)
    {
    deleteAndClear2(it->second);
    }
}

cmGeneratorTarget::~cmGeneratorTarget()
{
  deleteAndClear2(this->CachedLinkInterfaceIncludeDirectoriesEntries);

  for (cmTargetLinkInformationMap::const_iterator it
      = this->LinkInformation.begin();
      it != this->LinkInformation.end(); ++it)
    {
    delete it->second;
    }
  this->LinkInformation.clear();
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
const char *cmGeneratorTarget::GetProperty(const char *prop) const
{
  return this->Target->GetProperty(prop);
}

//----------------------------------------------------------------------------
std::vector<cmSourceFile*> const*
cmGeneratorTarget::GetSourceDepends(cmSourceFile* sf) const
{
  SourceEntriesType::const_iterator i = this->SourceEntries.find(sf);
  if(i != this->SourceEntries.end())
    {
    return &i->second.Depends;
    }
  return 0;
}

static void handleSystemIncludesDep(cmMakefile *mf, const std::string &name,
                                  const char *config, cmTarget *headTarget,
                                  cmGeneratorExpressionDAGChecker *dagChecker,
                                  std::vector<std::string>& result,
                                  bool excludeImported)
{
  cmTarget* depTgt = mf->FindTargetToUse(name.c_str());

  if (!depTgt)
    {
    return;
    }

  cmListFileBacktrace lfbt;

  if (const char* dirs =
          depTgt->GetProperty("INTERFACE_SYSTEM_INCLUDE_DIRECTORIES"))
    {
    cmGeneratorExpression ge(lfbt);
    cmSystemTools::ExpandListArgument(ge.Parse(dirs)
                                      ->Evaluate(mf,
                                      config, false, headTarget,
                                      depTgt, dagChecker), result);
    }
  if (!depTgt->IsImported() || excludeImported)
    {
    return;
    }

  if (const char* dirs =
                depTgt->GetProperty("INTERFACE_INCLUDE_DIRECTORIES"))
    {
    cmGeneratorExpression ge(lfbt);
    cmSystemTools::ExpandListArgument(ge.Parse(dirs)
                                      ->Evaluate(mf,
                                      config, false, headTarget,
                                      depTgt, dagChecker), result);
    }
}

//----------------------------------------------------------------------------
void
cmGeneratorTarget::GetObjectSources(std::vector<cmSourceFile*> &objs) const
{
  objs = this->ObjectSources;
}

//----------------------------------------------------------------------------
const std::string& cmGeneratorTarget::GetObjectName(cmSourceFile const* file)
{
  return this->Objects[file];
}

void cmGeneratorTarget::AddObject(cmSourceFile *sf, std::string const&name)
{
    this->Objects[sf] = name;
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::AddExplicitObjectName(cmSourceFile* sf)
{
  this->ExplicitObjectName.insert(sf);
}

//----------------------------------------------------------------------------
bool cmGeneratorTarget::HasExplicitObjectName(cmSourceFile const* file) const
{
  std::set<cmSourceFile const*>::const_iterator it
                                        = this->ExplicitObjectName.find(file);
  return it != this->ExplicitObjectName.end();
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::GetResxSources(std::vector<cmSourceFile*>& srcs) const
{
  srcs = this->ResxSources;
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::GetIDLSources(std::vector<cmSourceFile*>& srcs) const
{
  srcs = this->IDLSources;
}

//----------------------------------------------------------------------------
void
cmGeneratorTarget::GetHeaderSources(std::vector<cmSourceFile*>& srcs) const
{
  srcs = this->HeaderSources;
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::GetExtraSources(std::vector<cmSourceFile*>& srcs) const
{
  srcs = this->ExtraSources;
}

//----------------------------------------------------------------------------
void
cmGeneratorTarget::GetCustomCommands(std::vector<cmSourceFile*>& srcs) const
{
  srcs = this->CustomCommands;
}

//----------------------------------------------------------------------------
void
cmGeneratorTarget::GetExpectedResxHeaders(std::set<std::string>& srcs) const
{
  srcs = this->ExpectedResxHeaders;
}

//----------------------------------------------------------------------------
void
cmGeneratorTarget::GetExternalObjects(std::vector<cmSourceFile*>& srcs) const
{
  srcs = this->ExternalObjects;
}

//----------------------------------------------------------------------------
const char* cmGeneratorTarget::GetLocation(const char* config) const
{
  if (this->Target->IsImported())
    {
    return this->Target->ImportedGetLocation(config);
    }
  else
    {
    return this->NormalGetLocation(config);
    }
}

bool cmGeneratorTarget::IsImported() const
{
  return this->Target->IsImported();
}

//----------------------------------------------------------------------------
const char* cmGeneratorTarget::NormalGetLocation(const char* config) const
{
  // Handle the configuration-specific case first.
  static std::string location;
  if(config)
    {
    location = this->GetFullPath(config, false);
    return location.c_str();
    }

  // Now handle the deprecated build-time configuration location.
  location = this->Target->GetDirectory();
  const char* cfgid = this->Makefile->GetDefinition("CMAKE_CFG_INTDIR");
  if(cfgid && strcmp(cfgid, ".") != 0)
    {
    location += "/";
    location += cfgid;
    }

  if(this->Target->IsAppBundleOnApple())
    {
    std::string macdir = this->BuildMacContentDirectory("",
                                                        config,
                                                        false);
    if(!macdir.empty())
      {
      location += "/";
      location += macdir;
      }
    }
  location += "/";
  location += this->GetFullName(config, false);
  return location.c_str();
}

//----------------------------------------------------------------------------
bool cmGeneratorTarget::IsSystemIncludeDirectory(const char *dir,
                                                 const char *config) const
{
  assert(this->GetType() != cmTarget::INTERFACE_LIBRARY);
  std::string config_upper;
  if(config && *config)
    {
    config_upper = cmSystemTools::UpperCase(config);
    }

  typedef std::map<std::string, std::vector<std::string> > IncludeCacheType;
  IncludeCacheType::const_iterator iter =
      this->SystemIncludesCache.find(config_upper);

  if (iter == this->SystemIncludesCache.end())
    {
    LinkImplementation const* impl
                          = this->GetLinkImplementation(config, this->Target);
    if(!impl)
      {
      return false;
      }

    cmListFileBacktrace lfbt;
    cmGeneratorExpressionDAGChecker dagChecker(lfbt,
                                        this->GetName(),
                                        "SYSTEM_INCLUDE_DIRECTORIES", 0, 0);

    bool excludeImported
                = this->Target->GetPropertyAsBool("NO_SYSTEM_FROM_IMPORTED");

    std::vector<std::string> result;
    for (std::set<cmStdString>::const_iterator
        it = this->Target->GetSystemIncludeDirectories().begin();
        it != this->Target->GetSystemIncludeDirectories().end(); ++it)
      {
      cmGeneratorExpression ge(lfbt);
      cmSystemTools::ExpandListArgument(ge.Parse(*it)
                                          ->Evaluate(this->Makefile,
                                          config, false, this->Target,
                                          &dagChecker), result);
      }

    std::set<cmStdString> uniqueDeps;
    for(std::vector<std::string>::const_iterator li = impl->Libraries.begin();
        li != impl->Libraries.end(); ++li)
      {
      if (uniqueDeps.insert(*li).second)
        {
        cmGeneratorTarget* gtgt
                      = this->Makefile->FindGeneratorTargetToUse(li->c_str());

        if (!gtgt)
          {
          continue;
          }

        handleSystemIncludesDep(this->Makefile, *li, config, this->Target,
                                &dagChecker, result, excludeImported);

        std::vector<std::string> deps;
        gtgt->GetTransitivePropertyLinkLibraries(config, this->Target, deps);

        for(std::vector<std::string>::const_iterator di = deps.begin();
            di != deps.end(); ++di)
          {
          if (uniqueDeps.insert(*di).second)
            {
            handleSystemIncludesDep(this->Makefile, *di, config, this->Target,
                                    &dagChecker, result, excludeImported);
            }
          }
        }
      }
    std::set<cmStdString> unique;
    for(std::vector<std::string>::iterator li = result.begin();
        li != result.end(); ++li)
      {
      cmSystemTools::ConvertToUnixSlashes(*li);
      unique.insert(*li);
      }
    result.clear();
    for(std::set<cmStdString>::iterator li = unique.begin();
        li != unique.end(); ++li)
      {
      result.push_back(*li);
      }

    IncludeCacheType::value_type entry(config_upper, result);
    iter = this->SystemIncludesCache.insert(entry).first;
    }

  std::string dirString = dir;
  return std::binary_search(iter->second.begin(), iter->second.end(),
                            dirString);
}

//----------------------------------------------------------------------------
bool cmGeneratorTarget::GetPropertyAsBool(const char *prop) const
{
  return this->Target->GetPropertyAsBool(prop);
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::GetSourceFiles(std::vector<cmSourceFile*> &files) const
{
  this->Target->GetSourceFiles(files);
}

//----------------------------------------------------------------------------
bool cmGeneratorTarget::HasSOName(const char* config) const
{
  // soname is supported only for shared libraries and modules,
  // and then only when the platform supports an soname flag.
  return ((this->GetType() == cmTarget::SHARED_LIBRARY ||
           this->GetType() == cmTarget::MODULE_LIBRARY) &&
          !this->GetPropertyAsBool("NO_SONAME") &&
          this->Makefile->GetSONameFlag(this->GetLinkerLanguage(config,
                                                              this->Target)));
}

//----------------------------------------------------------------------------
bool cmGeneratorTarget::NeedRelinkBeforeInstall(const char* config) const
{
  // Only executables and shared libraries can have an rpath and may
  // need relinking.
  if(this->GetType() != cmTarget::EXECUTABLE &&
     this->GetType() != cmTarget::SHARED_LIBRARY &&
     this->GetType() != cmTarget::MODULE_LIBRARY)
    {
    return false;
    }

  // If there is no install location this target will not be installed
  // and therefore does not need relinking.
  if(!this->Target->GetHaveInstallRule())
    {
    return false;
    }

  // If skipping all rpaths completely then no relinking is needed.
  if(this->Makefile->IsOn("CMAKE_SKIP_RPATH"))
    {
    return false;
    }

  // If building with the install-tree rpath no relinking is needed.
  if(this->GetPropertyAsBool("BUILD_WITH_INSTALL_RPATH"))
    {
    return false;
    }

  // If chrpath is going to be used no relinking is needed.
  if(this->IsChrpathUsed(config))
    {
    return false;
    }

  // Check for rpath support on this platform.
  if(const char* ll = this->GetLinkerLanguage(config, this->Target))
    {
    std::string flagVar = "CMAKE_SHARED_LIBRARY_RUNTIME_";
    flagVar += ll;
    flagVar += "_FLAG";
    if(!this->Makefile->IsSet(flagVar.c_str()))
      {
      // There is no rpath support on this platform so nothing needs
      // relinking.
      return false;
      }
    }
  else
    {
    // No linker language is known.  This error will be reported by
    // other code.
    return false;
    }

  // If either a build or install tree rpath is set then the rpath
  // will likely change between the build tree and install tree and
  // this target must be relinked.
  return this->HaveBuildTreeRPATH(config)
      || this->HaveInstallTreeRPATH();
}

//----------------------------------------------------------------------------
bool cmGeneratorTarget::IsChrpathUsed(const char* config) const
{
  // Only certain target types have an rpath.
  if(!(this->GetType() == cmTarget::SHARED_LIBRARY ||
       this->GetType() == cmTarget::MODULE_LIBRARY ||
       this->GetType() == cmTarget::EXECUTABLE))
    {
    return false;
    }

  // If the target will not be installed we do not need to change its
  // rpath.
  if(!this->Target->GetHaveInstallRule())
    {
    return false;
    }

  // Skip chrpath if skipping rpath altogether.
  if(this->Makefile->IsOn("CMAKE_SKIP_RPATH"))
    {
    return false;
    }

  // Skip chrpath if it does not need to be changed at install time.
  if(this->GetPropertyAsBool("BUILD_WITH_INSTALL_RPATH"))
    {
    return false;
    }

  // Allow the user to disable builtin chrpath explicitly.
  if(this->Makefile->IsOn("CMAKE_NO_BUILTIN_CHRPATH"))
    {
    return false;
    }

  if(this->Makefile->IsOn("CMAKE_PLATFORM_HAS_INSTALLNAME"))
    {
    return true;
    }

#if defined(CMAKE_USE_ELF_PARSER)
  // Enable if the rpath flag uses a separator and the target uses ELF
  // binaries.
  if(const char* ll = this->GetLinkerLanguage(config, this->Target))
    {
    std::string sepVar = "CMAKE_SHARED_LIBRARY_RUNTIME_";
    sepVar += ll;
    sepVar += "_FLAG_SEP";
    const char* sep = this->Makefile->GetDefinition(sepVar.c_str());
    if(sep && *sep)
      {
      // TODO: Add ELF check to ABI detection and get rid of
      // CMAKE_EXECUTABLE_FORMAT.
      if(const char* fmt =
         this->Makefile->GetDefinition("CMAKE_EXECUTABLE_FORMAT"))
        {
        return strcmp(fmt, "ELF") == 0;
        }
      }
    }
#endif
  static_cast<void>(config);
  return false;
}


//----------------------------------------------------------------------------
std::string cmGeneratorTarget::GetSOName(const char* config) const
{
  if(this->Target->IsImported())
    {
    // Lookup the imported soname.
    if(cmTarget::ImportInfo const* info =
                            this->Target->GetImportInfo(config, this->Target))
      {
      if(info->NoSOName)
        {
        // The imported library has no builtin soname so the name
        // searched at runtime will be just the filename.
        return cmSystemTools::GetFilenameName(info->Location);
        }
      else
        {
        // Use the soname given if any.
        if(info->SOName.find("@rpath/") == 0)
          {
          return info->SOName.substr(6);
          }
        return info->SOName;
        }
      }
    else
      {
      return "";
      }
    }
  else
    {
    // Compute the soname that will be built.
    std::string name;
    std::string soName;
    std::string realName;
    std::string impName;
    std::string pdbName;
    this->GetLibraryNames(name, soName, realName,
                          impName, pdbName, config);
    return soName;
    }
}

//----------------------------------------------------------------------------
std::string cmGeneratorTarget::GetCFBundleDirectory(const char* config,
                                           bool contentOnly) const
{
  std::string fpath;
  fpath += this->Target->GetOutputName(config, false);
  fpath += ".";
  const char *ext = this->GetProperty("BUNDLE_EXTENSION");
  if (!ext)
    {
    ext = "bundle";
    }
  fpath += ext;
  fpath += "/Contents";
  if(!contentOnly)
    fpath += "/MacOS";
  return fpath;
}

//----------------------------------------------------------------------------
std::string cmGeneratorTarget::GetAppBundleDirectory(const char* config,
                                            bool contentOnly) const
{
  std::string fpath = this->GetFullName(config, false);
  fpath += ".app/Contents";
  if(!contentOnly)
    fpath += "/MacOS";
  return fpath;
}

//----------------------------------------------------------------------------
std::string
cmGeneratorTarget::GetFullName(const char* config, bool implib) const
{
  if(this->Target->IsImported())
    {
    return this->Target->GetFullNameImported(config, implib);
    }
  else
    {
    return this->GetFullNameInternal(config, implib);
    }
}

//----------------------------------------------------------------------------
std::string cmGeneratorTarget::GetFrameworkDirectory(const char* config,
                                                     bool rootDir) const
{
  std::string fpath;
  fpath += this->Target->GetOutputName(config, false);
  fpath += ".framework";
  if(!rootDir)
    {
    fpath += "/Versions/";
    fpath += this->Target->GetFrameworkVersion();
    }
  return fpath;
}

//----------------------------------------------------------------------------
std::string
cmGeneratorTarget::GetInstallNameDirForBuildTree(const char* config) const
{
  // If building directly for installation then the build tree install_name
  // is the same as the install tree.
  if(this->GetPropertyAsBool("BUILD_WITH_INSTALL_RPATH"))
    {
    return this->GetInstallNameDirForInstallTree();
    }

  // Use the build tree directory for the target.
  if(this->Makefile->IsOn("CMAKE_PLATFORM_HAS_INSTALLNAME") &&
     !this->Makefile->IsOn("CMAKE_SKIP_RPATH") &&
     !this->GetPropertyAsBool("SKIP_BUILD_RPATH"))
    {
    std::string dir;
    if(this->MacOSXRpathInstallNameDirDefault())
      {
      dir = "@rpath";
      }
    else
      {
      dir = this->Target->GetDirectory(config);
      }
    dir += "/";
    return dir;
    }
  else
    {
    return "";
    }
}

//----------------------------------------------------------------------------
std::string cmGeneratorTarget::GetInstallNameDirForInstallTree() const
{
  if(this->Makefile->IsOn("CMAKE_PLATFORM_HAS_INSTALLNAME"))
    {
    std::string dir;
    const char* install_name_dir = this->GetProperty("INSTALL_NAME_DIR");

    if(!this->Makefile->IsOn("CMAKE_SKIP_RPATH") &&
       !this->Makefile->IsOn("CMAKE_SKIP_INSTALL_RPATH"))
      {
      if(install_name_dir && *install_name_dir)
        {
        dir = install_name_dir;
        dir += "/";
        }
      }
    if(!install_name_dir)
      {
      if(this->MacOSXRpathInstallNameDirDefault())
        {
        dir = "@rpath/";
        }
      }
    return dir;
    }
  else
    {
    return "";
    }
}

//----------------------------------------------------------------------------
class cmTargetCollectLinkLanguages
{
public:
  cmTargetCollectLinkLanguages(cmGeneratorTarget const* target,
                               const char* config,
                               std::set<cmStdString>& languages,
                               cmTarget const* head):
    Config(config), Languages(languages), HeadTarget(head)
  { this->Visited.insert(target); }

  void Visit(cmGeneratorTarget const* target)
    {
    if(!target || !this->Visited.insert(target).second)
      {
      return;
      }

    cmGeneratorTarget::LinkInterface const* iface =
      target->GetLinkInterface(this->Config, this->HeadTarget);
    if(!iface) { return; }

    for(std::vector<std::string>::const_iterator
          li = iface->Languages.begin(); li != iface->Languages.end(); ++li)
      {
      this->Languages.insert(*li);
      }

    cmMakefile* mf = target->GetMakefile();
    for(std::vector<std::string>::const_iterator
          li = iface->Libraries.begin(); li != iface->Libraries.end(); ++li)
      {
      this->Visit(mf->FindGeneratorTargetToUse(li->c_str()));
      }
    }
private:
  const char* Config;
  std::set<cmStdString>& Languages;
  cmTarget const* HeadTarget;
  std::set<cmGeneratorTarget const*> Visited;
};

//----------------------------------------------------------------------------
cmGeneratorTarget::LinkClosure const*
cmGeneratorTarget::GetLinkClosure(const char* config,
                                                  cmTarget const* head) const
{
  TargetConfigPair key(head, cmSystemTools::UpperCase(config ? config : ""));
  LinkClosureMapType::iterator
    i = this->LinkClosureMap.find(key);
  if(i == this->LinkClosureMap.end())
    {
    LinkClosure lc;
    this->ComputeLinkClosure(config, lc, head);
    LinkClosureMapType::value_type entry(key, lc);
    i = this->LinkClosureMap.insert(entry).first;
    }
  return &i->second;
}

//----------------------------------------------------------------------------
class cmTargetSelectLinker
{
  int Preference;
  cmGeneratorTarget const* Target;
  cmMakefile* Makefile;
  cmGlobalGenerator* GG;
  std::set<cmStdString> Preferred;
public:
  cmTargetSelectLinker(cmGeneratorTarget const* target)
      : Preference(0), Target(target)
    {
    this->Makefile = this->Target->Makefile;
    this->GG = this->Makefile->GetLocalGenerator()->GetGlobalGenerator();
    }
  void Consider(const char* lang)
    {
    int preference = this->GG->GetLinkerPreference(lang);
    if(preference > this->Preference)
      {
      this->Preference = preference;
      this->Preferred.clear();
      }
    if(preference == this->Preference)
      {
      this->Preferred.insert(lang);
      }
    }
  std::string Choose()
    {
    if(this->Preferred.empty())
      {
      return "";
      }
    else if(this->Preferred.size() > 1)
      {
      cmOStringStream e;
      e << "Target " << this->Target->GetName()
        << " contains multiple languages with the highest linker preference"
        << " (" << this->Preference << "):\n";
      for(std::set<cmStdString>::const_iterator
            li = this->Preferred.begin(); li != this->Preferred.end(); ++li)
        {
        e << "  " << *li << "\n";
        }
      e << "Set the LINKER_LANGUAGE property for this target.";
      cmake* cm = this->Makefile->GetCMakeInstance();
      cm->IssueMessage(cmake::FATAL_ERROR, e.str(),
                       this->Target->Target->GetBacktrace());
      }
    return *this->Preferred.begin();
    }
};

//----------------------------------------------------------------------------
cmGeneratorTarget::LinkImplementation const*
cmGeneratorTarget::GetLinkImplementation(const char* config,
                                         cmTarget const* head) const
{
  // There is no link implementation for imported targets.
  if(this->Target->IsImported())
    {
    return 0;
    }

  // Lookup any existing link implementation for this configuration.
  std::string key = cmSystemTools::UpperCase(config? config : "");

  LinkImplMapType::iterator i = this->LinkImplMap.find(key);
  if(i == this->LinkImplMap.end())
    {
    // Compute the link implementation for this configuration.
    LinkImplementation impl;
    this->ComputeLinkImplementation(config, impl, head);

    // Store the information for this configuration.
    LinkImplMapType::value_type entry(key, impl);
    i = this->LinkImplMap.insert(entry).first;
    }

  return &i->second;
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::ComputeLinkImplementation(const char* config,
                                         LinkImplementation& impl,
                                         cmTarget const* head) const
{
  // Compute which library configuration to link.
  cmTarget::LinkLibraryType linkType = this->Target->ComputeLinkType(config);

  // Collect libraries directly linked in this configuration.
  std::vector<std::string> llibs;
  this->GetDirectLinkLibraries(config, llibs, head);
  for(std::vector<std::string>::const_iterator li = llibs.begin();
      li != llibs.end(); ++li)
    {
    // Skip entries that resolve to the target itself or are empty.
    std::string item = this->Target->CheckCMP0004(*li);
    if(item == this->GetName() || item.empty())
      {
      if(item == this->GetName())
        {
        bool noMessage = false;
        cmake::MessageType messageType = cmake::FATAL_ERROR;
        cmOStringStream e;
        switch(this->Makefile->GetPolicyStatus(cmPolicies::CMP0038))
          {
          case cmPolicies::WARN:
            {
            e << (this->Makefile->GetPolicies()
                  ->GetPolicyWarning(cmPolicies::CMP0038)) << "\n";
            messageType = cmake::AUTHOR_WARNING;
            }
            break;
          case cmPolicies::OLD:
            noMessage = true;
          case cmPolicies::REQUIRED_IF_USED:
          case cmPolicies::REQUIRED_ALWAYS:
          case cmPolicies::NEW:
            // Issue the fatal message.
            break;
          }

        if(!noMessage)
          {
          e << "Target \"" << this->GetName() << "\" links to itself.";
          this->Makefile->GetCMakeInstance()->IssueMessage(messageType,
                                                        e.str(),
                                                this->Target->GetBacktrace());
          if (messageType == cmake::FATAL_ERROR)
            {
            return;
            }
          }
        }
      continue;
      }
    cmTarget *tgt = this->Makefile->FindTargetToUse(li->c_str());

    if(!tgt && std::string(item).find("::") != std::string::npos)
      {
      bool noMessage = false;
      cmake::MessageType messageType = cmake::FATAL_ERROR;
      cmOStringStream e;
      switch(this->Makefile->GetPolicyStatus(cmPolicies::CMP0028))
        {
        case cmPolicies::WARN:
          {
          e << (this->Makefile->GetPolicies()
                ->GetPolicyWarning(cmPolicies::CMP0028)) << "\n";
          messageType = cmake::AUTHOR_WARNING;
          }
          break;
        case cmPolicies::OLD:
          noMessage = true;
        case cmPolicies::REQUIRED_IF_USED:
        case cmPolicies::REQUIRED_ALWAYS:
        case cmPolicies::NEW:
          // Issue the fatal message.
          break;
        }

      if(!noMessage)
        {
        e << "Target \"" << this->GetName() << "\" links to target \"" << item
          << "\" but the target was not found.  Perhaps a find_package() "
          "call is missing for an IMPORTED target, or an ALIAS target is "
          "missing?";
        this->Makefile->GetCMakeInstance()->IssueMessage(messageType,
                                                e.str(),
                                                this->Target->GetBacktrace());
        if (messageType == cmake::FATAL_ERROR)
          {
          return;
          }
        }
      }
    // The entry is meant for this configuration.
    impl.Libraries.push_back(item);
    }

  cmTarget::LinkLibraryVectorType const& oldllibs =
                                    this->Target->GetOriginalLinkLibraries();
  for(cmTarget::LinkLibraryVectorType::const_iterator li = oldllibs.begin();
      li != oldllibs.end(); ++li)
    {
    if(li->second != cmTarget::GENERAL && li->second != linkType)
      {
      std::string item = this->Target->CheckCMP0004(li->first);
      if(item == this->GetName() || item.empty())
        {
        continue;
        }
      // Support OLD behavior for CMP0003.
      impl.WrongConfigLibraries.push_back(item);
      }
    }

  // This target needs runtime libraries for its source languages.
  std::set<cmStdString> languages;
  // Get languages used in our source files.
  this->Target->GetLanguages(languages);
  // Get languages used in object library sources.
  for(std::vector<std::string>::const_iterator
      i = this->Target->GetObjectLibraries().begin();
      i != this->Target->GetObjectLibraries().end(); ++i)
    {
    if(cmTarget* objLib = this->Makefile->FindTargetToUse(i->c_str()))
      {
      if(objLib->GetType() == cmTarget::OBJECT_LIBRARY)
        {
        objLib->GetLanguages(languages);
        }
      }
    }
  // Copy the set of langauges to the link implementation.
  for(std::set<cmStdString>::iterator li = languages.begin();
      li != languages.end(); ++li)
    {
    impl.Languages.push_back(*li);
    }
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::ComputeLinkClosure(const char* config, LinkClosure& lc,
                                  cmTarget const* head) const
{
  // Get languages built in this target.
  std::set<cmStdString> languages;
  LinkImplementation const* impl = this->GetLinkImplementation(config, head);
  for(std::vector<std::string>::const_iterator li = impl->Languages.begin();
      li != impl->Languages.end(); ++li)
    {
    languages.insert(*li);
    }

  // Add interface languages from linked targets.
  cmTargetCollectLinkLanguages cll(this, config, languages, head);
  for(std::vector<std::string>::const_iterator li = impl->Libraries.begin();
      li != impl->Libraries.end(); ++li)
    {
    cll.Visit(this->Makefile->FindGeneratorTargetToUse(li->c_str()));
    }

  // Store the transitive closure of languages.
  for(std::set<cmStdString>::const_iterator li = languages.begin();
      li != languages.end(); ++li)
    {
    lc.Languages.push_back(*li);
    }

  // Choose the language whose linker should be used.
  if(this->GetProperty("HAS_CXX"))
    {
    lc.LinkerLanguage = "CXX";
    }
  else if(const char* linkerLang = this->GetProperty("LINKER_LANGUAGE"))
    {
    lc.LinkerLanguage = linkerLang;
    }
  else
    {
    // Find the language with the highest preference value.
    cmTargetSelectLinker tsl(this);

    // First select from the languages compiled directly in this target.
    for(std::vector<std::string>::const_iterator li = impl->Languages.begin();
        li != impl->Languages.end(); ++li)
      {
      tsl.Consider(li->c_str());
      }

    // Now consider languages that propagate from linked targets.
    for(std::set<cmStdString>::const_iterator sit = languages.begin();
        sit != languages.end(); ++sit)
      {
      std::string propagates = "CMAKE_"+*sit+"_LINKER_PREFERENCE_PROPAGATES";
      if(this->Makefile->IsOn(propagates.c_str()))
        {
        tsl.Consider(sit->c_str());
        }
      }

    lc.LinkerLanguage = tsl.Choose();
    }
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::GetFullNameComponents(std::string& prefix,
                                              std::string& base,
                                     std::string& suffix, const char* config,
                                     bool implib) const
{
  this->GetFullNameInternal(config, implib, prefix, base, suffix);
}

//----------------------------------------------------------------------------
std::string
cmGeneratorTarget::BuildMacContentDirectory(const std::string& base,
                                            const char* config,
                                            bool contentOnly) const
{
  std::string fpath = base;
  if(this->Target->IsAppBundleOnApple())
    {
    fpath += this->GetAppBundleDirectory(config, contentOnly);
    }
  if(this->Target->IsFrameworkOnApple())
    {
    fpath += this->GetFrameworkDirectory(config, contentOnly);
    }
  if(this->Target->IsCFBundleOnApple())
    {
    fpath += this->GetCFBundleDirectory(config, contentOnly);
    }
  return fpath;
}

//----------------------------------------------------------------------------
std::string cmGeneratorTarget::GetMacContentDirectory(const char* config,
                                             bool implib) const
{
  // Start with the output directory for the target.
  std::string fpath = this->Target->GetDirectory(config, implib);
  fpath += "/";
  bool contentOnly = true;
  if(this->Target->IsFrameworkOnApple())
    {
    // additional files with a framework go into the version specific
    // directory
    contentOnly = false;
    }
  fpath = this->BuildMacContentDirectory(fpath, config, contentOnly);
  return fpath;
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::ClassifySources()
{
  cmsys::RegularExpression header(CM_HEADER_REGEX);

  cmTarget::TargetType targetType = this->Target->GetType();
  bool isObjLib = targetType == cmTarget::OBJECT_LIBRARY;

  std::vector<cmSourceFile*> badObjLib;
  std::vector<cmSourceFile*> sources;
  this->Target->GetSourceFiles(sources);
  for(std::vector<cmSourceFile*>::const_iterator si = sources.begin();
      si != sources.end(); ++si)
    {
    cmSourceFile* sf = *si;
    std::string ext = cmSystemTools::LowerCase(sf->GetExtension());
    if(sf->GetCustomCommand())
      {
      this->CustomCommands.push_back(sf);
      }
    else if(targetType == cmTarget::UTILITY)
      {
      this->ExtraSources.push_back(sf);
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
void
cmGeneratorTarget::UseObjectLibraries(std::vector<std::string>& objs) const
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
void cmGeneratorTarget::GetTransitivePropertyLinkLibraries(
                                      const char* config,
                                      cmTarget const *headTarget,
                                      std::vector<std::string> &libs) const
{
  LinkInterface const* iface = this->GetLinkInterface(config, headTarget);
  if (!iface)
    {
    return;
    }
  if(this->GetType() != cmTarget::STATIC_LIBRARY
      || this->Target->GetPolicyStatusCMP0022() == cmPolicies::WARN
      || this->Target->GetPolicyStatusCMP0022() == cmPolicies::OLD)
    {
    libs = iface->Libraries;
    return;
    }

  const char* linkIfaceProp = "INTERFACE_LINK_LIBRARIES";
  const char* interfaceLibs = this->GetProperty(linkIfaceProp);

  if (!interfaceLibs)
    {
    return;
    }

  // The interface libraries have been explicitly set.
  cmListFileBacktrace lfbt;
  cmGeneratorExpression ge(lfbt);
  cmGeneratorExpressionDAGChecker dagChecker(lfbt, this->GetName(),
                                              linkIfaceProp, 0, 0);
  dagChecker.SetTransitivePropertiesOnly();
  cmSystemTools::ExpandListArgument(ge.Parse(interfaceLibs)->Evaluate(
                                      this->Makefile,
                                      config,
                                      false,
                                      headTarget,
                                      this->Target, &dagChecker), libs);
}

//----------------------------------------------------------------------------
template<typename PropertyType>
PropertyType getTypedProperty(cmTarget const* tgt, const char *prop,
                              PropertyType *);

//----------------------------------------------------------------------------
template<>
bool getTypedProperty<bool>(cmTarget const* tgt, const char *prop, bool *)
{
  return tgt->GetPropertyAsBool(prop);
}

//----------------------------------------------------------------------------
template<>
const char *getTypedProperty<const char *>(cmTarget const* tgt,
                                           const char *prop,
                                           const char **)
{
  return tgt->GetProperty(prop);
}

enum CompatibleType
{
  BoolType,
  StringType,
  NumberMinType,
  NumberMaxType
};

//----------------------------------------------------------------------------
template<typename PropertyType>
std::pair<bool, PropertyType> consistentProperty(PropertyType lhs,
                                                 PropertyType rhs,
                                                 CompatibleType t);

//----------------------------------------------------------------------------
template<>
std::pair<bool, bool> consistentProperty(bool lhs, bool rhs, CompatibleType)
{
  return std::make_pair(lhs == rhs, lhs);
}

//----------------------------------------------------------------------------
std::pair<bool, const char*> consistentStringProperty(const char *lhs,
                                                      const char *rhs)
{
  const bool b = strcmp(lhs, rhs) == 0;
  return std::make_pair(b, b ? lhs : 0);
}

#if defined(_MSC_VER) && _MSC_VER <= 1200
template<typename T> const T&
cmMaximum(const T& l, const T& r) {return l > r ? l : r;}
template<typename T> const T&
cmMinimum(const T& l, const T& r) {return l < r ? l : r;}
#else
#define cmMinimum std::min
#define cmMaximum std::max
#endif

//----------------------------------------------------------------------------
std::pair<bool, const char*> consistentNumberProperty(const char *lhs,
                                                      const char *rhs,
                                                      CompatibleType t)
{
  char *pEnd;

#if defined(_MSC_VER)
  static const char* const null_ptr = 0;
#else
# define null_ptr 0
#endif

  long lnum = strtol(lhs, &pEnd, 0);
  if (pEnd == lhs || *pEnd != '\0' || errno == ERANGE)
    {
    return std::pair<bool, const char*>(false, null_ptr);
    }

  long rnum = strtol(rhs, &pEnd, 0);
  if (pEnd == rhs || *pEnd != '\0' || errno == ERANGE)
    {
    return std::pair<bool, const char*>(false, null_ptr);
    }

#if !defined(_MSC_VER)
#undef null_ptr
#endif

  if (t == NumberMaxType)
    {
    return std::make_pair(true, cmMaximum(lnum, rnum) == lnum ? lhs : rhs);
    }
  else
    {
    return std::make_pair(true, cmMinimum(lnum, rnum) == lnum ? lhs : rhs);
    }
}

//----------------------------------------------------------------------------
template<>
std::pair<bool, const char*> consistentProperty(const char *lhs,
                                                const char *rhs,
                                                CompatibleType t)
{
  if (!lhs && !rhs)
    {
    return std::make_pair(true, lhs);
    }
  if (!lhs)
    {
    return std::make_pair(true, rhs);
    }
  if (!rhs)
    {
    return std::make_pair(true, lhs);
    }

#if defined(_MSC_VER)
  static const char* const null_ptr = 0;
#else
# define null_ptr 0
#endif

  switch(t)
  {
  case BoolType:
    assert(!"consistentProperty for strings called with BoolType");
    return std::pair<bool, const char*>(false, null_ptr);
  case StringType:
    return consistentStringProperty(lhs, rhs);
  case NumberMinType:
  case NumberMaxType:
    return consistentNumberProperty(lhs, rhs, t);
  }
  assert(!"Unreachable!");
  return std::pair<bool, const char*>(false, null_ptr);

#if !defined(_MSC_VER)
#undef null_ptr
#endif

}

template<typename PropertyType>
PropertyType impliedValue(PropertyType);
template<>
bool impliedValue<bool>(bool)
{
  return false;
}

template<>
const char* impliedValue<const char*>(const char*)
{
  return "";
}

template<typename PropertyType>
std::string valueAsString(PropertyType);
template<>
std::string valueAsString<bool>(bool value)
{
  return value ? "TRUE" : "FALSE";
}
template<>
std::string valueAsString<const char*>(const char* value)
{
  return value ? value : "(unset)";
}

//----------------------------------------------------------------------------
void
cmGeneratorTarget::ReportPropertyOrigin(const std::string &p,
                               const std::string &result,
                               const std::string &report,
                               const std::string &compatibilityType) const
{
  std::vector<std::string> debugProperties;
  const char *debugProp =
          this->Makefile->GetDefinition("CMAKE_DEBUG_TARGET_PROPERTIES");
  if (debugProp)
    {
    cmSystemTools::ExpandListArgument(debugProp, debugProperties);
    }

  bool debugOrigin = !this->DebugCompatiblePropertiesDone[p]
                    && std::find(debugProperties.begin(),
                                 debugProperties.end(),
                                 p)
                        != debugProperties.end();

  if (this->Makefile->IsGeneratingBuildSystem())
    {
    this->DebugCompatiblePropertiesDone[p] = true;
    }
  if (!debugOrigin)
    {
    return;
    }

  std::string areport = compatibilityType;
  areport += std::string(" of property \"") + p + "\" for target \"";
  areport += std::string(this->GetName());
  areport += "\" (result: \"";
  areport += result;
  areport += "\"):\n" + report;

  cmListFileBacktrace lfbt;
  this->Makefile->GetCMakeInstance()->IssueMessage(cmake::LOG, areport, lfbt);
}

//----------------------------------------------------------------------------
std::string compatibilityType(CompatibleType t)
{
  switch(t)
    {
    case BoolType:
      return "Boolean compatibility";
    case StringType:
      return "String compatibility";
    case NumberMaxType:
      return "Numeric maximum compatibility";
    case NumberMinType:
      return "Numeric minimum compatibility";
    }
  assert(!"Unreachable!");
  return "";
}

//----------------------------------------------------------------------------
std::string compatibilityAgree(CompatibleType t, bool dominant)
{
  switch(t)
    {
    case BoolType:
    case StringType:
      return dominant ? "(Disagree)\n" : "(Agree)\n";
    case NumberMaxType:
    case NumberMinType:
      return dominant ? "(Dominant)\n" : "(Ignored)\n";
    }
  assert(!"Unreachable!");
  return "";
}

//----------------------------------------------------------------------------
template<typename PropertyType>
PropertyType checkInterfacePropertyCompatibility(cmGeneratorTarget const* tgt,
                                          const std::string &p,
                                          const char *config,
                                          const char *defaultValue,
                                          CompatibleType t,
                                          PropertyType *)
{
  PropertyType propContent = getTypedProperty<PropertyType>(tgt->Target,
                                                            p.c_str(),
                                                            0);
  const bool explicitlySet = tgt->Target->GetProperties()
                                  .find(p.c_str())
                                  != tgt->Target->GetProperties().end();
  const bool impliedByUse =
          tgt->Target->IsNullImpliedByLinkLibraries(p);
  assert((impliedByUse ^ explicitlySet)
      || (!impliedByUse && !explicitlySet));

  cmComputeLinkInformation *info = tgt->GetLinkInformation(config);
  if(!info)
    {
    return propContent;
    }
  const cmComputeLinkInformation::ItemVector &deps = info->GetItems();
  bool propInitialized = explicitlySet;

  std::string report = " * Target \"";
  report += tgt->GetName();
  if (explicitlySet)
    {
    report += "\" has property content \"";
    report += valueAsString<PropertyType>(propContent);
    report += "\"\n";
    }
  else if (impliedByUse)
    {
    report += "\" property is implied by use.\n";
    }
  else
    {
    report += "\" property not set.\n";
    }

  for(cmComputeLinkInformation::ItemVector::const_iterator li =
      deps.begin();
      li != deps.end(); ++li)
    {
    // An error should be reported if one dependency
    // has INTERFACE_POSITION_INDEPENDENT_CODE ON and the other
    // has INTERFACE_POSITION_INDEPENDENT_CODE OFF, or if the
    // target itself has a POSITION_INDEPENDENT_CODE which disagrees
    // with a dependency.

    if (!li->Target)
      {
      continue;
      }

    const bool ifaceIsSet = li->Target->GetProperties()
                            .find("INTERFACE_" + p)
                            != li->Target->GetProperties().end();
    PropertyType ifacePropContent =
                    getTypedProperty<PropertyType>(li->Target,
                              ("INTERFACE_" + p).c_str(), 0);

    std::string reportEntry;
    if (ifaceIsSet)
      {
      reportEntry += " * Target \"";
      reportEntry += li->Target->GetName();
      reportEntry += "\" property value \"";
      reportEntry += valueAsString<PropertyType>(ifacePropContent);
      reportEntry += "\" ";
      }

    if (explicitlySet)
      {
      if (ifaceIsSet)
        {
        std::pair<bool, PropertyType> consistent =
                                  consistentProperty(propContent,
                                                     ifacePropContent, t);
        report += reportEntry;
        report += compatibilityAgree(t, propContent != consistent.second);
        if (!consistent.first)
          {
          cmOStringStream e;
          e << "Property " << p << " on target \""
            << tgt->GetName() << "\" does\nnot match the "
            "INTERFACE_" << p << " property requirement\nof "
            "dependency \"" << li->Target->GetName() << "\".\n";
          cmSystemTools::Error(e.str().c_str());
          break;
          }
        else
          {
          propContent = consistent.second;
          continue;
          }
        }
      else
        {
        // Explicitly set on target and not set in iface. Can't disagree.
        continue;
        }
      }
    else if (impliedByUse)
      {
      propContent = impliedValue<PropertyType>(propContent);

      if (ifaceIsSet)
        {
        std::pair<bool, PropertyType> consistent =
                                  consistentProperty(propContent,
                                                     ifacePropContent, t);
        report += reportEntry;
        report += compatibilityAgree(t, propContent != consistent.second);
        if (!consistent.first)
          {
          cmOStringStream e;
          e << "Property " << p << " on target \""
            << tgt->GetName() << "\" is\nimplied to be " << defaultValue
            << " because it was used to determine the link libraries\n"
               "already. The INTERFACE_" << p << " property on\ndependency \""
            << li->Target->GetName() << "\" is in conflict.\n";
          cmSystemTools::Error(e.str().c_str());
          break;
          }
        else
          {
          propContent = consistent.second;
          continue;
          }
        }
      else
        {
        // Implicitly set on target and not set in iface. Can't disagree.
        continue;
        }
      }
    else
      {
      if (ifaceIsSet)
        {
        if (propInitialized)
          {
          std::pair<bool, PropertyType> consistent =
                                    consistentProperty(propContent,
                                                       ifacePropContent, t);
          report += reportEntry;
          report += compatibilityAgree(t, propContent != consistent.second);
          if (!consistent.first)
            {
            cmOStringStream e;
            e << "The INTERFACE_" << p << " property of \""
              << li->Target->GetName() << "\" does\nnot agree with the value "
                "of " << p << " already determined\nfor \""
              << tgt->GetName() << "\".\n";
            cmSystemTools::Error(e.str().c_str());
            break;
            }
          else
            {
            propContent = consistent.second;
            continue;
            }
          }
        else
          {
          report += reportEntry + "(Interface set)\n";
          propContent = ifacePropContent;
          propInitialized = true;
          }
        }
      else
        {
        // Not set. Nothing to agree on.
        continue;
        }
      }
    }

  tgt->ReportPropertyOrigin(p, valueAsString<PropertyType>(propContent),
                            report, compatibilityType(t));
  return propContent;
}

//----------------------------------------------------------------------------
bool
cmGeneratorTarget::GetLinkInterfaceDependentBoolProperty(const std::string &p,
                                                     const char *config) const
{
  return checkInterfacePropertyCompatibility<bool>(this, p, config, "FALSE",
                                                   BoolType, 0);
}

//----------------------------------------------------------------------------
const char * cmGeneratorTarget::GetLinkInterfaceDependentStringProperty(
                                                      const std::string &p,
                                                      const char *config) const
{
  return checkInterfacePropertyCompatibility<const char *>(this,
                                                           p,
                                                           config,
                                                           "empty",
                                                           StringType, 0);
}

//----------------------------------------------------------------------------
const char * cmGeneratorTarget::GetLinkInterfaceDependentNumberMinProperty(
                                                      const std::string &p,
                                                      const char *config) const
{
  return checkInterfacePropertyCompatibility<const char *>(this,
                                                           p,
                                                           config,
                                                           "empty",
                                                           NumberMinType, 0);
}

//----------------------------------------------------------------------------
const char * cmGeneratorTarget::GetLinkInterfaceDependentNumberMaxProperty(
                                                      const std::string &p,
                                                      const char *config) const
{
  return checkInterfacePropertyCompatibility<const char *>(this,
                                                           p,
                                                           config,
                                                           "empty",
                                                           NumberMaxType, 0);
}

//----------------------------------------------------------------------------
bool isLinkDependentProperty(cmGeneratorTarget const* tgt,
                             const std::string &p,
                             const char *interfaceProperty,
                             const char *config)
{
  cmComputeLinkInformation *info = tgt->GetLinkInformation(config);
  if(!info)
    {
    return false;
    }

  const cmComputeLinkInformation::ItemVector &deps = info->GetItems();

  for(cmComputeLinkInformation::ItemVector::const_iterator li =
      deps.begin();
      li != deps.end(); ++li)
    {
    if (!li->Target)
      {
      continue;
      }
    const char *prop = li->Target->GetProperty(interfaceProperty);
    if (!prop)
      {
      continue;
      }

    std::vector<std::string> props;
    cmSystemTools::ExpandListArgument(prop, props);

    for(std::vector<std::string>::iterator pi = props.begin();
        pi != props.end(); ++pi)
      {
      if (*pi == p)
        {
        return true;
        }
      }
    }

  return false;
}

//----------------------------------------------------------------------------
bool
cmGeneratorTarget::IsLinkInterfaceDependentBoolProperty(const std::string &p,
                                           const char *config) const
{
  if (this->GetType() == cmTarget::OBJECT_LIBRARY
      || this->GetType() == cmTarget::INTERFACE_LIBRARY)
    {
    return false;
    }
  return (p == "POSITION_INDEPENDENT_CODE") ||
    isLinkDependentProperty(this, p, "COMPATIBLE_INTERFACE_BOOL",
                                 config);
}

//----------------------------------------------------------------------------
bool
cmGeneratorTarget::IsLinkInterfaceDependentStringProperty(const std::string &p,
                                    const char *config) const
{
  if (this->GetType() == cmTarget::OBJECT_LIBRARY
      || this->GetType() == cmTarget::INTERFACE_LIBRARY)
    {
    return false;
    }
  return isLinkDependentProperty(this, p, "COMPATIBLE_INTERFACE_STRING",
                                 config);
}

//----------------------------------------------------------------------------
bool cmGeneratorTarget::IsLinkInterfaceDependentNumberMinProperty(
                                    const std::string &p,
                                    const char *config) const
{
  if (this->GetType() == cmTarget::OBJECT_LIBRARY
      || this->GetType() == cmTarget::INTERFACE_LIBRARY)
    {
    return false;
    }
  return isLinkDependentProperty(this, p, "COMPATIBLE_INTERFACE_NUMBER_MIN",
                                 config);
}

//----------------------------------------------------------------------------
bool cmGeneratorTarget::IsLinkInterfaceDependentNumberMaxProperty(
                                    const std::string &p,
                                    const char *config) const
{
  if (this->GetType() == cmTarget::OBJECT_LIBRARY
      || this->GetType() == cmTarget::INTERFACE_LIBRARY)
    {
    return false;
    }
  return isLinkDependentProperty(this, p, "COMPATIBLE_INTERFACE_NUMBER_MAX",
                                 config);
}

template<typename PropertyType>
PropertyType getLinkInterfaceDependentProperty(cmGeneratorTarget const* tgt,
                                               const std::string prop,
                                               const char *config,
                                               CompatibleType,
                                               PropertyType *);

template<>
bool getLinkInterfaceDependentProperty(cmGeneratorTarget const* tgt,
                                       const std::string prop,
                                       const char *config,
                                       CompatibleType, bool *)
{
  return tgt->GetLinkInterfaceDependentBoolProperty(prop, config);
}

template<>
const char * getLinkInterfaceDependentProperty(cmGeneratorTarget const* tgt,
                                               const std::string prop,
                                               const char *config,
                                               CompatibleType t,
                                               const char **)
{
  switch(t)
  {
  case BoolType:
    assert(!"String compatibility check function called for boolean");
    return 0;
  case StringType:
    return tgt->GetLinkInterfaceDependentStringProperty(prop, config);
  case NumberMinType:
    return tgt->GetLinkInterfaceDependentNumberMinProperty(prop, config);
  case NumberMaxType:
    return tgt->GetLinkInterfaceDependentNumberMaxProperty(prop, config);
  }
  assert(!"Unreachable!");
  return 0;
}

//----------------------------------------------------------------------------
template<typename PropertyType>
void checkPropertyConsistency(cmGeneratorTarget const* depender,
                              cmTarget const* dependee,
                              const char *propName,
                              std::set<cmStdString> &emitted,
                              const char *config,
                              CompatibleType t,
                              PropertyType *)
{
  const char *prop = dependee->GetProperty(propName);
  if (!prop)
    {
    return;
    }

  std::vector<std::string> props;
  cmSystemTools::ExpandListArgument(prop, props);
  std::string pdir =
    dependee->GetMakefile()->GetRequiredDefinition("CMAKE_ROOT");
  pdir += "/Help/prop_tgt/";

  for(std::vector<std::string>::iterator pi = props.begin();
      pi != props.end(); ++pi)
    {
    std::string pname = cmSystemTools::HelpFileName(*pi);
    std::string pfile = pdir + pname + ".rst";
    if(cmSystemTools::FileExists(pfile.c_str(), true))
      {
      cmOStringStream e;
      e << "Target \"" << dependee->GetName() << "\" has property \""
        << *pi << "\" listed in its " << propName << " property.  "
          "This is not allowed.  Only user-defined properties may appear "
          "listed in the " << propName << " property.";
      depender->GetMakefile()->IssueMessage(cmake::FATAL_ERROR, e.str());
      return;
      }
    if(emitted.insert(*pi).second)
      {
      getLinkInterfaceDependentProperty<PropertyType>(depender, *pi, config,
                                                      t, 0);
      if (cmSystemTools::GetErrorOccuredFlag())
        {
        return;
        }
      }
    }
}

cmMakefile* cmGeneratorTarget::GetMakefile() const
{
  return this->Target->GetMakefile();
}

static cmStdString intersect(const std::set<cmStdString> &s1,
                             const std::set<cmStdString> &s2)
{
  std::set<cmStdString> intersect;
  std::set_intersection(s1.begin(),s1.end(),
                        s2.begin(),s2.end(),
                      std::inserter(intersect,intersect.begin()));
  if (!intersect.empty())
    {
    return *intersect.begin();
    }
  return "";
}
static cmStdString intersect(const std::set<cmStdString> &s1,
                       const std::set<cmStdString> &s2,
                       const std::set<cmStdString> &s3)
{
  cmStdString result;
  result = intersect(s1, s2);
  if (!result.empty())
    return result;
  result = intersect(s1, s3);
  if (!result.empty())
    return result;
  return intersect(s2, s3);
}
static cmStdString intersect(const std::set<cmStdString> &s1,
                       const std::set<cmStdString> &s2,
                       const std::set<cmStdString> &s3,
                       const std::set<cmStdString> &s4)
{
  cmStdString result;
  result = intersect(s1, s2);
  if (!result.empty())
    return result;
  result = intersect(s1, s3);
  if (!result.empty())
    return result;
  result = intersect(s1, s4);
  if (!result.empty())
    return result;
  return intersect(s2, s3, s4);
}

//----------------------------------------------------------------------------
void
cmGeneratorTarget::CheckPropertyCompatibility(cmComputeLinkInformation *info,
                                              const char* config) const
{
  const cmComputeLinkInformation::ItemVector &deps = info->GetItems();

  std::set<cmStdString> emittedBools;
  std::set<cmStdString> emittedStrings;
  std::set<cmStdString> emittedMinNumbers;
  std::set<cmStdString> emittedMaxNumbers;

  for(cmComputeLinkInformation::ItemVector::const_iterator li =
      deps.begin();
      li != deps.end(); ++li)
    {
    if (!li->Target)
      {
      continue;
      }

    checkPropertyConsistency<bool>(this, li->Target,
                                   "COMPATIBLE_INTERFACE_BOOL",
                                   emittedBools, config, BoolType, 0);
    if (cmSystemTools::GetErrorOccuredFlag())
      {
      return;
      }
    checkPropertyConsistency<const char *>(this, li->Target,
                                           "COMPATIBLE_INTERFACE_STRING",
                                           emittedStrings, config,
                                           StringType, 0);
    if (cmSystemTools::GetErrorOccuredFlag())
      {
      return;
      }
    checkPropertyConsistency<const char *>(this, li->Target,
                                           "COMPATIBLE_INTERFACE_NUMBER_MIN",
                                           emittedMinNumbers, config,
                                           NumberMinType, 0);
    if (cmSystemTools::GetErrorOccuredFlag())
      {
      return;
      }
    checkPropertyConsistency<const char *>(this, li->Target,
                                           "COMPATIBLE_INTERFACE_NUMBER_MAX",
                                           emittedMaxNumbers, config,
                                           NumberMaxType, 0);
    if (cmSystemTools::GetErrorOccuredFlag())
      {
      return;
      }
    }

  std::string prop = intersect(emittedBools,
                               emittedStrings,
                               emittedMinNumbers,
                               emittedMaxNumbers);

  if (!prop.empty())
    {
    std::set<std::string> props;
    std::set<cmStdString>::const_iterator i = emittedBools.find(prop);
    if (i != emittedBools.end())
      {
      props.insert("COMPATIBLE_INTERFACE_BOOL");
      }
    i = emittedStrings.find(prop);
    if (i != emittedStrings.end())
      {
      props.insert("COMPATIBLE_INTERFACE_STRING");
      }
    i = emittedMinNumbers.find(prop);
    if (i != emittedMinNumbers.end())
      {
      props.insert("COMPATIBLE_INTERFACE_NUMBER_MIN");
      }
    i = emittedMaxNumbers.find(prop);
    if (i != emittedMaxNumbers.end())
      {
      props.insert("COMPATIBLE_INTERFACE_NUMBER_MAX");
      }

    std::string propsString = *props.begin();
    props.erase(props.begin());
    while (props.size() > 1)
      {
      propsString += ", " + *props.begin();
      props.erase(props.begin());
      }
   if (props.size() == 1)
     {
     propsString += " and the " + *props.begin();
     }
    cmOStringStream e;
    e << "Property \"" << prop << "\" appears in both the "
      << propsString <<
    " property in the dependencies of target \"" << this->GetName() <<
    "\".  This is not allowed. A property may only require compatibility "
    "in a boolean interpretation, a numeric minimum, a numeric maximum or a "
    "string interpretation, but not a mixture.";
    this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
    }
}

//----------------------------------------------------------------------------
cmComputeLinkInformation*
cmGeneratorTarget::GetLinkInformation(const char* config,
                                      cmTarget const* head) const
{
  cmTarget const* headTarget = head ? head : this->Target;
  // Lookup any existing information for this configuration.
  TargetConfigPair key(headTarget,
                                  cmSystemTools::UpperCase(config?config:""));
  cmTargetLinkInformationMap::iterator
    i = this->LinkInformation.find(key);
  if(i == this->LinkInformation.end())
    {
    // Compute information for this configuration.
    cmComputeLinkInformation* info =
      new cmComputeLinkInformation(this, config, headTarget);
    if(!info || !info->Compute())
      {
      delete info;
      info = 0;
      }

    // Store the information for this configuration.
    cmTargetLinkInformationMap::value_type entry(key, info);
    i = this->LinkInformation.insert(entry).first;

    if (info)
      {
      this->CheckPropertyCompatibility(info, config);
      }
    }
  return i->second;
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::GetAutoUicOptions(std::vector<std::string> &result,
                                 const char *config) const
{
  const char *prop
            = this->GetLinkInterfaceDependentStringProperty("AUTOUIC_OPTIONS",
                                                            config);
  if (!prop)
    {
    return;
    }
  cmListFileBacktrace lfbt;
  cmGeneratorExpression ge(lfbt);

  cmGeneratorExpressionDAGChecker dagChecker(lfbt,
                                      this->GetName(),
                                      "AUTOUIC_OPTIONS", 0, 0);
  cmSystemTools::ExpandListArgument(ge.Parse(prop)
                                      ->Evaluate(this->Makefile,
                                                config,
                                                false,
                                                this->Target,
                                                &dagChecker),
                                  result);
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
  std::vector<cmSourceFile*> sources;
  this->Target->GetSourceFiles(sources);
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
  if(cmGeneratorTarget* t
                    = this->Makefile->FindGeneratorTargetToUse(util.c_str()))
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
                             std::vector<std::string>& archVec) const
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
const char* cmGeneratorTarget::GetCreateRuleVariable() const
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
static void processIncludeDirectories(cmTarget const* tgt,
      const std::vector<cmGeneratorTarget::TargetPropertyEntry*> &entries,
      std::vector<std::string> &includes,
      std::set<std::string> &uniqueIncludes,
      cmGeneratorExpressionDAGChecker *dagChecker,
      const char *config, bool debugIncludes)
{
  cmMakefile *mf = tgt->GetMakefile();

  for (std::vector<cmGeneratorTarget::TargetPropertyEntry*>::const_iterator
      it = entries.begin(), end = entries.end(); it != end; ++it)
    {
    bool testIsOff = true;
    bool cacheIncludes = false;
    std::vector<std::string> entryIncludes = (*it)->CachedEntries;
    if(!entryIncludes.empty())
      {
      testIsOff = false;
      }
    else
      {
      cmSystemTools::ExpandListArgument((*it)->ge->Evaluate(mf,
                                                config,
                                                false,
                                                tgt,
                                                dagChecker),
                                      entryIncludes);
      if (mf->IsGeneratingBuildSystem()
          && !(*it)->ge->GetHadContextSensitiveCondition())
        {
        cacheIncludes = true;
        }
      }
    std::string usedIncludes;
    cmListFileBacktrace lfbt;
    for(std::vector<std::string>::iterator
          li = entryIncludes.begin(); li != entryIncludes.end(); ++li)
      {
      std::string targetName = (*it)->TargetName;
      std::string evaluatedTargetName;
      {
      cmGeneratorExpression ge(lfbt);
      cmsys::auto_ptr<cmCompiledGeneratorExpression> cge =
                                                        ge.Parse(targetName);
      evaluatedTargetName = cge->Evaluate(mf, config, false, tgt, 0, 0);
      }

      cmTarget *dependentTarget = mf->FindTargetToUse(targetName.c_str());

      const bool fromImported = dependentTarget
                             && dependentTarget->IsImported();

      cmTarget *evaluatedDependentTarget =
        (targetName != evaluatedTargetName)
          ? mf->FindTargetToUse(evaluatedTargetName.c_str())
          : 0;

      targetName = evaluatedTargetName;

      const bool fromEvaluatedImported = evaluatedDependentTarget
                             && evaluatedDependentTarget->IsImported();

      if ((fromImported || fromEvaluatedImported)
          && !cmSystemTools::FileExists(li->c_str()))
        {
        cmOStringStream e;
        cmake::MessageType messageType = cmake::FATAL_ERROR;
        if (fromEvaluatedImported)
          {
          switch(mf->GetPolicyStatus(cmPolicies::CMP0027))
            {
            case cmPolicies::WARN:
              e << (mf->GetPolicies()
                    ->GetPolicyWarning(cmPolicies::CMP0027)) << "\n";
            case cmPolicies::OLD:
              messageType = cmake::AUTHOR_WARNING;
              break;
            case cmPolicies::REQUIRED_ALWAYS:
            case cmPolicies::REQUIRED_IF_USED:
            case cmPolicies::NEW:
              break;
            }
          }
        e << "Imported target \"" << targetName << "\" includes "
             "non-existent path\n  \"" << *li << "\"\nin its "
             "INTERFACE_INCLUDE_DIRECTORIES. Possible reasons include:\n"
             "* The path was deleted, renamed, or moved to another "
             "location.\n"
             "* An install or uninstall procedure did not complete "
             "successfully.\n"
             "* The installation package was faulty and references files it "
             "does not provide.\n";
        tgt->GetMakefile()->IssueMessage(messageType, e.str().c_str());
        return;
        }

      if (!cmSystemTools::FileIsFullPath(li->c_str()))
        {
        cmOStringStream e;
        bool noMessage = false;
        cmake::MessageType messageType = cmake::FATAL_ERROR;
        if (!targetName.empty())
          {
          e << "Target \"" << targetName << "\" contains relative "
            "path in its INTERFACE_INCLUDE_DIRECTORIES:\n"
            "  \"" << *li << "\"";
          }
        else
          {
          switch(tgt->GetPolicyStatusCMP0021())
            {
            case cmPolicies::WARN:
              {
              e << (mf->GetPolicies()
                    ->GetPolicyWarning(cmPolicies::CMP0021)) << "\n";
              messageType = cmake::AUTHOR_WARNING;
              }
              break;
            case cmPolicies::OLD:
              noMessage = true;
            case cmPolicies::REQUIRED_IF_USED:
            case cmPolicies::REQUIRED_ALWAYS:
            case cmPolicies::NEW:
              // Issue the fatal message.
              break;
            }
          e << "Found relative path while evaluating include directories of "
          "\"" << tgt->GetName() << "\":\n  \"" << *li << "\"\n";
          }
        if (!noMessage)
          {
          tgt->GetMakefile()->IssueMessage(messageType, e.str().c_str());
          if (messageType == cmake::FATAL_ERROR)
            {
            return;
            }
          }
        }

      if (testIsOff && !cmSystemTools::IsOff(li->c_str()))
        {
        cmSystemTools::ConvertToUnixSlashes(*li);
        }
      std::string inc = *li;

      if(uniqueIncludes.insert(inc).second)
        {
        includes.push_back(inc);
        if (debugIncludes)
          {
          usedIncludes += " * " + inc + "\n";
          }
        }
      }
    if (cacheIncludes)
      {
      (*it)->CachedEntries = entryIncludes;
      }
    if (!usedIncludes.empty())
      {
      mf->GetCMakeInstance()->IssueMessage(cmake::LOG,
                            std::string("Used includes for target ")
                            + tgt->GetName() + ":\n"
                            + usedIncludes, (*it)->ge->GetBacktrace());
      }
    }
}

//----------------------------------------------------------------------------
std::vector<std::string>
cmGeneratorTarget::GetIncludeDirectories(const char *config) const
{
  std::vector<std::string> includes;
  std::set<std::string> uniqueIncludes;
  cmListFileBacktrace lfbt;

  cmGeneratorExpressionDAGChecker dagChecker(lfbt,
                                             this->GetName(),
                                             "INCLUDE_DIRECTORIES", 0, 0);

  std::vector<std::string> debugProperties;
  const char *debugProp =
              this->Makefile->GetDefinition("CMAKE_DEBUG_TARGET_PROPERTIES");
  if (debugProp)
    {
    cmSystemTools::ExpandListArgument(debugProp, debugProperties);
    }

  bool debugIncludes = !this->DebugIncludesDone
                    && std::find(debugProperties.begin(),
                                 debugProperties.end(),
                                 "INCLUDE_DIRECTORIES")
                        != debugProperties.end();

  if (this->Makefile->IsGeneratingBuildSystem())
    {
    this->DebugIncludesDone = true;
    }

  processIncludeDirectories(this->Target,
                            this->Target->GetIncludeDirectoriesEntries(),
                            includes,
                            uniqueIncludes,
                            &dagChecker,
                            config,
                            debugIncludes);

  std::string configString = config ? config : "";
  if (!this->CacheLinkInterfaceIncludeDirectoriesDone[configString])
    {
    for (std::vector<cmValueWithOrigin>::const_iterator
        it = this->Target->GetLinkImplementationPropertyEntries().begin(),
        end = this->Target->GetLinkImplementationPropertyEntries().end();
        it != end; ++it)
      {
      if (!cmGeneratorExpression::IsValidTargetName(it->Value)
          && cmGeneratorExpression::Find(it->Value) == std::string::npos)
        {
        continue;
        }
      {
      cmGeneratorExpression ge(lfbt);
      cmsys::auto_ptr<cmCompiledGeneratorExpression> cge =
                                                        ge.Parse(it->Value);
      std::string result = cge->Evaluate(this->Makefile, config,
                                        false, this->Target, 0, 0);
      if (!this->Makefile->FindTargetToUse(result.c_str()))
        {
        continue;
        }
      }
      std::string includeGenex = "$<TARGET_PROPERTY:" +
                              it->Value + ",INTERFACE_INCLUDE_DIRECTORIES>";
      if (cmGeneratorExpression::Find(it->Value) != std::string::npos)
        {
        // Because it->Value is a generator expression, ensure that it
        // evaluates to the non-empty string before being used in the
        // TARGET_PROPERTY expression.
        includeGenex = "$<$<BOOL:" + it->Value + ">:" + includeGenex + ">";
        }
      cmGeneratorExpression ge(it->Backtrace);
      cmsys::auto_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(
                                                              includeGenex);

      this
        ->CachedLinkInterfaceIncludeDirectoriesEntries[configString].push_back(
                        new cmGeneratorTarget::TargetPropertyEntry(cge,
                                                              it->Value));
      }

    if(this->Makefile->IsOn("APPLE"))
      {
      LinkImplementation const* impl =
                     this->GetLinkImplementation(config, this->Target);
      for(std::vector<std::string>::const_iterator
          it = impl->Libraries.begin();
          it != impl->Libraries.end(); ++it)
        {
        std::string libDir = cmSystemTools::CollapseFullPath(it->c_str());

        static cmsys::RegularExpression
          frameworkCheck("(.*\\.framework)(/Versions/[^/]+)?/[^/]+$");
        if(!frameworkCheck.find(libDir))
          {
          continue;
          }

        libDir = frameworkCheck.match(1);

        cmGeneratorExpression ge(lfbt);
        cmsys::auto_ptr<cmCompiledGeneratorExpression> cge =
                  ge.Parse(libDir.c_str());
        this->CachedLinkInterfaceIncludeDirectoriesEntries[configString]
                .push_back(new cmGeneratorTarget::TargetPropertyEntry(cge));
        }
      }
    }

  processIncludeDirectories(this->Target,
    this->CachedLinkInterfaceIncludeDirectoriesEntries[configString],
                            includes,
                            uniqueIncludes,
                            &dagChecker,
                            config,
                            debugIncludes);

  if (!this->Makefile->IsGeneratingBuildSystem())
    {
    deleteAndClear2(
                this->CachedLinkInterfaceIncludeDirectoriesEntries);
    }
  else
    {
    this->CacheLinkInterfaceIncludeDirectoriesDone[configString]
                                                                      = true;
    }

  return includes;
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::GenerateTargetManifest(const char* config) const
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
    this->GetExecutableNames(name, realName, impName, pdbName,
                                     config);
    }
  else if(this->GetType() == cmTarget::STATIC_LIBRARY ||
          this->GetType() == cmTarget::SHARED_LIBRARY ||
          this->GetType() == cmTarget::MODULE_LIBRARY)
    {
    this->GetLibraryNames(name, soName, realName, impName, pdbName,
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

//----------------------------------------------------------------------------
std::string cmGeneratorTarget::GetFullPath(const char* config,
                                           bool implib, bool realname) const
{
  if(this->Target->IsImported())
    {
    return this->Target->ImportedGetFullPath(config, implib);
    }
  else
    {
    return this->NormalGetFullPath(config, implib, realname);
    }
}

std::string cmGeneratorTarget::NormalGetFullPath(const char* config,
                                                 bool implib,
                                                 bool realname) const
{
  std::string fpath = this->Target->GetDirectory(config, implib);
  fpath += "/";
  if(this->Target->IsAppBundleOnApple())
    {
    fpath = this->BuildMacContentDirectory(fpath, config, false);
    fpath += "/";
    }

  // Add the full name of the target.
  if(implib)
    {
    fpath += this->GetFullName(config, true);
    }
  else if(realname)
    {
    fpath += this->NormalGetRealName(config);
    }
  else
    {
    fpath += this->GetFullName(config, false);
    }
  return fpath;
}

//----------------------------------------------------------------------------
std::string cmGeneratorTarget::NormalGetRealName(const char* config) const
{
  // This should not be called for imported targets.
  // TODO: Split cmTarget into a class hierarchy to get compile-time
  // enforcement of the limited imported target API.
  if(this->Target->IsImported())
    {
    std::string msg =  "NormalGetRealName called on imported target: ";
    msg += this->GetName();
    this->Makefile->
      IssueMessage(cmake::INTERNAL_ERROR,
                   msg.c_str());
    }

  if(this->GetType() == cmTarget::EXECUTABLE)
    {
    // Compute the real name that will be built.
    std::string name;
    std::string realName;
    std::string impName;
    std::string pdbName;
    this->GetExecutableNames(name, realName, impName, pdbName, config);
    return realName;
    }
  else
    {
    // Compute the real name that will be built.
    std::string name;
    std::string soName;
    std::string realName;
    std::string impName;
    std::string pdbName;
    this->GetLibraryNames(name, soName, realName,
                          impName, pdbName, config);
    return realName;
    }
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::GetLibraryNames(std::string& name,
                               std::string& soName,
                               std::string& realName,
                               std::string& impName,
                               std::string& pdbName,
                               const char* config) const
{
  // This should not be called for imported targets.
  // TODO: Split cmTarget into a class hierarchy to get compile-time
  // enforcement of the limited imported target API.
  if(this->Target->IsImported())
    {
    std::string msg =  "GetLibraryNames called on imported target: ";
    msg += this->GetName();
    this->Makefile->IssueMessage(cmake::INTERNAL_ERROR,
                                 msg.c_str());
    return;
    }

  // Check for library version properties.
  const char* version = this->GetProperty("VERSION");
  const char* soversion = this->GetProperty("SOVERSION");
  if(!this->HasSOName(config) ||
     this->Target->IsFrameworkOnApple())
    {
    // Versioning is supported only for shared libraries and modules,
    // and then only when the platform supports an soname flag.
    version = 0;
    soversion = 0;
    }
  if(version && !soversion)
    {
    // The soversion must be set if the library version is set.  Use
    // the library version as the soversion.
    soversion = version;
    }
  if(!version && soversion)
    {
    // Use the soversion as the library version.
    version = soversion;
    }

  // Get the components of the library name.
  std::string prefix;
  std::string base;
  std::string suffix;
  this->GetFullNameInternal(config, false, prefix, base, suffix);

  // The library name.
  name = prefix+base+suffix;

  if(this->Target->IsFrameworkOnApple())
    {
    realName = prefix;
    realName += "Versions/";
    realName += this->Target->GetFrameworkVersion();
    realName += "/";
    realName += base;
    soName = realName;
    }
  else
  {
    // The library's soname.
    this->ComputeVersionedName(soName, prefix, base, suffix,
                              name, soversion);

    // The library's real name on disk.
    this->ComputeVersionedName(realName, prefix, base, suffix,
                              name, version);
  }

  // The import library name.
  if(this->GetType() == cmTarget::SHARED_LIBRARY ||
     this->GetType() == cmTarget::MODULE_LIBRARY)
    {
    impName = this->GetFullNameInternal(config, true);
    }
  else
    {
    impName = "";
    }

  // The program database file name.
  pdbName = this->GetPDBName(config);
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::GetExecutableNames(std::string& name,
                                  std::string& realName,
                                  std::string& impName,
                                  std::string& pdbName,
                                  const char* config) const
{
  // This should not be called for imported targets.
  // TODO: Split cmTarget into a class hierarchy to get compile-time
  // enforcement of the limited imported target API.
  if(this->Target->IsImported())
    {
    std::string msg =
      "GetExecutableNames called on imported target: ";
    msg += this->GetName();
    this->GetMakefile()->IssueMessage(cmake::INTERNAL_ERROR, msg.c_str());
    }

  // This versioning is supported only for executables and then only
  // when the platform supports symbolic links.
#if defined(_WIN32) && !defined(__CYGWIN__)
  const char* version = 0;
#else
  // Check for executable version properties.
  const char* version = this->GetProperty("VERSION");
  if(this->GetType() != cmTarget::EXECUTABLE || this->Makefile->IsOn("XCODE"))
    {
    version = 0;
    }
#endif

  // Get the components of the executable name.
  std::string prefix;
  std::string base;
  std::string suffix;
  this->GetFullNameInternal(config, false, prefix, base, suffix);

  // The executable name.
  name = prefix+base+suffix;

  // The executable's real name on disk.
#if defined(__CYGWIN__)
  realName = prefix+base;
#else
  realName = name;
#endif
  if(version)
    {
    realName += "-";
    realName += version;
    }
#if defined(__CYGWIN__)
  realName += suffix;
#endif

  // The import library name.
  impName = this->GetFullNameInternal(config, true);

  // The program database file name.
  pdbName = this->GetPDBName(config);
}

//----------------------------------------------------------------------------
cmGeneratorTarget::LinkInterface const*
cmGeneratorTarget::GetLinkInterface(const char* config,
                                    cmTarget const* head) const
{
  // Imported targets have their own link interface.
  if(this->Target->IsImported())
    {
    if(cmTarget::ImportInfo const* info = this->Target->GetImportInfo(config,
                                                                      head))
      {
      return &info->LinkInterface;
      }
    return 0;
    }

  // Link interfaces are not supported for executables that do not
  // export symbols.
  if(this->GetType() == cmTarget::EXECUTABLE &&
     !this->Target->IsExecutableWithExports())
    {
    return 0;
    }

  // Lookup any existing link interface for this configuration.
  TargetConfigPair key(head, cmSystemTools::UpperCase(config? config : ""));
  LinkInterfaceMapType::iterator
    i = this->LinkInterfaceMap.find(key);
  if(i == this->LinkInterfaceMap.end())
    {
    // Compute the link interface for this configuration.
    OptionalLinkInterface iface;
    iface.Exists = this->ComputeLinkInterface(config, iface, head);

    // Store the information for this configuration.
    LinkInterfaceMapType::value_type entry(key, iface);
    i = this->LinkInterfaceMap.insert(entry).first;
    }

  return i->second.Exists? &i->second : 0;
}

//----------------------------------------------------------------------------
bool cmGeneratorTarget::ComputeLinkInterface(const char* config,
                                             LinkInterface& iface,
                                             cmTarget const *headTarget) const
{
  // Construct the property name suffix for this configuration.
  std::string suffix = "_";
  if(config && *config)
    {
    suffix += cmSystemTools::UpperCase(config);
    }
  else
    {
    suffix += "NOCONFIG";
    }

  // An explicit list of interface libraries may be set for shared
  // libraries and executables that export symbols.
  const char* explicitLibraries = 0;
  std::string linkIfaceProp;
  if(this->Target->PolicyStatusCMP0022 != cmPolicies::OLD &&
     this->Target->PolicyStatusCMP0022 != cmPolicies::WARN)
    {
    // CMP0022 NEW behavior is to use INTERFACE_LINK_LIBRARIES.
    linkIfaceProp = "INTERFACE_LINK_LIBRARIES";
    explicitLibraries = this->GetProperty(linkIfaceProp.c_str());
    }
  else if(this->GetType() == cmTarget::SHARED_LIBRARY ||
          this->Target->IsExecutableWithExports())
    {
    // CMP0022 OLD behavior is to use LINK_INTERFACE_LIBRARIES if set on a
    // shared lib or executable.

    // Lookup the per-configuration property.
    linkIfaceProp = "LINK_INTERFACE_LIBRARIES";
    linkIfaceProp += suffix;
    explicitLibraries = this->GetProperty(linkIfaceProp.c_str());

    // If not set, try the generic property.
    if(!explicitLibraries)
      {
      linkIfaceProp = "LINK_INTERFACE_LIBRARIES";
      explicitLibraries = this->GetProperty(linkIfaceProp.c_str());
      }
    }

  if(explicitLibraries
      && this->Target->GetPolicyStatusCMP0022() == cmPolicies::WARN
      && !this->PolicyWarnedCMP0022)
    {
    // Compare the explicitly set old link interface properties to the
    // preferred new link interface property one and warn if different.
    const char* newExplicitLibraries =
      this->GetProperty("INTERFACE_LINK_LIBRARIES");
    if (newExplicitLibraries
        && strcmp(newExplicitLibraries, explicitLibraries) != 0)
      {
      cmOStringStream w;
      w <<
        (this->Makefile->GetPolicies()
         ->GetPolicyWarning(cmPolicies::CMP0022)) << "\n"
        "Target \"" << this->GetName() << "\" has an "
        "INTERFACE_LINK_LIBRARIES property which differs from its " <<
        linkIfaceProp << " properties."
        "\n"
        "INTERFACE_LINK_LIBRARIES:\n"
        "  " << newExplicitLibraries << "\n" <<
        linkIfaceProp << ":\n"
        "  " << (explicitLibraries ? explicitLibraries : "(empty)") << "\n";
      this->Makefile->IssueMessage(cmake::AUTHOR_WARNING, w.str());
      this->PolicyWarnedCMP0022 = true;
      }
    }

  // There is no implicit link interface for executables or modules
  // so if none was explicitly set then there is no link interface.
  if(!explicitLibraries &&
     (this->GetType() == cmTarget::EXECUTABLE ||
      (this->GetType() == cmTarget::MODULE_LIBRARY)))
    {
    return false;
    }

  if(explicitLibraries)
    {
    // The interface libraries have been explicitly set.
    cmListFileBacktrace lfbt;
    cmGeneratorExpression ge(lfbt);
    cmGeneratorExpressionDAGChecker dagChecker(lfbt, this->GetName(),
                                               linkIfaceProp, 0, 0);
    cmSystemTools::ExpandListArgument(ge.Parse(explicitLibraries)->Evaluate(
                                        this->Makefile,
                                        config,
                                        false,
                                        headTarget,
                                        this->Target, &dagChecker),
                                      iface.Libraries);

    if(this->GetType() == cmTarget::SHARED_LIBRARY
        || this->GetType() == cmTarget::STATIC_LIBRARY
        || this->GetType() == cmTarget::INTERFACE_LIBRARY)
      {
      // Shared libraries may have runtime implementation dependencies
      // on other shared libraries that are not in the interface.
      std::set<cmStdString> emitted;
      for(std::vector<std::string>::const_iterator
            li = iface.Libraries.begin(); li != iface.Libraries.end(); ++li)
        {
        emitted.insert(*li);
        }
      if (this->GetType() != cmTarget::INTERFACE_LIBRARY)
        {
        LinkImplementation const* impl =
                              this->GetLinkImplementation(config, headTarget);
        for(std::vector<std::string>::const_iterator
              li = impl->Libraries.begin(); li != impl->Libraries.end(); ++li)
          {
          if(emitted.insert(*li).second)
            {
            if(cmTarget* tgt = this->Makefile->FindTargetToUse(li->c_str()))
              {
              // This is a runtime dependency on another shared library.
              if(tgt->GetType() == cmTarget::SHARED_LIBRARY)
                {
                iface.SharedDeps.push_back(*li);
                }
              }
            else
              {
              // TODO: Recognize shared library file names.  Perhaps this
              // should be moved to cmComputeLinkInformation, but that creates
              // a chicken-and-egg problem since this list is needed for its
              // construction.
              }
            }
          }
        if(this->Target->LinkLanguagePropagatesToDependents())
          {
          // Targets using this archive need its language runtime libraries.
          iface.Languages = impl->Languages;
          }
        }
      }
    }
  else if (this->Target->GetPolicyStatusCMP0022() == cmPolicies::WARN
        || this->Target->GetPolicyStatusCMP0022() == cmPolicies::OLD)
    // If CMP0022 is NEW then the plain tll signature sets the
    // INTERFACE_LINK_LIBRARIES, so if we get here then the project
    // cleared the property explicitly and we should not fall back
    // to the link implementation.
    {
    // The link implementation is the default link interface.
    LinkImplementation const* impl =
                              this->GetLinkImplementation(config, headTarget);
    iface.ImplementationIsInterface = true;
    iface.Libraries = impl->Libraries;
    iface.WrongConfigLibraries = impl->WrongConfigLibraries;
    if(this->Target->LinkLanguagePropagatesToDependents())
      {
      // Targets using this archive need its language runtime libraries.
      iface.Languages = impl->Languages;
      }

    if(this->Target->GetPolicyStatusCMP0022() == cmPolicies::WARN &&
       !this->PolicyWarnedCMP0022)
      {
      // Compare the link implementation fallback link interface to the
      // preferred new link interface property and warn if different.
      cmListFileBacktrace lfbt;
      cmGeneratorExpression ge(lfbt);
      cmGeneratorExpressionDAGChecker dagChecker(lfbt, this->GetName(),
                                      "INTERFACE_LINK_LIBRARIES", 0, 0);
      std::vector<std::string> ifaceLibs;
      const char* newExplicitLibraries =
        this->GetProperty("INTERFACE_LINK_LIBRARIES");
      cmSystemTools::ExpandListArgument(
        ge.Parse(newExplicitLibraries)->Evaluate(this->Makefile,
                                                 config,
                                                 false,
                                                 headTarget,
                                                 this->Target, &dagChecker),
        ifaceLibs);
      if (ifaceLibs != impl->Libraries)
        {
        std::string oldLibraries;
        std::string newLibraries;
        const char *sep = "";
        for(std::vector<std::string>::const_iterator it
              = impl->Libraries.begin(); it != impl->Libraries.end(); ++it)
          {
          oldLibraries += sep;
          oldLibraries += *it;
          sep = ";";
          }
        sep = "";
        for(std::vector<std::string>::const_iterator it
              = ifaceLibs.begin(); it != ifaceLibs.end(); ++it)
          {
          newLibraries += sep;
          newLibraries += *it;
          sep = ";";
          }
        if(oldLibraries.empty())
          { oldLibraries = "(empty)"; }
        if(newLibraries.empty())
          { newLibraries = "(empty)"; }

        cmOStringStream w;
        w <<
          (this->Makefile->GetPolicies()
           ->GetPolicyWarning(cmPolicies::CMP0022)) << "\n"
          "Target \"" << this->GetName() << "\" has an "
          "INTERFACE_LINK_LIBRARIES property.  "
          "This should be preferred as the source of the link interface "
          "for this library but because CMP0022 is not set CMake is "
          "ignoring the property and using the link implementation "
          "as the link interface instead."
          "\n"
          "INTERFACE_LINK_LIBRARIES:\n"
          "  " << newLibraries << "\n"
          "Link implementation:\n"
          "  " << oldLibraries << "\n";
        this->Makefile->IssueMessage(cmake::AUTHOR_WARNING, w.str());
        this->PolicyWarnedCMP0022 = true;
        }
      }
    }

  if(this->GetType() == cmTarget::STATIC_LIBRARY)
    {
    // How many repetitions are needed if this library has cyclic
    // dependencies?
    std::string propName = "LINK_INTERFACE_MULTIPLICITY";
    propName += suffix;
    if(const char* config_reps = this->GetProperty(propName.c_str()))
      {
      sscanf(config_reps, "%u", &iface.Multiplicity);
      }
    else if(const char* reps =
            this->GetProperty("LINK_INTERFACE_MULTIPLICITY"))
      {
      sscanf(reps, "%u", &iface.Multiplicity);
      }
    }

  return true;
}


//----------------------------------------------------------------------------
std::string
cmGeneratorTarget::GetFullNameInternal(const char* config, bool implib) const
{
  std::string prefix;
  std::string base;
  std::string suffix;
  this->GetFullNameInternal(config, implib, prefix, base, suffix);
  return prefix+base+suffix;
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::GetFullNameInternal(const char* config,
                                   bool implib,
                                   std::string& outPrefix,
                                   std::string& outBase,
                                   std::string& outSuffix) const
{
  // Use just the target name for non-main target types.
  if(this->GetType() != cmTarget::STATIC_LIBRARY &&
     this->GetType() != cmTarget::SHARED_LIBRARY &&
     this->GetType() != cmTarget::MODULE_LIBRARY &&
     this->GetType() != cmTarget::EXECUTABLE)
    {
    outPrefix = "";
    outBase = this->GetName();
    outSuffix = "";
    return;
    }

  // Return an empty name for the import library if this platform
  // does not support import libraries.
  if(implib &&
     !this->Makefile->GetDefinition("CMAKE_IMPORT_LIBRARY_SUFFIX"))
    {
    outPrefix = "";
    outBase = "";
    outSuffix = "";
    return;
    }

  // The implib option is only allowed for shared libraries, module
  // libraries, and executables.
  if(this->GetType() != cmTarget::SHARED_LIBRARY &&
     this->GetType() != cmTarget::MODULE_LIBRARY &&
     this->GetType() != cmTarget::EXECUTABLE)
    {
    implib = false;
    }

  // Compute the full name for main target types.
  const char* targetPrefix = (implib
                              ? this->GetProperty("IMPORT_PREFIX")
                              : this->GetProperty("PREFIX"));
  const char* targetSuffix = (implib
                              ? this->GetProperty("IMPORT_SUFFIX")
                              : this->GetProperty("SUFFIX"));
  const char* configPostfix = 0;
  if(config && *config)
    {
    std::string configProp = cmSystemTools::UpperCase(config);
    configProp += "_POSTFIX";
    configPostfix = this->GetProperty(configProp.c_str());
    // Mac application bundles and frameworks have no postfix.
    if(configPostfix &&
       (this->Target->IsAppBundleOnApple()
         || this->Target->IsFrameworkOnApple()))
      {
      configPostfix = 0;
      }
    }
  const char* prefixVar = this->Target->GetPrefixVariableInternal(implib);
  const char* suffixVar = this->Target->GetSuffixVariableInternal(implib);

  // Check for language-specific default prefix and suffix.
  if(const char* ll = this->GetLinkerLanguage(config, this->Target))
    {
    if(!targetSuffix && suffixVar && *suffixVar)
      {
      std::string langSuff = suffixVar + std::string("_") + ll;
      targetSuffix = this->Makefile->GetDefinition(langSuff.c_str());
      }
    if(!targetPrefix && prefixVar && *prefixVar)
      {
      std::string langPrefix = prefixVar + std::string("_") + ll;
      targetPrefix = this->Makefile->GetDefinition(langPrefix.c_str());
      }
    }

  // if there is no prefix on the target use the cmake definition
  if(!targetPrefix && prefixVar)
    {
    targetPrefix = this->Makefile->GetSafeDefinition(prefixVar);
    }
  // if there is no suffix on the target use the cmake definition
  if(!targetSuffix && suffixVar)
    {
    targetSuffix = this->Makefile->GetSafeDefinition(suffixVar);
    }

  // frameworks have directory prefix but no suffix
  std::string fw_prefix;
  if(this->Target->IsFrameworkOnApple())
    {
    fw_prefix = this->Target->GetOutputName(config, false);
    fw_prefix += ".framework/";
    targetPrefix = fw_prefix.c_str();
    targetSuffix = 0;
    }

  if(this->Target->IsCFBundleOnApple())
    {
    fw_prefix = this->Target->GetOutputName(config, false);
    fw_prefix += ".";
    const char *ext = this->GetProperty("BUNDLE_EXTENSION");
    if (!ext)
      {
      ext = "bundle";
      }
    fw_prefix += ext;
    fw_prefix += "/Contents/MacOS/";
    targetPrefix = fw_prefix.c_str();
    targetSuffix = 0;
    }

  // Begin the final name with the prefix.
  outPrefix = targetPrefix?targetPrefix:"";

  // Append the target name or property-specified name.
  outBase += this->Target->GetOutputName(config, implib);

  // Append the per-configuration postfix.
  outBase += configPostfix?configPostfix:"";

  // Name shared libraries with their version number on some platforms.
  if(const char* soversion = this->GetProperty("SOVERSION"))
    {
    if(this->GetType() == cmTarget::SHARED_LIBRARY && !implib &&
       this->Makefile->IsOn("CMAKE_SHARED_LIBRARY_NAME_WITH_VERSION"))
      {
      outBase += "-";
      outBase += soversion;
      }
    }

  // Append the suffix.
  outSuffix = targetSuffix?targetSuffix:"";
}


//----------------------------------------------------------------------------
const char* cmGeneratorTarget::GetLinkerLanguage(const char* config,
                                                 cmTarget const* head) const
{
  cmTarget const* headTarget = head ? head : this->Target;
  const char* lang = this->GetLinkClosure(config, headTarget)
                                                    ->LinkerLanguage.c_str();
  return *lang? lang : 0;
}

//----------------------------------------------------------------------------
std::string cmGeneratorTarget::GetPDBName(const char* config) const
{
  std::string prefix;
  std::string base;
  std::string suffix;
  this->GetFullNameInternal(config, false, prefix, base, suffix);

  std::vector<std::string> props;
  std::string configUpper =
    cmSystemTools::UpperCase(config? config : "");
  if(!configUpper.empty())
    {
    // PDB_NAME_<CONFIG>
    props.push_back("PDB_NAME_" + configUpper);
    }

  // PDB_NAME
  props.push_back("PDB_NAME");

  for(std::vector<std::string>::const_iterator i = props.begin();
      i != props.end(); ++i)
    {
    if(const char* outName = this->GetProperty(i->c_str()))
      {
      base = outName;
      break;
      }
    }
  return prefix+base+".pdb";
}

bool cmStrictTargetComparison::operator()(cmTarget const* t1,
                                          cmTarget const* t2) const
{
  int nameResult = strcmp(t1->GetName(), t2->GetName());
  if (nameResult == 0)
    {
    return strcmp(t1->GetMakefile()->GetStartOutputDirectory(),
                  t2->GetMakefile()->GetStartOutputDirectory()) < 0;
    }
  return nameResult < 0;
}

//----------------------------------------------------------------------------
bool cmGeneratorTarget::HasMacOSXRpathInstallNameDir(const char* config) const
{
  bool install_name_is_rpath = false;
  bool macosx_rpath = false;

  if(!this->IsImported())
    {
    if(this->GetType() != cmTarget::SHARED_LIBRARY)
      {
      return false;
      }
    const char* install_name = this->GetProperty("INSTALL_NAME_DIR");
    bool use_install_name =
      this->GetPropertyAsBool("BUILD_WITH_INSTALL_RPATH");
    if(install_name && use_install_name &&
       std::string(install_name) == "@rpath")
      {
      install_name_is_rpath = true;
      }
    else if(install_name && use_install_name)
      {
      return false;
      }
    if(!install_name_is_rpath)
      {
      macosx_rpath = this->MacOSXRpathInstallNameDirDefault();
      }
    }
  else
    {
    // Lookup the imported soname.
    if(cmTarget::ImportInfo const* info =
                            this->Target->GetImportInfo(config, this->Target))
      {
      if(!info->NoSOName && !info->SOName.empty())
        {
        if(info->SOName.find("@rpath/") == 0)
          {
          install_name_is_rpath = true;
          }
        }
      else
        {
        std::string install_name;
        cmSystemTools::GuessLibraryInstallName(info->Location, install_name);
        if(install_name.find("@rpath") != std::string::npos)
          {
          install_name_is_rpath = true;
          }
        }
      }
    }

  if(!install_name_is_rpath && !macosx_rpath)
    {
    return false;
    }

  if(!this->Makefile->IsSet("CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG"))
    {
    cmOStringStream w;
    w << "Attempting to use";
    if(macosx_rpath)
      {
      w << " MACOSX_RPATH";
      }
    else
      {
      w << " @rpath";
      }
    w << " without CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG being set.";
    w << "  This could be because you are using a Mac OS X version";
    w << " less than 10.5 or because CMake's platform configuration is";
    w << " corrupt.";
    cmake* cm = this->Makefile->GetCMakeInstance();
    cm->IssueMessage(cmake::FATAL_ERROR, w.str(),
                     this->Target->GetBacktrace());
    }

  return true;
}

//----------------------------------------------------------------------------
bool cmGeneratorTarget::MacOSXRpathInstallNameDirDefault() const
{
  // we can't do rpaths when unsupported
  if(!this->Makefile->IsSet("CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG"))
    {
    return false;
    }

  const char* macosx_rpath_str = this->GetProperty("MACOSX_RPATH");
  if(macosx_rpath_str)
    {
    return this->GetPropertyAsBool("MACOSX_RPATH");
    }

  cmPolicies::PolicyStatus cmp0042 = this->Target->GetPolicyStatusCMP0042();

  if(cmp0042 == cmPolicies::WARN)
    {
    this->Makefile->GetLocalGenerator()->GetGlobalGenerator()->
      AddCMP0042WarnTarget(this->GetName());
    }

  if(cmp0042 == cmPolicies::NEW)
    {
    return true;
    }

  return false;
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::ComputeVersionedName(std::string& vName,
                                             std::string const& prefix,
                                             std::string const& base,
                                             std::string const& suffix,
                                             std::string const& name,
                                             const char* version) const
{
  const bool apple = this->Makefile->IsOn("APPLE");
  vName = apple ? (prefix+base) : name;
  if(version)
    {
    vName += ".";
    vName += version;
    }
  vName += apple ? suffix : std::string();
}

//----------------------------------------------------------------------------
bool cmGeneratorTarget::HaveBuildTreeRPATH(const char *config) const
{
  if (this->GetPropertyAsBool("SKIP_BUILD_RPATH"))
    {
    return false;
    }
  std::vector<std::string> libs;
  this->GetDirectLinkLibraries(config, libs, this->Target);
  return !libs.empty();
}

//----------------------------------------------------------------------------
bool cmGeneratorTarget::HaveInstallTreeRPATH() const
{
  const char* install_rpath = this->GetProperty("INSTALL_RPATH");
  return (install_rpath && *install_rpath) &&
          !this->Makefile->IsOn("CMAKE_SKIP_INSTALL_RPATH");
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::GetDirectLinkLibraries(const char *config,
                            std::vector<std::string> &libs,
                            cmTarget const* head) const
{
  const char *prop = this->GetProperty("LINK_LIBRARIES");
  if (prop)
    {
    cmListFileBacktrace lfbt;
    cmGeneratorExpression ge(lfbt);
    const cmsys::auto_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(prop);

    cmGeneratorExpressionDAGChecker dagChecker(lfbt,
                                        this->GetName(),
                                        "LINK_LIBRARIES", 0, 0);
    cmSystemTools::ExpandListArgument(cge->Evaluate(this->Makefile,
                                        config,
                                        false,
                                        head,
                                        &dagChecker),
                                      libs);

    std::set<cmStdString> seenProps = cge->GetSeenTargetProperties();
    for (std::set<cmStdString>::const_iterator it = seenProps.begin();
        it != seenProps.end(); ++it)
      {
      if (!this->GetProperty(it->c_str()))
        {
        this->Target->LinkImplicitNullProperties.insert(*it);
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::GetInterfaceLinkLibraries(const char *config,
                        std::vector<std::string> &libs, cmTarget *head) const
{
  const char *prop = this->GetProperty("INTERFACE_LINK_LIBRARIES");
  if (prop)
    {
    cmListFileBacktrace lfbt;
    cmGeneratorExpression ge(lfbt);
    const cmsys::auto_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(prop);

    cmGeneratorExpressionDAGChecker dagChecker(lfbt,
                                        this->GetName(),
                                        "INTERFACE_LINK_LIBRARIES", 0, 0);
    cmSystemTools::ExpandListArgument(cge->Evaluate(this->Makefile,
                                        config,
                                        false,
                                        head,
                                        &dagChecker),
                                      libs);
    }
}
