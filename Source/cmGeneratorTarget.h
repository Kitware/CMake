/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2012 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGeneratorTarget_h
#define cmGeneratorTarget_h

#include "cmStandardIncludes.h"
#include "cmGeneratorExpression.h"

class cmComputeLinkInformation;
class cmCustomCommand;
class cmGlobalGenerator;
class cmLocalGenerator;
class cmMakefile;
class cmSourceFile;
class cmTarget;

struct cmTargetLinkInformationMap:
  public std::map<std::pair<cmTarget const*, std::string>,
                           cmComputeLinkInformation*>
{
  typedef std::map<std::pair<cmTarget const*, std::string>,
                   cmComputeLinkInformation*> derived;
  cmTargetLinkInformationMap() {}
  cmTargetLinkInformationMap(cmTargetLinkInformationMap const& r);
  ~cmTargetLinkInformationMap();
};

class cmGeneratorTarget
{
public:
  cmGeneratorTarget(cmTarget*);
  ~cmGeneratorTarget();

  bool IsImported() const;
  const char *GetLocation(const char* config) const;
  const char *NormalGetLocation(const char* config) const;

  int GetType() const;
  const char *GetName() const;
  const char *GetProperty(const char *prop) const;
  bool GetPropertyAsBool(const char *prop) const;
  void GetSourceFiles(std::vector<cmSourceFile*>& files) const;

  void GetObjectSources(std::vector<cmSourceFile*> &) const;
  const std::string& GetObjectName(cmSourceFile const* file);

  void AddObject(cmSourceFile *sf, std::string const&name);
  bool HasExplicitObjectName(cmSourceFile const* file) const;
  void AddExplicitObjectName(cmSourceFile* sf);

  void GetResxSources(std::vector<cmSourceFile*>&) const;
  void GetIDLSources(std::vector<cmSourceFile*>&) const;
  void GetExternalObjects(std::vector<cmSourceFile*>&) const;
  void GetHeaderSources(std::vector<cmSourceFile*>&) const;
  void GetExtraSources(std::vector<cmSourceFile*>&) const;
  void GetCustomCommands(std::vector<cmSourceFile*>&) const;
  void GetExpectedResxHeaders(std::set<std::string>&) const;

  /** Get the full path to the target according to the settings in its
      makefile and the configuration type.  */
  std::string GetFullPath(const char* config=0, bool implib = false,
                          bool realname = false) const;
  std::string NormalGetFullPath(const char* config, bool implib,
                                bool realname) const;
  std::string NormalGetRealName(const char* config) const;

  /** Get the full name of the target according to the settings in its
      makefile.  */
  std::string GetFullName(const char* config=0, bool implib = false) const;

  /** @return the Mac framework directory without the base. */
  std::string GetFrameworkDirectory(const char* config, bool rootDir) const;

  /** @return the Mac CFBundle directory without the base */
  std::string GetCFBundleDirectory(const char* config, bool contentOnly) const;

  /** @return the Mac App directory without the base */
  std::string GetAppBundleDirectory(const char* config,
                                    bool contentOnly) const;

  /** Return the install name directory for the target in the
    * build tree.  For example: "\@rpath/", "\@loader_path/",
    * or "/full/path/to/library".  */
  std::string GetInstallNameDirForBuildTree(const char* config) const;

  /** Return the install name directory for the target in the
    * install tree.  For example: "\@rpath/" or "\@loader_path/". */
  std::string GetInstallNameDirForInstallTree() const;

  /** Get the soname of the target.  Allowed only for a shared library.  */
  std::string GetSOName(const char* config) const;

  void GetFullNameComponents(std::string& prefix,
                             std::string& base, std::string& suffix,
                             const char* config=0, bool implib = false) const;

  /** Append to @a base the mac content directory and return it. */
  std::string BuildMacContentDirectory(const std::string& base,
                                       const char* config = 0,
                                       bool contentOnly = true) const;

  /** @return the mac content directory for this target. */
  std::string GetMacContentDirectory(const char* config = 0,
                                     bool implib = false) const;

  cmTarget* Target;
  cmMakefile* Makefile;
  cmLocalGenerator* LocalGenerator;
  cmGlobalGenerator* GlobalGenerator;

  std::string ModuleDefinitionFile;

  /** Full path with trailing slash to the top-level directory
      holding object files for this target.  Includes the build
      time config name placeholder if needed for the generator.  */
  std::string ObjectDirectory;

  void UseObjectLibraries(std::vector<std::string>& objs) const;

  mutable cmTargetLinkInformationMap LinkInformation;

  cmComputeLinkInformation* GetLinkInformation(const char* config,
                                               cmTarget const* head = 0) const;

  bool IsLinkInterfaceDependentBoolProperty(const std::string &p,
                                            const char *config) const;
  bool IsLinkInterfaceDependentStringProperty(const std::string &p,
                                              const char *config) const;
  bool IsLinkInterfaceDependentNumberMinProperty(const std::string &p,
                                                 const char *config) const;
  bool IsLinkInterfaceDependentNumberMaxProperty(const std::string &p,
                                                 const char *config) const;

