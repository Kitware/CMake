/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2011 Peter Collingbourne <peter@pcc.me.uk>
  Copyright 2011 Nicolas Despres <nicolas.despres@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmNinjaTargetGenerator_h
#define cmNinjaTargetGenerator_h

#include "cmCommonTargetGenerator.h"

#include "cmStandardIncludes.h"
#include "cmNinjaTypes.h"
#include "cmGlobalNinjaGenerator.h"
#include "cmLocalNinjaGenerator.h"
#include "cmOSXBundleGenerator.h"

class cmTarget;
class cmGeneratedFileStream;
class cmGeneratorTarget;
class cmMakefile;
class cmSourceFile;
class cmCustomCommand;

class cmNinjaTargetGenerator: public cmCommonTargetGenerator
{
public:
  /// Create a cmNinjaTargetGenerator according to the @a target's type.
  static cmNinjaTargetGenerator* New(cmGeneratorTarget* target);

  /// Build a NinjaTargetGenerator.
  cmNinjaTargetGenerator(cmGeneratorTarget* target);

  /// Destructor.
  virtual ~cmNinjaTargetGenerator();

  virtual void Generate() = 0;

  std::string GetTargetName() const;

  bool NeedDepTypeMSVC(const std::string& lang) const;

protected:

  bool SetMsvcTargetPdbVariable(cmNinjaVars&) const;

  cmGeneratedFileStream& GetBuildFileStream() const;
  cmGeneratedFileStream& GetRulesFileStream() const;

  cmTarget* GetTarget() const
  { return this->Target; }

  cmGeneratorTarget* GetGeneratorTarget() const
  { return this->GeneratorTarget; }

  cmLocalNinjaGenerator* GetLocalGenerator() const
  { return this->LocalGenerator; }

  cmGlobalNinjaGenerator* GetGlobalGenerator() const;

  cmMakefile* GetMakefile() const
  { return this->Makefile; }

  std::string LanguageCompilerRule(const std::string& lang) const;

  std::string OrderDependsTargetForTarget();

  std::string ComputeOrderDependsForTarget();

  /**
   * Compute the flags for compilation of object files for a given @a language.
   * @note Generally it is the value of the variable whose name is computed
   *       by LanguageFlagsVarName().
   */
  std::string ComputeFlagsForObject(cmSourceFile const* source,
                                    const std::string& language);

  void AddIncludeFlags(std::string& flags, std::string const& lang);

  std::string ComputeDefines(cmSourceFile const* source,
                             const std::string& language);

  std::string ConvertToNinjaPath(const std::string& path) const {
    return this->GetGlobalGenerator()->ConvertToNinjaPath(path);
  }
  cmGlobalNinjaGenerator::MapToNinjaPathImpl MapToNinjaPath() const {
    return this->GetGlobalGenerator()->MapToNinjaPath();
  }

  /// @return the list of link dependency for the given target @a target.
  cmNinjaDeps ComputeLinkDeps() const;

  /// @return the source file path for the given @a source.
  std::string GetSourceFilePath(cmSourceFile const* source) const;

  /// @return the object file path for the given @a source.
  std::string GetObjectFilePath(cmSourceFile const* source) const;

  /// @return the file path where the target named @a name is generated.
  std::string GetTargetFilePath(const std::string& name) const;

  /// @return the output path for the target.
  virtual std::string GetTargetOutputDir() const;

  void WriteLanguageRules(const std::string& language);
  void WriteCompileRule(const std::string& language);
  void WriteObjectBuildStatements();
  void WriteObjectBuildStatement(cmSourceFile const* source,
                                 bool writeOrderDependsTargetForTarget);

  void ExportObjectCompileCommand(
    std::string const& language,
    std::string const& sourceFileName,
    std::string const& objectDir,
    std::string const& objectFileName,
    std::string const& objectFileDir,
    std::string const& flags,
    std::string const& defines,
    std::string const& includes
    );

  cmNinjaDeps GetObjects() const
  { return this->Objects; }

  void EnsureDirectoryExists(const std::string& dir) const;
  void EnsureParentDirectoryExists(const std::string& path) const;

  // write rules for Mac OS X Application Bundle content.
  struct MacOSXContentGeneratorType :
    cmOSXBundleGenerator::MacOSXContentGeneratorType
  {
    MacOSXContentGeneratorType(cmNinjaTargetGenerator* g) :
      Generator(g)  {}

    void operator()(cmSourceFile const& source, const char* pkgloc);

  private:
    cmNinjaTargetGenerator* Generator;
  };
  friend struct MacOSXContentGeneratorType;


  MacOSXContentGeneratorType* MacOSXContentGenerator;
  // Properly initialized by sub-classes.
  cmOSXBundleGenerator* OSXBundleGenerator;
  std::set<std::string> MacContentFolders;

  void addPoolNinjaVariable(const std::string& pool_property,
                            cmTarget* target,
                            cmNinjaVars& vars);

private:
  cmLocalNinjaGenerator* LocalGenerator;
  /// List of object files for this target.
  cmNinjaDeps Objects;
  std::vector<cmCustomCommand const*> CustomCommands;
};

#endif // ! cmNinjaTargetGenerator_h
