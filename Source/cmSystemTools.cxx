/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#if !defined(_WIN32) && !defined(__sun) && !defined(__OpenBSD__)
// POSIX APIs are needed
// NOLINTNEXTLINE(bugprone-reserved-identifier)
#  define _POSIX_C_SOURCE 200809L
#endif
#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__QNX__)
// For isascii
// NOLINTNEXTLINE(bugprone-reserved-identifier)
#  define _XOPEN_SOURCE 700
#endif
#if defined(__APPLE__)
// Restore Darwin APIs removed by _POSIX_C_SOURCE.
// NOLINTNEXTLINE(bugprone-reserved-identifier)
#  define _DARWIN_C_SOURCE
#endif

#include "cmSystemTools.h"

#include <cm/optional>
#include <cmext/algorithm>
#include <cmext/string_view>

#include <cm3p/uv.h>

#include "cmDuration.h"
#include "cmELF.h"
#include "cmMessageMetadata.h"
#include "cmProcessOutput.h"
#include "cmRange.h"
#include "cmStringAlgorithms.h"
#include "cmValue.h"

#if !defined(CMAKE_BOOTSTRAP)
#  include <cm3p/archive.h>
#  include <cm3p/archive_entry.h>

#  include "cmArchiveWrite.h"
#  include "cmLocale.h"
#  ifndef __LA_INT64_T
#    define __LA_INT64_T la_int64_t
#  endif
#  ifndef __LA_SSIZE_T
#    define __LA_SSIZE_T la_ssize_t
#  endif
#endif

#if !defined(CMAKE_BOOTSTRAP)
#  if defined(_WIN32)
#    include <cm/memory>
#  endif
#  include "cmCryptoHash.h"
#endif

#if defined(CMake_USE_MACH_PARSER)
#  include "cmMachO.h"
#endif

#if defined(CMake_USE_XCOFF_PARSER)
#  include "cmXCOFF.h"
#endif

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <functional>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

#include <fcntl.h>

#include "cmsys/Directory.hxx"
#include "cmsys/Encoding.hxx"
#include "cmsys/FStream.hxx"
#include "cmsys/RegularExpression.hxx"
#include "cmsys/System.h"
#include "cmsys/Terminal.h"

#if defined(_WIN32)
#  include <windows.h>
// include wincrypt.h after windows.h
#  include <wincrypt.h>
#else
#  include <unistd.h>

#  include <sys/time.h>
#endif

#if defined(_WIN32) &&                                                        \
  (defined(_MSC_VER) || defined(__WATCOMC__) || defined(__MINGW32__))
#  include <io.h>
#endif

#if defined(__APPLE__)
#  include <mach-o/dyld.h>
#endif

#ifdef __QNX__
#  include <malloc.h> /* for malloc/free on QNX */
#endif

#if !defined(_WIN32) && !defined(__ANDROID__)
#  include <sys/utsname.h>
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1800
#  define CM_WINDOWS_DEPRECATED_GetVersionEx
#endif

namespace {

cmSystemTools::InterruptCallback s_InterruptCallback;
cmSystemTools::MessageCallback s_MessageCallback;
cmSystemTools::OutputCallback s_StderrCallback;
cmSystemTools::OutputCallback s_StdoutCallback;

} // namespace

#if !defined(HAVE_ENVIRON_NOT_REQUIRE_PROTOTYPE)
// For GetEnvironmentVariables
#  if defined(_WIN32)
extern __declspec(dllimport) char** environ;
#  else
extern char** environ;
#  endif
#endif

#if !defined(CMAKE_BOOTSTRAP)
static std::string cm_archive_entry_pathname(struct archive_entry* entry)
{
#  if cmsys_STL_HAS_WSTRING
  return cmsys::Encoding::ToNarrow(archive_entry_pathname_w(entry));
#  else
  return archive_entry_pathname(entry);
#  endif
}

static int cm_archive_read_open_file(struct archive* a, const char* file,
                                     int block_size)
{
#  if cmsys_STL_HAS_WSTRING
  std::wstring wfile = cmsys::Encoding::ToWide(file);
  return archive_read_open_filename_w(a, wfile.c_str(), block_size);
#  else
  return archive_read_open_filename(a, file, block_size);
#  endif
}
#endif

#ifdef _WIN32
#elif defined(__APPLE__)
#  include <crt_externs.h>

#  define environ (*_NSGetEnviron())
#endif

bool cmSystemTools::s_RunCommandHideConsole = false;
bool cmSystemTools::s_DisableRunCommandOutput = false;
bool cmSystemTools::s_ErrorOccurred = false;
bool cmSystemTools::s_FatalErrorOccurred = false;
bool cmSystemTools::s_ForceUnixPaths = false;

// replace replace with with as many times as it shows up in source.
// write the result into source.
#if defined(_WIN32) && !defined(__CYGWIN__)
void cmSystemTools::ExpandRegistryValues(std::string& source, KeyWOW64 view)
{
  // Regular expression to match anything inside [...] that begins in HKEY.
  // Note that there is a special rule for regular expressions to match a
  // close square-bracket inside a list delimited by square brackets.
  // The "[^]]" part of this expression will match any character except
  // a close square-bracket.  The ']' character must be the first in the
  // list of characters inside the [^...] block of the expression.
  cmsys::RegularExpression regEntry("\\[(HKEY[^]]*)\\]");

  // check for black line or comment
  while (regEntry.find(source)) {
    // the arguments are the second match
    std::string key = regEntry.match(1);
    std::string val;
    if (ReadRegistryValue(key.c_str(), val, view)) {
      std::string reg = cmStrCat('[', key, ']');
      cmSystemTools::ReplaceString(source, reg.c_str(), val.c_str());
    } else {
      std::string reg = cmStrCat('[', key, ']');
      cmSystemTools::ReplaceString(source, reg.c_str(), "/registry");
    }
  }
}
#else
void cmSystemTools::ExpandRegistryValues(std::string& source,
                                         KeyWOW64 /*unused*/)
{
  cmsys::RegularExpression regEntry("\\[(HKEY[^]]*)\\]");
  while (regEntry.find(source)) {
    // the arguments are the second match
    std::string key = regEntry.match(1);
    std::string reg = cmStrCat('[', key, ']');
    cmSystemTools::ReplaceString(source, reg.c_str(), "/registry");
  }
}
#endif

std::string cmSystemTools::HelpFileName(cm::string_view str)
{
  std::string name(str);
  cmSystemTools::ReplaceString(name, "<", "");
  cmSystemTools::ReplaceString(name, ">", "");
  return name;
}

void cmSystemTools::Error(const std::string& m)
{
  std::string message = "CMake Error: " + m;
  cmSystemTools::s_ErrorOccurred = true;
  cmSystemTools::Message(message, "Error");
}

void cmSystemTools::SetInterruptCallback(InterruptCallback f)
{
  s_InterruptCallback = std::move(f);
}

bool cmSystemTools::GetInterruptFlag()
{
  if (s_InterruptCallback) {
    return s_InterruptCallback();
  }
  return false;
}

void cmSystemTools::SetMessageCallback(MessageCallback f)
{
  s_MessageCallback = std::move(f);
}

void cmSystemTools::SetStdoutCallback(OutputCallback f)
{
  s_StdoutCallback = std::move(f);
}

void cmSystemTools::SetStderrCallback(OutputCallback f)
{
  s_StderrCallback = std::move(f);
}

void cmSystemTools::Stderr(const std::string& s)
{
  if (s_StderrCallback) {
    s_StderrCallback(s);
  } else {
    std::cerr << s << std::flush;
  }
}

void cmSystemTools::Stdout(const std::string& s)
{
  if (s_StdoutCallback) {
    s_StdoutCallback(s);
  } else {
    std::cout << s << std::flush;
  }
}

void cmSystemTools::Message(const std::string& m, const char* title)
{
  cmMessageMetadata md;
  md.title = title;
  Message(m, md);
}

void cmSystemTools::Message(const std::string& m, const cmMessageMetadata& md)
{
  if (s_MessageCallback) {
    s_MessageCallback(m, md);
  } else {
    std::cerr << m << std::endl;
  }
}

void cmSystemTools::ReportLastSystemError(const char* msg)
{
  std::string m =
    cmStrCat(msg, ": System Error: ", Superclass::GetLastSystemError());
  cmSystemTools::Error(m);
}

void cmSystemTools::ParseWindowsCommandLine(const char* command,
                                            std::vector<std::string>& args)
{
  // See the MSDN document "Parsing C Command-Line Arguments" at
  // http://msdn2.microsoft.com/en-us/library/a1y7w461.aspx for rules
  // of parsing the windows command line.

  bool in_argument = false;
  bool in_quotes = false;
  int backslashes = 0;
  std::string arg;
  for (const char* c = command; *c; ++c) {
    if (*c == '\\') {
      ++backslashes;
      in_argument = true;
    } else if (*c == '"') {
      int backslash_pairs = backslashes >> 1;
      int backslash_escaped = backslashes & 1;
      arg.append(backslash_pairs, '\\');
      backslashes = 0;
      if (backslash_escaped) {
        /* An odd number of backslashes precede this quote.
           It is escaped.  */
        arg.append(1, '"');
      } else {
        /* An even number of backslashes precede this quote.
           It is not escaped.  */
        in_quotes = !in_quotes;
      }
      in_argument = true;
    } else {
      arg.append(backslashes, '\\');
      backslashes = 0;
      if (cmIsSpace(*c)) {
        if (in_quotes) {
          arg.append(1, *c);
        } else if (in_argument) {
          args.push_back(arg);
          arg.clear();
          in_argument = false;
        }
      } else {
        in_argument = true;
        arg.append(1, *c);
      }
    }
  }
  arg.append(backslashes, '\\');
  if (in_argument) {
    args.push_back(arg);
  }
}

class cmSystemToolsArgV
{
  char** ArgV;

public:
  cmSystemToolsArgV(char** argv)
    : ArgV(argv)
  {
  }
  ~cmSystemToolsArgV()
  {
    for (char** arg = this->ArgV; arg && *arg; ++arg) {
      free(*arg);
    }
    free(this->ArgV);
  }
  cmSystemToolsArgV(const cmSystemToolsArgV&) = delete;
  cmSystemToolsArgV& operator=(const cmSystemToolsArgV&) = delete;
  void Store(std::vector<std::string>& args) const
  {
    for (char** arg = this->ArgV; arg && *arg; ++arg) {
      args.emplace_back(*arg);
    }
  }
};

void cmSystemTools::ParseUnixCommandLine(const char* command,
                                         std::vector<std::string>& args)
{
  // Invoke the underlying parser.
  cmSystemToolsArgV argv(cmsysSystem_Parse_CommandForUnix(command, 0));
  argv.Store(args);
}

std::vector<std::string> cmSystemTools::HandleResponseFile(
  std::vector<std::string>::const_iterator argBeg,
  std::vector<std::string>::const_iterator argEnd)
{
  std::vector<std::string> arg_full;
  for (std::string const& arg : cmMakeRange(argBeg, argEnd)) {
    if (cmHasLiteralPrefix(arg, "@")) {
      cmsys::ifstream responseFile(arg.substr(1).c_str(), std::ios::in);
      if (!responseFile) {
        std::string error = cmStrCat("failed to open for reading (",
                                     cmSystemTools::GetLastSystemError(),
                                     "):\n  ", cm::string_view(arg).substr(1));
        cmSystemTools::Error(error);
      } else {
        std::string line;
        cmSystemTools::GetLineFromStream(responseFile, line);
        std::vector<std::string> args2;
#ifdef _WIN32
        cmSystemTools::ParseWindowsCommandLine(line.c_str(), args2);
#else
        cmSystemTools::ParseUnixCommandLine(line.c_str(), args2);
#endif
        cm::append(arg_full, args2);
      }
    } else {
      arg_full.push_back(arg);
    }
  }
  return arg_full;
}

std::vector<std::string> cmSystemTools::ParseArguments(const std::string& cmd)
{
  std::vector<std::string> args;
  std::string arg;

  bool win_path = false;

  const char* command = cmd.c_str();
  if (command[0] && command[1] &&
      ((command[0] != '/' && command[1] == ':' && command[2] == '\\') ||
       (command[0] == '\"' && command[1] != '/' && command[2] == ':' &&
        command[3] == '\\') ||
       (command[0] == '\'' && command[1] != '/' && command[2] == ':' &&
        command[3] == '\\') ||
       (command[0] == '\\' && command[1] == '\\'))) {
    win_path = true;
  }
  // Split the command into an argv array.
  for (const char* c = command; *c;) {
    // Skip over whitespace.
    while (*c == ' ' || *c == '\t') {
      ++c;
    }
    arg.clear();
    if (*c == '"') {
      // Parse a quoted argument.
      ++c;
      while (*c && *c != '"') {
        arg.append(1, *c);
        ++c;
      }
      if (*c) {
        ++c;
      }
      args.push_back(arg);
    } else if (*c == '\'') {
      // Parse a quoted argument.
      ++c;
      while (*c && *c != '\'') {
        arg.append(1, *c);
        ++c;
      }
      if (*c) {
        ++c;
      }
      args.push_back(arg);
    } else if (*c) {
      // Parse an unquoted argument.
      while (*c && *c != ' ' && *c != '\t') {
        if (*c == '\\' && !win_path) {
          ++c;
          if (*c) {
            arg.append(1, *c);
            ++c;
          }
        } else {
          arg.append(1, *c);
          ++c;
        }
      }
      args.push_back(arg);
    }
  }

  return args;
}

bool cmSystemTools::SplitProgramFromArgs(std::string const& command,
                                         std::string& program,
                                         std::string& args)
{
  const char* c = command.c_str();

  // Skip leading whitespace.
  while (isspace(static_cast<unsigned char>(*c))) {
    ++c;
  }

  // Parse one command-line element up to an unquoted space.
  bool in_escape = false;
  bool in_double = false;
  bool in_single = false;
  for (; *c; ++c) {
    if (in_single) {
      if (*c == '\'') {
        in_single = false;
      } else {
        program += *c;
      }
    } else if (in_escape) {
      in_escape = false;
      program += *c;
    } else if (*c == '\\') {
      in_escape = true;
    } else if (in_double) {
      if (*c == '"') {
        in_double = false;
      } else {
        program += *c;
      }
    } else if (*c == '"') {
      in_double = true;
    } else if (*c == '\'') {
      in_single = true;
    } else if (isspace(static_cast<unsigned char>(*c))) {
      break;
    } else {
      program += *c;
    }
  }

  // The remainder of the command line holds unparsed arguments.
  args = c;

  return !in_single && !in_escape && !in_double;
}

