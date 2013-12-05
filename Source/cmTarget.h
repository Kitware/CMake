/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmTarget_h
#define cmTarget_h

#include "cmCustomCommand.h"
#include "cmPropertyMap.h"
#include "cmPolicies.h"
#include "cmListFileCache.h"

#include <cmsys/auto_ptr.hxx>

#define CM_FOR_EACH_TARGET_POLICY(F) \
  F(CMP0003) \
  F(CMP0004) \
  F(CMP0008) \
  F(CMP0020) \
  F(CMP0021) \
  F(CMP0022) \
  F(CMP0041)

class cmake;
class cmMakefile;
class cmSourceFile;
class cmGlobalGenerator;
class cmComputeLinkInformation;
class cmListFileBacktrace;
class cmTarget;
class cmGeneratorTarget;
class cmTargetTraceDependencies;

struct cmTargetLinkInformationMap:
  public std::map<std::pair<cmTarget const* , std::string>,
                  cmComputeLinkInformation*>
{
  typedef std::map<std::pair<cmTarget const* , std::string>,
                   cmComputeLinkInformation*> derived;
  cmTargetLinkInformationMap() {}
  cmTargetLinkInformationMap(cmTargetLinkInformationMap const& r);
  ~cmTargetLinkInformationMap();
};

class cmTargetInternals;
class cmTargetInternalPointer
{
public:
  cmTargetInternalPointer();
  cmTargetInternalPointer(cmTargetInternalPointer const& r);
  ~cmTargetInternalPointer();
  cmTargetInternalPointer& operator=(cmTargetInternalPointer const& r);
  cmTargetInternals* operator->() const { return this->Pointer; }
  cmTargetInternals* Get() const { return this->Pointer; }
private:
  cmTargetInternals* Pointer;
};

/** \class cmTarget
 * \brief Represent a library or executable target loaded from a makefile.
 *
 * cmTarget represents a target loaded from
 * a makefile.
 */
class cmTarget
{
public:
  cmTarget();
  enum TargetType { EXECUTABLE, STATIC_LIBRARY,
                    SHARED_LIBRARY, MODULE_LIBRARY,
                    OBJECT_LIBRARY, UTILITY, GLOBAL_TARGET,
                    INTERFACE_LIBRARY,
                    UNKNOWN_LIBRARY};
  static const char* GetTargetTypeName(TargetType targetType);
  enum CustomCommandType { PRE_BUILD, PRE_LINK, POST_BUILD };

  /**
   * Return the type of target.
   */
  TargetType GetType() const
    {
    return this->TargetTypeValue;
    }

  /**
   * Set the target type
   */
  void SetType(TargetType f, const char* name);

  void MarkAsImported();

  ///! Set/Get the name of the target
  const char* GetName() const {return this->Name.c_str();}
  const char* GetExportName() const;

