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
#include "cmComputeLinkInformation.h"
#include "cmCustomCommandGenerator.h"
#include "cmAlgorithms.h"

#include <queue>

#include <errno.h>
#include "assert.h"

#if defined(CMAKE_BUILD_WITH_CMAKE)
#include <cmsys/hash_set.hxx>
#define UNORDERED_SET cmsys::hash_set
#else
#define UNORDERED_SET std::set
#endif

//----------------------------------------------------------------------------
void reportBadObjLib(std::vector<cmSourceFile*> const& badObjLib,
                     cmGeneratorTarget const* target, cmake *cm)
{
  if(!badObjLib.empty())
    {
    std::ostringstream e;
    e << "OBJECT library \"" << target->GetName() << "\" contains:\n";
    for(std::vector<cmSourceFile*>::const_iterator i = badObjLib.begin();
        i != badObjLib.end(); ++i)
      {
      e << "  " << (*i)->GetLocation().GetName() << "\n";
      }
    e << "but may contain only sources that compile, header files, and "
         "other files that would not affect linking of a normal library.";
    cm->IssueMessage(cmake::FATAL_ERROR, e.str(),
                     target->Target->GetBacktrace());
    }
}

struct ObjectSourcesTag {};
struct CustomCommandsTag {};
struct ExtraSourcesTag {};
struct HeaderSourcesTag {};
struct ExternalObjectsTag {};
struct IDLSourcesTag {};
struct ResxTag {};
struct ModuleDefinitionFileTag {};
struct AppManifestTag{};
struct CertificatesTag{};
struct XamlTag{};

template<typename Tag, typename OtherTag>
struct IsSameTag
{
  enum {
    Result = false
  };
};

template<typename Tag>
struct IsSameTag<Tag, Tag>
{
  enum {
    Result = true
  };
};

template<bool>
struct DoAccept
{
  template <typename T> static void Do(T&, cmSourceFile*) {}
};

template<>
struct DoAccept<true>
{
  static void Do(std::vector<cmSourceFile const*>& files, cmSourceFile* f)
    {
    files.push_back(f);
    }
  static void Do(cmGeneratorTarget::ResxData& data, cmSourceFile* f)
    {
    // Build and save the name of the corresponding .h file
    // This relationship will be used later when building the project files.
    // Both names would have been auto generated from Visual Studio
    // where the user supplied the file name and Visual Studio
    // appended the suffix.
    std::string resx = f->GetFullPath();
    std::string hFileName = resx.substr(0, resx.find_last_of(".")) + ".h";
    data.ExpectedResxHeaders.insert(hFileName);
    data.ResxSources.push_back(f);
    }
  static void Do(cmGeneratorTarget::XamlData& data, cmSourceFile* f)
    {
    // Build and save the name of the corresponding .h and .cpp file
    // This relationship will be used later when building the project files.
    // Both names would have been auto generated from Visual Studio
    // where the user supplied the file name and Visual Studio
    // appended the suffix.
    std::string xaml = f->GetFullPath();
    std::string hFileName = xaml + ".h";
    std::string cppFileName = xaml + ".cpp";
    data.ExpectedXamlHeaders.insert(hFileName);
    data.ExpectedXamlSources.insert(cppFileName);
    data.XamlSources.push_back(f);
    }
  static void Do(std::string& data, cmSourceFile* f)
    {
    data = f->GetFullPath();
    }
};

//----------------------------------------------------------------------------
template<typename Tag, typename DataType = std::vector<cmSourceFile const*> >
struct TagVisitor
{
  DataType& Data;
  std::vector<cmSourceFile*> BadObjLibFiles;
  cmGeneratorTarget const* Target;
  cmGlobalGenerator *GlobalGenerator;
  cmsys::RegularExpression Header;
  bool IsObjLib;

  TagVisitor(cmGeneratorTarget const* target, DataType& data)
    : Data(data), Target(target),
    GlobalGenerator(target->GetLocalGenerator()->GetGlobalGenerator()),
    Header(CM_HEADER_REGEX),
    IsObjLib(target->GetType() == cmTarget::OBJECT_LIBRARY)
  {
  }

  ~TagVisitor()
  {
    reportBadObjLib(this->BadObjLibFiles, this->Target,
                    this->GlobalGenerator->GetCMakeInstance());
  }

  void Accept(cmSourceFile *sf)
  {
    std::string ext = cmSystemTools::LowerCase(sf->GetExtension());
    if(sf->GetCustomCommand())
      {
      DoAccept<IsSameTag<Tag, CustomCommandsTag>::Result>::Do(this->Data, sf);
      }
    else if(this->Target->GetType() == cmTarget::UTILITY)
      {
      DoAccept<IsSameTag<Tag, ExtraSourcesTag>::Result>::Do(this->Data, sf);
      }
    else if(sf->GetPropertyAsBool("HEADER_FILE_ONLY"))
      {
      DoAccept<IsSameTag<Tag, HeaderSourcesTag>::Result>::Do(this->Data, sf);
      }
    else if(sf->GetPropertyAsBool("EXTERNAL_OBJECT"))
      {
      DoAccept<IsSameTag<Tag, ExternalObjectsTag>::Result>::Do(this->Data, sf);
      if(this->IsObjLib)
        {
        this->BadObjLibFiles.push_back(sf);
        }
      }
    else if(!sf->GetLanguage().empty())
      {
      DoAccept<IsSameTag<Tag, ObjectSourcesTag>::Result>::Do(this->Data, sf);
      }
    else if(ext == "def")
      {
      DoAccept<IsSameTag<Tag, ModuleDefinitionFileTag>::Result>::Do(this->Data,
                                                                    sf);
      if(this->IsObjLib)
        {
        this->BadObjLibFiles.push_back(sf);
        }
      }
    else if(ext == "idl")
      {
      DoAccept<IsSameTag<Tag, IDLSourcesTag>::Result>::Do(this->Data, sf);
      if(this->IsObjLib)
        {
        this->BadObjLibFiles.push_back(sf);
        }
      }
    else if(ext == "resx")
      {
      DoAccept<IsSameTag<Tag, ResxTag>::Result>::Do(this->Data, sf);
      }
    else if (ext == "appxmanifest")
      {
      DoAccept<IsSameTag<Tag, AppManifestTag>::Result>::Do(this->Data, sf);
      }
    else if (ext == "pfx")
      {
      DoAccept<IsSameTag<Tag, CertificatesTag>::Result>::Do(this->Data, sf);
      }
    else if (ext == "xaml")
      {
      DoAccept<IsSameTag<Tag, XamlTag>::Result>::Do(this->Data, sf);
      }
    else if(this->Header.find(sf->GetFullPath().c_str()))
      {
      DoAccept<IsSameTag<Tag, HeaderSourcesTag>::Result>::Do(this->Data, sf);
      }
    else if(this->GlobalGenerator->IgnoreFile(sf->GetExtension().c_str()))
      {
      DoAccept<IsSameTag<Tag, ExtraSourcesTag>::Result>::Do(this->Data, sf);
      }
    else
      {
      DoAccept<IsSameTag<Tag, ExtraSourcesTag>::Result>::Do(this->Data, sf);
      }
  }
};

//----------------------------------------------------------------------------
cmGeneratorTarget::cmGeneratorTarget(cmTarget* t, cmLocalGenerator* lg)
  : Target(t),
  SourceFileFlagsConstructed(false)
{
  this->Makefile = this->Target->GetMakefile();
  this->LocalGenerator = lg;
  this->GlobalGenerator = this->LocalGenerator->GetGlobalGenerator();
}

cmGeneratorTarget::~cmGeneratorTarget()
{
  cmDeleteAll(this->LinkInformation);
  this->LinkInformation.clear();
}

cmLocalGenerator* cmGeneratorTarget::GetLocalGenerator() const
{
  return this->LocalGenerator;
}

//----------------------------------------------------------------------------
int cmGeneratorTarget::GetType() const
{
  return this->Target->GetType();
}

//----------------------------------------------------------------------------
std::string cmGeneratorTarget::GetName() const
{
  return this->Target->GetName();
}

//----------------------------------------------------------------------------
const char *cmGeneratorTarget::GetProperty(const std::string& prop) const
{
  return this->Target->GetProperty(prop);
}

