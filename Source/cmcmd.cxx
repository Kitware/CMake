/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmcmd.h"

#include <functional>
#include <iterator>

#include <cm/optional>
#include <cmext/algorithm>

#include <cm3p/uv.h>
#include <fcntl.h>

#include "cmCommandLineArgument.h"
#include "cmConsoleBuf.h"
#include "cmCryptoHash.h"
#include "cmDuration.h"
#include "cmGlobalGenerator.h"
#include "cmList.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmQtAutoMocUic.h"
#include "cmQtAutoRcc.h"
#include "cmRange.h"
#include "cmState.h"
#include "cmStateDirectory.h"
#include "cmStateSnapshot.h"
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
#  include <algorithm>

#  include "cmCMakePath.h"
#  include "cmProcessTools.h"
#endif

#if !defined(CMAKE_BOOTSTRAP) && defined(_WIN32) && !defined(__CYGWIN__)
#  include "cmVisualStudioWCEPlatformParser.h"
#endif

#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <utility>

#ifdef _WIN32
#  include <fcntl.h> // for _O_BINARY
#  include <io.h>    // for _setmode
#endif

#include <cm/string_view>

#include "cmsys/Directory.hxx"
#include "cmsys/FStream.hxx"
#include "cmsys/Process.h"
#include "cmsys/RegularExpression.hxx"
#include "cmsys/Terminal.h"

int cmcmd_cmake_ninja_depends(std::vector<std::string>::const_iterator argBeg,
                              std::vector<std::string>::const_iterator argEnd);
int cmcmd_cmake_ninja_dyndep(std::vector<std::string>::const_iterator argBeg,
                             std::vector<std::string>::const_iterator argEnd);

namespace {
void CMakeCommandUsage(std::string const& program)
{
  std::ostringstream errorStream;

#ifndef CMAKE_BOOTSTRAP
  /* clang-format off */
  errorStream
    << "cmake version " << cmVersion::GetCMakeVersion() << "\n";
/* clang-format on */
#else
  /* clang-format off */
  errorStream
    << "cmake bootstrap\n";
/* clang-format on */
#endif
  // If you add new commands, change here,
  // and in cmakemain.cxx in the options table
  /* clang-format off */
  errorStream
    << "Usage: " << program << " -E <command> [arguments...]\n"
    << "Available commands: \n"
    << "  capabilities              - Report capabilities built into cmake "
       "in JSON format\n"
    << "  cat [--] <files>...       - concat the files and print them to the "
       "standard output\n"
    << "  chdir dir cmd [args...]   - run command in a given directory\n"
    << "  compare_files [--ignore-eol] file1 file2\n"
    << "                              - check if file1 is same as file2\n"
    << "  copy <file>... destination  - copy files to destination "
       "(either file or directory)\n"
    << "  copy_directory <dir>... destination   - copy content of <dir>... "
       "directories to 'destination' directory\n"
    << "  copy_directory_if_different <dir>... destination   - copy changed content of <dir>... "
       "directories to 'destination' directory\n"
    << "  copy_if_different <file>... destination  - copy files if it has "
       "changed\n"
    << "  echo [<string>...]        - displays arguments as text\n"
    << "  echo_append [<string>...] - displays arguments as text but no new "
       "line\n"
    << "  env [--unset=NAME ...] [NAME=VALUE ...] [--] <command> [<arg>...]\n"
    << "                            - run command in a modified environment\n"
    << "  environment               - display the current environment\n"
    << "  make_directory <dir>...   - create parent and <dir> directories\n"
    << "  md5sum <file>...          - create MD5 checksum of files\n"
    << "  sha1sum <file>...         - create SHA1 checksum of files\n"
    << "  sha224sum <file>...       - create SHA224 checksum of files\n"
    << "  sha256sum <file>...       - create SHA256 checksum of files\n"
    << "  sha384sum <file>...       - create SHA384 checksum of files\n"
    << "  sha512sum <file>...       - create SHA512 checksum of files\n"
    << "  remove [-f] <file>...     - remove the file(s), use -f to force "
       "it (deprecated: use rm instead)\n"
    << "  remove_directory <dir>... - remove directories and their contents (deprecated: use rm instead)\n"
    << "  rename oldname newname    - rename a file or directory "
       "(on one volume)\n"
    << "  rm [-rRf] [--] <file/dir>... - remove files or directories, use -f "
       "to force it, r or R to remove directories and their contents "
       "recursively\n"
    << "  sleep <number>...         - sleep for given number of seconds\n"
    << "  tar [cxt][vf][zjJ] file.tar [file/dir1 file/dir2 ...]\n"
    << "                            - create or extract a tar or zip archive\n"
    << "  time command [args...]    - run command and display elapsed time\n"
    << "  touch <file>...           - touch a <file>.\n"
    << "  touch_nocreate <file>...  - touch a <file> but do not create it.\n"
    << "  create_symlink old new    - create a symbolic link new -> old\n"
    << "  create_hardlink old new   - create a hard link new -> old\n"
    << "  true                      - do nothing with an exit code of 0\n"
    << "  false                     - do nothing with an exit code of 1\n"
#if defined(_WIN32) && !defined(__CYGWIN__)
    << "Available on Windows only:\n"
    << "  delete_regv key           - delete registry value\n"
    << "  env_vs8_wince sdkname     - displays a batch file which sets the "
       "environment for the provided Windows CE SDK installed in VS2005\n"
    << "  env_vs9_wince sdkname     - displays a batch file which sets the "
       "environment for the provided Windows CE SDK installed in VS2008\n"
    << "  write_regv key value      - write registry value\n"
#endif
    ;
  /* clang-format on */

  cmSystemTools::Error(errorStream.str());
}

bool cmTarFilesFrom(std::string const& file, std::vector<std::string>& files)
{
  if (cmSystemTools::FileIsDirectory(file)) {
    std::ostringstream e;
    e << "-E tar --files-from= file '" << file << "' is a directory";
    cmSystemTools::Error(e.str());
    return false;
  }
  cmsys::ifstream fin(file.c_str());
  if (!fin) {
    std::ostringstream e;
    e << "-E tar --files-from= file '" << file << "' not found";
    cmSystemTools::Error(e.str());
    return false;
  }
  std::string line;
  while (cmSystemTools::GetLineFromStream(fin, line)) {
    if (line.empty()) {
      continue;
    }
    if (cmHasLiteralPrefix(line, "--add-file=")) {
      files.push_back(line.substr(11));
    } else if (cmHasLiteralPrefix(line, "-")) {
      std::ostringstream e;
      e << "-E tar --files-from='" << file << "' file invalid line:\n"
        << line << "\n";
      cmSystemTools::Error(e.str());
      return false;
    } else {
      files.push_back(line);
    }
  }
  return true;
}

void cmCatFile(const std::string& fileToAppend)
{
#ifdef _WIN32
  _setmode(fileno(stdout), _O_BINARY);
#endif
  cmsys::ifstream source(fileToAppend.c_str(),
                         (std::ios::binary | std::ios::in));
  std::cout << source.rdbuf();
}

bool cmRemoveDirectory(const std::string& dir, bool recursive = true)
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
      this->Output << this->Line << std::endl << std::flush;
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

int CLCompileAndDependencies(const std::vector<std::string>& args)
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

