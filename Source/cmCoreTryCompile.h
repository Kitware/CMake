/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <string>
#include <utility>
#include <vector>

#include <cm/optional>
#include <cm/string_view>

#include "cmArgumentParser.h"
#include "cmArgumentParserTypes.h"
#include "cmList.h"
#include "cmStateTypes.h"

class cmConfigureLog;
class cmMakefile;
template <typename Iter>
class cmRange;

struct cmTryCompileResult
{
  cm::optional<std::string> LogDescription;
  std::map<std::string, std::string> CMakeVariables;

  std::string SourceDirectory;
  std::string BinaryDirectory;

  bool VariableCached = true;
  std::string Variable;

  std::string Output;
  int ExitCode = 1;
};

/** \class cmCoreTryCompile
 * \brief Base class for cmTryCompileCommand and cmTryRunCommand
 *
 * cmCoreTryCompile implements the functionality to build a program.
 * It is the base class for cmTryCompileCommand and cmTryRunCommand.
 */
class cmCoreTryCompile
{
public:
  cmCoreTryCompile(cmMakefile* mf)
    : Makefile(mf)
  {
  }

  struct Arguments : public ArgumentParser::ParseResult
  {
    Arguments(cmMakefile const* mf)
      : Makefile(mf)
    {
    }

    cmMakefile const* Makefile;

    enum class SourceType
    {
      Normal,
      CxxModule,
      Directory,
    };

    cm::optional<std::string> CompileResultVariable;
    cm::optional<std::string> BinaryDirectory;
    cm::optional<std::string> SourceDirectoryOrFile;
    cm::optional<std::string> ProjectName;
    cm::optional<std::string> TargetName;
    cm::optional<ArgumentParser::NonEmpty<
      std::vector<std::pair<std::string, SourceType>>>>
      Sources;
    cm::optional<ArgumentParser::NonEmpty<
      std::vector<std::pair<std::string, SourceType>>>>
      SourceFromContent;
    cm::optional<ArgumentParser::NonEmpty<
      std::vector<std::pair<std::string, SourceType>>>>
      SourceFromVar;
    cm::optional<ArgumentParser::NonEmpty<
      std::vector<std::pair<std::string, SourceType>>>>
      SourceFromFile;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> CMakeFlags{
      1, "CMAKE_FLAGS"
    }; // fake argv[0]
    cmList CompileDefs;
    cm::optional<ArgumentParser::MaybeEmpty<std::vector<std::string>>>
      LinkLibraries;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> LinkOptions;
    cm::optional<std::string> LinkerLanguage;
    std::map<std::string, std::string> LangProps;
    std::string CMakeInternal;
    cm::optional<std::string> OutputVariable;
    cm::optional<std::string> CopyFileTo;
    cm::optional<std::string> CopyFileError;
    cm::optional<ArgumentParser::NonEmpty<std::string>> LogDescription;
    bool NoCache = false;
    bool NoLog = false;

    ArgumentParser::Continue SetSourceType(cm::string_view sourceType);
    SourceType SourceTypeContext = SourceType::Normal;
    std::string SourceTypeError;

    // Argument for try_run only.
    // Keep in sync with warnings in cmCoreTryCompile::ParseArgs.
    cm::optional<std::string> CompileOutputVariable;
    cm::optional<std::string> RunOutputVariable;
    cm::optional<std::string> RunOutputStdOutVariable;
    cm::optional<std::string> RunOutputStdErrVariable;
    cm::optional<std::string> RunWorkingDirectory;
    cm::optional<ArgumentParser::MaybeEmpty<std::vector<std::string>>> RunArgs;
  };

  Arguments ParseArgs(cmRange<std::vector<std::string>::const_iterator> args,
                      bool isTryRun);

  /**
   * This is the core code for try compile. It is here so that other commands,
   * such as TryRun can access the same logic without duplication.
   *
   * This function requires at least two \p arguments and will crash if given
   * fewer.
   */
  cm::optional<cmTryCompileResult> TryCompileCode(
    Arguments& arguments, cmStateEnums::TargetType targetType);

  /**
   * Returns \c true if \p path resides within a CMake temporary directory,
   * otherwise returns \c false.
   */
  static bool IsTemporary(std::string const& path);

  /**
   * This deletes all the files created by TryCompileCode.
   * This way we do not have to rely on the timing and
   * dependencies of makefiles.
   */
  void CleanupFiles(std::string const& binDir);

  /**
   * This tries to find the (executable) file created by
  TryCompileCode. The result is stored in OutputFile. If nothing is found,
  the error message is stored in FindErrorMessage.
   */
  void FindOutputFile(const std::string& targetName);

  static void WriteTryCompileEventFields(
    cmConfigureLog& log, cmTryCompileResult const& compileResult);

  std::string BinaryDirectory;
  std::string OutputFile;
  std::string FindErrorMessage;
  bool SrcFileSignature = false;
  cmMakefile* Makefile;

private:
  std::string WriteSource(std::string const& name, std::string const& content,
                          char const* command) const;

  Arguments ParseArgs(
    const cmRange<std::vector<std::string>::const_iterator>& args,
    const cmArgumentParser<Arguments>& parser,
    std::vector<std::string>& unparsedArguments);
};