size_t cmSystemTools::CalculateCommandLineLengthLimit()
{
  size_t sz =
#ifdef _WIN32
    // There's a maximum of 65536 bytes and thus 32768 WCHARs on Windows
    // However, cmd.exe itself can only handle 8191 WCHARs and Ninja for
    // example uses it to spawn processes.
    size_t(8191);
#elif defined(__linux)
    // MAX_ARG_STRLEN is the maximum length of a string permissible for
    // the execve() syscall on Linux. It's defined as (PAGE_SIZE * 32)
    // in Linux's binfmts.h
    static_cast<size_t>(sysconf(_SC_PAGESIZE) * 32);
#else
    size_t(0);
#endif

#if defined(_SC_ARG_MAX)
  // ARG_MAX is the maximum size of the command and environment
  // that can be passed to the exec functions on UNIX.
  // The value in climits does not need to be present and may
  // depend upon runtime memory constraints, hence sysconf()
  // should be used to query it.
  long szArgMax = sysconf(_SC_ARG_MAX);
  // A return value of -1 signifies an undetermined limit, but
  // it does not imply an infinite limit, and thus is ignored.
  if (szArgMax != -1) {
    // We estimate the size of the environment block to be 1000.
    // This isn't accurate at all, but leaves some headroom.
    szArgMax = szArgMax < 1000 ? 0 : szArgMax - 1000;
#  if defined(_WIN32) || defined(__linux)
    sz = std::min(sz, static_cast<size_t>(szArgMax));
#  else
    sz = static_cast<size_t>(szArgMax);
#  endif
  }
#endif
  return sz;
}

bool cmSystemTools::RunSingleCommand(std::vector<std::string> const& command,
                                     std::string* captureStdOut,
                                     std::string* captureStdErr, int* retVal,
                                     const char* dir, OutputOption outputflag,
                                     cmDuration timeout, Encoding encoding)
{
  std::vector<const char*> argv;
  argv.reserve(command.size() + 1);
  for (std::string const& cmd : command) {
    argv.push_back(cmd.c_str());
  }
  argv.push_back(nullptr);

  cmsysProcess* cp = cmsysProcess_New();
  cmsysProcess_SetCommand(cp, argv.data());
  cmsysProcess_SetWorkingDirectory(cp, dir);
  if (cmSystemTools::GetRunCommandHideConsole()) {
    cmsysProcess_SetOption(cp, cmsysProcess_Option_HideWindow, 1);
  }

  if (outputflag == OUTPUT_PASSTHROUGH) {
    cmsysProcess_SetPipeShared(cp, cmsysProcess_Pipe_STDOUT, 1);
    cmsysProcess_SetPipeShared(cp, cmsysProcess_Pipe_STDERR, 1);
    captureStdOut = nullptr;
    captureStdErr = nullptr;
  } else if (outputflag == OUTPUT_MERGE ||
             (captureStdErr && captureStdErr == captureStdOut)) {
    cmsysProcess_SetOption(cp, cmsysProcess_Option_MergeOutput, 1);
    captureStdErr = nullptr;
  }
  assert(!captureStdErr || captureStdErr != captureStdOut);

  cmsysProcess_SetTimeout(cp, timeout.count());
  cmsysProcess_Execute(cp);

  std::vector<char> tempStdOut;
  std::vector<char> tempStdErr;
  char* data;
  int length;
  int pipe;
  cmProcessOutput processOutput(encoding);
  std::string strdata;
  if (outputflag != OUTPUT_PASSTHROUGH &&
      (captureStdOut || captureStdErr || outputflag != OUTPUT_NONE)) {
    while ((pipe = cmsysProcess_WaitForData(cp, &data, &length, nullptr)) >
           0) {
      // Translate NULL characters in the output into valid text.
      for (int i = 0; i < length; ++i) {
        if (data[i] == '\0') {
          data[i] = ' ';
        }
      }

      if (pipe == cmsysProcess_Pipe_STDOUT) {
        if (outputflag != OUTPUT_NONE) {
          processOutput.DecodeText(data, length, strdata, 1);
          cmSystemTools::Stdout(strdata);
        }
        if (captureStdOut) {
          cm::append(tempStdOut, data, data + length);
        }
      } else if (pipe == cmsysProcess_Pipe_STDERR) {
        if (outputflag != OUTPUT_NONE) {
          processOutput.DecodeText(data, length, strdata, 2);
          cmSystemTools::Stderr(strdata);
        }
        if (captureStdErr) {
          cm::append(tempStdErr, data, data + length);
        }
      }
    }

    if (outputflag != OUTPUT_NONE) {
      processOutput.DecodeText(std::string(), strdata, 1);
      if (!strdata.empty()) {
        cmSystemTools::Stdout(strdata);
      }
      processOutput.DecodeText(std::string(), strdata, 2);
      if (!strdata.empty()) {
        cmSystemTools::Stderr(strdata);
      }
    }
  }

  cmsysProcess_WaitForExit(cp, nullptr);

  if (captureStdOut) {
    captureStdOut->assign(tempStdOut.begin(), tempStdOut.end());
    processOutput.DecodeText(*captureStdOut, *captureStdOut);
  }
  if (captureStdErr) {
    captureStdErr->assign(tempStdErr.begin(), tempStdErr.end());
    processOutput.DecodeText(*captureStdErr, *captureStdErr);
  }

  bool result = true;
  if (cmsysProcess_GetState(cp) == cmsysProcess_State_Exited) {
    if (retVal) {
      *retVal = cmsysProcess_GetExitValue(cp);
    } else {
      if (cmsysProcess_GetExitValue(cp) != 0) {
        result = false;
      }
    }
  } else if (cmsysProcess_GetState(cp) == cmsysProcess_State_Exception) {
    const char* exception_str = cmsysProcess_GetExceptionString(cp);
    if (outputflag != OUTPUT_NONE) {
      std::cerr << exception_str << std::endl;
    }
    if (captureStdErr) {
      captureStdErr->append(exception_str, strlen(exception_str));
    } else if (captureStdOut) {
      captureStdOut->append(exception_str, strlen(exception_str));
    }
    result = false;
  } else if (cmsysProcess_GetState(cp) == cmsysProcess_State_Error) {
    const char* error_str = cmsysProcess_GetErrorString(cp);
    if (outputflag != OUTPUT_NONE) {
      std::cerr << error_str << std::endl;
    }
    if (captureStdErr) {
      captureStdErr->append(error_str, strlen(error_str));
    } else if (captureStdOut) {
      captureStdOut->append(error_str, strlen(error_str));
    }
    result = false;
  } else if (cmsysProcess_GetState(cp) == cmsysProcess_State_Expired) {
    const char* error_str = "Process terminated due to timeout\n";
    if (outputflag != OUTPUT_NONE) {
      std::cerr << error_str << std::endl;
    }
    if (captureStdErr) {
      captureStdErr->append(error_str, strlen(error_str));
    }
    result = false;
  }

  cmsysProcess_Delete(cp);
  return result;
}

bool cmSystemTools::RunSingleCommand(const std::string& command,
                                     std::string* captureStdOut,
                                     std::string* captureStdErr, int* retVal,
                                     const char* dir, OutputOption outputflag,
                                     cmDuration timeout)
{
  if (s_DisableRunCommandOutput) {
    outputflag = OUTPUT_NONE;
  }

  std::vector<std::string> args = cmSystemTools::ParseArguments(command);

  if (args.empty()) {
    return false;
  }
  return cmSystemTools::RunSingleCommand(args, captureStdOut, captureStdErr,
                                         retVal, dir, outputflag, timeout);
}

std::string cmSystemTools::PrintSingleCommand(
  std::vector<std::string> const& command)
{
  if (command.empty()) {
    return std::string();
  }

  return cmWrap('"', command, '"', " ");
}

bool cmSystemTools::DoesFileExistWithExtensions(
  const std::string& name, const std::vector<std::string>& headerExts)
{
  std::string hname;

  for (std::string const& headerExt : headerExts) {
    hname = cmStrCat(name, '.', headerExt);
    if (cmSystemTools::FileExists(hname)) {
      return true;
    }
  }
  return false;
}

std::string cmSystemTools::FileExistsInParentDirectories(
  const std::string& fname, const std::string& directory,
  const std::string& toplevel)
{
  std::string file = fname;
  cmSystemTools::ConvertToUnixSlashes(file);
  std::string dir = directory;
  cmSystemTools::ConvertToUnixSlashes(dir);
  std::string prevDir;
  while (dir != prevDir) {
    std::string path = cmStrCat(dir, "/", file);
    if (cmSystemTools::FileExists(path)) {
      return path;
    }
    if (dir.size() < toplevel.size()) {
      break;
    }
    prevDir = dir;
    dir = cmSystemTools::GetParentDirectory(dir);
  }
  return "";
}

#ifdef _WIN32
namespace {

/* Helper class to save and restore the specified file (or directory)
   attribute bits. Instantiate this class as an automatic variable on the
   stack. Its constructor saves a copy of the file attributes, and then its
   destructor restores the original attribute settings.  */
class SaveRestoreFileAttributes
{
public:
  SaveRestoreFileAttributes(std::wstring const& path,
                            uint32_t file_attrs_to_set);
  ~SaveRestoreFileAttributes();

  SaveRestoreFileAttributes(SaveRestoreFileAttributes const&) = delete;
  SaveRestoreFileAttributes& operator=(SaveRestoreFileAttributes const&) =
    delete;

  void SetPath(std::wstring const& path) { path_ = path; }

private:
  std::wstring path_;
  uint32_t original_attr_bits_;
};

SaveRestoreFileAttributes::SaveRestoreFileAttributes(
  std::wstring const& path, uint32_t file_attrs_to_set)
  : path_(path)
  , original_attr_bits_(0)
{
  // Set the specified attributes for the source file/directory.
  original_attr_bits_ = GetFileAttributesW(path_.c_str());
  if ((INVALID_FILE_ATTRIBUTES != original_attr_bits_) &&
      ((file_attrs_to_set & original_attr_bits_) != file_attrs_to_set)) {
    SetFileAttributesW(path_.c_str(), original_attr_bits_ | file_attrs_to_set);
  }
}

// We set attribute bits.  Now we need to restore their original state.
SaveRestoreFileAttributes::~SaveRestoreFileAttributes()
{
  DWORD last_error = GetLastError();
  // Verify or restore the original attributes.
  const DWORD source_attr_bits = GetFileAttributesW(path_.c_str());
  if (INVALID_FILE_ATTRIBUTES != source_attr_bits) {
    if (original_attr_bits_ != source_attr_bits) {
      // The file still exists, and its attributes aren't our saved values.
      // Time to restore them.
      SetFileAttributesW(path_.c_str(), original_attr_bits_);
    }
  }
  SetLastError(last_error);
}

struct WindowsFileRetryInit
{
  cmSystemTools::WindowsFileRetry Retry;
  bool Explicit;
};

WindowsFileRetryInit InitWindowsFileRetry(wchar_t const* const values[2],
                                          unsigned int const defaults[2])
{
  unsigned int data[2] = { 0, 0 };
  HKEY const keys[2] = { HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE };
  for (int k = 0; k < 2; ++k) {
    HKEY hKey;
    if (RegOpenKeyExW(keys[k], L"Software\\Kitware\\CMake\\Config", 0,
                      KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) {
      for (int v = 0; v < 2; ++v) {
        DWORD dwData, dwType, dwSize = 4;
        if (!data[v] &&
            RegQueryValueExW(hKey, values[v], 0, &dwType, (BYTE*)&dwData,
                             &dwSize) == ERROR_SUCCESS &&
            dwType == REG_DWORD && dwSize == 4) {
          data[v] = static_cast<unsigned int>(dwData);
        }
      }
      RegCloseKey(hKey);
    }
  }
  WindowsFileRetryInit init;
  init.Explicit = data[0] || data[1];
  init.Retry.Count = data[0] ? data[0] : defaults[0];
  init.Retry.Delay = data[1] ? data[1] : defaults[1];
  return init;
}

WindowsFileRetryInit InitWindowsFileRetry()
{
  static wchar_t const* const values[2] = { L"FilesystemRetryCount",
                                            L"FilesystemRetryDelay" };
  static unsigned int const defaults[2] = { 5, 500 };
  return InitWindowsFileRetry(values, defaults);
}

WindowsFileRetryInit InitWindowsDirectoryRetry()
{
  static wchar_t const* const values[2] = { L"FilesystemDirectoryRetryCount",
                                            L"FilesystemDirectoryRetryDelay" };
  static unsigned int const defaults[2] = { 120, 500 };
  WindowsFileRetryInit dirInit = InitWindowsFileRetry(values, defaults);
  if (dirInit.Explicit) {
    return dirInit;
  }
  WindowsFileRetryInit fileInit = InitWindowsFileRetry();
  if (fileInit.Explicit) {
    return fileInit;
  }
  return dirInit;
}

cmSystemTools::WindowsFileRetry GetWindowsRetry(std::wstring const& path)
{
  // If we are performing a directory operation, then try and get the
  // appropriate timing info.
  DWORD const attrs = GetFileAttributesW(path.c_str());
  if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
    return cmSystemTools::GetWindowsDirectoryRetry();
  }
  return cmSystemTools::GetWindowsFileRetry();
}

} // end of anonymous namespace

cmSystemTools::WindowsFileRetry cmSystemTools::GetWindowsFileRetry()
{
  static WindowsFileRetry retry = InitWindowsFileRetry().Retry;
  return retry;
}

cmSystemTools::WindowsFileRetry cmSystemTools::GetWindowsDirectoryRetry()
{
  static cmSystemTools::WindowsFileRetry retry =
    InitWindowsDirectoryRetry().Retry;
  return retry;
}

cmSystemTools::WindowsVersion cmSystemTools::GetWindowsVersion()
{
  /* Windows version number data.  */
  OSVERSIONINFOEXW osviex;
  ZeroMemory(&osviex, sizeof(osviex));
  osviex.dwOSVersionInfoSize = sizeof(osviex);

#  ifdef CM_WINDOWS_DEPRECATED_GetVersionEx
#    pragma warning(push)
#    ifdef __INTEL_COMPILER
#      pragma warning(disable : 1478)
#    elif defined __clang__
#      pragma clang diagnostic push
#      pragma clang diagnostic ignored "-Wdeprecated-declarations"
#    else
#      pragma warning(disable : 4996)
#    endif
#  endif
  GetVersionExW((OSVERSIONINFOW*)&osviex);
#  ifdef CM_WINDOWS_DEPRECATED_GetVersionEx
#    ifdef __clang__
#      pragma clang diagnostic pop
#    else
#      pragma warning(pop)
#    endif
#  endif

  WindowsVersion result;
  result.dwMajorVersion = osviex.dwMajorVersion;
  result.dwMinorVersion = osviex.dwMinorVersion;
  result.dwBuildNumber = osviex.dwBuildNumber;
  return result;
}
#endif

std::string cmSystemTools::GetRealPathResolvingWindowsSubst(
  const std::string& path, std::string* errorMessage)
{
#ifdef _WIN32
  // uv_fs_realpath uses Windows Vista API so fallback to kwsys if not found
  std::string resolved_path;
  uv_fs_t req;
  int err = uv_fs_realpath(nullptr, &req, path.c_str(), nullptr);
  if (!err) {
    resolved_path = std::string((char*)req.ptr);
    cmSystemTools::ConvertToUnixSlashes(resolved_path);
    // Normalize to upper-case drive letter as GetActualCaseForPath does.
    if (resolved_path.size() > 1 && resolved_path[1] == ':') {
      resolved_path[0] = toupper(resolved_path[0]);
    }
  } else if (err == UV_ENOSYS) {
    resolved_path = cmsys::SystemTools::GetRealPath(path, errorMessage);
  } else if (errorMessage) {
    LPSTR message = nullptr;
    DWORD size = FormatMessageA(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
      nullptr, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&message,
      0, nullptr);
    *errorMessage = std::string(message, size);
    LocalFree(message);

    resolved_path = "";
  } else {
    resolved_path = path;
  }
  return resolved_path;
#else
  return cmsys::SystemTools::GetRealPath(path, errorMessage);
#endif
}

