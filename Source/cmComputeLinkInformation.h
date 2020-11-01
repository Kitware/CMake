/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "cmsys/RegularExpression.hxx"

#include "cmListFileCache.h"

class cmGeneratorTarget;
class cmGlobalGenerator;
class cmMakefile;
class cmOrderDirectories;
class cmake;

/** \class cmComputeLinkInformation
 * \brief Compute link information for a target in one configuration.
 */
class cmComputeLinkInformation
{
public:
  cmComputeLinkInformation(cmGeneratorTarget const* target,
                           const std::string& config);
  cmComputeLinkInformation(const cmComputeLinkInformation&) = delete;
  cmComputeLinkInformation& operator=(const cmComputeLinkInformation&) =
    delete;
  ~cmComputeLinkInformation();
  bool Compute();

  struct Item
  {
    Item() = default;
    Item(BT<std::string> v, bool p, cmGeneratorTarget const* target = nullptr)
      : Value(std::move(v))
      , IsPath(p)
      , Target(target)
    {
    }
    BT<std::string> Value;
    bool IsPath = true;
    cmGeneratorTarget const* Target = nullptr;
  };
  using ItemVector = std::vector<Item>;
  void AppendValues(std::string& result, std::vector<BT<std::string>>& values);
  ItemVector const& GetItems() const;
  std::vector<std::string> const& GetDirectories() const;
  std::vector<BT<std::string>> GetDirectoriesWithBacktraces();
  std::vector<std::string> const& GetDepends() const;
  std::vector<std::string> const& GetFrameworkPaths() const;
  std::string GetLinkLanguage() const { return this->LinkLanguage; }
  std::vector<std::string> const& GetRuntimeSearchPath() const;
  std::string const& GetRuntimeFlag() const { return this->RuntimeFlag; }
  std::string const& GetRuntimeSep() const { return this->RuntimeSep; }
  void GetRPath(std::vector<std::string>& runtimeDirs, bool for_install) const;
  std::string GetRPathString(bool for_install) const;
  std::string GetChrpathString() const;
  std::set<cmGeneratorTarget const*> const& GetSharedLibrariesLinked() const;

  std::string const& GetLibLinkFileFlag() const
  {
    return this->LibLinkFileFlag;
  }

  std::string const& GetRPathLinkFlag() const { return this->RPathLinkFlag; }
  std::string GetRPathLinkString() const;

  std::string GetConfig() const { return this->Config; }

  const cmGeneratorTarget* GetTarget() { return this->Target; }

private:
  void AddItem(BT<std::string> const& item, const cmGeneratorTarget* tgt);
  void AddSharedDepItem(BT<std::string> const& item,
                        cmGeneratorTarget const* tgt);

  // Output information.
  ItemVector Items;
  std::vector<std::string> Directories;
  std::vector<std::string> Depends;
  std::vector<std::string> FrameworkPaths;
  std::vector<std::string> RuntimeSearchPath;
  std::set<cmGeneratorTarget const*> SharedLibrariesLinked;

  // Context information.
  cmGeneratorTarget const* const Target;
  cmMakefile* const Makefile;
  cmGlobalGenerator* const GlobalGenerator;
  cmake* const CMakeInstance;

  // Configuration information.
  std::string const Config;
  std::string LinkLanguage;

  // Modes for dealing with dependent shared libraries.
  enum SharedDepMode
  {
    SharedDepModeNone,   // Drop
    SharedDepModeDir,    // List dir in -rpath-link flag
    SharedDepModeLibDir, // List dir in linker search path
    SharedDepModeLink    // List file on link line
  };

  const char* LoaderFlag;
  std::string LibLinkFlag;
  std::string LibLinkFileFlag;
  std::string LibLinkSuffix;
  std::string RuntimeFlag;
  std::string RuntimeSep;
  std::string RuntimeAlways;
  std::string RPathLinkFlag;
  SharedDepMode SharedDependencyMode;

  enum LinkType
  {
    LinkUnknown,
    LinkStatic,
    LinkShared
  };
  void SetCurrentLinkType(LinkType lt);

  // Link type adjustment.
  void ComputeLinkTypeInfo();
  LinkType StartLinkType;
  LinkType CurrentLinkType;
  std::string StaticLinkTypeFlag;
  std::string SharedLinkTypeFlag;

  // Link item parsing.
  void ComputeItemParserInfo();
  std::vector<std::string> StaticLinkExtensions;
  std::vector<std::string> SharedLinkExtensions;
  std::vector<std::string> LinkExtensions;
  std::set<std::string> LinkPrefixes;
  cmsys::RegularExpression ExtractStaticLibraryName;
  cmsys::RegularExpression ExtractSharedLibraryName;
  cmsys::RegularExpression ExtractAnyLibraryName;
  std::string SharedRegexString;
  void AddLinkPrefix(std::string const& p);
  void AddLinkExtension(std::string const& e, LinkType type);
  std::string CreateExtensionRegex(std::vector<std::string> const& exts,
                                   LinkType type);
  std::string NoCaseExpression(std::string const& str);

  // Handling of link items.
  void AddTargetItem(BT<std::string> const& item,
                     const cmGeneratorTarget* target);
  void AddFullItem(BT<std::string> const& item);
  bool CheckImplicitDirItem(std::string const& item);
  void AddUserItem(BT<std::string> const& item, bool pathNotKnown);
  void AddDirectoryItem(std::string const& item);
  void AddFrameworkItem(std::string const& item);
  void DropDirectoryItem(std::string const& item);
  bool CheckSharedLibNoSOName(std::string const& item);
  void AddSharedLibNoSOName(std::string const& item);
  void HandleBadFullItem(std::string const& item, std::string const& file);

  // Framework info.
  void ComputeFrameworkInfo();
  void AddFrameworkPath(std::string const& p);
  std::set<std::string> FrameworkPathsEmmitted;
  cmsys::RegularExpression SplitFramework;

  // Linker search path computation.
  std::unique_ptr<cmOrderDirectories> OrderLinkerSearchPath;
  bool FinishLinkerSearchDirectories();
  void PrintLinkPolicyDiagnosis(std::ostream&);

  // Implicit link libraries and directories for linker language.
  void LoadImplicitLinkInfo();
  void AddImplicitLinkInfo();
  void AddImplicitLinkInfo(std::string const& lang);
  void AddRuntimeLinkLibrary(std::string const& lang);
  std::set<std::string> ImplicitLinkDirs;
  std::set<std::string> ImplicitLinkLibs;

  // Additional paths configured by the runtime linker
  std::vector<std::string> RuntimeLinkDirs;

  // Linker search path compatibility mode.
  std::set<std::string> OldLinkDirMask;
  std::vector<std::string> OldLinkDirItems;
  std::vector<std::string> OldUserFlagItems;
  std::set<std::string> CMP0060WarnItems;
  // Dependent library path computation.
  std::unique_ptr<cmOrderDirectories> OrderDependentRPath;
  // Runtime path computation.
  std::unique_ptr<cmOrderDirectories> OrderRuntimeSearchPath;

  bool OldLinkDirMode;
  bool OpenBSD;
  bool LinkDependsNoShared;
  bool RuntimeUseChrpath;
  bool NoSONameUsesPath;
  bool LinkWithRuntimePath;
  bool LinkTypeEnabled;
  bool ArchivesMayBeShared;
  bool CMP0060Warn;

  void AddLibraryRuntimeInfo(std::string const& fullPath,
                             const cmGeneratorTarget* target);
  void AddLibraryRuntimeInfo(std::string const& fullPath);
};