  std::unique_ptr<cmsysProcess, void (*)(cmsysProcess*)> cp(
    cmsysProcess_New(), cmsysProcess_Delete);
  std::vector<const char*> argv(command.size() + 1);
  std::transform(command.begin(), command.end(), argv.begin(),
                 [](std::string const& s) { return s.c_str(); });
  argv.back() = nullptr;
  cmsysProcess_SetCommand(cp.get(), argv.data());
  cmsysProcess_SetWorkingDirectory(cp.get(), currentBinaryDir.c_str());

  cmsys::ofstream fout(depFile.c_str());
  if (!fout) {
    return 3;
  }

  CLIncludeParser includeParser(filterPrefix, fout, std::cout);
  CLOutputLogger errLogger(std::cerr);

  // Start the process.
  cmProcessTools::RunProcess(cp.get(), &includeParser, &errLogger);

  int status = 0;
  // handle status of process
  switch (cmsysProcess_GetState(cp.get())) {
    case cmsysProcess_State_Exited:
      status = cmsysProcess_GetExitValue(cp.get());
      break;
    case cmsysProcess_State_Exception:
      status = 1;
      break;
    case cmsysProcess_State_Error:
      status = 2;
      break;
    default:
      break;
  }

  if (status != 0) {
    // remove the dependencies file because potentially invalid
    fout.close();
    cmSystemTools::RemoveFile(depFile);
  }

  return status;
}
#endif

int HandleIWYU(const std::string& runCmd, const std::string& /* sourceFile */,
               const std::vector<std::string>& orig_cmd)
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
    std::cerr << "Error running '" << iwyu_cmd[0] << "': " << stdErr << "\n";
    return 1;
  }
  // Warn if iwyu reported anything.
  if (stdErr.find("should remove these lines:") != std::string::npos ||
      stdErr.find("should add these lines:") != std::string::npos) {
    std::cerr << "Warning: include-what-you-use reported diagnostics:\n"
              << stdErr << "\n";
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

int HandleTidy(const std::string& runCmd, const std::string& sourceFile,
               const std::vector<std::string>& orig_cmd)
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
    std::cerr << "Error running '" << tidy_cmd[0] << "': " << stdErr << "\n";
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

int HandleLWYU(const std::string& runCmd, const std::string& sourceFile,
               const std::vector<std::string>&)
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
    std::cerr << "Error running '" << lwyu_cmd[0] << "': " << stdErr << "\n";
    return 1;
  }

  // Output the stdout from ldd -r -u to stderr
  // Warn if lwyu reported anything.
  if (stdOut.find("Unused direct dependencies:") != std::string::npos) {
    std::cerr << "Warning: " << stdOut;
  }
  return 0;
}

int HandleCppLint(const std::string& runCmd, const std::string& sourceFile,
                  const std::vector<std::string>&)
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
              << "\n";
    return 1;
  }
  if (!stdOut.empty()) {
    std::cerr << "Warning: cpplint diagnostics:\n";
    // Output the output from cpplint to stderr
    std::cerr << stdOut;
  }

  // always return 0 so the build can continue as cpplint returns non-zero
  // for any warning
  return 0;
}

int HandleCppCheck(const std::string& runCmd, const std::string& sourceFile,
                   const std::vector<std::string>& orig_cmd)
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
              << "\n";
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

using CoCompileHandler = int (*)(const std::string&, const std::string&,
                                 const std::vector<std::string>&);

struct CoCompiler
{
  const char* Option;
  CoCompileHandler Handler;
  bool NoOriginalCommand;
};

const std::array<CoCompiler, 5> CoCompilers = {
  { // Table of options and handlers.
    { "--cppcheck=", HandleCppCheck, false },
    { "--cpplint=", HandleCppLint, false },
    { "--iwyu=", HandleIWYU, false },
    { "--lwyu=", HandleLWYU, true },
    { "--tidy=", HandleTidy, false } }
};

struct CoCompileJob
{
  std::string Command;
  CoCompileHandler Handler;
};
}