void cmSystemTools::InitializeLibUV()
{
#if defined(_WIN32)
  // Perform libuv one-time initialization now, and then un-do its
  // global _fmode setting so that using libuv does not change the
  // default file text/binary mode.  See libuv issue 840.
  if (uv_loop_t* loop = uv_default_loop()) {
    uv_loop_close(loop);
  }
#  ifdef _MSC_VER
  _set_fmode(_O_TEXT);
#  else
  _fmode = _O_TEXT;
#  endif
  // Replace libuv's report handler with our own to suppress popups.
  cmSystemTools::EnableMSVCDebugHook();
#endif
}

#if defined(_WIN32)
#  include <random>

#  include <wctype.h>
#  ifdef _MSC_VER
using mode_t = cmSystemTools::SystemTools::mode_t;
#  endif
#else
#  include <sys/stat.h>
#endif

inline int Mkdir(const char* dir, const mode_t* mode)
{
#if defined(_WIN32)
  int ret = _wmkdir(cmSystemTools::ConvertToWindowsExtendedPath(dir).c_str());
  if (ret == 0 && mode)
    cmSystemTools::SystemTools::SetPermissions(dir, *mode);
  return ret;
#else
  return mkdir(dir, mode ? *mode : 0777);
#endif
}

cmsys::Status cmSystemTools::MakeTempDirectory(std::string& path,
                                               const mode_t* mode)
{
  if (path.empty()) {
    return cmsys::Status::POSIX(EINVAL);
  }
  return cmSystemTools::MakeTempDirectory(&path.front(), mode);
}

cmsys::Status cmSystemTools::MakeTempDirectory(char* path, const mode_t* mode)
{
  if (!path) {
    return cmsys::Status::POSIX(EINVAL);
  }

  // verify that path ends with "XXXXXX"
  const auto l = std::strlen(path);
  if (!cmHasLiteralSuffix(cm::string_view{ path, l }, "XXXXXX")) {
    return cmsys::Status::POSIX(EINVAL);
  }

  // create parent directories
  auto* sep = path;
  while ((sep = strchr(sep, '/'))) {
    // all underlying functions use C strings,
    // so temporarily end the string here
    *sep = '\0';
    Mkdir(path, mode);

    *sep = '/';
    ++sep;
  }

#ifdef _WIN32
  const int nchars = 36;
  const char chars[nchars + 1] = "abcdefghijklmnopqrstuvwxyz0123456789";

  std::random_device rd;
  std::mt19937 rg{ rd() };
  std::uniform_int_distribution<int> dist{ 0, nchars - 1 };

  for (auto tries = 100; tries; --tries) {
    for (auto n = l - 6; n < l; ++n) {
      path[n] = chars[dist(rg)];
    }
    if (Mkdir(path, mode) == 0) {
      return cmsys::Status::Success();
    } else if (errno != EEXIST) {
      return cmsys::Status::POSIX_errno();
    }
  }
  return cmsys::Status::POSIX(EAGAIN);
#else
  if (mkdtemp(path)) {
    if (mode) {
      chmod(path, *mode);
    }
  } else {
    return cmsys::Status::POSIX_errno();
  }
  return cmsys::Status::Success();
#endif
}

#ifdef _WIN32
namespace {
bool cmMoveFile(std::wstring const& oldname, std::wstring const& newname,
                cmSystemTools::Replace replace)
{
  // Not only ignore any previous error, but clear any memory of it.
  SetLastError(0);

  DWORD flags = 0;
  if (replace == cmSystemTools::Replace::Yes) {
    // Use MOVEFILE_REPLACE_EXISTING to replace an existing destination file.
    flags = flags | MOVEFILE_REPLACE_EXISTING;
  }

  return MoveFileExW(oldname.c_str(), newname.c_str(), flags);
}
}
#endif

cmSystemTools::CopyResult cmSystemTools::CopySingleFile(
  std::string const& oldname, std::string const& newname, CopyWhen when,
  CopyInputRecent inputRecent, std::string* err)
{
  switch (when) {
    case CopyWhen::Always:
      break;
    case CopyWhen::OnlyIfDifferent:
      if (!FilesDiffer(oldname, newname)) {
        return CopyResult::Success;
      }
      break;
  }

  mode_t perm = 0;
  cmsys::Status perms = SystemTools::GetPermissions(oldname, perm);

  // If files are the same do not copy
  if (SystemTools::SameFile(oldname, newname)) {
    return CopyResult::Success;
  }

  cmsys::SystemTools::CopyStatus status;
  status = cmsys::SystemTools::CloneFileContent(oldname, newname);
  if (!status) {
    // if cloning did not succeed, fall back to blockwise copy
#ifdef _WIN32
    if (inputRecent == CopyInputRecent::Yes) {
      // Windows sometimes locks a file immediately after creation.
      // Retry a few times.
      WindowsFileRetry retry = cmSystemTools::GetWindowsFileRetry();
      while ((status =
                cmsys::SystemTools::CopyFileContentBlockwise(oldname, newname),
              status.Path == cmsys::SystemTools::CopyStatus::SourcePath &&
                status.GetPOSIX() == EACCES && --retry.Count)) {
        cmSystemTools::Delay(retry.Delay);
      }
    } else {
      status = cmsys::SystemTools::CopyFileContentBlockwise(oldname, newname);
    }
#else
    static_cast<void>(inputRecent);
    status = cmsys::SystemTools::CopyFileContentBlockwise(oldname, newname);
#endif
  }
  if (!status) {
    if (err) {
      *err = status.GetString();
      switch (status.Path) {
        case cmsys::SystemTools::CopyStatus::SourcePath:
          *err = cmStrCat(*err, " (input)");
          break;
        case cmsys::SystemTools::CopyStatus::DestPath:
          *err = cmStrCat(*err, " (output)");
          break;
        default:
          break;
      }
    }
    return CopyResult::Failure;
  }
  if (perms) {
    perms = SystemTools::SetPermissions(newname, perm);
    if (!perms) {
      if (err) {
        *err = cmStrCat(perms.GetString(), " (output)");
      }
      return CopyResult::Failure;
    }
  }
  return CopyResult::Success;
}

bool cmSystemTools::RenameFile(const std::string& oldname,
                               const std::string& newname)
{
  return cmSystemTools::RenameFile(oldname, newname, Replace::Yes) ==
    RenameResult::Success;
}

cmSystemTools::RenameResult cmSystemTools::RenameFile(
  std::string const& oldname, std::string const& newname, Replace replace,
  std::string* err)
{
#ifdef _WIN32
#  ifndef INVALID_FILE_ATTRIBUTES
#    define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#  endif
  std::wstring const oldname_wstr =
    SystemTools::ConvertToWindowsExtendedPath(oldname);
  std::wstring const newname_wstr =
    SystemTools::ConvertToWindowsExtendedPath(newname);

  /* Windows MoveFileEx may not replace read-only or in-use files.  If it
     fails then remove the read-only attribute from any existing destination.
     Try multiple times since we may be racing against another process
     creating/opening the destination file just before our MoveFileEx.  */
  WindowsFileRetry retry = GetWindowsRetry(oldname_wstr);

  // Use RAII to set the attribute bit blocking Microsoft Search Indexing,
  // and restore the previous value upon return.
  SaveRestoreFileAttributes save_restore_file_attributes(
    oldname_wstr, FILE_ATTRIBUTE_NOT_CONTENT_INDEXED);

  DWORD move_last_error = 0;
  while (!cmMoveFile(oldname_wstr, newname_wstr, replace) && --retry.Count) {
    move_last_error = GetLastError();

    // There was no error ==> the operation is not yet complete.
    if (move_last_error == NO_ERROR) {
      break;
    }

    // Try again only if failure was due to access/sharing permissions.
    // Most often ERROR_ACCESS_DENIED (a.k.a. I/O error) for a directory, and
    // ERROR_SHARING_VIOLATION for a file, are caused by one of the following:
    // 1) Anti-Virus Software
    // 2) Windows Search Indexer
    // 3) Windows Explorer has an associated directory already opened.
    if (move_last_error != ERROR_ACCESS_DENIED &&
        move_last_error != ERROR_SHARING_VIOLATION) {
      if (replace == Replace::No && move_last_error == ERROR_ALREADY_EXISTS) {
        return RenameResult::NoReplace;
      }
      if (err) {
        *err = cmsys::Status::Windows(move_last_error).GetString();
      }
      return RenameResult::Failure;
    }

    DWORD const attrs = GetFileAttributesW(newname_wstr.c_str());
    if ((attrs != INVALID_FILE_ATTRIBUTES) &&
        (attrs & FILE_ATTRIBUTE_READONLY) &&
        // FILE_ATTRIBUTE_READONLY is not honored on directories.
        !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
      // Remove the read-only attribute from the destination file.
      SetFileAttributesW(newname_wstr.c_str(),
                         attrs & ~FILE_ATTRIBUTE_READONLY);
    } else {
      // The file may be temporarily in use so wait a bit.
      cmSystemTools::Delay(retry.Delay);
    }
  }

  // If we were successful, then there was no error.
  if (retry.Count > 0) {
    move_last_error = 0;
    // Restore the attributes on the new name.
    save_restore_file_attributes.SetPath(newname_wstr);
  }
  SetLastError(move_last_error);
  if (retry.Count > 0) {
    return RenameResult::Success;
  }
  if (replace == Replace::No && GetLastError() == ERROR_ALREADY_EXISTS) {
    return RenameResult::NoReplace;
  }
  if (err) {
    *err = cmsys::Status::Windows_GetLastError().GetString();
  }
  return RenameResult::Failure;
#else
  // On UNIX we have OS-provided calls to create 'newname' atomically.
  if (replace == Replace::No) {
    if (link(oldname.c_str(), newname.c_str()) == 0) {
      return RenameResult::Success;
    }
    if (errno == EEXIST) {
      return RenameResult::NoReplace;
    }
    if (err) {
      *err = cmsys::Status::POSIX_errno().GetString();
    }
    return RenameResult::Failure;
  }
  if (rename(oldname.c_str(), newname.c_str()) == 0) {
    return RenameResult::Success;
  }
  if (err) {
    *err = cmsys::Status::POSIX_errno().GetString();
  }
  return RenameResult::Failure;
#endif
}

void cmSystemTools::MoveFileIfDifferent(const std::string& source,
                                        const std::string& destination)
{
  if (FilesDiffer(source, destination)) {
    if (RenameFile(source, destination)) {
      return;
    }
    CopyFileAlways(source, destination);
  }
  RemoveFile(source);
}

#ifndef CMAKE_BOOTSTRAP
std::string cmSystemTools::ComputeFileHash(const std::string& source,
                                           cmCryptoHash::Algo algo)
{
  cmCryptoHash hash(algo);
  return hash.HashFile(source);
}

std::string cmSystemTools::ComputeStringMD5(const std::string& input)
{
  cmCryptoHash md5(cmCryptoHash::AlgoMD5);
  return md5.HashString(input);
}

#  ifdef _WIN32
std::string cmSystemTools::ComputeCertificateThumbprint(
  const std::string& source)
{
  std::string thumbprint;

  CRYPT_INTEGER_BLOB cryptBlob;
  HCERTSTORE certStore = nullptr;
  PCCERT_CONTEXT certContext = nullptr;

  HANDLE certFile = CreateFileW(
    cmsys::Encoding::ToWide(source.c_str()).c_str(), GENERIC_READ,
    FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

  if (certFile != INVALID_HANDLE_VALUE && certFile != nullptr) {
    DWORD fileSize = GetFileSize(certFile, nullptr);
    if (fileSize != INVALID_FILE_SIZE) {
      auto certData = cm::make_unique<BYTE[]>(fileSize);
      if (certData != nullptr) {
        DWORD dwRead = 0;
        if (ReadFile(certFile, certData.get(), fileSize, &dwRead, nullptr)) {
          cryptBlob.cbData = fileSize;
          cryptBlob.pbData = certData.get();

          // Verify that this is a valid cert
          if (PFXIsPFXBlob(&cryptBlob)) {
            // Open the certificate as a store
            certStore =
              PFXImportCertStore(&cryptBlob, nullptr, CRYPT_EXPORTABLE);
            if (certStore != nullptr) {
              // There should only be 1 cert.
              certContext =
                CertEnumCertificatesInStore(certStore, certContext);
              if (certContext != nullptr) {
                // The hash is 20 bytes
                BYTE hashData[20];
                DWORD hashLength = 20;

                // Buffer to print the hash. Each byte takes 2 chars +
                // terminating character
                char hashPrint[41];
                char* pHashPrint = hashPrint;
                // Get the hash property from the certificate
                if (CertGetCertificateContextProperty(
                      certContext, CERT_HASH_PROP_ID, hashData, &hashLength)) {
                  for (DWORD i = 0; i < hashLength; i++) {
                    // Convert each byte to hexadecimal
                    snprintf(pHashPrint, 3, "%02X", hashData[i]);
                    pHashPrint += 2;
                  }
                  *pHashPrint = '\0';
                  thumbprint = hashPrint;
                }
                CertFreeCertificateContext(certContext);
              }
              CertCloseStore(certStore, 0);
            }
          }
        }
      }
    }
    CloseHandle(certFile);
  }

  return thumbprint;
}
#  endif
#endif

void cmSystemTools::Glob(const std::string& directory,
                         const std::string& regexp,
                         std::vector<std::string>& files)
{
  cmsys::Directory d;
  cmsys::RegularExpression reg(regexp.c_str());

  if (d.Load(directory)) {
    size_t numf;
    unsigned int i;
    numf = d.GetNumberOfFiles();
    for (i = 0; i < numf; i++) {
      std::string fname = d.GetFile(i);
      if (reg.find(fname)) {
        files.push_back(std::move(fname));
      }
    }
  }
}

void cmSystemTools::GlobDirs(const std::string& path,
                             std::vector<std::string>& files)
{
  std::string::size_type pos = path.find("/*");
  if (pos == std::string::npos) {
    files.push_back(path);
    return;
  }
  std::string startPath = path.substr(0, pos);
  std::string finishPath = path.substr(pos + 2);

  cmsys::Directory d;
  if (d.Load(startPath)) {
    for (unsigned int i = 0; i < d.GetNumberOfFiles(); ++i) {
      if ((std::string(d.GetFile(i)) != ".") &&
          (std::string(d.GetFile(i)) != "..")) {
        std::string fname = cmStrCat(startPath, '/', d.GetFile(i));
        if (cmSystemTools::FileIsDirectory(fname)) {
          fname += finishPath;
          cmSystemTools::GlobDirs(fname, files);
        }
      }
    }
  }
}

bool cmSystemTools::SimpleGlob(const std::string& glob,
                               std::vector<std::string>& files,
                               int type /* = 0 */)
{
  files.clear();
  if (glob.back() != '*') {
    return false;
  }
  std::string path = cmSystemTools::GetFilenamePath(glob);
  std::string ppath = cmSystemTools::GetFilenameName(glob);
  ppath = ppath.substr(0, ppath.size() - 1);
  if (path.empty()) {
    path = "/";
  }

  bool res = false;
  cmsys::Directory d;
  if (d.Load(path)) {
    for (unsigned int i = 0; i < d.GetNumberOfFiles(); ++i) {
      if ((std::string(d.GetFile(i)) != ".") &&
          (std::string(d.GetFile(i)) != "..")) {
        std::string fname = path;
        if (path.back() != '/') {
          fname += "/";
        }
        fname += d.GetFile(i);
        std::string sfname = d.GetFile(i);
        if (type > 0 && cmSystemTools::FileIsDirectory(fname)) {
          continue;
        }
        if (type < 0 && !cmSystemTools::FileIsDirectory(fname)) {
          continue;
        }
        if (cmHasPrefix(sfname, ppath)) {
          files.push_back(fname);
          res = true;
        }
      }
    }
  }
  return res;
}

std::string cmSystemTools::ConvertToOutputPath(std::string const& path)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  if (s_ForceUnixPaths) {
    return cmSystemTools::ConvertToUnixOutputPath(path);
  }
  return cmSystemTools::ConvertToWindowsOutputPath(path);
#else
  return cmSystemTools::ConvertToUnixOutputPath(path);
#endif
}

