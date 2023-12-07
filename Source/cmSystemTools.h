/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#if !defined(_WIN32)
#  include <sys/types.h>
#endif

#include <cstddef>
#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <cm/optional>
#include <cm/string_view>

#include <cm3p/uv.h>

#include "cmsys/Status.hxx"      // IWYU pragma: export
#include "cmsys/SystemTools.hxx" // IWYU pragma: export

#include "cmDuration.h"
#include "cmProcessOutput.h"

struct cmMessageMetadata;

/** \class cmSystemTools
 * \brief A collection of useful functions for CMake.
 *
 * cmSystemTools is a class that provides helper functions
 * for the CMake build system.
 */
class cmSystemTools : public cmsys::SystemTools
{
public:
  using Superclass = cmsys::SystemTools;
  using Encoding = cmProcessOutput::Encoding;

  /**
   * Look for and replace registry values in a string
   */
  static void ExpandRegistryValues(std::string& source,
                                   KeyWOW64 view = KeyWOW64_Default);

  /** Map help document name to file name.  */
  static std::string HelpFileName(cm::string_view);

  using MessageCallback =
    std::function<void(const std::string&, const cmMessageMetadata&)>;
  /**
   *  Set the function used by GUIs to display error messages
   *  Function gets passed: message as a const char*,
   *  title as a const char*.
   */
  static void SetMessageCallback(MessageCallback f);

  /**
   * Display an error message.
   */
  static void Error(const std::string& m);

  /**
   * Display a message.
   */
  static void Message(const std::string& m, const char* title = nullptr);
  static void Message(const std::string& m, const cmMessageMetadata& md);

  using OutputCallback = std::function<void(std::string const&)>;

  //! Send a string to stdout
  static void Stdout(const std::string& s);
  static void SetStdoutCallback(OutputCallback f);

  //! Send a string to stderr
  static void Stderr(const std::string& s);
  static void SetStderrCallback(OutputCallback f);

  using InterruptCallback = std::function<bool()>;
  static void SetInterruptCallback(InterruptCallback f);
  static bool GetInterruptFlag();

  //! Return true if there was an error at any point.
  static bool GetErrorOccurredFlag()
  {
    return cmSystemTools::s_ErrorOccurred ||
      cmSystemTools::s_FatalErrorOccurred || GetInterruptFlag();
  }
  //! If this is set to true, cmake stops processing commands.
  static void SetFatalErrorOccurred()
  {
    cmSystemTools::s_FatalErrorOccurred = true;
  }
  static void SetErrorOccurred() { cmSystemTools::s_ErrorOccurred = true; }
  //! Return true if there was an error at any point.
  static bool GetFatalErrorOccurred()
  {
    return cmSystemTools::s_FatalErrorOccurred || GetInterruptFlag();
  }

  //! Set the error occurred flag and fatal error back to false
  static void ResetErrorOccurredFlag()
  {
    cmSystemTools::s_FatalErrorOccurred = false;
    cmSystemTools::s_ErrorOccurred = false;
  }

  //! Return true if the path is a framework
  static bool IsPathToFramework(const std::string& path);

  //! Return true if the path is a xcframework
  static bool IsPathToXcFramework(const std::string& path);

  //! Return true if the path is a macOS non-framework shared library (aka
  //! .dylib)
  static bool IsPathToMacOSSharedLibrary(const std::string& path);

  static bool DoesFileExistWithExtensions(
    const std::string& name, const std::vector<std::string>& sourceExts);

  /**
   * Check if the given file exists in one of the parent directory of the
   * given file or directory and if it does, return the name of the file.
   * Toplevel specifies the top-most directory to where it will look.
   */
  static std::string FileExistsInParentDirectories(
    const std::string& fname, const std::string& directory,
    const std::string& toplevel);

  static void Glob(const std::string& directory, const std::string& regexp,
                   std::vector<std::string>& files);
  static void GlobDirs(const std::string& fullPath,
                       std::vector<std::string>& files);

