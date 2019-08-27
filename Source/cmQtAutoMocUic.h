/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmQtAutoMocUic_h
#define cmQtAutoMocUic_h

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmFileTime.h"
#include "cmQtAutoGen.h"
#include "cmQtAutoGenerator.h"
#include "cmWorkerPool.h"
#include "cmsys/RegularExpression.hxx"

#include <atomic>
#include <cstddef>
#include <map>
#include <memory> // IWYU pragma: keep
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

class cmMakefile;

/** \class cmQtAutoMocUic
 * \brief AUTOMOC and AUTOUIC generator
 */
class cmQtAutoMocUic : public cmQtAutoGenerator
{
public:
  cmQtAutoMocUic();
  ~cmQtAutoMocUic() override;

  cmQtAutoMocUic(cmQtAutoMocUic const&) = delete;
  cmQtAutoMocUic& operator=(cmQtAutoMocUic const&) = delete;

public:
  // -- Types

  /**
   * Search key plus regular expression pair
   */
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

  /**
   * Include string with sub parts
   */
  struct IncludeKeyT
  {
    IncludeKeyT(std::string const& key, std::size_t basePrefixLength);

    std::string Key;  // Full include string
    std::string Dir;  // Include directory
    std::string Base; // Base part of the include file name
  };

  /**
   * Source file parsing cache
   */
  class ParseCacheT
  {
  public:
    // -- Types
    /**
     * Entry of the file parsing cache
     */
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
    typedef std::shared_ptr<FileT> FileHandleT;
    typedef std::pair<FileHandleT, bool> GetOrInsertT;

  public:
    ParseCacheT();
    ~ParseCacheT();

    void Clear();

    bool ReadFromFile(std::string const& fileName);
    bool WriteToFile(std::string const& fileName);

    //! Might return an invalid handle
    FileHandleT Get(std::string const& fileName) const;
    //! Always returns a valid handle
    GetOrInsertT GetOrInsert(std::string const& fileName);

  private:
    std::unordered_map<std::string, FileHandleT> Map_;
  };