void cmSystemTools::ConvertToOutputSlashes(std::string& path)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  if (!s_ForceUnixPaths) {
    // Convert to windows slashes.
    std::string::size_type pos = 0;
    while ((pos = path.find('/', pos)) != std::string::npos) {
      path[pos++] = '\\';
    }
  }
#else
  static_cast<void>(path);
#endif
}

void cmSystemTools::ConvertToLongPath(std::string& path)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  // Try to convert path to a long path only if the path contains character '~'
  if (path.find('~') == std::string::npos) {
    return;
  }

  std::wstring wPath = cmsys::Encoding::ToWide(path);
  DWORD ret = GetLongPathNameW(wPath.c_str(), nullptr, 0);
  std::vector<wchar_t> buffer(ret);
  if (ret != 0) {
    ret = GetLongPathNameW(wPath.c_str(), buffer.data(),
                           static_cast<DWORD>(buffer.size()));
  }

  if (ret != 0) {
    path = cmsys::Encoding::ToNarrow(buffer.data());
  }
#else
  static_cast<void>(path);
#endif
}

std::string cmSystemTools::ConvertToRunCommandPath(const std::string& path)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  return cmSystemTools::ConvertToWindowsOutputPath(path);
#else
  return cmSystemTools::ConvertToUnixOutputPath(path);
#endif
}

// compute the relative path from here to there
std::string cmSystemTools::RelativePath(std::string const& local,
                                        std::string const& remote)
{
  if (!cmSystemTools::FileIsFullPath(local)) {
    cmSystemTools::Error("RelativePath must be passed a full path to local: " +
                         local);
  }
  if (!cmSystemTools::FileIsFullPath(remote)) {
    cmSystemTools::Error(
      "RelativePath must be passed a full path to remote: " + remote);
  }
  return cmsys::SystemTools::RelativePath(local, remote);
}

std::string cmSystemTools::ForceToRelativePath(std::string const& local_path,
                                               std::string const& remote_path)
{
  // The paths should never be quoted.
  assert(local_path.front() != '\"');
  assert(remote_path.front() != '\"');

  // The local path should never have a trailing slash except if it is just the
  // bare root directory
  assert(local_path.empty() || local_path.back() != '/' ||
         local_path.size() == 1 ||
         (local_path.size() == 3 && local_path[1] == ':' &&
          ((local_path[0] >= 'A' && local_path[0] <= 'Z') ||
           (local_path[0] >= 'a' && local_path[0] <= 'z'))));

  // If the path is already relative then just return the path.
  if (!cmSystemTools::FileIsFullPath(remote_path)) {
    return remote_path;
  }

  // Identify the longest shared path component between the remote
  // path and the local path.
  std::vector<std::string> local;
  cmSystemTools::SplitPath(local_path, local);
  std::vector<std::string> remote;
  cmSystemTools::SplitPath(remote_path, remote);
  unsigned int common = 0;
  while (common < remote.size() && common < local.size() &&
         cmSystemTools::ComparePath(remote[common], local[common])) {
    ++common;
  }

  // If no part of the path is in common then return the full path.
  if (common == 0) {
    return remote_path;
  }

  // If the entire path is in common then just return a ".".
  if (common == remote.size() && common == local.size()) {
    return ".";
  }

  // If the entire path is in common except for a trailing slash then
  // just return a "./".
  if (common + 1 == remote.size() && remote[common].empty() &&
      common == local.size()) {
    return "./";
  }

  // Construct the relative path.
  std::string relative;

  // First add enough ../ to get up to the level of the shared portion
  // of the path.  Leave off the trailing slash.  Note that the last
  // component of local will never be empty because local should never
  // have a trailing slash.
  for (unsigned int i = common; i < local.size(); ++i) {
    relative += "..";
    if (i < local.size() - 1) {
      relative += "/";
    }
  }

  // Now add the portion of the destination path that is not included
  // in the shared portion of the path.  Add a slash the first time
  // only if there was already something in the path.  If there was a
  // trailing slash in the input then the last iteration of the loop
  // will add a slash followed by an empty string which will preserve
  // the trailing slash in the output.

  if (!relative.empty() && !remote.empty()) {
    relative += "/";
  }
  relative += cmJoin(cmMakeRange(remote).advance(common), "/");

  // Finally return the path.
  return relative;
}

std::string cmSystemTools::RelativeIfUnder(std::string const& top,
                                           std::string const& in)
{
  std::string out;
  if (in == top) {
    out = ".";
  } else if (cmSystemTools::IsSubDirectory(in, top)) {
    out = in.substr(top.size() + 1);
  } else {
    out = in;
  }
  return out;
}

cm::optional<std::string> cmSystemTools::GetEnvVar(std::string const& var)
{
  cm::optional<std::string> result;
  {
    std::string value;
    if (cmSystemTools::GetEnv(var, value)) {
      result = std::move(value);
    }
  }
  return result;
}

std::vector<std::string> cmSystemTools::SplitEnvPath(std::string const& value)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  static cm::string_view sep = ";"_s;
#else
  static cm::string_view sep = ":"_s;
#endif
  std::vector<std::string> paths = cmTokenize(value, sep);
  for (std::string& p : paths) {
    SystemTools::ConvertToUnixSlashes(p);
  }
  return paths;
}

#ifndef CMAKE_BOOTSTRAP
bool cmSystemTools::UnsetEnv(const char* value)
{
#  if !defined(HAVE_UNSETENV)
  return cmSystemTools::UnPutEnv(value);
#  else
  unsetenv(value);
  return true;
#  endif
}

std::vector<std::string> cmSystemTools::GetEnvironmentVariables()
{
  std::vector<std::string> env;
  int cc;
#  ifdef _WIN32
  // if program starts with main, _wenviron is initially NULL, call to
  // _wgetenv and create wide-character string environment
  _wgetenv(L"");
  for (cc = 0; _wenviron[cc]; ++cc) {
    env.emplace_back(cmsys::Encoding::ToNarrow(_wenviron[cc]));
  }
#  else
  for (cc = 0; environ[cc]; ++cc) {
    env.emplace_back(environ[cc]);
  }
#  endif
  return env;
}

void cmSystemTools::AppendEnv(std::vector<std::string> const& env)
{
  for (std::string const& eit : env) {
    cmSystemTools::PutEnv(eit);
  }
}

void cmSystemTools::EnvDiff::AppendEnv(std::vector<std::string> const& env)
{
  for (std::string const& eit : env) {
    this->PutEnv(eit);
  }
}

void cmSystemTools::EnvDiff::PutEnv(const std::string& env)
{
  auto const eq_loc = env.find('=');
  if (eq_loc != std::string::npos) {
    std::string name = env.substr(0, eq_loc);
    diff[name] = env.substr(eq_loc + 1);
  } else {
    this->UnPutEnv(env);
  }
}

void cmSystemTools::EnvDiff::UnPutEnv(const std::string& env)
{
  diff[env] = {};
}

bool cmSystemTools::EnvDiff::ParseOperation(const std::string& envmod)
{
  char path_sep = GetSystemPathlistSeparator();

  auto apply_diff = [this](const std::string& name,
                           std::function<void(std::string&)> const& apply) {
    cm::optional<std::string> old_value = diff[name];
    std::string output;
    if (old_value) {
      output = *old_value;
    } else {
      const char* curval = cmSystemTools::GetEnv(name);
      if (curval) {
        output = curval;
      }
    }
    apply(output);
    diff[name] = output;
  };

  // Split on `=`
  auto const eq_loc = envmod.find_first_of('=');
  if (eq_loc == std::string::npos) {
    cmSystemTools::Error(cmStrCat(
      "Error: Missing `=` after the variable name in: ", envmod, '\n'));
    return false;
  }

  auto const name = envmod.substr(0, eq_loc);

  // Split value on `:`
  auto const op_value_start = eq_loc + 1;
  auto const colon_loc = envmod.find_first_of(':', op_value_start);
  if (colon_loc == std::string::npos) {
    cmSystemTools::Error(
      cmStrCat("Error: Missing `:` after the operation in: ", envmod, '\n'));
    return false;
  }
  auto const op = envmod.substr(op_value_start, colon_loc - op_value_start);

  auto const value_start = colon_loc + 1;
  auto const value = envmod.substr(value_start);

  // Determine what to do with the operation.
  if (op == "reset"_s) {
    auto entry = diff.find(name);
    if (entry != diff.end()) {
      diff.erase(entry);
    }
  } else if (op == "set"_s) {
    diff[name] = value;
  } else if (op == "unset"_s) {
    diff[name] = {};
  } else if (op == "string_append"_s) {
    apply_diff(name, [&value](std::string& output) { output += value; });
  } else if (op == "string_prepend"_s) {
    apply_diff(name,
               [&value](std::string& output) { output.insert(0, value); });
  } else if (op == "path_list_append"_s) {
    apply_diff(name, [&value, path_sep](std::string& output) {
      if (!output.empty()) {
        output += path_sep;
      }
      output += value;
    });
  } else if (op == "path_list_prepend"_s) {
    apply_diff(name, [&value, path_sep](std::string& output) {
      if (!output.empty()) {
        output.insert(output.begin(), path_sep);
      }
      output.insert(0, value);
    });
  } else if (op == "cmake_list_append"_s) {
    apply_diff(name, [&value](std::string& output) {
      if (!output.empty()) {
        output += ';';
      }
      output += value;
    });
  } else if (op == "cmake_list_prepend"_s) {
    apply_diff(name, [&value](std::string& output) {
      if (!output.empty()) {
        output.insert(output.begin(), ';');
      }
      output.insert(0, value);
    });
  } else {
    cmSystemTools::Error(cmStrCat(
      "Error: Unrecognized environment manipulation argument: ", op, '\n'));
    return false;
  }

  return true;
}

void cmSystemTools::EnvDiff::ApplyToCurrentEnv(std::ostringstream* measurement)
{
  for (auto const& env_apply : diff) {
    if (env_apply.second) {
      auto const env_update =
        cmStrCat(env_apply.first, '=', *env_apply.second);
      cmSystemTools::PutEnv(env_update);
      if (measurement) {
        *measurement << env_update << std::endl;
      }
    } else {
      cmSystemTools::UnsetEnv(env_apply.first.c_str());
      if (measurement) {
        // Signify that this variable is being actively unset
        *measurement << '#' << env_apply.first << "=\n";
      }
    }
  }
}

cmSystemTools::SaveRestoreEnvironment::SaveRestoreEnvironment()
{
  this->Env = cmSystemTools::GetEnvironmentVariables();
}

cmSystemTools::SaveRestoreEnvironment::~SaveRestoreEnvironment()
{
  // First clear everything in the current environment:
  std::vector<std::string> currentEnv = GetEnvironmentVariables();
  for (std::string var : currentEnv) {
    std::string::size_type pos = var.find('=');
    if (pos != std::string::npos) {
      var = var.substr(0, pos);
    }

    cmSystemTools::UnsetEnv(var.c_str());
  }

  // Then put back each entry from the original environment:
  cmSystemTools::AppendEnv(this->Env);
}
#endif

void cmSystemTools::EnableVSConsoleOutput()
{
#ifdef _WIN32
  // Visual Studio tools like devenv may not
  // display output to the console unless this environment variable is
  // set.  We need it to capture the output of these build tools.
  // Note for future work that one could pass "/out \\.\pipe\NAME" to
  // either of these executables where NAME is created with
  // CreateNamedPipe.  This would bypass the internal buffering of the
  // output and allow it to be captured on the fly.
  cmSystemTools::PutEnv("vsconsoleoutput=1");

#  ifndef CMAKE_BOOTSTRAP
  // VS sets an environment variable to tell MS tools like "cl" to report
  // output through a backdoor pipe instead of stdout/stderr.  Unset the
  // environment variable to close this backdoor for any path of process
  // invocations that passes through CMake so we can capture the output.
  cmSystemTools::UnsetEnv("VS_UNICODE_OUTPUT");
#  endif
#endif
}

bool cmSystemTools::IsPathToFramework(const std::string& path)
{
  return (cmSystemTools::FileIsFullPath(path) &&
          cmHasLiteralSuffix(path, ".framework"));
}

bool cmSystemTools::IsPathToMacOSSharedLibrary(const std::string& path)
{
  return (cmSystemTools::FileIsFullPath(path) &&
          cmHasLiteralSuffix(path, ".dylib"));
}

bool cmSystemTools::CreateTar(const std::string& outFileName,
                              const std::vector<std::string>& files,
                              cmTarCompression compressType, bool verbose,
                              std::string const& mtime,
                              std::string const& format, int compressionLevel)
{
#if !defined(CMAKE_BOOTSTRAP)
  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
  cmsys::ofstream fout(outFileName.c_str(), std::ios::out | std::ios::binary);
  if (!fout) {
    std::string e = cmStrCat("Cannot open output file \"", outFileName,
                             "\": ", cmSystemTools::GetLastSystemError());
    cmSystemTools::Error(e);
    return false;
  }
  cmArchiveWrite::Compress compress = cmArchiveWrite::CompressNone;
  switch (compressType) {
    case TarCompressGZip:
      compress = cmArchiveWrite::CompressGZip;
      break;
    case TarCompressBZip2:
      compress = cmArchiveWrite::CompressBZip2;
      break;
    case TarCompressXZ:
      compress = cmArchiveWrite::CompressXZ;
      break;
    case TarCompressZstd:
      compress = cmArchiveWrite::CompressZstd;
      break;
    case TarCompressNone:
      compress = cmArchiveWrite::CompressNone;
      break;
  }

  cmArchiveWrite a(fout, compress, format.empty() ? "paxr" : format,
                   compressionLevel);

  if (!a.Open()) {
    cmSystemTools::Error(a.GetError());
    return false;
  }
  a.SetMTime(mtime);
  a.SetVerbose(verbose);
  bool tarCreatedSuccessfully = true;
  for (auto path : files) {
    if (cmSystemTools::FileIsFullPath(path)) {
      // Get the relative path to the file.
      path = cmSystemTools::RelativePath(cwd, path);
    }
    if (!a.Add(path)) {
      cmSystemTools::Error(a.GetError());
      tarCreatedSuccessfully = false;
    }
  }
  return tarCreatedSuccessfully;
#else
  (void)outFileName;
  (void)files;
  (void)verbose;
  return false;
#endif
}