  /**
   * Try to find a list of files that match the "simple" globbing
   * expression. At this point in time the globbing expressions have
   * to be in form: /directory/partial_file_name*. The * character has
   * to be at the end of the string and it does not support ?
   * []... The optional argument type specifies what kind of files you
   * want to find. 0 means all files, -1 means directories, 1 means
   * files only. This method returns true if search was successful.
   */
  static bool SimpleGlob(const std::string& glob,
                         std::vector<std::string>& files, int type = 0);

  enum class CopyWhen
  {
    Always,
    OnlyIfDifferent,
  };
  enum class CopyInputRecent
  {
    No,
    Yes,
  };
  enum class CopyResult
  {
    Success,
    Failure,
  };

#if defined(_MSC_VER)
  /** Visual C++ does not define mode_t. */
  using mode_t = unsigned short;
#endif

  /**
   * Make a new temporary directory.  The path must end in "XXXXXX", and will
   * be modified to reflect the name of the directory created.  This function
   * is similar to POSIX mkdtemp (and is implemented using the same where that
   * function is available).
   *
   * This function can make a full path even if none of the directories existed
   * prior to calling this function.
   *
   * Note that this function may modify \p path even if it does not succeed.
   */
  static cmsys::Status MakeTempDirectory(char* path,
                                         const mode_t* mode = nullptr);
  static cmsys::Status MakeTempDirectory(std::string& path,
                                         const mode_t* mode = nullptr);

  /** Copy a file. */
  static CopyResult CopySingleFile(std::string const& oldname,
                                   std::string const& newname, CopyWhen when,
                                   CopyInputRecent inputRecent,
                                   std::string* err = nullptr);

  enum class Replace
  {
    Yes,
    No,
  };
  enum class RenameResult
  {
    Success,
    NoReplace,
    Failure,
  };

  /** Rename a file or directory within a single disk volume (atomic
      if possible).  */
  static bool RenameFile(const std::string& oldname,
                         const std::string& newname);
  static RenameResult RenameFile(std::string const& oldname,
                                 std::string const& newname, Replace replace,
                                 std::string* err = nullptr);

  //! Rename a file if contents are different, delete the source otherwise
  static void MoveFileIfDifferent(const std::string& source,
                                  const std::string& destination);

  /**
   * Run a single executable command
   *
   * Output is controlled with outputflag. If outputflag is OUTPUT_NONE, no
   * user-viewable output from the program being run will be generated.
   * OUTPUT_MERGE is the legacy behavior where stdout and stderr are merged
   * into stdout.  OUTPUT_FORWARD copies the output to stdout/stderr as
   * it was received.  OUTPUT_PASSTHROUGH passes through the original handles.
   *
   * If timeout is specified, the command will be terminated after
   * timeout expires. Timeout is specified in seconds.
   *
   * Argument retVal should be a pointer to the location where the
   * exit code will be stored. If the retVal is not specified and
   * the program exits with a code other than 0, then the this
   * function will return false.
   *
   * If the command has spaces in the path the caller MUST call
   * cmSystemTools::ConvertToRunCommandPath on the command before passing
   * it into this function or it will not work.  The command must be correctly
   * escaped for this to with spaces.
   */
  enum OutputOption
  {
    OUTPUT_NONE = 0,
    OUTPUT_MERGE,
    OUTPUT_FORWARD,
    OUTPUT_PASSTHROUGH
  };
  static bool RunSingleCommand(const std::string& command,
                               std::string* captureStdOut = nullptr,
                               std::string* captureStdErr = nullptr,
                               int* retVal = nullptr,
                               const char* dir = nullptr,
                               OutputOption outputflag = OUTPUT_MERGE,
                               cmDuration timeout = cmDuration::zero());
  /**
   * In this version of RunSingleCommand, command[0] should be
   * the command to run, and each argument to the command should
   * be in command[1]...command[command.size()]
   */
  static bool RunSingleCommand(std::vector<std::string> const& command,
                               std::string* captureStdOut = nullptr,
                               std::string* captureStdErr = nullptr,
                               int* retVal = nullptr,
                               const char* dir = nullptr,
                               OutputOption outputflag = OUTPUT_MERGE,
                               cmDuration timeout = cmDuration::zero(),
                               Encoding encoding = cmProcessOutput::Auto);

