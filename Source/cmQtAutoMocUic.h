/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmQtAutoMocUic_h
#define cmQtAutoMocUic_h

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmQtAutoGen.h"
#include "cmQtAutoGenerator.h"
#include "cmWorkerPool.h"
#include "cmsys/RegularExpression.hxx"

#include <array>
#include <atomic>
#include <map>
#include <memory> // IWYU pragma: keep
#include <mutex>
#include <set>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

class cmMakefile;

// @brief AUTOMOC and AUTOUIC generator
class cmQtAutoMocUic : public cmQtAutoGenerator
{
public:
  cmQtAutoMocUic();
  ~cmQtAutoMocUic() override;

  cmQtAutoMocUic(cmQtAutoMocUic const&) = delete;
  cmQtAutoMocUic& operator=(cmQtAutoMocUic const&) = delete;

public:
  // -- Types
  typedef std::multimap<std::string, std::array<std::string, 2>> IncludesMap;

  /// @brief Search key plus regular expression pair
  ///
  struct KeyExpT
  {
    KeyExpT() = default;

    KeyExpT(const char* key, const char* exp)
      : Key(key)
      , Exp(exp)
    {
    }

    KeyExpT(std::string key, std::string const& exp)
      : Key(std::move(key))
      , Exp(exp)
    {
    }

    std::string Key;
    cmsys::RegularExpression Exp;
  };

  /// @brief Common settings
  ///
  class BaseSettingsT
  {
  public:
    // -- Volatile methods
    BaseSettingsT(FileSystem* fileSystem)
      : MultiConfig(false)
      , IncludeProjectDirsBefore(false)
      , QtVersionMajor(4)
      , NumThreads(1)
      , FileSys(fileSystem)
    {
    }

    BaseSettingsT(BaseSettingsT const&) = delete;
    BaseSettingsT& operator=(BaseSettingsT const&) = delete;

    // -- Const methods
    std::string AbsoluteBuildPath(std::string const& relativePath) const;
    bool FindHeader(std::string& header,
                    std::string const& testBasePath) const;

    // -- Attributes
    // - Config
    bool MultiConfig;
    bool IncludeProjectDirsBefore;
    unsigned int QtVersionMajor;
    unsigned int NumThreads;
    // - Directories
    std::string ProjectSourceDir;
    std::string ProjectBinaryDir;
    std::string CurrentSourceDir;
    std::string CurrentBinaryDir;
    std::string AutogenBuildDir;
    std::string AutogenIncludeDir;
    // - Files
    std::vector<std::string> HeaderExtensions;
    // - File system
    FileSystem* FileSys;
  };

  /// @brief Moc settings
  ///
  class MocSettingsT
  {
  public:
    MocSettingsT(FileSystem* fileSys)
      : FileSys(fileSys)
    {
    }

    MocSettingsT(MocSettingsT const&) = delete;
    MocSettingsT& operator=(MocSettingsT const&) = delete;

    // -- Const methods
    bool skipped(std::string const& fileName) const;
    std::string FindMacro(std::string const& content) const;
    std::string MacrosString() const;
    std::string FindIncludedFile(std::string const& sourcePath,
                                 std::string const& includeString) const;
    void FindDependencies(std::string const& content,
                          std::set<std::string>& depends) const;

    // -- Attributes
    bool Enabled = false;
    bool SettingsChanged = false;
    bool RelaxedMode = false;
    std::string Executable;
    std::string CompFileAbs;
    std::string PredefsFileRel;
    std::string PredefsFileAbs;
    std::unordered_set<std::string> SkipList;
    std::vector<std::string> IncludePaths;
    std::vector<std::string> Includes;
    std::vector<std::string> Definitions;
    std::vector<std::string> Options;
    std::vector<std::string> AllOptions;
    std::vector<std::string> PredefsCmd;
    std::vector<KeyExpT> DependFilters;
    std::vector<KeyExpT> MacroFilters;
    cmsys::RegularExpression RegExpInclude;
    // - File system
    FileSystem* FileSys;
  };

  /// @brief Uic settings
  ///
  class UicSettingsT
  {
  public:
    UicSettingsT() = default;

    UicSettingsT(UicSettingsT const&) = delete;
    UicSettingsT& operator=(UicSettingsT const&) = delete;

    // -- Const methods
    bool skipped(std::string const& fileName) const;

    // -- Attributes
    bool Enabled = false;
    bool SettingsChanged = false;
    std::string Executable;
    std::unordered_set<std::string> SkipList;
    std::vector<std::string> TargetOptions;
    std::map<std::string, std::vector<std::string>> Options;
    std::vector<std::string> SearchPaths;
    cmsys::RegularExpression RegExpInclude;
  };

  /// @brief Abstract job class for concurrent job processing
  ///
  class JobT : public cmWorkerPool::JobT
  {
  protected:
    /**
     * @brief Protected default constructor
     */
    JobT(bool fence = false)
      : cmWorkerPool::JobT(fence)
    {
    }

    //! Get the generator. Only valid during Process() call!
    cmQtAutoMocUic* Gen() const
    {
      return static_cast<cmQtAutoMocUic*>(UserData());
    };

    //! Get the file system interface. Only valid during Process() call!
    FileSystem& FileSys() { return Gen()->FileSys(); }
    //! Get the logger. Only valid during Process() call!
    Logger& Log() { return Gen()->Log(); }

    // -- Error logging with automatic abort
    void LogError(GenT genType, std::string const& message) const;
    void LogFileError(GenT genType, std::string const& filename,
                      std::string const& message) const;
    void LogCommandError(GenT genType, std::string const& message,
                         std::vector<std::string> const& command,
                         std::string const& output) const;

