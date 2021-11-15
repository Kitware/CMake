/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2004-2009 Kitware, Inc.
  Copyright 2004 Alexander Neundorf (neundorf@kde.org)
  Copyright 2007 Miguel A. Figueroa-Villanueva
  Copyright 2017 Jakub Korbel

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGlobalIarGenerator_h
#define cmGlobalIarGenerator_h

#include <vector>

#include "cmGlobalGenerator.h"
#include "cmGlobalGeneratorFactory.h"
#include "cmDocumentationEntry.h"

class cmMakefile;
class cmGeneratedFileStream;

/** \class cmGlobalIarGenerator
 * \brief Write Eclipse project files for Makefile based projects
 */
class cmGlobalIarGenerator : public cmGlobalGenerator
{
public:
  static const char* XML_DECL;

  cmGlobalIarGenerator(cmake* cm);
  ~cmGlobalIarGenerator();

  std::unique_ptr<cmLocalGenerator> CreateLocalGenerator(cmMakefile* mf);

  static std::unique_ptr<cmGlobalGeneratorFactory> NewFactory()
  {

    return std::unique_ptr<cmGlobalGeneratorFactory>(
      new cmGlobalGeneratorSimpleFactory<cmGlobalIarGenerator>());
  }

  virtual std::string GetName() const {
    return cmGlobalIarGenerator::GetActualName();
  }

  static std::string GetActualName() { return "IAR Workbench for ARM"; }

  /// Overloaded methods. @see cmGlobalGenerator::GetDocumentation()
  static void GetDocumentation(cmDocumentationEntry& entry);

  ///
  /// Utilized by the generator factory to determine if this generator
  /// supports toolsets.
  ///
  static bool SupportsToolset() { return false; }

  ///
  /// Utilized by the generator factory to determine if this generator
  /// supports platforms.
  ///
  static bool SupportsPlatform() { return false; }


  /**
  * Try to determine system information such as shared library
  * extension, pthreads, byte order etc.
  */
  virtual void EnableLanguage(std::vector<std::string> const& languages,
                              cmMakefile*, bool optional);
  /*
  * Determine what program to use for building the project.
  */
  bool FindMakeProgram(cmMakefile* mf) override;

  void Generate();

  bool Open(const std::string& bindir, const std::string& projectName,
            bool dryRun) override;

  bool IsMultiConfig() const override {return false;};

  static std::string ToToolkitPath(std::string absolutePath);

  static std::string ToWorkbenchPath(std::string absolutePath);

  void ConvertTargetToProject(const cmTarget& target,
                              cmGeneratorTarget* genTgt);

protected:

  virtual std::vector<cmGlobalGenerator::GeneratedMakeCommand>
    GenerateBuildCommand(
    const std::string& makeProgram, const std::string& projectName,
    const std::string& projectDir, std::vector<std::string> const& targetNames,
    const std::string& config, bool /*fast*/, int jobs, bool /*verbose*/,
    std::vector<std::string> const& makeOptions);
private:

  std::string FindIarBuildCommand();

  void RegisterProject(const std::string& projectName);

  void GetCmdLines(std::vector<cmCustomCommand> const& rTmpCmdVec,
                   std::string& rBuildCmd,
                   int& rStart);

  struct CompilerOpts
  {
    enum CxxStandard
    {
      /// --c++
      CXX_STANDARD_CXX,

      /// --ec++
      CXX_STANDARD_EMBEDDED_CXX,

      /// --eec++
      CXX_STANDARD_EXTENDED_EMBEDDED_CXX
    };

    enum Optimization
    {
      OPTIMIZATION_NONE,
      OPTIMIZATION_LOW,
      OPTIMIZATION_MEDIUM,
      OPTIMIZATION_HIGH_SIZE,
      OPTIMIZATION_HIGH_SPEED,
      OPTIMIZATION_HIGH_BALANCED
    };

    struct
    {
      /// Are debug symbols present? (--debug)
      unsigned int debug:1;

      /// Is IAR Extended standard in use? (-e)
      unsigned int iarExtStd:1;

      /// If to use thumb mode or arm mode (--cpu_mode={arm|a|thumb|t} or --arm)
      unsigned int useThumb:1;

      /// If char is signed (default unsigned).
      unsigned int signedChar:1;

      /// Whether to use big endian instead of little endian. (--endian={big|little})
      unsigned int useBigEndian:1;

      /// If C compiler uses C++ style inlines (--use_c++_inline).
      unsigned int cxxStyleInlines:1;

      /// If C++ supports exceptions (--no_exceptions).
      unsigned int hasExceptions:1;

      /// If C++ compiler supports RTTI (--no_rtti).
      unsigned int hasRtti:1;

      /// If C++ compiler supports Static destruction (--no_static_destruction).
      unsigned int hasStaticDestruction:1;

      /// If guard calls are enabled.
      unsigned int enableGuardCalls:1;

      // todo Add more options.

    } flags;

    /// What FPU to use. (--fpu={VFPv2|VFPv3|VFPv3_d16|VFPv4|VFPv4_sp|VFP9-S|none})
    std::string fpu;

    /// What CPU to use. (--fpu=<CPU_NAME>)
    std::string cpu;

