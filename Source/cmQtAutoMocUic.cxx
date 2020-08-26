/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmQtAutoMocUic.h"

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <cm/memory>
#include <cm/string_view>
#include <cmext/algorithm>

#include "cmsys/FStream.hxx"
#include "cmsys/RegularExpression.hxx"

#include "cm_jsoncpp_value.h"

#include "cmCryptoHash.h"
#include "cmFileTime.h"
#include "cmGccDepfileReader.h"
#include "cmGccDepfileReaderTypes.h"
#include "cmGeneratedFileStream.h"
#include "cmQtAutoGen.h"
#include "cmQtAutoGenerator.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmWorkerPool.h"

#if defined(__APPLE__)
#  include <unistd.h>
#endif

namespace {

constexpr std::size_t MocUnderscoreLength = 4; // Length of "moc_"
constexpr std::size_t UiUnderscoreLength = 3;  // Length of "ui_"

/** \class cmQtAutoMocUicT
 * \brief AUTOMOC and AUTOUIC generator
 */
class cmQtAutoMocUicT : public cmQtAutoGenerator
{
public:
  cmQtAutoMocUicT();
  ~cmQtAutoMocUicT() override;

  cmQtAutoMocUicT(cmQtAutoMocUicT const&) = delete;
  cmQtAutoMocUicT& operator=(cmQtAutoMocUicT const&) = delete;

public:
  // -- Types

  /** Include string with sub parts.  */
  struct IncludeKeyT
  {
    IncludeKeyT(std::string const& key, std::size_t basePrefixLength);

    std::string Key;  // Full include string
    std::string Dir;  // Include directory
    std::string Base; // Base part of the include file name
  };

  /** Search key plus regular expression pair.  */
  struct KeyExpT
  {
    KeyExpT(std::string key, std::string const& exp)
      : Key(std::move(key))
      , Exp(exp)
    {
    }

    std::string Key;
    cmsys::RegularExpression Exp;
  };

  /** Source file parsing cache.  */
  class ParseCacheT
  {
  public:
    // -- Types

    /** Entry of the file parsing cache.  */
    struct FileT
    {
      void Clear();

      struct MocT
      {
        std::string Macro;
        struct IncludeT
        {
          std::vector<IncludeKeyT> Underscore;
          std::vector<IncludeKeyT> Dot;
        } Include;
        std::vector<std::string> Depends;
      } Moc;

      struct UicT
      {
        std::vector<IncludeKeyT> Include;
        std::vector<std::string> Depends;
      } Uic;
    };
    using FileHandleT = std::shared_ptr<FileT>;
    using GetOrInsertT = std::pair<FileHandleT, bool>;

  public:
    ParseCacheT();
    ~ParseCacheT();

    bool ReadFromFile(std::string const& fileName);
    bool WriteToFile(std::string const& fileName);

    //! Always returns a valid handle
    GetOrInsertT GetOrInsert(std::string const& fileName);

  private:
    std::unordered_map<std::string, FileHandleT> Map_;
  };

  /** Source file data.  */
  class SourceFileT
  {
  public:
    SourceFileT(std::string fileName)
      : FileName(std::move(fileName))
    {
    }

  public:
    std::string FileName;
    cmFileTime FileTime;
    ParseCacheT::FileHandleT ParseData;
    std::string BuildPath;
    bool IsHeader = false;
    bool Moc = false;
    bool Uic = false;
  };
  using SourceFileHandleT = std::shared_ptr<SourceFileT>;
  using SourceFileMapT = std::map<std::string, SourceFileHandleT>;

  /** Meta compiler file mapping information.  */
  struct MappingT
  {
    SourceFileHandleT SourceFile;
    std::string OutputFile;
    std::string IncludeString;
    std::vector<SourceFileHandleT> IncluderFiles;
  };
  using MappingHandleT = std::shared_ptr<MappingT>;
  using MappingMapT = std::map<std::string, MappingHandleT>;

  /** Common settings.  */
  class BaseSettingsT
  {
  public:
    // -- Constructors
    BaseSettingsT();
    ~BaseSettingsT();

    BaseSettingsT(BaseSettingsT const&) = delete;
    BaseSettingsT& operator=(BaseSettingsT const&) = delete;

    // -- Attributes
    // - Config
    bool MultiConfig = false;
    IntegerVersion QtVersion = { 4, 0 };
    unsigned int ThreadCount = 0;
    // - Directories
    std::string AutogenBuildDir;
    std::string AutogenIncludeDir;
    // - Files
    std::string CMakeExecutable;
    cmFileTime CMakeExecutableTime;
    std::string ParseCacheFile;
    std::string DepFile;
    std::string DepFileRuleName;
    std::vector<std::string> HeaderExtensions;
    std::vector<std::string> ListFiles;
  };

  /** Shared common variables.  */
  class BaseEvalT
  {
  public:
    // -- Parse Cache
    bool ParseCacheChanged = false;
    cmFileTime ParseCacheTime;
    ParseCacheT ParseCache;

    // -- Sources
    SourceFileMapT Headers;
    SourceFileMapT Sources;
  };

  /** Moc settings.  */
  class MocSettingsT
  {
  public:
    // -- Constructors
    MocSettingsT();
    ~MocSettingsT();

    MocSettingsT(MocSettingsT const&) = delete;
    MocSettingsT& operator=(MocSettingsT const&) = delete;

    // -- Const methods
    bool skipped(std::string const& fileName) const;
    std::string MacrosString() const;

    // -- Attributes
    bool Enabled = false;
    bool SettingsChanged = false;
    bool RelaxedMode = false;
    bool PathPrefix = false;
    bool CanOutputDependencies = false;
    cmFileTime ExecutableTime;
    std::string Executable;
    std::string CompFileAbs;
    std::string PredefsFileAbs;
    std::unordered_set<std::string> SkipList;
    std::vector<std::string> IncludePaths;
    std::vector<std::string> Definitions;
    std::vector<std::string> OptionsIncludes;
    std::vector<std::string> OptionsDefinitions;
    std::vector<std::string> OptionsExtra;
    std::vector<std::string> PredefsCmd;
    std::vector<KeyExpT> DependFilters;
    std::vector<KeyExpT> MacroFilters;
    cmsys::RegularExpression RegExpInclude;
  };

  /** Moc shared variables.  */
  class MocEvalT
  {
  public:
    // -- predefines file
    cmFileTime PredefsTime;
    // -- Mappings
    MappingMapT HeaderMappings;
    MappingMapT SourceMappings;
    MappingMapT Includes;
    // -- Discovered files
    SourceFileMapT HeadersDiscovered;
    // -- Output directories
    std::unordered_set<std::string> OutputDirs;
    // -- Mocs compilation
    bool CompUpdated = false;
    std::vector<std::string> CompFiles;
  };

  /** Uic settings.  */
  class UicSettingsT
  {
  public:
    struct UiFile
    {
      std::vector<std::string> Options;
    };

  public:
    UicSettingsT();
    ~UicSettingsT();

    UicSettingsT(UicSettingsT const&) = delete;
    UicSettingsT& operator=(UicSettingsT const&) = delete;

    // -- Const methods
    bool skipped(std::string const& fileName) const;

    // -- Attributes
    bool Enabled = false;
    bool SettingsChanged = false;
    cmFileTime ExecutableTime;
    std::string Executable;
    std::unordered_set<std::string> SkipList;
    std::vector<std::string> Options;
    std::unordered_map<std::string, UiFile> UiFiles;
    std::vector<std::string> SearchPaths;
    cmsys::RegularExpression RegExpInclude;
  };

  /** Uic shared variables.  */
  class UicEvalT
  {
  public:
    // -- Discovered files
    SourceFileMapT UiFiles;
    // -- Mappings
    MappingMapT Includes;
    // -- Output directories
    std::unordered_set<std::string> OutputDirs;
  };

  /** Abstract job class for concurrent job processing.  */
  class JobT : public cmWorkerPool::JobT
  {
  protected:
    /** Protected default constructor.  */
    JobT(bool fence = false)
      : cmWorkerPool::JobT(fence)
    {
    }

    //! Get the generator. Only valid during Process() call!
    cmQtAutoMocUicT* Gen() const
    {
      return static_cast<cmQtAutoMocUicT*>(UserData());
    };

    // -- Accessors. Only valid during Process() call!
    Logger const& Log() const { return Gen()->Log(); }
    BaseSettingsT const& BaseConst() const { return Gen()->BaseConst(); }
    BaseEvalT& BaseEval() const { return Gen()->BaseEval(); }
    MocSettingsT const& MocConst() const { return Gen()->MocConst(); }
    MocEvalT& MocEval() const { return Gen()->MocEval(); }
    UicSettingsT const& UicConst() const { return Gen()->UicConst(); }
    UicEvalT& UicEval() const { return Gen()->UicEval(); }

    // -- Logging
    std::string MessagePath(cm::string_view path) const
    {
      return Gen()->MessagePath(path);
    }
    // - Error logging with automatic abort
    void LogError(GenT genType, cm::string_view message) const;
    void LogCommandError(GenT genType, cm::string_view message,
                         std::vector<std::string> const& command,
                         std::string const& output) const;

    /** @brief Run an external process. Use only during Process() call!  */
    bool RunProcess(GenT genType, cmWorkerPool::ProcessResultT& result,
                    std::vector<std::string> const& command,
                    std::string* infoMessage = nullptr);
  };

  /** Fence job utility class.  */
  class JobFenceT : public JobT
  {
  public:
    JobFenceT()
      : JobT(true)
    {
    }
    void Process() override{};
  };

  /** Generate moc_predefs.h.  */
  class JobMocPredefsT : public JobFenceT
  {
    void Process() override;
    bool Update(std::string* reason) const;
  };

  /** File parse job base class.  */
  class JobParseT : public JobT
  {
  public:
    JobParseT(SourceFileHandleT fileHandle)
      : FileHandle(std::move(fileHandle))
    {
    }

  protected:
    bool ReadFile();
    void CreateKeys(std::vector<IncludeKeyT>& container,
                    std::set<std::string> const& source,
                    std::size_t basePrefixLength);
    void MocMacro();
    void MocDependecies();
    void MocIncludes();
    void UicIncludes();

  protected:
    SourceFileHandleT FileHandle;
    std::string Content;
  };

  /** Header file parse job.  */
  class JobParseHeaderT : public JobParseT
  {
  public:
    using JobParseT::JobParseT;
    void Process() override;
  };

  /** Source file parse job.  */
  class JobParseSourceT : public JobParseT
  {
  public:
    using JobParseT::JobParseT;
    void Process() override;
  };

  /** Evaluate cached file parse data - moc.  */
  class JobEvalCacheT : public JobT
  {
  protected:
    std::string MessageSearchLocations() const;
    std::vector<std::string> SearchLocations;
  };

  /** Evaluate cached file parse data - moc.  */
  class JobEvalCacheMocT : public JobEvalCacheT
  {
    void Process() override;
    bool EvalHeader(SourceFileHandleT source);
    bool EvalSource(SourceFileHandleT const& source);
    bool FindIncludedHeader(SourceFileHandleT& headerHandle,
                            cm::string_view includerDir,
                            cm::string_view includeBase);
    bool RegisterIncluded(std::string const& includeString,
                          SourceFileHandleT includerFileHandle,
                          SourceFileHandleT sourceFileHandle) const;
    void RegisterMapping(MappingHandleT mappingHandle) const;
    std::string MessageHeader(cm::string_view headerBase) const;
  };

  /** Evaluate cached file parse data - uic.  */
  class JobEvalCacheUicT : public JobEvalCacheT
  {
    void Process() override;
    bool EvalFile(SourceFileHandleT const& sourceFileHandle);
    bool FindIncludedUi(cm::string_view sourceDirPrefix,
                        cm::string_view includePrefix);
    bool RegisterMapping(std::string const& includeString,
                         SourceFileHandleT includerFileHandle);

