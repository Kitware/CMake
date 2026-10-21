/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmcmd.h"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iomanip>
#include <iterator>
#include <limits>

#include <cm/optional>
#include <cmext/algorithm>
#include <cmext/string_view>

#include <cm3p/uv.h>
#include <fcntl.h>

#include "cmCommandLineArgument.h"
#include "cmCryptoHash.h"
#include "cmDiagnostics.h"
#include "cmDuration.h"
#include "cmEnvironment.h"
#include "cmGeneratedFileStream.h"
#include "cmGlobalGenerator.h"
#include "cmList.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessenger.h"
#include "cmProcessOutput.h"
#include "cmQtAutoMocUic.h"
#include "cmQtAutoRcc.h"
#include "cmRange.h"
#include "cmState.h"
#include "cmStateDirectory.h"
#include "cmStateSnapshot.h"
#include "cmStdIoConsole.h"
#include "cmStdIoStream.h"
#include "cmStdIoTerminal.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTransformDepfile.h"
#include "cmUVProcessChain.h"
#include "cmUVStream.h"
#include "cmUtils.hxx"
#include "cmValue.h"
#include "cmVersion.h"
#include "cmake.h"

#if !defined(CMAKE_BOOTSTRAP)
#  include "cmDependsFortran.h" // For -E cmake_copy_f90_mod callback.
#  include "cmFileTime.h"

#  include "bindexplib.h"
#endif

#if !defined(CMAKE_BOOTSTRAP) || defined(CMAKE_BOOTSTRAP_MAKEFILES)
#  include "cmCMakePath.h"
#  include "cmProcessTools.h"
#endif

#if !defined(CMAKE_BOOTSTRAP) && defined(_WIN32) && !defined(__CYGWIN__)
#  include "cmVisualStudioWCEPlatformParser.h"
#endif

#include <array>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <utility>

#ifdef _WIN32
#  include <fcntl.h> // for _O_BINARY
#  include <io.h>    // for _setmode
#endif

#include <cm/string_view>

#include "cmsys/Directory.hxx"
#include "cmsys/FStream.hxx"
#include "cmsys/RegularExpression.hxx"

// We need identifiers containing two underscores
// to make it easy to sort the commands table.
#if defined(__clang__) && defined(__has_warning)
#  if __has_warning("-Wreserved-identifier")
#    pragma clang diagnostic ignored "-Wreserved-identifier"
#  endif
#endif

int cmcmd_cmake_ninja_depends(std::vector<std::string>::const_iterator argBeg,
                              std::vector<std::string>::const_iterator argEnd);
int cmcmd_cmake_ninja_dyndep(std::vector<std::string>::const_iterator argBeg,
                             std::vector<std::string>::const_iterator argEnd);
int cmcmd_cmake_module_compile_db(
  std::vector<std::string>::const_iterator argBeg,
  std::vector<std::string>::const_iterator argEnd);

std::ostream& operator<<(
  std::ostream& stream,
  std::function<std::ostream&(std::ostream&)> const& func)
{
  return func(stream);
}

namespace {
enum class Group
{
  Internal,
  Common,
  Windows,
};

struct Command
{
  cm::string_view Name;

  friend bool operator<(Command const& l, cm::string_view r)
  {
    return l.Name < r;
  }

  cm::string_view Usage;
  cm::string_view Help;

  struct Arguments : public std::vector<std::string>
  {
    Arguments(std::vector<std::string> args,
              cm::optional<cm::StdIo::Console> console)
      : vector(std::move(args))
      , Console(std::move(console))
    {
    }
    cm::optional<cm::StdIo::Console> Console;
  };
  int (*Impl)(Arguments);

  ::Group Group;
};

bool cmTarFilesFrom(std::string const& file, std::vector<std::string>& files)
{
  if (cmSystemTools::FileIsDirectory(file)) {
    cmSystemTools::Error(
      cmStrCat("-E tar --files-from= file '", file, "' is a directory"));
    return false;
  }
  cmsys::ifstream fin(file.c_str());
  if (!fin) {
    cmSystemTools::Error(
      cmStrCat("-E tar --files-from= file '", file, "' not found"));
    return false;
  }
  std::string line;
  while (cmSystemTools::GetLineFromStream(fin, line)) {
    if (line.empty()) {
      continue;
    }
    if (cmHasLiteralPrefix(line, "--add-file=")) {
      files.push_back(line.substr(11));
    } else if (cmHasPrefix(line, '-')) {
      cmSystemTools::Error(cmStrCat("-E tar --files-from='", file,
                                    "' file invalid line:\n", line, '\n'));
      return false;
    } else {
      files.push_back(line);
    }
  }
  return true;
}

void cmCatFile(cm::optional<std::string> fileToAppend)
{
#ifdef _WIN32
  _setmode(fileno(stdin), _O_BINARY);
  _setmode(fileno(stdout), _O_BINARY);
#endif
  std::streambuf* buf = std::cin.rdbuf();
  cmsys::ifstream source;
  if (fileToAppend.has_value()) {
    source.open(fileToAppend->c_str(), (std::ios::binary | std::ios::in));
    buf = source.rdbuf();
  }
  std::cout << buf;
}

bool cmRemoveDirectory(std::string const& dir, bool recursive = true)
{
  if (cmSystemTools::FileIsSymlink(dir)) {
    if (!cmSystemTools::RemoveFile(dir)) {
      std::cerr << "Error removing directory symlink \"" << dir << "\".\n";
      return false;
    }
  } else if (!recursive) {
    std::cerr << "Error removing directory \"" << dir
              << "\" without recursive option.\n";
    return false;
  } else if (!cmSystemTools::RemoveADirectory(dir)) {
    std::cerr << "Error removing directory \"" << dir << "\".\n";
    return false;
  }
  return true;
}

#if !defined(CMAKE_BOOTSTRAP) || defined(CMAKE_BOOTSTRAP_MAKEFILES)
class CLIncludeParser : public cmProcessTools::LineParser
{
public:
  CLIncludeParser(cm::string_view includePrefix, cmsys::ofstream& depFile,
                  std::ostream& output)
    : IncludePrefix(includePrefix)
    , DepFile(depFile)
    , Output(output)
  {
  }

private:
  bool ProcessLine() override
  {
    if (cmHasPrefix(this->Line, this->IncludePrefix)) {
      auto path =
        cmTrimWhitespace(this->Line.c_str() + this->IncludePrefix.size());
      cmSystemTools::ConvertToLongPath(path);
      this->DepFile << cmCMakePath(path).GenericString() << std::endl;
    } else {
      this->Output << this->Line << std::endl;
    }

    return true;
  }

  cm::string_view IncludePrefix;
  cmsys::ofstream& DepFile;
  std::ostream& Output;
};

class CLOutputLogger : public cmProcessTools::OutputLogger
{
public:
  CLOutputLogger(std::ostream& log)
    : cmProcessTools::OutputLogger(log)
  {
  }