// called when args[0] == "__run_co_compile"
int cmcmd::HandleCoCompileCommands(std::vector<std::string> const& args)
{
  std::vector<CoCompileJob> jobs;
  std::string sourceFile; // store --source=
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
        } else {
          // if it was not a co-compiler or --source/--launcher then error
          std::cerr << "__run_co_compile given unknown argument: " << arg
                    << "\n";
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
      std::cerr << cc.Option << "\n";
    }
    return 1;
  }

  if (runOriginalCmd && orig_cmd.empty()) {
    std::cerr << "__run_co_compile missing compile command after --\n";
    return 1;
  }

  for (CoCompileJob const& job : jobs) {
    // call the command handler here
    int ret = job.Handler(job.Command, sourceFile, orig_cmd);

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

int cmcmd::ExecuteCMakeCommand(std::vector<std::string> const& args,
                               std::unique_ptr<cmConsoleBuf> consoleBuf)
{
  // IF YOU ADD A NEW COMMAND, DOCUMENT IT ABOVE and in cmakemain.cxx
  if (args.size() > 1) {
    // Copy file
    if (args[1] == "copy" && args.size() > 3) {
      using CommandArgument =
        cmCommandLineArgument<bool(const std::string& value)>;

      cm::optional<std::string> targetArg;
      std::vector<CommandArgument> argParsers{
        { "-t", CommandArgument::Values::One,
          CommandArgument::setToValue(targetArg) },
      };

      std::vector<std::string> files;
      for (decltype(args.size()) i = 2; i < args.size(); i++) {
        const std::string& arg = args[i];
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
        std::cerr << "Error: Target (for copy command) \"" << *targetArg
                  << "\" is not a directory.\n";
        return 1;
      }
      if (!targetArg) {
        if (files.size() < 2) {
          std::cerr
            << "Error: No files or target specified (for copy command).\n";
          return 1;
        }
        targetArg = files.back();
        files.pop_back();
      }
      // If error occurs we want to continue copying next files.
      bool return_value = false;
      for (auto const& file : files) {
        if (!cmsys::SystemTools::CopyFileAlways(file, *targetArg)) {
          std::cerr << "Error copying file \"" << file << "\" to \""
                    << *targetArg << "\".\n";
          return_value = true;
        }
      }
      return return_value;
    }

    // Copy file if different.
    if (args[1] == "copy_if_different" && args.size() > 3) {
      // If multiple source files specified,
      // then destination must be directory
      if ((args.size() > 4) &&
          (!cmSystemTools::FileIsDirectory(args.back()))) {
        std::cerr << "Error: Target (for copy_if_different command) \""
                  << args.back() << "\" is not a directory.\n";
        return 1;
      }
      // If error occurs we want to continue copying next files.
      bool return_value = false;
      for (auto const& arg : cmMakeRange(args).advance(2).retreat(1)) {
        if (!cmSystemTools::CopyFileIfDifferent(arg, args.back())) {
          std::cerr << "Error copying file (if different) from \"" << arg
                    << "\" to \"" << args.back() << "\".\n";
          return_value = true;
        }
      }
      return return_value;
    }

    // Copy directory contents
    if ((args[1] == "copy_directory" ||
         args[1] == "copy_directory_if_different") &&
        args.size() > 3) {
      // If error occurs we want to continue copying next files.
      bool return_value = false;
      const bool copy_always = (args[1] == "copy_directory");
      for (auto const& arg : cmMakeRange(args).advance(2).retreat(1)) {
        if (!cmSystemTools::CopyADirectory(arg, args.back(), copy_always)) {
          std::cerr << "Error copying directory from \"" << arg << "\" to \""
                    << args.back() << "\".\n";
          return_value = true;
        }
      }
      return return_value;
    }

    // Rename a file or directory
    if (args[1] == "rename" && args.size() == 4) {
      if (!cmSystemTools::RenameFile(args[2], args[3])) {
        std::string e = cmSystemTools::GetLastSystemError();
        std::cerr << "Error renaming from \"" << args[2] << "\" to \""
                  << args[3] << "\": " << e << "\n";
        return 1;
      }
      return 0;
    }

    // Compare files
    if (args[1] == "compare_files" && (args.size() == 4 || args.size() == 5)) {
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

#if !defined(CMAKE_BOOTSTRAP)
    if (args[1] == "__create_def") {
      if (args.size() < 4) {
        std::cerr << "__create_def Usage: -E __create_def outfile.def "
                     "objlistfile [--nm=nm-path]\n";
        return 1;
      }
      cmsys::ifstream fin(args[3].c_str(), std::ios::in | std::ios::binary);
      if (!fin) {
        std::cerr << "could not open object list file: " << args[3] << "\n";
        return 1;
      }
      std::vector<std::string> files;
      {
        std::string file;
        cmFileTime outTime;
        bool outValid = outTime.Load(args[2]);
        while (cmSystemTools::GetLineFromStream(fin, file)) {
          files.push_back(file);
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
      FILE* fout = cmsys::SystemTools::Fopen(args[2], "w+");
      if (!fout) {
        std::cerr << "could not open output .def file: " << args[2] << "\n";
        return 1;
      }
      bindexplib deffile;
      if (args.size() >= 5) {
        std::string const& a = args[4];
        if (cmHasLiteralPrefix(a, "--nm=")) {
          deffile.SetNmPath(a.substr(5));
        } else {
          std::cerr << "unknown argument: " << a << "\n";
        }
      }
      for (std::string const& file : files) {
        std::string const& ext = cmSystemTools::GetFilenameLastExtension(file);
        if (cmSystemTools::LowerCase(ext) == ".def") {
          if (!deffile.AddDefinitionFile(file.c_str())) {
            return 1;
          }
        } else {
          if (!deffile.AddObjectFile(file.c_str())) {
            return 1;
          }
        }
      }
      deffile.WriteFile(fout);
      fclose(fout);
      return 0;
    }
#endif
    if (args[1] == "__run_co_compile") {
      return cmcmd::HandleCoCompileCommands(args);
    }

    // Echo string
    if (args[1] == "echo") {
      std::cout << cmJoin(cmMakeRange(args).advance(2), " ") << std::endl;
      return 0;
    }

    // Echo string no new line
    if (args[1] == "echo_append") {
      std::cout << cmJoin(cmMakeRange(args).advance(2), " ");
      return 0;
    }

    if (args[1] == "env") {
#ifndef CMAKE_BOOTSTRAP
      cmSystemTools::EnvDiff env;
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
          env.UnPutEnv(a.substr(8));
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
          if (!env.ParseOperation(op)) {
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
          env.PutEnv(a);
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

#ifndef CMAKE_BOOTSTRAP
      env.ApplyToCurrentEnv();
#endif

      // Execute command from remaining arguments.
      std::vector<std::string> cmd(ai, ae);
      int retval;
      if (cmSystemTools::RunSingleCommand(cmd, nullptr, nullptr, &retval,
                                          nullptr,
                                          cmSystemTools::OUTPUT_PASSTHROUGH)) {
        return retval;
      }
      return 1;
    }

#if !defined(CMAKE_BOOTSTRAP)
    if (args[1] == "environment") {
      for (auto const& env : cmSystemTools::GetEnvironmentVariables()) {
        std::cout << env << std::endl;
      }
      return 0;
    }
#endif

    if (args[1] == "make_directory" && args.size() > 2) {
      // If an error occurs, we want to continue making directories.
      bool return_value = false;
      for (auto const& arg : cmMakeRange(args).advance(2)) {
        if (!cmSystemTools::MakeDirectory(arg)) {
          std::cerr << "Error creating directory \"" << arg << "\".\n";
          return_value = true;
        }
      }
      return return_value;
    }

    if (args[1] == "remove_directory" && args.size() > 2) {
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

    // Remove file
    if (args[1] == "remove" && args.size() > 2) {
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

    // Remove directories or files with rm
    if (args[1] == "rm" && args.size() > 2) {
      // If an error occurs, we want to continue removing the remaining
      // files/directories.
      int return_value = 0;
      bool force = false;
      bool recursive = false;
      bool doing_options = true;
      bool at_least_one_file = false;
      for (auto const& arg : cmMakeRange(args).advance(2)) {
        if (doing_options && cmHasLiteralPrefix(arg, "-")) {
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
              cmSystemTools::Error("File can't be removed and still exist: " +
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

    // Touch file
    if (args[1] == "touch" && args.size() > 2) {
      for (auto const& arg : cmMakeRange(args).advance(2)) {
        if (!cmSystemTools::Touch(arg, true)) {
          std::cerr << "cmake -E touch: failed to update \"";
          std::cerr << arg << "\".\n";
          return 1;
        }
      }
      return 0;
    }

    // Touch file
    if (args[1] == "touch_nocreate" && args.size() > 2) {
      for (auto const& arg : cmMakeRange(args).advance(2)) {
        if (!cmSystemTools::Touch(arg, false)) {
          std::cerr << "cmake -E touch_nocreate: failed to update \"";
          std::cerr << arg << "\".\n";
          return 1;
        }
      }
      return 0;
    }

    // capabilities
    if (args[1] == "capabilities") {
      if (args.size() > 2) {
        std::cerr << "-E capabilities accepts no additional arguments\n";
        return 1;
      }
      cmake cm(cmake::RoleInternal, cmState::Unknown);
      std::cout << cm.ReportCapabilities();
      return 0;
    }

    // Sleep command
    if (args[1] == "sleep" && args.size() > 2) {
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

    // Clock command
    if (args[1] == "time" && args.size() > 2) {
      std::vector<std::string> command(args.begin() + 2, args.end());

      int ret = 0;
      auto time_start = std::chrono::steady_clock::now();
      cmSystemTools::RunSingleCommand(command, nullptr, nullptr, &ret);
      auto time_finish = std::chrono::steady_clock::now();

      std::chrono::duration<double> time_elapsed = time_finish - time_start;
      std::cout << "Elapsed time (seconds): " << time_elapsed.count() << "\n";
      return ret;
    }

    // Command to calculate the md5sum of a file
    if (args[1] == "md5sum" && args.size() >= 3) {
      return HashSumFile(args, cmCryptoHash::AlgoMD5);
    }

    // Command to calculate the sha1sum of a file
    if (args[1] == "sha1sum" && args.size() >= 3) {
      return HashSumFile(args, cmCryptoHash::AlgoSHA1);
    }

    if (args[1] == "sha224sum" && args.size() >= 3) {
      return HashSumFile(args, cmCryptoHash::AlgoSHA224);
    }

    if (args[1] == "sha256sum" && args.size() >= 3) {
      return HashSumFile(args, cmCryptoHash::AlgoSHA256);
    }

    if (args[1] == "sha384sum" && args.size() >= 3) {
      return HashSumFile(args, cmCryptoHash::AlgoSHA384);
    }

    if (args[1] == "sha512sum" && args.size() >= 3) {
      return HashSumFile(args, cmCryptoHash::AlgoSHA512);
    }

    // Command to concat files into one
    if (args[1] == "cat" && args.size() >= 3) {
      int return_value = 0;
      bool doing_options = true;
      for (auto const& arg : cmMakeRange(args).advance(2)) {
        if (doing_options && cmHasLiteralPrefix(arg, "-")) {
          if (arg == "--") {
            doing_options = false;
          } else {
            cmSystemTools::Error(arg + ": option not handled");
            return_value = 1;
          }
        } else if (!cmSystemTools::TestFileAccess(arg,
                                                  cmsys::TEST_FILE_READ) &&
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
          consoleBuf.reset();
          cmCatFile(arg);
        }
      }
      return return_value;
    }

    // Command to change directory and run a program.
    if (args[1] == "chdir" && args.size() >= 4) {
      std::string const& directory = args[2];
      if (!cmSystemTools::FileExists(directory)) {
        cmSystemTools::Error("Directory does not exist for chdir command: " +
                             directory);
        return 1;
      }

      std::string command =
        cmWrap('"', cmMakeRange(args).advance(3), '"', " ");
      int retval = 0;
      if (cmSystemTools::RunSingleCommand(
            command, nullptr, nullptr, &retval, directory.c_str(),
            cmSystemTools::OUTPUT_PASSTHROUGH, cmDuration::zero())) {
        return retval;
      }

      return 1;
    }

    // Command to start progress for a build
    if (args[1] == "cmake_progress_start" && args.size() == 4) {
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

    // Command to report progress for a build
    if (args[1] == "cmake_progress_report" && args.size() >= 3) {
      // This has been superseded by cmake_echo_color --progress-*
      // options.  We leave it here to avoid errors if somehow this
      // is invoked by an existing makefile without regenerating.
      return 0;
    }

    // Command to create a symbolic link.  Fails on platforms not
    // supporting them.
    if (args[1] == "create_symlink" && args.size() == 4) {
      std::string const& destinationFileName = args[3];
      if (cmSystemTools::PathExists(destinationFileName) &&
          !cmSystemTools::RemoveFile(destinationFileName)) {
        std::string emsg = cmSystemTools::GetLastSystemError();
        std::cerr << "failed to create symbolic link '" << destinationFileName
                  << "' because existing path cannot be removed: " << emsg
                  << "\n";
        return 1;
      }
      if (!cmSystemTools::CreateSymlink(args[2], destinationFileName)) {
        return 1;
      }
      return 0;
    }

    // Command to create a hard link.  Fails on platforms not
    // supporting them.
    if (args[1] == "create_hardlink" && args.size() == 4) {
      std::string const& sourceFileName = args[2];
      std::string const& destinationFileName = args[3];

      if (!cmSystemTools::FileExists(sourceFileName)) {
        std::cerr << "failed to create hard link because source path '"
                  << sourceFileName << "' does not exist \n";
        return 1;
      }

      if (cmSystemTools::PathExists(destinationFileName) &&
          !cmSystemTools::RemoveFile(destinationFileName)) {
        std::string emsg = cmSystemTools::GetLastSystemError();
        std::cerr << "failed to create hard link '" << destinationFileName
                  << "' because existing path cannot be removed: " << emsg
                  << "\n";
        return 1;
      }

      if (!cmSystemTools::CreateLink(sourceFileName, destinationFileName)) {
        return 1;
      }
      return 0;
    }

    // Command to do nothing with an exit code of 0.
    if (args[1] == "true") {
      return 0;
    }

    // Command to do nothing with an exit code of 1.
    if (args[1] == "false") {
      return 1;
    }

    // Internal CMake shared library support.
    if (args[1] == "cmake_symlink_library" && args.size() == 5) {
      return cmcmd::SymlinkLibrary(args);
    }

    // Internal CMake versioned executable support.
    if (args[1] == "cmake_symlink_executable" && args.size() == 4) {
      return cmcmd::SymlinkExecutable(args);
    }

    // Internal CMake dependency scanning support.
    if (args[1] == "cmake_depends" && args.size() >= 6) {
      const bool verbose = isCMakeVerbose();

      // Create a cmake object instance to process dependencies.
      // All we need is the `set` command.
      cmake cm(cmake::RoleScript, cmState::Unknown);
      std::string gen;
      std::string homeDir;
      std::string startDir;
      std::string homeOutDir;
      std::string startOutDir;
      std::string depInfo;
      bool color = false;
      if (args.size() >= 8) {
        // Full signature:
        //
        //   -E cmake_depends <generator>
        //                    <home-src-dir> <start-src-dir>
        //                    <home-out-dir> <start-out-dir>
        //                    <dep-info> [--color=$(COLOR)]
        //
        // All paths are provided.
        gen = args[2];
        homeDir = args[3];
        startDir = args[4];
        homeOutDir = args[5];
        startOutDir = args[6];
        depInfo = args[7];
        if (args.size() >= 9 && cmHasLiteralPrefix(args[8], "--color=")) {
          // Enable or disable color based on the switch value.
          color = (args[8].size() == 8 || cmIsOn(args[8].substr(8)));
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
      }

      // Create a local generator configured for the directory in
      // which dependencies will be scanned.
      homeDir = cmSystemTools::CollapseFullPath(homeDir);
      startDir = cmSystemTools::CollapseFullPath(startDir);
      homeOutDir = cmSystemTools::CollapseFullPath(homeOutDir);
      startOutDir = cmSystemTools::CollapseFullPath(startOutDir);
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
        return lgd->UpdateDependencies(depInfo, verbose, color) ? 0 : 2;
      }
      return 1;
    }

#if !defined(CMAKE_BOOTSTRAP) || defined(CMAKE_BOOTSTRAP_MAKEFILES)
    // Internal CMake compiler dependencies filtering
    if (args[1] == "cmake_cl_compile_depends") {
      return CLCompileAndDependencies(args);
    }
#endif

    // Internal CMake link script support.
    if (args[1] == "cmake_link_script" && args.size() >= 3) {
      return cmcmd::ExecuteLinkScript(args);
    }

#if !defined(CMAKE_BOOTSTRAP)
    // Internal CMake ninja dependency scanning support.
    if (args[1] == "cmake_ninja_depends") {
      return cmcmd_cmake_ninja_depends(args.begin() + 2, args.end());
    }

    // Internal CMake ninja dyndep support.
    if (args[1] == "cmake_ninja_dyndep") {
      return cmcmd_cmake_ninja_dyndep(args.begin() + 2, args.end());
    }
#endif

    // Internal CMake unimplemented feature notification.
    if (args[1] == "cmake_unimplemented_variable") {
      std::cerr << "Feature not implemented for this platform.";
      if (args.size() == 3) {
        std::cerr << "  Variable " << args[2] << " is not set.";
      }
      std::cerr << std::endl;
      return 1;
    }

    if (args[1] == "vs_link_exe") {
      return cmcmd::VisualStudioLink(args, 1);
    }

    if (args[1] == "vs_link_dll") {
      return cmcmd::VisualStudioLink(args, 2);
    }

    if (args[1] == "cmake_llvm_rc") {
      return cmcmd::RunLLVMRC(args);
    }

    // Internal CMake color makefile support.
    if (args[1] == "cmake_echo_color") {
      return cmcmd::ExecuteEchoColor(args);
    }

#ifndef CMAKE_BOOTSTRAP
    if ((args[1] == "cmake_autogen") && (args.size() >= 4)) {
      cm::string_view const infoFile = args[2];
      cm::string_view const config = args[3];
      return cmQtAutoMocUic(infoFile, config) ? 0 : 1;
    }
    if ((args[1] == "cmake_autorcc") && (args.size() >= 3)) {
      cm::string_view const infoFile = args[2];
      cm::string_view const config =
        (args.size() > 3) ? cm::string_view(args[3]) : cm::string_view();
      return cmQtAutoRcc(infoFile, config) ? 0 : 1;
    }
#endif

    // Tar files
    if (args[1] == "tar" && args.size() > 3) {
      const char* knownFormats[] = { "7zip", "gnutar", "pax", "paxr", "zip" };

      std::string const& flags = args[2];
      std::string const& outFile = args[3];
      std::vector<std::string> files;
      std::string mtime;
      std::string format;
      cmSystemTools::cmTarExtractTimestamps extractTimestamps =
        cmSystemTools::cmTarExtractTimestamps::Yes;
      cmSystemTools::cmTarCompression compress =
        cmSystemTools::TarCompressNone;
      int nCompress = 0;
      bool doing_options = true;
      for (auto const& arg : cmMakeRange(args).advance(4)) {
        if (doing_options && cmHasLiteralPrefix(arg, "--")) {
          if (arg == "--") {
            doing_options = false;
          } else if (arg == "--zstd") {
            compress = cmSystemTools::TarCompressZstd;
            ++nCompress;
          } else if (cmHasLiteralPrefix(arg, "--mtime=")) {
            mtime = arg.substr(8);
          } else if (cmHasLiteralPrefix(arg, "--files-from=")) {
            std::string const& files_from = arg.substr(13);
            if (!cmTarFilesFrom(files_from, files)) {
              return 1;
            }
          } else if (cmHasLiteralPrefix(arg, "--format=")) {
            format = arg.substr(9);
            if (!cm::contains(knownFormats, format)) {
              cmSystemTools::Error("Unknown -E tar --format= argument: " +
                                   format);
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
            std::cerr << "tar: Unknown argument: " << flag << "\n";
          }
        }
      }
      if ((format == "7zip" || format == "zip") && nCompress > 0) {
        cmSystemTools::Error("Can not use compression flags with format: " +
                             format);
        return 1;
      }
      if (nCompress > 1) {
        cmSystemTools::Error("Can only compress a tar file one way; "
                             "at most one flag of z, j, or J may be used");
        return 1;
      }
      if (action == cmSystemTools::TarActionList) {
        if (!cmSystemTools::ListTar(outFile, files, verbose)) {
          cmSystemTools::Error("Problem listing tar: " + outFile);
          return 1;
        }
      } else if (action == cmSystemTools::TarActionCreate) {
        if (files.empty()) {
          std::cerr << "tar: No files or directories specified\n";
        }
        if (!cmSystemTools::CreateTar(outFile, files, compress, verbose, mtime,
                                      format)) {
          cmSystemTools::Error("Problem creating tar: " + outFile);
          return 1;
        }
      } else if (action == cmSystemTools::TarActionExtract) {
        if (!cmSystemTools::ExtractTar(outFile, files, extractTimestamps,
                                       verbose)) {
          cmSystemTools::Error("Problem extracting tar: " + outFile);
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

    if (args[1] == "server") {
      cmSystemTools::Error(
        "CMake server mode has been removed in favor of the file-api.");
      return 1;
    }

#if !defined(CMAKE_BOOTSTRAP)
    // Internal CMake Fortran module support.
    if (args[1] == "cmake_copy_f90_mod" && args.size() >= 4) {
      return cmDependsFortran::CopyModule(args) ? 0 : 1;
    }
#endif

#if defined(_WIN32) && !defined(__CYGWIN__)
    // Write registry value
    if (args[1] == "write_regv" && args.size() > 3) {
      return cmSystemTools::WriteRegistryValue(args[2], args[3]) ? 0 : 1;
    }

    // Delete registry value
    if (args[1] == "delete_regv" && args.size() > 2) {
      return cmSystemTools::DeleteRegistryValue(args[2]) ? 0 : 1;
    }

    // Remove file
    if (args[1] == "comspec" && args.size() > 2) {
      std::cerr << "Win9x helper \"cmake -E comspec\" no longer supported\n";
      return 1;
    }

    if (args[1] == "env_vs8_wince" && args.size() == 3) {
      return cmcmd::WindowsCEEnvironment("8.0", args[2]);
    }

    if (args[1] == "env_vs9_wince" && args.size() == 3) {
      return cmcmd::WindowsCEEnvironment("9.0", args[2]);
    }
#endif

    // Internal depfile transformation
    if (args[1] == "cmake_transform_depfile" && args.size() == 10) {
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
      cmake cm(cmake::RoleScript, cmState::Unknown);
      std::string homeDir;
      std::string startDir;
      std::string homeOutDir;
      std::string startOutDir;
      homeDir = cmSystemTools::CollapseFullPath(args[4]);
      startDir = cmSystemTools::CollapseFullPath(args[5]);
      homeOutDir = cmSystemTools::CollapseFullPath(args[6]);
      startOutDir = cmSystemTools::CollapseFullPath(args[7]);
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
  }

  CMakeCommandUsage(args[0]);
  return 1;
}

int cmcmd::HashSumFile(std::vector<std::string> const& args,
                       cmCryptoHash::Algo algo)
{
  if (args.size() < 3) {
    return -1;
  }
  int retval = 0;

  for (auto const& filename : cmMakeRange(args).advance(2)) {
    // Cannot compute sum of a directory
    if (cmSystemTools::FileIsDirectory(filename)) {
      std::cerr << "Error: " << filename << " is a directory" << std::endl;
      retval++;
    } else {
      cmCryptoHash hasher(algo);
      std::string value = hasher.HashFile(filename);
      if (value.empty()) {
        // To mimic "md5sum/shasum" behavior in a shell:
        std::cerr << filename << ": No such file or directory" << std::endl;
        retval++;
      } else {
        std::cout << value << "  " << filename << std::endl;
      }
    }
  }
  return retval;
}

int cmcmd::SymlinkLibrary(std::vector<std::string> const& args)
{
  int result = 0;
  std::string realName = args[2];
  std::string soName = args[3];
  std::string name = args[4];
  cmSystemTools::ConvertToUnixSlashes(realName);
  cmSystemTools::ConvertToUnixSlashes(soName);
  cmSystemTools::ConvertToUnixSlashes(name);
  if (soName != realName) {
    cmsys::Status status = cmcmd::SymlinkInternal(realName, soName);
    if (!status) {
      cmSystemTools::Error(
        cmStrCat("cmake_symlink_library: System Error: ", status.GetString()));
      result = 1;
    }
  }
  if (name != soName) {
    cmsys::Status status = cmcmd::SymlinkInternal(soName, name);
    if (!status) {
      cmSystemTools::Error(
        cmStrCat("cmake_symlink_library: System Error: ", status.GetString()));
      result = 1;
    }
  }
  return result;
}

int cmcmd::SymlinkExecutable(std::vector<std::string> const& args)
{
  int result = 0;
  std::string const& realName = args[2];
  std::string const& name = args[3];
  if (name != realName) {
    cmsys::Status status = cmcmd::SymlinkInternal(realName, name);
    if (!status) {
      cmSystemTools::Error(cmStrCat("cmake_symlink_executable: System Error: ",
                                    status.GetString()));
      result = 1;
    }
  }
  return result;
}

cmsys::Status cmcmd::SymlinkInternal(std::string const& file,
                                     std::string const& link)
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

static void cmcmdProgressReport(std::string const& dir, std::string const& num)
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

  const char* last = num.c_str();
  for (const char* c = last;; ++c) {
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
    // print the progress
    fprintf(stdout, "[%3i%%] ", ((fileNum - 3) * 100) / count);
  }
}

int cmcmd::ExecuteEchoColor(std::vector<std::string> const& args)
{
  // The arguments are
  //   args[0] == <cmake-executable>
  //   args[1] == cmake_echo_color

  bool enabled = true;
  int color = cmsysTerminal_Color_Normal;
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
      color = cmsysTerminal_Color_Normal;
    } else if (arg == "--black") {
      color = cmsysTerminal_Color_ForegroundBlack;
    } else if (arg == "--red") {
      color = cmsysTerminal_Color_ForegroundRed;
    } else if (arg == "--green") {
      color = cmsysTerminal_Color_ForegroundGreen;
    } else if (arg == "--yellow") {
      color = cmsysTerminal_Color_ForegroundYellow;
    } else if (arg == "--blue") {
      color = cmsysTerminal_Color_ForegroundBlue;
    } else if (arg == "--magenta") {
      color = cmsysTerminal_Color_ForegroundMagenta;
    } else if (arg == "--cyan") {
      color = cmsysTerminal_Color_ForegroundCyan;
    } else if (arg == "--white") {
      color = cmsysTerminal_Color_ForegroundWhite;
    } else if (arg == "--bold") {
      color |= cmsysTerminal_Color_ForegroundBold;
    } else if (arg == "--no-newline") {
      newline = false;
    } else if (arg == "--newline") {
      newline = true;
    } else {
      // Color is enabled.  Print with the current color.
      cmSystemTools::MakefileColorEcho(color, arg.c_str(), newline, enabled);
    }
  }

  return 0;
}

int cmcmd::ExecuteLinkScript(std::vector<std::string> const& args)
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

  // Allocate a process instance.
  cmsysProcess* cp = cmsysProcess_New();
  if (!cp) {
    std::cerr << "Error allocating process instance in link script."
              << std::endl;
    return 1;
  }

  // Children should share stdout and stderr with this process.
  cmsysProcess_SetPipeShared(cp, cmsysProcess_Pipe_STDOUT, 1);
  cmsysProcess_SetPipeShared(cp, cmsysProcess_Pipe_STDERR, 1);

  // Run the command lines verbatim.
  cmsysProcess_SetOption(cp, cmsysProcess_Option_Verbatim, 1);

  // Read command lines from the script.
  cmsys::ifstream fin(args[2].c_str());
  if (!fin) {
    std::cerr << "Error opening link script \"" << args[2] << "\""
              << std::endl;
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

    // Setup this command line.
    const char* cmd[2] = { command.c_str(), nullptr };
    cmsysProcess_SetCommand(cp, cmd);

    // Report the command if verbose output is enabled.
    if (verbose) {
      std::cout << command << std::endl;
    }

    // Run the command and wait for it to exit.
    cmsysProcess_Execute(cp);
    cmsysProcess_WaitForExit(cp, nullptr);

    // Report failure if any.
    switch (cmsysProcess_GetState(cp)) {
      case cmsysProcess_State_Exited: {
        int value = cmsysProcess_GetExitValue(cp);
        if (value != 0) {
          result = value;
        }
      } break;
      case cmsysProcess_State_Exception:
        std::cerr << "Error running link command: "
                  << cmsysProcess_GetExceptionString(cp) << std::endl;
        result = 1;
        break;
      case cmsysProcess_State_Error:
        std::cerr << "Error running link command: "
                  << cmsysProcess_GetErrorString(cp) << std::endl;
        result = 2;
        break;
      default:
        break;
    }
  }

  // Free the process instance.
  cmsysProcess_Delete(cp);

  // Return the final resulting return value.
  return result;
}

int cmcmd::WindowsCEEnvironment(const char* version, const std::string& name)
{
#if !defined(CMAKE_BOOTSTRAP) && defined(_WIN32) && !defined(__CYGWIN__)
  cmVisualStudioWCEPlatformParser parser(name.c_str());
  parser.ParseVersion(version);
  if (parser.Found()) {
    /* clang-format off */
    std::cout << "@echo off\n"
                 "echo Environment Selection: " << name << "\n"
                 "set PATH=" << parser.GetPathDirectories() << "\n"
                 "set INCLUDE=" << parser.GetIncludeDirectories() << "\n"
                 "set LIB=" << parser.GetLibraryDirectories() << std::endl;
    /* clang-format on */
    return 0;
  }
#else
  (void)version;
#endif

  std::cerr << "Could not find " << name;
  return -1;
}

int cmcmd::RunPreprocessor(const std::vector<std::string>& command,
                           const std::string& intermediate_file)
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
    cmUVPipeIStream errorStream(process.GetLoop(), process.ErrorStream());
    std::cerr << errorStream.rdbuf();

    return 1;
  }
  return 0;
}

int cmcmd::RunLLVMRC(std::vector<std::string> const& args)
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

  const std::string& intermediate_file = args[3];
  const std::string& source_file = args[2];
  std::vector<std::string> preprocess;
  std::vector<std::string> resource_compile;
  std::vector<std::string>* pArgTgt = &preprocess;

  static const cmsys::RegularExpression llvm_rc_only_single_arg("^[-/](N|Y)");
  static const cmsys::RegularExpression llvm_rc_only_double_arg(
    "^[-/](C|LN|L)(.)?");
  static const cmsys::RegularExpression common_double_arg(
    "^[-/](D|U|I|FO|fo|Fo)(.)?");
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
    cmUVPipeIStream errorStream(process.GetLoop(), process.ErrorStream());
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
int cmcmd::VisualStudioLink(std::vector<std::string> const& args, int type)
{
  // Replace streambuf so we output in the system codepage. CMake is set up
  // to output in Unicode (see SetUTF8Pipes) but the Visual Studio linker
  // outputs using the system codepage so we need to change behavior when
  // we run the link command.
  cmConsoleBuf consoleBuf;

  if (args.size() < 2) {
    return -1;
  }
  const bool verbose = cmSystemTools::HasEnv("VERBOSE");
  std::vector<std::string> expandedArgs;
  for (std::string const& i : args) {
    // check for nmake temporary files
    if (i[0] == '@' && !cmHasLiteralPrefix(i, "@CMakeFiles")) {
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
static std::ostream& operator<<(std::ostream& stream,
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

static bool RunCommand(const char* comment,
                       std::vector<std::string> const& command, bool verbose,
                       NumberFormat exitFormat, int* retCodeOut = nullptr,
                       bool (*retCodeOkay)(int) = nullptr)
{
  if (verbose) {
    std::cout << comment << ":\n";
    std::cout << cmJoin(command, " ") << "\n";
  }
  std::string output;
  int retCode = 0;
  bool commandResult = cmSystemTools::RunSingleCommand(
    command, &output, &output, &retCode, nullptr, cmSystemTools::OUTPUT_NONE);
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
  while (arg != argEnd && cmHasLiteralPrefix(*arg, "-")) {
    if (*arg == "--") {
      ++arg;
      break;
    }
    if (*arg == "--manifests") {
      for (++arg; arg != argEnd && !cmHasLiteralPrefix(*arg, "-"); ++arg) {
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

    if (this->LinkGeneratesManifest) {
      this->LinkCommand.emplace_back("/MANIFEST");
      this->LinkCommand.push_back("/MANIFESTFILE:" + this->LinkerManifestFile);
    }
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

static bool mtRetIsUpdate(int mtRet)
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
    cmSystemTools::CollapseFullPath(this->ManifestFile);
  if (this->Verbose) {
    std::cout << "Create " << this->ManifestFileRC << "\n";
  }
  {
    cmsys::ofstream fout(this->ManifestFileRC.c_str());
    if (!fout) {
      return -1;
    }
    // Insert a pragma statement to specify utf-8 encoding.
    fout << "#pragma code_page(65001)\n";
    fout << this->Type
         << " /* CREATEPROCESS_MANIFEST_RESOURCE_ID */ "
            "24 /* RT_MANIFEST */ \""
         << absManifestFile << "\"";
  }

  // If we have not previously generated a manifest file,
  // generate a manifest file so the resource compiler succeeds.
  if (!cmSystemTools::FileExists(this->ManifestFile)) {
    if (this->Verbose) {
      std::cout << "Create empty: " << this->ManifestFile << "\n";
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
  rcCommand.emplace_back("/fo");
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