    /// DLIB config header file. (--dlib_config <FILENAME.h>)
    std::string dlibCfgFile;

    /// Pre-include header file. (--preinclude <FILENAME.h>)
    std::string preinclude;

    /// Used C++ standard (--c++ or --ec++ or --eec++)
    CxxStandard cxxStd;

    /// What level of optimization is used.
    Optimization optimization;

    // todo Add more options.

  };

  struct LinkerOpts
  {
    struct
    {
      /// If to inline small routines (--inline).
      unsigned int inlineSmall:1;

      /// If linker supports exceptions (--no_exceptions).
      unsigned int hasExceptions:1;

      /// If to merge equivalent read-only sections (--merge_duplicate_sections).
      unsigned int mergeDuplicateSections:1;

      /// If to use virtual function elimination (--vfe).
      unsigned int vfe:1;

      /// If semihosting is enabled (--semihosting).
      unsigned int semihosting:1;

      // todo Add more options.
    } flags;

    /// Map file (--map <FILENAME.map>).
    std::string mapFile;

    /// Configuration file (--config <FILENAME.icf>).
    std::string configFile;

    /// Entry routine (--entry <entry_routine>).
    std::string entryRoutine;
  };

  struct BuildConfig
  {
    /// build configuration name, this could be anything.
    std::string name;

    /// build configuration name in uppercase, this could be anything.
    std::string nameUppercase;

    /// toolchain name - ARM for ARM.
    std::string toolchain;

    /// directory where to store all object files (.o).
    std::string objectDir;

    /// directory where to store targets (executables and libs).
    std::string exeDir;

    /// directory for IAR lists.
    std::string listDir;

    /// true if the build contains debug information, false otherwise.
    bool isDebug;

    /// .icf path
    std::string icfPath;

    /// output file name.
    std::string outputFile;

    /// pre-build command line.
    std::string preBuildCmd;

    /// post-build command line.
    std::string postBuildCmd;

    /// Compile definitions <state></state>
    std::vector<std::string> compileDefs;

    /// Compiler options <state></state>
    std::vector<std::string> compilerOpts;

    /// Linker keep symbols <state></state>
    std::vector<std::string> linkerKeepSymbols;

    /// Linker options <state></state>
    std::vector<std::string> linkerOpts;

    /// ASM options <state></state>
    std::vector<std::string> asmOpts;

    /// Libraries <state></state>
    std::vector<std::string> libraries;
  };

  struct Project
  {
    /// Project name
    std::string name;

    /// Build config collection.
    BuildConfig buildCfg;

    /// output binary type: true if lib, false if exe.
    bool isLib;

    /// project directory. Everything should be relative to this path.
    std::string projectDir;

    /// binary directory.
    std::string binaryDir;

    /// Includes <state></state>
    std::vector<std::string> includes;

    /// Includes <state></state>
    std::vector<std::string> sources;

    void CreateProjectFile();
    void CreateDebuggerFile();
  };

  static void cmGlobalIarGenerator::ParseCmdLineOpts(std::string cmdLine,
                                              const char* multiOpts[],
                                              size_t multiOptsLen,
                                              std::vector<std::string>& opts);

  struct Workspace
  {
    /// Workspace name.
    std::string name;

    /// Workspace directory.
    std::string workspaceDir;

    /// Full workspace path
    std::string workspacePath;

    /// Project names collection.
    std::map<std::string, Project*> projects;

    void RegisterProject(std::string name, Project* project);

    void CreateWorkspaceFile();

    //void CreateDebuggerFile();
  };

  struct GlobalCmakeCfg
  {
    std::string buildType;
    std::string iarCCompilerFlags;
    std::string iarAsmFlags;
    std::string iarCxxCompilerFlags ;
    std::string iarLinkerFlags      ;
    std::string iarArmPath          ;
    std::string compilerDlibConfig  ;
    int compilerDlibConfigId;
    std::string compilerPathExe     ;
    std::string cpuName             ;
    std::string systemName          ;
    std::string dbgExtraOptions     ;
    std::string dbgCspyFlashLoaderv3;
    std::string dbgCspyMacfile      ;
    std::string dbgCspyMemfile      ;
    std::string dbgProbeSelection   ;
    std::string dbgIjetProbeconfig  ;
    std::string dbgLogFile          ;
    std::string linkerEntryRoutine  ;
    std::string linkerIcfFile       ;
    std::string tgtArch             ;
    std::string wbVersion           ;
    std::string chipSelection       ;
    std::string rtos                ;
    std::string compilerPreInclude  ;
    std::string scanfFmt  ;
    std::string printfFmt  ;
    int scanfFmtId  ;
    int printfFmtId  ;
    std::string bufferedTermOut;
    std::string semihostingEnabled;
  };

  static const char* PROJ_FILE_EXT;
  static const char* WS_FILE_EXT;
  static const char* DEFAULT_MAKE_PROGRAM;
  static const char* MULTIOPTS_COMPILER[13];
  static const char* MULTIOPTS_LINKER[24];

  Workspace workspace;
  static GlobalCmakeCfg GLOBALCFG;

};

#endif