//----------------------------------------------------------------------------
std::string cmGeneratorTarget::GetOutputName(const std::string& config,
                                             bool implib) const
{
  std::vector<std::string> props;
  std::string type = this->Target->GetOutputTargetType(implib);
  std::string configUpper = cmSystemTools::UpperCase(config);
  if(!type.empty() && !configUpper.empty())
    {
    // <ARCHIVE|LIBRARY|RUNTIME>_OUTPUT_NAME_<CONFIG>
    props.push_back(type + "_OUTPUT_NAME_" + configUpper);
    }
  if(!type.empty())
    {
    // <ARCHIVE|LIBRARY|RUNTIME>_OUTPUT_NAME
    props.push_back(type + "_OUTPUT_NAME");
    }
  if(!configUpper.empty())
    {
    // OUTPUT_NAME_<CONFIG>
    props.push_back("OUTPUT_NAME_" + configUpper);
    // <CONFIG>_OUTPUT_NAME
    props.push_back(configUpper + "_OUTPUT_NAME");
    }
  // OUTPUT_NAME
  props.push_back("OUTPUT_NAME");

  std::string outName;
  for(std::vector<std::string>::const_iterator i = props.begin();
      i != props.end(); ++i)
    {
    if (const char* outNameProp = this->Target->GetProperty(*i))
      {
      outName = outNameProp;
      break;
      }
    }

  if (outName.empty())
    {
    outName = this->GetName();
    }

  cmGeneratorExpression ge;
  cmsys::auto_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(outName);
  return cge->Evaluate(this->Makefile, config);
}

//----------------------------------------------------------------------------
std::vector<cmSourceFile*> const*
cmGeneratorTarget::GetSourceDepends(cmSourceFile const* sf) const
{
  SourceEntriesType::const_iterator i = this->SourceEntries.find(sf);
  if(i != this->SourceEntries.end())
    {
    return &i->second.Depends;
    }
  return 0;
}

static void handleSystemIncludesDep(cmMakefile *mf, cmTarget const* depTgt,
                                  const std::string& config,
                                  cmTarget *headTarget,
                                  cmGeneratorExpressionDAGChecker *dagChecker,
                                  std::vector<std::string>& result,
                                  bool excludeImported)
{
  if (const char* dirs =
          depTgt->GetProperty("INTERFACE_SYSTEM_INCLUDE_DIRECTORIES"))
    {
    cmGeneratorExpression ge;
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
    cmGeneratorExpression ge;
    cmSystemTools::ExpandListArgument(ge.Parse(dirs)
                                      ->Evaluate(mf,
                                      config, false, headTarget,
                                      depTgt, dagChecker), result);
    }
}

#define IMPLEMENT_VISIT_IMPL(DATA, DATATYPE) \
  { \
  std::vector<cmSourceFile*> sourceFiles; \
  this->Target->GetSourceFiles(sourceFiles, config); \
  TagVisitor<DATA ## Tag DATATYPE> visitor(this, data); \
  for(std::vector<cmSourceFile*>::const_iterator si = sourceFiles.begin(); \
      si != sourceFiles.end(); ++si) \
    { \
    visitor.Accept(*si); \
    } \
  } \


#define IMPLEMENT_VISIT(DATA) \
  IMPLEMENT_VISIT_IMPL(DATA, EMPTY) \

#define EMPTY
#define COMMA ,

//----------------------------------------------------------------------------
void
cmGeneratorTarget
::GetObjectSources(std::vector<cmSourceFile const*> &data,
                   const std::string& config) const
{
  IMPLEMENT_VISIT(ObjectSources);

  if (!this->Objects.empty())
    {
    return;
    }

  for(std::vector<cmSourceFile const*>::const_iterator it = data.begin();
      it != data.end(); ++it)
    {
    this->Objects[*it];
    }

  this->LocalGenerator->ComputeObjectFilenames(this->Objects, this);
}

void cmGeneratorTarget::ComputeObjectMapping()
{
  if(!this->Objects.empty())
    {
    return;
    }

  std::vector<std::string> configs;
  this->Makefile->GetConfigurations(configs);
  if (configs.empty())
    {
    configs.push_back("");
    }
  for(std::vector<std::string>::const_iterator ci = configs.begin();
      ci != configs.end(); ++ci)
    {
    std::vector<cmSourceFile const*> sourceFiles;
    this->GetObjectSources(sourceFiles, *ci);
    }
}

//----------------------------------------------------------------------------
const char* cmGeneratorTarget::GetFeature(const std::string& feature,
                                          const std::string& config) const
{
  if(!config.empty())
    {
    std::string featureConfig = feature;
    featureConfig += "_";
    featureConfig += cmSystemTools::UpperCase(config);
    if(const char* value = this->Target->GetProperty(featureConfig))
      {
      return value;
      }
    }
  if(const char* value = this->Target->GetProperty(feature))
    {
    return value;
    }
  return this->LocalGenerator->GetFeature(feature, config);
}

//----------------------------------------------------------------------------
bool cmGeneratorTarget::GetFeatureAsBool(const std::string& feature,
                                         const std::string& config) const
{
  return cmSystemTools::IsOn(this->GetFeature(feature, config));
}