  static std::string PrintSingleCommand(std::vector<std::string> const&);

  /**
   * Parse arguments out of a single string command
   */
  static std::vector<std::string> ParseArguments(const std::string& command);

  /** Parse arguments out of a windows command line string.  */
  static void ParseWindowsCommandLine(const char* command,
                                      std::vector<std::string>& args);

  /** Parse arguments out of a unix command line string.  */
  static void ParseUnixCommandLine(const char* command,
                                   std::vector<std::string>& args);

  /** Split a command-line string into the parsed command and the unparsed
      arguments.  Returns false on unfinished quoting or escaping.  */
  static bool SplitProgramFromArgs(std::string const& command,
                                   std::string& program, std::string& args);

  /**
   * Handle response file in an argument list and return a new argument list
   * **/
  static std::vector<std::string> HandleResponseFile(
    std::vector<std::string>::const_iterator argBeg,
    std::vector<std::string>::const_iterator argEnd);

  static std::size_t CalculateCommandLineLengthLimit();

  static void DisableRunCommandOutput() { s_DisableRunCommandOutput = true; }
  static void EnableRunCommandOutput() { s_DisableRunCommandOutput = false; }
  static bool GetRunCommandOutput() { return s_DisableRunCommandOutput; }

  enum CompareOp
  {
    OP_EQUAL = 1,
    OP_LESS = 2,
    OP_GREATER = 4,
    OP_LESS_EQUAL = OP_LESS | OP_EQUAL,
    OP_GREATER_EQUAL = OP_GREATER | OP_EQUAL
  };

  /**
   * Compare versions
   */
  static bool VersionCompare(CompareOp op, const std::string& lhs,
                             const std::string& rhs);
  static bool VersionCompare(CompareOp op, const std::string& lhs,
                             const char rhs[]);
  static bool VersionCompareEqual(std::string const& lhs,
                                  std::string const& rhs);
  static bool VersionCompareGreater(std::string const& lhs,
                                    std::string const& rhs);
  static bool VersionCompareGreaterEq(std::string const& lhs,
                                      std::string const& rhs);

  /**
   * Compare two ASCII strings using natural versioning order.
   * Non-numerical characters are compared directly.
   * Numerical characters are first globbed such that, e.g.
   * `test000 < test01 < test0 < test1 < test10`.
   * Return a value less than, equal to, or greater than zero if lhs
   * precedes, equals, or succeeds rhs in the defined ordering.
   */
  static int strverscmp(std::string const& lhs, std::string const& rhs);

  /** Windows if this is true, the CreateProcess in RunCommand will
   *  not show new console windows when running programs.
   */
  static void SetRunCommandHideConsole(bool v) { s_RunCommandHideConsole = v; }
  static bool GetRunCommandHideConsole() { return s_RunCommandHideConsole; }
  /** Call cmSystemTools::Error with the message m, plus the
   * result of strerror(errno)
   */
  static void ReportLastSystemError(const char* m);

  enum class WaitForLineResult
  {
    None,
    STDOUT,
    STDERR,
    Timeout,
  };

  /** a general output handler for libuv  */
  static WaitForLineResult WaitForLine(uv_loop_t* loop, uv_stream_t* outPipe,
                                       uv_stream_t* errPipe, std::string& line,
                                       cmDuration timeout,
                                       std::vector<char>& out,
                                       std::vector<char>& err);

  static void SetForceUnixPaths(bool v) { s_ForceUnixPaths = v; }
  static bool GetForceUnixPaths() { return s_ForceUnixPaths; }

  // ConvertToOutputPath use s_ForceUnixPaths
  static std::string ConvertToOutputPath(std::string const& path);
  static void ConvertToOutputSlashes(std::string& path);

  // ConvertToRunCommandPath does not use s_ForceUnixPaths and should
  // be used when RunCommand is called from cmake, because the
  // running cmake needs paths to be in its format
  static std::string ConvertToRunCommandPath(const std::string& path);

  /**
   * For windows computes the long path for the given path,
   * For Unix, it is a noop
   */
  static void ConvertToLongPath(std::string& path);