  /**
   * Source file data
   */
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
    bool Moc = false;
    bool Uic = false;
  };
  typedef std::shared_ptr<SourceFileT> SourceFileHandleT;
  typedef std::map<std::string, SourceFileHandleT> SourceFileMapT;

  /**
   * Meta compiler file mapping information
   */
  struct MappingT
  {
    SourceFileHandleT SourceFile;
    std::string OutputFile;
    std::string IncludeString;
    std::vector<SourceFileHandleT> IncluderFiles;
  };
  typedef std::shared_ptr<MappingT> MappingHandleT;
  typedef std::map<std::string, MappingHandleT> MappingMapT;

  /**
   * Common settings
   */
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
    bool IncludeProjectDirsBefore = false;
    unsigned int QtVersionMajor = 4;
    // - Directories
    std::string ProjectSourceDir;
    std::string ProjectBinaryDir;
    std::string CurrentSourceDir;
    std::string CurrentBinaryDir;
    std::string AutogenBuildDir;
    std::string AutogenIncludeDir;
    // - Files
    std::string CMakeExecutable;
    cmFileTime CMakeExecutableTime;
    std::string ParseCacheFile;
    std::vector<std::string> HeaderExtensions;
  };

  /**
   * Shared common variables
   */
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

  /**
   * Moc settings
   */
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
    cmFileTime ExecutableTime;
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
  };

  /**
   * Moc shared variables
   */
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
    // -- Mocs compilation
    bool CompUpdated = false;
    std::vector<std::string> CompFiles;
  };

  /**
   * Uic settings
   */
  class UicSettingsT
  {
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
    std::vector<std::string> TargetOptions;
    std::map<std::string, std::vector<std::string>> Options;
    std::vector<std::string> SearchPaths;
    cmsys::RegularExpression RegExpInclude;
  };

  /**
   * Uic shared variables
   */
  class UicEvalT
  {
  public:
    SourceFileMapT UiFiles;
    MappingMapT Includes;
  };

  /**
   * Abstract job class for concurrent job processing
   */
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

    // -- Accessors. Only valid during Process() call!
    Logger const& Log() const { return Gen()->Log(); }
    BaseSettingsT const& BaseConst() const { return Gen()->BaseConst(); }
    BaseEvalT& BaseEval() const { return Gen()->BaseEval(); }
    MocSettingsT const& MocConst() const { return Gen()->MocConst(); }
    MocEvalT& MocEval() const { return Gen()->MocEval(); }
    UicSettingsT const& UicConst() const { return Gen()->UicConst(); }
    UicEvalT& UicEval() const { return Gen()->UicEval(); }

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
                    std::vector<std::string> const& command,
                    std::string* infoMessage = nullptr);
  };

  /**
   * Fence job utility class
   */
  class JobFenceT : public JobT
  {
  public:
    JobFenceT()
      : JobT(true)
    {
    }
    void Process() override{};
  };

  /**
   * Generate moc_predefs.h
   */
  class JobMocPredefsT : public JobFenceT
  {
    void Process() override;
    bool Update(std::string* reason) const;
  };

  /**
   * File parse job base class
   */
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

  /**
   * Header file parse job
   */
  class JobParseHeaderT : public JobParseT
  {
  public:
    using JobParseT::JobParseT;
    void Process() override;
  };

  /**
   * Source file parse job
   */
  class JobParseSourceT : public JobParseT
  {
  public:
    using JobParseT::JobParseT;
    void Process() override;
  };

  /**
   * Evaluate parsed files
   */
  class JobEvaluateT : public JobFenceT
  {
    void Process() override;

    // -- Moc
    bool MocEvalHeader(SourceFileHandleT source);
    bool MocEvalSource(SourceFileHandleT const& source);
    SourceFileHandleT MocFindIncludedHeader(
      std::string const& includerDir, std::string const& includeBase) const;
    SourceFileHandleT MocFindHeader(std::string const& basePath) const;
    std::string MocMessageTestHeaders(std::string const& fileBase) const;
    bool MocRegisterIncluded(std::string const& includeString,
                             SourceFileHandleT includerFileHandle,
                             SourceFileHandleT sourceFileHandle,
                             bool sourceIsHeader) const;
    void MocRegisterMapping(MappingHandleT mappingHandle,
                            bool sourceIsHeader) const;

    // -- Uic
    bool UicEval(SourceFileMapT const& fileMap);
    bool UicEvalFile(SourceFileHandleT const& sourceFileHandle);
    SourceFileHandleT UicFindIncludedUi(std::string const& sourceFile,
                                        std::string const& sourceDir,
                                        IncludeKeyT const& incKey) const;
    bool UicRegisterMapping(std::string const& includeString,
                            SourceFileHandleT uiFileHandle,
                            SourceFileHandleT includerFileHandle);
  };

  /**
   * Generates moc/uic jobs
   */
  class JobGenerateT : public JobFenceT
  {
    void Process() override;
    // -- Moc
    bool MocGenerate(MappingHandleT const& mapping, bool compFile) const;
    bool MocUpdate(MappingT const& mapping, std::string* reason) const;
    std::pair<std::string, cmFileTime> MocFindDependency(
      std::string const& sourceDir, std::string const& includeString) const;
    // -- Uic
    bool UicGenerate(MappingHandleT const& mapping) const;
    bool UicUpdate(MappingT const& mapping, std::string* reason) const;
  };

  /**
   * File compiling base job
   */
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

  /**
   * moc compiles a file
   */
  class JobMocT : public JobCompileT
  {
  public:
    using JobCompileT::JobCompileT;
    void Process() override;
  };

  /**
   * uic compiles a file
   */
  class JobUicT : public JobCompileT
  {
  public:
    using JobCompileT::JobCompileT;
    void Process() override;
  };

  /// @brief Generate mocs_compilation.cpp
  ///
  class JobMocsCompilationT : public JobFenceT
  {
  private:
    void Process() override;
  };

  /// @brief The last job
  ///
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
  std::string AbsoluteBuildPath(std::string const& relativePath) const;
  std::string AbsoluteIncludePath(std::string const& relativePath) const;
  template <class JOBTYPE>
  void CreateParseJobs(SourceFileMapT const& sourceMap);

private:
  // -- Utility accessors
  Logger const& Log() const { return Logger_; }
  // -- Abstract processing interface
  bool Init(cmMakefile* makefile) override;
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

private:
  // -- Utility
  Logger Logger_;
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
};

#endif