//----------------------------------------------------------------------------
const std::string& cmGeneratorTarget::GetObjectName(cmSourceFile const* file)
{
  this->ComputeObjectMapping();
  return this->Objects[file];
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::AddExplicitObjectName(cmSourceFile const* sf)
{
  this->ExplicitObjectName.insert(sf);
}

//----------------------------------------------------------------------------
bool cmGeneratorTarget::HasExplicitObjectName(cmSourceFile const* file) const
{
  const_cast<cmGeneratorTarget*>(this)->ComputeObjectMapping();
  std::set<cmSourceFile const*>::const_iterator it
                                        = this->ExplicitObjectName.find(file);
  return it != this->ExplicitObjectName.end();
}

//----------------------------------------------------------------------------
void cmGeneratorTarget
::GetIDLSources(std::vector<cmSourceFile const*>& data,
                const std::string& config) const
{
  IMPLEMENT_VISIT(IDLSources);
}

//----------------------------------------------------------------------------
void
cmGeneratorTarget
::GetHeaderSources(std::vector<cmSourceFile const*>& data,
                   const std::string& config) const
{
  IMPLEMENT_VISIT(HeaderSources);
}

//----------------------------------------------------------------------------
void cmGeneratorTarget
::GetExtraSources(std::vector<cmSourceFile const*>& data,
                  const std::string& config) const
{
  IMPLEMENT_VISIT(ExtraSources);
}

//----------------------------------------------------------------------------
void
cmGeneratorTarget
::GetCustomCommands(std::vector<cmSourceFile const*>& data,
                    const std::string& config) const
{
  IMPLEMENT_VISIT(CustomCommands);
}

//----------------------------------------------------------------------------
void
cmGeneratorTarget
::GetExternalObjects(std::vector<cmSourceFile const*>& data,
                     const std::string& config) const
{
  IMPLEMENT_VISIT(ExternalObjects);
}

//----------------------------------------------------------------------------
void
cmGeneratorTarget::GetExpectedResxHeaders(std::set<std::string>& srcs,
                                          const std::string& config) const
{
  ResxData data;
  IMPLEMENT_VISIT_IMPL(Resx, COMMA cmGeneratorTarget::ResxData)
  srcs = data.ExpectedResxHeaders;
}

//----------------------------------------------------------------------------
void cmGeneratorTarget
::GetResxSources(std::vector<cmSourceFile const*>& srcs,
                 const std::string& config) const
{
  ResxData data;
  IMPLEMENT_VISIT_IMPL(Resx, COMMA cmGeneratorTarget::ResxData)
  srcs = data.ResxSources;
}

//----------------------------------------------------------------------------
void
cmGeneratorTarget
::GetAppManifest(std::vector<cmSourceFile const*>& data,
                 const std::string& config) const
{
  IMPLEMENT_VISIT(AppManifest);
}

//----------------------------------------------------------------------------
void
cmGeneratorTarget
::GetCertificates(std::vector<cmSourceFile const*>& data,
                  const std::string& config) const
{
  IMPLEMENT_VISIT(Certificates);
}

//----------------------------------------------------------------------------
void
cmGeneratorTarget::GetExpectedXamlHeaders(std::set<std::string>& headers,
                                          const std::string& config) const
{
  XamlData data;
  IMPLEMENT_VISIT_IMPL(Xaml, COMMA cmGeneratorTarget::XamlData)
  headers = data.ExpectedXamlHeaders;
}

//----------------------------------------------------------------------------
void
cmGeneratorTarget::GetExpectedXamlSources(std::set<std::string>& srcs,
                                          const std::string& config) const
{
  XamlData data;
  IMPLEMENT_VISIT_IMPL(Xaml, COMMA cmGeneratorTarget::XamlData)
  srcs = data.ExpectedXamlSources;
}

//----------------------------------------------------------------------------
void cmGeneratorTarget
::GetXamlSources(std::vector<cmSourceFile const*>& srcs,
                 const std::string& config) const
{
  XamlData data;
  IMPLEMENT_VISIT_IMPL(Xaml, COMMA cmGeneratorTarget::XamlData)
  srcs = data.XamlSources;
}

//----------------------------------------------------------------------------
const char* cmGeneratorTarget::GetLocation(const std::string& config) const
{
  static std::string location;
  if (this->Target->IsImported())
    {
    location = this->Target->ImportedGetFullPath(config, false);
    }
  else
    {
    location = this->GetFullPath(config, false);
    }
  return location.c_str();
}

bool cmGeneratorTarget::IsImported() const
{
  return this->Target->IsImported();
}

//----------------------------------------------------------------------------
const char* cmGeneratorTarget::GetLocationForBuild() const
{
  static std::string location;
  if(this->IsImported())
    {
    location = this->Target->ImportedGetFullPath("", false);
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
    std::string macdir = this->BuildMacContentDirectory("", "",
                                                                false);
    if(!macdir.empty())
      {
      location += "/";
      location += macdir;
      }
    }
  location += "/";
  location += this->GetFullName("", false);
  return location.c_str();
}


//----------------------------------------------------------------------------
bool cmGeneratorTarget::IsSystemIncludeDirectory(const std::string& dir,
                                              const std::string& config) const
{
  assert(this->GetType() != cmTarget::INTERFACE_LIBRARY);
  std::string config_upper;
  if(!config.empty())
    {
    config_upper = cmSystemTools::UpperCase(config);
    }

  typedef std::map<std::string, std::vector<std::string> > IncludeCacheType;
  IncludeCacheType::const_iterator iter =
      this->SystemIncludesCache.find(config_upper);

  if (iter == this->SystemIncludesCache.end())
    {
    cmGeneratorExpressionDAGChecker dagChecker(
                                        this->GetName(),
                                        "SYSTEM_INCLUDE_DIRECTORIES", 0, 0);

    bool excludeImported
                = this->Target->GetPropertyAsBool("NO_SYSTEM_FROM_IMPORTED");

    std::vector<std::string> result;
    for (std::set<std::string>::const_iterator
        it = this->Target->GetSystemIncludeDirectories().begin();
        it != this->Target->GetSystemIncludeDirectories().end(); ++it)
      {
      cmGeneratorExpression ge;
      cmSystemTools::ExpandListArgument(ge.Parse(*it)
                                          ->Evaluate(this->Makefile,
                                          config, false, this->Target,
                                          &dagChecker), result);
      }

    std::vector<cmTarget const*> const& deps =
      this->GetLinkImplementationClosure(config);
    for(std::vector<cmTarget const*>::const_iterator
          li = deps.begin(), le = deps.end(); li != le; ++li)
      {
      handleSystemIncludesDep(this->Makefile, *li, config, this->Target,
                              &dagChecker, result, excludeImported);
      }

    std::set<std::string> unique;
    for(std::vector<std::string>::iterator li = result.begin();
        li != result.end(); ++li)
      {
      cmSystemTools::ConvertToUnixSlashes(*li);
      unique.insert(*li);
      }
    result.clear();
    result.insert(result.end(), unique.begin(), unique.end());

    IncludeCacheType::value_type entry(config_upper, result);
    iter = this->SystemIncludesCache.insert(entry).first;
    }

  return std::binary_search(iter->second.begin(), iter->second.end(), dir);
}

//----------------------------------------------------------------------------
bool cmGeneratorTarget::GetPropertyAsBool(const std::string& prop) const
{
  return this->Target->GetPropertyAsBool(prop);
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::GetSourceFiles(std::vector<cmSourceFile*> &files,
                                       const std::string& config) const
{
  this->Target->GetSourceFiles(files, config);
}

//----------------------------------------------------------------------------
std::string
cmGeneratorTarget::GetCompilePDBName(const std::string& config) const
{
  std::string prefix;
  std::string base;
  std::string suffix;
  this->GetFullNameInternal(config, false, prefix, base, suffix);

  // Check for a per-configuration output directory target property.
  std::string configUpper = cmSystemTools::UpperCase(config);
  std::string configProp = "COMPILE_PDB_NAME_";
  configProp += configUpper;
  const char* config_name = this->Target->GetProperty(configProp);
  if(config_name && *config_name)
    {
    return prefix + config_name + ".pdb";
    }

  const char* name = this->Target->GetProperty("COMPILE_PDB_NAME");
  if(name && *name)
    {
    return prefix + name + ".pdb";
    }

  return "";
}

//----------------------------------------------------------------------------
std::string
cmGeneratorTarget::GetCompilePDBPath(const std::string& config) const
{
  std::string dir = this->GetCompilePDBDirectory(config);
  std::string name = this->GetCompilePDBName(config);
  if(dir.empty() && !name.empty())
    {
    dir = this->Target->GetPDBDirectory(config);
    }
  if(!dir.empty())
    {
    dir += "/";
    }
  return dir + name;
}

//----------------------------------------------------------------------------
bool cmGeneratorTarget::HasSOName(const std::string& config) const
{
  // soname is supported only for shared libraries and modules,
  // and then only when the platform supports an soname flag.
  return ((this->GetType() == cmTarget::SHARED_LIBRARY ||
           this->GetType() == cmTarget::MODULE_LIBRARY) &&
          !this->GetPropertyAsBool("NO_SONAME") &&
          this->Makefile->GetSONameFlag(this->GetLinkerLanguage(config)));
}

//----------------------------------------------------------------------------
bool
cmGeneratorTarget::NeedRelinkBeforeInstall(const std::string& config) const
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
  std::string ll = this->GetLinkerLanguage(config);
  if(!ll.empty())
    {
    std::string flagVar = "CMAKE_SHARED_LIBRARY_RUNTIME_";
    flagVar += ll;
    flagVar += "_FLAG";
    if(!this->Makefile->IsSet(flagVar))
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
  return this->Target->HaveBuildTreeRPATH(config)
      || this->Target->HaveInstallTreeRPATH();
}

//----------------------------------------------------------------------------
bool cmGeneratorTarget::IsChrpathUsed(const std::string& config) const
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
  std::string ll = this->GetLinkerLanguage(config);
  if(!ll.empty())
    {
    std::string sepVar = "CMAKE_SHARED_LIBRARY_RUNTIME_";
    sepVar += ll;
    sepVar += "_FLAG_SEP";
    const char* sep = this->Makefile->GetDefinition(sepVar);
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
std::string cmGeneratorTarget::GetSOName(const std::string& config) const
{
  if(this->Target->IsImported())
    {
    // Lookup the imported soname.
    if(cmTarget::ImportInfo const* info = this->Target->GetImportInfo(config))
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
std::string
cmGeneratorTarget::GetAppBundleDirectory(const std::string& config,
                                         bool contentOnly) const
{
  std::string fpath = this->GetFullName(config, false);
  fpath += ".app";
  if(!this->Makefile->PlatformIsAppleIos())
    {
    fpath += "/Contents";
    if(!contentOnly)
      fpath += "/MacOS";
    }
  return fpath;
}

//----------------------------------------------------------------------------
bool cmGeneratorTarget::IsBundleOnApple() const
{
  return this->Target->IsFrameworkOnApple()
      || this->Target->IsAppBundleOnApple()
      || this->Target->IsCFBundleOnApple();
}

//----------------------------------------------------------------------------
std::string cmGeneratorTarget::GetCFBundleDirectory(const std::string& config,
                                                    bool contentOnly) const
{
  std::string fpath;
  fpath += this->GetOutputName(config, false);
  fpath += ".";
  const char *ext = this->Target->GetProperty("BUNDLE_EXTENSION");
  if (!ext)
    {
    if (this->Target->IsXCTestOnApple())
      {
      ext = "xctest";
      }
    else
      {
      ext = "bundle";
      }
    }
  fpath += ext;
  if(!this->Makefile->PlatformIsAppleIos())
  {
    fpath += "/Contents";
    if(!contentOnly)
      fpath += "/MacOS";
  }
  return fpath;
}

//----------------------------------------------------------------------------
std::string
cmGeneratorTarget::GetFrameworkDirectory(const std::string& config,
                                         bool rootDir) const
{
  std::string fpath;
  fpath += this->GetOutputName(config, false);
  fpath += ".framework";
  if(!rootDir && !this->Makefile->PlatformIsAppleIos())
    {
    fpath += "/Versions/";
    fpath += this->Target->GetFrameworkVersion();
    }
  return fpath;
}

//----------------------------------------------------------------------------
std::string
cmGeneratorTarget::GetFullName(const std::string& config, bool implib) const
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
std::string
cmGeneratorTarget::GetInstallNameDirForBuildTree(
                                            const std::string& config) const
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
    if(this->Target->MacOSXRpathInstallNameDirDefault())
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
      if(this->Target->MacOSXRpathInstallNameDirDefault())
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
                               const std::string& config,
                               UNORDERED_SET<std::string>& languages,
                               cmTarget const* head):
    Config(config), Languages(languages), HeadTarget(head),
    Makefile(target->Target->GetMakefile()), Target(target)
  { this->Visited.insert(target->Target); }

  void Visit(cmLinkItem const& item)
    {
    if(!item.Target)
      {
      if(item.find("::") != std::string::npos)
        {
        bool noMessage = false;
        cmake::MessageType messageType = cmake::FATAL_ERROR;
        std::stringstream e;
        switch(this->Makefile->GetPolicyStatus(cmPolicies::CMP0028))
          {
          case cmPolicies::WARN:
            {
            e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0028) << "\n";
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
          e << "Target \"" << this->Target->GetName()
            << "\" links to target \"" << item
            << "\" but the target was not found.  Perhaps a find_package() "
            "call is missing for an IMPORTED target, or an ALIAS target is "
            "missing?";
          this->Makefile->GetCMakeInstance()->IssueMessage(
            messageType, e.str(), this->Target->Target->GetBacktrace());
          }
        }
      return;
      }
    if(!this->Visited.insert(item.Target).second)
      {
      return;
      }

    cmTarget::LinkInterface const* iface =
      item.Target->GetLinkInterface(this->Config, this->HeadTarget);
    if(!iface) { return; }

    for(std::vector<std::string>::const_iterator
          li = iface->Languages.begin(); li != iface->Languages.end(); ++li)
      {
      this->Languages.insert(*li);
      }

    for(std::vector<cmLinkItem>::const_iterator
          li = iface->Libraries.begin(); li != iface->Libraries.end(); ++li)
      {
      this->Visit(*li);
      }
    }
private:
  std::string Config;
  UNORDERED_SET<std::string>& Languages;
  cmTarget const* HeadTarget;
  cmMakefile* Makefile;
  const cmGeneratorTarget* Target;
  std::set<cmTarget const*> Visited;
};

//----------------------------------------------------------------------------
cmGeneratorTarget::LinkClosure const*
cmGeneratorTarget::GetLinkClosure(const std::string& config) const
{
  std::string key(cmSystemTools::UpperCase(config));
  LinkClosureMapType::iterator
    i = this->LinkClosureMap.find(key);
  if(i == this->LinkClosureMap.end())
    {
    LinkClosure lc;
    this->ComputeLinkClosure(config, lc);
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
  std::set<std::string> Preferred;
public:
  cmTargetSelectLinker(cmGeneratorTarget const* target)
      : Preference(0), Target(target)
    {
    this->Makefile = this->Target->Makefile;
    this->GG = this->Makefile->GetGlobalGenerator();
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
      std::stringstream e;
      e << "Target " << this->Target->GetName()
        << " contains multiple languages with the highest linker preference"
        << " (" << this->Preference << "):\n";
      for(std::set<std::string>::const_iterator
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
void cmGeneratorTarget::ComputeLinkClosure(const std::string& config,
                                           LinkClosure& lc) const
{
  // Get languages built in this target.
  UNORDERED_SET<std::string> languages;
  cmTarget::LinkImplementation const* impl =
                            this->Target->GetLinkImplementation(config);
  assert(impl);
  for(std::vector<std::string>::const_iterator li = impl->Languages.begin();
      li != impl->Languages.end(); ++li)
    {
    languages.insert(*li);
    }

  // Add interface languages from linked targets.
  cmTargetCollectLinkLanguages cll(this, config, languages, this->Target);
  for(std::vector<cmLinkImplItem>::const_iterator li = impl->Libraries.begin();
      li != impl->Libraries.end(); ++li)
    {
    cll.Visit(*li);
    }

  // Store the transitive closure of languages.
  for(UNORDERED_SET<std::string>::const_iterator li = languages.begin();
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
    for(UNORDERED_SET<std::string>::const_iterator sit = languages.begin();
        sit != languages.end(); ++sit)
      {
      std::string propagates = "CMAKE_"+*sit+"_LINKER_PREFERENCE_PROPAGATES";
      if(this->Makefile->IsOn(propagates))
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
                                              std::string& suffix,
                                              const std::string& config,
                                              bool implib) const
{
  this->GetFullNameInternal(config, implib, prefix, base, suffix);
}

//----------------------------------------------------------------------------
std::string
cmGeneratorTarget::BuildMacContentDirectory(const std::string& base,
                                            const std::string& config,
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
std::string
cmGeneratorTarget::GetMacContentDirectory(const std::string& config,
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
cmGeneratorTarget::CompileInfo const* cmGeneratorTarget::GetCompileInfo(
                                            const std::string& config) const
{
  // There is no compile information for imported targets.
  if(this->IsImported())
    {
    return 0;
    }

  if(this->GetType() > cmTarget::OBJECT_LIBRARY)
    {
    std::string msg = "cmTarget::GetCompileInfo called for ";
    msg += this->GetName();
    msg += " which has type ";
    msg += cmTarget::GetTargetTypeName(this->Target->GetType());
    this->Makefile->IssueMessage(cmake::INTERNAL_ERROR, msg);
    return 0;
    }

  // Lookup/compute/cache the compile information for this configuration.
  std::string config_upper;
  if(!config.empty())
    {
    config_upper = cmSystemTools::UpperCase(config);
    }
  CompileInfoMapType::const_iterator i =
    this->CompileInfoMap.find(config_upper);
  if(i == this->CompileInfoMap.end())
    {
    CompileInfo info;
    this->Target
        ->ComputePDBOutputDir("COMPILE_PDB", config, info.CompilePdbDir);
    CompileInfoMapType::value_type entry(config_upper, info);
    i = this->CompileInfoMap.insert(entry).first;
    }
  return &i->second;
}

//----------------------------------------------------------------------------
std::string
cmGeneratorTarget::GetModuleDefinitionFile(const std::string& config) const
{
  std::string data;
  IMPLEMENT_VISIT_IMPL(ModuleDefinitionFile, COMMA std::string)
  return data;
}

//----------------------------------------------------------------------------
void
cmGeneratorTarget::UseObjectLibraries(std::vector<std::string>& objs,
                                      const std::string &config) const
{
  std::vector<cmSourceFile const*> objectFiles;
  this->GetExternalObjects(objectFiles, config);
  std::vector<cmTarget*> objectLibraries;
  for(std::vector<cmSourceFile const*>::const_iterator
      it = objectFiles.begin(); it != objectFiles.end(); ++it)
    {
    std::string objLib = (*it)->GetObjectLibrary();
    if (cmTarget* tgt = this->Makefile->FindTargetToUse(objLib))
      {
      objectLibraries.push_back(tgt);
      }
    }

  std::vector<cmTarget*>::const_iterator end
      = cmRemoveDuplicates(objectLibraries);

  for(std::vector<cmTarget*>::const_iterator
        ti = objectLibraries.begin();
      ti != end; ++ti)
    {
    cmTarget* objLib = *ti;
    cmGeneratorTarget* ogt =
      this->GlobalGenerator->GetGeneratorTarget(objLib);
    std::vector<cmSourceFile const*> objectSources;
    ogt->GetObjectSources(objectSources, config);
    for(std::vector<cmSourceFile const*>::const_iterator
          si = objectSources.begin();
        si != objectSources.end(); ++si)
      {
      std::string obj = ogt->ObjectDirectory;
      obj += ogt->Objects[*si];
      objs.push_back(obj);
      }
    }
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::GetAutoUicOptions(std::vector<std::string> &result,
                                 const std::string& config) const
{
  const char *prop
            = this->GetLinkInterfaceDependentStringProperty("AUTOUIC_OPTIONS",
                                                            config);
  if (!prop)
    {
    return;
    }
  cmGeneratorExpression ge;

  cmGeneratorExpressionDAGChecker dagChecker(
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
void processILibs(const std::string& config,
                  cmTarget const* headTarget,
                  cmLinkItem const& item,
                  std::vector<cmTarget const*>& tgts,
                  std::set<cmTarget const*>& emitted)
{
  if (item.Target && emitted.insert(item.Target).second)
    {
    tgts.push_back(item.Target);
    if(cmTarget::LinkInterfaceLibraries const* iface =
       item.Target->GetLinkInterfaceLibraries(config, headTarget, true))
      {
      for(std::vector<cmLinkItem>::const_iterator
            it = iface->Libraries.begin();
          it != iface->Libraries.end(); ++it)
        {
        processILibs(config, headTarget, *it, tgts, emitted);
        }
      }
    }
}

//----------------------------------------------------------------------------
const std::vector<const cmTarget*>&
cmGeneratorTarget::GetLinkImplementationClosure(
    const std::string& config) const
{
  LinkImplClosure& tgts =
    this->LinkImplClosureMap[config];
  if(!tgts.Done)
    {
    tgts.Done = true;
    std::set<cmTarget const*> emitted;

    cmLinkImplementationLibraries const* impl
      = this->Target->GetLinkImplementationLibraries(config);

    for(std::vector<cmLinkImplItem>::const_iterator
          it = impl->Libraries.begin();
        it != impl->Libraries.end(); ++it)
      {
      processILibs(config, this->Target, *it, tgts , emitted);
      }
    }
  return tgts;
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
  cmGlobalGenerator const* GlobalGenerator;
  typedef cmGeneratorTarget::SourceEntry SourceEntry;
  SourceEntry* CurrentEntry;
  std::queue<cmSourceFile*> SourceQueue;
  std::set<cmSourceFile*> SourcesQueued;
  typedef std::map<std::string, cmSourceFile*> NameMapType;
  NameMapType NameMap;
  std::vector<std::string> NewSources;

  void QueueSource(cmSourceFile* sf);
  void FollowName(std::string const& name);
  void FollowNames(std::vector<std::string> const& names);
  bool IsUtility(std::string const& dep);
  void CheckCustomCommand(cmCustomCommand const& cc);
  void CheckCustomCommands(const std::vector<cmCustomCommand>& commands);
  void FollowCommandDepends(cmCustomCommand const& cc,
                            const std::string& config,
                            std::set<std::string>& emitted);
};

//----------------------------------------------------------------------------
cmTargetTraceDependencies
::cmTargetTraceDependencies(cmGeneratorTarget* target):
  Target(target->Target), GeneratorTarget(target)
{
  // Convenience.
  this->Makefile = this->Target->GetMakefile();
  this->GlobalGenerator = target->GetLocalGenerator()->GetGlobalGenerator();
  this->CurrentEntry = 0;

  // Queue all the source files already specified for the target.
  if (this->Target->GetType() != cmTarget::INTERFACE_LIBRARY)
    {
    std::vector<std::string> configs;
    this->Makefile->GetConfigurations(configs);
    if (configs.empty())
      {
      configs.push_back("");
      }
    std::set<cmSourceFile*> emitted;
    for(std::vector<std::string>::const_iterator ci = configs.begin();
        ci != configs.end(); ++ci)
      {
      std::vector<cmSourceFile*> sources;
      this->Target->GetSourceFiles(sources, *ci);
      for(std::vector<cmSourceFile*>::const_iterator si = sources.begin();
          si != sources.end(); ++si)
        {
        cmSourceFile* sf = *si;
        const std::set<cmTarget const*> tgts =
                          this->GlobalGenerator->GetFilenameTargetDepends(sf);
        if (tgts.find(this->Target) != tgts.end())
          {
          std::ostringstream e;
          e << "Evaluation output file\n  \"" << sf->GetFullPath()
            << "\"\ndepends on the sources of a target it is used in.  This "
              "is a dependency loop and is not allowed.";
          this->GeneratorTarget
              ->LocalGenerator->IssueMessage(cmake::FATAL_ERROR, e.str());
          return;
          }
        if(emitted.insert(sf).second && this->SourcesQueued.insert(sf).second)
          {
          this->SourceQueue.push(sf);
          }
        }
      }
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
      for(std::vector<std::string>::iterator odi = objDeps.begin();
          odi != objDeps.end(); ++odi)
        {
        if (cmSystemTools::FileIsFullPath(*odi))
          {
          *odi = cmSystemTools::CollapseFullPath(*odi);
          }
        }
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

  this->Target->AddTracedSources(this->NewSources);
}

//----------------------------------------------------------------------------
void cmTargetTraceDependencies::QueueSource(cmSourceFile* sf)
{
  if(this->SourcesQueued.insert(sf).second)
    {
    this->SourceQueue.push(sf);

    // Make sure this file is in the target at the end.
    this->NewSources.push_back(sf->GetFullPath());
    }
}

//----------------------------------------------------------------------------
void cmTargetTraceDependencies::FollowName(std::string const& name)
{
  NameMapType::iterator i = this->NameMap.find(name);
  if(i == this->NameMap.end())
    {
    // Check if we know how to generate this file.
    cmSourceFile* sf = this->Makefile->GetSourceFileWithOutput(name);
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
                    = this->Makefile->FindGeneratorTargetToUse(util))
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
        std::string tLocation = t->GetLocationForBuild();
        tLocation = cmSystemTools::GetFilenamePath(tLocation);
        std::string depLocation = cmSystemTools::GetFilenamePath(dep);
        depLocation = cmSystemTools::CollapseFullPath(depLocation);
        tLocation = cmSystemTools::CollapseFullPath(tLocation);
        if(depLocation == tLocation)
          {
          this->Target->AddUtility(util);
          return true;
          }
        }
      }
    else
      {
      // The original name of the dependency was not a full path.  It
      // must name a target, so add the target-level dependency.
      this->Target->AddUtility(util);
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
    if(cmTarget* t = this->Makefile->FindTargetToUse(command))
      {
      if(t->GetType() == cmTarget::EXECUTABLE)
        {
        // The command refers to an executable target built in
        // this project.  Add the target-level dependency to make
        // sure the executable is up to date before this custom
        // command possibly runs.
        this->Target->AddUtility(command);
        }
      }

    // Check for target references in generator expressions.
    for(cmCustomCommandLine::const_iterator cli = cit->begin();
        cli != cit->end(); ++cli)
      {
      const cmsys::auto_ptr<cmCompiledGeneratorExpression> cge
                                                              = ge.Parse(*cli);
      cge->Evaluate(this->Makefile, "", true);
      std::set<cmTarget*> geTargets = cge->GetTargets();
      targets.insert(geTargets.begin(), geTargets.end());
      }
    }

  for(std::set<cmTarget*>::iterator ti = targets.begin();
      ti != targets.end(); ++ti)
    {
    this->Target->AddUtility((*ti)->GetName());
    }

  // Queue the custom command dependencies.
  std::vector<std::string> configs;
  std::set<std::string> emitted;
  this->Makefile->GetConfigurations(configs);
  if (configs.empty())
    {
    configs.push_back("");
    }
  for(std::vector<std::string>::const_iterator ci = configs.begin();
      ci != configs.end(); ++ci)
    {
    this->FollowCommandDepends(cc, *ci, emitted);
    }
}

//----------------------------------------------------------------------------
void cmTargetTraceDependencies::FollowCommandDepends(cmCustomCommand const& cc,
                                              const std::string& config,
                                              std::set<std::string>& emitted)
{
  cmCustomCommandGenerator ccg(cc, config,
                               this->GeneratorTarget->LocalGenerator);

  const std::vector<std::string>& depends = ccg.GetDepends();

  for(std::vector<std::string>::const_iterator di = depends.begin();
      di != depends.end(); ++di)
    {
    std::string const& dep = *di;
    if(emitted.insert(dep).second)
      {
      if(!this->IsUtility(dep))
        {
        // The dependency does not name a target and may be a file we
        // know how to generate.  Queue it.
        this->FollowName(dep);
        }
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

std::string
cmGeneratorTarget::GetCompilePDBDirectory(const std::string& config) const
{
  if(CompileInfo const* info = this->GetCompileInfo(config))
    {
    return info->CompilePdbDir;
    }
  return "";
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::GetAppleArchs(const std::string& config,
                             std::vector<std::string>& archVec) const
{
  const char* archs = 0;
  if(!config.empty())
    {
    std::string defVarName = "OSX_ARCHITECTURES_";
    defVarName += cmSystemTools::UpperCase(config);
    archs = this->Target->GetProperty(defVarName);
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
std::string
cmGeneratorTarget::GetCreateRuleVariable(std::string const& lang,
                                         std::string const& config) const
{
  switch(this->GetType())
    {
    case cmTarget::STATIC_LIBRARY:
      {
      std::string var = "CMAKE_" + lang + "_CREATE_STATIC_LIBRARY";
      if(this->GetFeatureAsBool(
           "INTERPROCEDURAL_OPTIMIZATION", config))
        {
        std::string varIPO = var + "_IPO";
        if(this->Makefile->GetDefinition(varIPO))
          {
          return varIPO;
          }
        }
      return var;
      }
    case cmTarget::SHARED_LIBRARY:
      return "CMAKE_" + lang + "_CREATE_SHARED_LIBRARY";
    case cmTarget::MODULE_LIBRARY:
      return "CMAKE_" + lang + "_CREATE_SHARED_MODULE";
    case cmTarget::EXECUTABLE:
      return "CMAKE_" + lang + "_LINK_EXECUTABLE";
    default:
      break;
    }
  return "";
}

//----------------------------------------------------------------------------
std::vector<std::string>
cmGeneratorTarget::GetIncludeDirectories(const std::string& config,
                                         const std::string& lang) const
{
  return this->Target->GetIncludeDirectories(config, lang);
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::GenerateTargetManifest(
                                              const std::string& config) const
{
  if (this->Target->IsImported())
    {
    return;
    }
  cmGlobalGenerator* gg = this->LocalGenerator->GetGlobalGenerator();

  // Get the names.
  std::string name;
  std::string soName;
  std::string realName;
  std::string impName;
  std::string pdbName;
  if(this->GetType() == cmTarget::EXECUTABLE)
    {
    this->GetExecutableNames(name, realName, impName, pdbName, config);
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
    gg->AddToManifest(f);
    }
  if(!soName.empty())
    {
    f = dir;
    f += "/";
    f += soName;
    gg->AddToManifest(f);
    }
  if(!realName.empty())
    {
    f = dir;
    f += "/";
    f += realName;
    gg->AddToManifest(f);
    }
  if(!pdbName.empty())
    {
    f = dir;
    f += "/";
    f += pdbName;
    gg->AddToManifest(f);
    }
  if(!impName.empty())
    {
    f = this->Target->GetDirectory(config, true);
    f += "/";
    f += impName;
    gg->AddToManifest(f);
    }
}

//----------------------------------------------------------------------------
std::string cmGeneratorTarget::GetFullPath(const std::string& config,
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

std::string cmGeneratorTarget::NormalGetFullPath(const std::string& config,
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
std::string
cmGeneratorTarget::NormalGetRealName(const std::string& config) const
{
  // This should not be called for imported targets.
  // TODO: Split cmTarget into a class hierarchy to get compile-time
  // enforcement of the limited imported target API.
  if(this->Target->IsImported())
    {
    std::string msg =  "NormalGetRealName called on imported target: ";
    msg += this->GetName();
    this->Makefile->IssueMessage(cmake::INTERNAL_ERROR, msg);
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
                               const std::string& config) const
{
  // This should not be called for imported targets.
  // TODO: Split cmTarget into a class hierarchy to get compile-time
  // enforcement of the limited imported target API.
  if(this->Target->IsImported())
    {
    std::string msg =  "GetLibraryNames called on imported target: ";
    msg += this->GetName();
    this->LocalGenerator->IssueMessage(cmake::INTERNAL_ERROR,
                                 msg);
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
    if(!this->Makefile->PlatformIsAppleIos())
      {
      realName += "Versions/";
      realName += this->Target->GetFrameworkVersion();
      realName += "/";
      }
    realName += base;
    soName = realName;
    }
  else
  {
    // The library's soname.
    this->Target->ComputeVersionedName(soName, prefix, base, suffix,
                              name, soversion);

    // The library's real name on disk.
    this->Target->ComputeVersionedName(realName, prefix, base, suffix,
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
                                  const std::string& config) const
{
  // This should not be called for imported targets.
  // TODO: Split cmTarget into a class hierarchy to get compile-time
  // enforcement of the limited imported target API.
  if(this->Target->IsImported())
    {
    std::string msg =
      "GetExecutableNames called on imported target: ";
    msg += this->GetName();
    this->LocalGenerator->IssueMessage(cmake::INTERNAL_ERROR, msg);
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
std::string cmGeneratorTarget::GetFullNameInternal(const std::string& config,
                                                   bool implib) const
{
  std::string prefix;
  std::string base;
  std::string suffix;
  this->GetFullNameInternal(config, implib, prefix, base, suffix);
  return prefix+base+suffix;
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::GetFullNameInternal(const std::string& config,
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
  if(!config.empty())
    {
    std::string configProp = cmSystemTools::UpperCase(config);
    configProp += "_POSTFIX";
    configPostfix = this->GetProperty(configProp);
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
  std::string ll = this->GetLinkerLanguage(config);
  if(!ll.empty())
    {
    if(!targetSuffix && suffixVar && *suffixVar)
      {
      std::string langSuff = suffixVar + std::string("_") + ll;
      targetSuffix = this->Makefile->GetDefinition(langSuff);
      }
    if(!targetPrefix && prefixVar && *prefixVar)
      {
      std::string langPrefix = prefixVar + std::string("_") + ll;
      targetPrefix = this->Makefile->GetDefinition(langPrefix);
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
    fw_prefix = this->GetOutputName(config, false);
    fw_prefix += ".framework/";
    targetPrefix = fw_prefix.c_str();
    targetSuffix = 0;
    }

  if(this->Target->IsCFBundleOnApple())
    {
    fw_prefix = this->GetCFBundleDirectory(config, false);
    fw_prefix += "/";
    targetPrefix = fw_prefix.c_str();
    targetSuffix = 0;
    }

  // Begin the final name with the prefix.
  outPrefix = targetPrefix?targetPrefix:"";

  // Append the target name or property-specified name.
  outBase += this->GetOutputName(config, implib);

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
std::string
cmGeneratorTarget::GetLinkerLanguage(const std::string& config) const
{
  return this->GetLinkClosure(config)->LinkerLanguage;
}

//----------------------------------------------------------------------------
std::string cmGeneratorTarget::GetPDBName(const std::string& config) const
{
  std::string prefix;
  std::string base;
  std::string suffix;
  this->GetFullNameInternal(config, false, prefix, base, suffix);

  std::vector<std::string> props;
  std::string configUpper =
    cmSystemTools::UpperCase(config);
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
    if(const char* outName = this->GetProperty(*i))
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
  int nameResult = strcmp(t1->GetName().c_str(), t2->GetName().c_str());
  if (nameResult == 0)
    {
    return strcmp(t1->GetMakefile()->GetCurrentBinaryDirectory(),
                  t2->GetMakefile()->GetCurrentBinaryDirectory()) < 0;
    }
  return nameResult < 0;
}

//----------------------------------------------------------------------------
struct cmGeneratorTarget::SourceFileFlags
cmGeneratorTarget::GetTargetSourceFileFlags(const cmSourceFile* sf) const
{
  struct SourceFileFlags flags;
  this->ConstructSourceFileFlags();
  std::map<cmSourceFile const*, SourceFileFlags>::iterator si =
    this->SourceFlagsMap.find(sf);
  if(si != this->SourceFlagsMap.end())
    {
    flags = si->second;
    }
  else
    {
    // Handle the MACOSX_PACKAGE_LOCATION property on source files that
    // were not listed in one of the other lists.
    if(const char* location = sf->GetProperty("MACOSX_PACKAGE_LOCATION"))
      {
      flags.MacFolder = location;
      if(strcmp(location, "Resources") == 0)
        {
        flags.Type = cmGeneratorTarget::SourceFileTypeResource;
        }
      else
        {
        flags.Type = cmGeneratorTarget::SourceFileTypeMacContent;
        }
      }
    }
  return flags;
}

//----------------------------------------------------------------------------
void cmGeneratorTarget::ConstructSourceFileFlags() const
{
  if(this->SourceFileFlagsConstructed)
    {
    return;
    }
  this->SourceFileFlagsConstructed = true;

  // Process public headers to mark the source files.
  if(const char* files = this->Target->GetProperty("PUBLIC_HEADER"))
    {
    std::vector<std::string> relFiles;
    cmSystemTools::ExpandListArgument(files, relFiles);
    for(std::vector<std::string>::iterator it = relFiles.begin();
        it != relFiles.end(); ++it)
      {
      if(cmSourceFile* sf = this->Makefile->GetSource(*it))
        {
        SourceFileFlags& flags = this->SourceFlagsMap[sf];
        flags.MacFolder = "Headers";
        flags.Type = cmGeneratorTarget::SourceFileTypePublicHeader;
        }
      }
    }

  // Process private headers after public headers so that they take
  // precedence if a file is listed in both.
  if(const char* files = this->Target->GetProperty("PRIVATE_HEADER"))
    {
    std::vector<std::string> relFiles;
    cmSystemTools::ExpandListArgument(files, relFiles);
    for(std::vector<std::string>::iterator it = relFiles.begin();
        it != relFiles.end(); ++it)
      {
      if(cmSourceFile* sf = this->Makefile->GetSource(*it))
        {
        SourceFileFlags& flags = this->SourceFlagsMap[sf];
        flags.MacFolder = "PrivateHeaders";
        flags.Type = cmGeneratorTarget::SourceFileTypePrivateHeader;
        }
      }
    }

  // Mark sources listed as resources.
  if(const char* files = this->Target->GetProperty("RESOURCE"))
    {
    std::vector<std::string> relFiles;
    cmSystemTools::ExpandListArgument(files, relFiles);
    for(std::vector<std::string>::iterator it = relFiles.begin();
        it != relFiles.end(); ++it)
      {
      if(cmSourceFile* sf = this->Makefile->GetSource(*it))
        {
        SourceFileFlags& flags = this->SourceFlagsMap[sf];
        flags.MacFolder = "Resources";
        flags.Type = cmGeneratorTarget::SourceFileTypeResource;
        }
      }
    }
}

//----------------------------------------------------------------------------
const cmGeneratorTarget::CompatibleInterfacesBase&
cmGeneratorTarget::GetCompatibleInterfaces(std::string const& config) const
{
  cmGeneratorTarget::CompatibleInterfaces& compat =
    this->CompatibleInterfacesMap[config];
  if(!compat.Done)
    {
    compat.Done = true;
    compat.PropsBool.insert("POSITION_INDEPENDENT_CODE");
    compat.PropsString.insert("AUTOUIC_OPTIONS");
    std::vector<cmTarget const*> const& deps =
      this->GetLinkImplementationClosure(config);
    for(std::vector<cmTarget const*>::const_iterator li = deps.begin();
        li != deps.end(); ++li)
      {
#define CM_READ_COMPATIBLE_INTERFACE(X, x) \
      if(const char* prop = (*li)->GetProperty("COMPATIBLE_INTERFACE_" #X)) \
        { \
        std::vector<std::string> props; \
        cmSystemTools::ExpandListArgument(prop, props); \
        compat.Props##x.insert(props.begin(), props.end()); \
        }
      CM_READ_COMPATIBLE_INTERFACE(BOOL, Bool)
      CM_READ_COMPATIBLE_INTERFACE(STRING, String)
      CM_READ_COMPATIBLE_INTERFACE(NUMBER_MIN, NumberMin)
      CM_READ_COMPATIBLE_INTERFACE(NUMBER_MAX, NumberMax)
#undef CM_READ_COMPATIBLE_INTERFACE
      }
    }
  return compat;
}

//----------------------------------------------------------------------------
bool cmGeneratorTarget::IsLinkInterfaceDependentBoolProperty(
    const std::string &p, const std::string& config) const
{
  if (this->Target->GetType() == cmTarget::OBJECT_LIBRARY
      || this->Target->GetType() == cmTarget::INTERFACE_LIBRARY)
    {
    return false;
    }
  return this->GetCompatibleInterfaces(config).PropsBool.count(p) > 0;
}

//----------------------------------------------------------------------------
bool cmGeneratorTarget::IsLinkInterfaceDependentStringProperty(
    const std::string &p, const std::string& config) const
{
  if (this->Target->GetType() == cmTarget::OBJECT_LIBRARY
      || this->Target->GetType() == cmTarget::INTERFACE_LIBRARY)
    {
    return false;
    }
  return this->GetCompatibleInterfaces(config).PropsString.count(p) > 0;
}

//----------------------------------------------------------------------------
bool cmGeneratorTarget::IsLinkInterfaceDependentNumberMinProperty(
    const std::string &p, const std::string& config) const
{
  if (this->Target->GetType() == cmTarget::OBJECT_LIBRARY
      || this->Target->GetType() == cmTarget::INTERFACE_LIBRARY)
    {
    return false;
    }
  return this->GetCompatibleInterfaces(config).PropsNumberMin.count(p) > 0;
}

//----------------------------------------------------------------------------
bool cmGeneratorTarget::IsLinkInterfaceDependentNumberMaxProperty(
    const std::string &p, const std::string& config) const
{
  if (this->Target->GetType() == cmTarget::OBJECT_LIBRARY
      || this->Target->GetType() == cmTarget::INTERFACE_LIBRARY)
    {
    return false;
    }
  return this->GetCompatibleInterfaces(config).PropsNumberMax.count(p) > 0;
}

enum CompatibleType
{
  BoolType,
  StringType,
  NumberMinType,
  NumberMaxType
};

template<typename PropertyType>
PropertyType getLinkInterfaceDependentProperty(cmGeneratorTarget const* tgt,
                                               const std::string& prop,
                                               const std::string& config,
                                               CompatibleType,
                                               PropertyType *);

template<>
bool getLinkInterfaceDependentProperty(cmGeneratorTarget const* tgt,
                                       const std::string& prop,
                                       const std::string& config,
                                       CompatibleType, bool *)
{
  return tgt->GetLinkInterfaceDependentBoolProperty(prop, config);
}

template<>
const char * getLinkInterfaceDependentProperty(cmGeneratorTarget const* tgt,
                                               const std::string& prop,
                                               const std::string& config,
                                               CompatibleType t,
                                               const char **)
{
  switch(t)
  {
  case BoolType:
    assert(0 && "String compatibility check function called for boolean");
    return 0;
  case StringType:
    return tgt->GetLinkInterfaceDependentStringProperty(prop, config);
  case NumberMinType:
    return tgt->GetLinkInterfaceDependentNumberMinProperty(prop, config);
  case NumberMaxType:
    return tgt->GetLinkInterfaceDependentNumberMaxProperty(prop, config);
  }
  assert(0 && "Unreachable!");
  return 0;
}

//----------------------------------------------------------------------------
template<typename PropertyType>
void checkPropertyConsistency(cmGeneratorTarget const* depender,
                              cmTarget const* dependee,
                              const std::string& propName,
                              std::set<std::string> &emitted,
                              const std::string& config,
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
      std::ostringstream e;
      e << "Target \"" << dependee->GetName() << "\" has property \""
        << *pi << "\" listed in its " << propName << " property.  "
          "This is not allowed.  Only user-defined properties may appear "
          "listed in the " << propName << " property.";
      depender->GetLocalGenerator()->IssueMessage(cmake::FATAL_ERROR, e.str());
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

static std::string intersect(const std::set<std::string> &s1,
                             const std::set<std::string> &s2)
{
  std::set<std::string> intersect;
  std::set_intersection(s1.begin(),s1.end(),
                        s2.begin(),s2.end(),
                      std::inserter(intersect,intersect.begin()));
  if (!intersect.empty())
    {
    return *intersect.begin();
    }
  return "";
}

static std::string intersect(const std::set<std::string> &s1,
                       const std::set<std::string> &s2,
                       const std::set<std::string> &s3)
{
  std::string result;
  result = intersect(s1, s2);
  if (!result.empty())
    return result;
  result = intersect(s1, s3);
  if (!result.empty())
    return result;
  return intersect(s2, s3);
}

static std::string intersect(const std::set<std::string> &s1,
                       const std::set<std::string> &s2,
                       const std::set<std::string> &s3,
                       const std::set<std::string> &s4)
{
  std::string result;
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
void cmGeneratorTarget::CheckPropertyCompatibility(
    cmComputeLinkInformation *info, const std::string& config) const
{
  const cmComputeLinkInformation::ItemVector &deps = info->GetItems();

  std::set<std::string> emittedBools;
  static std::string strBool = "COMPATIBLE_INTERFACE_BOOL";
  std::set<std::string> emittedStrings;
  static std::string strString = "COMPATIBLE_INTERFACE_STRING";
  std::set<std::string> emittedMinNumbers;
  static std::string strNumMin = "COMPATIBLE_INTERFACE_NUMBER_MIN";
  std::set<std::string> emittedMaxNumbers;
  static std::string strNumMax = "COMPATIBLE_INTERFACE_NUMBER_MAX";

  for(cmComputeLinkInformation::ItemVector::const_iterator li =
      deps.begin(); li != deps.end(); ++li)
    {
    if (!li->Target)
      {
      continue;
      }

    checkPropertyConsistency<bool>(this, li->Target,
                                strBool,
                                emittedBools, config, BoolType, 0);
    if (cmSystemTools::GetErrorOccuredFlag())
      {
      return;
      }
    checkPropertyConsistency<const char *>(this, li->Target,
                                strString,
                                emittedStrings, config,
                                StringType, 0);
    if (cmSystemTools::GetErrorOccuredFlag())
      {
      return;
      }
    checkPropertyConsistency<const char *>(this, li->Target,
                                strNumMin,
                                emittedMinNumbers, config,
                                NumberMinType, 0);
    if (cmSystemTools::GetErrorOccuredFlag())
      {
      return;
      }
    checkPropertyConsistency<const char *>(this, li->Target,
                                strNumMax,
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
    // Use a sorted std::vector to keep the error message sorted.
    std::vector<std::string> props;
    std::set<std::string>::const_iterator i = emittedBools.find(prop);
    if (i != emittedBools.end())
      {
      props.push_back(strBool);
      }
    i = emittedStrings.find(prop);
    if (i != emittedStrings.end())
      {
      props.push_back(strString);
      }
    i = emittedMinNumbers.find(prop);
    if (i != emittedMinNumbers.end())
      {
      props.push_back(strNumMin);
      }
    i = emittedMaxNumbers.find(prop);
    if (i != emittedMaxNumbers.end())
      {
      props.push_back(strNumMax);
      }
    std::sort(props.begin(), props.end());

    std::string propsString = cmJoin(cmMakeRange(props).retreat(1), ", ");
    propsString += " and the " + props.back();

    std::ostringstream e;
    e << "Property \"" << prop << "\" appears in both the "
      << propsString <<
    " property in the dependencies of target \"" << this->GetName() <<
    "\".  This is not allowed. A property may only require compatibility "
    "in a boolean interpretation, a numeric minimum, a numeric maximum or a "
    "string interpretation, but not a mixture.";
    this->LocalGenerator->IssueMessage(cmake::FATAL_ERROR, e.str());
    }
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
  assert(0 && "Unreachable!");
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
  assert(0 && "Unreachable!");
  return "";
}

//----------------------------------------------------------------------------
template<typename PropertyType>
PropertyType getTypedProperty(cmTarget const* tgt, const std::string& prop);

//----------------------------------------------------------------------------
template<>
bool getTypedProperty<bool>(cmTarget const* tgt, const std::string& prop)
{
  return tgt->GetPropertyAsBool(prop);
}

//----------------------------------------------------------------------------
template<>
const char *getTypedProperty<const char *>(cmTarget const* tgt,
                                           const std::string& prop)
{
  return tgt->GetProperty(prop);
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

//----------------------------------------------------------------------------
template<typename PropertyType>
std::pair<bool, PropertyType> consistentProperty(PropertyType lhs,
                                                 PropertyType rhs,
                                                 CompatibleType t);

//----------------------------------------------------------------------------
template<>
std::pair<bool, bool> consistentProperty(bool lhs, bool rhs,
                                         CompatibleType)
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

//----------------------------------------------------------------------------
std::pair<bool, const char*> consistentNumberProperty(const char *lhs,
                                                   const char *rhs,
                                                   CompatibleType t)
{
  char *pEnd;

  const char* const null_ptr = 0;

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

  if (t == NumberMaxType)
    {
    return std::make_pair(true, std::max(lnum, rnum) == lnum ? lhs : rhs);
    }
  else
    {
    return std::make_pair(true, std::min(lnum, rnum) == lnum ? lhs : rhs);
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

  const char* const null_ptr = 0;

  switch(t)
  {
  case BoolType:
    assert(0 && "consistentProperty for strings called with BoolType");
    return std::pair<bool, const char*>(false, null_ptr);
  case StringType:
    return consistentStringProperty(lhs, rhs);
  case NumberMinType:
  case NumberMaxType:
    return consistentNumberProperty(lhs, rhs, t);
  }
  assert(0 && "Unreachable!");
  return std::pair<bool, const char*>(false, null_ptr);
}

//----------------------------------------------------------------------------
template<typename PropertyType>
PropertyType checkInterfacePropertyCompatibility(cmGeneratorTarget const* tgt,
                                          const std::string &p,
                                          const std::string& config,
                                          const char *defaultValue,
                                          CompatibleType t,
                                          PropertyType *)
{
  PropertyType propContent = getTypedProperty<PropertyType>(tgt->Target, p);
  const bool explicitlySet = tgt->Target->GetProperties()
                                  .find(p)
                                  != tgt->Target->GetProperties().end();
  const bool impliedByUse =
          tgt->Target->IsNullImpliedByLinkLibraries(p);
  assert((impliedByUse ^ explicitlySet)
      || (!impliedByUse && !explicitlySet));

  std::vector<cmTarget const*> const& deps =
    tgt->GetLinkImplementationClosure(config);

  if(deps.empty())
    {
    return propContent;
    }
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

  std::string interfaceProperty = "INTERFACE_" + p;
  for(std::vector<cmTarget const*>::const_iterator li =
      deps.begin();
      li != deps.end(); ++li)
    {
    // An error should be reported if one dependency
    // has INTERFACE_POSITION_INDEPENDENT_CODE ON and the other
    // has INTERFACE_POSITION_INDEPENDENT_CODE OFF, or if the
    // target itself has a POSITION_INDEPENDENT_CODE which disagrees
    // with a dependency.

    cmTarget const* theTarget = *li;

    const bool ifaceIsSet = theTarget->GetProperties()
                            .find(interfaceProperty)
                            != theTarget->GetProperties().end();
    PropertyType ifacePropContent =
                    getTypedProperty<PropertyType>(theTarget,
                              interfaceProperty);

    std::string reportEntry;
    if (ifaceIsSet)
      {
      reportEntry += " * Target \"";
      reportEntry += theTarget->GetName();
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
          std::ostringstream e;
          e << "Property " << p << " on target \""
            << tgt->GetName() << "\" does\nnot match the "
            "INTERFACE_" << p << " property requirement\nof "
            "dependency \"" << theTarget->GetName() << "\".\n";
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
          std::ostringstream e;
          e << "Property " << p << " on target \""
            << tgt->GetName() << "\" is\nimplied to be " << defaultValue
            << " because it was used to determine the link libraries\n"
               "already. The INTERFACE_" << p << " property on\ndependency \""
            << theTarget->GetName() << "\" is in conflict.\n";
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
            std::ostringstream e;
            e << "The INTERFACE_" << p << " property of \""
              << theTarget->GetName() << "\" does\nnot agree with the value "
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
bool cmGeneratorTarget::GetLinkInterfaceDependentBoolProperty(
    const std::string &p, const std::string& config) const
{
  return checkInterfacePropertyCompatibility<bool>(this, p, config,
                                                   "FALSE",
                                                   BoolType, 0);
}

//----------------------------------------------------------------------------
const char* cmGeneratorTarget::GetLinkInterfaceDependentStringProperty(
                                              const std::string &p,
                                              const std::string& config) const
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
                                              const std::string& config) const
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
                                              const std::string& config) const
{
  return checkInterfacePropertyCompatibility<const char *>(this,
                                                           p,
                                                           config,
                                                           "empty",
                                                   NumberMaxType, 0);
}

//----------------------------------------------------------------------------
cmComputeLinkInformation*
cmGeneratorTarget::GetLinkInformation(const std::string& config) const
{
  // Lookup any existing information for this configuration.
  std::string key(cmSystemTools::UpperCase(config));
  cmTargetLinkInformationMap::iterator
    i = this->LinkInformation.find(key);
  if(i == this->LinkInformation.end())
    {
    // Compute information for this configuration.
    cmComputeLinkInformation* info =
      new cmComputeLinkInformation(this, config);
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
void
cmGeneratorTarget::ReportPropertyOrigin(const std::string &p,
                               const std::string &result,
                               const std::string &report,
                               const std::string &compatibilityType) const
{
  std::vector<std::string> debugProperties;
  const char *debugProp = this->Target->GetMakefile()
      ->GetDefinition("CMAKE_DEBUG_TARGET_PROPERTIES");
  if (debugProp)
    {
    cmSystemTools::ExpandListArgument(debugProp, debugProperties);
    }

  bool debugOrigin = !this->DebugCompatiblePropertiesDone[p]
                    && std::find(debugProperties.begin(),
                                 debugProperties.end(),
                                 p)
                        != debugProperties.end();

  if (this->Target->GetMakefile()->IsConfigured())
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

  this->Makefile->GetCMakeInstance()->IssueMessage(cmake::LOG, areport);
}
