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
#include <cm/optional>
#include <cm/string_view>
#include <cmext/algorithm>

#include <cm3p/json/value.h>

#include "cmsys/FStream.hxx"
#include "cmsys/RegularExpression.hxx"

#include "cmCryptoHash.h"
#include "cmFileTime.h"
#include "cmGccDepfileReader.h"
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
    std::atomic<bool> ParseCacheChanged = ATOMIC_VAR_INIT(false);
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
      return static_cast<cmQtAutoMocUicT*>(this->UserData());
    }

    // -- Accessors. Only valid during Process() call!
    Logger const& Log() const { return this->Gen()->Log(); }
    BaseSettingsT const& BaseConst() const { return this->Gen()->BaseConst(); }
    BaseEvalT& BaseEval() const { return this->Gen()->BaseEval(); }
    MocSettingsT const& MocConst() const { return this->Gen()->MocConst(); }
    MocEvalT& MocEval() const { return this->Gen()->MocEval(); }
    UicSettingsT const& UicConst() const { return this->Gen()->UicConst(); }
    UicEvalT& UicEval() const { return this->Gen()->UicEval(); }

    // -- Logging
    std::string MessagePath(cm::string_view path) const
    {
      return this->Gen()->MessagePath(path);
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
    void Process() override {}
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

  private:
    void MaybeWriteMocResponseFile(std::string const& outputFile,
                                   std::vector<std::string>& cmd) const;
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
    std::vector<std::string> initialDependencies() const;
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
  cmWorkerPool& WorkerPool() { return this->WorkerPool_; }
  void AbortError() { this->Abort(true); }
  void AbortSuccess() { this->Abort(false); }

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
  std::vector<std::string> dependenciesFromDepFile(const char* filePath);

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
    this->Base = this->Base.substr(basePrefixLength);
  }
}

void cmQtAutoMocUicT::ParseCacheT::FileT::Clear()
{
  this->Moc.Macro.clear();
  this->Moc.Include.Underscore.clear();
  this->Moc.Include.Dot.clear();
  this->Moc.Depends.clear();

  this->Uic.Include.clear();
  this->Uic.Depends.clear();
}