  /** compute the relative path from local to remote.  local must
      be a directory.  remote can be a file or a directory.
      Both remote and local must be full paths.  Basically, if
      you are in directory local and you want to access the file in remote
      what is the relative path to do that.  For example:
      /a/b/c/d to /a/b/c1/d1 -> ../../c1/d1
      from /usr/src to /usr/src/test/blah/foo.cpp -> test/blah/foo.cpp
  */
  static std::string RelativePath(std::string const& local,
                                  std::string const& remote);

  /**
   * Convert the given remote path to a relative path with respect to
   * the given local path.  Both paths must use forward slashes and not
   * already be escaped or quoted.
   */
  static std::string ForceToRelativePath(std::string const& local_path,
                                         std::string const& remote_path);

  /**
   * Express the 'in' path relative to 'top' if it does not start in '../'.
   */
  static std::string RelativeIfUnder(std::string const& top,
                                     std::string const& in);

  static cm::optional<std::string> GetEnvVar(std::string const& var);
  static std::vector<std::string> SplitEnvPath(std::string const& value);

#ifndef CMAKE_BOOTSTRAP
  /** Remove an environment variable */
  static bool UnsetEnv(const char* value);

  /** Get the list of all environment variables */
  static std::vector<std::string> GetEnvironmentVariables();

  /** Append multiple variables to the current environment. */
  static void AppendEnv(std::vector<std::string> const& env);

  /**
   * Helper class to represent an environment diff directly. This is to avoid
   * repeated in-place environment modification (i.e. via setenv/putenv), which
   * could be slow.
   */
  class EnvDiff
  {
  public:
    /** Append multiple variables to the current environment diff */
    void AppendEnv(std::vector<std::string> const& env);

    /**
     * Add a single variable (or remove if no = sign) to the current
     * environment diff.
     */
    void PutEnv(const std::string& env);

    /** Remove a single variable from the current environment diff. */
    void UnPutEnv(const std::string& env);

    /**
     * Apply an ENVIRONMENT_MODIFICATION operation to this diff. Returns
     * false and issues an error on parse failure.
     */
    bool ParseOperation(const std::string& envmod);

    /**
     * Apply this diff to the actual environment, optionally writing out the
     * modifications to a CTest-compatible measurement stream.
     */
    void ApplyToCurrentEnv(std::ostringstream* measurement = nullptr);

  private:
    std::map<std::string, cm::optional<std::string>> diff;
  };

  /** Helper class to save and restore the environment.
      Instantiate this class as an automatic variable on
      the stack. Its constructor saves a copy of the current
      environment and then its destructor restores the
      original environment. */
  class SaveRestoreEnvironment
  {
  public:
    SaveRestoreEnvironment();
    ~SaveRestoreEnvironment();

    SaveRestoreEnvironment(SaveRestoreEnvironment const&) = delete;
    SaveRestoreEnvironment& operator=(SaveRestoreEnvironment const&) = delete;

  private:
    std::vector<std::string> Env;
  };
#endif

  /** Setup the environment to enable VS 8 IDE output.  */
  static void EnableVSConsoleOutput();

  enum cmTarAction
  {
    TarActionCreate,
    TarActionList,
    TarActionExtract,
    TarActionNone
  };

  /** Create tar */
  enum cmTarCompression
  {
    TarCompressGZip,
    TarCompressBZip2,
    TarCompressXZ,
    TarCompressZstd,
    TarCompressNone
  };

  enum class cmTarExtractTimestamps
  {
    Yes,
    No
  };

  static bool ListTar(const std::string& outFileName,
                      const std::vector<std::string>& files, bool verbose);
  static bool CreateTar(const std::string& outFileName,
                        const std::vector<std::string>& files,
                        cmTarCompression compressType, bool verbose,
                        std::string const& mtime = std::string(),
                        std::string const& format = std::string(),
                        int compressionLevel = 0);
  static bool ExtractTar(const std::string& inFileName,
                         const std::vector<std::string>& files,
                         cmTarExtractTimestamps extractTimestamps,
                         bool verbose);
  // This should be called first thing in main
  // it will keep child processes from inheriting the
  // stdin and stdout of this process.  This is important
  // if you want to be able to kill child processes and
  // not get stuck waiting for all the output on the pipes.
  static void DoNotInheritStdPipes();

