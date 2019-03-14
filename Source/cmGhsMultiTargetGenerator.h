/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmGhsMultiTargetGenerator_h
#define cmGhsMultiTargetGenerator_h

#include "cmGhsMultiGpj.h"

#include "cmTarget.h"

class cmCustomCommand;
class cmGeneratedFileStream;
class cmGeneratorTarget;
class cmGlobalGhsMultiGenerator;
class cmLocalGhsMultiGenerator;
class cmMakefile;
class cmSourceFile;

class cmGhsMultiTargetGenerator
{
public:
  cmGhsMultiTargetGenerator(cmGeneratorTarget* target);

  virtual ~cmGhsMultiTargetGenerator();

  virtual void Generate();

private:
  cmGlobalGhsMultiGenerator* GetGlobalGenerator() const;

  void GenerateTarget();

  void WriteTargetSpecifics(std::ostream& fout, const std::string& config);

  void WriteCompilerFlags(std::ostream& fout, const std::string& config,
                          const std::string& language);
  void WriteCompilerDefinitions(std::ostream& fout, const std::string& config,
                                const std::string& language);

  void SetCompilerFlags(std::string const& config,
                        const std::string& language);

  std::string GetDefines(const std::string& langugae,
                         std::string const& config);

  void WriteIncludes(std::ostream& fout, const std::string& config,
                     const std::string& language);
  void WriteTargetLinkLine(std::ostream& fout, std::string const& config);
  void WriteCustomCommands(std::ostream& fout);
  void WriteCustomCommandsHelper(
    std::ostream& fout, std::vector<cmCustomCommand> const& commandsSet,
    cmTarget::CustomCommandType commandType);
  void WriteSources(std::ostream& fout_proj);
  void WriteSourceProperty(std::ostream& fout, const cmSourceFile* sf,
                           std::string propName, std::string propFlag);
  void WriteReferences(std::ostream& fout);
  static void WriteObjectLangOverride(std::ostream& fout,
                                      const cmSourceFile* sourceFile);

  bool DetermineIfIntegrityApp(void);
  cmGeneratorTarget* GeneratorTarget;
  cmLocalGhsMultiGenerator* LocalGenerator;
  cmMakefile* Makefile;
  std::map<std::string, std::string> FlagsByLanguage;
  std::map<std::string, std::string> DefinesByLanguage;

  std::string TargetNameReal;
  GhsMultiGpj::Types TagType;
  std::string const Name;
  std::string ConfigName; /* CMAKE_BUILD_TYPE */
};

#endif // ! cmGhsMultiTargetGenerator_h