#if !defined(CMAKE_BOOTSTRAP)
namespace {
#  define BSDTAR_FILESIZE_PRINTF "%lu"
#  define BSDTAR_FILESIZE_TYPE unsigned long
void list_item_verbose(FILE* out, struct archive_entry* entry)
{
  char tmp[100];
  size_t w;
  const char* p;
  const char* fmt;
  time_t tim;
  static time_t now;
  size_t u_width = 6;
  size_t gs_width = 13;

  /*
   * We avoid collecting the entire list in memory at once by
   * listing things as we see them.  However, that also means we can't
   * just pre-compute the field widths.  Instead, we start with guesses
   * and just widen them as necessary.  These numbers are completely
   * arbitrary.
   */
  if (!now) {
    time(&now);
  }
  fprintf(out, "%s %d ", archive_entry_strmode(entry),
          archive_entry_nlink(entry));

  /* Use uname if it's present, else uid. */
  p = archive_entry_uname(entry);
  if ((p == nullptr) || (*p == '\0')) {
    snprintf(tmp, sizeof(tmp), "%lu ",
             static_cast<unsigned long>(archive_entry_uid(entry)));
    p = tmp;
  }
  w = strlen(p);
  if (w > u_width) {
    u_width = w;
  }
  fprintf(out, "%-*s ", static_cast<int>(u_width), p);
  /* Use gname if it's present, else gid. */
  p = archive_entry_gname(entry);
  if (p != nullptr && p[0] != '\0') {
    fprintf(out, "%s", p);
    w = strlen(p);
  } else {
    snprintf(tmp, sizeof(tmp), "%lu",
             static_cast<unsigned long>(archive_entry_gid(entry)));
    w = strlen(tmp);
    fprintf(out, "%s", tmp);
  }

  /*
   * Print device number or file size, right-aligned so as to make
   * total width of group and devnum/filesize fields be gs_width.
   * If gs_width is too small, grow it.
   */
  if (archive_entry_filetype(entry) == AE_IFCHR ||
      archive_entry_filetype(entry) == AE_IFBLK) {
    unsigned long rdevmajor = archive_entry_rdevmajor(entry);
    unsigned long rdevminor = archive_entry_rdevminor(entry);
    snprintf(tmp, sizeof(tmp), "%lu,%lu", rdevmajor, rdevminor);
  } else {
    /*
     * Note the use of platform-dependent macros to format
     * the filesize here.  We need the format string and the
     * corresponding type for the cast.
     */
    snprintf(tmp, sizeof(tmp), BSDTAR_FILESIZE_PRINTF,
             static_cast<BSDTAR_FILESIZE_TYPE>(archive_entry_size(entry)));
  }
  if (w + strlen(tmp) >= gs_width) {
    gs_width = w + strlen(tmp) + 1;
  }
  fprintf(out, "%*s", static_cast<int>(gs_width - w), tmp);

  /* Format the time using 'ls -l' conventions. */
  tim = archive_entry_mtime(entry);
#  define HALF_YEAR ((time_t)365 * 86400 / 2)
#  if defined(_WIN32) && !defined(__CYGWIN__)
/* Windows' strftime function does not support %e format. */
#    define DAY_FMT "%d"
#  else
#    define DAY_FMT "%e" /* Day number without leading zeros */
#  endif
  if (tim < now - HALF_YEAR || tim > now + HALF_YEAR) {
    fmt = DAY_FMT " %b  %Y";
  } else {
    fmt = DAY_FMT " %b %H:%M";
  }
  strftime(tmp, sizeof(tmp), fmt, localtime(&tim));
  fprintf(out, " %s ", tmp);
  fprintf(out, "%s", cm_archive_entry_pathname(entry).c_str());

  /* Extra information for links. */
  if (archive_entry_hardlink(entry)) /* Hard link */
  {
    fprintf(out, " link to %s", archive_entry_hardlink(entry));
  } else if (archive_entry_symlink(entry)) /* Symbolic link */
  {
    fprintf(out, " -> %s", archive_entry_symlink(entry));
  }
  fflush(out);
}

void ArchiveError(const char* m1, struct archive* a)
{
  std::string message(m1);
  const char* m2 = archive_error_string(a);
  if (m2) {
    message += m2;
  }
  cmSystemTools::Error(message);
}

bool la_diagnostic(struct archive* ar, __LA_SSIZE_T r)
{
  // See archive.h definition of ARCHIVE_OK for return values.

  if (r >= ARCHIVE_OK) {
    return true;
  }

  if (r >= ARCHIVE_WARN) {
    const char* warn = archive_error_string(ar);
    if (!warn) {
      warn = "unknown warning";
    }
    std::cerr << "cmake -E tar: warning: " << warn << '\n';
    return true;
  }

  // Error.
  const char* err = archive_error_string(ar);
  if (!err) {
    err = "unknown error";
  }
  std::cerr << "cmake -E tar: error: " << err << '\n';
  return false;
}

// Return 'true' on success
bool copy_data(struct archive* ar, struct archive* aw)
{
  long r;
  const void* buff;
  size_t size;
#  if defined(ARCHIVE_VERSION_NUMBER) && ARCHIVE_VERSION_NUMBER >= 3000000
  __LA_INT64_T offset;
#  else
  off_t offset;
#  endif

  for (;;) {
    // See archive.h definition of ARCHIVE_OK for return values.
    r = archive_read_data_block(ar, &buff, &size, &offset);
    if (r == ARCHIVE_EOF) {
      return true;
    }
    if (!la_diagnostic(ar, r)) {
      return false;
    }
    // See archive.h definition of ARCHIVE_OK for return values.
    __LA_SSIZE_T const w = archive_write_data_block(aw, buff, size, offset);
    if (!la_diagnostic(ar, w)) {
      return false;
    }
  }
#  if !defined(__clang__) && !defined(__NVCOMPILER) && !defined(__HP_aCC)
  return false; /* this should not happen but it quiets some compilers */
#  endif
}

bool extract_tar(const std::string& outFileName,
                 const std::vector<std::string>& files, bool verbose,
                 cmSystemTools::cmTarExtractTimestamps extractTimestamps,
                 bool extract)
{
  cmLocaleRAII localeRAII;
  static_cast<void>(localeRAII);
  struct archive* a = archive_read_new();
  struct archive* ext = archive_write_disk_new();
  archive_read_support_filter_all(a);
  archive_read_support_format_all(a);
  struct archive_entry* entry;

  struct archive* matching = archive_match_new();
  if (matching == nullptr) {
    cmSystemTools::Error("Out of memory");
    return false;
  }

  for (const auto& filename : files) {
    if (archive_match_include_pattern(matching, filename.c_str()) !=
        ARCHIVE_OK) {
      cmSystemTools::Error("Failed to add to inclusion list: " + filename);
      return false;
    }
  }

  int r = cm_archive_read_open_file(a, outFileName.c_str(), 10240);
  if (r) {
    ArchiveError("Problem with archive_read_open_file(): ", a);
    archive_write_free(ext);
    archive_read_close(a);
    return false;
  }
  for (;;) {
    r = archive_read_next_header(a, &entry);
    if (r == ARCHIVE_EOF) {
      break;
    }
    if (r != ARCHIVE_OK) {
      ArchiveError("Problem with archive_read_next_header(): ", a);
      break;
    }

    if (archive_match_excluded(matching, entry)) {
      continue;
    }

    if (verbose) {
      if (extract) {
        cmSystemTools::Stdout("x ");
        cmSystemTools::Stdout(cm_archive_entry_pathname(entry));
      } else {
        list_item_verbose(stdout, entry);
      }
      cmSystemTools::Stdout("\n");
    } else if (!extract) {
      cmSystemTools::Stdout(cm_archive_entry_pathname(entry));
      cmSystemTools::Stdout("\n");
    }
    if (extract) {
      if (extractTimestamps == cmSystemTools::cmTarExtractTimestamps::Yes) {
        r = archive_write_disk_set_options(ext, ARCHIVE_EXTRACT_TIME);
        if (r != ARCHIVE_OK) {
          ArchiveError("Problem with archive_write_disk_set_options(): ", ext);
          break;
        }
      }

      r = archive_write_header(ext, entry);
      if (r == ARCHIVE_OK) {
        if (!copy_data(a, ext)) {
          break;
        }
        r = archive_write_finish_entry(ext);
        if (r != ARCHIVE_OK) {
          ArchiveError("Problem with archive_write_finish_entry(): ", ext);
          break;
        }
      }
#  ifdef _WIN32
      else if (const char* linktext = archive_entry_symlink(entry)) {
        std::cerr << "cmake -E tar: warning: skipping symbolic link \""
                  << cm_archive_entry_pathname(entry) << "\" -> \"" << linktext
                  << "\"." << std::endl;
      }
#  endif
      else {
        ArchiveError("Problem with archive_write_header(): ", ext);
        cmSystemTools::Error("Current file: " +
                             cm_archive_entry_pathname(entry));
        break;
      }
    }
  }

  bool error_occured = false;
  if (matching != nullptr) {
    const char* p;
    int ar;

    while ((ar = archive_match_path_unmatched_inclusions_next(matching, &p)) ==
           ARCHIVE_OK) {
      cmSystemTools::Error("tar: " + std::string(p) +
                           ": Not found in archive");
      error_occured = true;
    }
    if (error_occured) {
      return false;
    }
    if (ar == ARCHIVE_FATAL) {
      cmSystemTools::Error("tar: Out of memory");
      return false;
    }
  }
  archive_match_free(matching);
  archive_write_free(ext);
  archive_read_close(a);
  archive_read_free(a);
  return r == ARCHIVE_EOF || r == ARCHIVE_OK;
}
}
#endif

bool cmSystemTools::ExtractTar(const std::string& outFileName,
                               const std::vector<std::string>& files,
                               cmTarExtractTimestamps extractTimestamps,
                               bool verbose)
{
#if !defined(CMAKE_BOOTSTRAP)
  return extract_tar(outFileName, files, verbose, extractTimestamps, true);
#else
  (void)outFileName;
  (void)files;
  (void)extractTimestamps;
  (void)verbose;
  return false;
#endif
}

bool cmSystemTools::ListTar(const std::string& outFileName,
                            const std::vector<std::string>& files,
                            bool verbose)
{
#if !defined(CMAKE_BOOTSTRAP)
  return extract_tar(outFileName, files, verbose, cmTarExtractTimestamps::Yes,
                     false);
#else
  (void)outFileName;
  (void)files;
  (void)verbose;
  return false;
#endif
}

int cmSystemTools::WaitForLine(cmsysProcess* process, std::string& line,
                               cmDuration timeout, std::vector<char>& out,
                               std::vector<char>& err)
{
  line.clear();
  auto outiter = out.begin();
  auto erriter = err.begin();
  cmProcessOutput processOutput;
  std::string strdata;
  while (true) {
    // Check for a newline in stdout.
    for (; outiter != out.end(); ++outiter) {
      if ((*outiter == '\r') && ((outiter + 1) == out.end())) {
        break;
      }
      if (*outiter == '\n' || *outiter == '\0') {
        std::vector<char>::size_type length = outiter - out.begin();
        if (length > 1 && *(outiter - 1) == '\r') {
          --length;
        }
        if (length > 0) {
          line.append(out.data(), length);
        }
        out.erase(out.begin(), outiter + 1);
        return cmsysProcess_Pipe_STDOUT;
      }
    }

    // Check for a newline in stderr.
    for (; erriter != err.end(); ++erriter) {
      if ((*erriter == '\r') && ((erriter + 1) == err.end())) {
        break;
      }
      if (*erriter == '\n' || *erriter == '\0') {
        std::vector<char>::size_type length = erriter - err.begin();
        if (length > 1 && *(erriter - 1) == '\r') {
          --length;
        }
        if (length > 0) {
          line.append(err.data(), length);
        }
        err.erase(err.begin(), erriter + 1);
        return cmsysProcess_Pipe_STDERR;
      }
    }

    // No newlines found.  Wait for more data from the process.
    int length;
    char* data;
    double timeoutAsDbl = timeout.count();
    int pipe =
      cmsysProcess_WaitForData(process, &data, &length, &timeoutAsDbl);
    if (pipe == cmsysProcess_Pipe_Timeout) {
      // Timeout has been exceeded.
      return pipe;
    }
    if (pipe == cmsysProcess_Pipe_STDOUT) {
      processOutput.DecodeText(data, length, strdata, 1);
      // Append to the stdout buffer.
      std::vector<char>::size_type size = out.size();
      cm::append(out, strdata);
      outiter = out.begin() + size;
    } else if (pipe == cmsysProcess_Pipe_STDERR) {
      processOutput.DecodeText(data, length, strdata, 2);
      // Append to the stderr buffer.
      std::vector<char>::size_type size = err.size();
      cm::append(err, strdata);
      erriter = err.begin() + size;
    } else if (pipe == cmsysProcess_Pipe_None) {
      // Both stdout and stderr pipes have broken.  Return leftover data.
      processOutput.DecodeText(std::string(), strdata, 1);
      if (!strdata.empty()) {
        std::vector<char>::size_type size = out.size();
        cm::append(out, strdata);
        outiter = out.begin() + size;
      }
      processOutput.DecodeText(std::string(), strdata, 2);
      if (!strdata.empty()) {
        std::vector<char>::size_type size = err.size();
        cm::append(err, strdata);
        erriter = err.begin() + size;
      }
      if (!out.empty()) {
        line.append(out.data(), outiter - out.begin());
        out.erase(out.begin(), out.end());
        return cmsysProcess_Pipe_STDOUT;
      }
      if (!err.empty()) {
        line.append(err.data(), erriter - err.begin());
        err.erase(err.begin(), err.end());
        return cmsysProcess_Pipe_STDERR;
      }
      return cmsysProcess_Pipe_None;
    }
  }
}