    std::string UiName;
    SourceFileHandleT UiFileHandle;
  };

  /** Evaluate cached file parse data - finish  */
  class JobEvalCacheFinishT : public JobFenceT
  {
    void Process() override;
  };

  /** Dependency probing base job.  */
  class JobProbeDepsT : public JobT
  {
  };

  /** Probes file dependencies and generates moc compile jobs.  */
  class JobProbeDepsMocT : public JobProbeDepsT
  {
    void Process() override;
    bool Generate(MappingHandleT const& mapping, bool compFile) const;
    bool Probe(MappingT const& mapping, std::string* reason) const;
    std::pair<std::string, cmFileTime> FindDependency(
      std::string const& sourceDir, std::string const& includeString) const;
  };

  /** Probes file dependencies and generates uic compile jobs.  */
  class JobProbeDepsUicT : public JobProbeDepsT
  {
    void Process() override;
    bool Probe(MappingT const& mapping, std::string* reason) const;
  };

  /** Dependency probing finish job.  */
  class JobProbeDepsFinishT : public JobFenceT
  {
    void Process() override;
  };

  /** Meta compiler base job.  */
  class JobCompileT : public JobT
  {
  public:
    JobCompileT(MappingHandleT uicMapping, std::unique_ptr<std::string> reason)
      : Mapping(std::move(uicMapping))
      , Reason(std::move(reason))
    {
    }

  protected:
    MappingHandleT Mapping;
    std::unique_ptr<std::string> Reason;
  };

  /** moc compiles a file.  */
  class JobCompileMocT : public JobCompileT
  {
  public:
    JobCompileMocT(MappingHandleT uicMapping,
                   std::unique_ptr<std::string> reason,
                   ParseCacheT::FileHandleT cacheEntry)
      : JobCompileT(std::move(uicMapping), std::move(reason))
      , CacheEntry(std::move(cacheEntry))
    {
    }
    void Process() override;

  protected:
    ParseCacheT::FileHandleT CacheEntry;
  };

  /** uic compiles a file.  */
  class JobCompileUicT : public JobCompileT
  {
  public:
    using JobCompileT::JobCompileT;
    void Process() override;
  };

  /** Generate mocs_compilation.cpp.  */
  class JobMocsCompilationT : public JobFenceT
  {
  private:
    void Process() override;
  };

  class JobDepFilesMergeT : public JobFenceT
  {
  private:
    void Process() override;
  };

  /** @brief The last job.  */
  class JobFinishT : public JobFenceT
  {
  private:
    void Process() override;
  };

  // -- Const settings interface
  BaseSettingsT const& BaseConst() const { return this->BaseConst_; }
  BaseEvalT& BaseEval() { return this->BaseEval_; }
  MocSettingsT const& MocConst() const { return this->MocConst_; }
  MocEvalT& MocEval() { return this->MocEval_; }
  UicSettingsT const& UicConst() const { return this->UicConst_; }
  UicEvalT& UicEval() { return this->UicEval_; }

  // -- Parallel job processing interface
  cmWorkerPool& WorkerPool() { return WorkerPool_; }
  void AbortError() { Abort(true); }
  void AbortSuccess() { Abort(false); }