cmQtAutoMocUicT::ParseCacheT::GetOrInsertT
cmQtAutoMocUicT::ParseCacheT::GetOrInsert(std::string const& fileName)
{
  // Find existing entry
  {
    auto it = this->Map_.find(fileName);
    if (it != this->Map_.end()) {
      return GetOrInsertT{ it->second, false };
    }
  }

  // Insert new entry
  return GetOrInsertT{
    this->Map_.emplace(fileName, std::make_shared<FileT>()).first->second, true
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
      fileHandle = this->GetOrInsert(line).first;
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
  ofs << "# Generated by CMake. Changes will be overwritten.\n";
  for (auto const& pair : this->Map_) {
    ofs << pair.first << '\n';
    FileT const& file = *pair.second;
    if (!file.Moc.Macro.empty()) {
      ofs << " mmc:" << file.Moc.Macro << '\n';
    }
    for (IncludeKeyT const& item : file.Moc.Include.Underscore) {
      ofs << " miu:" << item.Key << '\n';
    }
    for (IncludeKeyT const& item : file.Moc.Include.Dot) {
      ofs << " mid:" << item.Key << '\n';
    }
    for (std::string const& item : file.Moc.Depends) {
      ofs << " mdp:" << item << '\n';
    }
    for (IncludeKeyT const& item : file.Uic.Include) {
      ofs << " uic:" << item.Key << '\n';
    }
    for (std::string const& item : file.Uic.Depends) {
      ofs << " udp:" << item << '\n';
    }
  }
  return ofs.Close();
}

cmQtAutoMocUicT::BaseSettingsT::BaseSettingsT() = default;
cmQtAutoMocUicT::BaseSettingsT::~BaseSettingsT() = default;

cmQtAutoMocUicT::MocSettingsT::MocSettingsT()
{
  this->RegExpInclude.compile(
    "(^|\n)[ \t]*#[ \t]*include[ \t]+"
    "[\"<](([^ \">]+/)?moc_[^ \">/]+\\.cpp|[^ \">]+\\.moc)[\">]");
}

cmQtAutoMocUicT::MocSettingsT::~MocSettingsT() = default;

bool cmQtAutoMocUicT::MocSettingsT::skipped(std::string const& fileName) const
{
  return (!this->Enabled ||
          (this->SkipList.find(fileName) != this->SkipList.end()));
}

std::string cmQtAutoMocUicT::MocSettingsT::MacrosString() const
{
  std::string res;
  const auto itB = this->MacroFilters.cbegin();
  const auto itE = this->MacroFilters.cend();
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
  this->RegExpInclude.compile("(^|\n)[ \t]*#[ \t]*include[ \t]+"
                              "[\"<](([^ \">]+/)?ui_[^ \">/]+\\.h)[\">]");
}

cmQtAutoMocUicT::UicSettingsT::~UicSettingsT() = default;

bool cmQtAutoMocUicT::UicSettingsT::skipped(std::string const& fileName) const
{
  return (!this->Enabled ||
          (this->SkipList.find(fileName) != this->SkipList.end()));
}

void cmQtAutoMocUicT::JobT::LogError(GenT genType,
                                     cm::string_view message) const
{
  this->Gen()->AbortError();
  this->Gen()->Log().Error(genType, message);
}

void cmQtAutoMocUicT::JobT::LogCommandError(
  GenT genType, cm::string_view message,
  std::vector<std::string> const& command, std::string const& output) const
{
  this->Gen()->AbortError();
  this->Gen()->Log().ErrorCommand(genType, message, command, output);
}

bool cmQtAutoMocUicT::JobT::RunProcess(GenT genType,
                                       cmWorkerPool::ProcessResultT& result,
                                       std::vector<std::string> const& command,
                                       std::string* infoMessage)
{
  // Log command
  if (this->Log().Verbose()) {
    cm::string_view info;
    if (infoMessage != nullptr) {
      info = *infoMessage;
    }
    this->Log().Info(
      genType,
      cmStrCat(info, info.empty() || cmHasSuffix(info, '\n') ? "" : "\n",
               QuotedCommand(command), '\n'));
  }
  // Run command
  return this->cmWorkerPool::JobT::RunProcess(
    result, command, this->BaseConst().AutogenBuildDir);
}

void cmQtAutoMocUicT::JobMocPredefsT::Process()
{
  // (Re)generate moc_predefs.h on demand
  std::unique_ptr<std::string> reason;
  if (this->Log().Verbose()) {
    reason = cm::make_unique<std::string>();
  }
  if (!this->Update(reason.get())) {
    return;
  }
  std::string const& predefsFileAbs = this->MocConst().PredefsFileAbs;
  {
    cmWorkerPool::ProcessResultT result;
    {
      // Compose command
      std::vector<std::string> cmd = this->MocConst().PredefsCmd;
      // Add definitions
      cm::append(cmd, this->MocConst().OptionsDefinitions);
      // Add includes
      cm::append(cmd, this->MocConst().OptionsIncludes);
      // Execute command
      if (!this->RunProcess(GenT::MOC, result, cmd, reason.get())) {
        this->LogCommandError(GenT::MOC,
                              cmStrCat("The content generation command for ",
                                       this->MessagePath(predefsFileAbs),
                                       " failed.\n", result.ErrorMessage),
                              cmd, result.StdOut);
        return;
      }
    }

    // (Re)write predefs file only on demand
    if (cmQtAutoGenerator::FileDiffers(predefsFileAbs, result.StdOut)) {
      if (!cmQtAutoGenerator::FileWrite(predefsFileAbs, result.StdOut)) {
        this->LogError(
          GenT::MOC,
          cmStrCat("Writing ", this->MessagePath(predefsFileAbs), " failed."));
        return;
      }
    } else {
      // Touch to update the time stamp
      if (this->Log().Verbose()) {
        this->Log().Info(GenT::MOC,
                         "Touching " + this->MessagePath(predefsFileAbs));
      }
      if (!cmSystemTools::Touch(predefsFileAbs, false)) {
        this->LogError(GenT::MOC,
                       cmStrCat("Touching ", this->MessagePath(predefsFileAbs),
                                " failed."));
        return;
      }
    }
  }

  // Read file time afterwards
  if (!this->MocEval().PredefsTime.Load(predefsFileAbs)) {
    this->LogError(GenT::MOC,
                   cmStrCat("Reading the file time of ",
                            this->MessagePath(predefsFileAbs), " failed."));
    return;
  }
}

bool cmQtAutoMocUicT::JobMocPredefsT::Update(std::string* reason) const
{
  // Test if the file exists
  if (!this->MocEval().PredefsTime.Load(this->MocConst().PredefsFileAbs)) {
    if (reason != nullptr) {
      *reason = cmStrCat("Generating ",
                         this->MessagePath(this->MocConst().PredefsFileAbs),
                         ", because it doesn't exist.");
    }
    return true;
  }

  // Test if the settings changed
  if (this->MocConst().SettingsChanged) {
    if (reason != nullptr) {
      *reason = cmStrCat("Generating ",
                         this->MessagePath(this->MocConst().PredefsFileAbs),
                         ", because the moc settings changed.");
    }
    return true;
  }

  // Test if the executable is newer
  {
    std::string const& exec = this->MocConst().PredefsCmd.at(0);
    cmFileTime execTime;
    if (execTime.Load(exec)) {
      if (this->MocEval().PredefsTime.Older(execTime)) {
        if (reason != nullptr) {
          *reason = cmStrCat(
            "Generating ", this->MessagePath(this->MocConst().PredefsFileAbs),
            " because it is older than ", this->MessagePath(exec), '.');
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
  this->FileHandle->ParseData->Clear();
  std::string const& fileName = this->FileHandle->FileName;
  // Write info
  if (this->Log().Verbose()) {
    this->Log().Info(GenT::GEN,
                     cmStrCat("Parsing ", this->MessagePath(fileName)));
  }
  // Read file content
  {
    std::string error;
    if (!cmQtAutoGenerator::FileRead(this->Content, fileName, &error)) {
      this->LogError(GenT::GEN,
                     cmStrCat("Could not read ", this->MessagePath(fileName),
                              ".\n", error));
      return false;
    }
  }
  // Warn if empty
  if (this->Content.empty()) {
    this->Log().Warning(GenT::GEN,
                        cmStrCat(this->MessagePath(fileName), " is empty."));
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
  for (KeyExpT const& filter : this->MocConst().MacroFilters) {
    // Run a simple find string check
    if (this->Content.find(filter.Key) == std::string::npos) {
      continue;
    }
    // Run the expensive regular expression check loop
    cmsys::RegularExpressionMatch match;
    if (filter.Exp.find(this->Content.c_str(), match)) {
      // Keep detected macro name
      this->FileHandle->ParseData->Moc.Macro = filter.Key;
      return;
    }
  }
}

void cmQtAutoMocUicT::JobParseT::MocDependecies()
{
  if (this->MocConst().DependFilters.empty() ||
      this->MocConst().CanOutputDependencies) {
    return;
  }

  // Find dependency strings
  std::set<std::string> parseDepends;
  for (KeyExpT const& filter : this->MocConst().DependFilters) {
    // Run a simple find string check
    if (this->Content.find(filter.Key) == std::string::npos) {
      continue;
    }
    // Run the expensive regular expression check loop
    const char* contentChars = this->Content.c_str();
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
    auto& Depends = this->FileHandle->ParseData->Moc.Depends;
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
  if (this->Content.find("moc") == std::string::npos) {
    return;
  }

  std::set<std::string> underscore;
  std::set<std::string> dot;
  {
    const char* contentChars = this->Content.c_str();
    cmsys::RegularExpression const& regExp = this->MocConst().RegExpInclude;
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
  auto& Include = this->FileHandle->ParseData->Moc.Include;
  this->CreateKeys(Include.Underscore, underscore, MocUnderscoreLength);
  this->CreateKeys(Include.Dot, dot, 0);
}

void cmQtAutoMocUicT::JobParseT::UicIncludes()
{
  if (this->Content.find("ui_") == std::string::npos) {
    return;
  }

  std::set<std::string> includes;
  {
    const char* contentChars = this->Content.c_str();
    cmsys::RegularExpression const& regExp = this->UicConst().RegExpInclude;
    cmsys::RegularExpressionMatch match;
    while (regExp.find(contentChars, match)) {
      includes.emplace(match.match(2));
      // Forward content pointer
      contentChars += match.end();
    }
  }
  this->CreateKeys(this->FileHandle->ParseData->Uic.Include, includes,
                   UiUnderscoreLength);
}

void cmQtAutoMocUicT::JobParseHeaderT::Process()
{
  if (!this->ReadFile()) {
    return;
  }
  // Moc parsing
  if (this->FileHandle->Moc) {
    this->MocMacro();
    this->MocDependecies();
  }
  // Uic parsing
  if (this->FileHandle->Uic) {
    this->UicIncludes();
  }
}

void cmQtAutoMocUicT::JobParseSourceT::Process()
{
  if (!this->ReadFile()) {
    return;
  }
  // Moc parsing
  if (this->FileHandle->Moc) {
    this->MocMacro();
    this->MocDependecies();
    this->MocIncludes();
  }
  // Uic parsing
  if (this->FileHandle->Uic) {
    this->UicIncludes();
  }
}

std::string cmQtAutoMocUicT::JobEvalCacheT::MessageSearchLocations() const
{
  std::string res;
  res.reserve(512);
  for (std::string const& path : this->SearchLocations) {
    res += "  ";
    res += this->MessagePath(path);
    res += '\n';
  }
  return res;
}

void cmQtAutoMocUicT::JobEvalCacheMocT::Process()
{
  // Evaluate headers
  for (auto const& pair : this->BaseEval().Headers) {
    if (!this->EvalHeader(pair.second)) {
      return;
    }
  }
  // Evaluate sources
  for (auto const& pair : this->BaseEval().Sources) {
    if (!this->EvalSource(pair.second)) {
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
    if (this->BaseConst().MultiConfig) {
      handle->OutputFile =
        this->Gen()->AbsoluteIncludePath(sourceFile.BuildPath);
    } else {
      handle->OutputFile =
        this->Gen()->AbsoluteBuildPath(sourceFile.BuildPath);
    }

    // Register mapping in headers map
    this->RegisterMapping(handle);
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
  bool const relaxedMode = this->MocConst().RelaxedMode;
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
    this->LogError(GenT::MOC,
                   cmStrCat(this->MessagePath(sourceFile.FileName),
                            "\ncontains a ", Quoted(parseData.Macro),
                            " macro, but does not include ",
                            this->MessagePath(sourceBase + ".moc"),
                            "!\nConsider to\n  - add #include \"", sourceBase,
                            ".moc\"\n  - enable SKIP_AUTOMOC for this file"));
    return false;
  }

  // Evaluate "moc_" includes
  for (IncludeKeyT const& incKey : parseData.Include.Underscore) {
    SourceFileHandleT headerHandle;
    {
      std::string const headerBase = cmStrCat(incKey.Dir, incKey.Base);
      if (!this->FindIncludedHeader(headerHandle, sourceDirPrefix,
                                    headerBase)) {
        this->LogError(
          GenT::MOC,
          cmStrCat(this->MessagePath(sourceFile.FileName),
                   "\nincludes the moc file ", this->MessagePath(incKey.Key),
                   ",\nbut a header ", this->MessageHeader(headerBase),
                   "\ncould not be found "
                   "in the following directories\n",
                   this->MessageSearchLocations()));
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
      this->Log().Warning(
        GenT::MOC,
        cmStrCat(this->MessagePath(sourceFile.FileName), "\ncontains a ",
                 Quoted(parseData.Macro), " macro, but does not include ",
                 this->MessagePath(sourceBase + ".moc"),
                 ".\nInstead it includes ", this->MessagePath(incKey.Key),
                 ".\nRunning moc on the source\n  ",
                 this->MessagePath(sourceFile.FileName), "!\nBetter include ",
                 this->MessagePath(sourceBase + ".moc"),
                 " for compatibility with regular mode.\n",
                 "This is a CMAKE_AUTOMOC_RELAXED_MODE warning.\n"));

      // Create mapping
      if (!this->RegisterIncluded(incKey.Key, source, source)) {
        return false;
      }
      continue;
    }

    // Check if header is skipped
    if (this->MocConst().skipped(headerHandle->FileName)) {
      continue;
    }
    // Create mapping
    if (!this->RegisterIncluded(incKey.Key, source, std::move(headerHandle))) {
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
        if (!this->RegisterIncluded(incKey.Key, source, source)) {
          return false;
        }
        continue;
      }
      // Try to find a header instead but issue a warning.
      // This is for KDE4 compatibility.
      SourceFileHandleT headerHandle;
      {
        std::string const headerBase = cmStrCat(incKey.Dir, incKey.Base);
        if (!this->FindIncludedHeader(headerHandle, sourceDirPrefix,
                                      headerBase)) {
          this->LogError(
            GenT::MOC,
            cmStrCat(
              this->MessagePath(sourceFile.FileName),
              "\nincludes the moc file ", this->MessagePath(incKey.Key),
              ",\nwhich seems to be the moc file from a different source "
              "file.\nCMAKE_AUTOMOC_RELAXED_MODE:\nAlso a matching header ",
              this->MessageHeader(headerBase),
              "\ncould not be found in the following directories\n",
              this->MessageSearchLocations()));
          return false;
        }
      }
      // Check if header is skipped
      if (this->MocConst().skipped(headerHandle->FileName)) {
        continue;
      }
      // Issue a warning
      if (ownMoc && parseData.Macro.empty()) {
        this->Log().Warning(
          GenT::MOC,
          cmStrCat(
            this->MessagePath(sourceFile.FileName), "\nincludes the moc file ",
            this->MessagePath(incKey.Key), ", but does not contain a\n",
            this->MocConst().MacrosString(),
            " macro.\nRunning moc on the header\n  ",
            this->MessagePath(headerHandle->FileName), "!\nBetter include ",
            this->MessagePath("moc_" + incKey.Base + ".cpp"),
            " for a compatibility with regular mode.\n",
            "This is a CMAKE_AUTOMOC_RELAXED_MODE warning.\n"));
      } else {
        this->Log().Warning(
          GenT::MOC,
          cmStrCat(
            this->MessagePath(sourceFile.FileName), "\nincludes the moc file ",
            this->MessagePath(incKey.Key), " instead of ",
            this->MessagePath("moc_" + incKey.Base + ".cpp"),
            ".\nRunning moc on the header\n  ",
            this->MessagePath(headerHandle->FileName), "!\nBetter include ",
            this->MessagePath("moc_" + incKey.Base + ".cpp"),
            " for compatibility with regular mode.\n",
            "This is a CMAKE_AUTOMOC_RELAXED_MODE warning.\n"));
      }
      // Create mapping
      if (!this->RegisterIncluded(incKey.Key, source,
                                  std::move(headerHandle))) {
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
        this->LogError(
          GenT::MOC,
          cmStrCat(this->MessagePath(sourceFile.FileName),
                   "\nincludes the moc file ", this->MessagePath(incKey.Key),
                   ",\nwhich seems to be the moc file from a different "
                   "source file.\nThis is not supported.  Include ",
                   this->MessagePath(sourceBase + ".moc"),
                   " to run moc on this source file."));
        return false;
      }
      // Accept but issue a warning if moc isn't required
      if (parseData.Macro.empty()) {
        this->Log().Warning(
          GenT::MOC,
          cmStrCat(this->MessagePath(sourceFile.FileName),
                   "\nincludes the moc file ", this->MessagePath(incKey.Key),
                   ", but does not contain a ",
                   this->MocConst().MacrosString(), " macro."));
      }
      // Create mapping
      if (!this->RegisterIncluded(incKey.Key, source, source)) {
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
  this->SearchLocations.clear();

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
        auto it = this->BaseEval().Headers.find(testPath);
        if (it != this->BaseEval().Headers.end()) {
          headerHandle = it->second;
          found = true;
          break;
        }
      }

      // Created and return discovered file entry
      {
        SourceFileHandleT& handle =
          this->MocEval().HeadersDiscovered[testPath];
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
  auto const& includePaths = this->MocConst().IncludePaths;
  return std::any_of(
    includePaths.begin(), includePaths.end(),
    [&findHeader, &includeBase](std::string const& path) -> bool {
      return findHeader(cmStrCat(path, '/', includeBase));
    });
}

bool cmQtAutoMocUicT::JobEvalCacheMocT::RegisterIncluded(
  std::string const& includeString, SourceFileHandleT includerFileHandle,
  SourceFileHandleT sourceFileHandle) const
{
  // Check if this file is already included
  MappingHandleT& handle = this->MocEval().Includes[includeString];
  if (handle) {
    // Check if the output file would be generated from different source files
    if (handle->SourceFile != sourceFileHandle) {
      std::string files =
        cmStrCat("  ", this->MessagePath(includerFileHandle->FileName), '\n');
      for (auto const& item : handle->IncluderFiles) {
        files += cmStrCat("  ", this->MessagePath(item->FileName), '\n');
      }
      this->LogError(
        GenT::MOC,
        cmStrCat("The source files\n", files,
                 "contain the same include string ",
                 this->MessagePath(includeString),
                 ", but\nthe moc file would be generated from different "
                 "source files\n  ",
                 this->MessagePath(sourceFileHandle->FileName), " and\n  ",
                 this->MessagePath(handle->SourceFile->FileName),
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
  handle->OutputFile = this->Gen()->AbsoluteIncludePath(includeString);

  // Register mapping in sources/headers map
  this->RegisterMapping(handle);
  return true;
}

void cmQtAutoMocUicT::JobEvalCacheMocT::RegisterMapping(
  MappingHandleT mappingHandle) const
{
  auto& regMap = mappingHandle->SourceFile->IsHeader
    ? this->MocEval().HeaderMappings
    : this->MocEval().SourceMappings;
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
  return this->MessagePath(cmStrCat(
    headerBase, ".{", cmJoin(this->BaseConst().HeaderExtensions, ","), '}'));
}

void cmQtAutoMocUicT::JobEvalCacheUicT::Process()
{
  // Prepare buffers
  this->SearchLocations.reserve((this->UicConst().SearchPaths.size() + 1) * 2);

  // Evaluate headers
  for (auto const& pair : this->BaseEval().Headers) {
    if (!this->EvalFile(pair.second)) {
      return;
    }
  }
  // Evaluate sources
  for (auto const& pair : this->BaseEval().Sources) {
    if (!this->EvalFile(pair.second)) {
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
  return std::all_of(
    Include.begin(), Include.end(),
    [this, &sourceDirPrefix, &sourceFile,
     &sourceFileHandle](IncludeKeyT const& incKey) -> bool {
      // Find .ui file
      this->UiName = cmStrCat(incKey.Base, ".ui");
      if (!this->FindIncludedUi(sourceDirPrefix, incKey.Dir)) {
        this->LogError(
          GenT::UIC,
          cmStrCat(this->MessagePath(sourceFile.FileName),
                   "\nincludes the uic file ", this->MessagePath(incKey.Key),
                   ",\nbut the user interface file ",
                   this->MessagePath(this->UiName),
                   "\ncould not be found in the following directories\n",
                   this->MessageSearchLocations()));
        return false;
      }
      // Check if the file is skipped
      if (this->UicConst().skipped(this->UiFileHandle->FileName)) {
        return true;
      }
      // Register mapping
      return this->RegisterMapping(incKey.Key, sourceFileHandle);
    });
}

bool cmQtAutoMocUicT::JobEvalCacheUicT::FindIncludedUi(
  cm::string_view sourceDirPrefix, cm::string_view includePrefix)
{
  // Clear locations buffer
  this->SearchLocations.clear();

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
  if (!includePrefix.empty()) {
    if (findUi(cmStrCat(sourceDirPrefix, includePrefix, this->UiName))) {
      return true;
    }
  }
  if (findUi(cmStrCat(sourceDirPrefix, this->UiName))) {
    return true;
  }
  // Additional AUTOUIC search paths
  auto const& searchPaths = this->UicConst().SearchPaths;
  if (!searchPaths.empty()) {
    for (std::string const& sPath : searchPaths) {
      if (findUi(cmStrCat(sPath, '/', this->UiName))) {
        return true;
      }
    }
    if (!includePrefix.empty()) {
      for (std::string const& sPath : searchPaths) {
        if (findUi(cmStrCat(sPath, '/', includePrefix, this->UiName))) {
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
  auto& Includes = this->Gen()->UicEval().Includes;
  auto it = Includes.find(includeString);
  if (it != Includes.end()) {
    MappingHandleT const& handle = it->second;
    if (handle->SourceFile != this->UiFileHandle) {
      // The output file already gets generated - from a different .ui file!
      std::string files =
        cmStrCat("  ", this->MessagePath(includerFileHandle->FileName), '\n');
      for (auto const& item : handle->IncluderFiles) {
        files += cmStrCat("  ", this->MessagePath(item->FileName), '\n');
      }
      this->LogError(
        GenT::UIC,
        cmStrCat(
          "The source files\n", files, "contain the same include string ",
          Quoted(includeString),
          ", but\nthe uic file would be generated from different "
          "user interface files\n  ",
          this->MessagePath(this->UiFileHandle->FileName), " and\n  ",
          this->MessagePath(handle->SourceFile->FileName),
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
    handle->SourceFile = this->UiFileHandle;
    handle->OutputFile = this->Gen()->AbsoluteIncludePath(includeString);
    // Register mapping
    Includes.emplace(includeString, std::move(handle));
  }
  return true;
}

void cmQtAutoMocUicT::JobEvalCacheFinishT::Process()
{
  // Add discovered header parse jobs
  this->Gen()->CreateParseJobs<JobParseHeaderT>(
    this->MocEval().HeadersDiscovered);

  // Add dependency probing jobs
  {
    // Add fence job to ensure all parsing has finished
    this->Gen()->WorkerPool().EmplaceJob<JobFenceT>();
    if (this->MocConst().Enabled) {
      this->Gen()->WorkerPool().EmplaceJob<JobProbeDepsMocT>();
    }
    if (this->UicConst().Enabled) {
      this->Gen()->WorkerPool().EmplaceJob<JobProbeDepsUicT>();
    }
    // Add probe finish job
    this->Gen()->WorkerPool().EmplaceJob<JobProbeDepsFinishT>();
  }
}

void cmQtAutoMocUicT::JobProbeDepsMocT::Process()
{
  // Create moc header jobs
  for (auto const& pair : this->MocEval().HeaderMappings) {
    // Register if this mapping is a candidate for mocs_compilation.cpp
    bool const compFile = pair.second->IncludeString.empty();
    if (compFile) {
      this->MocEval().CompFiles.emplace_back(
        pair.second->SourceFile->BuildPath);
    }
    if (!this->Generate(pair.second, compFile)) {
      return;
    }
  }

  // Create moc source jobs
  for (auto const& pair : this->MocEval().SourceMappings) {
    if (!this->Generate(pair.second, false)) {
      return;
    }
  }
}

bool cmQtAutoMocUicT::JobProbeDepsMocT::Generate(MappingHandleT const& mapping,
                                                 bool compFile) const
{
  std::unique_ptr<std::string> reason;
  if (this->Log().Verbose()) {
    reason = cm::make_unique<std::string>();
  }
  if (this->Probe(*mapping, reason.get())) {
    // Register the parent directory for creation
    this->MocEval().OutputDirs.emplace(
      cmQtAutoGen::ParentDir(mapping->OutputFile));
    // Fetch the cache entry for the source file
    std::string const& sourceFile = mapping->SourceFile->FileName;
    ParseCacheT::GetOrInsertT cacheEntry =
      this->BaseEval().ParseCache.GetOrInsert(sourceFile);
    // Add moc job
    this->Gen()->WorkerPool().EmplaceJob<JobCompileMocT>(
      mapping, std::move(reason), std::move(cacheEntry.first));
    // Check if a moc job for a mocs_compilation.cpp entry was generated
    if (compFile) {
      this->MocEval().CompUpdated = true;
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
      *reason = cmStrCat("Generating ", this->MessagePath(outputFile),
                         ", because it doesn't exist, from ",
                         this->MessagePath(sourceFile));
    }
    return true;
  }

  // Test if any setting changed
  if (this->MocConst().SettingsChanged) {
    if (reason != nullptr) {
      *reason = cmStrCat("Generating ", this->MessagePath(outputFile),
                         ", because the uic settings changed, from ",
                         this->MessagePath(sourceFile));
    }
    return true;
  }

  // Test if the source file is newer
  if (outputFileTime.Older(mapping.SourceFile->FileTime)) {
    if (reason != nullptr) {
      *reason = cmStrCat("Generating ", this->MessagePath(outputFile),
                         ", because it's older than its source file, from ",
                         this->MessagePath(sourceFile));
    }
    return true;
  }

  // Test if the moc_predefs file is newer
  if (!this->MocConst().PredefsFileAbs.empty()) {
    if (outputFileTime.Older(this->MocEval().PredefsTime)) {
      if (reason != nullptr) {
        *reason = cmStrCat("Generating ", this->MessagePath(outputFile),
                           ", because it's older than ",
                           this->MessagePath(this->MocConst().PredefsFileAbs),
                           ", from ", this->MessagePath(sourceFile));
      }
      return true;
    }
  }

  // Test if the moc executable is newer
  if (outputFileTime.Older(this->MocConst().ExecutableTime)) {
    if (reason != nullptr) {
      *reason = cmStrCat("Generating ", this->MessagePath(outputFile),
                         ", because it's older than the moc executable, from ",
                         this->MessagePath(sourceFile));
    }
    return true;
  }

  // Test if a dependency file is newer
  {
    // Check dependency timestamps
    std::string const sourceDir = SubDirPrefix(sourceFile);
    auto& dependencies = mapping.SourceFile->ParseData->Moc.Depends;
    for (auto it = dependencies.begin(); it != dependencies.end(); ++it) {
      auto& dep = *it;

      // Find dependency file
      auto const depMatch = this->FindDependency(sourceDir, dep);
      if (depMatch.first.empty()) {
        if (reason != nullptr) {
          *reason = cmStrCat("Generating ", this->MessagePath(outputFile),
                             " from ", this->MessagePath(sourceFile),
                             ", because its dependency ",
                             this->MessagePath(dep), " vanished.");
        }
        dependencies.erase(it);
        this->BaseEval().ParseCacheChanged = true;
        return true;
      }

      // Test if dependency file is older
      if (outputFileTime.Older(depMatch.second)) {
        if (reason != nullptr) {
          *reason = cmStrCat("Generating ", this->MessagePath(outputFile),
                             ", because it's older than its dependency file ",
                             this->MessagePath(depMatch.first), ", from ",
                             this->MessagePath(sourceFile));
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
  if (this->MocConst().CanOutputDependencies) {
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
  for (std::string const& includePath : this->MocConst().IncludePaths) {
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
  for (auto const& pair : this->Gen()->UicEval().Includes) {
    MappingHandleT const& mapping = pair.second;
    std::unique_ptr<std::string> reason;
    if (this->Log().Verbose()) {
      reason = cm::make_unique<std::string>();
    }
    if (!this->Probe(*mapping, reason.get())) {
      continue;
    }

    // Register the parent directory for creation
    this->UicEval().OutputDirs.emplace(
      cmQtAutoGen::ParentDir(mapping->OutputFile));
    // Add uic job
    this->Gen()->WorkerPool().EmplaceJob<JobCompileUicT>(mapping,
                                                         std::move(reason));
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
      *reason = cmStrCat("Generating ", this->MessagePath(outputFile),
                         ", because it doesn't exist, from ",
                         this->MessagePath(sourceFile));
    }
    return true;
  }

  // Test if the uic settings changed
  if (this->UicConst().SettingsChanged) {
    if (reason != nullptr) {
      *reason = cmStrCat("Generating ", this->MessagePath(outputFile),
                         ", because the uic settings changed, from ",
                         this->MessagePath(sourceFile));
    }
    return true;
  }

  // Test if the source file is newer
  if (outputFileTime.Older(mapping.SourceFile->FileTime)) {
    if (reason != nullptr) {
      *reason = cmStrCat("Generating ", this->MessagePath(outputFile),
                         " because it's older than the source file ",
                         this->MessagePath(sourceFile));
    }
    return true;
  }

  // Test if the uic executable is newer
  if (outputFileTime.Older(this->UicConst().ExecutableTime)) {
    if (reason != nullptr) {
      *reason = cmStrCat("Generating ", this->MessagePath(outputFile),
                         ", because it's older than the uic executable, from ",
                         this->MessagePath(sourceFile));
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
          this->LogError(genType,
                         cmStrCat("Creating directory ",
                                  this->MessagePath(dirName), " failed."));
          return;
        }
      }
    };
    if (this->MocConst().Enabled && this->UicConst().Enabled) {
      StringSet outputDirs = this->MocEval().OutputDirs;
      outputDirs.insert(this->UicEval().OutputDirs.begin(),
                        this->UicEval().OutputDirs.end());
      createDirs(GenT::GEN, outputDirs);
    } else if (this->MocConst().Enabled) {
      createDirs(GenT::MOC, this->MocEval().OutputDirs);
    } else if (this->UicConst().Enabled) {
      createDirs(GenT::UIC, this->UicEval().OutputDirs);
    }
  }

  if (this->MocConst().Enabled) {
    // Add mocs compilations job
    this->Gen()->WorkerPool().EmplaceJob<JobMocsCompilationT>();
  }

  if (!this->BaseConst().DepFile.empty()) {
    // Add job to merge dep files
    this->Gen()->WorkerPool().EmplaceJob<JobDepFilesMergeT>();
  }

  // Add finish job
  this->Gen()->WorkerPool().EmplaceJob<JobFinishT>();
}

void cmQtAutoMocUicT::JobCompileMocT::Process()
{
  std::string const& sourceFile = this->Mapping->SourceFile->FileName;
  std::string const& outputFile = this->Mapping->OutputFile;

  // Compose moc command
  std::vector<std::string> cmd;
  {
    // Reserve large enough
    cmd.reserve(this->MocConst().OptionsDefinitions.size() +
                this->MocConst().OptionsIncludes.size() +
                this->MocConst().OptionsExtra.size() + 16);
    cmd.push_back(this->MocConst().Executable);
    // Add definitions
    cm::append(cmd, this->MocConst().OptionsDefinitions);
    // Add includes
    cm::append(cmd, this->MocConst().OptionsIncludes);
    // Add predefs include
    if (!this->MocConst().PredefsFileAbs.empty()) {
      cmd.emplace_back("--include");
      cmd.push_back(this->MocConst().PredefsFileAbs);
    }
    // Add path prefix on demand
    if (this->MocConst().PathPrefix && this->Mapping->SourceFile->IsHeader) {
      for (std::string const& dir : this->MocConst().IncludePaths) {
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
    cm::append(cmd, this->MocConst().OptionsExtra);
    if (this->MocConst().CanOutputDependencies) {
      cmd.emplace_back("--output-dep-file");
    }
    // Add output file
    cmd.emplace_back("-o");
    cmd.push_back(outputFile);
    // Add source file
    cmd.push_back(sourceFile);

    MaybeWriteMocResponseFile(outputFile, cmd);
  }

  // Execute moc command
  cmWorkerPool::ProcessResultT result;
  if (!this->RunProcess(GenT::MOC, result, cmd, this->Reason.get())) {
    // Moc command failed
    std::string includers;
    if (!this->Mapping->IncluderFiles.empty()) {
      includers = "included by\n";
      for (auto const& item : this->Mapping->IncluderFiles) {
        includers += cmStrCat("  ", this->MessagePath(item->FileName), '\n');
      }
    }
    this->LogCommandError(GenT::MOC,
                          cmStrCat("The moc process failed to compile\n  ",
                                   this->MessagePath(sourceFile), "\ninto\n  ",
                                   this->MessagePath(outputFile), '\n',
                                   includers, result.ErrorMessage),
                          cmd, result.StdOut);
    return;
  }

  // Moc command success. Print moc output.
  if (!result.StdOut.empty()) {
    this->Log().Info(GenT::MOC, result.StdOut);
  }

  // Extract dependencies from the dep file moc generated for us
  if (this->MocConst().CanOutputDependencies) {
    const std::string depfile = outputFile + ".d";
    if (this->Log().Verbose()) {
      this->Log().Info(
        GenT::MOC, "Reading dependencies from " + this->MessagePath(depfile));
    }
    if (!cmSystemTools::FileExists(depfile)) {
      this->Log().Warning(GenT::MOC,
                          "Dependency file " + this->MessagePath(depfile) +
                            " does not exist.");
      return;
    }
    this->CacheEntry->Moc.Depends =
      this->Gen()->dependenciesFromDepFile(depfile.c_str());
  }
}

/*
 * Check if command line exceeds maximum length supported by OS
 * (if on Windows) and switch to using a response file instead.
 */
void cmQtAutoMocUicT::JobCompileMocT::MaybeWriteMocResponseFile(
  std::string const& outputFile, std::vector<std::string>& cmd) const
{
#ifdef _WIN32
  // Ensure cmd is less than CommandLineLengthMax characters
  size_t commandLineLength = cmd.size(); // account for separating spaces
  for (std::string const& str : cmd) {
    commandLineLength += str.length();
  }
  if (commandLineLength >= CommandLineLengthMax) {
    // Command line exceeds maximum size allowed by OS
    // => create response file
    std::string const responseFile = cmStrCat(outputFile, ".rsp");

    cmsys::ofstream fout(responseFile.c_str());
    if (!fout) {
      this->LogError(
        GenT::MOC,
        cmStrCat("AUTOMOC was unable to create a response file at\n  ",
                 this->MessagePath(responseFile)));
      return;
    }

    auto it = cmd.begin();
    while (++it != cmd.end()) {
      fout << *it << "\n";
    }
    fout.close();

    // Keep all but executable
    cmd.resize(1);

    // Specify response file
    cmd.push_back(cmStrCat('@', responseFile));
  }
#else
  static_cast<void>(outputFile);
  static_cast<void>(cmd);
#endif
}

void cmQtAutoMocUicT::JobCompileUicT::Process()
{
  std::string const& sourceFile = this->Mapping->SourceFile->FileName;
  std::string const& outputFile = this->Mapping->OutputFile;

  // Compose uic command
  std::vector<std::string> cmd;
  cmd.push_back(this->UicConst().Executable);
  {
    std::vector<std::string> allOpts = this->UicConst().Options;
    auto optionIt = this->UicConst().UiFiles.find(sourceFile);
    if (optionIt != this->UicConst().UiFiles.end()) {
      UicMergeOptions(allOpts, optionIt->second.Options,
                      (this->BaseConst().QtVersion.Major >= 5));
    }
    cm::append(cmd, allOpts);
  }
  cmd.emplace_back("-o");
  cmd.emplace_back(outputFile);
  cmd.emplace_back(sourceFile);

  cmWorkerPool::ProcessResultT result;
  if (this->RunProcess(GenT::UIC, result, cmd, this->Reason.get())) {
    // Uic command success
    // Print uic output
    if (!result.StdOut.empty()) {
      this->Log().Info(GenT::UIC, result.StdOut);
    }
  } else {
    // Uic command failed
    std::string includers;
    for (auto const& item : this->Mapping->IncluderFiles) {
      includers += cmStrCat("  ", this->MessagePath(item->FileName), '\n');
    }
    this->LogCommandError(GenT::UIC,
                          cmStrCat("The uic process failed to compile\n  ",
                                   this->MessagePath(sourceFile), "\ninto\n  ",
                                   this->MessagePath(outputFile),
                                   "\nincluded by\n", includers,
                                   result.ErrorMessage),
                          cmd, result.StdOut);
  }
}

void cmQtAutoMocUicT::JobMocsCompilationT::Process()
{
  std::string const& compAbs = this->MocConst().CompFileAbs;

  // Compose mocs compilation file content
  std::string content =
    "// This file is autogenerated. Changes will be overwritten.\n";

  if (this->MocEval().CompFiles.empty()) {
    // Placeholder content
    cmCryptoHash hash(cmCryptoHash::AlgoSHA256);
    const std::string hashedPath = hash.HashString(compAbs);
    const std::string functionName =
      "cmake_automoc_silence_linker_warning" + hashedPath;

    content += "// No files found that require moc or the moc files are "
               "included\n"
               "void " +
      functionName + "() {}\n";
  } else {
    // Valid content
    const bool mc = this->BaseConst().MultiConfig;
    cm::string_view const wrapFront = mc ? "#include <" : "#include \"";
    cm::string_view const wrapBack = mc ? ">\n" : "\"\n";
    content += cmWrap(wrapFront, this->MocEval().CompFiles, wrapBack, "");
  }

  if (cmQtAutoGenerator::FileDiffers(compAbs, content)) {
    // Actually write mocs compilation file
    if (this->Log().Verbose()) {
      this->Log().Info(
        GenT::MOC, "Generating MOC compilation " + this->MessagePath(compAbs));
    }
    if (!FileWrite(compAbs, content)) {
      this->LogError(GenT::MOC,
                     cmStrCat("Writing MOC compilation ",
                              this->MessagePath(compAbs), " failed."));
    }
  } else if (this->MocEval().CompUpdated) {
    // Only touch mocs compilation file
    if (this->Log().Verbose()) {
      this->Log().Info(
        GenT::MOC, "Touching MOC compilation " + this->MessagePath(compAbs));
    }
    if (!cmSystemTools::Touch(compAbs, false)) {
      this->LogError(GenT::MOC,
                     cmStrCat("Touching MOC compilation ",
                              this->MessagePath(compAbs), " failed."));
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

/*
 * Return the initial dependencies of the merged depfile.
 * Those are dependencies from the project files, not from moc runs.
 */
std::vector<std::string>
cmQtAutoMocUicT::JobDepFilesMergeT::initialDependencies() const
{
  std::vector<std::string> dependencies;
  dependencies.reserve(this->BaseConst().ListFiles.size() +
                       this->BaseEval().Headers.size() +
                       this->BaseEval().Sources.size());
  cm::append(dependencies, this->BaseConst().ListFiles);
  auto append_file_path =
    [&dependencies](const SourceFileMapT::value_type& p) {
      dependencies.push_back(p.first);
    };
  std::for_each(this->BaseEval().Headers.begin(),
                this->BaseEval().Headers.end(), append_file_path);
  std::for_each(this->BaseEval().Sources.begin(),
                this->BaseEval().Sources.end(), append_file_path);
  return dependencies;
}

void cmQtAutoMocUicT::JobDepFilesMergeT::Process()
{
  if (this->Log().Verbose()) {
    this->Log().Info(
      GenT::MOC,
      cmStrCat("Merging MOC dependencies into ",
               this->MessagePath(this->BaseConst().DepFile.c_str())));
  }
  auto processDepFile =
    [this](const std::string& mocOutputFile) -> std::vector<std::string> {
    std::string f = mocOutputFile + ".d";
    if (!cmSystemTools::FileExists(f)) {
      return {};
    }
    return this->Gen()->dependenciesFromDepFile(f.c_str());
  };

  std::vector<std::string> dependencies = this->initialDependencies();
  ParseCacheT& parseCache = this->BaseEval().ParseCache;
  auto processMappingEntry = [&](const MappingMapT::value_type& m) {
    auto cacheEntry = parseCache.GetOrInsert(m.first);
    if (cacheEntry.first->Moc.Depends.empty()) {
      cacheEntry.first->Moc.Depends = processDepFile(m.second->OutputFile);
    }
    dependencies.insert(dependencies.end(),
                        cacheEntry.first->Moc.Depends.begin(),
                        cacheEntry.first->Moc.Depends.end());
  };

  std::for_each(this->MocEval().HeaderMappings.begin(),
                this->MocEval().HeaderMappings.end(), processMappingEntry);
  std::for_each(this->MocEval().SourceMappings.begin(),
                this->MocEval().SourceMappings.end(), processMappingEntry);

  // Remove SKIP_AUTOMOC files.
  // Also remove AUTOUIC header files to avoid cyclic dependency.
  dependencies.erase(
    std::remove_if(dependencies.begin(), dependencies.end(),
                   [this](const std::string& dep) {
                     return this->MocConst().skipped(dep) ||
                       std::any_of(
                              this->UicEval().Includes.begin(),
                              this->UicEval().Includes.end(),
                              [&dep](MappingMapT::value_type const& mapping) {
                                return dep == mapping.second->OutputFile;
                              });
                   }),
    dependencies.end());

  // Remove duplicates to make the depfile smaller
  std::sort(dependencies.begin(), dependencies.end());
  dependencies.erase(std::unique(dependencies.begin(), dependencies.end()),
                     dependencies.end());

  // Add form files
  for (const auto& uif : this->UicEval().UiFiles) {
    dependencies.push_back(uif.first);
  }

  // Write the file
  cmsys::ofstream ofs;
  ofs.open(this->BaseConst().DepFile.c_str(),
           (std::ios::out | std::ios::binary | std::ios::trunc));
  if (!ofs) {
    this->LogError(GenT::GEN,
                   cmStrCat("Cannot open ",
                            this->MessagePath(this->BaseConst().DepFile),
                            " for writing."));
    return;
  }
  ofs << this->BaseConst().DepFileRuleName << ": \\\n";
  for (const std::string& file : dependencies) {
    ofs << '\t' << escapeDependencyPath(file) << " \\\n";
    if (!ofs.good()) {
      this->LogError(GenT::GEN,
                     cmStrCat("Writing depfile",
                              this->MessagePath(this->BaseConst().DepFile),
                              " failed."));
      return;
    }
  }

  // Add the CMake executable to re-new cache data if necessary.
  // Also, this is the last entry, so don't add a backslash.
  ofs << '\t' << escapeDependencyPath(this->BaseConst().CMakeExecutable)
      << '\n';
}

void cmQtAutoMocUicT::JobFinishT::Process()
{
  this->Gen()->AbortSuccess();
}

cmQtAutoMocUicT::cmQtAutoMocUicT()
  : cmQtAutoGenerator(GenT::GEN)
{
}
cmQtAutoMocUicT::~cmQtAutoMocUicT() = default;

bool cmQtAutoMocUicT::InitFromInfo(InfoT const& info)
{
  // -- Required settings
  if (!info.GetBool("MULTI_CONFIG", this->BaseConst_.MultiConfig, true) ||
      !info.GetUInt("QT_VERSION_MAJOR", this->BaseConst_.QtVersion.Major,
                    true) ||
      !info.GetUInt("QT_VERSION_MINOR", this->BaseConst_.QtVersion.Minor,
                    true) ||
      !info.GetUInt("PARALLEL", this->BaseConst_.ThreadCount, false) ||
      !info.GetString("BUILD_DIR", this->BaseConst_.AutogenBuildDir, true) ||
      !info.GetStringConfig("INCLUDE_DIR", this->BaseConst_.AutogenIncludeDir,
                            true) ||
      !info.GetString("CMAKE_EXECUTABLE", this->BaseConst_.CMakeExecutable,
                      true) ||
      !info.GetStringConfig("PARSE_CACHE_FILE",
                            this->BaseConst_.ParseCacheFile, true) ||
      !info.GetString("DEP_FILE", this->BaseConst_.DepFile, false) ||
      !info.GetString("DEP_FILE_RULE_NAME", this->BaseConst_.DepFileRuleName,
                      false) ||
      !info.GetStringConfig("SETTINGS_FILE", this->SettingsFile_, true) ||
      !info.GetArray("CMAKE_LIST_FILES", this->BaseConst_.ListFiles, true) ||
      !info.GetArray("HEADER_EXTENSIONS", this->BaseConst_.HeaderExtensions,
                     true) ||
      !info.GetString("QT_MOC_EXECUTABLE", this->MocConst_.Executable,
                      false) ||
      !info.GetString("QT_UIC_EXECUTABLE", this->UicConst_.Executable,
                      false)) {
    return false;
  }

  // -- Checks
  if (!this->BaseConst_.CMakeExecutableTime.Load(
        this->BaseConst_.CMakeExecutable)) {
    return info.LogError(
      cmStrCat("The CMake executable ",
               this->MessagePath(this->BaseConst_.CMakeExecutable),
               " does not exist."));
  }

  // -- Evaluate values
  this->BaseConst_.ThreadCount =
    std::min(this->BaseConst_.ThreadCount, ParallelMax);
  this->WorkerPool_.SetThreadCount(this->BaseConst_.ThreadCount);

  // -- Moc
  if (!this->MocConst_.Executable.empty()) {
    // -- Moc is enabled
    this->MocConst_.Enabled = true;

    // -- Temporary buffers
    struct
    {
      std::vector<std::string> MacroNames;
      std::vector<std::string> DependFilters;
    } tmp;

    // -- Required settings
    if (!info.GetBool("MOC_RELAXED_MODE", this->MocConst_.RelaxedMode,
                      false) ||
        !info.GetBool("MOC_PATH_PREFIX", this->MocConst_.PathPrefix, true) ||
        !info.GetArray("MOC_SKIP", this->MocConst_.SkipList, false) ||
        !info.GetArrayConfig("MOC_DEFINITIONS", this->MocConst_.Definitions,
                             false) ||
        !info.GetArrayConfig("MOC_INCLUDES", this->MocConst_.IncludePaths,
                             false) ||
        !info.GetArray("MOC_OPTIONS", this->MocConst_.OptionsExtra, false) ||
        !info.GetStringConfig("MOC_COMPILATION_FILE",
                              this->MocConst_.CompFileAbs, true) ||
        !info.GetArray("MOC_PREDEFS_CMD", this->MocConst_.PredefsCmd, false) ||
        !info.GetStringConfig("MOC_PREDEFS_FILE",
                              this->MocConst_.PredefsFileAbs,
                              !this->MocConst_.PredefsCmd.empty()) ||
        !info.GetArray("MOC_MACRO_NAMES", tmp.MacroNames, true) ||
        !info.GetArray("MOC_DEPEND_FILTERS", tmp.DependFilters, false)) {
      return false;
    }

    // -- Evaluate settings
    for (std::string const& item : tmp.MacroNames) {
      this->MocConst_.MacroFilters.emplace_back(
        item, ("[\n][ \t]*{?[ \t]*" + item).append("[^a-zA-Z0-9_]"));
    }
    // Can moc output dependencies or do we need to setup dependency filters?
    if (this->BaseConst_.QtVersion >= IntegerVersion(5, 15)) {
      this->MocConst_.CanOutputDependencies = true;
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
    if (!this->MocConst_.ExecutableTime.Load(this->MocConst_.Executable)) {
      return info.LogError(cmStrCat(
        "The moc executable ", this->MessagePath(this->MocConst_.Executable),
        " does not exist."));
    }
  }

  // -- Uic
  if (!this->UicConst_.Executable.empty()) {
    // Uic is enabled
    this->UicConst_.Enabled = true;

    // -- Required settings
    if (!info.GetArray("UIC_SKIP", this->UicConst_.SkipList, false) ||
        !info.GetArray("UIC_SEARCH_PATHS", this->UicConst_.SearchPaths,
                       false) ||
        !info.GetArrayConfig("UIC_OPTIONS", this->UicConst_.Options, false)) {
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

        auto& uiFile = this->UicConst_.UiFiles[entryName.asString()];
        InfoT::GetJsonArray(uiFile.Options, entryOptions);
      }
    }

    // -- Evaluate settings
    // Check if uic executable exists (by reading the file time)
    if (!this->UicConst_.ExecutableTime.Load(this->UicConst_.Executable)) {
      return info.LogError(cmStrCat(
        "The uic executable ", this->MessagePath(this->UicConst_.Executable),
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
          testEntry(entry.size() == 4, "JSON array size invalid.")) {
        return false;
      }

      Json::Value const& entryName = entry[0u];
      Json::Value const& entryFlags = entry[1u];
      Json::Value const& entryBuild = entry[2u];
      Json::Value const& entryConfigs = entry[3u];
      if (testEntry(entryName.isString(),
                    "JSON value for name is not a string.") ||
          testEntry(entryFlags.isString(),
                    "JSON value for flags is not a string.") ||
          testEntry(entryConfigs.isNull() || entryConfigs.isArray(),
                    "JSON value for configs is not null or array.") ||
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

      if (entryConfigs.isArray()) {
        bool configFound = false;
        Json::ArrayIndex const configArraySize = entryConfigs.size();
        for (Json::ArrayIndex ci = 0; ci != configArraySize; ++ci) {
          Json::Value const& config = entryConfigs[ci];
          if (testEntry(config.isString(),
                        "JSON value in config array is not a string.")) {
            return false;
          }
          configFound = configFound || config.asString() == this->InfoConfig();
        }
        if (!configFound) {
          continue;
        }
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
      if (sourceHandle->Moc && this->MocConst().Enabled) {
        if (build.empty()) {
          return info.LogError(
            cmStrCat("Header file ", ii, " build path is empty"));
        }
        sourceHandle->BuildPath = std::move(build);
      }
      this->BaseEval().Headers.emplace(std::move(name),
                                       std::move(sourceHandle));
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
          testEntry(entry.size() == 3, "JSON array size invalid.")) {
        return false;
      }

      Json::Value const& entryName = entry[0u];
      Json::Value const& entryFlags = entry[1u];
      Json::Value const& entryConfigs = entry[2u];
      if (testEntry(entryName.isString(),
                    "JSON value for name is not a string.") ||
          testEntry(entryFlags.isString(),
                    "JSON value for flags is not a string.") ||
          testEntry(entryConfigs.isNull() || entryConfigs.isArray(),
                    "JSON value for configs is not null or array.")) {
        return false;
      }

      std::string name = entryName.asString();
      std::string flags = entryFlags.asString();
      if (testEntry(flags.size() == 2, "Invalid flags string size")) {
        return false;
      }

      if (entryConfigs.isArray()) {
        bool configFound = false;
        Json::ArrayIndex const configArraySize = entryConfigs.size();
        for (Json::ArrayIndex ci = 0; ci != configArraySize; ++ci) {
          Json::Value const& config = entryConfigs[ci];
          if (testEntry(config.isString(),
                        "JSON value in config array is not a string.")) {
            return false;
          }
          configFound = configFound || config.asString() == this->InfoConfig();
        }
        if (!configFound) {
          continue;
        }
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
      this->BaseEval().Sources.emplace(std::move(name),
                                       std::move(sourceHandle));
    }
  }

  // -- Init derived information
  // Moc variables
  if (this->MocConst().Enabled) {
    // Compose moc includes list
    {
      // Compute framework paths
      std::set<std::string> frameworkPaths;
      for (std::string const& path : this->MocConst().IncludePaths) {
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
      this->MocConst_.OptionsIncludes.reserve(
        this->MocConst().IncludePaths.size() + frameworkPaths.size() * 2);
      // Append includes
      for (std::string const& path : this->MocConst().IncludePaths) {
        this->MocConst_.OptionsIncludes.emplace_back("-I" + path);
      }
      // Append framework includes
      for (std::string const& path : frameworkPaths) {
        this->MocConst_.OptionsIncludes.emplace_back("-F");
        this->MocConst_.OptionsIncludes.push_back(path);
      }
    }

    // Compose moc definitions list
    {
      this->MocConst_.OptionsDefinitions.reserve(
        this->MocConst().Definitions.size());
      for (std::string const& def : this->MocConst().Definitions) {
        this->MocConst_.OptionsDefinitions.emplace_back("-D" + def);
      }
    }
  }

  return true;
}

template <class JOBTYPE>
void cmQtAutoMocUicT::CreateParseJobs(SourceFileMapT const& sourceMap)
{
  cmFileTime const parseCacheTime = this->BaseEval().ParseCacheTime;
  ParseCacheT& parseCache = this->BaseEval().ParseCache;
  for (const auto& src : sourceMap) {
    // Get or create the file parse data reference
    ParseCacheT::GetOrInsertT cacheEntry = parseCache.GetOrInsert(src.first);
    src.second->ParseData = std::move(cacheEntry.first);
    // Create a parse job if the cache file was missing or is older
    if (cacheEntry.second || src.second->FileTime.Newer(parseCacheTime)) {
      this->BaseEval().ParseCacheChanged = true;
      this->WorkerPool().EmplaceJob<JOBTYPE>(src.second);
    }
  }
}

/** Concurrently callable implementation of cmSystemTools::CollapseFullPath */
std::string cmQtAutoMocUicT::CollapseFullPathTS(std::string const& path) const
{
  std::lock_guard<std::mutex> guard(this->CMakeLibMutex_);
#if defined(__NVCOMPILER) || defined(__LCC__)
  static_cast<void>(guard); // convince compiler var is used
#endif
  return cmSystemTools::CollapseFullPath(path,
                                         this->ProjectDirs().CurrentSource);
}

void cmQtAutoMocUicT::InitJobs()
{
  // Add moc_predefs.h job
  if (this->MocConst().Enabled && !this->MocConst().PredefsCmd.empty()) {
    this->WorkerPool().EmplaceJob<JobMocPredefsT>();
  }

  // Add header parse jobs
  this->CreateParseJobs<JobParseHeaderT>(this->BaseEval().Headers);
  // Add source parse jobs
  this->CreateParseJobs<JobParseSourceT>(this->BaseEval().Sources);

  // Add parse cache evaluations jobs
  {
    // Add a fence job to ensure all parsing has finished
    this->WorkerPool().EmplaceJob<JobFenceT>();
    if (this->MocConst().Enabled) {
      this->WorkerPool().EmplaceJob<JobEvalCacheMocT>();
    }
    if (this->UicConst().Enabled) {
      this->WorkerPool().EmplaceJob<JobEvalCacheUicT>();
    }
    // Add evaluate job
    this->WorkerPool().EmplaceJob<JobEvalCacheFinishT>();
  }
}

bool cmQtAutoMocUicT::Process()
{
  this->SettingsFileRead();
  this->ParseCacheRead();
  if (!this->CreateDirectories()) {
    return false;
  }
  this->InitJobs();
  if (!this->WorkerPool_.Process(this)) {
    return false;
  }
  if (this->JobError_) {
    return false;
  }
  if (!this->ParseCacheWrite()) {
    return false;
  }
  if (!this->SettingsFileWrite()) {
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

    if (this->MocConst_.Enabled) {
      cryptoHash.Initialize();
      cha(this->MocConst().Executable);
      for (auto const& item : this->MocConst().OptionsDefinitions) {
        cha(item);
      }
      for (auto const& item : this->MocConst().OptionsIncludes) {
        cha(item);
      }
      for (auto const& item : this->MocConst().OptionsExtra) {
        cha(item);
      }
      for (auto const& item : this->MocConst().PredefsCmd) {
        cha(item);
      }
      for (auto const& filter : this->MocConst().DependFilters) {
        cha(filter.Key);
      }
      for (auto const& filter : this->MocConst().MacroFilters) {
        cha(filter.Key);
      }
      this->SettingsStringMoc_ = cryptoHash.FinalizeHex();
    }

    if (this->UicConst().Enabled) {
      cryptoHash.Initialize();
      cha(this->UicConst().Executable);
      std::for_each(this->UicConst().Options.begin(),
                    this->UicConst().Options.end(), cha);
      for (const auto& item : this->UicConst().UiFiles) {
        cha(item.first);
        auto const& opts = item.second.Options;
        std::for_each(opts.begin(), opts.end(), cha);
      }
      this->SettingsStringUic_ = cryptoHash.FinalizeHex();
    }
  }

  // Read old settings and compare
  {
    std::string content;
    if (cmQtAutoGenerator::FileRead(content, this->SettingsFile_)) {
      if (this->MocConst().Enabled) {
        if (this->SettingsStringMoc_ != SettingsFind(content, "moc")) {
          this->MocConst_.SettingsChanged = true;
        }
      }
      if (this->UicConst().Enabled) {
        if (this->SettingsStringUic_ != SettingsFind(content, "uic")) {
          this->UicConst_.SettingsChanged = true;
        }
      }
      // In case any setting changed remove the old settings file.
      // This triggers a full rebuild on the next run if the current
      // build is aborted before writing the current settings in the end.
      if (this->MocConst().SettingsChanged ||
          this->UicConst().SettingsChanged) {
        cmSystemTools::RemoveFile(this->SettingsFile_);
      }
    } else {
      // Settings file read failed
      if (this->MocConst().Enabled) {
        this->MocConst_.SettingsChanged = true;
      }
      if (this->UicConst().Enabled) {
        this->UicConst_.SettingsChanged = true;
      }
    }
  }
}

bool cmQtAutoMocUicT::SettingsFileWrite()
{
  // Only write if any setting changed
  if (this->MocConst().SettingsChanged || this->UicConst().SettingsChanged) {
    if (this->Log().Verbose()) {
      this->Log().Info(GenT::GEN,
                       cmStrCat("Writing the settings file ",
                                this->MessagePath(this->SettingsFile_)));
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
      SettingAppend("moc", this->SettingsStringMoc_);
      SettingAppend("uic", this->SettingsStringUic_);
    }
    // Write settings file
    std::string error;
    if (!cmQtAutoGenerator::FileWrite(this->SettingsFile_, content, &error)) {
      this->Log().Error(GenT::GEN,
                        cmStrCat("Writing the settings file ",
                                 this->MessagePath(this->SettingsFile_),
                                 " failed.\n", error));
      // Remove old settings file to trigger a full rebuild on the next run
      cmSystemTools::RemoveFile(this->SettingsFile_);
      return false;
    }
  }
  return true;
}

void cmQtAutoMocUicT::ParseCacheRead()
{
  cm::string_view reason;
  // Don't read the cache if it is invalid
  if (!this->BaseEval().ParseCacheTime.Load(
        this->BaseConst().ParseCacheFile)) {
    reason = "Refreshing parse cache because it doesn't exist.";
  } else if (this->MocConst().SettingsChanged ||
             this->UicConst().SettingsChanged) {
    reason = "Refreshing parse cache because the settings changed.";
  } else if (this->BaseEval().ParseCacheTime.Older(
               this->BaseConst().CMakeExecutableTime)) {
    reason =
      "Refreshing parse cache because it is older than the CMake executable.";
  }

  if (!reason.empty()) {
    // Don't read but refresh the complete parse cache
    if (this->Log().Verbose()) {
      this->Log().Info(GenT::GEN, reason);
    }
    this->BaseEval().ParseCacheChanged = true;
  } else {
    // Read parse cache
    this->BaseEval().ParseCache.ReadFromFile(this->BaseConst().ParseCacheFile);
  }
}

bool cmQtAutoMocUicT::ParseCacheWrite()
{
  if (this->BaseEval().ParseCacheChanged) {
    if (this->Log().Verbose()) {
      this->Log().Info(
        GenT::GEN,
        cmStrCat("Writing the parse cache file ",
                 this->MessagePath(this->BaseConst().ParseCacheFile)));
    }
    if (!this->BaseEval().ParseCache.WriteToFile(
          this->BaseConst().ParseCacheFile)) {
      this->Log().Error(
        GenT::GEN,
        cmStrCat("Writing the parse cache file ",
                 this->MessagePath(this->BaseConst().ParseCacheFile),
                 " failed."));
      return false;
    }
  }
  return true;
}

bool cmQtAutoMocUicT::CreateDirectories()
{
  // Create AUTOGEN include directory
  if (!cmSystemTools::MakeDirectory(this->BaseConst().AutogenIncludeDir)) {
    this->Log().Error(
      GenT::GEN,
      cmStrCat("Creating the AUTOGEN include directory ",
               this->MessagePath(this->BaseConst().AutogenIncludeDir),
               " failed."));
    return false;
  }
  return true;
}

std::vector<std::string> cmQtAutoMocUicT::dependenciesFromDepFile(
  const char* filePath)
{
  std::lock_guard<std::mutex> guard(this->CMakeLibMutex_);
#if defined(__NVCOMPILER) || defined(__LCC__)
  static_cast<void>(guard); // convince compiler var is used
#endif
  auto const content = cmReadGccDepfile(filePath);
  if (!content || content->empty()) {
    return {};
  }

  // Moc outputs a depfile with exactly one rule.
  // Discard the rule and return the dependencies.
  return content->front().paths;
}

void cmQtAutoMocUicT::Abort(bool error)
{
  if (error) {
    this->JobError_.store(true);
  }
  this->WorkerPool_.Abort();
}

std::string cmQtAutoMocUicT::AbsoluteBuildPath(
  cm::string_view relativePath) const
{
  return cmStrCat(this->BaseConst().AutogenBuildDir, '/', relativePath);
}

std::string cmQtAutoMocUicT::AbsoluteIncludePath(
  cm::string_view relativePath) const
{
  return cmStrCat(this->BaseConst().AutogenIncludeDir, '/', relativePath);
}

} // End of unnamed namespace

bool cmQtAutoMocUic(cm::string_view infoFile, cm::string_view config)
{
  return cmQtAutoMocUicT().Run(infoFile, config);
}