  bool GetLinkInterfaceDependentBoolProperty(const std::string &p,
                                             const char *config) const;
  const char *GetLinkInterfaceDependentStringProperty(const std::string &p,
                                                    const char *config) const;
  const char *GetLinkInterfaceDependentNumberMinProperty(const std::string &p,
                                                    const char *config) const;
  const char *GetLinkInterfaceDependentNumberMaxProperty(const std::string &p,
                                                    const char *config) const;

  void CheckPropertyCompatibility(cmComputeLinkInformation *info,
                                  const char* config) const;

  cmMakefile* GetMakefile() const;

  void GetAppleArchs(const char* config,
                     std::vector<std::string>& archVec) const;

  ///! Return the rule variable used to create this type of target,
  //  need to add CMAKE_(LANG) for full name.
  const char* GetCreateRuleVariable() const;

  /** Get the include directories for this target.  */
  std::vector<std::string> GetIncludeDirectories(const char *config) const;

  bool IsSystemIncludeDirectory(const char *dir, const char *config) const;

  /** Add the target output files to the global generator manifest.  */
  void GenerateTargetManifest(const char* config) const;

  /**
   * Trace through the source files in this target and add al source files
   * that they depend on, used by all generators
   */
  void TraceDependencies();

  void ClassifySources();
  void LookupObjectLibraries();

  /** Get sources that must be built before the given source.  */
  std::vector<cmSourceFile*> const* GetSourceDepends(cmSourceFile* sf) const;

  /** Get the name of the pdb file for the target.  */
  std::string GetPDBName(const char* config=0) const;

  /** Whether this library has soname enabled and platform supports it.  */
  bool HasSOName(const char* config) const;

  struct TargetPropertyEntry {
    TargetPropertyEntry(cmsys::auto_ptr<cmCompiledGeneratorExpression> cge,
      const std::string &targetName = std::string())
      : ge(cge), TargetName(targetName)
    {}
    const cmsys::auto_ptr<cmCompiledGeneratorExpression> ge;
    std::vector<std::string> CachedEntries;
    const std::string TargetName;
  };

  void ReportPropertyOrigin(const std::string &p,
                            const std::string &result,
                            const std::string &report,
                            const std::string &compatibilityType) const;

  void GetAutoUicOptions(std::vector<std::string> &result,
                         const char *config) const;

  /** Get the names of the executable needed to generate a build rule
      that takes into account executable version numbers.  This should
      be called only on an executable target.  */
  void GetExecutableNames(std::string& name, std::string& realName,
                          std::string& impName,
                          std::string& pdbName, const char* config) const;

  /** Get the names of the library needed to generate a build rule
      that takes into account shared library version numbers.  This
      should be called only on a library target.  */
  void GetLibraryNames(std::string& name, std::string& soName,
                       std::string& realName, std::string& impName,
                       std::string& pdbName, const char* config) const;

  /**
   * Compute whether this target must be relinked before installing.
   */
  bool NeedRelinkBeforeInstall(const char* config) const;

  /** Return true if builtin chrpath will work for this target */
  bool IsChrpathUsed(const char* config) const;

  ///! Return the preferred linker language for this target
  const char* GetLinkerLanguage(const char* config = 0,
                                cmTarget const* head = 0) const;

private:
  friend class cmTargetTraceDependencies;
  struct SourceEntry { std::vector<cmSourceFile*> Depends; };
  typedef std::map<cmSourceFile*, SourceEntry> SourceEntriesType;
  SourceEntriesType SourceEntries;
  std::vector<cmSourceFile*> CustomCommands;
  std::vector<cmSourceFile*> ExtraSources;
  std::vector<cmSourceFile*> HeaderSources;
  std::vector<cmSourceFile*> ExternalObjects;
  std::vector<cmSourceFile*> IDLSources;
  std::vector<cmSourceFile*> ResxSources;
  std::map<cmSourceFile const*, std::string> Objects;
  std::set<cmSourceFile const*> ExplicitObjectName;
  std::set<std::string> ExpectedResxHeaders;
  std::vector<cmSourceFile*> ObjectSources;
  std::vector<cmTarget*> ObjectLibraries;
  mutable std::map<std::string, std::vector<std::string> > SystemIncludesCache;
  mutable bool DebugIncludesDone;
  mutable std::map<std::string, std::vector<TargetPropertyEntry*> >
                                CachedLinkInterfaceIncludeDirectoriesEntries;
  mutable std::map<std::string, bool> CacheLinkInterfaceIncludeDirectoriesDone;
  mutable std::map<std::string, bool> DebugCompatiblePropertiesDone;

  std::string GetFullNameInternal(const char* config, bool implib) const;
  void GetFullNameInternal(const char* config, bool implib,
                           std::string& outPrefix, std::string& outBase,
                           std::string& outSuffix) const;

  cmGeneratorTarget(cmGeneratorTarget const&);
  void operator=(cmGeneratorTarget const&);
};

struct cmStrictTargetComparison {
  bool operator()(cmTarget const* t1, cmTarget const* t2) const;
};

typedef std::map<cmTarget const*,
                 cmGeneratorTarget*,
                 cmStrictTargetComparison> cmGeneratorTargetsType;

#endif