#ifdef _WIN32
static void EnsureStdPipe(DWORD fd)
{
  if (GetStdHandle(fd) != INVALID_HANDLE_VALUE) {
    return;
  }
  SECURITY_ATTRIBUTES sa;
  sa.nLength = sizeof(sa);
  sa.lpSecurityDescriptor = nullptr;
  sa.bInheritHandle = TRUE;

  HANDLE h = CreateFileW(
    L"NUL",
    fd == STD_INPUT_HANDLE ? FILE_GENERIC_READ
                           : FILE_GENERIC_WRITE | FILE_READ_ATTRIBUTES,
    FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, OPEN_EXISTING, 0, nullptr);

  if (h == INVALID_HANDLE_VALUE) {
    LPSTR message = nullptr;
    DWORD size = FormatMessageA(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
      nullptr, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPSTR)&message, 0, nullptr);
    std::string msg = std::string(message, size);
    LocalFree(message);
    std::cerr << "failed to open NUL for missing stdio pipe: " << msg;
    abort();
  }

  SetStdHandle(fd, h);
}

void cmSystemTools::EnsureStdPipes()
{
  EnsureStdPipe(STD_INPUT_HANDLE);
  EnsureStdPipe(STD_OUTPUT_HANDLE);
  EnsureStdPipe(STD_ERROR_HANDLE);
}
#else
static void EnsureStdPipe(int fd)
{
  if (fcntl(fd, F_GETFD) != -1 || errno != EBADF) {
    return;
  }

  int f = open("/dev/null", fd == STDIN_FILENO ? O_RDONLY : O_WRONLY);
  if (f == -1) {
    perror("failed to open /dev/null for missing stdio pipe");
    abort();
  }
  if (f != fd) {
    dup2(f, fd);
    close(f);
  }
}

void cmSystemTools::EnsureStdPipes()
{
  EnsureStdPipe(STDIN_FILENO);
  EnsureStdPipe(STDOUT_FILENO);
  EnsureStdPipe(STDERR_FILENO);
}
#endif

void cmSystemTools::DoNotInheritStdPipes()
{
#ifdef _WIN32
  // Check to see if we are attached to a console
  // if so, then do not stop the inherited pipes
  // or stdout and stderr will not show up in dos
  // shell windows
  CONSOLE_SCREEN_BUFFER_INFO hOutInfo;
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (GetConsoleScreenBufferInfo(hOut, &hOutInfo)) {
    return;
  }
  {
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    DuplicateHandle(GetCurrentProcess(), out, GetCurrentProcess(), &out, 0,
                    FALSE, DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE);
    SetStdHandle(STD_OUTPUT_HANDLE, out);
  }
  {
    HANDLE out = GetStdHandle(STD_ERROR_HANDLE);
    DuplicateHandle(GetCurrentProcess(), out, GetCurrentProcess(), &out, 0,
                    FALSE, DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE);
    SetStdHandle(STD_ERROR_HANDLE, out);
  }
#endif
}

#ifdef _WIN32
#  ifndef CRYPT_SILENT
#    define CRYPT_SILENT 0x40 /* Not defined by VS 6 version of header.  */
#  endif
static int WinCryptRandom(void* data, size_t size)
{
  int result = 0;
  HCRYPTPROV hProvider = 0;
  if (CryptAcquireContextW(&hProvider, 0, 0, PROV_RSA_FULL,
                           CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
    result = CryptGenRandom(hProvider, (DWORD)size, (BYTE*)data) ? 1 : 0;
    CryptReleaseContext(hProvider, 0);
  }
  return result;
}
#endif

unsigned int cmSystemTools::RandomSeed()
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  unsigned int seed = 0;

  // Try using a real random source.
  if (WinCryptRandom(&seed, sizeof(seed))) {
    return seed;
  }

  // Fall back to the time and pid.
  FILETIME ft;
  GetSystemTimeAsFileTime(&ft);
  unsigned int t1 = static_cast<unsigned int>(ft.dwHighDateTime);
  unsigned int t2 = static_cast<unsigned int>(ft.dwLowDateTime);
  unsigned int pid = static_cast<unsigned int>(GetCurrentProcessId());
  return t1 ^ t2 ^ pid;
#else
  union
  {
    unsigned int integer;
    char bytes[sizeof(unsigned int)];
  } seed;

  // Try using a real random source.
  cmsys::ifstream fin;
  fin.rdbuf()->pubsetbuf(nullptr, 0); // Unbuffered read.
  fin.open("/dev/urandom");
  if (fin.good() && fin.read(seed.bytes, sizeof(seed)) &&
      fin.gcount() == sizeof(seed)) {
    return seed.integer;
  }

  // Fall back to the time and pid.
  struct timeval t;
  gettimeofday(&t, nullptr);
  unsigned int pid = static_cast<unsigned int>(getpid());
  unsigned int tv_sec = static_cast<unsigned int>(t.tv_sec);
  unsigned int tv_usec = static_cast<unsigned int>(t.tv_usec);
  // Since tv_usec never fills more than 11 bits we shift it to fill
  // in the slow-changing high-order bits of tv_sec.
  return tv_sec ^ (tv_usec << 21) ^ pid;
#endif
}

static std::string cmSystemToolsCMakeCommand;
static std::string cmSystemToolsCTestCommand;
static std::string cmSystemToolsCPackCommand;
static std::string cmSystemToolsCMakeCursesCommand;
static std::string cmSystemToolsCMakeGUICommand;
static std::string cmSystemToolsCMClDepsCommand;
static std::string cmSystemToolsCMakeRoot;
static std::string cmSystemToolsHTMLDoc;
void cmSystemTools::FindCMakeResources(const char* argv0)
{
  std::string exe_dir;
#if defined(_WIN32) && !defined(__CYGWIN__)
  (void)argv0; // ignore this on windows
  wchar_t modulepath[_MAX_PATH];
  ::GetModuleFileNameW(nullptr, modulepath, sizeof(modulepath));
  std::string path = cmsys::Encoding::ToNarrow(modulepath);
  std::string realPath =
    cmSystemTools::GetRealPathResolvingWindowsSubst(path, nullptr);
  if (realPath.empty()) {
    realPath = path;
  }
  exe_dir = cmSystemTools::GetFilenamePath(realPath);
#elif defined(__APPLE__)
  (void)argv0; // ignore this on OS X
#  define CM_EXE_PATH_LOCAL_SIZE 16384
  char exe_path_local[CM_EXE_PATH_LOCAL_SIZE];
#  if defined(MAC_OS_X_VERSION_10_3) && !defined(MAC_OS_X_VERSION_10_4)
  unsigned long exe_path_size = CM_EXE_PATH_LOCAL_SIZE;
#  else
  uint32_t exe_path_size = CM_EXE_PATH_LOCAL_SIZE;
#  endif
#  undef CM_EXE_PATH_LOCAL_SIZE
  char* exe_path = exe_path_local;
  if (_NSGetExecutablePath(exe_path, &exe_path_size) < 0) {
    exe_path = static_cast<char*>(malloc(exe_path_size));
    _NSGetExecutablePath(exe_path, &exe_path_size);
  }
  exe_dir =
    cmSystemTools::GetFilenamePath(cmSystemTools::GetRealPath(exe_path));
  if (exe_path != exe_path_local) {
    free(exe_path);
  }
  if (cmSystemTools::GetFilenameName(exe_dir) == "MacOS") {
    // The executable is inside an application bundle.
    // Look for ..<CMAKE_BIN_DIR> (install tree) and then fall back to
    // ../../../bin (build tree).
    exe_dir = cmSystemTools::GetFilenamePath(exe_dir);
    if (cmSystemTools::FileExists(exe_dir + CMAKE_BIN_DIR "/cmake")) {
      exe_dir += CMAKE_BIN_DIR;
    } else {
      exe_dir = cmSystemTools::GetFilenamePath(exe_dir);
      exe_dir = cmSystemTools::GetFilenamePath(exe_dir);
    }
  }
#else
  std::string errorMsg;
  std::string exe;
  if (cmSystemTools::FindProgramPath(argv0, exe, errorMsg)) {
    // remove symlinks
    exe = cmSystemTools::GetRealPath(exe);
    exe_dir = cmSystemTools::GetFilenamePath(exe);
  } else {
    // ???
  }
#endif
  exe_dir = cmSystemTools::GetActualCaseForPath(exe_dir);
  cmSystemToolsCMakeCommand =
    cmStrCat(exe_dir, "/cmake", cmSystemTools::GetExecutableExtension());
#ifdef CMAKE_BOOTSTRAP
  // The bootstrap cmake does not provide the other tools,
  // so use the directory where they are about to be built.
  exe_dir = CMAKE_BOOTSTRAP_BINARY_DIR "/bin";
#endif
  cmSystemToolsCTestCommand =
    cmStrCat(exe_dir, "/ctest", cmSystemTools::GetExecutableExtension());
  cmSystemToolsCPackCommand =
    cmStrCat(exe_dir, "/cpack", cmSystemTools::GetExecutableExtension());
  cmSystemToolsCMakeGUICommand =
    cmStrCat(exe_dir, "/cmake-gui", cmSystemTools::GetExecutableExtension());
  if (!cmSystemTools::FileExists(cmSystemToolsCMakeGUICommand)) {
    cmSystemToolsCMakeGUICommand.clear();
  }
  cmSystemToolsCMakeCursesCommand =
    cmStrCat(exe_dir, "/ccmake", cmSystemTools::GetExecutableExtension());
  if (!cmSystemTools::FileExists(cmSystemToolsCMakeCursesCommand)) {
    cmSystemToolsCMakeCursesCommand.clear();
  }
  cmSystemToolsCMClDepsCommand =
    cmStrCat(exe_dir, "/cmcldeps", cmSystemTools::GetExecutableExtension());
  if (!cmSystemTools::FileExists(cmSystemToolsCMClDepsCommand)) {
    cmSystemToolsCMClDepsCommand.clear();
  }

#ifndef CMAKE_BOOTSTRAP
  // Install tree has
  // - "<prefix><CMAKE_BIN_DIR>/cmake"
  // - "<prefix><CMAKE_DATA_DIR>"
  // - "<prefix><CMAKE_DOC_DIR>"
  if (cmHasLiteralSuffix(exe_dir, CMAKE_BIN_DIR)) {
    std::string const prefix =
      exe_dir.substr(0, exe_dir.size() - cmStrLen(CMAKE_BIN_DIR));
    cmSystemToolsCMakeRoot = cmStrCat(prefix, CMAKE_DATA_DIR);
    if (cmSystemTools::FileExists(
          cmStrCat(prefix, CMAKE_DOC_DIR "/html/index.html"))) {
      cmSystemToolsHTMLDoc = cmStrCat(prefix, CMAKE_DOC_DIR "/html");
    }
  }
  if (cmSystemToolsCMakeRoot.empty() ||
      !cmSystemTools::FileExists(
        cmStrCat(cmSystemToolsCMakeRoot, "/Modules/CMake.cmake"))) {
    // Build tree has "<build>/bin[/<config>]/cmake" and
    // "<build>/CMakeFiles/CMakeSourceDir.txt".
    std::string dir = cmSystemTools::GetFilenamePath(exe_dir);
    std::string src_dir_txt = cmStrCat(dir, "/CMakeFiles/CMakeSourceDir.txt");
    cmsys::ifstream fin(src_dir_txt.c_str());
    std::string src_dir;
    if (fin && cmSystemTools::GetLineFromStream(fin, src_dir) &&
        cmSystemTools::FileIsDirectory(src_dir)) {
      cmSystemToolsCMakeRoot = src_dir;
    } else {
      dir = cmSystemTools::GetFilenamePath(dir);
      src_dir_txt = cmStrCat(dir, "/CMakeFiles/CMakeSourceDir.txt");
      cmsys::ifstream fin2(src_dir_txt.c_str());
      if (fin2 && cmSystemTools::GetLineFromStream(fin2, src_dir) &&
          cmSystemTools::FileIsDirectory(src_dir)) {
        cmSystemToolsCMakeRoot = src_dir;
      }
    }
    if (!cmSystemToolsCMakeRoot.empty() && cmSystemToolsHTMLDoc.empty() &&
        cmSystemTools::FileExists(
          cmStrCat(dir, "/Utilities/Sphinx/html/index.html"))) {
      cmSystemToolsHTMLDoc = cmStrCat(dir, "/Utilities/Sphinx/html");
    }
  }
#else
  // Bootstrap build knows its source.
  cmSystemToolsCMakeRoot = CMAKE_BOOTSTRAP_SOURCE_DIR;
#endif
}

std::string const& cmSystemTools::GetCMakeCommand()
{
  return cmSystemToolsCMakeCommand;
}

std::string const& cmSystemTools::GetCTestCommand()
{
  return cmSystemToolsCTestCommand;
}

std::string const& cmSystemTools::GetCPackCommand()
{
  return cmSystemToolsCPackCommand;
}

std::string const& cmSystemTools::GetCMakeCursesCommand()
{
  return cmSystemToolsCMakeCursesCommand;
}

std::string const& cmSystemTools::GetCMakeGUICommand()
{
  return cmSystemToolsCMakeGUICommand;
}

std::string const& cmSystemTools::GetCMClDepsCommand()
{
  return cmSystemToolsCMClDepsCommand;
}

std::string const& cmSystemTools::GetCMakeRoot()
{
  return cmSystemToolsCMakeRoot;
}

std::string const& cmSystemTools::GetHTMLDoc()
{
  return cmSystemToolsHTMLDoc;
}

std::string cmSystemTools::GetCurrentWorkingDirectory()
{
  return cmSystemTools::CollapseFullPath(
    cmsys::SystemTools::GetCurrentWorkingDirectory());
}

void cmSystemTools::MakefileColorEcho(int color, const char* message,
                                      bool newline, bool enabled)
{
  // On some platforms (an MSYS prompt) cmsysTerminal may not be able
  // to determine whether the stream is displayed on a tty.  In this
  // case it assumes no unless we tell it otherwise.  Since we want
  // color messages to be displayed for users we will assume yes.
  // However, we can test for some situations when the answer is most
  // likely no.
  int assumeTTY = cmsysTerminal_Color_AssumeTTY;
  if (cmSystemTools::HasEnv("DART_TEST_FROM_DART") ||
      cmSystemTools::HasEnv("DASHBOARD_TEST_FROM_CTEST") ||
      cmSystemTools::HasEnv("CTEST_INTERACTIVE_DEBUG_MODE")) {
    // Avoid printing color escapes during dashboard builds.
    assumeTTY = 0;
  }

  if (enabled && color != cmsysTerminal_Color_Normal) {
    // Print with color.  Delay the newline until later so that
    // all color restore sequences appear before it.
    cmsysTerminal_cfprintf(color | assumeTTY, stdout, "%s", message);
  } else {
    // Color is disabled.  Print without color.
    fprintf(stdout, "%s", message);
  }

  if (newline) {
    fprintf(stdout, "\n");
  }
}

bool cmSystemTools::GuessLibrarySOName(std::string const& fullPath,
                                       std::string& soname)
{
  // For ELF shared libraries use a real parser to get the correct
  // soname.
  cmELF elf(fullPath.c_str());
  if (elf) {
    return elf.GetSOName(soname);
  }

  // If the file is not a symlink we have no guess for its soname.
  if (!cmSystemTools::FileIsSymlink(fullPath)) {
    return false;
  }
  if (!cmSystemTools::ReadSymlink(fullPath, soname)) {
    return false;
  }

  // If the symlink has a path component we have no guess for the soname.
  if (!cmSystemTools::GetFilenamePath(soname).empty()) {
    return false;
  }

  // If the symlink points at an extended version of the same name
  // assume it is the soname.
  std::string name = cmSystemTools::GetFilenameName(fullPath);
  return soname.length() > name.length() &&
    soname.compare(0, name.length(), name) == 0;
}