  // -- Utility
  std::string AbsoluteBuildPath(cm::string_view relativePath) const;
  std::string AbsoluteIncludePath(cm::string_view relativePath) const;
  template <class JOBTYPE>
  void CreateParseJobs(SourceFileMapT const& sourceMap);
  std::string CollapseFullPathTS(std::string const& path) const;

private:
  // -- Abstract processing interface
  bool InitFromInfo(InfoT const& info) override;
  void InitJobs();
  bool Process() override;
  // -- Settings file
  void SettingsFileRead();
  bool SettingsFileWrite();
  // -- Parse cache
  void ParseCacheRead();
  bool ParseCacheWrite();
  // -- Thread processing
  void Abort(bool error);
  // -- Generation
  bool CreateDirectories();
  // -- Support for depfiles
  static std::vector<std::string> dependenciesFromDepFile(
    const char* filePath);

private:
  // -- Settings
  BaseSettingsT BaseConst_;
  BaseEvalT BaseEval_;
  MocSettingsT MocConst_;
  MocEvalT MocEval_;
  UicSettingsT UicConst_;
  UicEvalT UicEval_;
  // -- Settings file
  std::string SettingsFile_;
  std::string SettingsStringMoc_;
  std::string SettingsStringUic_;
  // -- Worker thread pool
  std::atomic<bool> JobError_ = ATOMIC_VAR_INIT(false);
  cmWorkerPool WorkerPool_;
  // -- Concurrent processing
  mutable std::mutex CMakeLibMutex_;
};

cmQtAutoMocUicT::IncludeKeyT::IncludeKeyT(std::string const& key,
                                          std::size_t basePrefixLength)
  : Key(key)
  , Dir(SubDirPrefix(key))
  , Base(cmSystemTools::GetFilenameWithoutLastExtension(key))
{
  if (basePrefixLength != 0) {
    Base = Base.substr(basePrefixLength);
  }
}

void cmQtAutoMocUicT::ParseCacheT::FileT::Clear()
{
  Moc.Macro.clear();
  Moc.Include.Underscore.clear();
  Moc.Include.Dot.clear();
  Moc.Depends.clear();

  Uic.Include.clear();
  Uic.Depends.clear();
}

cmQtAutoMocUicT::ParseCacheT::GetOrInsertT
cmQtAutoMocUicT::ParseCacheT::GetOrInsert(std::string const& fileName)
{
  // Find existing entry
  {
    auto it = Map_.find(fileName);
    if (it != Map_.end()) {
      return GetOrInsertT{ it->second, false };
    }
  }

  // Insert new entry
  return GetOrInsertT{
    Map_.emplace(fileName, std::make_shared<FileT>()).first->second, true
  };
}

cmQtAutoMocUicT::ParseCacheT::ParseCacheT() = default;
cmQtAutoMocUicT::ParseCacheT::~ParseCacheT() = default;

bool cmQtAutoMocUicT::ParseCacheT::ReadFromFile(std::string const& fileName)
{
  cmsys::ifstream fin(fileName.c_str());
  if (!fin) {
    return false;
  }
  FileHandleT fileHandle;

  std::string line;
  while (std::getline(fin, line)) {
    // Check if this an empty or a comment line
    if (line.empty() || line.front() == '#') {
      continue;
    }
    // Drop carriage return character at the end
    if (line.back() == '\r') {
      line.pop_back();
      if (line.empty()) {
        continue;
      }
    }
    // Check if this a file name line
    if (line.front() != ' ') {
      fileHandle = GetOrInsert(line).first;
      continue;
    }

    // Bad line or bad file handle
    if (!fileHandle || (line.size() < 6)) {
      continue;
    }

    constexpr std::size_t offset = 5;
    if (cmHasLiteralPrefix(line, " mmc:")) {
      fileHandle->Moc.Macro = line.substr(offset);
      continue;
    }
    if (cmHasLiteralPrefix(line, " miu:")) {
      fileHandle->Moc.Include.Underscore.emplace_back(line.substr(offset),
                                                      MocUnderscoreLength);
      continue;
    }
    if (cmHasLiteralPrefix(line, " mid:")) {
      fileHandle->Moc.Include.Dot.emplace_back(line.substr(offset), 0);
      continue;
    }
    if (cmHasLiteralPrefix(line, " mdp:")) {
      fileHandle->Moc.Depends.emplace_back(line.substr(offset));
      continue;
    }
    if (cmHasLiteralPrefix(line, " uic:")) {
      fileHandle->Uic.Include.emplace_back(line.substr(offset),
                                           UiUnderscoreLength);
      continue;
    }
    if (cmHasLiteralPrefix(line, " udp:")) {
      fileHandle->Uic.Depends.emplace_back(line.substr(offset));
      continue;
    }
  }
  return true;
}

bool cmQtAutoMocUicT::ParseCacheT::WriteToFile(std::string const& fileName)
{
  cmGeneratedFileStream ofs(fileName);
  if (!ofs) {
    return false;
  }
  ofs << "# Generated by CMake. Changes will be overwritten." << std::endl;
  for (auto const& pair : Map_) {
    ofs << pair.first << std::endl;
    FileT const& file = *pair.second;
    if (!file.Moc.Macro.empty()) {
      ofs << " mmc:" << file.Moc.Macro << std::endl;
    }
    for (IncludeKeyT const& item : file.Moc.Include.Underscore) {
      ofs << " miu:" << item.Key << std::endl;
    }
    for (IncludeKeyT const& item : file.Moc.Include.Dot) {
      ofs << " mid:" << item.Key << std::endl;
    }
    for (std::string const& item : file.Moc.Depends) {
      ofs << " mdp:" << item << std::endl;
    }
    for (IncludeKeyT const& item : file.Uic.Include) {
      ofs << " uic:" << item.Key << std::endl;
    }
    for (std::string const& item : file.Uic.Depends) {
      ofs << " udp:" << item << std::endl;
    }
  }
  return ofs.Close();
}

cmQtAutoMocUicT::BaseSettingsT::BaseSettingsT() = default;
cmQtAutoMocUicT::BaseSettingsT::~BaseSettingsT() = default;

cmQtAutoMocUicT::MocSettingsT::MocSettingsT()
{
  RegExpInclude.compile(
    "(^|\n)[ \t]*#[ \t]*include[ \t]+"
    "[\"<](([^ \">]+/)?moc_[^ \">/]+\\.cpp|[^ \">]+\\.moc)[\">]");
}

cmQtAutoMocUicT::MocSettingsT::~MocSettingsT() = default;

bool cmQtAutoMocUicT::MocSettingsT::skipped(std::string const& fileName) const
{
  return (!Enabled || (SkipList.find(fileName) != SkipList.end()));
}

std::string cmQtAutoMocUicT::MocSettingsT::MacrosString() const
{
  std::string res;
  const auto itB = MacroFilters.cbegin();
  const auto itE = MacroFilters.cend();
  const auto itL = itE - 1;
  auto itC = itB;
  for (; itC != itE; ++itC) {
    // Separator
    if (itC != itB) {
      if (itC != itL) {
        res += ", ";
      } else {
        res += " or ";
      }
    }
    // Key
    res += itC->Key;
  }
  return res;
}

cmQtAutoMocUicT::UicSettingsT::UicSettingsT()
{
  RegExpInclude.compile("(^|\n)[ \t]*#[ \t]*include[ \t]+"
                        "[\"<](([^ \">]+/)?ui_[^ \">/]+\\.h)[\">]");
}

cmQtAutoMocUicT::UicSettingsT::~UicSettingsT() = default;

bool cmQtAutoMocUicT::UicSettingsT::skipped(std::string const& fileName) const
{
  return (!Enabled || (SkipList.find(fileName) != SkipList.end()));
}

void cmQtAutoMocUicT::JobT::LogError(GenT genType,
                                     cm::string_view message) const
{
  Gen()->AbortError();
  Gen()->Log().Error(genType, message);
}

void cmQtAutoMocUicT::JobT::LogCommandError(
  GenT genType, cm::string_view message,
  std::vector<std::string> const& command, std::string const& output) const
{
  Gen()->AbortError();
  Gen()->Log().ErrorCommand(genType, message, command, output);
}

bool cmQtAutoMocUicT::JobT::RunProcess(GenT genType,
                                       cmWorkerPool::ProcessResultT& result,
                                       std::vector<std::string> const& command,
                                       std::string* infoMessage)
{
  // Log command
  if (Log().Verbose()) {
    cm::string_view info;
    if (infoMessage != nullptr) {
      info = *infoMessage;
    }
    Log().Info(genType,
               cmStrCat(info,
                        info.empty() || cmHasSuffix(info, '\n') ? "" : "\n",
                        QuotedCommand(command), '\n'));
  }
  // Run command
  return cmWorkerPool::JobT::RunProcess(result, command,
                                        BaseConst().AutogenBuildDir);
}

void cmQtAutoMocUicT::JobMocPredefsT::Process()
{
  // (Re)generate moc_predefs.h on demand
  std::unique_ptr<std::string> reason;
  if (Log().Verbose()) {
    reason = cm::make_unique<std::string>();
  }
  if (!Update(reason.get())) {
    return;
  }
  std::string const& predefsFileAbs = MocConst().PredefsFileAbs;
  {
    cmWorkerPool::ProcessResultT result;
    {
      // Compose command
      std::vector<std::string> cmd = MocConst().PredefsCmd;
      // Add definitions
      cm::append(cmd, MocConst().OptionsDefinitions);
      // Add includes
      cm::append(cmd, MocConst().OptionsIncludes);
      // Execute command
      if (!RunProcess(GenT::MOC, result, cmd, reason.get())) {
        LogCommandError(GenT::MOC,
                        cmStrCat("The content generation command for ",
                                 MessagePath(predefsFileAbs), " failed.\n",
                                 result.ErrorMessage),
                        cmd, result.StdOut);
        return;
      }
    }

    // (Re)write predefs file only on demand
    if (cmQtAutoGenerator::FileDiffers(predefsFileAbs, result.StdOut)) {
      if (!cmQtAutoGenerator::FileWrite(predefsFileAbs, result.StdOut)) {
        LogError(
          GenT::MOC,
          cmStrCat("Writing ", MessagePath(predefsFileAbs), " failed."));
        return;
      }
    } else {
      // Touch to update the time stamp
      if (Log().Verbose()) {
        Log().Info(GenT::MOC, "Touching " + MessagePath(predefsFileAbs));
      }
      if (!cmSystemTools::Touch(predefsFileAbs, false)) {
        LogError(
          GenT::MOC,
          cmStrCat("Touching ", MessagePath(predefsFileAbs), " failed."));
        return;
      }
    }
  }

  // Read file time afterwards
  if (!MocEval().PredefsTime.Load(predefsFileAbs)) {
    LogError(GenT::MOC,
             cmStrCat("Reading the file time of ", MessagePath(predefsFileAbs),
                      " failed."));
    return;
  }
}

bool cmQtAutoMocUicT::JobMocPredefsT::Update(std::string* reason) const
{
  // Test if the file exists
  if (!MocEval().PredefsTime.Load(MocConst().PredefsFileAbs)) {
    if (reason != nullptr) {
      *reason = cmStrCat("Generating ", MessagePath(MocConst().PredefsFileAbs),
                         ", because it doesn't exist.");
    }
    return true;
  }

  // Test if the settings changed
  if (MocConst().SettingsChanged) {
    if (reason != nullptr) {
      *reason = cmStrCat("Generating ", MessagePath(MocConst().PredefsFileAbs),
                         ", because the moc settings changed.");
    }
    return true;
  }

  // Test if the executable is newer
  {
    std::string const& exec = MocConst().PredefsCmd.at(0);
    cmFileTime execTime;
    if (execTime.Load(exec)) {
      if (MocEval().PredefsTime.Older(execTime)) {
        if (reason != nullptr) {
          *reason =
            cmStrCat("Generating ", MessagePath(MocConst().PredefsFileAbs),
                     " because it is older than ", MessagePath(exec), '.');
        }
        return true;
      }
    }
  }

  return false;
}

bool cmQtAutoMocUicT::JobParseT::ReadFile()
{
  // Clear old parse information
  FileHandle->ParseData->Clear();
  std::string const& fileName = FileHandle->FileName;
  // Write info
  if (Log().Verbose()) {
    Log().Info(GenT::GEN, cmStrCat("Parsing ", MessagePath(fileName)));
  }
  // Read file content
  {
    std::string error;
    if (!cmQtAutoGenerator::FileRead(Content, fileName, &error)) {
      LogError(
        GenT::GEN,
        cmStrCat("Could not read ", MessagePath(fileName), ".\n", error));
      return false;
    }
  }
  // Warn if empty
  if (Content.empty()) {
    Log().Warning(GenT::GEN, cmStrCat(MessagePath(fileName), " is empty."));
    return false;
  }
  return true;
}

void cmQtAutoMocUicT::JobParseT::CreateKeys(
  std::vector<IncludeKeyT>& container, std::set<std::string> const& source,
  std::size_t basePrefixLength)
{
  if (source.empty()) {
    return;
  }
  container.reserve(source.size());
  for (std::string const& src : source) {
    container.emplace_back(src, basePrefixLength);
  }
}

void cmQtAutoMocUicT::JobParseT::MocMacro()
{
  for (KeyExpT const& filter : MocConst().MacroFilters) {
    // Run a simple find string check
    if (Content.find(filter.Key) == std::string::npos) {
      continue;
    }
    // Run the expensive regular expression check loop
    cmsys::RegularExpressionMatch match;
    if (filter.Exp.find(Content.c_str(), match)) {
      // Keep detected macro name
      FileHandle->ParseData->Moc.Macro = filter.Key;
      return;
    }
  }
}

void cmQtAutoMocUicT::JobParseT::MocDependecies()
{
  if (MocConst().DependFilters.empty() || MocConst().CanOutputDependencies) {
    return;
  }

  // Find dependency strings
  std::set<std::string> parseDepends;
  for (KeyExpT const& filter : MocConst().DependFilters) {
    // Run a simple find string check
    if (Content.find(filter.Key) == std::string::npos) {
      continue;
    }
    // Run the expensive regular expression check loop
    const char* contentChars = Content.c_str();
    cmsys::RegularExpressionMatch match;
    while (filter.Exp.find(contentChars, match)) {
      {
        std::string dep = match.match(1);
        if (!dep.empty()) {
          parseDepends.emplace(std::move(dep));
        }
      }
      contentChars += match.end();
    }
  }

  // Store dependency strings
  {
    auto& Depends = FileHandle->ParseData->Moc.Depends;
    Depends.reserve(parseDepends.size());
    for (std::string const& item : parseDepends) {
      Depends.emplace_back(item);
      // Replace end of line characters in filenames
      std::string& path = Depends.back();
      std::replace(path.begin(), path.end(), '\n', ' ');
      std::replace(path.begin(), path.end(), '\r', ' ');
    }
  }
}

void cmQtAutoMocUicT::JobParseT::MocIncludes()
{
  if (Content.find("moc") == std::string::npos) {
    return;
  }

  std::set<std::string> underscore;
  std::set<std::string> dot;
  {
    const char* contentChars = Content.c_str();
    cmsys::RegularExpression const& regExp = MocConst().RegExpInclude;
    cmsys::RegularExpressionMatch match;
    while (regExp.find(contentChars, match)) {
      std::string incString = match.match(2);
      std::string const incBase =
        cmSystemTools::GetFilenameWithoutLastExtension(incString);
      if (cmHasLiteralPrefix(incBase, "moc_")) {
        // moc_<BASE>.cpp
        // Remove the moc_ part from the base name
        underscore.emplace(std::move(incString));
      } else {
        // <BASE>.moc
        dot.emplace(std::move(incString));
      }
      // Forward content pointer
      contentChars += match.end();
    }
  }
  auto& Include = FileHandle->ParseData->Moc.Include;
  CreateKeys(Include.Underscore, underscore, MocUnderscoreLength);
  CreateKeys(Include.Dot, dot, 0);
}

void cmQtAutoMocUicT::JobParseT::UicIncludes()
{
  if (Content.find("ui_") == std::string::npos) {
    return;
  }

  std::set<std::string> includes;
  {
    const char* contentChars = Content.c_str();
    cmsys::RegularExpression const& regExp = UicConst().RegExpInclude;
    cmsys::RegularExpressionMatch match;
    while (regExp.find(contentChars, match)) {
      includes.emplace(match.match(2));
      // Forward content pointer
      contentChars += match.end();
    }
  }
  CreateKeys(FileHandle->ParseData->Uic.Include, includes, UiUnderscoreLength);
}

void cmQtAutoMocUicT::JobParseHeaderT::Process()
{
  if (!ReadFile()) {
    return;
  }
  // Moc parsing
  if (FileHandle->Moc) {
    MocMacro();
    MocDependecies();
  }
  // Uic parsing
  if (FileHandle->Uic) {
    UicIncludes();
  }
}

void cmQtAutoMocUicT::JobParseSourceT::Process()
{
  if (!ReadFile()) {
    return;
  }
  // Moc parsing
  if (FileHandle->Moc) {
    MocMacro();
    MocDependecies();
    MocIncludes();
  }
  // Uic parsing
  if (FileHandle->Uic) {
    UicIncludes();
  }
}

std::string cmQtAutoMocUicT::JobEvalCacheT::MessageSearchLocations() const
{
  std::string res;
  res.reserve(512);
  for (std::string const& path : SearchLocations) {
    res += "  ";
    res += MessagePath(path);
    res += '\n';
  }
  return res;
}

void cmQtAutoMocUicT::JobEvalCacheMocT::Process()
{
  // Evaluate headers
  for (auto const& pair : BaseEval().Headers) {
    if (!EvalHeader(pair.second)) {
      return;
    }
  }
  // Evaluate sources
  for (auto const& pair : BaseEval().Sources) {
    if (!EvalSource(pair.second)) {
      return;
    }
  }
}

bool cmQtAutoMocUicT::JobEvalCacheMocT::EvalHeader(SourceFileHandleT source)
{
  SourceFileT const& sourceFile = *source;
  auto const& parseData = sourceFile.ParseData->Moc;
  if (!source->Moc) {
    return true;
  }

  if (!parseData.Macro.empty()) {
    // Create a new mapping
    MappingHandleT handle = std::make_shared<MappingT>();
    handle->SourceFile = std::move(source);

    // Absolute build path
    if (BaseConst().MultiConfig) {
      handle->OutputFile = Gen()->AbsoluteIncludePath(sourceFile.BuildPath);
    } else {
      handle->OutputFile = Gen()->AbsoluteBuildPath(sourceFile.BuildPath);
    }

    // Register mapping in headers map
    RegisterMapping(handle);
  }

  return true;
}

bool cmQtAutoMocUicT::JobEvalCacheMocT::EvalSource(
  SourceFileHandleT const& source)
{
  SourceFileT const& sourceFile = *source;
  auto const& parseData = sourceFile.ParseData->Moc;
  if (!sourceFile.Moc ||
      (parseData.Macro.empty() && parseData.Include.Underscore.empty() &&
       parseData.Include.Dot.empty())) {
    return true;
  }

  std::string const sourceDirPrefix = SubDirPrefix(sourceFile.FileName);
  std::string const sourceBase =
    cmSystemTools::GetFilenameWithoutLastExtension(sourceFile.FileName);

  // For relaxed mode check if the own "moc_" or ".moc" file is included
  bool const relaxedMode = MocConst().RelaxedMode;
  bool sourceIncludesMocUnderscore = false;
  bool sourceIncludesDotMoc = false;
  // Check if the sources own "moc_" or ".moc" file is included
  if (relaxedMode) {
    for (IncludeKeyT const& incKey : parseData.Include.Underscore) {
      if (incKey.Base == sourceBase) {
        sourceIncludesMocUnderscore = true;
        break;
      }
    }
  }
  for (IncludeKeyT const& incKey : parseData.Include.Dot) {
    if (incKey.Base == sourceBase) {
      sourceIncludesDotMoc = true;
      break;
    }
  }

  // Check if this source needs to be moc processed but doesn't.
  if (!sourceIncludesDotMoc && !parseData.Macro.empty() &&
      !(relaxedMode && sourceIncludesMocUnderscore)) {
    LogError(GenT::MOC,
             cmStrCat(MessagePath(sourceFile.FileName), "\ncontains a ",
                      Quoted(parseData.Macro), " macro, but does not include ",
                      MessagePath(sourceBase + ".moc"),
                      "!\nConsider to\n  - add #include \"", sourceBase,
                      ".moc\"\n  - enable SKIP_AUTOMOC for this file"));
    return false;
  }

  // Evaluate "moc_" includes
  for (IncludeKeyT const& incKey : parseData.Include.Underscore) {
    SourceFileHandleT headerHandle;
    {
      std::string const headerBase = cmStrCat(incKey.Dir, incKey.Base);
      if (!FindIncludedHeader(headerHandle, sourceDirPrefix, headerBase)) {
        LogError(GenT::MOC,
                 cmStrCat(MessagePath(sourceFile.FileName),
                          "\nincludes the moc file ", MessagePath(incKey.Key),
                          ",\nbut a header ", MessageHeader(headerBase),
                          "\ncould not be found "
                          "in the following directories\n",
                          MessageSearchLocations()));
        return false;
      }
    }
    // The include might be handled differently in relaxed mode
    if (relaxedMode && !sourceIncludesDotMoc && !parseData.Macro.empty() &&
        (incKey.Base == sourceBase)) {
      // The <BASE>.cpp file includes a Qt macro but does not include the
      // <BASE>.moc file. In this case, the moc_<BASE>.cpp should probably
      // be generated from <BASE>.cpp instead of <BASE>.h, because otherwise
      // it won't build. But warn, since this is not how it is supposed to be
      // used. This is for KDE4 compatibility.

      // Issue a warning
      Log().Warning(
        GenT::MOC,
        cmStrCat(MessagePath(sourceFile.FileName), "\ncontains a ",
                 Quoted(parseData.Macro), " macro, but does not include ",
                 MessagePath(sourceBase + ".moc"), ".\nInstead it includes ",
                 MessagePath(incKey.Key), ".\nRunning moc on the source\n  ",
                 MessagePath(sourceFile.FileName), "!\nBetter include ",
                 MessagePath(sourceBase + ".moc"),
                 " for compatibility with regular mode.\n",
                 "This is a CMAKE_AUTOMOC_RELAXED_MODE warning.\n"));

      // Create mapping
      if (!RegisterIncluded(incKey.Key, source, source)) {
        return false;
      }
      continue;
    }

    // Check if header is skipped
    if (MocConst().skipped(headerHandle->FileName)) {
      continue;
    }
    // Create mapping
    if (!RegisterIncluded(incKey.Key, source, std::move(headerHandle))) {
      return false;
    }
  }

  // Evaluate ".moc" includes
  if (relaxedMode) {
    // Relaxed mode
    for (IncludeKeyT const& incKey : parseData.Include.Dot) {
      // Check if this is the sources own .moc file
      bool const ownMoc = (incKey.Base == sourceBase);
      if (ownMoc && !parseData.Macro.empty()) {
        // Create mapping for the regular use case
        if (!RegisterIncluded(incKey.Key, source, source)) {
          return false;
        }
        continue;
      }
      // Try to find a header instead but issue a warning.
      // This is for KDE4 compatibility.
      SourceFileHandleT headerHandle;
      {
        std::string const headerBase = cmStrCat(incKey.Dir, incKey.Base);
        if (!FindIncludedHeader(headerHandle, sourceDirPrefix, headerBase)) {
          LogError(
            GenT::MOC,
            cmStrCat(
              MessagePath(sourceFile.FileName), "\nincludes the moc file ",
              MessagePath(incKey.Key),
              ",\nwhich seems to be the moc file from a different source "
              "file.\nCMAKE_AUTOMOC_RELAXED_MODE:\nAlso a matching header ",
              MessageHeader(headerBase),
              "\ncould not be found in the following directories\n",
              MessageSearchLocations()));
          return false;
        }
      }
      // Check if header is skipped
      if (MocConst().skipped(headerHandle->FileName)) {
        continue;
      }
      // Issue a warning
      if (ownMoc && parseData.Macro.empty()) {
        Log().Warning(
          GenT::MOC,
          cmStrCat(MessagePath(sourceFile.FileName),
                   "\nincludes the moc file ", MessagePath(incKey.Key),
                   ", but does not contain a\n", MocConst().MacrosString(),
                   " macro.\nRunning moc on the header\n  ",
                   MessagePath(headerHandle->FileName), "!\nBetter include ",
                   MessagePath("moc_" + incKey.Base + ".cpp"),
                   " for a compatibility with regular mode.\n",
                   "This is a CMAKE_AUTOMOC_RELAXED_MODE warning.\n"));
      } else {
        Log().Warning(
          GenT::MOC,
          cmStrCat(MessagePath(sourceFile.FileName),
                   "\nincludes the moc file ", MessagePath(incKey.Key),
                   " instead of ", MessagePath("moc_" + incKey.Base + ".cpp"),
                   ".\nRunning moc on the header\n  ",
                   MessagePath(headerHandle->FileName), "!\nBetter include ",
                   MessagePath("moc_" + incKey.Base + ".cpp"),
                   " for compatibility with regular mode.\n",
                   "This is a CMAKE_AUTOMOC_RELAXED_MODE warning.\n"));
      }
      // Create mapping
      if (!RegisterIncluded(incKey.Key, source, std::move(headerHandle))) {
        return false;
      }
    }
  } else {
    // Strict mode
    for (IncludeKeyT const& incKey : parseData.Include.Dot) {
      // Check if this is the sources own .moc file
      bool const ownMoc = (incKey.Base == sourceBase);
      if (!ownMoc) {
        // Don't allow <BASE>.moc include other than own in regular mode
        LogError(GenT::MOC,
                 cmStrCat(MessagePath(sourceFile.FileName),
                          "\nincludes the moc file ", MessagePath(incKey.Key),
                          ",\nwhich seems to be the moc file from a different "
                          "source file.\nThis is not supported.  Include ",
                          MessagePath(sourceBase + ".moc"),
                          " to run moc on this source file."));
        return false;
      }
      // Accept but issue a warning if moc isn't required
      if (parseData.Macro.empty()) {
        Log().Warning(GenT::MOC,
                      cmStrCat(MessagePath(sourceFile.FileName),
                               "\nincludes the moc file ",
                               MessagePath(incKey.Key),
                               ", but does not contain a ",
                               MocConst().MacrosString(), " macro."));
      }
      // Create mapping
      if (!RegisterIncluded(incKey.Key, source, source)) {
        return false;
      }
    }
  }

  return true;
}

bool cmQtAutoMocUicT::JobEvalCacheMocT::FindIncludedHeader(
  SourceFileHandleT& headerHandle, cm::string_view includerDir,
  cm::string_view includeBase)
{
  // Clear search locations
  SearchLocations.clear();

  auto findHeader = [this,
                     &headerHandle](std::string const& basePath) -> bool {
    bool found = false;
    for (std::string const& ext : this->BaseConst().HeaderExtensions) {
      std::string const testPath =
        this->Gen()->CollapseFullPathTS(cmStrCat(basePath, '.', ext));
      cmFileTime fileTime;
      if (!fileTime.Load(testPath)) {
        // File not found
        continue;
      }

      // Return a known file if it exists already
      {
        auto it = BaseEval().Headers.find(testPath);
        if (it != BaseEval().Headers.end()) {
          headerHandle = it->second;
          found = true;
          break;
        }
      }

      // Created and return discovered file entry
      {
        SourceFileHandleT& handle = MocEval().HeadersDiscovered[testPath];
        if (!handle) {
          handle = std::make_shared<SourceFileT>(testPath);
          handle->FileTime = fileTime;
          handle->IsHeader = true;
          handle->Moc = true;
        }
        headerHandle = handle;
        found = true;
        break;
      }
    }
    if (!found) {
      this->SearchLocations.emplace_back(cmQtAutoGen::ParentDir(basePath));
    }
    return found;
  };

  // Search in vicinity of the source
  if (findHeader(cmStrCat(includerDir, includeBase))) {
    return true;
  }
  // Search in include directories
  for (std::string const& path : MocConst().IncludePaths) {
    if (findHeader(cmStrCat(path, '/', includeBase))) {
      return true;
    }
  }
  // Return without success
  return false;
}

bool cmQtAutoMocUicT::JobEvalCacheMocT::RegisterIncluded(
  std::string const& includeString, SourceFileHandleT includerFileHandle,
  SourceFileHandleT sourceFileHandle) const
{
  // Check if this file is already included
  MappingHandleT& handle = MocEval().Includes[includeString];
  if (handle) {
    // Check if the output file would be generated from different source files
    if (handle->SourceFile != sourceFileHandle) {
      std::string files =
        cmStrCat("  ", MessagePath(includerFileHandle->FileName), '\n');
      for (auto const& item : handle->IncluderFiles) {
        files += cmStrCat("  ", MessagePath(item->FileName), '\n');
      }
      LogError(
        GenT::MOC,
        cmStrCat("The source files\n", files,
                 "contain the same include string ",
                 MessagePath(includeString),
                 ", but\nthe moc file would be generated from different "
                 "source files\n  ",
                 MessagePath(sourceFileHandle->FileName), " and\n  ",
                 MessagePath(handle->SourceFile->FileName),
                 ".\nConsider to\n"
                 "  - not include the \"moc_<NAME>.cpp\" file\n"
                 "  - add a directory prefix to a \"<NAME>.moc\" include "
                 "(e.g \"sub/<NAME>.moc\")\n"
                 "  - rename the source file(s)\n"));
      return false;
    }

    // The same mapping already exists. Just add to the includers list.
    handle->IncluderFiles.emplace_back(std::move(includerFileHandle));
    return true;
  }

  // Create a new mapping
  handle = std::make_shared<MappingT>();
  handle->IncludeString = includeString;
  handle->IncluderFiles.emplace_back(std::move(includerFileHandle));
  handle->SourceFile = std::move(sourceFileHandle);
  handle->OutputFile = Gen()->AbsoluteIncludePath(includeString);

  // Register mapping in sources/headers map
  RegisterMapping(handle);
  return true;
}

void cmQtAutoMocUicT::JobEvalCacheMocT::RegisterMapping(
  MappingHandleT mappingHandle) const
{
  auto& regMap = mappingHandle->SourceFile->IsHeader
    ? MocEval().HeaderMappings
    : MocEval().SourceMappings;
  // Check if source file already gets mapped
  auto& regHandle = regMap[mappingHandle->SourceFile->FileName];
  if (!regHandle) {
    // Yet unknown mapping
    regHandle = std::move(mappingHandle);
  } else {
    // Mappings with include string override those without
    if (!mappingHandle->IncludeString.empty()) {
      regHandle = std::move(mappingHandle);
    }
  }
}

std::string cmQtAutoMocUicT::JobEvalCacheMocT::MessageHeader(
  cm::string_view headerBase) const
{
  return MessagePath(cmStrCat(
    headerBase, ".{", cmJoin(this->BaseConst().HeaderExtensions, ","), '}'));
}

void cmQtAutoMocUicT::JobEvalCacheUicT::Process()
{
  // Prepare buffers
  SearchLocations.reserve((UicConst().SearchPaths.size() + 1) * 2);

  // Evaluate headers
  for (auto const& pair : BaseEval().Headers) {
    if (!EvalFile(pair.second)) {
      return;
    }
  }
  // Evaluate sources
  for (auto const& pair : BaseEval().Sources) {
    if (!EvalFile(pair.second)) {
      return;
    }
  }
}

bool cmQtAutoMocUicT::JobEvalCacheUicT::EvalFile(
  SourceFileHandleT const& sourceFileHandle)
{
  SourceFileT const& sourceFile = *sourceFileHandle;
  auto const& Include = sourceFile.ParseData->Uic.Include;
  if (!sourceFile.Uic || Include.empty()) {
    return true;
  }

  std::string const sourceDirPrefix = SubDirPrefix(sourceFile.FileName);
  for (IncludeKeyT const& incKey : Include) {
    // Find .ui file
    UiName = cmStrCat(incKey.Base, ".ui");
    if (!FindIncludedUi(sourceDirPrefix, incKey.Dir)) {
      LogError(GenT::UIC,
               cmStrCat(MessagePath(sourceFile.FileName),
                        "\nincludes the uic file ", MessagePath(incKey.Key),
                        ",\nbut the user interface file ", MessagePath(UiName),
                        "\ncould not be found in the following directories\n",
                        MessageSearchLocations()));
      return false;
    }
    // Check if the file is skipped
    if (UicConst().skipped(UiFileHandle->FileName)) {
      continue;
    }
    // Register mapping
    if (!RegisterMapping(incKey.Key, sourceFileHandle)) {
      return false;
    }
  }

  return true;
}

bool cmQtAutoMocUicT::JobEvalCacheUicT::FindIncludedUi(
  cm::string_view sourceDirPrefix, cm::string_view includePrefix)
{
  // Clear locations buffer
  SearchLocations.clear();

  auto findUi = [this](std::string const& testPath) -> bool {
    std::string const fullPath = this->Gen()->CollapseFullPathTS(testPath);
    cmFileTime fileTime;
    if (!fileTime.Load(fullPath)) {
      this->SearchLocations.emplace_back(cmQtAutoGen::ParentDir(fullPath));
      return false;
    }
    // .ui file found in files system!
    // Get or create .ui file handle
    SourceFileHandleT& handle = this->UicEval().UiFiles[fullPath];
    if (!handle) {
      // The file wasn't registered, yet
      handle = std::make_shared<SourceFileT>(fullPath);
      handle->FileTime = fileTime;
    }
    this->UiFileHandle = handle;
    return true;
  };

  // Vicinity of the source
  if (findUi(cmStrCat(sourceDirPrefix, UiName))) {
    return true;
  }
  if (!includePrefix.empty()) {
    if (findUi(cmStrCat(sourceDirPrefix, includePrefix, UiName))) {
      return true;
    }
  }
  // Additional AUTOUIC search paths
  auto const& searchPaths = UicConst().SearchPaths;
  if (!searchPaths.empty()) {
    for (std::string const& sPath : searchPaths) {
      if (findUi(cmStrCat(sPath, '/', UiName))) {
        return true;
      }
    }
    if (!includePrefix.empty()) {
      for (std::string const& sPath : searchPaths) {
        if (findUi(cmStrCat(sPath, '/', includePrefix, UiName))) {
          return true;
        }
      }
    }
  }

  return false;
}

bool cmQtAutoMocUicT::JobEvalCacheUicT::RegisterMapping(
  std::string const& includeString, SourceFileHandleT includerFileHandle)
{
  auto& Includes = Gen()->UicEval().Includes;
  auto it = Includes.find(includeString);
  if (it != Includes.end()) {
    MappingHandleT const& handle = it->second;
    if (handle->SourceFile != UiFileHandle) {
      // The output file already gets generated - from a different .ui file!
      std::string files =
        cmStrCat("  ", MessagePath(includerFileHandle->FileName), '\n');
      for (auto const& item : handle->IncluderFiles) {
        files += cmStrCat("  ", MessagePath(item->FileName), '\n');
      }
      LogError(
        GenT::UIC,
        cmStrCat(
          "The source files\n", files, "contain the same include string ",
          Quoted(includeString),
          ", but\nthe uic file would be generated from different "
          "user interface files\n  ",
          MessagePath(UiFileHandle->FileName), " and\n  ",
          MessagePath(handle->SourceFile->FileName),
          ".\nConsider to\n"
          "  - add a directory prefix to a \"ui_<NAME>.h\" include "
          "(e.g \"sub/ui_<NAME>.h\")\n"
          "  - rename the <NAME>.ui file(s) and adjust the \"ui_<NAME>.h\" "
          "include(s)\n"));
      return false;
    }
    // Add includer file to existing mapping
    handle->IncluderFiles.emplace_back(std::move(includerFileHandle));
  } else {
    // New mapping handle
    MappingHandleT handle = std::make_shared<MappingT>();
    handle->IncludeString = includeString;
    handle->IncluderFiles.emplace_back(std::move(includerFileHandle));
    handle->SourceFile = UiFileHandle;
    handle->OutputFile = Gen()->AbsoluteIncludePath(includeString);
    // Register mapping
    Includes.emplace(includeString, std::move(handle));
  }
  return true;
}

void cmQtAutoMocUicT::JobEvalCacheFinishT::Process()
{
  // Add discovered header parse jobs
  Gen()->CreateParseJobs<JobParseHeaderT>(MocEval().HeadersDiscovered);

  // Add dependency probing jobs
  {
    // Add fence job to ensure all parsing has finished
    Gen()->WorkerPool().EmplaceJob<JobFenceT>();
    if (MocConst().Enabled) {
      Gen()->WorkerPool().EmplaceJob<JobProbeDepsMocT>();
    }
    if (UicConst().Enabled) {
      Gen()->WorkerPool().EmplaceJob<JobProbeDepsUicT>();
    }
    // Add probe finish job
    Gen()->WorkerPool().EmplaceJob<JobProbeDepsFinishT>();
  }
}

void cmQtAutoMocUicT::JobProbeDepsMocT::Process()
{
  // Create moc header jobs
  for (auto const& pair : MocEval().HeaderMappings) {
    // Register if this mapping is a candidate for mocs_compilation.cpp
    bool const compFile = pair.second->IncludeString.empty();
    if (compFile) {
      MocEval().CompFiles.emplace_back(pair.second->SourceFile->BuildPath);
    }
    if (!Generate(pair.second, compFile)) {
      return;
    }
  }

  // Create moc source jobs
  for (auto const& pair : MocEval().SourceMappings) {
    if (!Generate(pair.second, false)) {
      return;
    }
  }
}

bool cmQtAutoMocUicT::JobProbeDepsMocT::Generate(MappingHandleT const& mapping,
                                                 bool compFile) const
{
  std::unique_ptr<std::string> reason;
  if (Log().Verbose()) {
    reason = cm::make_unique<std::string>();
  }
  if (Probe(*mapping, reason.get())) {
    // Register the parent directory for creation
    MocEval().OutputDirs.emplace(cmQtAutoGen::ParentDir(mapping->OutputFile));
    // Fetch the cache entry for the source file
    std::string const& sourceFile = mapping->SourceFile->FileName;
    ParseCacheT::GetOrInsertT cacheEntry =
      BaseEval().ParseCache.GetOrInsert(sourceFile);
    // Add moc job
    Gen()->WorkerPool().EmplaceJob<JobCompileMocT>(
      mapping, std::move(reason), std::move(cacheEntry.first));
    // Check if a moc job for a mocs_compilation.cpp entry was generated
    if (compFile) {
      MocEval().CompUpdated = true;
    }
  }
  return true;
}

bool cmQtAutoMocUicT::JobProbeDepsMocT::Probe(MappingT const& mapping,
                                              std::string* reason) const
{
  std::string const& sourceFile = mapping.SourceFile->FileName;
  std::string const& outputFile = mapping.OutputFile;

  // Test if the output file exists
  cmFileTime outputFileTime;
  if (!outputFileTime.Load(outputFile)) {
    if (reason != nullptr) {
      *reason =
        cmStrCat("Generating ", MessagePath(outputFile),
                 ", because it doesn't exist, from ", MessagePath(sourceFile));
    }
    return true;
  }

  // Test if any setting changed
  if (MocConst().SettingsChanged) {
    if (reason != nullptr) {
      *reason = cmStrCat("Generating ", MessagePath(outputFile),
                         ", because the uic settings changed, from ",
                         MessagePath(sourceFile));
    }
    return true;
  }

  // Test if the source file is newer
  if (outputFileTime.Older(mapping.SourceFile->FileTime)) {
    if (reason != nullptr) {
      *reason = cmStrCat("Generating ", MessagePath(outputFile),
                         ", because it's older than its source file, from ",
                         MessagePath(sourceFile));
    }
    return true;
  }

  // Test if the moc_predefs file is newer
  if (!MocConst().PredefsFileAbs.empty()) {
    if (outputFileTime.Older(MocEval().PredefsTime)) {
      if (reason != nullptr) {
        *reason = cmStrCat("Generating ", MessagePath(outputFile),
                           ", because it's older than ",
                           MessagePath(MocConst().PredefsFileAbs), ", from ",
                           MessagePath(sourceFile));
      }
      return true;
    }
  }

  // Test if the moc executable is newer
  if (outputFileTime.Older(MocConst().ExecutableTime)) {
    if (reason != nullptr) {
      *reason = cmStrCat("Generating ", MessagePath(outputFile),
                         ", because it's older than the moc executable, from ",
                         MessagePath(sourceFile));
    }
    return true;
  }

  // Test if a dependency file is newer
  {
    // Check dependency timestamps
    std::string const sourceDir = SubDirPrefix(sourceFile);
    for (std::string const& dep : mapping.SourceFile->ParseData->Moc.Depends) {
      // Find dependency file
      auto const depMatch = FindDependency(sourceDir, dep);
      if (depMatch.first.empty()) {
        Log().Warning(GenT::MOC,
                      cmStrCat(MessagePath(sourceFile), " depends on ",
                               MessagePath(dep),
                               " but the file does not exist."));
        continue;
      }
      // Test if dependency file is older
      if (outputFileTime.Older(depMatch.second)) {
        if (reason != nullptr) {
          *reason = cmStrCat("Generating ", MessagePath(outputFile),
                             ", because it's older than its dependency file ",
                             MessagePath(depMatch.first), ", from ",
                             MessagePath(sourceFile));
        }
        return true;
      }
    }
  }

  return false;
}

std::pair<std::string, cmFileTime>
cmQtAutoMocUicT::JobProbeDepsMocT::FindDependency(
  std::string const& sourceDir, std::string const& includeString) const
{
  using ResPair = std::pair<std::string, cmFileTime>;
  // moc's dependency file contains absolute paths
  if (MocConst().CanOutputDependencies) {
    ResPair res{ includeString, {} };
    if (res.second.Load(res.first)) {
      return res;
    }
    return {};
  }
  // Search in vicinity of the source
  {
    ResPair res{ sourceDir + includeString, {} };
    if (res.second.Load(res.first)) {
      return res;
    }
  }
  // Search in include directories
  for (std::string const& includePath : MocConst().IncludePaths) {
    ResPair res{ cmStrCat(includePath, '/', includeString), {} };
    if (res.second.Load(res.first)) {
      return res;
    }
  }
  // Return empty
  return ResPair();
}

void cmQtAutoMocUicT::JobProbeDepsUicT::Process()
{
  for (auto const& pair : Gen()->UicEval().Includes) {
    MappingHandleT const& mapping = pair.second;
    std::unique_ptr<std::string> reason;
    if (Log().Verbose()) {
      reason = cm::make_unique<std::string>();
    }
    if (!Probe(*mapping, reason.get())) {
      continue;
    }

    // Register the parent directory for creation
    UicEval().OutputDirs.emplace(cmQtAutoGen::ParentDir(mapping->OutputFile));
    // Add uic job
    Gen()->WorkerPool().EmplaceJob<JobCompileUicT>(mapping, std::move(reason));
  }
}

bool cmQtAutoMocUicT::JobProbeDepsUicT::Probe(MappingT const& mapping,
                                              std::string* reason) const
{
  std::string const& sourceFile = mapping.SourceFile->FileName;
  std::string const& outputFile = mapping.OutputFile;

  // Test if the build file exists
  cmFileTime outputFileTime;
  if (!outputFileTime.Load(outputFile)) {
    if (reason != nullptr) {
      *reason =
        cmStrCat("Generating ", MessagePath(outputFile),
                 ", because it doesn't exist, from ", MessagePath(sourceFile));
    }
    return true;
  }

  // Test if the uic settings changed
  if (UicConst().SettingsChanged) {
    if (reason != nullptr) {
      *reason = cmStrCat("Generating ", MessagePath(outputFile),
                         ", because the uic settings changed, from ",
                         MessagePath(sourceFile));
    }
    return true;
  }

  // Test if the source file is newer
  if (outputFileTime.Older(mapping.SourceFile->FileTime)) {
    if (reason != nullptr) {
      *reason = cmStrCat("Generating ", MessagePath(outputFile),
                         " because it's older than the source file ",
                         MessagePath(sourceFile));
    }
    return true;
  }

  // Test if the uic executable is newer
  if (outputFileTime.Older(UicConst().ExecutableTime)) {
    if (reason != nullptr) {
      *reason = cmStrCat("Generating ", MessagePath(outputFile),
                         ", because it's older than the uic executable, from ",
                         MessagePath(sourceFile));
    }
    return true;
  }

  return false;
}

void cmQtAutoMocUicT::JobProbeDepsFinishT::Process()
{
  // Create output directories
  {
    using StringSet = std::unordered_set<std::string>;
    auto createDirs = [this](GenT genType, StringSet const& dirSet) {
      for (std::string const& dirName : dirSet) {
        if (!cmSystemTools::MakeDirectory(dirName)) {
          this->LogError(
            genType,
            cmStrCat("Creating directory ", MessagePath(dirName), " failed."));
          return;
        }
      }
    };
    if (MocConst().Enabled && UicConst().Enabled) {
      StringSet outputDirs = MocEval().OutputDirs;
      outputDirs.insert(UicEval().OutputDirs.begin(),
                        UicEval().OutputDirs.end());
      createDirs(GenT::GEN, outputDirs);
    } else if (MocConst().Enabled) {
      createDirs(GenT::MOC, MocEval().OutputDirs);
    } else if (UicConst().Enabled) {
      createDirs(GenT::UIC, UicEval().OutputDirs);
    }
  }

  if (MocConst().Enabled) {
    // Add mocs compilations job
    Gen()->WorkerPool().EmplaceJob<JobMocsCompilationT>();
  }

  if (!BaseConst().DepFile.empty()) {
    // Add job to merge dep files
    Gen()->WorkerPool().EmplaceJob<JobDepFilesMergeT>();
  }

  // Add finish job
  Gen()->WorkerPool().EmplaceJob<JobFinishT>();
}

void cmQtAutoMocUicT::JobCompileMocT::Process()
{
  std::string const& sourceFile = Mapping->SourceFile->FileName;
  std::string const& outputFile = Mapping->OutputFile;

  // Compose moc command
  std::vector<std::string> cmd;
  {
    // Reserve large enough
    cmd.reserve(MocConst().OptionsDefinitions.size() +
                MocConst().OptionsIncludes.size() +
                MocConst().OptionsExtra.size() + 16);
    cmd.push_back(MocConst().Executable);
    // Add definitions
    cm::append(cmd, MocConst().OptionsDefinitions);
    // Add includes
    cm::append(cmd, MocConst().OptionsIncludes);
    // Add predefs include
    if (!MocConst().PredefsFileAbs.empty()) {
      cmd.emplace_back("--include");
      cmd.push_back(MocConst().PredefsFileAbs);
    }
    // Add path prefix on demand
    if (MocConst().PathPrefix && Mapping->SourceFile->IsHeader) {
      for (std::string const& dir : MocConst().IncludePaths) {
        cm::string_view prefix = sourceFile;
        if (cmHasPrefix(prefix, dir)) {
          prefix.remove_prefix(dir.size());
          if (cmHasPrefix(prefix, '/')) {
            prefix.remove_prefix(1);
            auto slashPos = prefix.rfind('/');
            if (slashPos != cm::string_view::npos) {
              cmd.emplace_back("-p");
              cmd.emplace_back(prefix.substr(0, slashPos));
            } else {
              cmd.emplace_back("-p");
              cmd.emplace_back("./");
            }
            break;
          }
        }
      }
    }
    // Add extra options
    cm::append(cmd, MocConst().OptionsExtra);
    if (MocConst().CanOutputDependencies) {
      cmd.emplace_back("--output-dep-file");
    }
    // Add output file
    cmd.emplace_back("-o");
    cmd.push_back(outputFile);
    // Add source file
    cmd.push_back(sourceFile);
  }

  // Execute moc command
  cmWorkerPool::ProcessResultT result;
  if (!RunProcess(GenT::MOC, result, cmd, Reason.get())) {
    // Moc command failed
    std::string includers;
    if (!Mapping->IncluderFiles.empty()) {
      includers = "included by\n";
      for (auto const& item : Mapping->IncluderFiles) {
        includers += cmStrCat("  ", MessagePath(item->FileName), '\n');
      }
    }
    LogCommandError(GenT::MOC,
                    cmStrCat("The moc process failed to compile\n  ",
                             MessagePath(sourceFile), "\ninto\n  ",
                             MessagePath(outputFile), '\n', includers,
                             result.ErrorMessage),
                    cmd, result.StdOut);
    return;
  }

  // Moc command success. Print moc output.
  if (!result.StdOut.empty()) {
    Log().Info(GenT::MOC, result.StdOut);
  }

  // Extract dependencies from the dep file moc generated for us
  if (MocConst().CanOutputDependencies) {
    const std::string depfile = outputFile + ".d";
    if (Log().Verbose()) {
      Log().Info(GenT::MOC,
                 "Reading dependencies from " + MessagePath(depfile));
    }
    if (!cmSystemTools::FileExists(depfile)) {
      Log().Warning(GenT::MOC,
                    "Dependency file " + MessagePath(depfile) +
                      " does not exist.");
      return;
    }
    CacheEntry->Moc.Depends = dependenciesFromDepFile(depfile.c_str());
  }
}

void cmQtAutoMocUicT::JobCompileUicT::Process()
{
  std::string const& sourceFile = Mapping->SourceFile->FileName;
  std::string const& outputFile = Mapping->OutputFile;

  // Compose uic command
  std::vector<std::string> cmd;
  cmd.push_back(UicConst().Executable);
  {
    std::vector<std::string> allOpts = UicConst().Options;
    auto optionIt = UicConst().UiFiles.find(sourceFile);
    if (optionIt != UicConst().UiFiles.end()) {
      UicMergeOptions(allOpts, optionIt->second.Options,
                      (BaseConst().QtVersion.Major == 5));
    }
    cm::append(cmd, allOpts);
  }
  cmd.emplace_back("-o");
  cmd.emplace_back(outputFile);
  cmd.emplace_back(sourceFile);

  cmWorkerPool::ProcessResultT result;
  if (RunProcess(GenT::UIC, result, cmd, Reason.get())) {
    // Uic command success
    // Print uic output
    if (!result.StdOut.empty()) {
      Log().Info(GenT::UIC, result.StdOut);
    }
  } else {
    // Uic command failed
    std::string includers;
    for (auto const& item : Mapping->IncluderFiles) {
      includers += cmStrCat("  ", MessagePath(item->FileName), '\n');
    }
    LogCommandError(GenT::UIC,
                    cmStrCat("The uic process failed to compile\n  ",
                             MessagePath(sourceFile), "\ninto\n  ",
                             MessagePath(outputFile), "\nincluded by\n",
                             includers, result.ErrorMessage),
                    cmd, result.StdOut);
  }
}

void cmQtAutoMocUicT::JobMocsCompilationT::Process()
{
  // Compose mocs compilation file content
  std::string content =
    "// This file is autogenerated. Changes will be overwritten.\n";

  if (MocEval().CompFiles.empty()) {
    // Placeholder content
    content += "// No files found that require moc or the moc files are "
               "included\n"
               "enum some_compilers { need_more_than_nothing };\n";
  } else {
    // Valid content
    const bool mc = BaseConst().MultiConfig;
    cm::string_view const wrapFront = mc ? "#include <" : "#include \"";
    cm::string_view const wrapBack = mc ? ">\n" : "\"\n";
    content += cmWrap(wrapFront, MocEval().CompFiles, wrapBack, "");
  }

  std::string const& compAbs = MocConst().CompFileAbs;
  if (cmQtAutoGenerator::FileDiffers(compAbs, content)) {
    // Actually write mocs compilation file
    if (Log().Verbose()) {
      Log().Info(GenT::MOC,
                 "Generating MOC compilation " + MessagePath(compAbs));
    }
    if (!FileWrite(compAbs, content)) {
      LogError(GenT::MOC,
               cmStrCat("Writing MOC compilation ", MessagePath(compAbs),
                        " failed."));
    }
  } else if (MocEval().CompUpdated) {
    // Only touch mocs compilation file
    if (Log().Verbose()) {
      Log().Info(GenT::MOC,
                 "Touching MOC compilation " + MessagePath(compAbs));
    }
    if (!cmSystemTools::Touch(compAbs, false)) {
      LogError(GenT::MOC,
               cmStrCat("Touching MOC compilation ", MessagePath(compAbs),
                        " failed."));
    }
  }
}

/*
 * Escapes paths for Ninja depfiles.
 * This is a re-implementation of what moc does when writing depfiles.
 */
std::string escapeDependencyPath(cm::string_view path)
{
  std::string escapedPath;
  escapedPath.reserve(path.size());
  const size_t s = path.size();
  int backslashCount = 0;
  for (size_t i = 0; i < s; ++i) {
    if (path[i] == '\\') {
      ++backslashCount;
    } else {
      if (path[i] == '$') {
        escapedPath.push_back('$');
      } else if (path[i] == '#') {
        escapedPath.push_back('\\');
      } else if (path[i] == ' ') {
        // Double the amount of written backslashes,
        // and add one more to escape the space.
        while (backslashCount-- >= 0) {
          escapedPath.push_back('\\');
        }
      }
      backslashCount = 0;
    }
    escapedPath.push_back(path[i]);
  }
  return escapedPath;
}

void cmQtAutoMocUicT::JobDepFilesMergeT::Process()
{
  if (Log().Verbose()) {
    Log().Info(GenT::MOC,
               cmStrCat("Merging MOC dependencies into ",
                        MessagePath(BaseConst().DepFile.c_str())));
  }
  auto processDepFile =
    [](const std::string& mocOutputFile) -> std::vector<std::string> {
    std::string f = mocOutputFile + ".d";
    if (!cmSystemTools::FileExists(f)) {
      return {};
    }
    return dependenciesFromDepFile(f.c_str());
  };

  std::vector<std::string> dependencies = BaseConst().ListFiles;
  ParseCacheT& parseCache = BaseEval().ParseCache;
  auto processMappingEntry = [&](const MappingMapT::value_type& m) {
    auto cacheEntry = parseCache.GetOrInsert(m.first);
    if (cacheEntry.first->Moc.Depends.empty()) {
      cacheEntry.first->Moc.Depends = processDepFile(m.second->OutputFile);
    }
    dependencies.insert(dependencies.end(),
                        cacheEntry.first->Moc.Depends.begin(),
                        cacheEntry.first->Moc.Depends.end());
  };

  std::for_each(MocEval().HeaderMappings.begin(),
                MocEval().HeaderMappings.end(), processMappingEntry);
  std::for_each(MocEval().SourceMappings.begin(),
                MocEval().SourceMappings.end(), processMappingEntry);

  // Remove duplicates to make the depfile smaller
  std::sort(dependencies.begin(), dependencies.end());
  dependencies.erase(std::unique(dependencies.begin(), dependencies.end()),
                     dependencies.end());

  // Add form files
  for (const auto& uif : UicEval().UiFiles) {
    dependencies.push_back(uif.first);
  }

  // Write the file
  cmsys::ofstream ofs;
  ofs.open(BaseConst().DepFile.c_str(),
           (std::ios::out | std::ios::binary | std::ios::trunc));
  if (!ofs) {
    LogError(GenT::GEN,
             cmStrCat("Cannot open ", MessagePath(BaseConst().DepFile),
                      " for writing."));
    return;
  }
  ofs << BaseConst().DepFileRuleName << ": \\" << std::endl;
  for (const std::string& file : dependencies) {
    ofs << '\t' << escapeDependencyPath(file) << " \\" << std::endl;
    if (!ofs.good()) {
      LogError(GenT::GEN,
               cmStrCat("Writing depfile", MessagePath(BaseConst().DepFile),
                        " failed."));
      return;
    }
  }

  // Add the CMake executable to re-new cache data if necessary.
  // Also, this is the last entry, so don't add a backslash.
  ofs << '\t' << escapeDependencyPath(BaseConst().CMakeExecutable)
      << std::endl;
}

void cmQtAutoMocUicT::JobFinishT::Process()
{
  Gen()->AbortSuccess();
}

cmQtAutoMocUicT::cmQtAutoMocUicT()
  : cmQtAutoGenerator(GenT::GEN)
{
}
cmQtAutoMocUicT::~cmQtAutoMocUicT() = default;

bool cmQtAutoMocUicT::InitFromInfo(InfoT const& info)
{
  // -- Required settings
  if (!info.GetBool("MULTI_CONFIG", BaseConst_.MultiConfig, true) ||
      !info.GetUInt("QT_VERSION_MAJOR", BaseConst_.QtVersion.Major, true) ||
      !info.GetUInt("QT_VERSION_MINOR", BaseConst_.QtVersion.Minor, true) ||
      !info.GetUInt("PARALLEL", BaseConst_.ThreadCount, false) ||
      !info.GetString("BUILD_DIR", BaseConst_.AutogenBuildDir, true) ||
      !info.GetStringConfig("INCLUDE_DIR", BaseConst_.AutogenIncludeDir,
                            true) ||
      !info.GetString("CMAKE_EXECUTABLE", BaseConst_.CMakeExecutable, true) ||
      !info.GetStringConfig("PARSE_CACHE_FILE", BaseConst_.ParseCacheFile,
                            true) ||
      !info.GetString("DEP_FILE", BaseConst_.DepFile, false) ||
      !info.GetString("DEP_FILE_RULE_NAME", BaseConst_.DepFileRuleName,
                      false) ||
      !info.GetStringConfig("SETTINGS_FILE", SettingsFile_, true) ||
      !info.GetArray("CMAKE_LIST_FILES", BaseConst_.ListFiles, true) ||
      !info.GetArray("HEADER_EXTENSIONS", BaseConst_.HeaderExtensions, true) ||
      !info.GetString("QT_MOC_EXECUTABLE", MocConst_.Executable, false) ||
      !info.GetString("QT_UIC_EXECUTABLE", UicConst_.Executable, false)) {
    return false;
  }

  // -- Checks
  if (!BaseConst_.CMakeExecutableTime.Load(BaseConst_.CMakeExecutable)) {
    return info.LogError(cmStrCat("The CMake executable ",
                                  MessagePath(BaseConst_.CMakeExecutable),
                                  " does not exist."));
  }

  // -- Evaluate values
  BaseConst_.ThreadCount = std::min(BaseConst_.ThreadCount, ParallelMax);
  WorkerPool_.SetThreadCount(BaseConst_.ThreadCount);

  // -- Moc
  if (!MocConst_.Executable.empty()) {
    // -- Moc is enabled
    MocConst_.Enabled = true;

    // -- Temporary buffers
    struct
    {
      std::vector<std::string> MacroNames;
      std::vector<std::string> DependFilters;
    } tmp;

    // -- Required settings
    if (!info.GetBool("MOC_RELAXED_MODE", MocConst_.RelaxedMode, false) ||
        !info.GetBool("MOC_PATH_PREFIX", MocConst_.PathPrefix, true) ||
        !info.GetArray("MOC_SKIP", MocConst_.SkipList, false) ||
        !info.GetArrayConfig("MOC_DEFINITIONS", MocConst_.Definitions,
                             false) ||
        !info.GetArrayConfig("MOC_INCLUDES", MocConst_.IncludePaths, false) ||
        !info.GetArray("MOC_OPTIONS", MocConst_.OptionsExtra, false) ||
        !info.GetStringConfig("MOC_COMPILATION_FILE", MocConst_.CompFileAbs,
                              true) ||
        !info.GetArray("MOC_PREDEFS_CMD", MocConst_.PredefsCmd, false) ||
        !info.GetStringConfig("MOC_PREDEFS_FILE", MocConst_.PredefsFileAbs,
                              !MocConst_.PredefsCmd.empty()) ||
        !info.GetArray("MOC_MACRO_NAMES", tmp.MacroNames, true) ||
        !info.GetArray("MOC_DEPEND_FILTERS", tmp.DependFilters, false)) {
      return false;
    }

    // -- Evaluate settings
    for (std::string const& item : tmp.MacroNames) {
      MocConst_.MacroFilters.emplace_back(
        item, ("[\n][ \t]*{?[ \t]*" + item).append("[^a-zA-Z0-9_]"));
    }
    // Can moc output dependencies or do we need to setup dependency filters?
    if (BaseConst_.QtVersion >= IntegerVersion(5, 15)) {
      MocConst_.CanOutputDependencies = true;
    } else {
      Json::Value const& val = info.GetValue("MOC_DEPEND_FILTERS");
      if (!val.isArray()) {
        return info.LogError("MOC_DEPEND_FILTERS JSON value is not an array.");
      }
      Json::ArrayIndex const arraySize = val.size();
      for (Json::ArrayIndex ii = 0; ii != arraySize; ++ii) {
        // Test entry closure
        auto testEntry = [&info, ii](bool test, cm::string_view msg) -> bool {
          if (!test) {
            info.LogError(
              cmStrCat("MOC_DEPEND_FILTERS filter ", ii, ": ", msg));
          }
          return !test;
        };

        Json::Value const& pairVal = val[ii];

        if (testEntry(pairVal.isArray(), "JSON value is not an array.") ||
            testEntry(pairVal.size() == 2, "JSON array size invalid.")) {
          return false;
        }

        Json::Value const& keyVal = pairVal[0u];
        Json::Value const& expVal = pairVal[1u];
        if (testEntry(keyVal.isString(),
                      "JSON value for keyword is not a string.") ||
            testEntry(expVal.isString(),
                      "JSON value for regular expression is not a string.")) {
          return false;
        }

        std::string const key = keyVal.asString();
        std::string const exp = expVal.asString();
        if (testEntry(!key.empty(), "Keyword is empty.") ||
            testEntry(!exp.empty(), "Regular expression is empty.")) {
          return false;
        }

        this->MocConst_.DependFilters.emplace_back(key, exp);
        if (testEntry(
              this->MocConst_.DependFilters.back().Exp.is_valid(),
              cmStrCat("Regular expression compilation failed.\nKeyword: ",
                       Quoted(key), "\nExpression: ", Quoted(exp)))) {
          return false;
        }
      }
    }
    // Check if moc executable exists (by reading the file time)
    if (!MocConst_.ExecutableTime.Load(MocConst_.Executable)) {
      return info.LogError(cmStrCat("The moc executable ",
                                    MessagePath(MocConst_.Executable),
                                    " does not exist."));
    }
  }

  // -- Uic
  if (!UicConst_.Executable.empty()) {
    // Uic is enabled
    UicConst_.Enabled = true;

    // -- Required settings
    if (!info.GetArray("UIC_SKIP", UicConst_.SkipList, false) ||
        !info.GetArray("UIC_SEARCH_PATHS", UicConst_.SearchPaths, false) ||
        !info.GetArrayConfig("UIC_OPTIONS", UicConst_.Options, false)) {
      return false;
    }
    // .ui files
    {
      Json::Value const& val = info.GetValue("UIC_UI_FILES");
      if (!val.isArray()) {
        return info.LogError("UIC_UI_FILES JSON value is not an array.");
      }
      Json::ArrayIndex const arraySize = val.size();
      for (Json::ArrayIndex ii = 0; ii != arraySize; ++ii) {
        // Test entry closure
        auto testEntry = [&info, ii](bool test, cm::string_view msg) -> bool {
          if (!test) {
            info.LogError(cmStrCat("UIC_UI_FILES entry ", ii, ": ", msg));
          }
          return !test;
        };

        Json::Value const& entry = val[ii];
        if (testEntry(entry.isArray(), "JSON value is not an array.") ||
            testEntry(entry.size() == 2, "JSON array size invalid.")) {
          return false;
        }

        Json::Value const& entryName = entry[0u];
        Json::Value const& entryOptions = entry[1u];
        if (testEntry(entryName.isString(),
                      "JSON value for name is not a string.") ||
            testEntry(entryOptions.isArray(),
                      "JSON value for options is not an array.")) {
          return false;
        }

        auto& uiFile = UicConst_.UiFiles[entryName.asString()];
        InfoT::GetJsonArray(uiFile.Options, entryOptions);
      }
    }

    // -- Evaluate settings
    // Check if uic executable exists (by reading the file time)
    if (!UicConst_.ExecutableTime.Load(UicConst_.Executable)) {
      return info.LogError(cmStrCat("The uic executable ",
                                    MessagePath(UicConst_.Executable),
                                    " does not exist."));
    }
  }

  // -- Headers
  {
    Json::Value const& val = info.GetValue("HEADERS");
    if (!val.isArray()) {
      return info.LogError("HEADERS JSON value is not an array.");
    }
    Json::ArrayIndex const arraySize = val.size();
    for (Json::ArrayIndex ii = 0; ii != arraySize; ++ii) {
      // Test entry closure
      auto testEntry = [&info, ii](bool test, cm::string_view msg) -> bool {
        if (!test) {
          info.LogError(cmStrCat("HEADERS entry ", ii, ": ", msg));
        }
        return !test;
      };

      Json::Value const& entry = val[ii];
      if (testEntry(entry.isArray(), "JSON value is not an array.") ||
          testEntry(entry.size() == 3, "JSON array size invalid.")) {
        return false;
      }

      Json::Value const& entryName = entry[0u];
      Json::Value const& entryFlags = entry[1u];
      Json::Value const& entryBuild = entry[2u];
      if (testEntry(entryName.isString(),
                    "JSON value for name is not a string.") ||
          testEntry(entryFlags.isString(),
                    "JSON value for flags is not a string.") ||
          testEntry(entryBuild.isString(),
                    "JSON value for build path is not a string.")) {
        return false;
      }

      std::string name = entryName.asString();
      std::string flags = entryFlags.asString();
      std::string build = entryBuild.asString();
      if (testEntry(flags.size() == 2, "Invalid flags string size")) {
        return false;
      }

      cmFileTime fileTime;
      if (!fileTime.Load(name)) {
        return info.LogError(cmStrCat(
          "The header file ", this->MessagePath(name), " does not exist."));
      }

      SourceFileHandleT sourceHandle = std::make_shared<SourceFileT>(name);
      sourceHandle->FileTime = fileTime;
      sourceHandle->IsHeader = true;
      sourceHandle->Moc = (flags[0] == 'M');
      sourceHandle->Uic = (flags[1] == 'U');
      if (sourceHandle->Moc && MocConst().Enabled) {
        if (build.empty()) {
          return info.LogError(
            cmStrCat("Header file ", ii, " build path is empty"));
        }
        sourceHandle->BuildPath = std::move(build);
      }
      BaseEval().Headers.emplace(std::move(name), std::move(sourceHandle));
    }
  }

  // -- Sources
  {
    Json::Value const& val = info.GetValue("SOURCES");
    if (!val.isArray()) {
      return info.LogError("SOURCES JSON value is not an array.");
    }
    Json::ArrayIndex const arraySize = val.size();
    for (Json::ArrayIndex ii = 0; ii != arraySize; ++ii) {
      // Test entry closure
      auto testEntry = [&info, ii](bool test, cm::string_view msg) -> bool {
        if (!test) {
          info.LogError(cmStrCat("SOURCES entry ", ii, ": ", msg));
        }
        return !test;
      };

      Json::Value const& entry = val[ii];
      if (testEntry(entry.isArray(), "JSON value is not an array.") ||
          testEntry(entry.size() == 2, "JSON array size invalid.")) {
        return false;
      }

      Json::Value const& entryName = entry[0u];
      Json::Value const& entryFlags = entry[1u];
      if (testEntry(entryName.isString(),
                    "JSON value for name is not a string.") ||
          testEntry(entryFlags.isString(),
                    "JSON value for flags is not a string.")) {
        return false;
      }

      std::string name = entryName.asString();
      std::string flags = entryFlags.asString();
      if (testEntry(flags.size() == 2, "Invalid flags string size")) {
        return false;
      }

      cmFileTime fileTime;
      if (!fileTime.Load(name)) {
        return info.LogError(cmStrCat(
          "The source file ", this->MessagePath(name), " does not exist."));
      }

      SourceFileHandleT sourceHandle = std::make_shared<SourceFileT>(name);
      sourceHandle->FileTime = fileTime;
      sourceHandle->IsHeader = false;
      sourceHandle->Moc = (flags[0] == 'M');
      sourceHandle->Uic = (flags[1] == 'U');
      BaseEval().Sources.emplace(std::move(name), std::move(sourceHandle));
    }
  }

  // -- Init derived information
  // Moc variables
  if (MocConst().Enabled) {
    // Compose moc includes list
    {
      // Compute framework paths
      std::set<std::string> frameworkPaths;
      for (std::string const& path : MocConst().IncludePaths) {
        // Extract framework path
        if (cmHasLiteralSuffix(path, ".framework/Headers")) {
          // Go up twice to get to the framework root
          std::vector<std::string> pathComponents;
          cmSystemTools::SplitPath(path, pathComponents);
          frameworkPaths.emplace(cmSystemTools::JoinPath(
            pathComponents.begin(), pathComponents.end() - 2));
        }
      }
      // Reserve options
      MocConst_.OptionsIncludes.reserve(MocConst().IncludePaths.size() +
                                        frameworkPaths.size() * 2);
      // Append includes
      for (std::string const& path : MocConst().IncludePaths) {
        MocConst_.OptionsIncludes.emplace_back("-I" + path);
      }
      // Append framework includes
      for (std::string const& path : frameworkPaths) {
        MocConst_.OptionsIncludes.emplace_back("-F");
        MocConst_.OptionsIncludes.push_back(path);
      }
    }

    // Compose moc definitions list
    {
      MocConst_.OptionsDefinitions.reserve(MocConst().Definitions.size());
      for (std::string const& def : MocConst().Definitions) {
        MocConst_.OptionsDefinitions.emplace_back("-D" + def);
      }
    }
  }

  return true;
}

template <class JOBTYPE>
void cmQtAutoMocUicT::CreateParseJobs(SourceFileMapT const& sourceMap)
{
  cmFileTime const parseCacheTime = BaseEval().ParseCacheTime;
  ParseCacheT& parseCache = BaseEval().ParseCache;
  for (auto& src : sourceMap) {
    // Get or create the file parse data reference
    ParseCacheT::GetOrInsertT cacheEntry = parseCache.GetOrInsert(src.first);
    src.second->ParseData = std::move(cacheEntry.first);
    // Create a parse job if the cache file was missing or is older
    if (cacheEntry.second || src.second->FileTime.Newer(parseCacheTime)) {
      BaseEval().ParseCacheChanged = true;
      WorkerPool().EmplaceJob<JOBTYPE>(src.second);
    }
  }
}

/** Concurrently callable implementation of cmSystemTools::CollapseFullPath */
std::string cmQtAutoMocUicT::CollapseFullPathTS(std::string const& path) const
{
  std::lock_guard<std::mutex> guard(CMakeLibMutex_);
  return cmSystemTools::CollapseFullPath(path, ProjectDirs().CurrentSource);
}

void cmQtAutoMocUicT::InitJobs()
{
  // Add moc_predefs.h job
  if (MocConst().Enabled && !MocConst().PredefsCmd.empty()) {
    WorkerPool().EmplaceJob<JobMocPredefsT>();
  }

  // Add header parse jobs
  CreateParseJobs<JobParseHeaderT>(BaseEval().Headers);
  // Add source parse jobs
  CreateParseJobs<JobParseSourceT>(BaseEval().Sources);

  // Add parse cache evaluations jobs
  {
    // Add a fence job to ensure all parsing has finished
    WorkerPool().EmplaceJob<JobFenceT>();
    if (MocConst().Enabled) {
      WorkerPool().EmplaceJob<JobEvalCacheMocT>();
    }
    if (UicConst().Enabled) {
      WorkerPool().EmplaceJob<JobEvalCacheUicT>();
    }
    // Add evaluate job
    WorkerPool().EmplaceJob<JobEvalCacheFinishT>();
  }
}

bool cmQtAutoMocUicT::Process()
{
  SettingsFileRead();
  ParseCacheRead();
  if (!CreateDirectories()) {
    return false;
  }
  InitJobs();
  if (!WorkerPool_.Process(this)) {
    return false;
  }
  if (JobError_) {
    return false;
  }
  if (!ParseCacheWrite()) {
    return false;
  }
  if (!SettingsFileWrite()) {
    return false;
  }
  return true;
}

void cmQtAutoMocUicT::SettingsFileRead()
{
  // Compose current settings strings
  {
    cmCryptoHash cryptoHash(cmCryptoHash::AlgoSHA256);
    auto cha = [&cryptoHash](cm::string_view value) {
      cryptoHash.Append(value);
      cryptoHash.Append(";");
    };

    if (MocConst_.Enabled) {
      cryptoHash.Initialize();
      cha(MocConst().Executable);
      for (auto const& item : MocConst().OptionsDefinitions) {
        cha(item);
      }
      for (auto const& item : MocConst().OptionsIncludes) {
        cha(item);
      }
      for (auto const& item : MocConst().OptionsExtra) {
        cha(item);
      }
      for (auto const& item : MocConst().PredefsCmd) {
        cha(item);
      }
      for (auto const& filter : MocConst().DependFilters) {
        cha(filter.Key);
      }
      for (auto const& filter : MocConst().MacroFilters) {
        cha(filter.Key);
      }
      SettingsStringMoc_ = cryptoHash.FinalizeHex();
    }

    if (UicConst().Enabled) {
      cryptoHash.Initialize();
      cha(UicConst().Executable);
      std::for_each(UicConst().Options.begin(), UicConst().Options.end(), cha);
      for (const auto& item : UicConst().UiFiles) {
        cha(item.first);
        auto const& opts = item.second.Options;
        std::for_each(opts.begin(), opts.end(), cha);
      }
      SettingsStringUic_ = cryptoHash.FinalizeHex();
    }
  }

  // Read old settings and compare
  {
    std::string content;
    if (cmQtAutoGenerator::FileRead(content, SettingsFile_)) {
      if (MocConst().Enabled) {
        if (SettingsStringMoc_ != SettingsFind(content, "moc")) {
          MocConst_.SettingsChanged = true;
        }
      }
      if (UicConst().Enabled) {
        if (SettingsStringUic_ != SettingsFind(content, "uic")) {
          UicConst_.SettingsChanged = true;
        }
      }
      // In case any setting changed remove the old settings file.
      // This triggers a full rebuild on the next run if the current
      // build is aborted before writing the current settings in the end.
      if (MocConst().SettingsChanged || UicConst().SettingsChanged) {
        cmSystemTools::RemoveFile(SettingsFile_);
      }
    } else {
      // Settings file read failed
      if (MocConst().Enabled) {
        MocConst_.SettingsChanged = true;
      }
      if (UicConst().Enabled) {
        UicConst_.SettingsChanged = true;
      }
    }
  }
}

bool cmQtAutoMocUicT::SettingsFileWrite()
{
  // Only write if any setting changed
  if (MocConst().SettingsChanged || UicConst().SettingsChanged) {
    if (Log().Verbose()) {
      Log().Info(
        GenT::GEN,
        cmStrCat("Writing the settings file ", MessagePath(SettingsFile_)));
    }
    // Compose settings file content
    std::string content;
    {
      auto SettingAppend = [&content](cm::string_view key,
                                      cm::string_view value) {
        if (!value.empty()) {
          content += cmStrCat(key, ':', value, '\n');
        }
      };
      SettingAppend("moc", SettingsStringMoc_);
      SettingAppend("uic", SettingsStringUic_);
    }
    // Write settings file
    std::string error;
    if (!cmQtAutoGenerator::FileWrite(SettingsFile_, content, &error)) {
      Log().Error(GenT::GEN,
                  cmStrCat("Writing the settings file ",
                           MessagePath(SettingsFile_), " failed.\n", error));
      // Remove old settings file to trigger a full rebuild on the next run
      cmSystemTools::RemoveFile(SettingsFile_);
      return false;
    }
  }
  return true;
}

void cmQtAutoMocUicT::ParseCacheRead()
{
  cm::string_view reason;
  // Don't read the cache if it is invalid
  if (!BaseEval().ParseCacheTime.Load(BaseConst().ParseCacheFile)) {
    reason = "Refreshing parse cache because it doesn't exist.";
  } else if (MocConst().SettingsChanged || UicConst().SettingsChanged) {
    reason = "Refreshing parse cache because the settings changed.";
  } else if (BaseEval().ParseCacheTime.Older(
               BaseConst().CMakeExecutableTime)) {
    reason =
      "Refreshing parse cache because it is older than the CMake executable.";
  }

  if (!reason.empty()) {
    // Don't read but refresh the complete parse cache
    if (Log().Verbose()) {
      Log().Info(GenT::GEN, reason);
    }
    BaseEval().ParseCacheChanged = true;
  } else {
    // Read parse cache
    BaseEval().ParseCache.ReadFromFile(BaseConst().ParseCacheFile);
  }
}

bool cmQtAutoMocUicT::ParseCacheWrite()
{
  if (BaseEval().ParseCacheChanged) {
    if (Log().Verbose()) {
      Log().Info(GenT::GEN,
                 cmStrCat("Writing the parse cache file ",
                          MessagePath(BaseConst().ParseCacheFile)));
    }
    if (!BaseEval().ParseCache.WriteToFile(BaseConst().ParseCacheFile)) {
      Log().Error(GenT::GEN,
                  cmStrCat("Writing the parse cache file ",
                           MessagePath(BaseConst().ParseCacheFile),
                           " failed."));
      return false;
    }
  }
  return true;
}

bool cmQtAutoMocUicT::CreateDirectories()
{
  // Create AUTOGEN include directory
  if (!cmSystemTools::MakeDirectory(BaseConst().AutogenIncludeDir)) {
    Log().Error(GenT::GEN,
                cmStrCat("Creating the AUTOGEN include directory ",
                         MessagePath(BaseConst().AutogenIncludeDir),
                         " failed."));
    return false;
  }
  return true;
}

std::vector<std::string> cmQtAutoMocUicT::dependenciesFromDepFile(
  const char* filePath)
{
  cmGccDepfileContent content = cmReadGccDepfile(filePath);
  if (content.empty()) {
    return {};
  }

  // Moc outputs a depfile with exactly one rule.
  // Discard the rule and return the dependencies.
  return content.front().paths;
}

void cmQtAutoMocUicT::Abort(bool error)
{
  if (error) {
    JobError_.store(true);
  }
  WorkerPool_.Abort();
}

std::string cmQtAutoMocUicT::AbsoluteBuildPath(
  cm::string_view relativePath) const
{
  return cmStrCat(BaseConst().AutogenBuildDir, '/', relativePath);
}

std::string cmQtAutoMocUicT::AbsoluteIncludePath(
  cm::string_view relativePath) const
{
  return cmStrCat(BaseConst().AutogenIncludeDir, '/', relativePath);
}

} // End of unnamed namespace

bool cmQtAutoMocUic(cm::string_view infoFile, cm::string_view config)
{
  return cmQtAutoMocUicT().Run(infoFile, config);
}