  bool ProcessLine() override
  {
    *this->Log << std::flush;
    return true;
  }
};

int CLCompileAndDependencies(std::vector<std::string> const& args)
{
  std::string depFile;
  std::string currentBinaryDir;
  std::string filterPrefix;
  std::vector<std::string> command;
  for (auto it = args.cbegin() + 2; it != args.cend(); it++) {
    if (cmHasLiteralPrefix(*it, "--dep-file=")) {
      depFile = it->substr(11);
    } else if (cmHasLiteralPrefix(*it, "--working-dir=")) {
      currentBinaryDir = it->substr(14);
    } else if (cmHasLiteralPrefix(*it, "--filter-prefix=")) {
      filterPrefix = it->substr(16);
    } else if (*it == "--") {
      command.insert(command.begin(), ++it, args.cend());
      break;
    } else {
      return 1;
    }
  }

  cmUVProcessChainBuilder builder;
  builder.AddCommand(command).SetWorkingDirectory(currentBinaryDir);

  cmsys::ofstream fout(depFile.c_str());
  if (!fout) {
    return 3;
  }

  CLIncludeParser includeParser(filterPrefix, fout, std::cout);
  CLOutputLogger errLogger(std::cerr);

  // Start the process.
  auto result =
    cmProcessTools::RunProcess(builder, &includeParser, &errLogger);
  auto const& subStatus = result.front();

  int status = 0;
  // handle status of process
  if (subStatus.SpawnResult != 0) {
    status = 2;
  } else if (subStatus.TermSignal != 0) {
    status = 1;
  } else {
    status = static_cast<int>(subStatus.ExitStatus);
  }

  if (status != 0) {
    // remove the dependencies file because potentially invalid
    fout.close();
    cmSystemTools::RemoveFile(depFile);
  }

  return status;
}
#endif

int HandleIWYU(std::string const& runCmd, std::string const& /* sourceFile */,
               std::string const& /*objectFile*/,
               std::vector<std::string> const& orig_cmd)
{
  // Construct the iwyu command line by taking what was given
  // and adding all the arguments we give to the compiler.
  cmList iwyu_cmd{ runCmd, cmList::EmptyElements::Yes };
  cm::append(iwyu_cmd, orig_cmd.begin() + 1, orig_cmd.end());
  // Run the iwyu command line.  Capture its stderr and hide its stdout.
  std::string stdErr;
  int ret;
  if (!cmSystemTools::RunSingleCommand(iwyu_cmd, nullptr, &stdErr, &ret,
                                       nullptr, cmSystemTools::OUTPUT_NONE)) {
    std::cerr << "Error running '" << iwyu_cmd[0] << "': " << stdErr << '\n';
    return 1;
  }
  // Warn if iwyu reported anything.
  if (stdErr.find("should remove these lines:") != std::string::npos ||
      stdErr.find("should add these lines:") != std::string::npos) {
    std::cerr << "Warning: include-what-you-use reported diagnostics:\n"
              << stdErr << '\n';
  }
  // Older versions of iwyu always returned a non-zero exit code,
  // so ignore it unless the user has enabled errors.
  auto has_error_opt = std::find_if(
    iwyu_cmd.cbegin(), iwyu_cmd.cend(),
    [](std::string const& opt) { return cmHasLiteralPrefix(opt, "--error"); });
  bool errors_enabled = has_error_opt != iwyu_cmd.cend() &&
    has_error_opt != iwyu_cmd.cbegin() &&
    *std::prev(has_error_opt) == "-Xiwyu";
  return errors_enabled ? ret : 0;
}

int HandleTidy(std::string const& runCmd, std::string const& sourceFile,
               std::string const& /*objectFile*/,
               std::vector<std::string> const& orig_cmd)
{
  cmList tidy_cmd{ runCmd, cmList::EmptyElements::Yes };
  tidy_cmd.push_back(sourceFile);

  for (auto const& arg : tidy_cmd) {
    if (cmHasLiteralPrefix(arg, "--export-fixes=")) {
      cmSystemTools::RemoveFile(arg.substr(cmStrLen("--export-fixes=")));
    }
  }

  // clang-tidy supports working out the compile commands from a
  // compile_commands.json file in a directory given by a "-p" option, or by
  // passing the compiler command line arguments after --. When the latter
  // strategy is used and the build is using a compiler other than the system
  // default, clang-tidy may erroneously use the system default compiler's
  // headers instead of those from the custom compiler. It doesn't do that if
  // given a compile_commands.json to work with instead, so prefer to use the
  // compile_commands.json file when "-p" is present.
  if (!cm::contains(tidy_cmd.cbegin(), tidy_cmd.cend() - 1, "-p")) {
    // Construct the clang-tidy command line by taking what was given
    // and adding our compiler command line.  The clang-tidy tool will
    // automatically skip over the compiler itself and extract the
    // options. If the compiler is a custom compiler, clang-tidy might
    // not correctly handle that with this approach.
    tidy_cmd.emplace_back("--");
    cm::append(tidy_cmd, orig_cmd);
  }

  // Run the tidy command line.  Capture its stdout and hide its stderr.
  int ret;
  std::string stdOut;
  std::string stdErr;
  if (!cmSystemTools::RunSingleCommand(tidy_cmd, &stdOut, &stdErr, &ret,
                                       nullptr, cmSystemTools::OUTPUT_NONE)) {
    std::cerr << "Error running '" << tidy_cmd[0] << "': " << stdErr << '\n';
    return 1;
  }
  // Output the stdout from clang-tidy to stderr
  std::cerr << stdOut;
  // If clang-tidy exited with an error do the same.
  if (ret != 0) {
    std::cerr << stdErr;
  }
  return ret;
}

int HandlePVSStudio(std::string const& runCmd, std::string const& sourceFile,
                    std::string const& objectFile,
                    std::vector<std::string> const& orig_cmd)
{
  cmList pvsCmd{ runCmd, cmList::EmptyElements::Yes };
  std::string logFile = cmStrCat(objectFile, "-pvs.log");
  std::string errFile = cmStrCat(objectFile, "-pvs.err");
  pvsCmd.reserve(pvsCmd.size() + 5 + orig_cmd.size());
  pvsCmd.emplace_back("--source-file");
  pvsCmd.emplace_back(sourceFile);
  pvsCmd.emplace_back("--output-file");
  pvsCmd.emplace_back(logFile);
  pvsCmd.emplace_back("--cl-params");
  for (size_t i = 1; i < orig_cmd.size(); ++i) {
    pvsCmd.emplace_back(orig_cmd[i]);
  }
  int ret;
  std::string stdOut;
  std::string stdErr;

  // Run the PVS command line.  Capture its stdout and hide its stderr.
  if (!cmSystemTools::RunSingleCommand(pvsCmd, &stdOut, &stdErr, &ret, nullptr,
                                       cmSystemTools::OUTPUT_NONE)) {
    std::cerr << "Error running '" << pvsCmd[0] << "': " << stdErr << '\n';
    return 1;
  }
  if (ret != 0) {
    if (ret == 7 && !cmSystemTools::FileExists(logFile)) {
      return 0; // Analyzer generated no output from source
    }
    std::cout << stdOut;
    std::cerr << stdErr;
    return ret;
  }

  // Find the plog-converter tool
#ifdef _WIN32
  std::string plogConvertName = "HtmlGenerator.exe";
#else
  std::string plogConvertName = "plog-converter";
#endif
  std::string plogConvert = cmStrCat(
    cmSystemTools::GetFilenamePath(pvsCmd.front()), '/', plogConvertName);
  if (!cmSystemTools::FileIsExecutable(plogConvert)) {
    plogConvert = cmSystemTools::FindProgram(plogConvertName);
  }
  if (plogConvert.empty()) {
    std::cerr << "Could not find " << plogConvertName << std::endl;
    return 1;
  }

  // Run the plog-converter tool
  std::vector<std::string> plogCmd{ plogConvert, "-t",    "errorfile",
                                    "-o",        errFile, logFile };
  if (!cmSystemTools::RunSingleCommand(plogCmd, &stdOut, &stdErr, &ret,
                                       nullptr, cmSystemTools::OUTPUT_NONE)) {
    std::cerr << "Error running '" << plogCmd[0] << "': " << stdErr << '\n';
    return 1;
  }

  // Show error messages from plog-converter output
  if (stdOut.find("Total messages 0") == std::string::npos) {
    cmsys::ifstream errFileStream(errFile.c_str());
    // output always begins with a Help message
    errFileStream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cerr << errFileStream.rdbuf();
  }
  cmSystemTools::RemoveFile(logFile);

  if (ret != 0) {
    std::cerr << stdErr;
  }
  return ret;
}

int HandleLWYU(std::string const& runCmd, std::string const& sourceFile,
               std::string const& /*objectFile*/,
               std::vector<std::string> const&)
{
  // Construct the ldd -r -u (link what you use lwyu) command line
  // ldd -u -r lwuy target
  cmList lwyu_cmd{ runCmd, cmList::EmptyElements::Yes };
  lwyu_cmd.push_back(sourceFile);

  // Run the lwyu check command line,  currently ldd is expected.
  // Capture its stdout and hide its stderr.
  // Ignore its return code because the tool always returns non-zero
  // if there are any warnings, but we just want to warn.
  std::string stdOut;
  std::string stdErr;
  int ret;
  if (!cmSystemTools::RunSingleCommand(lwyu_cmd, &stdOut, &stdErr, &ret,
                                       nullptr, cmSystemTools::OUTPUT_NONE)) {
    std::cerr << "Error running '" << lwyu_cmd[0] << "': " << stdErr << '\n';
    return 1;
  }

  // Output the stdout from ldd -r -u to stderr
  // Warn if lwyu reported anything.
  if (stdOut.find("Unused direct dependencies:") != std::string::npos) {
    std::cerr << "Warning: " << stdOut;
  }
  return 0;
}

int HandleCppLint(std::string const& runCmd, std::string const& sourceFile,
                  std::string const& /*objectFile*/,
                  std::vector<std::string> const&)
{
  // Construct the cpplint command line.
  cmList cpplint_cmd{ runCmd, cmList::EmptyElements::Yes };
  cpplint_cmd.push_back(sourceFile);

  // Run the cpplint command line.  Capture its output.
  std::string stdOut;
  int ret;
  if (!cmSystemTools::RunSingleCommand(cpplint_cmd, &stdOut, &stdOut, &ret,
                                       nullptr, cmSystemTools::OUTPUT_NONE)) {
    std::cerr << "Error running '" << cpplint_cmd[0] << "': " << stdOut
              << '\n';
    return 1;
  }
  if (!stdOut.empty()) {
    std::cerr << "Warning: cpplint diagnostics:\n"
              // Output the output from cpplint to stderr
              << stdOut;
  }

  // always return 0 so the build can continue as cpplint returns non-zero
  // for any warning
  return 0;
}

int HandleCppCheck(std::string const& runCmd, std::string const& sourceFile,
                   std::string const& /*objectFile*/,
                   std::vector<std::string> const& orig_cmd)
{
  // Construct the cpplint command line.
  cmList cppcheck_cmd{ runCmd, cmList::EmptyElements::Yes };
  // extract all the -D, -U, and -I options from the compile line
  for (auto const& opt : orig_cmd) {
    if (opt.size() > 2) {
      if ((opt[0] == '-') &&
          ((opt[1] == 'D') || (opt[1] == 'I') || (opt[1] == 'U'))) {
        cppcheck_cmd.push_back(opt);
// convert cl / options to - options if needed
#if defined(_WIN32)
      } else if ((opt[0] == '/') &&
                 ((opt[1] == 'D') || (opt[1] == 'I') || (opt[1] == 'U'))) {
        std::string optcopy = opt;
        optcopy[0] = '-';
        cppcheck_cmd.push_back(optcopy);
#endif
      }
    }
  }
  // add the source file
  cppcheck_cmd.push_back(sourceFile);

  // Run the cpplint command line.  Capture its output.
  std::string stdOut;
  std::string stdErr;
  int ret;
  if (!cmSystemTools::RunSingleCommand(cppcheck_cmd, &stdOut, &stdErr, &ret,
                                       nullptr, cmSystemTools::OUTPUT_NONE)) {
    std::cerr << "Error running '" << cppcheck_cmd[0] << "': " << stdOut
              << '\n';
    return 1;
  }
  std::cerr << stdOut;
  // Output the output from cpplint to stderr
  if (stdErr.find("(error)") != std::string::npos ||
      stdErr.find("(warning)") != std::string::npos ||
      stdErr.find("(style)") != std::string::npos ||
      stdErr.find("(performance)") != std::string::npos ||
      stdErr.find("(portability)") != std::string::npos ||
      stdErr.find("(information)") != std::string::npos) {
    if (ret == 0) {
      std::cerr << "Warning: cppcheck reported diagnostics:\n";
    } else {
      std::cerr << "Error: cppcheck reported failure:\n";
    }
  }
  std::cerr << stdErr;

  return ret;
}

int HandleIcstat(std::string const& runCmd, std::string const& sourceFile,
                 std::string const& /*objectFile*/,
                 std::vector<std::string> const& orig_cmd)
{
  // Construct the IAR C-STAT command line.
  cmList icstat_cmd{ runCmd, cmList::EmptyElements::Yes };
  std::string stdOut;
  std::string stdErr;
  int ret;

  std::string const double_dash{ "--" };
  icstat_cmd.push_back(double_dash);

  for (auto const& cmd : orig_cmd) {
    icstat_cmd.emplace_back(cmd);
  }

  // Create a default C-STAT manifest file containing standard checks
  // if it does not already exist.
  if (!cmSystemTools::FileExists("cstat_sel_checks.txt")) {
    std::vector<std::string> ichecks_cmd;
    ichecks_cmd.emplace_back(
      cmStrCat(cmSystemTools::GetFilenamePath(orig_cmd[0]), "/ichecks"));
    ichecks_cmd.emplace_back("--default=stdchecks");

    if (!cmSystemTools::RunSingleCommand(ichecks_cmd, &stdOut, &stdErr, &ret,
                                         nullptr,
                                         cmSystemTools::OUTPUT_NONE)) {
      std::cerr << "C-STAT: Error creating default manifest file '"
                << ichecks_cmd[0] << "': " << stdOut << '\n';
      return 1;
    }
  }

  // Run the IAR C-STAT command line. Capture its output.
  if (!cmSystemTools::RunSingleCommand(icstat_cmd, &stdOut, &stdErr, &ret,
                                       nullptr, cmSystemTools::OUTPUT_NONE)) {
    std::cerr << "C-STAT: Error running '" << icstat_cmd[0] << "': " << stdOut
              << '\n';
    return 1;
  }

  // Post-process successful output for consistent messaging and path cleanup.
  if (ret == 0) {
    // Harmonize IAR C-STAT headers between the first-time analysis
    // and subsequent builds that use cached results.
    std::cerr << "C-STAT: ";
    if (stdOut.find("Analyzing") == std::string::npos) {
      std::string prepend;
      prepend =
        cmStrCat("Analyzing ", cmSystemTools::GetFilenameName(sourceFile),
                 " (cached)\n");
      stdOut.insert(0, prepend);
    }

    // Single-pass removal of consecutive spurious relative paths,
    // `"../` or `"..\`, that might appear after a newline followed by
    // an opening quote, effectively displaying paths relative to
    // CMAKE_SOURCE_DIR.
    // Example: `\n"../../../src/file.c` -> `\n"src/file.c`
    std::string cleaned;
    size_t const n = stdOut.size();
    cleaned.reserve(n);
    size_t pos = 0;
    while (pos < n) {
      if (pos + 1 < n && stdOut[pos] == '\n' && stdOut[pos + 1] == '"') {
        cleaned += "\n\"";
        pos += 2; // skip `\n"`

        while (pos + 2 < n && stdOut[pos] == '.' && stdOut[pos + 1] == '.' &&
               (stdOut[pos + 2] == '/' || stdOut[pos + 2] == '\\')) {
          pos += 3; // skip `../` or `..\`
        }
      } else {
        cleaned += stdOut[pos];
        ++pos;
      }
    }
    stdOut.swap(cleaned);
  } else if (ret != 0) {
    std::cerr << "C-STAT ";
  }
  std::cerr << stdOut;
  std::cerr << stdErr;

  return ret;
}

using CoCompileHandler = int (*)(std::string const&, std::string const&,
                                 std::string const&,
                                 std::vector<std::string> const&);

struct CoCompiler
{
  char const* Option;
  CoCompileHandler Handler;
  bool NoOriginalCommand;
};

std::array<CoCompiler, 7> const CoCompilers = { {
  // Table of options and handlers.
  { "--cppcheck=", HandleCppCheck, false },
  { "--cpplint=", HandleCppLint, false },
  { "--icstat=", HandleIcstat, false },
  { "--iwyu=", HandleIWYU, false },
  { "--lwyu=", HandleLWYU, true },
  { "--pvs-studio=", HandlePVSStudio, false },
  { "--tidy=", HandleTidy, false },
} };

struct CoCompileJob
{
  std::string Command;
  CoCompileHandler Handler;
};

struct Bin2CTemplateFile
{
  std::istream* TemplateStream;
  cm::string_view ArrayPlaceholder;
  cm::string_view LengthPlaceholder;
};

enum class Bin2CBase
{
  Hex,
  Decimal,
};

std::size_t const BIN2C_BUFFER_SIZE = 16384;
std::size_t const BIN2C_ROW_WIDTH = 32;

inline std::size_t Bin2CColumnPosHex(std::size_t column)
{
  return cmStrLen(" ") + column * cmStrLen(" 0x__,");
}

inline std::size_t Bin2CColumnPosDecimal(std::size_t column)
{
  return cmStrLen(" ") + column * cmStrLen("____,");
}

inline char Bin2CDigit(unsigned char byte, int place)
{
  return '0' + static_cast<char>((byte / place) % 10);
}

inline void Bin2CPrintCharToBuffer(bool printSigned, Bin2CBase base,
                                   std::string& line, unsigned char byte,
                                   std::uint64_t pos)
{
  static char const hextable[] = "0123456789ABCDEF";
  bool neg = false;
  if (printSigned && byte & 0x80) {
    neg = true;
    byte = ~byte + 1;
  }
  switch (base) {
    case Bin2CBase::Hex:
      if (printSigned) {
        line[Bin2CColumnPosHex(pos % BIN2C_ROW_WIDTH)] = neg ? '-' : ' ';
      }
      line[Bin2CColumnPosHex(pos % BIN2C_ROW_WIDTH) + cmStrLen(" 0x")] =
        hextable[byte >> 4];
      line[Bin2CColumnPosHex(pos % BIN2C_ROW_WIDTH) + cmStrLen(" 0x_")] =
        hextable[byte & 0xF];
      break;
    case Bin2CBase::Decimal:
      std::size_t negOffset = byte >= 100 ? cmStrLen("")
        : byte >= 10                      ? cmStrLen("_")
                                          : cmStrLen("__");
      for (std::size_t i = cmStrLen(""); i < negOffset; i++) {
        line[Bin2CColumnPosDecimal(pos % BIN2C_ROW_WIDTH) + i] = ' ';
      }
      line[Bin2CColumnPosDecimal(pos % BIN2C_ROW_WIDTH) + negOffset] =
        neg ? '-' : ' ';
      if (byte >= 100) {
        line[Bin2CColumnPosDecimal(pos % BIN2C_ROW_WIDTH) + cmStrLen("_")] =
          Bin2CDigit(byte, 100);
      }
      if (byte >= 10) {
        line[Bin2CColumnPosDecimal(pos % BIN2C_ROW_WIDTH) + cmStrLen("__")] =
          Bin2CDigit(byte, 10);
      }
      line[Bin2CColumnPosDecimal(pos % BIN2C_ROW_WIDTH) + cmStrLen("___")] =
        Bin2CDigit(byte, 1);
      break;
  }
}

void Bin2CPrintChar(std::ostream& sout, bool printSigned, Bin2CBase base,
                    bool printFirstNewline, std::string& line,
                    unsigned char byte, std::uint64_t& pos, bool& any)
{
  if (!any) {
    if (printFirstNewline) {
      sout << "\n";
    }
  } else if (pos % BIN2C_ROW_WIDTH == 0) {
    sout << line;
  }
  Bin2CPrintCharToBuffer(printSigned, base, line, byte, pos);
  any = true;
  pos++;
}

std::function<std::ostream&(std::ostream& sout)> Bin2CPrintChars(
  std::istream& sin, bool printTrailingComma, bool printSigned, Bin2CBase base,
  bool printFirstNewline, std::uint64_t* length = nullptr)
{
  return [&sin, printTrailingComma, printSigned, base, printFirstNewline,
          length](std::ostream& sout) -> std::ostream& {
    // Construct a line buffer and modify the characters within it. This is
    // an order of magnitude faster than `sout <<`-ing everything.
    std::string line = " ";
    line.reserve(
      cmStrLen(" ") +
      (base == Bin2CBase::Hex ? cmStrLen(" 0x__,") : cmStrLen("____,")) *
        BIN2C_ROW_WIDTH +
      cmStrLen("\n"));
    for (std::size_t i = 0; i < BIN2C_ROW_WIDTH; i++) {
      line =
        cmStrCat(std::move(line), base == Bin2CBase::Hex ? " 0x__," : "____,");
    }
    line = cmStrCat(std::move(line), '\n');

    bool any = false;
    std::uint64_t pos = 0;
    std::size_t readSize;
    std::vector<std::uint8_t> buffer(BIN2C_BUFFER_SIZE);
    do {
      sin.read(reinterpret_cast<char*>(buffer.data()), BIN2C_BUFFER_SIZE);
      readSize = sin.gcount();
      for (std::size_t i = 0; i < readSize; i++) {
        Bin2CPrintChar(sout, printSigned, base, printFirstNewline, line,
                       buffer[i], pos, any);
      }
    } while (readSize >= BIN2C_BUFFER_SIZE);
    if (any) {
      std::size_t column = pos % BIN2C_ROW_WIDTH;
      if (column == 0) {
        column = BIN2C_ROW_WIDTH;
      }
      sout << line.substr(0,
                          (base == Bin2CBase::Hex
                             ? Bin2CColumnPosHex(column)
                             : Bin2CColumnPosDecimal(column)) -
                            cmStrLen(","))
           << (printTrailingComma ? ",\n" : "\n");
    }

    if (length) {
      *length = pos;
    }
    return sout;
  };
}

bool Bin2CFromTemplateFile(std::ostream& sout, std::istream& sin,
                           Bin2CTemplateFile& templateFile,
                           bool printTrailingComma, bool printSigned,
                           Bin2CBase base)
{
  cm::optional<std::uint64_t> length;
  std::string line;

  bool hasNewline;
  while (cmSystemTools::GetLineFromStream(*templateFile.TemplateStream, line,
                                          &hasNewline)) {
    cm::optional<std::size_t> at;
    for (std::size_t i = 0; i < line.length(); i++) {
      if (line[i] == '@') {
        if (at) {
          cm::string_view variableName{ &line[*at + 1], i - (*at + 1) };
          at.reset();
          if (variableName == templateFile.ArrayPlaceholder) {
            if (length) {
              std::cerr << "Cannot print array twice\n";
              return false;
            }
            length.emplace();
            sout << Bin2CPrintChars(sin, printTrailingComma, printSigned, base,
                                    true, &*length);
          } else if (variableName == templateFile.LengthPlaceholder) {
            if (!length) {
              std::cerr << "Cannot print length before array\n";
              return false;
            }
            sout << *length;
          } else {
            sout << '@' << variableName;
            at = i; // Allow for `@` outside of placeholder expansion
          }
        } else {
          at = i;
        }
      } else if (!at) {
        sout << line[i];
      }
    }
    if (at) {
      sout << &line[*at];
    }
    if (hasNewline) {
      sout << '\n';
    }
  }

  return true;
}

bool Bin2C(std::ostream& sout, std::istream& sin,
           cm::optional<Bin2CTemplateFile>& templateFile,
           bool printTrailingComma, bool printSigned, Bin2CBase base)
{
  if (templateFile) {
    return Bin2CFromTemplateFile(sout, sin, *templateFile, printTrailingComma,
                                 printSigned, base);
  }

  sout << Bin2CPrintChars(sin, printTrailingComma, printSigned, base, false);
  return true;
}

void IssueDeprecation(std::string const& text)
{
  // cmake -E mode does not accept -W options, so do not suggest them.
  cmake cm(cmState::Role::Internal);
  cm.GetMessenger()->SetSuppressionHint(cmMessenger::SuppressionHint::Hide);
  cm.IssueDiagnostic(cmDiagnostics::CMD_DEPRECATED, text);
}

int HandleCoCompileCommands(std::vector<std::string> const& args);
int CMakeCommandUsage(std::string const& program);
int HashSumFile(std::vector<std::string> const& args, cmCryptoHash::Algo algo);
int SymlinkLibrary(std::vector<std::string> const& args);
int SymlinkExecutable(std::vector<std::string> const& args);
cmsys::Status SymlinkInternal(std::string const& file,
                              std::string const& link);
int ExecuteEchoColor(std::vector<std::string> const& args);
int ExecuteLinkScript(std::vector<std::string> const& args);
#if defined(_WIN32) && !defined(__CYGWIN__)
int WindowsCEEnvironment(char const* version, std::string const& name);
#endif
int RunPreprocessor(std::vector<std::string> const& command,
                    std::string const& intermediate_file);
int RunLLVMRC(std::vector<std::string> const& args);
int VisualStudioLink(std::vector<std::string> const& args, int type,
                     cm::optional<cm::StdIo::Console> console);

// called when args[0] == "__run_co_compile"
int HandleCoCompileCommands(std::vector<std::string> const& args)
{
  std::vector<CoCompileJob> jobs;
  std::string sourceFile; // store --source=
  std::string objectFile; // store --object=
  cmList launchers;       // store --launcher=

  // Default is to run the original command found after -- if the option
  // does not need to do that, it should be specified here, currently only
  // lwyu does that.
  bool runOriginalCmd = true;

  std::vector<std::string> orig_cmd;
  bool doing_options = true;
  for (std::string const& arg : cmMakeRange(args).advance(2)) {
    // if the arg is -- then the rest of the args after
    // go into orig_cmd
    if (arg == "--") {
      doing_options = false;
    } else if (doing_options) {
      bool optionFound = false;
      for (CoCompiler const& cc : CoCompilers) {
        size_t optionLen = strlen(cc.Option);
        if (arg.compare(0, optionLen, cc.Option) == 0) {
          optionFound = true;
          CoCompileJob job;
          job.Command = arg.substr(optionLen);
          job.Handler = cc.Handler;
          jobs.push_back(std::move(job));
          if (cc.NoOriginalCommand) {
            runOriginalCmd = false;
          }
        }
      }
      if (!optionFound) {
        if (cmHasLiteralPrefix(arg, "--source=")) {
          sourceFile = arg.substr(9);
        } else if (cmHasLiteralPrefix(arg, "--launcher=")) {
          launchers.append(arg.substr(11), cmList::EmptyElements::Yes);
        } else if (cmHasLiteralPrefix(arg, "--object=")) {
          objectFile = arg.substr(9);
        } else {
          // if it was not a co-compiler or --source/--launcher then error
          std::cerr << "__run_co_compile given unknown argument: " << arg
                    << '\n';
          return 1;
        }
      }
    } else { // if not doing_options then push to orig_cmd
      orig_cmd.push_back(arg);
    }
  }
  if (jobs.empty()) {
    std::cerr << "__run_co_compile missing command to run. "
                 "Looking for one or more of the following:\n";
    for (CoCompiler const& cc : CoCompilers) {
      std::cerr << cc.Option << '\n';
    }
    return 1;
  }

  if (runOriginalCmd && orig_cmd.empty()) {
    std::cerr << "__run_co_compile missing compile command after --\n";
    return 1;
  }

  for (CoCompileJob const& job : jobs) {
    // call the command handler here
    int ret = job.Handler(job.Command, sourceFile, objectFile, orig_cmd);

    // if the command returns non-zero then return and fail.
    // for commands that do not want to break the build, they should return
    // 0 no matter what.
    if (ret != 0) {
      return ret;
    }
  }

  // if there is no original command to run return now
  if (!runOriginalCmd) {
    return 0;
  }

  // Prepend launcher argument(s), if any
  if (!launchers.empty()) {
    orig_cmd.insert(orig_cmd.begin(), launchers.begin(), launchers.end());
  }

  // Now run the real compiler command and return its result value
  int ret;
  if (!cmSystemTools::RunSingleCommand(orig_cmd, nullptr, nullptr, &ret,
                                       nullptr,
                                       cmSystemTools::OUTPUT_PASSTHROUGH)) {
    std::cerr << "Error running '" << orig_cmd[0] << "'\n";
    return 1;
  }
  // return the return value from the original compiler command
  return ret;
}

// Copy file, copy file if different, copy file if newer.
int _cmake_E_copy(Command::Arguments args)
{
  if (args.size() <= 3) {
    return CMakeCommandUsage(args[0]);
  }
  using CopyFn =
    cmsys::SystemTools::CopyStatus (*)(std::string const&, std::string const&);
  CopyFn copyFn;
  std::string copyErrPrefix;
  if (args[1] == "copy") {
    copyFn = cmSystemTools::CopyFileAlways;
    copyErrPrefix = "Error copying file";
  } else if (args[1] == "copy_if_different") {
    copyFn = cmSystemTools::CopyFileIfDifferent;
    copyErrPrefix = "Error copying file (if different) from";
  } else {
    copyFn = cmSystemTools::CopyFileIfNewer;
    copyErrPrefix = "Error copying file (if newer) from";
  }
  std::string const& cmdName = args[1];

  using CommandArgument =
    cmCommandLineArgument<bool(std::string const& value)>;

  cm::optional<std::string> targetArg;
  std::vector<CommandArgument> argParsers{
    { "-t", CommandArgument::Values::One,
      CommandArgument::setToValue(targetArg) },
  };

  std::vector<std::string> files;
  for (decltype(args.size()) i = 2; i < args.size(); i++) {
    std::string const& arg = args[i];
    bool matched = false;
    for (auto const& m : argParsers) {
      if (m.matches(arg)) {
        matched = true;
        if (m.parse(arg, i, args)) {
          break;
        }
        return 1; // failed to parse
      }
    }
    if (!matched) {
      files.push_back(arg);
    }
  }

  // If multiple source files specified,
  // then destination must be directory
  if (files.size() > 2 && !targetArg) {
    targetArg = files.back();
    files.pop_back();
  }
  if (targetArg && (!cmSystemTools::FileIsDirectory(*targetArg))) {
    std::cerr << "Error: Target (for " << cmdName << " command) \""
              << *targetArg << "\" is not a directory.\n";
    return 1;
  }
  if (!targetArg) {
    if (files.size() < 2) {
      std::cerr << "Error: No files or target specified (for " << cmdName
                << " command).\n";
      return 1;
    }
    targetArg = files.back();
    files.pop_back();
  }
  // If error occurs we want to continue copying next files.
  bool return_value = false;
  for (auto const& file : files) {
    cmsys::SystemTools::CopyStatus const status = copyFn(file, *targetArg);
    if (!status) {
      std::cerr << copyErrPrefix << " \"" << file << "\" to \"" << *targetArg
                << "\": " << status.GetString() << '\n';
      return_value = true;
    }
  }
  return return_value;
}
Command constexpr cmake_E_copy{
  "copy"_s,
  "<file>... destination | -t <destination> <file>..."_s,
  "copy files to destination (either file or directory)"_s,
  _cmake_E_copy,
  Group::Common,
};
Command constexpr cmake_E_copy_if_different{
  "copy_if_different"_s,
  "<file>... destination | -t <destination> <file>..."_s,
  "copy files if source has changed"_s,
  _cmake_E_copy,
  Group::Common,
};
Command constexpr cmake_E_copy_if_newer{
  "copy_if_newer"_s,
  "<file>... destination | -t <destination> <file>..."_s,
  "copy files if source is newer than destination"_s,
  _cmake_E_copy,
  Group::Common,
};

// Copy directory contents
int _cmake_E_copy_directory(Command::Arguments args)
{
  cmsys::SystemTools::CopyWhen when =
    cmsys::SystemTools::CopyWhen::Unconditional;
  if (args[1] == "copy_directory_if_different") {
    when = cmsys::SystemTools::CopyWhen::OnlyIfDifferent;
  } else if (args[1] == "copy_directory_if_newer") {
    when = cmsys::SystemTools::CopyWhen::OnlyIfNewer;
  }
  std::string const& cmdName = args[1];

  using CommandArgument =
    cmCommandLineArgument<bool(std::string const& value)>;

  cm::optional<std::string> targetArg;
  std::vector<CommandArgument> argParsers{
    { "-t", CommandArgument::Values::One,
      CommandArgument::setToValue(targetArg) },
  };

  std::vector<std::string> dirs;
  for (decltype(args.size()) i = 2; i < args.size(); i++) {
    std::string const& arg = args[i];
    bool matched = false;
    for (auto const& m : argParsers) {
      if (m.matches(arg)) {
        matched = true;
        if (m.parse(arg, i, args)) {
          break;
        }
        return 1; // failed to parse
      }
    }
    if (!matched) {
      dirs.push_back(arg);
    }
  }

  if (!targetArg) {
    if (dirs.size() < 2) {
      std::cerr << "Error: No directories or target specified (for " << cmdName
                << " command).\n";
      return 1;
    }
    targetArg = dirs.back();
    dirs.pop_back();
  }
  if (dirs.empty()) {
    std::cerr << "Error: No source directories specified (for " << cmdName
              << " command).\n";
    return 1;
  }

  // If error occurs we want to continue copying next files.
  bool return_value = false;
  for (auto const& dir : dirs) {
    cmsys::Status const status =
      cmSystemTools::CopyADirectory(dir, *targetArg, when);
    if (!status) {
      std::cerr << "Error copying directory from \"" << dir << "\" to \""
                << *targetArg << "\": " << status.GetString() << '\n';
      return_value = true;
    }
  }
  return return_value;
}
Command constexpr cmake_E_copy_directory{
  "copy_directory"_s,
  "<dir>... destination | -t <destination> <dir>..."_s,
  "copy content of <dir>... directories to 'destination' directory"_s,
  _cmake_E_copy_directory,
  Group::Common,
};
Command constexpr cmake_E_copy_directory_if_different{
  "copy_directory_if_different"_s,
  "<dir>... destination | -t <destination> <dir>..."_s,
  "copy changed content of <dir>... directories to 'destination' "
  "directory"_s,
  _cmake_E_copy_directory,
  Group::Common,
};
Command constexpr cmake_E_copy_directory_if_newer{
  "copy_directory_if_newer"_s,
  "<dir>... destination | -t <destination> <dir>..."_s,
  "copy newer content of <dir>... directories to 'destination' "
  "directory"_s,
  _cmake_E_copy_directory,
  Group::Common,
};

// Rename a file or directory
int _cmake_E_rename(Command::Arguments args)
{
  if (args.size() != 4) {
    return CMakeCommandUsage(args[0]);
  }
  if (!cmSystemTools::RenameFile(args[2], args[3])) {
    std::string e = cmSystemTools::GetLastSystemError();
    std::cerr << "Error renaming from \"" << args[2] << "\" to \"" << args[3]
              << "\": " << e << "\n";
    return 1;
  }
  return 0;
}
Command constexpr cmake_E_rename{
  "rename"_s,
  "oldname newname"_s,
  "rename a file or directory (on one volume)"_s,
  _cmake_E_rename,
  Group::Common,
};

// Compare files
int _cmake_E_compare_files(Command::Arguments args)
{
  if (args.size() != 4 && args.size() != 5) {
    return CMakeCommandUsage(args[0]);
  }
  bool filesDiffer;
  if (args.size() == 4) {
    filesDiffer = cmSystemTools::FilesDiffer(args[2], args[3]);
  } else if (args[2] == "--ignore-eol") {
    filesDiffer = cmsys::SystemTools::TextFilesDiffer(args[3], args[4]);
  } else {
    CMakeCommandUsage(args[0]);
    return 2;
  }

  if (filesDiffer) {
    return 1;
  }
  return 0;
}
Command constexpr cmake_E_compare_files{
  "compare_files"_s,
  "[--ignore-eol] file1 file2"_s,
  "check if file1 is same as file2"_s,
  _cmake_E_compare_files,
  Group::Common,
};

#if !defined(CMAKE_BOOTSTRAP)
// NOLINTNEXTLINE(bugprone-reserved-identifier)
int _cmake_E___create_def(Command::Arguments args)
{
  if (args.size() < 4) {
    std::cerr << "__create_def Usage: -E __create_def outfile.def "
                 "objlistfile [--nm=nm-path]\n";
    return 1;
  }
  cmsys::ifstream fin(args[3].c_str(), std::ios::in | std::ios::binary);
  if (!fin) {
    std::cerr << "could not open object list file: " << args[3] << '\n';
    return 1;
  }
  std::vector<std::string> defFiles;
  std::vector<std::string> objFiles;
  {
    std::string file;
    cmFileTime outTime;
    bool outValid = outTime.Load(args[2]);
    while (cmSystemTools::GetLineFromStream(fin, file)) {
      std::string const& ext = cmSystemTools::GetFilenameLastExtension(file);
      if (cmSystemTools::LowerCase(ext) == ".def") {
        defFiles.push_back(file);
      } else {
        objFiles.push_back(file);
      }
      if (outValid) {
        cmFileTime inTime;
        outValid = inTime.Load(file) && inTime.Older(outTime);
      }
    }
    if (outValid) {
      // The def file already exists and all input files are older than
      // the existing def file.
      return 0;
    }
  }
  fin.close();
  cmsys::ofstream fout(args[2].c_str());
  if (!fout) {
    std::cerr << "could not open output .def file: " << args[2] << '\n';
    return 1;
  }
  bindexplib deffile;
  if (args.size() >= 5) {
    std::string const& a = args[4];
    if (cmHasLiteralPrefix(a, "--nm=")) {
      deffile.SetNmPath(a.substr(5));
    } else {
      std::cerr << "unknown argument: " << a << '\n';
    }
  }
  if (!deffile.AddDefinitionFile(defFiles) ||
      !deffile.AddObjectFile(objFiles, cmStrCat(args[3], ".rsp"))) {
    return 1;
  }
  if (!deffile.WriteFile(fout)) {
    std::cerr << "could not write output .def file: " << args[2] << '\n';
    return 1;
  }
  return 0;
}
#else
// NOLINTNEXTLINE(bugprone-reserved-identifier)
std::nullptr_t constexpr _cmake_E___create_def = nullptr;
#endif
// NOLINTNEXTLINE(bugprone-reserved-identifier)
Command constexpr cmake_E___create_def{
  "__create_def"_s, ""_s, ""_s, _cmake_E___create_def, Group::Internal,
};

// NOLINTNEXTLINE(bugprone-reserved-identifier)
int _cmake_E___run_co_compile(Command::Arguments args)
{
  return HandleCoCompileCommands(args);
}
// NOLINTNEXTLINE(bugprone-reserved-identifier)
Command constexpr cmake_E___run_co_compile{
  "__run_co_compile"_s, ""_s, ""_s, _cmake_E___run_co_compile, Group::Internal,
};

// Echo string
int _cmake_E_echo(Command::Arguments args)
{
  std::cout << cmJoin(cmMakeRange(args).advance(2), " ") << std::endl;
  return 0;
}
Command constexpr cmake_E_echo{
  "echo"_s,      "[<string>...]"_s, "displays arguments as text"_s,
  _cmake_E_echo, Group::Common,
};

// Echo string no new line
int _cmake_E_echo_append(Command::Arguments args)
{
  std::cout << cmJoin(cmMakeRange(args).advance(2), " ");
  return 0;
}
Command constexpr cmake_E_echo_append{
  "echo_append"_s,
  "[<string>...]"_s,
  "displays arguments as text but no new line"_s,
  _cmake_E_echo_append,
  Group::Common,
};

int _cmake_E_env(Command::Arguments args)
{
#ifndef CMAKE_BOOTSTRAP
  auto envdiff = cmEnvironmentModification{};
#endif

  auto ai = args.cbegin() + 2;
  auto ae = args.cend();
  for (; ai != ae; ++ai) {
    std::string const& a = *ai;
    if (a == "--") {
      // Stop parsing options/environment variables; the next argument
      // should be the command.
      ++ai;
      break;
    }
    if (cmHasLiteralPrefix(a, "--unset=")) {
      // Unset environment variable.
#ifdef CMAKE_BOOTSTRAP
      cmSystemTools::UnPutEnv(a.substr(8));
#else
      envdiff.Add(a.substr(8) + "=unset:");
#endif
    } else if (a == "--modify") {
#ifdef CMAKE_BOOTSTRAP
      std::cerr
        << "cmake -E env: --modify not available during bootstrapping\n";
      return 1;
#else
      if (++ai == ae) {
        std::cerr << "cmake -E env: --modify missing a parameter\n";
        return 1;
      }
      std::string const& op = *ai;
      if (!envdiff.Add(op)) {
        std::cerr << "cmake -E env: invalid parameter to --modify: " << op
                  << '\n';
        return 1;
      }
#endif
    } else if (!a.empty() && a[0] == '-') {
      // Environment variable and command names cannot start in '-',
      // so this must be an unknown option.
      std::cerr << "cmake -E env: unknown option '" << a << "'\n";
      return 1;
    } else if (a.find('=') != std::string::npos) {
      // Set environment variable.
#ifdef CMAKE_BOOTSTRAP
      cmSystemTools::PutEnv(a);
#else
      auto const pos = a.find('=');
      std::string const& name = a.substr(0, pos);
      std::string const& value = a.substr(pos + 1);
      envdiff.Add(cmStrCat(name, "=set:", value));
#endif
    } else {
      // This is the beginning of the command.
      break;
    }
  }

  if (ai == ae) {
    std::cerr << "cmake -E env: no command given\n";
    return 1;
  }

  auto env = cmEnvironment{};
#ifndef CMAKE_BOOTSTRAP
  env.Update(cmSystemTools::GetEnvironmentVariables());
  envdiff.ApplyTo(env);
#endif

  // Execute command from remaining arguments.
  std::vector<std::string> cmd(ai, ae);
  int retval;
  if (cmSystemTools::RunSingleCommand(
        cmd, nullptr, nullptr, &retval, nullptr,
        cmSystemTools::OUTPUT_PASSTHROUGH, cmDuration::zero(),
        cmProcessOutput::Auto, env.GetVariables())) {
    return retval;
  }
  return 1;
}
Command constexpr cmake_E_env{
  "env"_s,
  "[--unset=NAME ...] [NAME=VALUE ...] [--] <command> [<arg>...]"_s,
  "run command in a modified environment"_s,
  _cmake_E_env,
  Group::Common,
};

#if !defined(CMAKE_BOOTSTRAP)
int _cmake_E_environment(Command::Arguments)
{
  for (auto const& env : cmSystemTools::GetEnvironmentVariables()) {
    std::cout << env << std::endl;
  }
  return 0;
}
#else
std::nullptr_t constexpr _cmake_E_environment = nullptr;
#endif
Command constexpr cmake_E_environment{
  "environment"_s,      ""_s,          "display the current environment"_s,
  _cmake_E_environment, Group::Common,
};

int _cmake_E_make_directory(Command::Arguments args)
{
  if (args.size() <= 2) {
    return CMakeCommandUsage(args[0]);
  }
  // If an error occurs, we want to continue making directories.
  bool return_value = false;
  for (auto const& arg : cmMakeRange(args).advance(2)) {
    cmsys::Status const status = cmSystemTools::MakeDirectory(arg);
    if (!status) {
      std::cerr << "Error creating directory \"" << arg
                << "\": " << status.GetString() << '\n';
      return_value = true;
    }
  }
  return return_value;
}
Command constexpr cmake_E_make_directory{
  "make_directory"_s,
  "<dir>..."_s,
  "create parent and <dir> directories"_s,
  _cmake_E_make_directory,
  Group::Common,
};

int _cmake_E_remove_directory(Command::Arguments args)
{
  if (args.size() <= 2) {
    return CMakeCommandUsage(args[0]);
  }

  IssueDeprecation(
    "The 'cmake -E remove_directory' command has been superseded. "
    "Use 'cmake -E rm' instead.");

  // If an error occurs, we want to continue removing directories.
  bool return_value = false;
  for (auto const& arg : cmMakeRange(args).advance(2)) {
    if (cmSystemTools::FileIsDirectory(arg)) {
      if (!cmRemoveDirectory(arg)) {
        return_value = true;
      }
    }
  }
  return return_value;
}
Command constexpr cmake_E_remove_directory{
  "remove_directory"_s,
  "<dir>..."_s,
  "remove directories and their contents (deprecated: use rm instead)"_s,
  _cmake_E_remove_directory,
  Group::Common,
};

// Remove file
int _cmake_E_remove(Command::Arguments args)
{
  if (args.size() <= 2) {
    return CMakeCommandUsage(args[0]);
  }

  IssueDeprecation("The 'cmake -E remove' command has been superseded. "
                   "Use 'cmake -E rm' instead.");

  bool force = false;
  for (auto const& arg : cmMakeRange(args).advance(2)) {
    if (arg == "\\-f" || arg == "-f") {
      force = true;
    } else {
      // Complain if the file could not be removed, still exists,
      // and the -f option was not given.
      if (!cmSystemTools::RemoveFile(arg) && !force &&
          cmSystemTools::FileExists(arg)) {
        return 1;
      }
    }
  }
  return 0;
}
Command constexpr cmake_E_remove{
  "remove"_s,
  "[-f] <file>..."_s,
  "remove the file(s), use -f to force it (deprecated: use rm instead)"_s,
  _cmake_E_remove,
  Group::Common,
};

// Remove directories or files with rm
int _cmake_E_rm(Command::Arguments args)
{
  if (args.size() <= 2) {
    return CMakeCommandUsage(args[0]);
  }
  // If an error occurs, we want to continue removing the remaining
  // files/directories.
  int return_value = 0;
  bool force = false;
  bool recursive = false;
  bool doing_options = true;
  bool at_least_one_file = false;
  for (auto const& arg : cmMakeRange(args).advance(2)) {
    if (doing_options && cmHasPrefix(arg, '-')) {
      if (arg == "--") {
        doing_options = false;
      }
      if (arg.find('f') != std::string::npos) {
        force = true;
      }
      if (arg.find_first_of("rR") != std::string::npos) {
        recursive = true;
      }
      if (arg.find_first_not_of("-frR") != std::string::npos) {
        cmSystemTools::Error("Unknown -E rm argument: " + arg);
        return 1;
      }
    } else {
      if (arg.empty()) {
        continue;
      }
      at_least_one_file = true;
      // Complain if the -f option was not given and
      // either file does not exist or
      // file could not be removed and still exists
      bool file_exists_or_forced_remove =
        cmSystemTools::PathExists(arg) || force;
      if (cmSystemTools::FileIsDirectory(arg)) {
        if (!cmRemoveDirectory(arg, recursive)) {
          return_value = 1;
        }
      } else if ((!file_exists_or_forced_remove) ||
                 (!cmSystemTools::RemoveFile(arg) &&
                  cmSystemTools::FileExists(arg))) {
        if (!file_exists_or_forced_remove) {
          cmSystemTools::Error(
            "File to remove does not exist and force is not set: " + arg);
        } else {
          cmSystemTools::Error("File can't be removed and still exists: " +
                               arg);
        }
        return_value = 1;
      }
    }
  }
  if (!at_least_one_file) {
    cmSystemTools::Error("Missing file/directory to remove");
    return 1;
  }
  return return_value;
}
Command constexpr cmake_E_rm{
  "rm"_s,
  "[-rRf] [--] <file/dir>..."_s,
  "remove files or directories, use -f to force it, r or R to remove "
  "directories and their contents recursively"_s,
  _cmake_E_rm,
  Group::Common,
};

// Touch file
int _cmake_E_touch(Command::Arguments args)
{
  if (args.size() <= 2) {
    return CMakeCommandUsage(args[0]);
  }
  for (auto const& arg : cmMakeRange(args).advance(2)) {
    if (!cmSystemTools::Touch(arg, true)) {
      std::cerr << "cmake -E touch: failed to update \"" << arg << "\".\n";
      return 1;
    }
  }
  return 0;
}
Command constexpr cmake_E_touch{
  "touch"_s, "<file>..."_s, "touch a <file>."_s, _cmake_E_touch, Group::Common,
};

// Touch file
int _cmake_E_touch_nocreate(Command::Arguments args)
{
  if (args.size() <= 2) {
    return CMakeCommandUsage(args[0]);
  }
  for (auto const& arg : cmMakeRange(args).advance(2)) {
    if (!cmSystemTools::Touch(arg, false)) {
      std::cerr << "cmake -E touch_nocreate: failed to update \"" << arg
                << "\".\n";
      return 1;
    }
  }
  return 0;
}
Command constexpr cmake_E_touch_nocreate{
  "touch_nocreate"_s,
  "<file>..."_s,
  "touch a <file> but do not create it."_s,
  _cmake_E_touch_nocreate,
  Group::Common,
};

// capabilities
int _cmake_E_capabilities(Command::Arguments args)
{
  if (args.size() > 2) {
    std::cerr << "-E capabilities accepts no additional arguments\n";
    return 1;
  }
  cmake cm(cmState::Role::Internal);
  std::cout << cm.ReportCapabilities();
  return 0;
}
Command constexpr cmake_E_capabilities{
  "capabilities"_s,
  ""_s,
  "Report capabilities built into cmake in JSON format"_s,
  _cmake_E_capabilities,
  Group::Common,
};

// Sleep command
int _cmake_E_sleep(Command::Arguments args)
{
  if (args.size() <= 2) {
    return CMakeCommandUsage(args[0]);
  }
  double total = 0;
  for (auto const& arg : cmMakeRange(args).advance(2)) {
    double num = 0.0;
    char unit;
    char extra;
    int n = sscanf(arg.c_str(), "%lg%c%c", &num, &unit, &extra);
    if ((n == 1 || (n == 2 && unit == 's')) && num >= 0) {
      total += num;
    } else {
      std::cerr << "Unknown sleep time format \"" << arg << "\".\n";
      return 1;
    }
  }
  if (total > 0) {
    cmSystemTools::Delay(static_cast<unsigned int>(total * 1000));
  }
  return 0;
}
Command constexpr cmake_E_sleep{
  "sleep"_s,      "<number>..."_s, "sleep for given number of seconds"_s,
  _cmake_E_sleep, Group::Common,
};

// Clock command
int _cmake_E_time(Command::Arguments args)
{
  if (args.size() <= 2) {
    return CMakeCommandUsage(args[0]);
  }
  std::vector<std::string> command(args.begin() + 2, args.end());

  int ret = 0;
  auto time_start = std::chrono::steady_clock::now();
  cmSystemTools::RunSingleCommand(command, nullptr, nullptr, &ret, nullptr,
                                  cmSystemTools::OUTPUT_PASSTHROUGH);
  auto time_finish = std::chrono::steady_clock::now();

  std::chrono::duration<double> time_elapsed = time_finish - time_start;
  std::cout << "Elapsed time (seconds): " << time_elapsed.count() << '\n';
  return ret;
}
Command constexpr cmake_E_time{
  "time"_s,
  "command [args...]"_s,
  "run command and display elapsed time"_s,
  _cmake_E_time,
  Group::Common,
};

// Command to calculate the md5sum of a file
int _cmake_E_md5sum(Command::Arguments args)
{
  if (args.size() < 3) {
    return CMakeCommandUsage(args[0]);
  }
  return HashSumFile(args, cmCryptoHash::AlgoMD5);
}
Command constexpr cmake_E_md5sum{
  "md5sum"_s,      "<file>..."_s, "create MD5 checksum of files"_s,
  _cmake_E_md5sum, Group::Common,
};

// Command to calculate the sha1sum of a file
int _cmake_E_sha1sum(Command::Arguments args)
{
  if (args.size() < 3) {
    return CMakeCommandUsage(args[0]);
  }
  return HashSumFile(args, cmCryptoHash::AlgoSHA1);
}
Command constexpr cmake_E_sha1sum{
  "sha1sum"_s,      "<file>..."_s, "create SHA1 checksum of files"_s,
  _cmake_E_sha1sum, Group::Common,
};

int _cmake_E_sha224sum(Command::Arguments args)
{
  if (args.size() < 3) {
    return CMakeCommandUsage(args[0]);
  }
  return HashSumFile(args, cmCryptoHash::AlgoSHA224);
}
Command constexpr cmake_E_sha224sum{
  "sha224sum"_s,      "<file>..."_s, "create SHA224 checksum of files"_s,
  _cmake_E_sha224sum, Group::Common,
};

int _cmake_E_sha256sum(Command::Arguments args)
{
  if (args.size() < 3) {
    return CMakeCommandUsage(args[0]);
  }
  return HashSumFile(args, cmCryptoHash::AlgoSHA256);
}
Command constexpr cmake_E_sha256sum{
  "sha256sum"_s,      "<file>..."_s, "create SHA256 checksum of files"_s,
  _cmake_E_sha256sum, Group::Common,
};

int _cmake_E_sha384sum(Command::Arguments args)
{
  if (args.size() < 3) {
    return CMakeCommandUsage(args[0]);
  }
  return HashSumFile(args, cmCryptoHash::AlgoSHA384);
}
Command constexpr cmake_E_sha384sum{
  "sha384sum"_s,      "<file>..."_s, "create SHA384 checksum of files"_s,
  _cmake_E_sha384sum, Group::Common,
};

int _cmake_E_sha512sum(Command::Arguments args)
{
  if (args.size() < 3) {
    return CMakeCommandUsage(args[0]);
  }
  return HashSumFile(args, cmCryptoHash::AlgoSHA512);
}
Command constexpr cmake_E_sha512sum{
  "sha512sum"_s,      "<file>..."_s, "create SHA512 checksum of files"_s,
  _cmake_E_sha512sum, Group::Common,
};

// Command to concat files into one
int _cmake_E_cat(Command::Arguments args)
{
  if (args.size() == 2) {
    // Destroy console buffers to drop cout/cerr encoding transform.
    args.Console.reset();
    cmCatFile(cm::nullopt);
    return 0;
  }
  int return_value = 0;
  bool doing_options = true;
  for (auto const& arg : cmMakeRange(args).advance(2)) {
    if (arg == "-") {
      doing_options = false;
      // Destroy console buffers to drop cout/cerr encoding transform.
      args.Console.reset();
      cmCatFile(cm::nullopt);
    } else if (doing_options && cmHasPrefix(arg, '-')) {
      if (arg == "--") {
        doing_options = false;
      } else {
        cmSystemTools::Error(arg + ": option not handled");
        return_value = 1;
      }
    } else if (!cmSystemTools::TestFileAccess(arg, cmsys::TEST_FILE_READ) &&
               cmSystemTools::TestFileAccess(arg, cmsys::TEST_FILE_OK)) {
      cmSystemTools::Error(arg + ": permission denied (ignoring)");
      return_value = 1;
    } else if (cmSystemTools::FileIsDirectory(arg)) {
      cmSystemTools::Error(arg + ": is a directory (ignoring)");
      return_value = 1;
    } else if (!cmSystemTools::FileExists(arg)) {
      cmSystemTools::Error(arg + ": no such file or directory (ignoring)");
      return_value = 1;
    } else if (cmSystemTools::FileLength(arg) == 0) {
      // Ignore empty files, this is not an error
    } else {
      // Destroy console buffers to drop cout/cerr encoding transform.
      args.Console.reset();
      cmCatFile(arg);
    }
  }
  return return_value;
}
Command constexpr cmake_E_cat{
  "cat"_s,
  "[--] <files>..."_s,
  "concat the files and print them to the standard output"_s,
  _cmake_E_cat,
  Group::Common,
};

// Command to change directory and run a program.
int _cmake_E_chdir(Command::Arguments args)
{
  if (args.size() < 4) {
    return CMakeCommandUsage(args[0]);
  }
  std::string const& directory = args[2];
  if (!cmSystemTools::FileExists(directory)) {
    cmSystemTools::Error("Directory does not exist for chdir command: " +
                         directory);
    return 1;
  }

  std::string command = cmWrap('"', cmMakeRange(args).advance(3), '"', " ");
  int retval = 0;
  if (cmSystemTools::RunSingleCommand(
        command, nullptr, nullptr, &retval, directory.c_str(),
        cmSystemTools::OUTPUT_PASSTHROUGH, cmDuration::zero())) {
    return retval;
  }

  return 1;
}
Command constexpr cmake_E_chdir{
  "chdir"_s,      "dir cmd [args...]"_s, "run command in a given directory"_s,
  _cmake_E_chdir, Group::Common,
};

// Command to start progress for a build
int _cmake_E_cmake_progress_start(Command::Arguments args)
{
  if (args.size() != 4) {
    return CMakeCommandUsage(args[0]);
  }
  // basically remove the directory
  std::string dirName = cmStrCat(args[2], "/Progress");
  cmSystemTools::RemoveADirectory(dirName);

  // is the last argument a filename that exists?
  FILE* countFile = cmsys::SystemTools::Fopen(args[3], "r");
  int count;
  if (countFile) {
    if (1 != fscanf(countFile, "%i", &count)) {
      std::cerr << "Could not read from count file.\n";
    }
    fclose(countFile);
  } else {
    count = atoi(args[3].c_str());
  }
  if (count) {
    cmSystemTools::MakeDirectory(dirName);
    // write the count into the directory
    std::string fName = cmStrCat(dirName, "/count.txt");
    FILE* progFile = cmsys::SystemTools::Fopen(fName, "w");
    if (progFile) {
      fprintf(progFile, "%i\n", count);
      fclose(progFile);
    }
  }
  return 0;
}
Command constexpr cmake_E_cmake_progress_start{
  "cmake_progress_start"_s, ""_s, ""_s, _cmake_E_cmake_progress_start,
  Group::Internal,
};

// Command to report progress for a build
int _cmake_E_cmake_progress_report(Command::Arguments args)
{
  if (args.size() < 3) {
    return CMakeCommandUsage(args[0]);
  }
  // This has been superseded by cmake_echo_color --progress-*
  // options.  We leave it here to avoid errors if somehow this
  // is invoked by an existing makefile without regenerating.
  return 0;
}
Command constexpr cmake_E_cmake_progress_report{
  "cmake_progress_report"_s, ""_s, ""_s, _cmake_E_cmake_progress_report,
  Group::Internal,
};

// Command to create a symbolic link.  Fails on platforms not
// supporting them.
int _cmake_E_create_symlink(Command::Arguments args)
{
  if (args.size() != 4) {
    return CMakeCommandUsage(args[0]);
  }
  std::string const& destinationFileName = args[3];
  if (cmSystemTools::PathExists(destinationFileName) &&
      !cmSystemTools::RemoveFile(destinationFileName)) {
    std::string emsg = cmSystemTools::GetLastSystemError();
    std::cerr << "failed to create symbolic link '" << destinationFileName
              << "' because existing path cannot be removed: " << emsg << '\n';
    return 1;
  }
  if (!cmSystemTools::CreateSymlink(args[2], destinationFileName)) {
    return 1;
  }
  return 0;
}
Command constexpr cmake_E_create_symlink{
  "create_symlink"_s,
  "old new"_s,
  "create a symbolic link new -> old"_s,
  _cmake_E_create_symlink,
  Group::Common,
};

// Command to create a hard link.  Fails on platforms not
// supporting them.
int _cmake_E_create_hardlink(Command::Arguments args)
{
  if (args.size() != 4) {
    return CMakeCommandUsage(args[0]);
  }
  std::string const& sourceFileName = args[2];
  std::string const& destinationFileName = args[3];

  if (!cmSystemTools::FileExists(sourceFileName)) {
    std::cerr << "failed to create hard link because source path '"
              << sourceFileName << "' does not exist\n";
    return 1;
  }

  if (cmSystemTools::PathExists(destinationFileName) &&
      !cmSystemTools::RemoveFile(destinationFileName)) {
    std::string emsg = cmSystemTools::GetLastSystemError();
    std::cerr << "failed to create hard link '" << destinationFileName
              << "' because existing path cannot be removed: " << emsg << '\n';
    return 1;
  }

  if (!cmSystemTools::CreateLink(sourceFileName, destinationFileName)) {
    return 1;
  }
  return 0;
}
Command constexpr cmake_E_create_hardlink{
  "create_hardlink"_s,      "old new"_s,   "create a hard link new -> old"_s,
  _cmake_E_create_hardlink, Group::Common,
};

// Command to do nothing with an exit code of 0.
int _cmake_E_true(Command::Arguments)
{
  return 0;
}
Command constexpr cmake_E_true{
  "true"_s,      ""_s,          "do nothing with an exit code of 0"_s,
  _cmake_E_true, Group::Common,
};

// Command to do nothing with an exit code of 1.
int _cmake_E_false(Command::Arguments)
{
  return 1;
}
Command constexpr cmake_E_false{
  "false"_s,      ""_s,          "do nothing with an exit code of 1"_s,
  _cmake_E_false, Group::Common,
};

// Internal CMake shared library support.
int _cmake_E_cmake_symlink_library(Command::Arguments args)
{
  if (args.size() != 5) {
    return CMakeCommandUsage(args[0]);
  }
  return SymlinkLibrary(args);
}
Command constexpr cmake_E_cmake_symlink_library{
  "cmake_symlink_library"_s, ""_s, ""_s, _cmake_E_cmake_symlink_library,
  Group::Internal,
};

// Internal CMake versioned executable support.
int _cmake_E_cmake_symlink_executable(Command::Arguments args)
{
  if (args.size() != 4) {
    return CMakeCommandUsage(args[0]);
  }
  return SymlinkExecutable(args);
}
Command constexpr cmake_E_cmake_symlink_executable{
  "cmake_symlink_executable"_s,
  ""_s,
  ""_s,
  _cmake_E_cmake_symlink_executable,
  Group::Internal,
};

// Internal CMake dependency scanning support.
// The format is: -E cmake_fastbuild_check_depends <dummy_file>
// <space_separated_list_of_real_outputs>
int _cmake_E_cmake_fastbuild_check_depends(Command::Arguments args)
{
  if (args.size() < 3) {
    return CMakeCommandUsage(args[0]);
  }
  auto const dummyFile = args[2];
  for (auto const& arg : cmMakeRange(args).advance(3)) {
    if (!cmSystemTools::FileExists(arg)) {
      cmSystemTools::RemoveFile(dummyFile);
      return 0;
    }
  }
  return 0;
}
Command constexpr cmake_E_cmake_fastbuild_check_depends{
  "cmake_fastbuild_check_depends"_s,
  ""_s,
  ""_s,
  _cmake_E_cmake_fastbuild_check_depends,
  Group::Internal,
};

int _cmake_E_cmake_depends(Command::Arguments args)
{
  if (args.size() < 6) {
    return CMakeCommandUsage(args[0]);
  }
  bool const verbose = isCMakeVerbose();

  // Create a cmake object instance to process dependencies.
  // All we need is the `set` command.
  cmake cm(cmState::Role::Script);
  std::string gen;
  std::string homeDir;
  std::string startDir;
  std::string homeOutDir;
  std::string startOutDir;
  std::string depInfo;
  std::string targetName;
  bool color = false;
  if (args.size() >= 8) {
    // Full signature:
    //
    //   -E cmake_depends <generator>
    //                    <home-src-dir> <start-src-dir>
    //                    <home-out-dir> <start-out-dir>
    //                    <dep-info> [--color=$(COLOR)]
    //                    <target-name>
    //
    // All paths are provided.
    gen = args[2];
    homeDir = args[3];
    startDir = args[4];
    homeOutDir = args[5];
    startOutDir = args[6];
    depInfo = args[7];
    size_t targetNameIdx = 8;
    if (args.size() >= 9 && cmHasLiteralPrefix(args[8], "--color=")) {
      // Enable or disable color based on the switch value.
      targetNameIdx = 9;
      color = (args[8].size() == 8 || cmIsOn(args[8].substr(8)));
    }
    if (args.size() > targetNameIdx) {
      targetName = args[targetNameIdx];
    } else {
      std::string targetDir = cmSystemTools::GetFilenamePath(depInfo);
      targetDir = cmSystemTools::GetFilenameName(targetDir);
      targetName = targetDir.substr(0, targetDir.size() - 4);
    }
  } else {
    // Support older signature for existing makefiles:
    //
    //   -E cmake_depends <generator>
    //                    <home-out-dir> <start-out-dir>
    //                    <dep-info>
    //
    // Just pretend the source directories are the same as the
    // binary directories so at least scanning will work.
    gen = args[2];
    homeDir = args[3];
    startDir = args[4];
    homeOutDir = args[3];
    startOutDir = args[3];
    depInfo = args[5];
    // Strip the `.dir` suffix. Old CMake always uses this pattern.
    std::string targetDir = cmSystemTools::GetFilenamePath(depInfo);
    targetDir = cmSystemTools::GetFilenameName(targetDir);
    targetName = targetDir.substr(0, targetDir.size() - 4);
  }

  // Create a local generator configured for the directory in
  // which dependencies will be scanned.
  homeDir = cmSystemTools::ToNormalizedPathOnDisk(homeDir);
  startDir = cmSystemTools::ToNormalizedPathOnDisk(startDir);
  homeOutDir = cmSystemTools::ToNormalizedPathOnDisk(homeOutDir);
  startOutDir = cmSystemTools::ToNormalizedPathOnDisk(startOutDir);
  cm.SetHomeDirectory(homeDir);
  cm.SetHomeOutputDirectory(homeOutDir);
  cm.GetCurrentSnapshot().SetDefaultDefinitions();
  if (auto ggd = cm.CreateGlobalGenerator(gen)) {
    cm.SetGlobalGenerator(std::move(ggd));
    cmStateSnapshot snapshot = cm.GetCurrentSnapshot();
    snapshot.GetDirectory().SetCurrentBinary(startOutDir);
    snapshot.GetDirectory().SetCurrentSource(startDir);
    cmMakefile mf(cm.GetGlobalGenerator(), snapshot);
    auto lgd = cm.GetGlobalGenerator()->CreateLocalGenerator(&mf);

    // FIXME: With advanced add_subdirectory usage, these are
    // not necessarily the same as the generator originally used.
    // We should pass all these directories through an info file.
    lgd->SetRelativePathTop(homeDir, homeOutDir);

    // Actually scan dependencies.
    return lgd->UpdateDependencies(depInfo, targetName, verbose, color) ? 0
                                                                        : 2;
  }
  return 1;
}
Command constexpr cmake_E_cmake_depends{
  "cmake_depends"_s, ""_s, ""_s, _cmake_E_cmake_depends, Group::Internal,
};

#if !defined(CMAKE_BOOTSTRAP) || defined(CMAKE_BOOTSTRAP_MAKEFILES)
// Internal CMake compiler dependencies filtering
int _cmake_E_cmake_cl_compile_depends(Command::Arguments args)
{
  return CLCompileAndDependencies(args);
}
#else
std::nullptr_t constexpr _cmake_E_cmake_cl_compile_depends = nullptr;
#endif
Command constexpr cmake_E_cmake_cl_compile_depends{
  "cmake_cl_compile_depends"_s,
  ""_s,
  ""_s,
  _cmake_E_cmake_cl_compile_depends,
  Group::Internal,
};

// Internal CMake link script support.
int _cmake_E_cmake_link_script(Command::Arguments args)
{
  if (args.size() < 3) {
    return CMakeCommandUsage(args[0]);
  }
  return ExecuteLinkScript(args);
}
Command constexpr cmake_E_cmake_link_script{
  "cmake_link_script"_s, ""_s, ""_s, _cmake_E_cmake_link_script,
  Group::Internal,
};

#if !defined(CMAKE_BOOTSTRAP)
// Internal CMake ninja dependency scanning support.
int _cmake_E_cmake_ninja_depends(Command::Arguments args)
{
  return cmcmd_cmake_ninja_depends(args.begin() + 2, args.end());
}

// Internal CMake ninja dyndep support.
int _cmake_E_cmake_ninja_dyndep(Command::Arguments args)
{
  return cmcmd_cmake_ninja_dyndep(args.begin() + 2, args.end());
}
#else
std::nullptr_t constexpr _cmake_E_cmake_ninja_depends = nullptr;
std::nullptr_t constexpr _cmake_E_cmake_ninja_dyndep = nullptr;
#endif
Command constexpr cmake_E_cmake_ninja_depends{
  "cmake_ninja_depends"_s, ""_s, ""_s, _cmake_E_cmake_ninja_depends,
  Group::Internal,
};
Command constexpr cmake_E_cmake_ninja_dyndep{
  "cmake_ninja_dyndep"_s, ""_s, ""_s, _cmake_E_cmake_ninja_dyndep,
  Group::Internal,
};

// Internal CMake C++ module compilation database support.
int _cmake_E_cmake_module_compile_db(Command::Arguments args)
{
  return cmcmd_cmake_module_compile_db(args.begin() + 2, args.end());
}
Command constexpr cmake_E_cmake_module_compile_db{
  "cmake_module_compile_db"_s,
  ""_s,
  ""_s,
  _cmake_E_cmake_module_compile_db,
  Group::Internal,
};

// Internal CMake unimplemented feature notification.
int _cmake_E_cmake_unimplemented_variable(Command::Arguments args)
{
  std::cerr << "Feature not implemented for this platform.";
  if (args.size() == 3) {
    std::cerr << "  Variable " << args[2] << " is not set.";
  }
  std::cerr << '\n';
  return 1;
}
Command constexpr cmake_E_cmake_unimplemented_variable{
  "cmake_unimplemented_variable"_s,
  ""_s,
  ""_s,
  _cmake_E_cmake_unimplemented_variable,
  Group::Internal,
};

int _cmake_E_vs_link_exe(Command::Arguments args)
{
  return VisualStudioLink(args, 1, std::move(args.Console));
}
Command constexpr cmake_E_vs_link_exe{
  "vs_link_exe"_s, ""_s, ""_s, _cmake_E_vs_link_exe, Group::Internal,
};

int _cmake_E_vs_link_dll(Command::Arguments args)
{
  return VisualStudioLink(args, 2, std::move(args.Console));
}
Command constexpr cmake_E_vs_link_dll{
  "vs_link_dll"_s, ""_s, ""_s, _cmake_E_vs_link_dll, Group::Internal,
};

int _cmake_E_cmake_llvm_rc(Command::Arguments args)
{
  return RunLLVMRC(args);
}
Command constexpr cmake_E_cmake_llvm_rc{
  "cmake_llvm_rc"_s, ""_s, ""_s, _cmake_E_cmake_llvm_rc, Group::Internal,
};

// Internal CMake color makefile support.
int _cmake_E_cmake_echo_color(Command::Arguments args)
{
  return ExecuteEchoColor(args);
}
Command constexpr cmake_E_cmake_echo_color{
  "cmake_echo_color"_s, ""_s, ""_s, _cmake_E_cmake_echo_color, Group::Internal,
};

#ifndef CMAKE_BOOTSTRAP
int _cmake_E_cmake_autogen(Command::Arguments args)
{
  if (args.size() < 4) {
    return CMakeCommandUsage(args[0]);
  }
  cm::string_view const infoFile = args[2];
  cm::string_view const config = args[3];
  cm::string_view const executableConfig =
    (args.size() >= 5) ? cm::string_view(args[4]) : cm::string_view();
  return cmQtAutoMocUic(infoFile, config, executableConfig) ? 0 : 1;
}

int _cmake_E_cmake_autorcc(Command::Arguments args)
{
  if (args.size() < 3) {
    return CMakeCommandUsage(args[0]);
  }
  cm::string_view const infoFile = args[2];
  cm::string_view const config =
    (args.size() > 3) ? cm::string_view(args[3]) : cm::string_view();
  cm::string_view const executableConfig =
    (args.size() >= 5) ? cm::string_view(args[4]) : cm::string_view();
  return cmQtAutoRcc(infoFile, config, executableConfig) ? 0 : 1;
}
#else
std::nullptr_t constexpr _cmake_E_cmake_autogen = nullptr;
std::nullptr_t constexpr _cmake_E_cmake_autorcc = nullptr;
#endif
Command constexpr cmake_E_cmake_autogen{
  "cmake_autogen"_s, ""_s, ""_s, _cmake_E_cmake_autogen, Group::Internal,
};
Command constexpr cmake_E_cmake_autorcc{
  "cmake_autorcc"_s, ""_s, ""_s, _cmake_E_cmake_autorcc, Group::Internal,
};

// Tar files
int _cmake_E_tar(Command::Arguments args)
{
  if (args.size() <= 3) {
    return CMakeCommandUsage(args[0]);
  }
  char const* knownFormats[] = {
    "7zip", "gnutar", "pax", "paxr", "raw", "zip"
  };

  std::string const& flags = args[2];
  std::string const& outFile = args[3];
  std::vector<std::string> files;
  std::string mtime;
  std::string format;
  int numThreads = 1;
  int compressionLevel = 0;
  bool compressionLevelFlagPassed = false;
  cmSystemTools::cmTarExtractTimestamps extractTimestamps =
    cmSystemTools::cmTarExtractTimestamps::Yes;
  cmSystemTools::cmTarCompression compress = cmSystemTools::TarCompressAuto;
  int nCompress = 0;
  bool doing_options = true;
  std::string encoding = "UTF-8";
  for (auto const& arg : cmMakeRange(args).advance(4)) {
    if (doing_options && cmHasLiteralPrefix(arg, "--")) {
      if (arg == "--") {
        doing_options = false;
      } else if (arg == "--zstd") {
        compress = cmSystemTools::TarCompressZstd;
        ++nCompress;
      } else if (arg == "--lzma") {
        compress = cmSystemTools::TarCompressLZMA;
        ++nCompress;
      } else if (cmHasLiteralPrefix(arg, "--mtime=")) {
        mtime = arg.substr(8);
      } else if (cmHasLiteralPrefix(arg, "--cmake-tar-threads=")) {
        std::string const& numThreadsStr = arg.substr(20);
        long numThreadsLong = 0;
        if (!cmStrToLong(numThreadsStr, &numThreadsLong)) {
          cmSystemTools::Error(cmStrCat("Invalid --cmake-tar-threads value: '",
                                        numThreadsStr, "' - not a number"));
          return 1;
        }
        if (numThreadsLong >
            std::numeric_limits<decltype(numThreads)>::max()) {
          cmSystemTools::Error(cmStrCat("Invalid --cmake-tar-threads value: '",
                                        numThreadsStr, "' - too large"));
          return 1;
        }
        if (numThreadsLong <
            std::numeric_limits<decltype(numThreads)>::min()) {
          cmSystemTools::Error(cmStrCat("Invalid --cmake-tar-threads value: '",
                                        numThreadsStr, "' - too small"));
          return 1;
        }

        numThreads = static_cast<decltype(numThreads)>(numThreadsLong);
      } else if (cmHasLiteralPrefix(arg, "--cmake-tar-encoding=")) {
        encoding = arg.substr(21);
        if (encoding.empty()) {
          cmSystemTools::Error(
            "Encoding value is empty - it must be filled if passed");
          return 1;
        }
      } else if (cmHasLiteralPrefix(arg, "--cmake-tar-compression-level=")) {
        std::string const& compressionLevelStr = arg.substr(30);
        long compressionLevelLong = 0;
        if (!cmStrToLong(compressionLevelStr, &compressionLevelLong)) {
          cmSystemTools::Error(
            cmStrCat("Invalid --cmake-tar-compression-level value: '",
                     compressionLevelStr, "' - not a number"));
          return 1;
        }
        if (compressionLevelLong >
            std::numeric_limits<decltype(compressionLevel)>::max()) {
          cmSystemTools::Error(
            cmStrCat("Invalid --cmake-tar-compression-level value: '",
                     compressionLevelStr, "' - too large"));
          return 1;
        }
        if (compressionLevelLong <
            std::numeric_limits<decltype(compressionLevel)>::min()) {
          cmSystemTools::Error(
            cmStrCat("Invalid --cmake-tar-compression-level value: '",
                     compressionLevelStr, "' - too small"));
          return 1;
        }

        compressionLevel =
          static_cast<decltype(compressionLevel)>(compressionLevelLong);
        compressionLevelFlagPassed = true;
      } else if (cmHasLiteralPrefix(arg, "--cmake-tar-compression-method=")) {
        std::string const& compressionMethodStr = arg.substr(31);
        if (compressionMethodStr == "none" ||
            compressionMethodStr == "store") {
          compress = cmSystemTools::TarCompressNone;
          ++nCompress;
        } else if (compressionMethodStr == "deflate" ||
                   compressionMethodStr == "gzip") {
          compress = cmSystemTools::TarCompressGZip;
          ++nCompress;
        } else if (compressionMethodStr == "bzip2") {
          compress = cmSystemTools::TarCompressBZip2;
          ++nCompress;
        } else if (compressionMethodStr == "lzma") {
          compress = cmSystemTools::TarCompressLZMA;
          ++nCompress;
        } else if (compressionMethodStr == "lzma2" ||
                   compressionMethodStr == "xz") {
          compress = cmSystemTools::TarCompressXZ;
          ++nCompress;
        } else if (compressionMethodStr == "zstd") {
          compress = cmSystemTools::TarCompressZstd;
          ++nCompress;
        } else if (compressionMethodStr == "ppmd") {
          compress = cmSystemTools::TarCompressPPMd;
          ++nCompress;
        } else {
          cmSystemTools::Error(
            cmStrCat("Unknown --cmake-tar-compression-method value: '",
                     compressionMethodStr, '\''));
          return 1;
        }
      } else if (cmHasLiteralPrefix(arg, "--files-from=")) {
        std::string const& files_from = arg.substr(13);
        if (!cmTarFilesFrom(files_from, files)) {
          return 1;
        }
      } else if (cmHasLiteralPrefix(arg, "--format=")) {
        format = arg.substr(9);
        if (!cm::contains(knownFormats, format)) {
          cmSystemTools::Error("Unknown -E tar --format= argument: " + format);
          return 1;
        }
      } else if (arg == "--touch") {
        extractTimestamps = cmSystemTools::cmTarExtractTimestamps::No;
      } else {
        cmSystemTools::Error("Unknown option to -E tar: " + arg);
        return 1;
      }
    } else {
      files.push_back(arg);
    }
  }
  cmSystemTools::cmTarAction action = cmSystemTools::TarActionNone;
  bool verbose = false;

  for (auto flag : flags) {
    switch (flag) {
      case '-':
      case 'f': {
        // Keep for backward compatibility. Ignored
      } break;
      case 'j': {
        compress = cmSystemTools::TarCompressBZip2;
        ++nCompress;
      } break;
      case 'J': {
        compress = cmSystemTools::TarCompressXZ;
        ++nCompress;
      } break;
      case 'z': {
        compress = cmSystemTools::TarCompressGZip;
        ++nCompress;
      } break;
      case 'v': {
        verbose = true;
      } break;
      case 't': {
        action = cmSystemTools::TarActionList;
      } break;
      case 'c': {
        action = cmSystemTools::TarActionCreate;
      } break;
      case 'x': {
        action = cmSystemTools::TarActionExtract;
      } break;
      default: {
        std::cerr << "tar: Unknown argument: " << flag << '\n';
      }
    }
  }
  if (nCompress > 1) {
    cmSystemTools::Error("Can only compress a tar file one way; "
                         "at most one flag of z, j, or J may be used");
    return 1;
  }
  if (compressionLevelFlagPassed) {
    if (nCompress == 0 && format != "zip" && format != "7zip") {
      cmSystemTools::Error("Can not use --cmake-tar-compression-level "
                           "without compression algorithm selection");
      return 1;
    }

    constexpr int minCompressionLevel = 0;
    int maxCompressionLevel = 9;
    if (compress == cmSystemTools::TarCompressZstd && format != "zip") {
      maxCompressionLevel = 19;
    }

    if (compressionLevel < minCompressionLevel ||
        compressionLevel > maxCompressionLevel) {
      cmSystemTools::Error(
        cmStrCat("Compression level must be between ", minCompressionLevel,
                 " and ", maxCompressionLevel, ". Got ", compressionLevel));
      return 1;
    }
  }

  if (action == cmSystemTools::TarActionList) {
    if (!cmSystemTools::ListTar(outFile, files, {}, encoding, verbose)) {
      cmSystemTools::Error("Problem listing tar: " + outFile);
      return 1;
    }
  } else if (action == cmSystemTools::TarActionCreate) {
    if (files.empty()) {
      std::cerr << "tar: No files or directories specified\n";
    }
    if (!cmSystemTools::CreateTar(outFile, files, {}, {}, compress, encoding,
                                  verbose, mtime, format, compressionLevel,
                                  numThreads)) {
      cmSystemTools::Error(cmStrCat("Problem creating tar:\n  ", outFile));
      return 1;
    }
  } else if (action == cmSystemTools::TarActionExtract) {
    if (!cmSystemTools::ExtractTar(outFile, files, {}, extractTimestamps,
                                   encoding, verbose)) {
      cmSystemTools::Error(cmStrCat("Problem extracting tar:\n  ", outFile));
      return 1;
    }
#ifdef _WIN32
    // OK, on windows 7 after we untar some files,
    // sometimes we can not rename the directory after
    // the untar is done. This breaks the external project
    // untar and rename code.  So, by default we will wait
    // 1/10th of a second after the untar.  If CMAKE_UNTAR_DELAY
    // is set in the env, its value will be used instead of 100.
    int delay = 100;
    std::string delayVar;
    if (cmSystemTools::GetEnv("CMAKE_UNTAR_DELAY", delayVar)) {
      delay = atoi(delayVar.c_str());
    }
    if (delay) {
      cmSystemTools::Delay(delay);
    }
#endif
  } else {
    cmSystemTools::Error("tar: No action specified. Please choose: 't' "
                         "(list), 'c' (create) or 'x' (extract)");
    return 1;
  }
  return 0;
}
Command constexpr cmake_E_tar{
  "tar"_s,
  "[cxt][vf][zjJ] file.tar [file/dir1 file/dir2 ...]"_s,
  "create or extract a tar or zip archive"_s,
  _cmake_E_tar,
  Group::Common,
};

// bin2c
int _cmake_E_bin2c(Command::Arguments args)
{
  auto const usage = []() {
    std::cerr << "bin2c Usage: -E bin2c "
                 "[<options>...] "
                 "[--] [<input-file> [<output-file>]]\n";
    return 1;
  };
  auto const isFilename = [](std::string const& arg) -> bool {
    return arg == "-"_s || !cmHasPrefix(arg, '-');
  };

  static char const validPlaceholderChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                              "abcdefghijklmnopqrstuvwxyz"
                                              "0123456789/_.+-";

  std::string inputFile = "-";
  std::string outputFile = "-";
  cm::optional<cm::string_view> templateFilename;
  cm::optional<cm::string_view> templateArrayPlaceholder;
  cm::optional<cm::string_view> templateLengthPlaceholder;
  bool printTrailingComma = false;
  bool printSigned = false;
  auto base = Bin2CBase::Hex;

  using CommandArgument =
    cmCommandLineArgument<bool(std::string const& value)>;
  std::vector<CommandArgument> arguments = {
    CommandArgument{ "--template-file", CommandArgument::Values::One,
                     [&templateFilename](std::string const& arg) -> bool {
                       templateFilename = arg;
                       return true;
                     } },
    CommandArgument{
      "--template-array-placeholder", CommandArgument::Values::One,
      [&templateArrayPlaceholder](std::string const& arg) -> bool {
        if (arg.find_first_not_of(validPlaceholderChars) !=
            std::string::npos) {
          std::cerr << "Invalid array placeholder name: \"" << arg << "\"\n";
          return false;
        }
        templateArrayPlaceholder = arg;
        return true;
      } },
    CommandArgument{
      "--template-length-placeholder", CommandArgument::Values::One,
      [&templateLengthPlaceholder](std::string const& arg) -> bool {
        if (arg.find_first_not_of(validPlaceholderChars) !=
            std::string::npos) {
          std::cerr << "Invalid length placeholder name: \"" << arg << "\"\n";
          return false;
        }
        templateLengthPlaceholder = arg;
        return true;
      } },
    CommandArgument{ "--trailing-comma", CommandArgument::Values::Zero,
                     CommandArgument::setToTrue(printTrailingComma) },
    CommandArgument{ "--signed", CommandArgument::Values::Zero,
                     CommandArgument::setToTrue(printSigned) },
    CommandArgument{ "--decimal", CommandArgument::Values::Zero,
                     [&base](std::string const&) -> bool {
                       base = Bin2CBase::Decimal;
                       return true;
                     } },
  };

  size_t i;
  for (i = 2; i < args.size(); i++) {
    bool matched = false;
    auto const& arg = args[i];
    if (arg == "--"_s) {
      i++;
      break;
    }
    for (auto const& argument : arguments) {
      if (argument.matches(arg)) {
        matched = true;
        if (!argument.parse(arg, i, args)) {
          std::cerr << "\n";
          return usage();
        }
        break;
      }
    }

    if (isFilename(arg)) {
      break;
    }
    if (!matched) {
      return usage();
    }
  }

  if (i < args.size() - 2) {
    return usage();
  }
  if (i < args.size()) {
    if (!isFilename(args[i])) {
      return usage();
    }
    inputFile = args[i];
    i++;
  }
  if (i < args.size()) {
    if (!isFilename(args[i])) {
      return usage();
    }
    outputFile = args[i];
    i++;
  }

  cm::optional<Bin2CTemplateFile> templateFile;
  cmsys::ifstream templateStream;
  if (templateFilename) {
    templateStream.open(templateFilename->data());
    if (!templateStream) {
      std::cerr << "Could not open template file for reading: \""
                << *templateFilename << "\"\n";
      return 1;
    }
    templateFile =
      Bin2CTemplateFile{ &templateStream,
                         templateArrayPlaceholder.value_or("array"_s),
                         templateLengthPlaceholder.value_or("length"_s) };
  } else {
    if (templateArrayPlaceholder) {
      std::cerr << "Cannot use --template-array-placeholder without "
                   "--template-file\n\n";
      return usage();
    }
    if (templateLengthPlaceholder) {
      std::cerr << "Cannot use --template-length-placeholder without "
                   "--template-file\n\n";
      return usage();
    }
  }

  std::istream* sin = &std::cin;
  cmsys::ifstream fin;
  if (inputFile != "-"_s) {
    fin.open(inputFile.c_str(), std::ios::in | std::ios::binary);
    if (!fin) {
      std::cerr << "Could not open file for reading: \"" << inputFile
                << "\"\n";
      return 1;
    }
    sin = &fin;
#ifdef _WIN32
  } else {
    _setmode(fileno(stdin), _O_BINARY);
#endif
  }

  std::ostream* sout = &std::cout;
  cmGeneratedFileStream fout;
  if (outputFile != "-"_s) {
    fout.Open(outputFile);
    if (!fout) {
      std::cerr << "Could not open file for writing: \"" << outputFile
                << "\"\n";
      return 1;
    }
    fout.SetCopyIfDifferent(true);
    sout = &fout;
  }

  if (!Bin2C(*sout, *sin, templateFile, printTrailingComma, printSigned,
             base)) {
    return 1;
  }

  return 0;
}
Command constexpr cmake_E_bin2c{
  "bin2c"_s,      ""_s,          "Turn a binary file into a C array"_s,
  _cmake_E_bin2c, Group::Common,
};

int _cmake_E_server(Command::Arguments)
{
  cmSystemTools::Error(
    "CMake server mode has been removed in favor of the file-api.");
  return 1;
}
Command constexpr cmake_E_server{
  "server"_s, ""_s, ""_s, _cmake_E_server, Group::Internal,
};

#if !defined(CMAKE_BOOTSTRAP)
// Internal CMake Fortran module support.
int _cmake_E_cmake_copy_f90_mod(Command::Arguments args)
{
  if (args.size() < 4) {
    return CMakeCommandUsage(args[0]);
  }
  return cmDependsFortran::CopyModule(args) ? 0 : 1;
}
#else
std::nullptr_t constexpr _cmake_E_cmake_copy_f90_mod = nullptr;
#endif
Command constexpr cmake_E_cmake_copy_f90_mod{
  "cmake_copy_f90_mod"_s, ""_s, ""_s, _cmake_E_cmake_copy_f90_mod,
  Group::Internal,
};

#if defined(_WIN32) && !defined(__CYGWIN__)
// Write registry value
int _cmake_E_write_regv(Command::Arguments args)
{
  if (args.size() <= 3) {
    return CMakeCommandUsage(args[0]);
  }
  return cmSystemTools::WriteRegistryValue(args[2], args[3]) ? 0 : 1;
}

// Delete registry value
int _cmake_E_delete_regv(Command::Arguments args)
{
  if (args.size() <= 2) {
    return CMakeCommandUsage(args[0]);
  }
  return cmSystemTools::DeleteRegistryValue(args[2]) ? 0 : 1;
}

int _cmake_E_comspec(Command::Arguments args)
{
  if (args.size() <= 2) {
    return CMakeCommandUsage(args[0]);
  }
  std::cerr << "Win9x helper \"cmake -E comspec\" no longer supported\n";
  return 1;
}

int _cmake_E_env_vs8_wince(Command::Arguments args)
{
  if (args.size() != 3) {
    return CMakeCommandUsage(args[0]);
  }
  return WindowsCEEnvironment("8.0", args[2]);
}

int _cmake_E_env_vs9_wince(Command::Arguments args)
{
  if (args.size() != 3) {
    return CMakeCommandUsage(args[0]);
  }
  return WindowsCEEnvironment("9.0", args[2]);
}
#else
std::nullptr_t constexpr _cmake_E_write_regv = nullptr;
std::nullptr_t constexpr _cmake_E_delete_regv = nullptr;
std::nullptr_t constexpr _cmake_E_comspec = nullptr;
std::nullptr_t constexpr _cmake_E_env_vs8_wince = nullptr;
std::nullptr_t constexpr _cmake_E_env_vs9_wince = nullptr;
#endif

Command constexpr cmake_E_write_regv{
  "write_regv"_s,      "key value"_s,  "write registry value"_s,
  _cmake_E_write_regv, Group::Windows,
};
Command constexpr cmake_E_delete_regv{
  "delete_regv"_s,      "key"_s,        "delete registry value"_s,
  _cmake_E_delete_regv, Group::Windows,
};
Command constexpr cmake_E_comspec{
  "comspec"_s, ""_s, ""_s, _cmake_E_comspec, Group::Internal,
};
Command constexpr cmake_E_env_vs8_wince{
  "env_vs8_wince"_s,
  "sdkname"_s,
  "displays a batch file which sets the environment for the provided "
  "Windows CE SDK installed in VS2005"_s,
  _cmake_E_env_vs8_wince,
  Group::Windows,
};
Command constexpr cmake_E_env_vs9_wince{
  "env_vs9_wince"_s,
  "sdkname"_s,
  "displays a batch file which sets the environment for the provided "
  "Windows CE SDK installed in VS2008"_s,
  _cmake_E_env_vs9_wince,
  Group::Windows,
};

// Internal depfile transformation
int _cmake_E_cmake_transform_depfile(Command::Arguments args)
{
  if (args.size() != 10) {
    return CMakeCommandUsage(args[0]);
  }
  auto format = cmDepfileFormat::GccDepfile;
  if (args[3] == "gccdepfile") {
    format = cmDepfileFormat::GccDepfile;
  } else if (args[3] == "makedepfile") {
    format = cmDepfileFormat::MakeDepfile;
  } else if (args[3] == "MSBuildAdditionalInputs") {
    format = cmDepfileFormat::MSBuildAdditionalInputs;
  } else {
    return 1;
  }
  // Create a cmake object instance to process dependencies.
  // All we need is the `set` command.
  cmake cm(cmState::Role::Script);
  std::string homeDir;
  std::string startDir;
  std::string homeOutDir;
  std::string startOutDir;
  homeDir = cmSystemTools::ToNormalizedPathOnDisk(args[4]);
  startDir = cmSystemTools::ToNormalizedPathOnDisk(args[5]);
  homeOutDir = cmSystemTools::ToNormalizedPathOnDisk(args[6]);
  startOutDir = cmSystemTools::ToNormalizedPathOnDisk(args[7]);
  cm.SetHomeDirectory(homeDir);
  cm.SetHomeOutputDirectory(homeOutDir);
  cm.GetCurrentSnapshot().SetDefaultDefinitions();
  if (auto ggd = cm.CreateGlobalGenerator(args[2])) {
    cm.SetGlobalGenerator(std::move(ggd));
    cmStateSnapshot snapshot = cm.GetCurrentSnapshot();
    snapshot.GetDirectory().SetCurrentBinary(startOutDir);
    snapshot.GetDirectory().SetCurrentSource(startDir);
    cmMakefile mf(cm.GetGlobalGenerator(), snapshot);
    auto lgd = cm.GetGlobalGenerator()->CreateLocalGenerator(&mf);

    // FIXME: With advanced add_subdirectory usage, these are
    // not necessarily the same as the generator originally used.
    // We should pass all these directories through an info file.
    lgd->SetRelativePathTop(homeDir, homeOutDir);

    return cmTransformDepfile(format, *lgd, args[8], args[9]) ? 0 : 2;
  }
  return 1;
}
Command constexpr cmake_E_cmake_transform_depfile{
  "cmake_transform_depfile"_s,
  ""_s,
  ""_s,
  _cmake_E_cmake_transform_depfile,
  Group::Internal,
};

// Order by `LC_COLLATE=C sort` to search using `std::lower_bound`.
std::array<Command, 63> const Commands{ {
  cmake_E___create_def,
  cmake_E___run_co_compile,
  cmake_E_bin2c,
  cmake_E_capabilities,
  cmake_E_cat,
  cmake_E_chdir,
  cmake_E_cmake_autogen,
  cmake_E_cmake_autorcc,
  cmake_E_cmake_cl_compile_depends,
  cmake_E_cmake_copy_f90_mod,
  cmake_E_cmake_depends,
  cmake_E_cmake_echo_color,
  cmake_E_cmake_fastbuild_check_depends,
  cmake_E_cmake_link_script,
  cmake_E_cmake_llvm_rc,
  cmake_E_cmake_module_compile_db,
  cmake_E_cmake_ninja_depends,
  cmake_E_cmake_ninja_dyndep,
  cmake_E_cmake_progress_report,
  cmake_E_cmake_progress_start,
  cmake_E_cmake_symlink_executable,
  cmake_E_cmake_symlink_library,
  cmake_E_cmake_transform_depfile,
  cmake_E_cmake_unimplemented_variable,
  cmake_E_compare_files,
  cmake_E_comspec,
  cmake_E_copy,
  cmake_E_copy_directory,
  cmake_E_copy_directory_if_different,
  cmake_E_copy_directory_if_newer,
  cmake_E_copy_if_different,
  cmake_E_copy_if_newer,
  cmake_E_create_hardlink,
  cmake_E_create_symlink,
  cmake_E_delete_regv,
  cmake_E_echo,
  cmake_E_echo_append,
  cmake_E_env,
  cmake_E_env_vs8_wince,
  cmake_E_env_vs9_wince,
  cmake_E_environment,
  cmake_E_false,
  cmake_E_make_directory,
  cmake_E_md5sum,
  cmake_E_remove,
  cmake_E_remove_directory,
  cmake_E_rename,
  cmake_E_rm,
  cmake_E_server,
  cmake_E_sha1sum,
  cmake_E_sha224sum,
  cmake_E_sha256sum,
  cmake_E_sha384sum,
  cmake_E_sha512sum,
  cmake_E_sleep,
  cmake_E_tar,
  cmake_E_time,
  cmake_E_touch,
  cmake_E_touch_nocreate,
  cmake_E_true,
  cmake_E_vs_link_dll,
  cmake_E_vs_link_exe,
  cmake_E_write_regv,
} };

std::vector<std::string> CommandNames()
{
  std::vector<std::string> names;
  names.reserve(Commands.size());
  for (Command const& c : Commands) {
    if (c.Impl) {
      names.emplace_back(c.Name);
    }
  }
  return names;
}

std::string FormatCommandLineHelp(cm::string_view header, Group group)
{
  std::size_t const helpColumn = 28;
  std::string result = cmStrCat(header, ":\n");
  for (Command const& entry : Commands) {
    if (entry.Group != group || !entry.Impl) {
      continue;
    }
    std::string invocation{ entry.Name };
    if (!entry.Usage.empty()) {
      invocation = cmStrCat(std::move(invocation), ' ', entry.Usage);
    }
    std::string line = cmStrCat("  ", invocation);
    if (line.size() + 2 <= helpColumn) {
      line.resize(helpColumn, ' ');
      result = cmStrCat(std::move(result), line, "- ", entry.Help, '\n');
    } else {
      result = cmStrCat(std::move(result), line, '\n',
                        std::string(helpColumn, ' '), "- ", entry.Help, '\n');
    }
  }
  return result;
}

int CMakeCommandUsage(std::string const& program)
{
  /* clang-format off */
  std::string help_screen = cmStrCat(
#ifndef CMAKE_BOOTSTRAP
    "cmake version "
  , cmVersion::GetCMakeVersion()
  , "\n"
#else
    "cmake bootstrap\n"
#endif
    "Usage: "
  , program
  , " -E <command> [arguments...]\n"
  , FormatCommandLineHelp("Available commands", Group::Common)
#if defined(_WIN32) && !defined(__CYGWIN__)
  , FormatCommandLineHelp("Available on Windows only",
                          Group::Windows)
#endif
  );
  /* clang-format on */
  cmSystemTools::Error(help_screen);
  return 1;
}

}