bool cmSystemTools::GuessLibraryInstallName(std::string const& fullPath,
                                            std::string& soname)
{
#if defined(CMake_USE_MACH_PARSER)
  cmMachO macho(fullPath.c_str());
  if (macho) {
    return macho.GetInstallName(soname);
  }
#else
  (void)fullPath;
  (void)soname;
#endif

  return false;
}

static std::string::size_type cmSystemToolsFindRPath(
  cm::string_view const& have, cm::string_view const& want)
{
  std::string::size_type pos = 0;
  while (pos < have.size()) {
    // Look for an occurrence of the string.
    std::string::size_type const beg = have.find(want, pos);
    if (beg == std::string::npos) {
      return std::string::npos;
    }

    // Make sure it is separated from preceding entries.
    if (beg > 0 && have[beg - 1] != ':') {
      pos = beg + 1;
      continue;
    }

    // Make sure it is separated from following entries.
    std::string::size_type const end = beg + want.size();
    if (end < have.size() && have[end] != ':') {
      pos = beg + 1;
      continue;
    }

    // Return the position of the path portion.
    return beg;
  }

  // The desired rpath was not found.
  return std::string::npos;
}

namespace {
struct cmSystemToolsRPathInfo
{
  unsigned long Position;
  unsigned long Size;
  std::string Name;
  std::string Value;
};

using EmptyCallback = std::function<bool(std::string*, const cmELF&)>;
using AdjustCallback = std::function<bool(
  cm::optional<std::string>&, const std::string&, const char*, std::string*)>;

cm::optional<bool> AdjustRPathELF(std::string const& file,
                                  const EmptyCallback& emptyCallback,
                                  const AdjustCallback& adjustCallback,
                                  std::string* emsg, bool* changed)
{
  if (changed) {
    *changed = false;
  }
  int rp_count = 0;
  bool remove_rpath = true;
  cmSystemToolsRPathInfo rp[2];
  {
    // Parse the ELF binary.
    cmELF elf(file.c_str());
    if (!elf) {
      return cm::nullopt; // Not a valid ELF file.
    }

    // Get the RPATH and RUNPATH entries from it.
    int se_count = 0;
    cmELF::StringEntry const* se[2] = { nullptr, nullptr };
    const char* se_name[2] = { nullptr, nullptr };
    if (cmELF::StringEntry const* se_rpath = elf.GetRPath()) {
      se[se_count] = se_rpath;
      se_name[se_count] = "RPATH";
      ++se_count;
    }
    if (cmELF::StringEntry const* se_runpath = elf.GetRunPath()) {
      se[se_count] = se_runpath;
      se_name[se_count] = "RUNPATH";
      ++se_count;
    }
    if (se_count == 0) {
      return emptyCallback(emsg, elf);
    }

    for (int i = 0; i < se_count; ++i) {
      // If both RPATH and RUNPATH refer to the same string literal it
      // needs to be changed only once.
      if (rp_count && rp[0].Position == se[i]->Position) {
        continue;
      }

      // Store information about the entry in the file.
      rp[rp_count].Position = se[i]->Position;
      rp[rp_count].Size = se[i]->Size;
      rp[rp_count].Name = se_name[i];

      // Adjust the rpath.
      cm::optional<std::string> outRPath;
      if (!adjustCallback(outRPath, se[i]->Value, se_name[i], emsg)) {
        return false;
      }

      if (outRPath) {
        if (!outRPath->empty()) {
          remove_rpath = false;
        }

        // Make sure there is enough room to store the new rpath and at
        // least one null terminator.
        if (rp[rp_count].Size < outRPath->length() + 1) {
          if (emsg) {
            *emsg = cmStrCat("The replacement path is too long for the ",
                             se_name[i], " entry.");
          }
          return false;
        }

        // This entry is ready for update.
        rp[rp_count].Value = std::move(*outRPath);
        ++rp_count;
      } else {
        remove_rpath = false;
      }
    }
  }

  // If no runtime path needs to be changed, we are done.
  if (rp_count == 0) {
    return true;
  }

  // If the resulting rpath is empty, just remove the entire entry instead.
  if (remove_rpath) {
    return cmSystemTools::RemoveRPath(file, emsg, changed);
  }

  {
    // Open the file for update.
    cmsys::ofstream f(file.c_str(),
                      std::ios::in | std::ios::out | std::ios::binary);
    if (!f) {
      if (emsg) {
        *emsg = "Error opening file for update.";
      }
      return false;
    }

    // Store the new RPATH and RUNPATH strings.
    for (int i = 0; i < rp_count; ++i) {
      // Seek to the RPATH position.
      if (!f.seekp(rp[i].Position)) {
        if (emsg) {
          *emsg = cmStrCat("Error seeking to ", rp[i].Name, " position.");
        }
        return false;
      }

      // Write the new rpath.  Follow it with enough null terminators to
      // fill the string table entry.
      f << rp[i].Value;
      for (unsigned long j = rp[i].Value.length(); j < rp[i].Size; ++j) {
        f << '\0';
      }

      // Make sure it wrote correctly.
      if (!f) {
        if (emsg) {
          *emsg = cmStrCat("Error writing the new ", rp[i].Name,
                           " string to the file.");
        }
        return false;
      }
    }
  }

  // Everything was updated successfully.
  if (changed) {
    *changed = true;
  }
  return true;
}

std::function<bool(std::string*, const cmELF&)> MakeEmptyCallback(
  const std::string& newRPath)
{
  return [newRPath](std::string* emsg, const cmELF& elf) -> bool {
    if (newRPath.empty()) {
      // The new rpath is empty and there is no rpath anyway so it is
      // okay.
      return true;
    }
    if (emsg) {
      *emsg =
        cmStrCat("No valid ELF RPATH or RUNPATH entry exists in the file; ",
                 elf.GetErrorMessage());
    }
    return false;
  };
}
}

static cm::optional<bool> ChangeRPathELF(std::string const& file,
                                         std::string const& oldRPath,
                                         std::string const& newRPath,
                                         bool removeEnvironmentRPath,
                                         std::string* emsg, bool* changed)
{
  auto adjustCallback = [oldRPath, newRPath, removeEnvironmentRPath](
                          cm::optional<std::string>& outRPath,
                          const std::string& inRPath, const char* se_name,
                          std::string* emsg2) -> bool {
    // Make sure the current rpath contains the old rpath.
    std::string::size_type pos = cmSystemToolsFindRPath(inRPath, oldRPath);
    if (pos == std::string::npos) {
      // If it contains the new rpath instead then it is okay.
      if (cmSystemToolsFindRPath(inRPath, newRPath) != std::string::npos) {
        return true;
      }
      if (emsg2) {
        std::ostringstream e;
        /* clang-format off */
        e << "The current " << se_name << " is:\n"
          << "  " << inRPath << "\n"
          << "which does not contain:\n"
          << "  " << oldRPath << "\n"
          << "as was expected.";
        /* clang-format on */
        *emsg2 = e.str();
      }
      return false;
    }

    std::string::size_type prefix_len = pos;

    // If oldRPath was at the end of the file's RPath, and newRPath is empty,
    // we should remove the unnecessary ':' at the end.
    if (newRPath.empty() && pos > 0 && inRPath[pos - 1] == ':' &&
        pos + oldRPath.length() == inRPath.length()) {
      prefix_len--;
    }

    // Construct the new value which preserves the part of the path
    // not being changed.
    outRPath.emplace();
    if (!removeEnvironmentRPath) {
      *outRPath += inRPath.substr(0, prefix_len);
    }
    *outRPath += newRPath;
    *outRPath += inRPath.substr(pos + oldRPath.length());

    return true;
  };

  return AdjustRPathELF(file, MakeEmptyCallback(newRPath), adjustCallback,
                        emsg, changed);
}

static cm::optional<bool> SetRPathELF(std::string const& file,
                                      std::string const& newRPath,
                                      std::string* emsg, bool* changed)
{
  auto adjustCallback = [newRPath](cm::optional<std::string>& outRPath,
                                   const std::string& inRPath,
                                   const char* /*se_name*/, std::string*
                                   /*emsg*/) -> bool {
    if (inRPath != newRPath) {
      outRPath = newRPath;
    }
    return true;
  };

  return AdjustRPathELF(file, MakeEmptyCallback(newRPath), adjustCallback,
                        emsg, changed);
}
static cm::optional<bool> ChangeRPathXCOFF(std::string const& file,
                                           std::string const& oldRPath,
                                           std::string const& newRPath,
                                           bool removeEnvironmentRPath,
                                           std::string* emsg, bool* changed)
{
  if (changed) {
    *changed = false;
  }
#if !defined(CMake_USE_XCOFF_PARSER)
  (void)file;
  (void)oldRPath;
  (void)newRPath;
  (void)removeEnvironmentRPath;
  (void)emsg;
  return cm::nullopt;
#else
  bool chg = false;
  cmXCOFF xcoff(file.c_str(), cmXCOFF::Mode::ReadWrite);
  if (!xcoff) {
    return cm::nullopt; // Not a valid XCOFF file
  }
  if (cm::optional<cm::string_view> maybeLibPath = xcoff.GetLibPath()) {
    cm::string_view libPath = *maybeLibPath;
    // Make sure the current rpath contains the old rpath.
    std::string::size_type pos = cmSystemToolsFindRPath(libPath, oldRPath);
    if (pos == std::string::npos) {
      // If it contains the new rpath instead then it is okay.
      if (cmSystemToolsFindRPath(libPath, newRPath) != std::string::npos) {
        return true;
      }
      if (emsg) {
        std::ostringstream e;
        /* clang-format off */
        e << "The current RPATH is:\n"
          << "  " << libPath << "\n"
          << "which does not contain:\n"
          << "  " << oldRPath << "\n"
          << "as was expected.";
        /* clang-format on */
        *emsg = e.str();
      }
      return false;
    }

    // The prefix is either empty or ends in a ':'.
    cm::string_view prefix = libPath.substr(0, pos);
    if (newRPath.empty() && !prefix.empty()) {
      prefix.remove_suffix(1);
    }

    // The suffix is either empty or starts in a ':'.
    cm::string_view suffix = libPath.substr(pos + oldRPath.length());

    // Construct the new value which preserves the part of the path
    // not being changed.
    std::string newLibPath;
    if (!removeEnvironmentRPath) {
      newLibPath = std::string(prefix);
    }
    newLibPath += newRPath;
    newLibPath += suffix;

    chg = xcoff.SetLibPath(newLibPath);
  }
  if (!xcoff) {
    if (emsg) {
      *emsg = xcoff.GetErrorMessage();
    }
    return false;
  }

  // Everything was updated successfully.
  if (changed) {
    *changed = chg;
  }
  return true;
#endif
}

static cm::optional<bool> SetRPathXCOFF(std::string const& /*file*/,
                                        std::string const& /*newRPath*/,
                                        std::string* /*emsg*/,
                                        bool* /*changed*/)
{
  return cm::nullopt; // Not implemented.
}

bool cmSystemTools::ChangeRPath(std::string const& file,
                                std::string const& oldRPath,
                                std::string const& newRPath,
                                bool removeEnvironmentRPath, std::string* emsg,
                                bool* changed)
{
  if (cm::optional<bool> result = ChangeRPathELF(
        file, oldRPath, newRPath, removeEnvironmentRPath, emsg, changed)) {
    return result.value();
  }
  if (cm::optional<bool> result = ChangeRPathXCOFF(
        file, oldRPath, newRPath, removeEnvironmentRPath, emsg, changed)) {
    return result.value();
  }
  // The file format is not recognized.  Assume it has no RPATH.
  if (newRPath.empty()) {
    // The caller wanted no RPATH anyway.
    return true;
  }
  if (emsg) {
    *emsg = "The file format is not recognized.";
  }
  return false;
}

bool cmSystemTools::SetRPath(std::string const& file,
                             std::string const& newRPath, std::string* emsg,
                             bool* changed)
{
  if (cm::optional<bool> result = SetRPathELF(file, newRPath, emsg, changed)) {
    return result.value();
  }
  if (cm::optional<bool> result =
        SetRPathXCOFF(file, newRPath, emsg, changed)) {
    return result.value();
  }
  // The file format is not recognized.  Assume it has no RPATH.
  if (newRPath.empty()) {
    // The caller wanted no RPATH anyway.
    return true;
  }
  if (emsg) {
    *emsg = "The file format is not recognized.";
  }
  return false;
}

namespace {
bool VersionCompare(cmSystemTools::CompareOp op, const char* lhss,
                    const char* rhss)
{
  const char* endl = lhss;
  const char* endr = rhss;

  while (((*endl >= '0') && (*endl <= '9')) ||
         ((*endr >= '0') && (*endr <= '9'))) {
    // Do component-wise comparison, ignoring leading zeros
    // (components are treated as integers, not as mantissas)
    while (*endl == '0') {
      endl++;
    }
    while (*endr == '0') {
      endr++;
    }

    const char* beginl = endl;
    const char* beginr = endr;

    // count significant digits
    while ((*endl >= '0') && (*endl <= '9')) {
      endl++;
    }
    while ((*endr >= '0') && (*endr <= '9')) {
      endr++;
    }

    // compare number of digits first
    ptrdiff_t r = ((endl - beginl) - (endr - beginr));
    if (r == 0) {
      // compare the digits if number of digits is equal
      r = strncmp(beginl, beginr, endl - beginl);
    }

    if (r < 0) {
      // lhs < rhs, so true if operation is LESS
      return (op & cmSystemTools::OP_LESS) != 0;
    }
    if (r > 0) {
      // lhs > rhs, so true if operation is GREATER
      return (op & cmSystemTools::OP_GREATER) != 0;
    }

    if (*endr == '.') {
      endr++;
    }

    if (*endl == '.') {
      endl++;
    }
  }
  // lhs == rhs, so true if operation is EQUAL
  return (op & cmSystemTools::OP_EQUAL) != 0;
}
}

bool cmSystemTools::VersionCompare(cmSystemTools::CompareOp op,
                                   const std::string& lhs,
                                   const std::string& rhs)
{
  return ::VersionCompare(op, lhs.c_str(), rhs.c_str());
}
bool cmSystemTools::VersionCompare(cmSystemTools::CompareOp op,
                                   const std::string& lhs, const char rhs[])
{
  return ::VersionCompare(op, lhs.c_str(), rhs);
}

bool cmSystemTools::VersionCompareEqual(std::string const& lhs,
                                        std::string const& rhs)
{
  return cmSystemTools::VersionCompare(cmSystemTools::OP_EQUAL, lhs, rhs);
}

bool cmSystemTools::VersionCompareGreater(std::string const& lhs,
                                          std::string const& rhs)
{
  return cmSystemTools::VersionCompare(cmSystemTools::OP_GREATER, lhs, rhs);
}

