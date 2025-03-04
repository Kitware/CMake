/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <string>

#include <cm/optional>

#include "cmGlobalVisualStudioGenerator.h"
#include "cmIDEFlagTable.h"
#include "cmIDEOptions.h"

class cmLocalVisualStudioGenerator;

using cmVS7FlagTable = cmIDEFlagTable;

class cmVisualStudioGeneratorOptions : public cmIDEOptions
{
public:
  // Construct an options table for a given tool.
  enum Tool
  {
    Compiler,
    ResourceCompiler,
    CudaCompiler,
    MarmasmCompiler,
    MasmCompiler,
    NasmCompiler,
    Linker,
    FortranCompiler,
    CSharpCompiler
  };
  cmVisualStudioGeneratorOptions(cmLocalVisualStudioGenerator* lg, Tool tool,
                                 cmVS7FlagTable const* table = nullptr,
                                 cmVS7FlagTable const* extraTable = nullptr);

  // Add a table of flags.
  void AddTable(cmVS7FlagTable const* table);

  // Clear the flag tables.
  void ClearTables();

  // Store options from command line flags.
  void Parse(std::string const& flags);
  void ParseFinish();

  void PrependInheritedString(std::string const& key);

  // Parse the content of the given flag table entry again to extract
  // known flags and leave the rest in the original entry.
  void Reparse(std::string const& key);

  // Fix the ExceptionHandling option to default to off.
  void FixExceptionHandlingDefault();

  // Store options for verbose builds.
  void SetVerboseMakefile(bool verbose);

  // Check for specific options.
  bool UsingUnicode() const;
  bool UsingSBCS() const;

  void FixCudaCodeGeneration();

  void FixManifestUACFlags();

  bool UsingDebugInfo() const;
  cm::optional<bool> UsingDebugRuntime() const;
  bool IsWinRt() const;
  bool IsManaged() const;
  // Write options to output.
  void OutputPreprocessorDefinitions(std::ostream& fout, int indent,
                                     std::string const& lang);
  void OutputAdditionalIncludeDirectories(std::ostream& fout, int indent,
                                          std::string const& lang);
  void OutputFlagMap(std::ostream& fout, int indent);
  void SetConfiguration(std::string const& config);
  std::string const& GetConfiguration() const;

protected:
  virtual void OutputFlag(std::ostream& fout, int indent,
                          std::string const& tag,
                          std::string const& content) = 0;

private:
  cmLocalVisualStudioGenerator* LocalGenerator;

  std::string Configuration;
  Tool CurrentTool;

  bool FortranRuntimeDebug;
  bool FortranRuntimeDLL;
  bool FortranRuntimeMT;

  std::string UnknownFlagField;

  void StoreUnknownFlag(std::string const& flag) override;

  FlagValue TakeFlag(std::string const& key);
};