  static void EnsureStdPipes();

  /** Random seed generation.  */
  static unsigned int RandomSeed();

  /** Find the directory containing CMake executables.  */
  static void FindCMakeResources(const char* argv0);

  /** Get the CMake resource paths, after FindCMakeResources.  */
  static std::string const& GetCTestCommand();
  static std::string const& GetCPackCommand();
  static std::string const& GetCMakeCommand();
  static std::string const& GetCMakeGUICommand();
  static std::string const& GetCMakeCursesCommand();
  static std::string const& GetCMClDepsCommand();
  static std::string const& GetCMakeRoot();
  static std::string const& GetHTMLDoc();

  /** Get the CWD mapped through the KWSys translation map.  */
  static std::string GetCurrentWorkingDirectory();

  /** Echo a message in color using KWSys's Terminal cprintf.  */
  static void MakefileColorEcho(int color, const char* message, bool newLine,
                                bool enabled);

  /** Try to guess the soname of a shared library.  */
  static bool GuessLibrarySOName(std::string const& fullPath,
                                 std::string& soname);

  /** Try to guess the install name of a shared library.  */
  static bool GuessLibraryInstallName(std::string const& fullPath,
                                      std::string& soname);

  /** Try to change the RPATH in an ELF binary.  */
  static bool ChangeRPath(std::string const& file, std::string const& oldRPath,
                          std::string const& newRPath,
                          bool removeEnvironmentRPath,
                          std::string* emsg = nullptr,
                          bool* changed = nullptr);

  /** Try to set the RPATH in an ELF binary.  */
  static bool SetRPath(std::string const& file, std::string const& newRPath,
                       std::string* emsg = nullptr, bool* changed = nullptr);

  /** Try to remove the RPATH from an ELF binary.  */
  static bool RemoveRPath(std::string const& file, std::string* emsg = nullptr,
                          bool* removed = nullptr);

  /** Check whether the RPATH in an ELF binary contains the path
      given.  */
  static bool CheckRPath(std::string const& file, std::string const& newRPath);

  /** Remove a directory; repeat a few times in case of locked files.  */
  static bool RepeatedRemoveDirectory(const std::string& dir);

  /** Encode a string as a URL.  */
  static std::string EncodeURL(std::string const& in,
                               bool escapeSlashes = true);

#ifdef _WIN32
  struct WindowsFileRetry
  {
    unsigned int Count;
    unsigned int Delay;
  };
  static WindowsFileRetry GetWindowsFileRetry();
  static WindowsFileRetry GetWindowsDirectoryRetry();

  struct WindowsVersion
  {
    unsigned int dwMajorVersion;
    unsigned int dwMinorVersion;
    unsigned int dwBuildNumber;
  };
  static WindowsVersion GetWindowsVersion();
#endif

  /** Get the real path for a given path, removing all symlinks.
      This variant of GetRealPath also works on Windows but will
      resolve subst drives too.  */
  static std::string GetRealPathResolvingWindowsSubst(
    const std::string& path, std::string* errorMessage = nullptr);

  /** Perform one-time initialization of libuv.  */
  static void InitializeLibUV();

  /** Create a symbolic link if the platform supports it.  Returns whether
      creation succeeded. */
  static cmsys::Status CreateSymlink(std::string const& origName,
                                     std::string const& newName);
  static cmsys::Status CreateSymlinkQuietly(std::string const& origName,
                                            std::string const& newName);

  /** Create a hard link if the platform supports it.  Returns whether
      creation succeeded. */
  static cmsys::Status CreateLink(std::string const& origName,
                                  std::string const& newName);
  static cmsys::Status CreateLinkQuietly(std::string const& origName,
                                         std::string const& newName);

  /** Get the system name. */
  static cm::string_view GetSystemName();

  /** Get the system path separator character */
  static char GetSystemPathlistSeparator();

private:
  static bool s_ForceUnixPaths;
  static bool s_RunCommandHideConsole;
  static bool s_ErrorOccurred;
  static bool s_FatalErrorOccurred;
  static bool s_DisableRunCommandOutput;
};
