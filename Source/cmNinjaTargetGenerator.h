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

#include "cmStandardIncludes.h"
#include "cmNinjaTypes.h"
#include "cmLocalNinjaGenerator.h"
#include "cmOSXBundleGenerator.h"

class cmTarget;
class cmGlobalNinjaGenerator;
class cmGeneratedFileStream;
class cmGeneratorTarget;
class cmMakefile;
class cmSourceFile;
class cmCustomCommand;

class cmNinjaTargetGenerator
{
public:
  /// Create a cmNinjaTargetGenerator according to the @a target's type.
  static cmNinjaTargetGenerator* New(cmTarget* target);

  /// Build a NinjaTargetGenerator.
  cmNinjaTargetGenerator(cmTarget* target);

  /// Destructor.
  virtual ~cmNinjaTargetGenerator();

  virtual void Generate() = 0;

  std::string GetTargetName() const;

  bool needsDepFile(const std::string& lang);

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

  const char* GetConfigName() const;

  std::string LanguageCompilerRule(const std::string& lang) const
  { return lang + "_COMPILER"; }

  const char* GetFeature(const char* feature);
  bool GetFeatureAsBool(const char* feature);
  void AddFeatureFlags(std::string& flags, const char* lang);

  /**
   * Compute the flags for compilation of object files for a given @a language.
   * @note Generally it is the value of the variable whose name is computed
   *       by LanguageFlagsVarName().
   */
  std::string ComputeFlagsForObject(cmSourceFile *source,
                                    const std::string& language);

  std::string ComputeDefines(cmSourceFile *source,
                             const std::string& language);

  std::string ConvertToNinjaPath(const char *path) const {
    return this->GetLocalGenerator()->ConvertToNinjaPath(path);
  }
  cmLocalNinjaGenerator::map_to_ninja_path MapToNinjaPath() const {
    return this->GetLocalGenerator()->MapToNinjaPath();
  }

  /// @return the list of link dependency for the given target @a target.
  cmNinjaDeps ComputeLinkDeps() const;

  /// @return the source file path for the given @a source.
  std::string GetSourceFilePath(cmSourceFile* source) const;

  /// @return the object file path for the given @a source.
  std::string GetObjectFilePath(cmSourceFile* source) const;

  /// @return the file path where the target named @a name is generated.
  std::string GetTargetFilePath(const std::string& name) const;

  /// @return the output path for the target.
  virtual std::string GetTargetOutputDir() const;

  void WriteLanguageRules(const std::string& language);
  void WriteCompileRule(const std::string& language);
  void WriteObjectBuildStatements();
  void WriteObjectBuildStatement(cmSourceFile* source);
  void WriteCustomCommandBuildStatement(cmCustomCommand *cc);

  cmNinjaDeps GetObjects() const
  { return this->Objects; }

  // Helper to add flag for windows .def file.
  void AddModuleDefinitionFlag(std::string& flags);

  void EnsureDirectoryExists(const std::string& dir) const;
  void EnsureParentDirectoryExists(const std::string& path) const;

  // write rules for Mac OS X Application Bundle content.
  struct MacOSXContentGeneratorType :
    cmOSXBundleGenerator::MacOSXContentGeneratorType
  {
    MacOSXContentGeneratorType(cmNinjaTargetGenerator* g) :
      Generator(g)  {}

    void operator()(cmSourceFile& source, const char* pkgloc);

  private:
    cmNinjaTargetGenerator* Generator;
  };
  friend struct MacOSXContentGeneratorType;

protected:
  MacOSXContentGeneratorType* MacOSXContentGenerator;
  // Properly initialized by sub-classes.
  cmOSXBundleGenerator* OSXBundleGenerator;
  std::set<cmStdString> MacContentFolders;


private:
  cmTarget* Target;
  cmGeneratorTarget* GeneratorTarget;
  cmMakefile* Makefile;
  cmLocalNinjaGenerator* LocalGenerator;
  /// List of object files for this target.
  cmNinjaDeps Objects;

  // The windows module definition source file (.def), if any.
  std::string ModuleDefinitionFile;
};

#endif // ! cmNinjaTargetGenerator_h