namespace cmcmd {

int ExecuteCMakeCommand(std::vector<std::string> args_,
                        cm::optional<cm::StdIo::Console> console)
{
  Command::Arguments args(std::move(args_), std::move(console));
  if (args.size() > 1) {
    // NOLINTNEXTLINE(readability-qualified-auto)
    auto i = std::lower_bound(Commands.begin(), Commands.end(), args[1]);
    if (i != Commands.end() && i->Name == args[1] && i->Impl) {
      return i->Impl(std::move(args));
    }

    std::string error = cmStrCat("Unknown command \"", args[1], "\".");
    std::string const suggestion =
      cmFindClosestString(args[1], CommandNames());
    if (!suggestion.empty()) {
      error =
        cmStrCat(std::move(error), " Did you mean \"", suggestion, "\"?");
    }
    cmSystemTools::Error(error);
  }

  return CMakeCommandUsage(args[0]);
}

}

namespace {

int HashSumFile(std::vector<std::string> const& args, cmCryptoHash::Algo algo)
{
  if (args.size() < 3) {
    return -1;
  }
  int retval = 0;

  for (auto const& filename : cmMakeRange(args).advance(2)) {
    if (filename == "-") {
#ifdef _WIN32
      _setmode(fileno(stdin), _O_BINARY);
#endif
      cmCryptoHash hasher(algo);
      std::string value = hasher.HashStream(std::cin);
      if (value.empty()) {
        // To mimic "md5sum/shasum" behavior in a shell:
        std::cerr << filename << ": No such file or directory\n";
        retval++;
      } else {
        std::cout << value << "  " << filename << '\n';
      }
    } else if (cmSystemTools::FileIsDirectory(filename)) {
      // Cannot compute sum of a directory
      std::cerr << "Error: " << filename << " is a directory\n";
      retval++;
    } else {
      cmCryptoHash hasher(algo);
      std::string value = hasher.HashFile(filename);
      if (value.empty()) {
        // To mimic "md5sum/shasum" behavior in a shell:
        std::cerr << filename << ": No such file or directory\n";
        retval++;
      } else {
        std::cout << value << "  " << filename << '\n';
      }
    }
  }
  return retval;
}

int SymlinkLibrary(std::vector<std::string> const& args)
{
  int result = 0;
  std::string realName = args[2];
  std::string soName = args[3];
  std::string name = args[4];
  cmSystemTools::ConvertToUnixSlashes(realName);
  cmSystemTools::ConvertToUnixSlashes(soName);
  cmSystemTools::ConvertToUnixSlashes(name);
  if (soName != realName) {
    cmsys::Status status = SymlinkInternal(realName, soName);
    if (!status) {
      cmSystemTools::Error(
        cmStrCat("cmake_symlink_library: System Error: ", status.GetString()));
      result = 1;
    }
  }
  if (name != soName) {
    cmsys::Status status = SymlinkInternal(soName, name);
    if (!status) {
      cmSystemTools::Error(
        cmStrCat("cmake_symlink_library: System Error: ", status.GetString()));
      result = 1;
    }
  }
  return result;
}

int SymlinkExecutable(std::vector<std::string> const& args)
{
  int result = 0;
  std::string const& realName = args[2];
  std::string const& name = args[3];
  if (name != realName) {
    cmsys::Status status = SymlinkInternal(realName, name);
    if (!status) {
      cmSystemTools::Error(cmStrCat("cmake_symlink_executable: System Error: ",
                                    status.GetString()));
      result = 1;
    }
  }
  return result;
}

cmsys::Status SymlinkInternal(std::string const& file, std::string const& link)
{
  if (cmSystemTools::PathExists(link)) {
    cmSystemTools::RemoveFile(link);
  }
  std::string linktext = cmSystemTools::GetFilenameName(file);
#if defined(_WIN32) && !defined(__CYGWIN__)
  cmsys::Status status = cmSystemTools::CreateSymlinkQuietly(linktext, link);
  // Creating a symlink will fail with ERROR_PRIVILEGE_NOT_HELD if the user
  // does not have SeCreateSymbolicLinkPrivilege, or if developer mode is not
  // active. In that case, we try to copy the file.
  if (status.GetWindows() == ERROR_PRIVILEGE_NOT_HELD) {
    status = cmSystemTools::CopyFileAlways(file, link);
  } else if (!status) {
    cmSystemTools::Error(cmStrCat("failed to create symbolic link '", link,
                                  "': ", status.GetString()));
  }
  return status;
#else
  return cmSystemTools::CreateSymlink(linktext, link);
#endif
}

void cmcmdProgressReport(std::string const& dir, std::string const& num)
{
  std::string dirName = cmStrCat(dir, "/Progress");
  std::string fName;
  FILE* progFile;

  // read the count
  fName = cmStrCat(dirName, "/count.txt");
  progFile = cmsys::SystemTools::Fopen(fName, "r");
  int count = 0;
  if (!progFile) {
    return;
  }
  if (1 != fscanf(progFile, "%i", &count)) {
    std::cerr << "Could not read from progress file.\n";
  }
  fclose(progFile);

  char const* last = num.c_str();
  for (char const* c = last;; ++c) {
    if (*c == ',' || *c == '\0') {
      if (c != last) {
        fName = cmStrCat(dirName, '/');
        fName.append(last, c - last);
        progFile = cmsys::SystemTools::Fopen(fName, "w");
        if (progFile) {
          fprintf(progFile, "empty");
          fclose(progFile);
        }
      }
      if (*c == '\0') {
        break;
      }
      last = c + 1;
    }
  }
  int fileNum =
    static_cast<int>(cmsys::Directory::GetNumberOfFilesInDirectory(dirName));
  if (count > 0) {
    int const percent = ((fileNum - 3) * 100) / count;
    std::cout << '[' << std::setw(3) << percent << "%] ";
  }
}

int ExecuteEchoColor(std::vector<std::string> const& args)
{
  // The arguments are
  //   args[0] == <cmake-executable>
  //   args[1] == cmake_echo_color

  bool enabled = true;
  static cm::StdIo::TermAttrSet const noAttrs;
  cm::StdIo::TermAttrSet attrs;
  bool newline = true;
  std::string progressDir;
  for (auto const& arg : cmMakeRange(args).advance(2)) {
    if (cmHasLiteralPrefix(arg, "--switch=")) {
      // Enable or disable color based on the switch value.
      std::string value = arg.substr(9);
      if (!value.empty()) {
        enabled = cmIsOn(value);
      }
    } else if (cmHasLiteralPrefix(arg, "--progress-dir=")) {
      progressDir = arg.substr(15);
    } else if (cmHasLiteralPrefix(arg, "--progress-num=")) {
      if (!progressDir.empty()) {
        std::string const& progressNum = arg.substr(15);
        cmcmdProgressReport(progressDir, progressNum);
      }
    } else if (arg == "--normal") {
      attrs = cm::StdIo::TermAttr::Normal;
    } else if (arg == "--black") {
      attrs = cm::StdIo::TermAttr::ForegroundBlack;
    } else if (arg == "--red") {
      attrs = cm::StdIo::TermAttr::ForegroundRed;
    } else if (arg == "--green") {
      attrs = cm::StdIo::TermAttr::ForegroundGreen;
    } else if (arg == "--yellow") {
      attrs = cm::StdIo::TermAttr::ForegroundYellow;
    } else if (arg == "--blue") {
      attrs = cm::StdIo::TermAttr::ForegroundBlue;
    } else if (arg == "--magenta") {
      attrs = cm::StdIo::TermAttr::ForegroundMagenta;
    } else if (arg == "--cyan") {
      attrs = cm::StdIo::TermAttr::ForegroundCyan;
    } else if (arg == "--white") {
      attrs = cm::StdIo::TermAttr::ForegroundWhite;
    } else if (arg == "--bold") {
      if (attrs.empty()) {
        attrs = cm::StdIo::TermAttr::Normal;
      }
      attrs |= cm::StdIo::TermAttr::ForegroundBold;
    } else if (arg == "--no-newline") {
      newline = false;
    } else if (arg == "--newline") {
      newline = true;
    } else {
      Print(cm::StdIo::Out(), enabled ? attrs : noAttrs, arg);
      if (newline) {
        std::cout << std::endl;
      }
    }
  }

  return 0;
}

int ExecuteLinkScript(std::vector<std::string> const& args)
{
  // The arguments are
  //   args[0] == <cmake-executable>
  //   args[1] == cmake_link_script
  //   args[2] == <link-script-name>
  //   args[3] == --verbose=?
  bool verbose = false;
  if (args.size() >= 4) {
    if (cmHasLiteralPrefix(args[3], "--verbose=")) {
      if (!cmIsOff(args[3].substr(10))) {
        verbose = true;
      }
    }
  }

  // Read command lines from the script.
  cmsys::ifstream fin(args[2].c_str());
  if (!fin) {
    std::cerr << "Error opening link script \"" << args[2] << "\"\n";
    return 1;
  }

  // Run one command at a time.
  std::string command;
  int result = 0;
  while (result == 0 && cmSystemTools::GetLineFromStream(fin, command)) {
    // Skip empty command lines.
    if (command.find_first_not_of(" \t") == std::string::npos) {
      continue;
    }

    // Allocate a process instance.
    cmUVProcessChainBuilder builder;

    // Children should share stdout and stderr with this process.
    builder.SetExternalStream(cmUVProcessChainBuilder::Stream_OUTPUT, stdout)
      .SetExternalStream(cmUVProcessChainBuilder::Stream_ERROR, stderr);

    // Setup this command line.
    std::vector<std::string> args2;
#ifdef _WIN32
    cmSystemTools::ParseWindowsCommandLine(command.c_str(), args2);
#else
    cmSystemTools::ParseUnixCommandLine(command.c_str(), args2);
#endif
    builder.AddCommand(args2);

    // Report the command if verbose output is enabled.
    if (verbose) {
      std::cout << command << '\n';
    }

    // Run the command and wait for it to exit.
    auto chain = builder.Start();
    chain.Wait();

    // Report failure if any.
    auto const& status = chain.GetStatus(0);
    auto exception = status.GetException();
    switch (exception.first) {
      case cmUVProcessChain::ExceptionCode::None:
        if (status.ExitStatus != 0) {
          result = static_cast<int>(status.ExitStatus);
        }
        break;
      case cmUVProcessChain::ExceptionCode::Spawn:
        std::cerr << "Error running link command: " << exception.second;
        result = 2;
        break;
      default:
        std::cerr << "Error running link command: " << exception.second;
        result = 1;
        break;
    }
  }

  // Return the final resulting return value.
  return result;
}

#if defined(_WIN32) && !defined(__CYGWIN__)
int WindowsCEEnvironment(char const* version, std::string const& name)
{
#  if !defined(CMAKE_BOOTSTRAP)
  cmVisualStudioWCEPlatformParser parser(name.c_str());
  parser.ParseVersion(version);
  if (parser.Found()) {
    /* clang-format off */
    std::cout << "@echo off\n"
                 "echo Environment Selection: " << name << "\n"
                 "set PATH=" << parser.GetPathDirectories() << "\n"
                 "set INCLUDE=" << parser.GetIncludeDirectories() << "\n"
                 "set LIB=" << parser.GetLibraryDirectories() << '\n';
    /* clang-format on */
    return 0;
  }
#  else
  (void)version;
#  endif

  std::cerr << "Could not find " << name;
  return -1;
}
#endif

int RunPreprocessor(std::vector<std::string> const& command,
                    std::string const& intermediate_file)
{
  cmUVProcessChainBuilder builder;

  uv_fs_t fs_req;
  int preprocessedFile =
    uv_fs_open(nullptr, &fs_req, intermediate_file.c_str(), O_CREAT | O_RDWR,
               0644, nullptr);
  uv_fs_req_cleanup(&fs_req);

  builder
    .SetExternalStream(cmUVProcessChainBuilder::Stream_OUTPUT,
                       preprocessedFile)
    .SetBuiltinStream(cmUVProcessChainBuilder::Stream_ERROR)
    .AddCommand(command);
  auto process = builder.Start();
  if (!process.Valid() || process.GetStatus(0).SpawnResult != 0) {
    std::cerr << "Failed to start preprocessor.";
    return 1;
  }
  if (!process.Wait()) {
    std::cerr << "Failed to wait for preprocessor";
    return 1;
  }
  if (process.GetStatus(0).ExitStatus != 0) {
    cmUVIStream errorStream(process.ErrorStream());
    std::cerr << errorStream.rdbuf();

    return 1;
  }
  return 0;
}

int RunLLVMRC(std::vector<std::string> const& args)
{
  // The arguments are
  //   args[0] == <cmake-executable>
  //   args[1] == cmake_llvm_rc
  //   args[2] == source_file_path
  //   args[3] == intermediate_file
  //   args[4..n] == preprocess+args
  //   args[n+1] == ++
  //   args[n+2...] == llvm-rc+args
  if (args.size() < 3) {
    std::cerr << "Invalid cmake_llvm_rc arguments";
    return 1;
  }

  std::string const& intermediate_file = args[3];
  std::string const& source_file = args[2];
  std::vector<std::string> preprocess;
  std::vector<std::string> resource_compile;
  std::vector<std::string>* pArgTgt = &preprocess;

  static cmsys::RegularExpression const llvm_rc_only_single_arg("^[-/](N|Y)");
  static cmsys::RegularExpression const llvm_rc_only_double_arg(
    "^[-/](C|LN|L)(.)?");
  static cmsys::RegularExpression const common_double_arg(
    "^[-/](D|U|I|FO|fo|Fo)(.)?"); // noqa: spellcheck disable-line
  bool acceptNextArg = false;
  bool skipNextArg = false;
  for (std::string const& arg : cmMakeRange(args).advance(4)) {
    if (skipNextArg) {
      skipNextArg = false;
      continue;
    }
    // We use ++ as separator between the preprocessing step definition and
    // the rc compilation step because we need to prepend a -- to separate the
    // source file properly from other options when using clang-cl for
    // preprocessing.
    if (arg == "++") {
      pArgTgt = &resource_compile;
      skipNextArg = false;
      acceptNextArg = true;
    } else {
      cmsys::RegularExpressionMatch match;
      if (!acceptNextArg) {
        if (common_double_arg.find(arg.c_str(), match)) {
          acceptNextArg = match.match(2).empty();
        } else {
          if (llvm_rc_only_single_arg.find(arg.c_str(), match)) {
            if (pArgTgt == &preprocess) {
              continue;
            }
          } else if (llvm_rc_only_double_arg.find(arg.c_str(), match)) {
            if (pArgTgt == &preprocess) {
              skipNextArg = match.match(2).empty();
              continue;
            }
            acceptNextArg = match.match(2).empty();
          } else if (pArgTgt == &resource_compile) {
            continue;
          }
        }
      } else {
        acceptNextArg = false;
      }
      if (arg.find("SOURCE_DIR") != std::string::npos) {
        std::string sourceDirArg = arg;
        cmSystemTools::ReplaceString(
          sourceDirArg, "SOURCE_DIR",
          cmSystemTools::GetFilenamePath(source_file));
        pArgTgt->push_back(sourceDirArg);
      } else {
        pArgTgt->push_back(arg);
      }
    }
  }
  if (preprocess.empty()) {
    std::cerr << "Empty preprocessing command";
    return 1;
  }
  if (resource_compile.empty()) {
    std::cerr << "Empty resource compilation command";
    return 1;
  }
  // Since we might have skipped the last argument to llvm-rc
  // we need to make sure the llvm-rc source file is present in the
  // commandline
  if (resource_compile.back() != intermediate_file) {
    resource_compile.push_back(intermediate_file);
  }

  auto result = RunPreprocessor(preprocess, intermediate_file);
  if (result != 0) {
    cmSystemTools::RemoveFile(intermediate_file);
    return result;
  }
  cmUVProcessChainBuilder builder;

  builder.SetBuiltinStream(cmUVProcessChainBuilder::Stream_OUTPUT)
    .SetBuiltinStream(cmUVProcessChainBuilder::Stream_ERROR)
    .AddCommand(resource_compile);
  auto process = builder.Start();
  result = 0;
  if (!process.Valid() || process.GetStatus(0).SpawnResult != 0) {
    std::cerr << "Failed to start resource compiler.";
    result = 1;
  } else {
    if (!process.Wait()) {
      std::cerr << "Failed to wait for resource compiler";
      result = 1;
    }
  }

  cmSystemTools::RemoveFile(intermediate_file);
  if (result != 0) {
    return result;
  }
  if (process.GetStatus(0).ExitStatus != 0) {
    cmUVIStream errorStream(process.ErrorStream());
    std::cerr << errorStream.rdbuf();
    return 1;
  }

  return 0;
}

class cmVSLink
{
  int Type;
  bool Verbose;
  bool Incremental = false;
  bool LinkEmbedsManifest = true;
  bool LinkGeneratesManifest = true;
  std::vector<std::string> LinkCommand;
  std::vector<std::string> UserManifests;
  std::string LinkerManifestFile;
  std::string ManifestFile;
  std::string ManifestFileRC;
  std::string ManifestFileRes;
  std::string TargetFile;
  std::string MtPath;
  std::string RcPath;

public:
  cmVSLink(int type, bool verbose)
    : Type(type)
    , Verbose(verbose)
  {
  }
  bool Parse(std::vector<std::string>::const_iterator argBeg,
             std::vector<std::string>::const_iterator argEnd);
  int Link();

private:
  int LinkIncremental();
  int LinkNonIncremental();
  int RunMT(std::string const& out, bool notify);
};

// For visual studio 2005 and newer manifest files need to be embedded into
// exe and dll's.  This code does that in such a way that incremental linking
// still works.
int VisualStudioLink(std::vector<std::string> const& args, int type,
                     cm::optional<cm::StdIo::Console> console)
{
  // MSVC tools print output in the language specified by the VSLANG
  // environment variable, and encoded in the console output code page.
  // Since vs_link_{exe,dll} just wraps these, pass through that encoding.
  // RunCommand tells RunSingleCommand to *not* convert encoding, so
  // we buffer the output in its original encoding instead of UTF-8.
  // Drop our output encoding conversion so we print with original encoding.
  console.reset();

  if (args.size() < 2) {
    return -1;
  }
  bool const verbose = cmSystemTools::HasEnv("VERBOSE");
  std::vector<std::string> expandedArgs;
  for (std::string const& i : args) {
    // check for nmake temporary files
    if (i[0] == '@' &&
        !(cmHasLiteralPrefix(i, "@CMakeFiles") ||
          cmHasLiteralPrefix(i, "@.o/") || cmHasLiteralPrefix(i, "@.o\\"))) {
      cmsys::ifstream fin(i.substr(1).c_str());
      std::string line;
      while (cmSystemTools::GetLineFromStream(fin, line)) {
        cmSystemTools::ParseWindowsCommandLine(line.c_str(), expandedArgs);
      }
    } else {
      expandedArgs.push_back(i);
    }
  }

  cmVSLink vsLink(type, verbose);
  if (!vsLink.Parse(expandedArgs.begin() + 2, expandedArgs.end())) {
    return -1;
  }
  return vsLink.Link();
}

enum NumberFormat
{
  FORMAT_DECIMAL,
  FORMAT_HEX
};
struct NumberFormatter
{
  NumberFormat Format;
  int Value;
  NumberFormatter(NumberFormat format, int value)
    : Format(format)
    , Value(value)
  {
  }
};
std::ostream& operator<<(std::ostream& stream,
                         NumberFormatter const& formatter)
{
  auto const& flags = stream.flags();
  if (formatter.Format == FORMAT_DECIMAL) {
    stream << std::dec << formatter.Value;
  } else {
    stream << "0x" << std::hex << formatter.Value;
  }
  stream.flags(flags);
  return stream;
}

bool RunCommand(char const* comment, std::vector<std::string> const& command,
                bool verbose, NumberFormat exitFormat,
                int* retCodeOut = nullptr, bool (*retCodeOkay)(int) = nullptr)
{
  // See comment in VisualStudioLink for why we suppress encoding conversion.
  cmProcessOutput::Encoding const encoding = cmProcessOutput::None;

  if (verbose) {
    std::cout << comment << ":\n";
    std::cout << cmJoin(command, " ") << '\n';
  }
  std::string output;
  int retCode = 0;
  bool commandResult = cmSystemTools::RunSingleCommand(
    command, &output, &output, &retCode, nullptr, cmSystemTools::OUTPUT_NONE,
    cmDuration::zero(), encoding);
  bool const retCodeSuccess =
    retCode == 0 || (retCodeOkay && retCodeOkay(retCode));
  bool const success = commandResult && retCodeSuccess;
  if (retCodeOut) {
    if (commandResult || !retCodeSuccess) {
      *retCodeOut = retCode;
    } else {
      *retCodeOut = -1;
    }
  }
  if (!success) {
    std::cout << comment << ": command \"" << cmJoin(command, " ")
              << "\" failed (exit code "
              << NumberFormatter(exitFormat, retCode)
              << ") with the following output:\n"
              << output;
  } else if (verbose) {
    // always print the output of the command, unless
    // it is the dumb rc command banner
    if (output.find("Resource Compiler Version") == std::string::npos) {
      std::cout << output;
    }
  }
  return success;
}

bool cmVSLink::Parse(std::vector<std::string>::const_iterator argBeg,
                     std::vector<std::string>::const_iterator argEnd)
{
  // Parse our own arguments.
  std::string intDir;
  auto arg = argBeg;
  while (arg != argEnd && cmHasPrefix(*arg, '-')) {
    if (*arg == "--") {
      ++arg;
      break;
    }
    if (*arg == "--manifests") {
      for (++arg; arg != argEnd && !cmHasPrefix(*arg, '-'); ++arg) {
        this->UserManifests.push_back(*arg);
      }
    } else if (cmHasLiteralPrefix(*arg, "--intdir=")) {
      intDir = arg->substr(9);
      ++arg;
    } else if (cmHasLiteralPrefix(*arg, "--rc=")) {
      this->RcPath = arg->substr(5);
      ++arg;
    } else if (cmHasLiteralPrefix(*arg, "--mt=")) {
      this->MtPath = arg->substr(5);
      ++arg;
    } else if (cmHasLiteralPrefix(*arg, "--msvc-ver=")) {
      unsigned long msvc_ver = 0;
      if (cmStrToULong(arg->c_str() + 11, &msvc_ver)) {
        this->LinkEmbedsManifest = msvc_ver > 1600;
      }
      ++arg;
    } else {
      std::cerr << "unknown argument '" << *arg << "'\n";
      return false;
    }
  }
  if (intDir.empty()) {
    return false;
  }

  // The rest of the arguments form the link command.
  if (arg == argEnd) {
    return false;
  }
  this->LinkCommand.insert(this->LinkCommand.begin(), arg, argEnd);

  // Parse the link command to extract information we need.
  for (; arg != argEnd; ++arg) {
    if (cmSystemTools::Strucmp(arg->c_str(), "/INCREMENTAL:YES") == 0 ||
        cmSystemTools::Strucmp(arg->c_str(), "-INCREMENTAL:YES") == 0 ||
        cmSystemTools::Strucmp(arg->c_str(), "/INCREMENTAL") == 0 ||
        cmSystemTools::Strucmp(arg->c_str(), "-INCREMENTAL") == 0) {
      this->Incremental = true;
    } else if (cmSystemTools::Strucmp(arg->c_str(), "/INCREMENTAL:NO") == 0 ||
               cmSystemTools::Strucmp(arg->c_str(), "-INCREMENTAL:NO") == 0) {
      this->Incremental = false;
    } else if (cmSystemTools::Strucmp(arg->c_str(), "/MANIFEST:NO") == 0 ||
               cmSystemTools::Strucmp(arg->c_str(), "-MANIFEST:NO") == 0) {
      this->LinkGeneratesManifest = false;
    } else if (cmHasLiteralPrefix(*arg, "/Fe") ||
               cmHasLiteralPrefix(*arg, "-Fe")) {
      this->TargetFile = arg->substr(3);
    } else if (cmHasLiteralPrefix(*arg, "/out:") ||
               cmHasLiteralPrefix(*arg, "-out:")) {
      this->TargetFile = arg->substr(5);
    }
  }

  if (this->TargetFile.empty()) {
    return false;
  }

  this->ManifestFile = intDir + "/embed.manifest";
  this->LinkerManifestFile = intDir + "/intermediate.manifest";

  if (this->Incremental) {
    // We will compile a resource containing the manifest and
    // pass it to the link command.
    this->ManifestFileRC = intDir + "/manifest.rc";
    this->ManifestFileRes = intDir + "/manifest.res";
  }

  if (this->LinkGeneratesManifest &&
      (this->Incremental || !this->LinkEmbedsManifest)) {
    this->LinkCommand.emplace_back("/MANIFEST");
    this->LinkCommand.emplace_back("/MANIFESTFILE:" +
                                   this->LinkerManifestFile);
  }

  return true;
}

int cmVSLink::Link()
{
  if (this->Incremental &&
      (this->LinkGeneratesManifest || !this->UserManifests.empty())) {
    if (this->Verbose) {
      std::cout << "Visual Studio Incremental Link with embedded manifests\n";
    }
    return this->LinkIncremental();
  }
  if (this->Verbose) {
    if (!this->Incremental) {
      std::cout << "Visual Studio Non-Incremental Link\n";
    } else {
      std::cout << "Visual Studio Incremental Link without manifests\n";
    }
  }
  return this->LinkNonIncremental();
}

bool mtRetIsUpdate(int mtRet)
{
  // 'mt /notify_update' returns a special value (differing between
  // Windows and POSIX hosts) when it updated the manifest file.
  return mtRet == 0x41020001 || mtRet == 0xbb;
}

int cmVSLink::LinkIncremental()
{
  // This follows the steps listed here:
  // http://blogs.msdn.com/zakramer/archive/2006/05/22/603558.aspx

  //    1.  Compiler compiles the application and generates the *.obj files.
  //    2.  An empty manifest file is generated if this is a clean build and
  //    if not the previous one is reused.
  //    3.  The resource compiler (rc.exe) compiles the *.manifest file to a
  //    *.res file.
  //    4.  Linker generates the binary (EXE or DLL) with the /incremental
  //    switch and embeds the dummy manifest file. The linker also generates
  //    the real manifest file based on the binaries that your binary depends
  //    on.
  //    5.  The manifest tool (mt.exe) is then used to generate the final
  //    manifest.

  // If the final manifest is changed, then 6 and 7 are run, if not
  // they are skipped, and it is done.

  //    6.  The resource compiler is invoked one more time.
  //    7.  Finally, the Linker does another incremental link, but since the
  //    only thing that has changed is the *.res file that contains the
  //    manifest it is a short link.

  // Create a resource file referencing the manifest.
  std::string absManifestFile =
    cmSystemTools::ToNormalizedPathOnDisk(this->ManifestFile);
  if (this->Verbose) {
    std::cout << "Create " << this->ManifestFileRC << '\n';
  }
  {
    cmsys::ofstream fout(this->ManifestFileRC.c_str());
    if (!fout) {
      return -1;
    }
    // Insert a pragma statement to specify utf-8 encoding.
    fout << "#pragma code_page(65001)\n"
         << this->Type
         << " /* CREATEPROCESS_MANIFEST_RESOURCE_ID */ "
            "24 /* RT_MANIFEST */ \""
         << absManifestFile << "\"";
  }

  // If we have not previously generated a manifest file,
  // generate a manifest file so the resource compiler succeeds.
  if (!cmSystemTools::FileExists(this->ManifestFile)) {
    if (this->Verbose) {
      std::cout << "Create empty: " << this->ManifestFile << '\n';
    }
    if (this->UserManifests.empty()) {
      // generate an empty manifest because there are no user provided
      // manifest files.
      cmsys::ofstream foutTmp(this->ManifestFile.c_str());
    } else {
      this->RunMT("/out:" + this->ManifestFile, false);
    }
  }

  // Compile the resource file.
  std::vector<std::string> rcCommand;
  rcCommand.push_back(this->RcPath.empty() ? "rc" : this->RcPath);
  rcCommand.emplace_back("/fo"); // noqa: spellcheck disable-line
  rcCommand.push_back(this->ManifestFileRes);
  rcCommand.push_back(this->ManifestFileRC);
  if (!RunCommand("RC Pass 1", rcCommand, this->Verbose, FORMAT_DECIMAL)) {
    return -1;
  }

  // Tell the linker to use our manifest compiled into a resource.
  this->LinkCommand.push_back(this->ManifestFileRes);

  // Run the link command (possibly generates intermediate manifest).
  if (!RunCommand("LINK Pass 1", this->LinkCommand, this->Verbose,
                  FORMAT_DECIMAL)) {
    return -1;
  }

  // Run the manifest tool to create the final manifest.
  int mtRet = this->RunMT("/out:" + this->ManifestFile, true);

  // If mt returns a special value then it updated the manifest file so
  // we need to embed it again.  Otherwise we are done.
  if (!mtRetIsUpdate(mtRet)) {
    return mtRet;
  }

  // Compile the resource file again.
  if (!RunCommand("RC Pass 2", rcCommand, this->Verbose, FORMAT_DECIMAL)) {
    return -1;
  }

  // Link incrementally again to use the updated resource.
  if (!RunCommand("FINAL LINK", this->LinkCommand, this->Verbose,
                  FORMAT_DECIMAL)) {
    return -1;
  }
  return 0;
}

int cmVSLink::LinkNonIncremental()
{
  if (!this->LinkEmbedsManifest) {
    // Run the link command (possibly generates intermediate manifest).
    if (!RunCommand("LINK", this->LinkCommand, this->Verbose,
                    FORMAT_DECIMAL)) {
      return -1;
    }

    // If we have no manifest files we are done.
    if (!this->LinkGeneratesManifest && this->UserManifests.empty()) {
      return 0;
    }

    // Run the manifest tool to embed the final manifest in the binary.
    std::string mtOut = cmStrCat("/outputresource:", this->TargetFile,
                                 (this->Type == 1 ? ";#1" : ";#2"));
    return this->RunMT(mtOut, false);
  }

  // The MSVC link tool expects 'rc' to be in the PATH if it needs to embed
  // manifests, but the user might explicitly set 'CMAKE_RC_COMPILER' instead.
  // Add its location as a fallback at the end of PATH.
  if (cmSystemTools::FileIsFullPath(this->RcPath)) {
    std::string rcDir = cmSystemTools::GetFilenamePath(this->RcPath);
#ifdef _WIN32
    std::replace(rcDir.begin(), rcDir.end(), '/', '\\');
    char const pathSep = ';';
#else
    char const pathSep = ':';
#endif
    cm::optional<std::string> path = cmSystemTools::GetEnvVar("PATH");
    if (path) {
      path = cmStrCat(*path, pathSep, rcDir);
    } else {
      path = rcDir;
    }
    cmSystemTools::PutEnv(cmStrCat("PATH=", *path));
  }

  // Sort out any manifests.
  if (this->LinkGeneratesManifest || !this->UserManifests.empty()) {
    std::string opt =
      std::string("/MANIFEST:EMBED,ID=") + (this->Type == 1 ? '1' : '2');
    this->LinkCommand.emplace_back(opt);

    for (auto const& m : this->UserManifests) {
      opt = "/MANIFESTINPUT:" + m;
      this->LinkCommand.emplace_back(opt);
    }
  }

  // Run the link command.
  if (!RunCommand("LINK", this->LinkCommand, this->Verbose, FORMAT_DECIMAL)) {
    return -1;
  }
  return 0;
}

int cmVSLink::RunMT(std::string const& out, bool notify)
{
  std::vector<std::string> mtCommand;
  mtCommand.push_back(this->MtPath.empty() ? "mt" : this->MtPath);
  mtCommand.emplace_back("/nologo");

  // add the linker generated manifest if the file exists.
  if (this->LinkGeneratesManifest &&
      cmSystemTools::FileExists(this->LinkerManifestFile)) {
    mtCommand.emplace_back("/manifest");
    mtCommand.push_back(this->LinkerManifestFile);
  }
  for (auto const& m : this->UserManifests) {
    mtCommand.emplace_back("/manifest");
    mtCommand.push_back(m);
  }
  mtCommand.push_back(out);
  if (notify) {
    // Add an undocumented option that enables a special return
    // code to notify us when the manifest is modified.
    mtCommand.emplace_back("/notify_update");
  }
  int mtRet = 0;
  if (!RunCommand("MT", mtCommand, this->Verbose, FORMAT_HEX, &mtRet,
                  mtRetIsUpdate)) {
    return -1;
  }
  return mtRet;
}

}