bool cmSystemTools::VersionCompareGreaterEq(std::string const& lhs,
                                            std::string const& rhs)
{
  return cmSystemTools::VersionCompare(cmSystemTools::OP_GREATER_EQUAL, lhs,
                                       rhs);
}

static size_t cm_strverscmp_find_first_difference_or_end(const char* lhs,
                                                         const char* rhs)
{
  size_t i = 0;
  /* Step forward until we find a difference or both strings end together.
     The difference may lie on the null-terminator of one string.  */
  while (lhs[i] == rhs[i] && lhs[i] != 0) {
    ++i;
  }
  return i;
}

static size_t cm_strverscmp_find_digits_begin(const char* s, size_t i)
{
  /* Step back until we are not preceded by a digit.  */
  while (i > 0 && isdigit(s[i - 1])) {
    --i;
  }
  return i;
}

static size_t cm_strverscmp_find_digits_end(const char* s, size_t i)
{
  /* Step forward over digits.  */
  while (isdigit(s[i])) {
    ++i;
  }
  return i;
}

static size_t cm_strverscmp_count_leading_zeros(const char* s, size_t b)
{
  size_t i = b;
  /* Step forward over zeros that are followed by another digit.  */
  while (s[i] == '0' && isdigit(s[i + 1])) {
    ++i;
  }
  return i - b;
}

static int cm_strverscmp(const char* lhs, const char* rhs)
{
  size_t const i = cm_strverscmp_find_first_difference_or_end(lhs, rhs);
  if (lhs[i] != rhs[i]) {
    /* The strings differ starting at 'i'.  Check for a digit sequence.  */
    size_t const b = cm_strverscmp_find_digits_begin(lhs, i);
    if (b != i || (isdigit(lhs[i]) && isdigit(rhs[i]))) {
      /* A digit sequence starts at 'b', preceding or at 'i'.  */

      /* Look for leading zeros, implying a leading decimal point.  */
      size_t const lhs_zeros = cm_strverscmp_count_leading_zeros(lhs, b);
      size_t const rhs_zeros = cm_strverscmp_count_leading_zeros(rhs, b);
      if (lhs_zeros != rhs_zeros) {
        /* The side with more leading zeros orders first.  */
        return rhs_zeros > lhs_zeros ? 1 : -1;
      }
      if (lhs_zeros == 0) {
        /* No leading zeros; compare digit sequence lengths.  */
        size_t const lhs_end = cm_strverscmp_find_digits_end(lhs, i);
        size_t const rhs_end = cm_strverscmp_find_digits_end(rhs, i);
        if (lhs_end != rhs_end) {
          /* The side with fewer digits orders first.  */
          return lhs_end > rhs_end ? 1 : -1;
        }
      }
    }
  }

  /* Ordering was not decided by digit sequence lengths; compare bytes.  */
  return lhs[i] - rhs[i];
}

int cmSystemTools::strverscmp(std::string const& lhs, std::string const& rhs)
{
  return cm_strverscmp(lhs.c_str(), rhs.c_str());
}

static cm::optional<bool> RemoveRPathELF(std::string const& file,
                                         std::string* emsg, bool* removed)
{
  if (removed) {
    *removed = false;
  }
  int zeroCount = 0;
  unsigned long zeroPosition[2] = { 0, 0 };
  unsigned long zeroSize[2] = { 0, 0 };
  unsigned long bytesBegin = 0;
  std::vector<char> bytes;
  {
    // Parse the ELF binary.
    cmELF elf(file.c_str());
    if (!elf) {
      return cm::nullopt; // Not a valid ELF file.
    }

    // Get the RPATH and RUNPATH entries from it and sort them by index
    // in the dynamic section header.
    int se_count = 0;
    cmELF::StringEntry const* se[2] = { nullptr, nullptr };
    if (cmELF::StringEntry const* se_rpath = elf.GetRPath()) {
      se[se_count++] = se_rpath;
    }
    if (cmELF::StringEntry const* se_runpath = elf.GetRunPath()) {
      se[se_count++] = se_runpath;
    }
    if (se_count == 0) {
      // There is no RPATH or RUNPATH anyway.
      return true;
    }
    if (se_count == 2 && se[1]->IndexInSection < se[0]->IndexInSection) {
      std::swap(se[0], se[1]);
    }

    // Obtain a copy of the dynamic entries
    cmELF::DynamicEntryList dentries = elf.GetDynamicEntries();
    if (dentries.empty()) {
      // This should happen only for invalid ELF files where a DT_NULL
      // appears before the end of the table.
      if (emsg) {
        *emsg = "DYNAMIC section contains a DT_NULL before the end.";
      }
      return false;
    }

    // Save information about the string entries to be zeroed.
    zeroCount = se_count;
    for (int i = 0; i < se_count; ++i) {
      zeroPosition[i] = se[i]->Position;
      zeroSize[i] = se[i]->Size;
    }

    // Get size of one DYNAMIC entry
    unsigned long const sizeof_dentry =
      elf.GetDynamicEntryPosition(1) - elf.GetDynamicEntryPosition(0);

    // Adjust the entry list as necessary to remove the run path
    unsigned long entriesErased = 0;
    for (auto it = dentries.begin(); it != dentries.end();) {
      if (it->first == cmELF::TagRPath || it->first == cmELF::TagRunPath) {
        it = dentries.erase(it);
        entriesErased++;
        continue;
      }
      if (it->first == cmELF::TagMipsRldMapRel && elf.IsMIPS()) {
        // Background: debuggers need to know the "linker map" which contains
        // the addresses each dynamic object is loaded at. Most arches use
        // the DT_DEBUG tag which the dynamic linker writes to (directly) and
        // contain the location of the linker map, however on MIPS the
        // .dynamic section is always read-only so this is not possible. MIPS
        // objects instead contain a DT_MIPS_RLD_MAP tag which contains the
        // address where the dynamic linker will write to (an indirect
        // version of DT_DEBUG). Since this doesn't work when using PIE, a
        // relative equivalent was created - DT_MIPS_RLD_MAP_REL. Since this
        // version contains a relative offset, moving it changes the
        // calculated address. This may cause the dynamic linker to write
        // into memory it should not be changing.
        //
        // To fix this, we adjust the value of DT_MIPS_RLD_MAP_REL here. If
        // we move it up by n bytes, we add n bytes to the value of this tag.
        it->second += entriesErased * sizeof_dentry;
      }

      it++;
    }

    // Encode new entries list
    bytes = elf.EncodeDynamicEntries(dentries);
    bytesBegin = elf.GetDynamicEntryPosition(0);
  }

  // Open the file for update.
  cmsys::ofstream f(file.c_str(),
                    std::ios::in | std::ios::out | std::ios::binary);
  if (!f) {
    if (emsg) {
      *emsg = "Error opening file for update.";
    }
    return false;
  }

  // Write the new DYNAMIC table header.
  if (!f.seekp(bytesBegin)) {
    if (emsg) {
      *emsg = "Error seeking to DYNAMIC table header for RPATH.";
    }
    return false;
  }
  if (!f.write(bytes.data(), bytes.size())) {
    if (emsg) {
      *emsg = "Error replacing DYNAMIC table header.";
    }
    return false;
  }

  // Fill the RPATH and RUNPATH strings with zero bytes.
  for (int i = 0; i < zeroCount; ++i) {
    if (!f.seekp(zeroPosition[i])) {
      if (emsg) {
        *emsg = "Error seeking to RPATH position.";
      }
      return false;
    }
    for (unsigned long j = 0; j < zeroSize[i]; ++j) {
      f << '\0';
    }
    if (!f) {
      if (emsg) {
        *emsg = "Error writing the empty rpath string to the file.";
      }
      return false;
    }
  }

  // Everything was updated successfully.
  if (removed) {
    *removed = true;
  }
  return true;
}

static cm::optional<bool> RemoveRPathXCOFF(std::string const& file,
                                           std::string* emsg, bool* removed)
{
  if (removed) {
    *removed = false;
  }
#if !defined(CMake_USE_XCOFF_PARSER)
  (void)file;
  (void)emsg;
  return cm::nullopt; // Cannot handle XCOFF files.
#else
  cmXCOFF xcoff(file.c_str(), cmXCOFF::Mode::ReadWrite);
  if (!xcoff) {
    return cm::nullopt; // Not a valid XCOFF file.
  }
  bool rm = xcoff.RemoveLibPath();
  if (!xcoff) {
    if (emsg) {
      *emsg = xcoff.GetErrorMessage();
    }
    return false;
  }

  if (removed) {
    *removed = rm;
  }
  return true;
#endif
}
bool cmSystemTools::RemoveRPath(std::string const& file, std::string* emsg,
                                bool* removed)
{
  if (cm::optional<bool> result = RemoveRPathELF(file, emsg, removed)) {
    return result.value();
  }
  if (cm::optional<bool> result = RemoveRPathXCOFF(file, emsg, removed)) {
    return result.value();
  }
  // The file format is not recognized.  Assume it has no RPATH.
  return true;
}

bool cmSystemTools::CheckRPath(std::string const& file,
                               std::string const& newRPath)
{
  // Parse the ELF binary.
  cmELF elf(file.c_str());
  if (elf) {
    // Get the RPATH or RUNPATH entry from it.
    cmELF::StringEntry const* se = elf.GetRPath();
    if (!se) {
      se = elf.GetRunPath();
    }

    // Make sure the current rpath contains the new rpath.
    if (newRPath.empty()) {
      if (!se) {
        return true;
      }
    } else {
      if (se &&
          cmSystemToolsFindRPath(se->Value, newRPath) != std::string::npos) {
        return true;
      }
    }
    return false;
  }
#if defined(CMake_USE_XCOFF_PARSER)
  // Parse the XCOFF binary.
  cmXCOFF xcoff(file.c_str());
  if (xcoff) {
    if (cm::optional<cm::string_view> libPath = xcoff.GetLibPath()) {
      if (cmSystemToolsFindRPath(*libPath, newRPath) != std::string::npos) {
        return true;
      }
    }
    return false;
  }
#endif
  // The file format is not recognized.  Assume it has no RPATH.
  // Therefore we succeed if the new rpath is empty anyway.
  return newRPath.empty();
}

bool cmSystemTools::RepeatedRemoveDirectory(const std::string& dir)
{
#ifdef _WIN32
  // Windows sometimes locks files temporarily so try a few times.
  WindowsFileRetry retry = cmSystemTools::GetWindowsFileRetry();

  for (unsigned int i = 0; i < retry.Count; ++i) {
    if (cmSystemTools::RemoveADirectory(dir)) {
      return true;
    }
    cmSystemTools::Delay(retry.Delay);
  }
  return false;
#else
  return static_cast<bool>(cmSystemTools::RemoveADirectory(dir));
#endif
}

std::string cmSystemTools::EncodeURL(std::string const& in, bool escapeSlashes)
{
  std::string out;
  for (char c : in) {
    char hexCh[4] = { 0, 0, 0, 0 };
    hexCh[0] = c;
    switch (c) {
      case '+':
      case '?':
      case '\\':
      case '&':
      case ' ':
      case '=':
      case '%':
        snprintf(hexCh, sizeof(hexCh), "%%%02X", static_cast<int>(c));
        break;
      case '/':
        if (escapeSlashes) {
          strcpy(hexCh, "%2F");
        }
        break;
      default:
        break;
    }
    out.append(hexCh);
  }
  return out;
}

cmsys::Status cmSystemTools::CreateSymlink(std::string const& origName,
                                           std::string const& newName)
{
  cmsys::Status status =
    cmSystemTools::CreateSymlinkQuietly(origName, newName);
  if (!status) {
    cmSystemTools::Error(cmStrCat("failed to create symbolic link '", newName,
                                  "': ", status.GetString()));
  }
  return status;
}

cmsys::Status cmSystemTools::CreateSymlinkQuietly(std::string const& origName,
                                                  std::string const& newName)
{
  uv_fs_t req;
  int flags = 0;
#if defined(_WIN32)
  if (cmsys::SystemTools::FileIsDirectory(origName)) {
    flags |= UV_FS_SYMLINK_DIR;
  }
#endif
  int err = uv_fs_symlink(nullptr, &req, origName.c_str(), newName.c_str(),
                          flags, nullptr);
  cmsys::Status status;
  if (err) {
#if defined(_WIN32)
    status = cmsys::Status::Windows(uv_fs_get_system_error(&req));
#elif UV_VERSION_MAJOR > 1 || (UV_VERSION_MAJOR == 1 && UV_VERSION_MINOR >= 38)
    status = cmsys::Status::POSIX(uv_fs_get_system_error(&req));
#else
    status = cmsys::Status::POSIX(-err);
#endif
  }
  return status;
}

cmsys::Status cmSystemTools::CreateLink(std::string const& origName,
                                        std::string const& newName)
{
  cmsys::Status status = cmSystemTools::CreateLinkQuietly(origName, newName);
  if (!status) {
    cmSystemTools::Error(
      cmStrCat("failed to create link '", newName, "': ", status.GetString()));
  }
  return status;
}

cmsys::Status cmSystemTools::CreateLinkQuietly(std::string const& origName,
                                               std::string const& newName)
{
  uv_fs_t req;
  int err =
    uv_fs_link(nullptr, &req, origName.c_str(), newName.c_str(), nullptr);
  cmsys::Status status;
  if (err) {
#if defined(_WIN32)
    status = cmsys::Status::Windows(uv_fs_get_system_error(&req));
#elif UV_VERSION_MAJOR > 1 || (UV_VERSION_MAJOR == 1 && UV_VERSION_MINOR >= 38)
    status = cmsys::Status::POSIX(uv_fs_get_system_error(&req));
#else
    status = cmsys::Status::POSIX(-err);
#endif
  }
  return status;
}

cm::string_view cmSystemTools::GetSystemName()
{
#if defined(_WIN32)
  return "Windows";
#elif defined(__ANDROID__)
  return "Android";
#else
  static struct utsname uts_name;
  static bool initialized = false;
  static cm::string_view systemName;
  if (initialized) {
    return systemName;
  }
  if (uname(&uts_name) >= 0) {
    initialized = true;
    systemName = uts_name.sysname;

    if (cmIsOff(systemName)) {
      systemName = "UnknownOS";
    }

    // fix for BSD/OS, remove the /
    static const cmsys::RegularExpression bsdOsRegex("BSD.OS");
    cmsys::RegularExpressionMatch match;
    if (bsdOsRegex.find(uts_name.sysname, match)) {
      systemName = "BSDOS";
    }

    // fix for GNU/kFreeBSD, remove the GNU/
    if (systemName.find("kFreeBSD") != cm::string_view::npos) {
      systemName = "kFreeBSD";
    }

    // fix for CYGWIN and MSYS which have windows version in them
    if (systemName.find("CYGWIN") != cm::string_view::npos) {
      systemName = "CYGWIN";
    }

    if (systemName.find("MSYS") != cm::string_view::npos) {
      systemName = "MSYS";
    }
    return systemName;
  }
  return "";
#endif
}

char cmSystemTools::GetSystemPathlistSeparator()
{
#if defined(_WIN32)
  return ';';
#else
  return ':';
#endif
}