  ///! Set the cmMakefile that owns this target
  void SetMakefile(cmMakefile *mf);
  cmMakefile *GetMakefile() const { return this->Makefile;};

#define DECLARE_TARGET_POLICY(POLICY) \
  cmPolicies::PolicyStatus GetPolicyStatus ## POLICY () const \
    { return this->PolicyStatus ## POLICY; }

  CM_FOR_EACH_TARGET_POLICY(DECLARE_TARGET_POLICY)

#undef DECLARE_TARGET_POLICY

  /**
   * Get the list of the custom commands for this target
   */
  std::vector<cmCustomCommand> const &GetPreBuildCommands() const
    {return this->PreBuildCommands;}
  std::vector<cmCustomCommand> const &GetPreLinkCommands() const
    {return this->PreLinkCommands;}
  std::vector<cmCustomCommand> const &GetPostBuildCommands() const
    {return this->PostBuildCommands;}
  void AddPreBuildCommand(cmCustomCommand const &cmd)
    {this->PreBuildCommands.push_back(cmd);}
  void AddPreLinkCommand(cmCustomCommand const &cmd)
    {this->PreLinkCommands.push_back(cmd);}
  void AddPostBuildCommand(cmCustomCommand const &cmd)
    {this->PostBuildCommands.push_back(cmd);}

  /**
   * Get the list of the source files used by this target
   */
  std::vector<cmSourceFile*> const& GetSourceFiles() const;
  void AddSourceFile(cmSourceFile* sf);
  std::vector<std::string> const& GetObjectLibraries() const
    {
    return this->ObjectLibraries;
    }

  /**
   * Flags for a given source file as used in this target. Typically assigned
   * via SET_TARGET_PROPERTIES when the property is a list of source files.
   */
  enum SourceFileType
  {
    SourceFileTypeNormal,
    SourceFileTypePrivateHeader, // is in "PRIVATE_HEADER" target property
    SourceFileTypePublicHeader,  // is in "PUBLIC_HEADER" target property
    SourceFileTypeResource,      // is in "RESOURCE" target property *or*
                                 // has MACOSX_PACKAGE_LOCATION=="Resources"
    SourceFileTypeMacContent     // has MACOSX_PACKAGE_LOCATION!="Resources"
  };
  struct SourceFileFlags
  {
    SourceFileFlags(): Type(SourceFileTypeNormal), MacFolder(0) {}
    SourceFileFlags(SourceFileFlags const& r):
      Type(r.Type), MacFolder(r.MacFolder) {}
    SourceFileType Type;
    const char* MacFolder; // location inside Mac content folders
  };

  /**
   * Get the flags for a given source file as used in this target
   */
  struct SourceFileFlags
  GetTargetSourceFileFlags(const cmSourceFile* sf) const;

  /**
   * Add sources to the target.
   */
  void AddSources(std::vector<std::string> const& srcs);
  cmSourceFile* AddSource(const char* src);

  enum LinkLibraryType {GENERAL, DEBUG, OPTIMIZED};

  //* how we identify a library, by name and type
  typedef std::pair<cmStdString, LinkLibraryType> LibraryID;

  typedef std::vector<LibraryID > LinkLibraryVectorType;
  const LinkLibraryVectorType &GetLinkLibraries() const {
  return this->LinkLibraries;}
  const LinkLibraryVectorType &GetOriginalLinkLibraries() const
    {return this->OriginalLinkLibraries;}
  void GetDirectLinkLibraries(const char *config,
                              std::vector<std::string> &,
                              cmTarget const* head) const;
  void GetInterfaceLinkLibraries(const char *config,
                              std::vector<std::string> &,
                              cmTarget *head) const;

  /** Compute the link type to use for the given configuration.  */
  LinkLibraryType ComputeLinkType(const char* config) const;

  /**
   * Clear the dependency information recorded for this target, if any.
   */
  void ClearDependencyInformation(cmMakefile& mf, const char* target);

  // Check to see if a library is a framework and treat it different on Mac
  bool NameResolvesToFramework(const std::string& libname) const;
  void AddLinkLibrary(cmMakefile& mf,
                      const char *target, const char* lib,
                      LinkLibraryType llt);
  enum TLLSignature {
    KeywordTLLSignature,
    PlainTLLSignature
  };
  bool PushTLLCommandTrace(TLLSignature signature);
  void GetTllSignatureTraces(cmOStringStream &s, TLLSignature sig) const;

  void MergeLinkLibraries( cmMakefile& mf, const char* selfname,
                           const LinkLibraryVectorType& libs );

  const std::vector<std::string>& GetLinkDirectories() const;

  void AddLinkDirectory(const char* d);

  /**
   * Set the path where this target should be installed. This is relative to
   * INSTALL_PREFIX
   */
  std::string GetInstallPath() const {return this->InstallPath;}
  void SetInstallPath(const char *name) {this->InstallPath = name;}

  /**
   * Set the path where this target (if it has a runtime part) should be
   * installed. This is relative to INSTALL_PREFIX
   */
  std::string GetRuntimeInstallPath() const {return this->RuntimeInstallPath;}
  void SetRuntimeInstallPath(const char *name) {
    this->RuntimeInstallPath = name; }

  /**
   * Get/Set whether there is an install rule for this target.
   */
  bool GetHaveInstallRule() const { return this->HaveInstallRule; }
  void SetHaveInstallRule(bool h) { this->HaveInstallRule = h; }

  /** Add a utility on which this project depends. A utility is an executable
   * name as would be specified to the ADD_EXECUTABLE or UTILITY_SOURCE
   * commands. It is not a full path nor does it have an extension.
   */
  void AddUtility(const char* u) { this->Utilities.insert(u);}
  ///! Get the utilities used by this target
  std::set<cmStdString>const& GetUtilities() const { return this->Utilities; }

  /** Finalize the target at the end of the Configure step.  */
  void FinishConfigure();

  ///! Set/Get a property of this target file
  void SetProperty(const char *prop, const char *value);
  void AppendProperty(const char* prop, const char* value,bool asString=false);
  const char *GetProperty(const char *prop) const;
  const char *GetProperty(const char *prop, cmProperty::ScopeType scope) const;
  bool GetPropertyAsBool(const char *prop) const;
  void CheckProperty(const char* prop, cmMakefile* context) const;

  const char* GetFeature(const char* feature, const char* config) const;

  bool IsImported() const {return this->IsImportedTarget;}

  /** The link interface specifies transitive library dependencies and
      other information needed by targets that link to this target.  */
  struct LinkInterface
  {
    // Languages whose runtime libraries must be linked.
    std::vector<std::string> Languages;

    // Libraries listed in the interface.
    std::vector<std::string> Libraries;

    // Shared library dependencies needed for linking on some platforms.
    std::vector<std::string> SharedDeps;

    // Number of repetitions of a strongly connected component of two
    // or more static libraries.
    int Multiplicity;

    // Libraries listed for other configurations.
    // Needed only for OLD behavior of CMP0003.
    std::vector<std::string> WrongConfigLibraries;

    bool ImplementationIsInterface;

    LinkInterface(): Multiplicity(0), ImplementationIsInterface(false) {}
  };

  /** Get the link interface for the given configuration.  Returns 0
      if the target cannot be linked.  */
  LinkInterface const* GetLinkInterface(const char* config,
                                        cmTarget const* headTarget) const;
  void GetTransitivePropertyLinkLibraries(const char* config,
                                        cmTarget const* headTarget,
                                        std::vector<std::string> &libs) const;

  /** The link implementation specifies the direct library
      dependencies needed by the object files of the target.  */
  struct LinkImplementation
  {
    // Languages whose runtime libraries must be linked.
    std::vector<std::string> Languages;

    // Libraries linked directly in this configuration.
    std::vector<std::string> Libraries;

    // Libraries linked directly in other configurations.
    // Needed only for OLD behavior of CMP0003.
    std::vector<std::string> WrongConfigLibraries;
  };
  LinkImplementation const* GetLinkImplementation(const char* config,
                                                  cmTarget const* head) const;

  /** Link information from the transitive closure of the link
      implementation and the interfaces of its dependencies.  */
  struct LinkClosure
  {
    // The preferred linker language.
    std::string LinkerLanguage;

    // Languages whose runtime libraries must be linked.
    std::vector<std::string> Languages;
  };
  LinkClosure const* GetLinkClosure(const char* config,
                                    cmTarget const* head) const;

  /** Strip off leading and trailing whitespace from an item named in
      the link dependencies of this target.  */
  std::string CheckCMP0004(std::string const& item) const;

  /** Get the directory in which this target will be built.  If the
      configuration name is given then the generator will add its
      subdirectory for that configuration.  Otherwise just the canonical
      output directory is given.  */
  std::string GetDirectory(const char* config = 0, bool implib = false) const;

  /** Get the directory in which this targets .pdb files will be placed.
      If the configuration name is given then the generator will add its
      subdirectory for that configuration.  Otherwise just the canonical
      pdb output directory is given.  */
  std::string GetPDBDirectory(const char* config = 0) const;

  /** Get the location of the target in the build tree for the given
      configuration.  This location is suitable for use as the LOCATION
      target property.  */
  const char* GetLocation(const char* config) const;

  /** Get the target major and minor version numbers interpreted from
      the VERSION property.  Version 0 is returned if the property is
      not set or cannot be parsed.  */
  void GetTargetVersion(int& major, int& minor) const;

  /** Get the target major, minor, and patch version numbers
      interpreted from the VERSION or SOVERSION property.  Version 0
      is returned if the property is not set or cannot be parsed.  */
  void
  GetTargetVersion(bool soversion, int& major, int& minor, int& patch) const;

  /**
   * Make sure the full path to all source files is known.
   */
  bool FindSourceFiles();

  ///! Return the preferred linker language for this target
  const char* GetLinkerLanguage(const char* config = 0,
                                cmTarget const* head = 0) const;

  /** Get the full name of the target according to the settings in its
      makefile.  */
  std::string GetFullName(const char* config=0, bool implib = false) const;
  void GetFullNameComponents(std::string& prefix,
                             std::string& base, std::string& suffix,
                             const char* config=0, bool implib = false) const;

  /** Get the name of the pdb file for the target.  */
  std::string GetPDBName(const char* config=0) const;

  /** Whether this library has soname enabled and platform supports it.  */
  bool HasSOName(const char* config) const;

  /** Get the soname of the target.  Allowed only for a shared library.  */
  std::string GetSOName(const char* config) const;

  /** Whether this library has \@rpath and platform supports it.  */
  bool HasMacOSXRpath(const char* config) const;

  /** Test for special case of a third-party shared library that has
      no soname at all.  */
  bool IsImportedSharedLibWithoutSOName(const char* config) const;

  /** Get the full path to the target according to the settings in its
      makefile and the configuration type.  */
  std::string GetFullPath(const char* config=0, bool implib = false,
                          bool realname = false) const;

  /** Get the names of the library needed to generate a build rule
      that takes into account shared library version numbers.  This
      should be called only on a library target.  */
  void GetLibraryNames(std::string& name, std::string& soName,
                       std::string& realName, std::string& impName,
                       std::string& pdbName, const char* config) const;

  /** Get the names of the executable needed to generate a build rule
      that takes into account executable version numbers.  This should
      be called only on an executable target.  */
  void GetExecutableNames(std::string& name, std::string& realName,
                          std::string& impName,
                          std::string& pdbName, const char* config) const;

  /** Does this target have a GNU implib to convert to MS format?  */
  bool HasImplibGNUtoMS() const;

  /** Convert the given GNU import library name (.dll.a) to a name with a new
      extension (.lib or ${CMAKE_IMPORT_LIBRARY_SUFFIX}).  */
  bool GetImplibGNUtoMS(std::string const& gnuName, std::string& out,
                        const char* newExt = 0) const;

  /**
   * Compute whether this target must be relinked before installing.
   */
  bool NeedRelinkBeforeInstall(const char* config) const;

  bool HaveBuildTreeRPATH(const char *config) const;
  bool HaveInstallTreeRPATH() const;

  /** Return true if builtin chrpath will work for this target */
  bool IsChrpathUsed(const char* config) const;

  /** Return the install name directory for the target in the
    * build tree.  For example: "\@rpath/", "\@loader_path/",
    * or "/full/path/to/library".  */
  std::string GetInstallNameDirForBuildTree(const char* config) const;

  /** Return the install name directory for the target in the
    * install tree.  For example: "\@rpath/" or "\@loader_path/". */
  std::string GetInstallNameDirForInstallTree() const;

  cmComputeLinkInformation* GetLinkInformation(const char* config,
                                               cmTarget const* head = 0) const;

  // Get the properties
  cmPropertyMap &GetProperties() const { return this->Properties; };

  bool GetMappedConfig(std::string const& desired_config,
                       const char** loc,
                       const char** imp,
                       std::string& suffix) const;

  // Define the properties
  static void DefineProperties(cmake *cm);

  /** Get the macro to define when building sources in this target.
      If no macro should be defined null is returned.  */
  const char* GetExportMacro() const;

  void GetCompileDefinitions(std::vector<std::string> &result,
                             const char *config) const;

  // Compute the set of languages compiled by the target.  This is
  // computed every time it is called because the languages can change
  // when source file properties are changed and we do not have enough
  // information to forward these property changes to the targets
  // until we have per-target object file properties.
  void GetLanguages(std::set<cmStdString>& languages) const;

  /** Return whether this target is an executable with symbol exports
      enabled.  */
  bool IsExecutableWithExports() const;

  /** Return whether this target may be used to link another target.  */
  bool IsLinkable() const;

  /** Return whether or not the target is for a DLL platform.  */
  bool IsDLLPlatform() const { return this->DLLPlatform; }

  /** Return whether or not the target has a DLL import library.  */
  bool HasImportLibrary() const;

  /** Return whether this target is a shared library Framework on
      Apple.  */
  bool IsFrameworkOnApple() const;

  /** Return whether this target is a CFBundle (plugin) on Apple.  */
  bool IsCFBundleOnApple() const;

  /** Return whether this target is an executable Bundle on Apple.  */
  bool IsAppBundleOnApple() const;

  /** Return whether this target is an executable Bundle, a framework
      or CFBundle on Apple.  */
  bool IsBundleOnApple() const;

  /** Return the framework version string.  Undefined if
      IsFrameworkOnApple returns false.  */
  std::string GetFrameworkVersion() const;

  /** Get a backtrace from the creation of the target.  */
  cmListFileBacktrace const& GetBacktrace() const;

  /** Get a build-tree directory in which to place target support files.  */
  std::string GetSupportDirectory() const;

  /** Return whether this target uses the default value for its output
      directory.  */
  bool UsesDefaultOutputDir(const char* config, bool implib) const;

  /** @return the mac content directory for this target. */
  std::string GetMacContentDirectory(const char* config,
                                     bool implib) const;

  /** @return whether this target have a well defined output file name. */
  bool HaveWellDefinedOutputFiles() const;

  /** @return the Mac framework directory without the base. */
  std::string GetFrameworkDirectory(const char* config, bool rootDir) const;

  /** @return the Mac CFBundle directory without the base */
  std::string GetCFBundleDirectory(const char* config, bool contentOnly) const;

  /** @return the Mac App directory without the base */
  std::string GetAppBundleDirectory(const char* config,
                                    bool contentOnly) const;

  std::vector<std::string> GetIncludeDirectories(const char *config) const;
  void InsertInclude(const cmValueWithOrigin &entry,
                     bool before = false);
  void InsertCompileOption(const cmValueWithOrigin &entry,
                     bool before = false);
  void InsertCompileDefinition(const cmValueWithOrigin &entry,
                     bool before = false);

  void AppendBuildInterfaceIncludes();

  void GetCompileOptions(std::vector<std::string> &result,
                         const char *config) const;
  void GetAutoUicOptions(std::vector<std::string> &result,
                         const char *config) const;

  bool IsNullImpliedByLinkLibraries(const std::string &p) const;
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

  std::string GetDebugGeneratorExpressions(const std::string &value,
                                  cmTarget::LinkLibraryType llt) const;

  void AddSystemIncludeDirectories(const std::set<cmStdString> &incs);
  void AddSystemIncludeDirectories(const std::vector<std::string> &incs);
  std::set<cmStdString> const & GetSystemIncludeDirectories() const
    { return this->SystemIncludeDirectories; }

  void FinalizeSystemIncludeDirectories();

  bool LinkLanguagePropagatesToDependents() const
  { return this->TargetTypeValue == STATIC_LIBRARY; }

private:
  bool HandleLocationPropertyPolicy() const;

  // The set of include directories that are marked as system include
  // directories.
  std::set<cmStdString> SystemIncludeDirectories;

  std::vector<std::pair<TLLSignature, cmListFileBacktrace> > TLLCommands;

  /**
   * A list of direct dependencies. Use in conjunction with DependencyMap.
   */
  typedef std::vector< LibraryID > DependencyList;

  /**
   * This map holds the dependency graph. map[x] returns a set of
   * direct dependencies of x. Note that the direct depenencies are
   * ordered. This is necessary to handle direct dependencies that
   * themselves have no dependency information.
   */
  typedef std::map< LibraryID, DependencyList > DependencyMap;

  /**
   * Inserts \a dep at the end of the dependency list of \a lib.
   */
  void InsertDependency( DependencyMap& depMap,
                         const LibraryID& lib,
                         const LibraryID& dep);

  /*
   * Deletes \a dep from the dependency list of \a lib.
   */
  void DeleteDependency( DependencyMap& depMap,
                         const LibraryID& lib,
                         const LibraryID& dep);

  /**
   * Emits the library \a lib and all its dependencies into link_line.
   * \a emitted keeps track of the libraries that have been emitted to
   * avoid duplicates--it is more efficient than searching
   * link_line. \a visited is used detect cycles. Note that \a
   * link_line is in reverse order, in that the dependencies of a
   * library are listed before the library itself.
   */
  void Emit( const LibraryID lib,
             const DependencyMap& dep_map,
             std::set<LibraryID>& emitted,
             std::set<LibraryID>& visited,
             DependencyList& link_line);

  /**
   * Finds the dependencies for \a lib and inserts them into \a
   * dep_map.
   */
  void GatherDependencies( const cmMakefile& mf,
                           const LibraryID& lib,
                           DependencyMap& dep_map);

  void AnalyzeLibDependencies( const cmMakefile& mf );

  const char* GetSuffixVariableInternal(bool implib) const;
  const char* GetPrefixVariableInternal(bool implib) const;
  std::string GetFullNameInternal(const char* config, bool implib) const;
  void GetFullNameInternal(const char* config, bool implib,
                           std::string& outPrefix, std::string& outBase,
                           std::string& outSuffix) const;

  // Use a makefile variable to set a default for the given property.
  // If the variable is not defined use the given default instead.
  void SetPropertyDefault(const char* property, const char* default_value);

  // Returns ARCHIVE, LIBRARY, or RUNTIME based on platform and type.
  const char* GetOutputTargetType(bool implib) const;

  // Get the target base name.
  std::string GetOutputName(const char* config, bool implib) const;

  const char* ImportedGetLocation(const char* config) const;
  const char* NormalGetLocation(const char* config) const;

  std::string GetFullNameImported(const char* config, bool implib) const;

  std::string ImportedGetFullPath(const char* config, bool implib) const;
  std::string NormalGetFullPath(const char* config, bool implib,
                                bool realname) const;

  /** Get the real name of the target.  Allowed only for non-imported
      targets.  When a library or executable file is versioned this is
      the full versioned name.  If the target is not versioned this is
      the same as GetFullName.  */
  std::string NormalGetRealName(const char* config) const;

  /** Append to @a base the mac content directory and return it. */
  std::string BuildMacContentDirectory(const std::string& base,
                                       const char* config,
                                       bool contentOnly) const;

private:
  std::string Name;
  std::vector<cmCustomCommand> PreBuildCommands;
  std::vector<cmCustomCommand> PreLinkCommands;
  std::vector<cmCustomCommand> PostBuildCommands;
  TargetType TargetTypeValue;
  std::vector<cmSourceFile*> SourceFiles;
  std::vector<std::string> ObjectLibraries;
  LinkLibraryVectorType LinkLibraries;
  LinkLibraryVectorType PrevLinkedLibraries;
  bool LinkLibrariesAnalyzed;
  std::vector<std::string> LinkDirectories;
  std::set<cmStdString> LinkDirectoriesEmmitted;
  bool HaveInstallRule;
  std::string InstallPath;
  std::string RuntimeInstallPath;
  mutable std::string ExportMacro;
  std::set<cmStdString> Utilities;
  bool RecordDependencies;
  mutable cmPropertyMap Properties;
  LinkLibraryVectorType OriginalLinkLibraries;
  bool DLLPlatform;
  bool IsApple;
  bool IsImportedTarget;
  mutable bool DebugIncludesDone;
  mutable bool DebugCompileOptionsDone;
  mutable bool DebugAutoUicOptionsDone;
  mutable bool DebugCompileDefinitionsDone;
  mutable std::set<std::string> LinkImplicitNullProperties;
  bool BuildInterfaceIncludesAppended;

  // Cache target output paths for each configuration.
  struct OutputInfo;
  OutputInfo const* GetOutputInfo(const char* config) const;
  bool
  ComputeOutputDir(const char* config, bool implib, std::string& out) const;
  bool ComputePDBOutputDir(const char* config, std::string& out) const;

  // Cache import information from properties for each configuration.
  struct ImportInfo;
  ImportInfo const* GetImportInfo(const char* config,
                                        cmTarget const* workingTarget) const;
  void ComputeImportInfo(std::string const& desired_config, ImportInfo& info,
                                        cmTarget const* head) const;

  mutable cmTargetLinkInformationMap LinkInformation;
  void CheckPropertyCompatibility(cmComputeLinkInformation *info,
                                  const char* config) const;

  bool ComputeLinkInterface(const char* config, LinkInterface& iface,
                                        cmTarget const* head) const;

  void ComputeLinkImplementation(const char* config,
                                 LinkImplementation& impl,
                                 cmTarget const* head) const;
  void ComputeLinkClosure(const char* config, LinkClosure& lc,
                          cmTarget const* head) const;

  void ClearLinkMaps();

  void MaybeInvalidatePropertyCache(const char* prop);

  void ProcessSourceExpression(std::string const& expr);

  // The cmMakefile instance that owns this target.  This should
  // always be set.
  cmMakefile* Makefile;

  // Policy status recorded when target was created.
#define TARGET_POLICY_MEMBER(POLICY) \
  cmPolicies::PolicyStatus PolicyStatus ## POLICY;

  CM_FOR_EACH_TARGET_POLICY(TARGET_POLICY_MEMBER)

#undef TARGET_POLICY_MEMBER

  // Internal representation details.
  friend class cmTargetInternals;
  friend class cmGeneratorTarget;
  friend class cmTargetTraceDependencies;
  cmTargetInternalPointer Internal;

  void ConstructSourceFileFlags() const;
  void ComputeVersionedName(std::string& vName,
                            std::string const& prefix,
                            std::string const& base,
                            std::string const& suffix,
                            std::string const& name,
                            const char* version) const;
};

typedef std::map<cmStdString,cmTarget> cmTargets;

class cmTargetSet: public std::set<cmStdString> {};
class cmTargetManifest: public std::map<cmStdString, cmTargetSet> {};

#endif