    /**
     * @brief Run an external process. Use only during Process() call!
     */
    bool RunProcess(GenT genType, cmWorkerPool::ProcessResultT& result,
                    std::vector<std::string> const& command);
  };

  /// @brief Fence job utility class
  ///
  class JobFenceT : public JobT
  {
  public:
    JobFenceT()
      : JobT(true)
    {
    }
    void Process() override{};
  };

  /// @brief Generate moc_predefs.h
  ///
  class JobMocPredefsT : public JobT
  {
  private:
    void Process() override;
  };

  /// @brief Parses a source file
  ///
  class JobParseT : public JobT
  {
  public:
    JobParseT(std::string fileName, bool moc, bool uic, bool header = false)
      : FileName(std::move(fileName))
      , AutoMoc(moc)
      , AutoUic(uic)
      , Header(header)
    {
    }

  private:
    struct MetaT
    {
      std::string Content;
      std::string FileDir;
      std::string FileBase;
    };

    void Process() override;
    bool ParseMocSource(MetaT const& meta);
    bool ParseMocHeader(MetaT const& meta);
    std::string MocStringHeaders(std::string const& fileBase) const;
    std::string MocFindIncludedHeader(std::string const& includerDir,
                                      std::string const& includeBase);
    bool ParseUic(MetaT const& meta);
    bool ParseUicInclude(MetaT const& meta, std::string&& includeString);
    std::string UicFindIncludedFile(MetaT const& meta,
                                    std::string const& includeString);

  private:
    std::string FileName;
    bool AutoMoc = false;
    bool AutoUic = false;
    bool Header = false;
  };

  /// @brief Generates additional jobs after all files have been parsed
  ///
  class JobPostParseT : public JobFenceT
  {
  private:
    void Process() override;
  };

  /// @brief Generate mocs_compilation.cpp
  ///
  class JobMocsCompilationT : public JobFenceT
  {
  private:
    void Process() override;
  };

  /// @brief Moc a file job
  ///
  class JobMocT : public JobT
  {
  public:
    JobMocT(std::string sourceFile, std::string includerFile,
            std::string includeString)
      : SourceFile(std::move(sourceFile))
      , IncluderFile(std::move(includerFile))
      , IncludeString(std::move(includeString))
    {
    }

    void FindDependencies(std::string const& content);

  private:
    void Process() override;
    bool UpdateRequired();
    void GenerateMoc();

  public:
    std::string SourceFile;
    std::string IncluderFile;
    std::string IncludeString;
    std::string BuildFile;
    bool DependsValid = false;
    std::set<std::string> Depends;
  };

  /// @brief Uic a file job
  ///
  class JobUicT : public JobT
  {
  public:
    JobUicT(std::string sourceFile, std::string includerFile,
            std::string includeString)
      : SourceFile(std::move(sourceFile))
      , IncluderFile(std::move(includerFile))
      , IncludeString(std::move(includeString))
    {
    }

  private:
    void Process() override;
    bool UpdateRequired();
    void GenerateUic();

  public:
    std::string SourceFile;
    std::string IncluderFile;
    std::string IncludeString;
    std::string BuildFile;
  };

  /// @brief The last job
  ///
  class JobFinishT : public JobFenceT
  {
  private:
    void Process() override;
  };

  // -- Const settings interface
  const BaseSettingsT& Base() const { return this->Base_; }
  const MocSettingsT& Moc() const { return this->Moc_; }
  const UicSettingsT& Uic() const { return this->Uic_; }

  // -- Parallel job processing interface
  cmWorkerPool& WorkerPool() { return WorkerPool_; }
  void AbortError() { Abort(true); }
  void AbortSuccess() { Abort(false); }
  bool ParallelJobPushMoc(cmWorkerPool::JobHandleT&& jobHandle);
  bool ParallelJobPushUic(cmWorkerPool::JobHandleT&& jobHandle);

  // -- Mocs compilation include file updated flag
  void ParallelMocAutoUpdated() { MocAutoFileUpdated_.store(true); }
  bool MocAutoFileUpdated() const { return MocAutoFileUpdated_.load(); }

  // -- Mocs compilation file register
  std::string ParallelMocAutoRegister(std::string const& baseName);
  bool ParallelMocIncluded(std::string const& sourceFile);
  std::set<std::string> const& MocAutoFiles() const
  {
    return this->MocAutoFiles_;
  }

private:
  // -- Utility accessors
  Logger& Log() { return Logger_; }
  FileSystem& FileSys() { return FileSys_; }
  // -- Abstract processing interface
  bool Init(cmMakefile* makefile) override;
  bool Process() override;
  // -- Settings file
  void SettingsFileRead();
  bool SettingsFileWrite();
  // -- Thread processing
  void Abort(bool error);
  // -- Generation
  bool CreateDirectories();

private:
  // -- Utility
  Logger Logger_;
  FileSystem FileSys_;
  // -- Settings
  BaseSettingsT Base_;
  MocSettingsT Moc_;
  UicSettingsT Uic_;
  // -- Moc meta
  std::mutex MocMetaMutex_;
  std::set<std::string> MocIncludedFiles_;
  IncludesMap MocIncludes_;
  std::set<std::string> MocAutoFiles_;
  std::atomic<bool> MocAutoFileUpdated_ = ATOMIC_VAR_INIT(false);
  // -- Uic meta
  std::mutex UicMetaMutex_;
  IncludesMap UicIncludes_;
  // -- Settings file
  std::string SettingsFile_;
  std::string SettingsStringMoc_;
  std::string SettingsStringUic_;
  // -- Thread pool and job queue
  std::atomic<bool> JobError_ = ATOMIC_VAR_INIT(false);
  cmWorkerPool WorkerPool_;
};

#endif
