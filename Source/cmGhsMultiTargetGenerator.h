/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "cmGhsMultiGpj.h"

class cmCustomCommand;
class cmCustomCommandGenerator;
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

  void WriteTargetSpecifics(std::ostream& fout, std::string const& config);

  void WriteCompilerFlags(std::ostream& fout, std::string const& config,
                          std::string const& language);
  void WriteCompilerDefinitions(std::ostream& fout, std::string const& config,
                                std::string const& language);

  void SetCompilerFlags(std::string const& config,
                        std::string const& language);

  std::string GetDefines(std::string const& language,
                         std::string const& config);

  void WriteIncludes(std::ostream& fout, std::string const& config,
                     std::string const& language);
  void WriteTargetLinkLine(std::ostream& fout, std::string const& config);
  void WriteBuildEvents(std::ostream& fout);
  void WriteBuildEventsHelper(std::ostream& fout,
                              std::vector<cmCustomCommand> const& ccv,
                              std::string const& name, std::string const& cmd);
  void WriteCustomCommandsHelper(std::ostream& fout,
                                 cmCustomCommandGenerator const& ccg);
  void WriteCustomCommandLine(std::ostream& fout, std::string& fname,
                              cmCustomCommandGenerator const& ccg);
  bool ComputeCustomCommandOrder(std::vector<cmSourceFile const*>& order);
  bool VisitCustomCommand(std::set<cmSourceFile const*>& temp,
                          std::set<cmSourceFile const*>& perm,
                          std::vector<cmSourceFile const*>& order,
                          cmSourceFile const* sf);
  void WriteSources(std::ostream& fout_proj);
  void WriteSourceProperty(std::ostream& fout, cmSourceFile const* sf,
                           std::string const& propName,
                           std::string const& propFlag);
  static std::string WriteObjectLangOverride(cmSourceFile const* sourceFile);

  bool DetermineIfIntegrityApp();
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
