/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmFileCommand.h"

#include "cm_kwiml.h"
#include "cm_static_string_view.hxx"
#include "cmsys/FStream.hxx"
#include "cmsys/Glob.hxx"
#include "cmsys/RegularExpression.hxx"

#include <algorithm>
#include <assert.h>
#include <cmath>
#include <ctype.h>
#include <memory> // IWYU pragma: keep
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <utility>
#include <vector>

#include "cmAlgorithms.h"
#include "cmArgumentParser.h"
#include "cmCryptoHash.h"
#include "cmFileCopier.h"
#include "cmFileInstaller.h"
#include "cmFileLockPool.h"
#include "cmFileTimes.h"
#include "cmGeneratorExpression.h"
#include "cmGlobalGenerator.h"
#include "cmHexFileConverter.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmRange.h"
#include "cmSystemTools.h"
#include "cmTimestamp.h"
#include "cm_sys_stat.h"
#include "cmake.h"

#if defined(CMAKE_BUILD_WITH_CMAKE)
#  include "cmCurl.h"
#  include "cmFileLockResult.h"
#  include "cm_curl.h"
#endif

#if defined(CMAKE_USE_ELF_PARSER)
#  include "cmELF.h"
#endif

#if defined(_WIN32)
#  include <windows.h>
#endif

#if defined(_WIN32)
// libcurl doesn't support file:// urls for unicode filenames on Windows.
// Convert string from UTF-8 to ACP if this is a file:// URL.
static std::string fix_file_url_windows(const std::string& url)
{
  std::string ret = url;
  if (strncmp(url.c_str(), "file://", 7) == 0) {
    std::wstring wurl = cmsys::Encoding::ToWide(url);
    if (!wurl.empty()) {
      int mblen =
        WideCharToMultiByte(CP_ACP, 0, wurl.c_str(), -1, NULL, 0, NULL, NULL);
      if (mblen > 0) {
        std::vector<char> chars(mblen);
        mblen = WideCharToMultiByte(CP_ACP, 0, wurl.c_str(), -1, &chars[0],
                                    mblen, NULL, NULL);
        if (mblen > 0) {
          ret = &chars[0];
        }
      }
    }
  }
  return ret;
}
#endif

// cmLibraryCommand
bool cmFileCommand::InitialPass(std::vector<std::string> const& args,
                                cmExecutionStatus&)
{
  if (args.size() < 2) {
    this->SetError("must be called with at least two arguments.");
    return false;
  }
  std::string const& subCommand = args[0];
  if (subCommand == "WRITE") {
    return this->HandleWriteCommand(args, false);
  }
  if (subCommand == "APPEND") {
    return this->HandleWriteCommand(args, true);
  }
  if (subCommand == "DOWNLOAD") {
    return this->HandleDownloadCommand(args);
  }
  if (subCommand == "UPLOAD") {
    return this->HandleUploadCommand(args);
  }
  if (subCommand == "READ") {
    return this->HandleReadCommand(args);
  }
  if (subCommand == "MD5" || subCommand == "SHA1" || subCommand == "SHA224" ||
      subCommand == "SHA256" || subCommand == "SHA384" ||
      subCommand == "SHA512" || subCommand == "SHA3_224" ||
      subCommand == "SHA3_256" || subCommand == "SHA3_384" ||
      subCommand == "SHA3_512") {
    return this->HandleHashCommand(args);
  }
  if (subCommand == "STRINGS") {
    return this->HandleStringsCommand(args);
  }
  if (subCommand == "GLOB") {
    return this->HandleGlobCommand(args, false);
  }
  if (subCommand == "GLOB_RECURSE") {
    return this->HandleGlobCommand(args, true);
  }
  if (subCommand == "MAKE_DIRECTORY") {
    return this->HandleMakeDirectoryCommand(args);
  }
  if (subCommand == "RENAME") {
    return this->HandleRename(args);
  }
  if (subCommand == "REMOVE") {
    return this->HandleRemove(args, false);
  }
  if (subCommand == "REMOVE_RECURSE") {
    return this->HandleRemove(args, true);
  }
  if (subCommand == "COPY") {
    return this->HandleCopyCommand(args);
  }
  if (subCommand == "INSTALL") {
    return this->HandleInstallCommand(args);
  }
  if (subCommand == "DIFFERENT") {
    return this->HandleDifferentCommand(args);
  }
  if (subCommand == "RPATH_CHANGE" || subCommand == "CHRPATH") {
    return this->HandleRPathChangeCommand(args);
  }
  if (subCommand == "RPATH_CHECK") {
    return this->HandleRPathCheckCommand(args);
  }
  if (subCommand == "RPATH_REMOVE") {
    return this->HandleRPathRemoveCommand(args);
  }
  if (subCommand == "READ_ELF") {
    return this->HandleReadElfCommand(args);
  }
  if (subCommand == "RELATIVE_PATH") {
    return this->HandleRelativePathCommand(args);
  }
  if (subCommand == "TO_CMAKE_PATH") {
    return this->HandleCMakePathCommand(args, false);
  }
  if (subCommand == "TO_NATIVE_PATH") {
    return this->HandleCMakePathCommand(args, true);
  }
  if (subCommand == "TOUCH") {
    return this->HandleTouchCommand(args, true);
  }
  if (subCommand == "TOUCH_NOCREATE") {
    return this->HandleTouchCommand(args, false);
  }
  if (subCommand == "TIMESTAMP") {
    return this->HandleTimestampCommand(args);
  }
  if (subCommand == "GENERATE") {
    return this->HandleGenerateCommand(args);
  }
  if (subCommand == "LOCK") {
    return this->HandleLockCommand(args);
  }
  if (subCommand == "SIZE") {
    return this->HandleSizeCommand(args);
  }
  if (subCommand == "READ_SYMLINK") {
    return this->HandleReadSymlinkCommand(args);
  }
  if (subCommand == "CREATE_LINK") {
    return this->HandleCreateLinkCommand(args);
  }

  std::string e = "does not recognize sub-command " + subCommand;
  this->SetError(e);
  return false;
}

bool cmFileCommand::HandleWriteCommand(std::vector<std::string> const& args,
                                       bool append)
{
  std::vector<std::string>::const_iterator i = args.begin();

  i++; // Get rid of subcommand

  std::string fileName = *i;
  if (!cmsys::SystemTools::FileIsFullPath(*i)) {
    fileName = this->Makefile->GetCurrentSourceDirectory();
    fileName += "/" + *i;
  }

  i++;

  if (!this->Makefile->CanIWriteThisFile(fileName)) {
    std::string e =
      "attempted to write a file: " + fileName + " into a source directory.";
    this->SetError(e);
    cmSystemTools::SetFatalErrorOccured();
    return false;
  }
  std::string dir = cmSystemTools::GetFilenamePath(fileName);
  cmSystemTools::MakeDirectory(dir);

  mode_t mode = 0;
  bool writable = false;

  // Set permissions to writable
  if (cmSystemTools::GetPermissions(fileName, mode)) {
#if defined(_MSC_VER) || defined(__MINGW32__)
    writable = (mode & S_IWRITE) != 0;
    mode_t newMode = mode | S_IWRITE;
#else
    writable = mode & S_IWUSR;
    mode_t newMode = mode | S_IWUSR | S_IWGRP;
#endif
    if (!writable) {
      cmSystemTools::SetPermissions(fileName, newMode);
    }
  }
  // If GetPermissions fails, pretend like it is ok. File open will fail if
  // the file is not writable
  cmsys::ofstream file(fileName.c_str(),
                       append ? std::ios::app : std::ios::out);
  if (!file) {
    std::string error = "failed to open for writing (";
    error += cmSystemTools::GetLastSystemError();
    error += "):\n  ";
    error += fileName;
    this->SetError(error);
    return false;
  }
  std::string message = cmJoin(cmMakeRange(i, args.end()), std::string());
  file << message;
  if (!file) {
    std::string error = "write failed (";
    error += cmSystemTools::GetLastSystemError();
    error += "):\n  ";
    error += fileName;
    this->SetError(error);
    return false;
  }
  file.close();
  if (mode && !writable) {
    cmSystemTools::SetPermissions(fileName, mode);
  }
  return true;
}

bool cmFileCommand::HandleReadCommand(std::vector<std::string> const& args)
{
  if (args.size() < 3) {
    this->SetError("READ must be called with at least two additional "
                   "arguments");
    return false;
  }

  std::string const& fileNameArg = args[1];
  std::string const& variable = args[2];

  struct Arguments
  {
    std::string Offset;
    std::string Limit;
    bool Hex = false;
  };

  static auto const parser = cmArgumentParser<Arguments>{}
                               .Bind("OFFSET"_s, &Arguments::Offset)
                               .Bind("LIMIT"_s, &Arguments::Limit)
                               .Bind("HEX"_s, &Arguments::Hex);

  Arguments const arguments = parser.Parse(cmMakeRange(args).advance(3));

  std::string fileName = fileNameArg;
  if (!cmsys::SystemTools::FileIsFullPath(fileName)) {
    fileName = this->Makefile->GetCurrentSourceDirectory();
    fileName += "/" + fileNameArg;
  }

// Open the specified file.
#if defined(_WIN32) || defined(__CYGWIN__)
  cmsys::ifstream file(fileName.c_str(),
                       arguments.Hex ? (std::ios::binary | std::ios::in)
                                     : std::ios::in);
#else
  cmsys::ifstream file(fileName.c_str());
#endif

  if (!file) {
    std::string error = "failed to open for reading (";
    error += cmSystemTools::GetLastSystemError();
    error += "):\n  ";
    error += fileName;
    this->SetError(error);
    return false;
  }

  // is there a limit?
  long sizeLimit = -1;
  if (!arguments.Limit.empty()) {
    sizeLimit = atoi(arguments.Limit.c_str());
  }

  // is there an offset?
  long offset = 0;
  if (!arguments.Offset.empty()) {
    offset = atoi(arguments.Offset.c_str());
  }

  file.seekg(offset, std::ios::beg); // explicit ios::beg for IBM VisualAge 6

  std::string output;

  if (arguments.Hex) {
    // Convert part of the file into hex code
    char c;
    while ((sizeLimit != 0) && (file.get(c))) {
      char hex[4];
      sprintf(hex, "%.2x", c & 0xff);
      output += hex;
      if (sizeLimit > 0) {
        sizeLimit--;
      }
    }
  } else {
    std::string line;
    bool has_newline = false;
    while (
      sizeLimit != 0 &&
      cmSystemTools::GetLineFromStream(file, line, &has_newline, sizeLimit)) {
      if (sizeLimit > 0) {
        sizeLimit = sizeLimit - static_cast<long>(line.size());
        if (has_newline) {
          sizeLimit--;
        }
        if (sizeLimit < 0) {
          sizeLimit = 0;
        }
      }
      output += line;
      if (has_newline) {
        output += "\n";
      }
    }
  }
  this->Makefile->AddDefinition(variable, output.c_str());
  return true;
}

bool cmFileCommand::HandleHashCommand(std::vector<std::string> const& args)
{
#if defined(CMAKE_BUILD_WITH_CMAKE)
  if (args.size() != 3) {
    std::ostringstream e;
    e << args[0] << " requires a file name and output variable";
    this->SetError(e.str());
    return false;
  }

  std::unique_ptr<cmCryptoHash> hash(cmCryptoHash::New(args[0].c_str()));
  if (hash) {
    std::string out = hash->HashFile(args[1]);
    if (!out.empty()) {
      this->Makefile->AddDefinition(args[2], out.c_str());
      return true;
    }
    std::ostringstream e;
    e << args[0] << " failed to read file \"" << args[1]
      << "\": " << cmSystemTools::GetLastSystemError();
    this->SetError(e.str());
  }
  return false;
#else
  std::ostringstream e;
  e << args[0] << " not available during bootstrap";
  this->SetError(e.str());
  return false;
#endif
}

bool cmFileCommand::HandleStringsCommand(std::vector<std::string> const& args)
{
  if (args.size() < 3) {
    this->SetError("STRINGS requires a file name and output variable");
    return false;
  }

  // Get the file to read.
  std::string fileName = args[1];
  if (!cmsys::SystemTools::FileIsFullPath(fileName)) {
    fileName = this->Makefile->GetCurrentSourceDirectory();
    fileName += "/" + args[1];
  }

  // Get the variable in which to store the results.
  std::string outVar = args[2];

  // Parse the options.
  enum
  {
    arg_none,
    arg_limit_input,
    arg_limit_output,
    arg_limit_count,
    arg_length_minimum,
    arg_length_maximum,
    arg__maximum,
    arg_regex,
    arg_encoding
  };
  unsigned int minlen = 0;
  unsigned int maxlen = 0;
  int limit_input = -1;
  int limit_output = -1;
  unsigned int limit_count = 0;
  cmsys::RegularExpression regex;
  bool have_regex = false;
  bool newline_consume = false;
  bool hex_conversion_enabled = true;
  enum
  {
    encoding_none = cmsys::FStream::BOM_None,
    encoding_utf8 = cmsys::FStream::BOM_UTF8,
    encoding_utf16le = cmsys::FStream::BOM_UTF16LE,
    encoding_utf16be = cmsys::FStream::BOM_UTF16BE,
    encoding_utf32le = cmsys::FStream::BOM_UTF32LE,
    encoding_utf32be = cmsys::FStream::BOM_UTF32BE
  };
  int encoding = encoding_none;
  int arg_mode = arg_none;
  for (unsigned int i = 3; i < args.size(); ++i) {
    if (args[i] == "LIMIT_INPUT") {
      arg_mode = arg_limit_input;
    } else if (args[i] == "LIMIT_OUTPUT") {
      arg_mode = arg_limit_output;
    } else if (args[i] == "LIMIT_COUNT") {
      arg_mode = arg_limit_count;
    } else if (args[i] == "LENGTH_MINIMUM") {
      arg_mode = arg_length_minimum;
    } else if (args[i] == "LENGTH_MAXIMUM") {
      arg_mode = arg_length_maximum;
    } else if (args[i] == "REGEX") {
      arg_mode = arg_regex;
    } else if (args[i] == "NEWLINE_CONSUME") {
      newline_consume = true;
      arg_mode = arg_none;
    } else if (args[i] == "NO_HEX_CONVERSION") {
      hex_conversion_enabled = false;
      arg_mode = arg_none;
    } else if (args[i] == "ENCODING") {
      arg_mode = arg_encoding;
    } else if (arg_mode == arg_limit_input) {
      if (sscanf(args[i].c_str(), "%d", &limit_input) != 1 ||
          limit_input < 0) {
        std::ostringstream e;
        e << "STRINGS option LIMIT_INPUT value \"" << args[i]
          << "\" is not an unsigned integer.";
        this->SetError(e.str());
        return false;
      }
      arg_mode = arg_none;
    } else if (arg_mode == arg_limit_output) {
      if (sscanf(args[i].c_str(), "%d", &limit_output) != 1 ||
          limit_output < 0) {
        std::ostringstream e;
        e << "STRINGS option LIMIT_OUTPUT value \"" << args[i]
          << "\" is not an unsigned integer.";
        this->SetError(e.str());
        return false;
      }
      arg_mode = arg_none;
    } else if (arg_mode == arg_limit_count) {
      int count;
      if (sscanf(args[i].c_str(), "%d", &count) != 1 || count < 0) {
        std::ostringstream e;
        e << "STRINGS option LIMIT_COUNT value \"" << args[i]
          << "\" is not an unsigned integer.";
        this->SetError(e.str());
        return false;
      }
      limit_count = count;
      arg_mode = arg_none;
    } else if (arg_mode == arg_length_minimum) {
      int len;
      if (sscanf(args[i].c_str(), "%d", &len) != 1 || len < 0) {
        std::ostringstream e;
        e << "STRINGS option LENGTH_MINIMUM value \"" << args[i]
          << "\" is not an unsigned integer.";
        this->SetError(e.str());
        return false;
      }
      minlen = len;
      arg_mode = arg_none;
    } else if (arg_mode == arg_length_maximum) {
      int len;
      if (sscanf(args[i].c_str(), "%d", &len) != 1 || len < 0) {
        std::ostringstream e;
        e << "STRINGS option LENGTH_MAXIMUM value \"" << args[i]
          << "\" is not an unsigned integer.";
        this->SetError(e.str());
        return false;
      }
      maxlen = len;
      arg_mode = arg_none;
    } else if (arg_mode == arg_regex) {
      if (!regex.compile(args[i])) {
        std::ostringstream e;
        e << "STRINGS option REGEX value \"" << args[i]
          << "\" could not be compiled.";
        this->SetError(e.str());
        return false;
      }
      have_regex = true;
      arg_mode = arg_none;
    } else if (arg_mode == arg_encoding) {
      if (args[i] == "UTF-8") {
        encoding = encoding_utf8;
      } else if (args[i] == "UTF-16LE") {
        encoding = encoding_utf16le;
      } else if (args[i] == "UTF-16BE") {
        encoding = encoding_utf16be;
      } else if (args[i] == "UTF-32LE") {
        encoding = encoding_utf32le;
      } else if (args[i] == "UTF-32BE") {
        encoding = encoding_utf32be;
      } else {
        std::ostringstream e;
        e << "STRINGS option ENCODING \"" << args[i] << "\" not recognized.";
        this->SetError(e.str());
        return false;
      }
      arg_mode = arg_none;
    } else {
      std::ostringstream e;
      e << "STRINGS given unknown argument \"" << args[i] << "\"";
      this->SetError(e.str());
      return false;
    }
  }

  if (hex_conversion_enabled) {
    // TODO: should work without temp file, but just on a memory buffer
    std::string binaryFileName = this->Makefile->GetCurrentBinaryDirectory();
    binaryFileName += "/CMakeFiles";
    binaryFileName += "/FileCommandStringsBinaryFile";
    if (cmHexFileConverter::TryConvert(fileName, binaryFileName)) {
      fileName = binaryFileName;
    }
  }

// Open the specified file.
#if defined(_WIN32) || defined(__CYGWIN__)
  cmsys::ifstream fin(fileName.c_str(), std::ios::in | std::ios::binary);
#else
  cmsys::ifstream fin(fileName.c_str());
#endif
  if (!fin) {
    std::ostringstream e;
    e << "STRINGS file \"" << fileName << "\" cannot be read.";
    this->SetError(e.str());
    return false;
  }

  // If BOM is found and encoding was not specified, use the BOM
  int bom_found = cmsys::FStream::ReadBOM(fin);
  if (encoding == encoding_none && bom_found != cmsys::FStream::BOM_None) {
    encoding = bom_found;
  }

  unsigned int bytes_rem = 0;
  if (encoding == encoding_utf16le || encoding == encoding_utf16be) {
    bytes_rem = 1;
  }
  if (encoding == encoding_utf32le || encoding == encoding_utf32be) {
    bytes_rem = 3;
  }

  // Parse strings out of the file.
  int output_size = 0;
  std::vector<std::string> strings;
  std::string s;
  while ((!limit_count || strings.size() < limit_count) &&
         (limit_input < 0 || static_cast<int>(fin.tellg()) < limit_input) &&
         fin) {
    std::string current_str;

    int c = fin.get();
    for (unsigned int i = 0; i < bytes_rem; ++i) {
      int c1 = fin.get();
      if (!fin) {
        fin.putback(static_cast<char>(c1));
        break;
      }
      c = (c << 8) | c1;
    }
    if (encoding == encoding_utf16le) {
      c = ((c & 0xFF) << 8) | ((c & 0xFF00) >> 8);
    } else if (encoding == encoding_utf32le) {
      c = (((c & 0xFF) << 24) | ((c & 0xFF00) << 8) | ((c & 0xFF0000) >> 8) |
           ((c & 0xFF000000) >> 24));
    }

    if (c == '\r') {
      // Ignore CR character to make output always have UNIX newlines.
      continue;
    }

    if (c >= 0 && c <= 0xFF &&
        (isprint(c) || c == '\t' || (c == '\n' && newline_consume))) {
      // This is an ASCII character that may be part of a string.
      // Cast added to avoid compiler warning. Cast is ok because
      // c is guaranteed to fit in char by the above if...
      current_str += static_cast<char>(c);
    } else if (encoding == encoding_utf8) {
      // Check for UTF-8 encoded string (up to 4 octets)
      static const unsigned char utf8_check_table[3][2] = {
        { 0xE0, 0xC0 },
        { 0xF0, 0xE0 },
        { 0xF8, 0xF0 },
      };

      // how many octets are there?
      unsigned int num_utf8_bytes = 0;
      for (unsigned int j = 0; num_utf8_bytes == 0 && j < 3; j++) {
        if ((c & utf8_check_table[j][0]) == utf8_check_table[j][1]) {
          num_utf8_bytes = j + 2;
        }
      }

      // get subsequent octets and check that they are valid
      for (unsigned int j = 0; j < num_utf8_bytes; j++) {
        if (j != 0) {
          c = fin.get();
          if (!fin || (c & 0xC0) != 0x80) {
            fin.putback(static_cast<char>(c));
            break;
          }
        }
        current_str += static_cast<char>(c);
      }

      // if this was an invalid utf8 sequence, discard the data, and put
      // back subsequent characters
      if ((current_str.length() != num_utf8_bytes)) {
        for (unsigned int j = 0; j < current_str.size() - 1; j++) {
          c = current_str[current_str.size() - 1 - j];
          fin.putback(static_cast<char>(c));
        }
        current_str.clear();
      }
    }

    if (c == '\n' && !newline_consume) {
      // The current line has been terminated.  Check if the current
      // string matches the requirements.  The length may now be as
      // low as zero since blank lines are allowed.
      if (s.length() >= minlen && (!have_regex || regex.find(s))) {
        output_size += static_cast<int>(s.size()) + 1;
        if (limit_output >= 0 && output_size >= limit_output) {
          s.clear();
          break;
        }
        strings.push_back(s);
      }

      // Reset the string to empty.
      s.clear();
    } else if (current_str.empty()) {
      // A non-string character has been found.  Check if the current
      // string matches the requirements.  We require that the length
      // be at least one no matter what the user specified.
      if (s.length() >= minlen && !s.empty() &&
          (!have_regex || regex.find(s))) {
        output_size += static_cast<int>(s.size()) + 1;
        if (limit_output >= 0 && output_size >= limit_output) {
          s.clear();
          break;
        }
        strings.push_back(s);
      }

      // Reset the string to empty.
      s.clear();
    } else {
      s += current_str;
    }

    if (maxlen > 0 && s.size() == maxlen) {
      // Terminate a string if the maximum length is reached.
      if (s.length() >= minlen && (!have_regex || regex.find(s))) {
        output_size += static_cast<int>(s.size()) + 1;
        if (limit_output >= 0 && output_size >= limit_output) {
          s.clear();
          break;
        }
        strings.push_back(s);
      }
      s.clear();
    }
  }

  // If there is a non-empty current string we have hit the end of the
  // input file or the input size limit.  Check if the current string
  // matches the requirements.
  if ((!limit_count || strings.size() < limit_count) && !s.empty() &&
      s.length() >= minlen && (!have_regex || regex.find(s))) {
    output_size += static_cast<int>(s.size()) + 1;
    if (limit_output < 0 || output_size < limit_output) {
      strings.push_back(s);
    }
  }

  // Encode the result in a CMake list.
  const char* sep = "";
  std::string output;
  for (std::string const& sr : strings) {
    // Separate the strings in the output to make it a list.
    output += sep;
    sep = ";";

    // Store the string in the output, but escape semicolons to
    // make sure it is a list.
    for (char i : sr) {
      if (i == ';') {
        output += '\\';
      }
      output += i;
    }
  }

  // Save the output in a makefile variable.
  this->Makefile->AddDefinition(outVar, output.c_str());
  return true;
}

bool cmFileCommand::HandleGlobCommand(std::vector<std::string> const& args,
                                      bool recurse)
{
  // File commands has at least one argument
  assert(args.size() > 1);

  std::vector<std::string>::const_iterator i = args.begin();

  i++; // Get rid of subcommand

  std::string variable = *i;
  i++;
  cmsys::Glob g;
  g.SetRecurse(recurse);

  bool explicitFollowSymlinks = false;
  cmPolicies::PolicyStatus status =
    this->Makefile->GetPolicyStatus(cmPolicies::CMP0009);
  if (recurse) {
    switch (status) {
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
      case cmPolicies::NEW:
        g.RecurseThroughSymlinksOff();
        break;
      case cmPolicies::OLD:
      case cmPolicies::WARN:
        g.RecurseThroughSymlinksOn();
        break;
    }
  }

  std::vector<std::string> files;
  bool configureDepends = false;
  bool warnConfigureLate = false;
  bool warnFollowedSymlinks = false;
  const cmake::WorkingMode workingMode =
    this->Makefile->GetCMakeInstance()->GetWorkingMode();
  while (i != args.end()) {
    if (*i == "LIST_DIRECTORIES") {
      ++i; // skip LIST_DIRECTORIES
      if (i != args.end()) {
        if (cmSystemTools::IsOn(*i)) {
          g.SetListDirs(true);
          g.SetRecurseListDirs(true);
        } else if (cmSystemTools::IsOff(*i)) {
          g.SetListDirs(false);
          g.SetRecurseListDirs(false);
        } else {
          this->SetError("LIST_DIRECTORIES missing bool value.");
          return false;
        }
        ++i;
      } else {
        this->SetError("LIST_DIRECTORIES missing bool value.");
        return false;
      }
    } else if (*i == "FOLLOW_SYMLINKS") {
      ++i; // skip FOLLOW_SYMLINKS
      if (recurse) {
        explicitFollowSymlinks = true;
        g.RecurseThroughSymlinksOn();
        if (i == args.end()) {
          this->SetError(
            "GLOB_RECURSE requires a glob expression after FOLLOW_SYMLINKS.");
          return false;
        }
      }
    } else if (*i == "RELATIVE") {
      ++i; // skip RELATIVE
      if (i == args.end()) {
        this->SetError("GLOB requires a directory after the RELATIVE tag.");
        return false;
      }
      g.SetRelative(i->c_str());
      ++i;
      if (i == args.end()) {
        this->SetError("GLOB requires a glob expression after the directory.");
        return false;
      }
    } else if (*i == "CONFIGURE_DEPENDS") {
      // Generated build system depends on glob results
      if (!configureDepends && warnConfigureLate) {
        this->Makefile->IssueMessage(
          MessageType::AUTHOR_WARNING,
          "CONFIGURE_DEPENDS flag was given after a glob expression was "
          "already evaluated.");
      }
      if (workingMode != cmake::NORMAL_MODE) {
        this->Makefile->IssueMessage(
          MessageType::FATAL_ERROR,
          "CONFIGURE_DEPENDS is invalid for script and find package modes.");
        return false;
      }
      configureDepends = true;
      ++i;
      if (i == args.end()) {
        this->SetError(
          "GLOB requires a glob expression after CONFIGURE_DEPENDS.");
        return false;
      }
    } else {
      std::string expr = *i;
      if (!cmsys::SystemTools::FileIsFullPath(*i)) {
        expr = this->Makefile->GetCurrentSourceDirectory();
        // Handle script mode
        if (!expr.empty()) {
          expr += "/" + *i;
        } else {
          expr = *i;
        }
      }

      cmsys::Glob::GlobMessages globMessages;
      g.FindFiles(expr, &globMessages);

      if (!globMessages.empty()) {
        bool shouldExit = false;
        for (cmsys::Glob::Message const& globMessage : globMessages) {
          if (globMessage.type == cmsys::Glob::cyclicRecursion) {
            this->Makefile->IssueMessage(
              MessageType::AUTHOR_WARNING,
              "Cyclic recursion detected while globbing for '" + *i + "':\n" +
                globMessage.content);
          } else {
            this->Makefile->IssueMessage(
              MessageType::FATAL_ERROR,
              "Error has occurred while globbing for '" + *i + "' - " +
                globMessage.content);
            shouldExit = true;
          }
        }
        if (shouldExit) {
          return false;
        }
      }

      if (recurse && !explicitFollowSymlinks &&
          g.GetFollowedSymlinkCount() != 0) {
        warnFollowedSymlinks = true;
      }

      std::vector<std::string>& foundFiles = g.GetFiles();
      cmAppend(files, foundFiles);

      if (configureDepends) {
        std::sort(foundFiles.begin(), foundFiles.end());
        foundFiles.erase(std::unique(foundFiles.begin(), foundFiles.end()),
                         foundFiles.end());
        this->Makefile->GetCMakeInstance()->AddGlobCacheEntry(
          recurse, (recurse ? g.GetRecurseListDirs() : g.GetListDirs()),
          (recurse ? g.GetRecurseThroughSymlinks() : false),
          (g.GetRelative() ? g.GetRelative() : ""), expr, foundFiles, variable,
          this->Makefile->GetBacktrace());
      } else {
        warnConfigureLate = true;
      }
      ++i;
    }
  }

  switch (status) {
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::REQUIRED_ALWAYS:
    case cmPolicies::NEW:
      // Correct behavior, yay!
      break;
    case cmPolicies::OLD:
    // Probably not really the expected behavior, but the author explicitly
    // asked for the old behavior... no warning.
    case cmPolicies::WARN:
      // Possibly unexpected old behavior *and* we actually traversed
      // symlinks without being explicitly asked to: warn the author.
      if (warnFollowedSymlinks) {
        this->Makefile->IssueMessage(
          MessageType::AUTHOR_WARNING,
          cmPolicies::GetPolicyWarning(cmPolicies::CMP0009));
      }
      break;
  }

  std::sort(files.begin(), files.end());
  files.erase(std::unique(files.begin(), files.end()), files.end());
  this->Makefile->AddDefinition(variable, cmJoin(files, ";").c_str());
  return true;
}

bool cmFileCommand::HandleMakeDirectoryCommand(
  std::vector<std::string> const& args)
{
  // File command has at least one argument
  assert(args.size() > 1);

  std::string expr;
  for (std::string const& arg :
       cmMakeRange(args).advance(1)) // Get rid of subcommand
  {
    const std::string* cdir = &arg;
    if (!cmsys::SystemTools::FileIsFullPath(arg)) {
      expr = this->Makefile->GetCurrentSourceDirectory();
      expr += "/" + arg;
      cdir = &expr;
    }
    if (!this->Makefile->CanIWriteThisFile(*cdir)) {
      std::string e = "attempted to create a directory: " + *cdir +
        " into a source directory.";
      this->SetError(e);
      cmSystemTools::SetFatalErrorOccured();
      return false;
    }
    if (!cmSystemTools::MakeDirectory(*cdir)) {
      std::string error = "problem creating directory: " + *cdir;
      this->SetError(error);
      return false;
    }
  }
  return true;
}

bool cmFileCommand::HandleTouchCommand(std::vector<std::string> const& args,
                                       bool create)
{
  // File command has at least one argument
  assert(args.size() > 1);

  for (std::string const& arg :
       cmMakeRange(args).advance(1)) // Get rid of subcommand
  {
    std::string tfile = arg;
    if (!cmsys::SystemTools::FileIsFullPath(tfile)) {
      tfile = this->Makefile->GetCurrentSourceDirectory();
      tfile += "/" + arg;
    }
    if (!this->Makefile->CanIWriteThisFile(tfile)) {
      std::string e =
        "attempted to touch a file: " + tfile + " in a source directory.";
      this->SetError(e);
      cmSystemTools::SetFatalErrorOccured();
      return false;
    }
    if (!cmSystemTools::Touch(tfile, create)) {
      std::string error = "problem touching file: " + tfile;
      this->SetError(error);
      return false;
    }
  }
  return true;
}

bool cmFileCommand::HandleDifferentCommand(
  std::vector<std::string> const& args)
{
  /*
    FILE(DIFFERENT <variable> FILES <lhs> <rhs>)
   */

  // Evaluate arguments.
  const char* file_lhs = nullptr;
  const char* file_rhs = nullptr;
  const char* var = nullptr;
  enum Doing
  {
    DoingNone,
    DoingVar,
    DoingFileLHS,
    DoingFileRHS
  };
  Doing doing = DoingVar;
  for (unsigned int i = 1; i < args.size(); ++i) {
    if (args[i] == "FILES") {
      doing = DoingFileLHS;
    } else if (doing == DoingVar) {
      var = args[i].c_str();
      doing = DoingNone;
    } else if (doing == DoingFileLHS) {
      file_lhs = args[i].c_str();
      doing = DoingFileRHS;
    } else if (doing == DoingFileRHS) {
      file_rhs = args[i].c_str();
      doing = DoingNone;
    } else {
      std::ostringstream e;
      e << "DIFFERENT given unknown argument " << args[i];
      this->SetError(e.str());
      return false;
    }
  }
  if (!var) {
    this->SetError("DIFFERENT not given result variable name.");
    return false;
  }
  if (!file_lhs || !file_rhs) {
    this->SetError("DIFFERENT not given FILES option with two file names.");
    return false;
  }

  // Compare the files.
  const char* result =
    cmSystemTools::FilesDiffer(file_lhs, file_rhs) ? "1" : "0";
  this->Makefile->AddDefinition(var, result);
  return true;
}

bool cmFileCommand::HandleCopyCommand(std::vector<std::string> const& args)
{
  cmFileCopier copier(this);
  return copier.Run(args);
}

bool cmFileCommand::HandleRPathChangeCommand(
  std::vector<std::string> const& args)
{
  // Evaluate arguments.
  std::string file;
  const char* oldRPath = nullptr;
  const char* newRPath = nullptr;
  enum Doing
  {
    DoingNone,
    DoingFile,
    DoingOld,
    DoingNew
  };
  Doing doing = DoingNone;
  for (unsigned int i = 1; i < args.size(); ++i) {
    if (args[i] == "OLD_RPATH") {
      doing = DoingOld;
    } else if (args[i] == "NEW_RPATH") {
      doing = DoingNew;
    } else if (args[i] == "FILE") {
      doing = DoingFile;
    } else if (doing == DoingFile) {
      file = args[i];
      doing = DoingNone;
    } else if (doing == DoingOld) {
      oldRPath = args[i].c_str();
      doing = DoingNone;
    } else if (doing == DoingNew) {
      newRPath = args[i].c_str();
      doing = DoingNone;
    } else {
      std::ostringstream e;
      e << "RPATH_CHANGE given unknown argument " << args[i];
      this->SetError(e.str());
      return false;
    }
  }
  if (file.empty()) {
    this->SetError("RPATH_CHANGE not given FILE option.");
    return false;
  }
  if (!oldRPath) {
    this->SetError("RPATH_CHANGE not given OLD_RPATH option.");
    return false;
  }
  if (!newRPath) {
    this->SetError("RPATH_CHANGE not given NEW_RPATH option.");
    return false;
  }
  if (!cmSystemTools::FileExists(file, true)) {
    std::ostringstream e;
    e << "RPATH_CHANGE given FILE \"" << file << "\" that does not exist.";
    this->SetError(e.str());
    return false;
  }
  bool success = true;
  cmFileTimes const ft(file);
  std::string emsg;
  bool changed;
  if (!cmSystemTools::ChangeRPath(file, oldRPath, newRPath, &emsg, &changed)) {
    std::ostringstream e;
    /* clang-format off */
    e << "RPATH_CHANGE could not write new RPATH:\n"
      << "  " << newRPath << "\n"
      << "to the file:\n"
      << "  " << file << "\n"
      << emsg;
    /* clang-format on */
    this->SetError(e.str());
    success = false;
  }
  if (success) {
    if (changed) {
      std::string message = "Set runtime path of \"";
      message += file;
      message += "\" to \"";
      message += newRPath;
      message += "\"";
      this->Makefile->DisplayStatus(message, -1);
    }
    ft.Store(file);
  }
  return success;
}

bool cmFileCommand::HandleRPathRemoveCommand(
  std::vector<std::string> const& args)
{
  // Evaluate arguments.
  std::string file;
  enum Doing
  {
    DoingNone,
    DoingFile
  };
  Doing doing = DoingNone;
  for (unsigned int i = 1; i < args.size(); ++i) {
    if (args[i] == "FILE") {
      doing = DoingFile;
    } else if (doing == DoingFile) {
      file = args[i];
      doing = DoingNone;
    } else {
      std::ostringstream e;
      e << "RPATH_REMOVE given unknown argument " << args[i];
      this->SetError(e.str());
      return false;
    }
  }
  if (file.empty()) {
    this->SetError("RPATH_REMOVE not given FILE option.");
    return false;
  }
  if (!cmSystemTools::FileExists(file, true)) {
    std::ostringstream e;
    e << "RPATH_REMOVE given FILE \"" << file << "\" that does not exist.";
    this->SetError(e.str());
    return false;
  }
  bool success = true;
  cmFileTimes const ft(file);
  std::string emsg;
  bool removed;
  if (!cmSystemTools::RemoveRPath(file, &emsg, &removed)) {
    std::ostringstream e;
    /* clang-format off */
    e << "RPATH_REMOVE could not remove RPATH from file:\n"
      << "  " << file << "\n"
      << emsg;
    /* clang-format on */
    this->SetError(e.str());
    success = false;
  }
  if (success) {
    if (removed) {
      std::string message = "Removed runtime path from \"";
      message += file;
      message += "\"";
      this->Makefile->DisplayStatus(message, -1);
    }
    ft.Store(file);
  }
  return success;
}

bool cmFileCommand::HandleRPathCheckCommand(
  std::vector<std::string> const& args)
{
  // Evaluate arguments.
  std::string file;
  const char* rpath = nullptr;
  enum Doing
  {
    DoingNone,
    DoingFile,
    DoingRPath
  };
  Doing doing = DoingNone;
  for (unsigned int i = 1; i < args.size(); ++i) {
    if (args[i] == "RPATH") {
      doing = DoingRPath;
    } else if (args[i] == "FILE") {
      doing = DoingFile;
    } else if (doing == DoingFile) {
      file = args[i];
      doing = DoingNone;
    } else if (doing == DoingRPath) {
      rpath = args[i].c_str();
      doing = DoingNone;
    } else {
      std::ostringstream e;
      e << "RPATH_CHECK given unknown argument " << args[i];
      this->SetError(e.str());
      return false;
    }
  }
  if (file.empty()) {
    this->SetError("RPATH_CHECK not given FILE option.");
    return false;
  }
  if (!rpath) {
    this->SetError("RPATH_CHECK not given RPATH option.");
    return false;
  }

  // If the file exists but does not have the desired RPath then
  // delete it.  This is used during installation to re-install a file
  // if its RPath will change.
  if (cmSystemTools::FileExists(file, true) &&
      !cmSystemTools::CheckRPath(file, rpath)) {
    cmSystemTools::RemoveFile(file);
  }

  return true;
}

bool cmFileCommand::HandleReadElfCommand(std::vector<std::string> const& args)
{
  if (args.size() < 4) {
    this->SetError("READ_ELF must be called with at least three additional "
                   "arguments.");
    return false;
  }

  std::string const& fileNameArg = args[1];

  struct Arguments
  {
    std::string RPath;
    std::string RunPath;
    std::string Error;
  };

  static auto const parser = cmArgumentParser<Arguments>{}
                               .Bind("RPATH"_s, &Arguments::RPath)
                               .Bind("RUNPATH"_s, &Arguments::RunPath)
                               .Bind("CAPTURE_ERROR"_s, &Arguments::Error);
  Arguments const arguments = parser.Parse(cmMakeRange(args).advance(2));

  if (!cmSystemTools::FileExists(fileNameArg, true)) {
    std::ostringstream e;
    e << "READ_ELF given FILE \"" << fileNameArg << "\" that does not exist.";
    this->SetError(e.str());
    return false;
  }

#if defined(CMAKE_USE_ELF_PARSER)
  cmELF elf(fileNameArg.c_str());

  if (!arguments.RPath.empty()) {
    if (cmELF::StringEntry const* se_rpath = elf.GetRPath()) {
      std::string rpath(se_rpath->Value);
      std::replace(rpath.begin(), rpath.end(), ':', ';');
      this->Makefile->AddDefinition(arguments.RPath, rpath.c_str());
    }
  }
  if (!arguments.RunPath.empty()) {
    if (cmELF::StringEntry const* se_runpath = elf.GetRunPath()) {
      std::string runpath(se_runpath->Value);
      std::replace(runpath.begin(), runpath.end(), ':', ';');
      this->Makefile->AddDefinition(arguments.RunPath, runpath.c_str());
    }
  }

  return true;
#else
  std::string error = "ELF parser not available on this platform.";
  if (arguments.Error.empty()) {
    this->SetError(error);
    return false;
  }
  this->Makefile->AddDefinition(arguments.Error, error.c_str());
  return true;
#endif
}

bool cmFileCommand::HandleInstallCommand(std::vector<std::string> const& args)
{
  cmFileInstaller installer(this);
  return installer.Run(args);
}

bool cmFileCommand::HandleRelativePathCommand(
  std::vector<std::string> const& args)
{
  if (args.size() != 4) {
    this->SetError("RELATIVE_PATH called with incorrect number of arguments");
    return false;
  }

  const std::string& outVar = args[1];
  const std::string& directoryName = args[2];
  const std::string& fileName = args[3];

  if (!cmSystemTools::FileIsFullPath(directoryName)) {
    std::string errstring =
      "RELATIVE_PATH must be passed a full path to the directory: " +
      directoryName;
    this->SetError(errstring);
    return false;
  }
  if (!cmSystemTools::FileIsFullPath(fileName)) {
    std::string errstring =
      "RELATIVE_PATH must be passed a full path to the file: " + fileName;
    this->SetError(errstring);
    return false;
  }

  std::string res = cmSystemTools::RelativePath(directoryName, fileName);
  this->Makefile->AddDefinition(outVar, res.c_str());
  return true;
}

bool cmFileCommand::HandleRename(std::vector<std::string> const& args)
{
  if (args.size() != 3) {
    this->SetError("RENAME given incorrect number of arguments.");
    return false;
  }

  // Compute full path for old and new names.
  std::string oldname = args[1];
  if (!cmsys::SystemTools::FileIsFullPath(oldname)) {
    oldname = this->Makefile->GetCurrentSourceDirectory();
    oldname += "/" + args[1];
  }
  std::string newname = args[2];
  if (!cmsys::SystemTools::FileIsFullPath(newname)) {
    newname = this->Makefile->GetCurrentSourceDirectory();
    newname += "/" + args[2];
  }

  if (!cmSystemTools::RenameFile(oldname, newname)) {
    std::string err = cmSystemTools::GetLastSystemError();
    std::ostringstream e;
    /* clang-format off */
    e << "RENAME failed to rename\n"
      << "  " << oldname << "\n"
      << "to\n"
      << "  " << newname << "\n"
      << "because: " << err << "\n";
    /* clang-format on */
    this->SetError(e.str());
    return false;
  }
  return true;
}

bool cmFileCommand::HandleRemove(std::vector<std::string> const& args,
                                 bool recurse)
{

  std::string message;

  for (std::string const& arg :
       cmMakeRange(args).advance(1)) // Get rid of subcommand
  {
    std::string fileName = arg;
    if (fileName.empty()) {
      std::string const r = recurse ? "REMOVE_RECURSE" : "REMOVE";
      this->Makefile->IssueMessage(MessageType::AUTHOR_WARNING,
                                   "Ignoring empty file name in " + r + ".");
      continue;
    }
    if (!cmsys::SystemTools::FileIsFullPath(fileName)) {
      fileName = this->Makefile->GetCurrentSourceDirectory();
      fileName += "/" + arg;
    }

    if (cmSystemTools::FileIsDirectory(fileName) &&
        !cmSystemTools::FileIsSymlink(fileName) && recurse) {
      cmSystemTools::RemoveADirectory(fileName);
    } else {
      cmSystemTools::RemoveFile(fileName);
    }
  }
  return true;
}

namespace {
std::string ToNativePath(const std::string& path)
{
  const auto& outPath = cmSystemTools::ConvertToOutputPath(path);
  if (outPath.size() > 1 && outPath.front() == '\"' &&
      outPath.back() == '\"') {
    return outPath.substr(1, outPath.size() - 2);
  }
  return outPath;
}

std::string ToCMakePath(const std::string& path)
{
  auto temp = path;
  cmSystemTools::ConvertToUnixSlashes(temp);
  return temp;
}
}

bool cmFileCommand::HandleCMakePathCommand(
  std::vector<std::string> const& args, bool nativePath)
{
  if (args.size() != 3) {
    this->SetError("FILE([TO_CMAKE_PATH|TO_NATIVE_PATH] path result) must be "
                   "called with exactly three arguments.");
    return false;
  }
#if defined(_WIN32) && !defined(__CYGWIN__)
  char pathSep = ';';
#else
  char pathSep = ':';
#endif
  std::vector<std::string> path = cmSystemTools::SplitString(args[1], pathSep);

  std::string value = cmJoin(
    cmMakeRange(path).transform(nativePath ? ToNativePath : ToCMakePath), ";");
  this->Makefile->AddDefinition(args[2], value.c_str());
  return true;
}

#if defined(CMAKE_BUILD_WITH_CMAKE)

// Stuff for curl download/upload
typedef std::vector<char> cmFileCommandVectorOfChar;

namespace {

size_t cmWriteToFileCallback(void* ptr, size_t size, size_t nmemb, void* data)
{
  int realsize = static_cast<int>(size * nmemb);
  cmsys::ofstream* fout = static_cast<cmsys::ofstream*>(data);
  const char* chPtr = static_cast<char*>(ptr);
  fout->write(chPtr, realsize);
  return realsize;
}

size_t cmWriteToMemoryCallback(void* ptr, size_t size, size_t nmemb,
                               void* data)
{
  int realsize = static_cast<int>(size * nmemb);
  const char* chPtr = static_cast<char*>(ptr);
  cmAppend(*static_cast<cmFileCommandVectorOfChar*>(data), chPtr,
           chPtr + realsize);
  return realsize;
}

size_t cmFileCommandCurlDebugCallback(CURL*, curl_infotype type, char* chPtr,
                                      size_t size, void* data)
{
  cmFileCommandVectorOfChar& vec =
    *static_cast<cmFileCommandVectorOfChar*>(data);
  switch (type) {
    case CURLINFO_TEXT:
    case CURLINFO_HEADER_IN:
    case CURLINFO_HEADER_OUT:
      cmAppend(vec, chPtr, chPtr + size);
      break;
    case CURLINFO_DATA_IN:
    case CURLINFO_DATA_OUT:
    case CURLINFO_SSL_DATA_IN:
    case CURLINFO_SSL_DATA_OUT: {
      char buf[128];
      int n = sprintf(buf, "[%" KWIML_INT_PRIu64 " bytes data]\n",
                      static_cast<KWIML_INT_uint64_t>(size));
      if (n > 0) {
        cmAppend(vec, buf, buf + n);
      }
    } break;
    default:
      break;
  }
  return 0;
}

class cURLProgressHelper
{
public:
  cURLProgressHelper(cmFileCommand* fc, const char* text)
  {
    this->CurrentPercentage = -1;
    this->FileCommand = fc;
    this->Text = text;
  }

  bool UpdatePercentage(double value, double total, std::string& status)
  {
    long OldPercentage = this->CurrentPercentage;

    if (total > 0.0) {
      this->CurrentPercentage = std::lround(value / total * 100.0);
      if (this->CurrentPercentage > 100) {
        // Avoid extra progress reports for unexpected data beyond total.
        this->CurrentPercentage = 100;
      }
    }

    bool updated = (OldPercentage != this->CurrentPercentage);

    if (updated) {
      std::ostringstream oss;
      oss << "[" << this->Text << " " << this->CurrentPercentage
          << "% complete]";
      status = oss.str();
    }

    return updated;
  }

  cmFileCommand* GetFileCommand() { return this->FileCommand; }

private:
  long CurrentPercentage;
  cmFileCommand* FileCommand;
  std::string Text;
};

int cmFileDownloadProgressCallback(void* clientp, double dltotal, double dlnow,
                                   double ultotal, double ulnow)
{
  cURLProgressHelper* helper = reinterpret_cast<cURLProgressHelper*>(clientp);

  static_cast<void>(ultotal);
  static_cast<void>(ulnow);

  std::string status;
  if (helper->UpdatePercentage(dlnow, dltotal, status)) {
    cmFileCommand* fc = helper->GetFileCommand();
    cmMakefile* mf = fc->GetMakefile();
    mf->DisplayStatus(status, -1);
  }

  return 0;
}

int cmFileUploadProgressCallback(void* clientp, double dltotal, double dlnow,
                                 double ultotal, double ulnow)
{
  cURLProgressHelper* helper = reinterpret_cast<cURLProgressHelper*>(clientp);

  static_cast<void>(dltotal);
  static_cast<void>(dlnow);

  std::string status;
  if (helper->UpdatePercentage(ulnow, ultotal, status)) {
    cmFileCommand* fc = helper->GetFileCommand();
    cmMakefile* mf = fc->GetMakefile();
    mf->DisplayStatus(status, -1);
  }

  return 0;
}
}

namespace {

class cURLEasyGuard
{
public:
  cURLEasyGuard(CURL* easy)
    : Easy(easy)
  {
  }

  ~cURLEasyGuard()
  {
    if (this->Easy) {
      ::curl_easy_cleanup(this->Easy);
    }
  }

  cURLEasyGuard(const cURLEasyGuard&) = delete;
  cURLEasyGuard& operator=(const cURLEasyGuard&) = delete;

  void release() { this->Easy = nullptr; }

private:
  ::CURL* Easy;
};
}
#endif

#define check_curl_result(result, errstr)                                     \
  do {                                                                        \
    if (result != CURLE_OK) {                                                 \
      std::string e(errstr);                                                  \
      e += ::curl_easy_strerror(result);                                      \
      this->SetError(e);                                                      \
      return false;                                                           \
    }                                                                         \
  } while (false)

bool cmFileCommand::HandleDownloadCommand(std::vector<std::string> const& args)
{
#if defined(CMAKE_BUILD_WITH_CMAKE)
  std::vector<std::string>::const_iterator i = args.begin();
  if (args.size() < 3) {
    this->SetError("DOWNLOAD must be called with at least three arguments.");
    return false;
  }
  ++i; // Get rid of subcommand
  std::string url = *i;
  ++i;
  std::string file = *i;
  ++i;

  long timeout = 0;
  long inactivity_timeout = 0;
  std::string logVar;
  std::string statusVar;
  bool tls_verify = this->Makefile->IsOn("CMAKE_TLS_VERIFY");
  const char* cainfo = this->Makefile->GetDefinition("CMAKE_TLS_CAINFO");
  std::string netrc_level = this->Makefile->GetSafeDefinition("CMAKE_NETRC");
  std::string netrc_file =
    this->Makefile->GetSafeDefinition("CMAKE_NETRC_FILE");
  std::string expectedHash;
  std::string hashMatchMSG;
  std::unique_ptr<cmCryptoHash> hash;
  bool showProgress = false;
  std::string userpwd;

  std::vector<std::string> curl_headers;

  while (i != args.end()) {
    if (*i == "TIMEOUT") {
      ++i;
      if (i != args.end()) {
        timeout = atol(i->c_str());
      } else {
        this->SetError("DOWNLOAD missing time for TIMEOUT.");
        return false;
      }
    } else if (*i == "INACTIVITY_TIMEOUT") {
      ++i;
      if (i != args.end()) {
        inactivity_timeout = atol(i->c_str());
      } else {
        this->SetError("DOWNLOAD missing time for INACTIVITY_TIMEOUT.");
        return false;
      }
    } else if (*i == "LOG") {
      ++i;
      if (i == args.end()) {
        this->SetError("DOWNLOAD missing VAR for LOG.");
        return false;
      }
      logVar = *i;
    } else if (*i == "STATUS") {
      ++i;
      if (i == args.end()) {
        this->SetError("DOWNLOAD missing VAR for STATUS.");
        return false;
      }
      statusVar = *i;
    } else if (*i == "TLS_VERIFY") {
      ++i;
      if (i != args.end()) {
        tls_verify = cmSystemTools::IsOn(*i);
      } else {
        this->SetError("TLS_VERIFY missing bool value.");
        return false;
      }
    } else if (*i == "TLS_CAINFO") {
      ++i;
      if (i != args.end()) {
        cainfo = i->c_str();
      } else {
        this->SetError("TLS_CAFILE missing file value.");
        return false;
      }
    } else if (*i == "NETRC_FILE") {
      ++i;
      if (i != args.end()) {
        netrc_file = *i;
      } else {
        this->SetError("DOWNLOAD missing file value for NETRC_FILE.");
        return false;
      }
    } else if (*i == "NETRC") {
      ++i;
      if (i != args.end()) {
        netrc_level = *i;
      } else {
        this->SetError("DOWNLOAD missing level value for NETRC.");
        return false;
      }
    } else if (*i == "EXPECTED_MD5") {
      ++i;
      if (i == args.end()) {
        this->SetError("DOWNLOAD missing sum value for EXPECTED_MD5.");
        return false;
      }
      hash = cm::make_unique<cmCryptoHash>(cmCryptoHash::AlgoMD5);
      hashMatchMSG = "MD5 sum";
      expectedHash = cmSystemTools::LowerCase(*i);
    } else if (*i == "SHOW_PROGRESS") {
      showProgress = true;
    } else if (*i == "EXPECTED_HASH") {
      ++i;
      if (i == args.end()) {
        this->SetError("DOWNLOAD missing ALGO=value for EXPECTED_HASH.");
        return false;
      }
      std::string::size_type pos = i->find("=");
      if (pos == std::string::npos) {
        std::string err =
          "DOWNLOAD EXPECTED_HASH expects ALGO=value but got: ";
        err += *i;
        this->SetError(err);
        return false;
      }
      std::string algo = i->substr(0, pos);
      expectedHash = cmSystemTools::LowerCase(i->substr(pos + 1));
      hash = std::unique_ptr<cmCryptoHash>(cmCryptoHash::New(algo.c_str()));
      if (!hash) {
        std::string err = "DOWNLOAD EXPECTED_HASH given unknown ALGO: ";
        err += algo;
        this->SetError(err);
        return false;
      }
      hashMatchMSG = algo + " hash";
    } else if (*i == "USERPWD") {
      ++i;
      if (i == args.end()) {
        this->SetError("DOWNLOAD missing string for USERPWD.");
        return false;
      }
      userpwd = *i;
    } else if (*i == "HTTPHEADER") {
      ++i;
      if (i == args.end()) {
        this->SetError("DOWNLOAD missing string for HTTPHEADER.");
        return false;
      }
      curl_headers.push_back(*i);
    } else {
      // Do not return error for compatibility reason.
      std::string err = "Unexpected argument: ";
      err += *i;
      this->Makefile->IssueMessage(MessageType::AUTHOR_WARNING, err);
    }
    ++i;
  }
  // If file exists already, and caller specified an expected md5 or sha,
  // and the existing file already has the expected hash, then simply
  // return.
  //
  if (cmSystemTools::FileExists(file) && hash.get()) {
    std::string msg;
    std::string actualHash = hash->HashFile(file);
    if (actualHash == expectedHash) {
      msg = "returning early; file already exists with expected ";
      msg += hashMatchMSG;
      msg += "\"";
      if (!statusVar.empty()) {
        std::ostringstream result;
        result << 0 << ";\"" << msg;
        this->Makefile->AddDefinition(statusVar, result.str().c_str());
      }
      return true;
    }
  }
  // Make sure parent directory exists so we can write to the file
  // as we receive downloaded bits from curl...
  //
  std::string dir = cmSystemTools::GetFilenamePath(file);
  if (!cmSystemTools::FileExists(dir) && !cmSystemTools::MakeDirectory(dir)) {
    std::string errstring = "DOWNLOAD error: cannot create directory '" + dir +
      "' - Specify file by full path name and verify that you "
      "have directory creation and file write privileges.";
    this->SetError(errstring);
    return false;
  }

  cmsys::ofstream fout(file.c_str(), std::ios::binary);
  if (!fout) {
    this->SetError("DOWNLOAD cannot open file for write.");
    return false;
  }

#  if defined(_WIN32)
  url = fix_file_url_windows(url);
#  endif

  ::CURL* curl;
  ::curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = ::curl_easy_init();
  if (!curl) {
    this->SetError("DOWNLOAD error initializing curl.");
    return false;
  }

  cURLEasyGuard g_curl(curl);
  ::CURLcode res = ::curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  check_curl_result(res, "DOWNLOAD cannot set url: ");

  // enable HTTP ERROR parsing
  res = ::curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
  check_curl_result(res, "DOWNLOAD cannot set http failure option: ");

  res = ::curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/" LIBCURL_VERSION);
  check_curl_result(res, "DOWNLOAD cannot set user agent option: ");

  res = ::curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cmWriteToFileCallback);
  check_curl_result(res, "DOWNLOAD cannot set write function: ");

  res = ::curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION,
                           cmFileCommandCurlDebugCallback);
  check_curl_result(res, "DOWNLOAD cannot set debug function: ");

  // check to see if TLS verification is requested
  if (tls_verify) {
    res = ::curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
    check_curl_result(res, "Unable to set TLS/SSL Verify on: ");
  } else {
    res = ::curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    check_curl_result(res, "Unable to set TLS/SSL Verify off: ");
  }
  // check to see if a CAINFO file has been specified
  // command arg comes first
  std::string const& cainfo_err = cmCurlSetCAInfo(curl, cainfo);
  if (!cainfo_err.empty()) {
    this->SetError(cainfo_err);
    return false;
  }

  // check to see if netrc parameters have been specified
  // local command args takes precedence over CMAKE_NETRC*
  netrc_level = cmSystemTools::UpperCase(netrc_level);
  std::string const& netrc_option_err =
    cmCurlSetNETRCOption(curl, netrc_level, netrc_file);
  if (!netrc_option_err.empty()) {
    this->SetError(netrc_option_err);
    return false;
  }

  cmFileCommandVectorOfChar chunkDebug;

  res = ::curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fout);
  check_curl_result(res, "DOWNLOAD cannot set write data: ");

  res = ::curl_easy_setopt(curl, CURLOPT_DEBUGDATA, &chunkDebug);
  check_curl_result(res, "DOWNLOAD cannot set debug data: ");

  res = ::curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  check_curl_result(res, "DOWNLOAD cannot set follow-redirect option: ");

  if (!logVar.empty()) {
    res = ::curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    check_curl_result(res, "DOWNLOAD cannot set verbose: ");
  }

  if (timeout > 0) {
    res = ::curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
    check_curl_result(res, "DOWNLOAD cannot set timeout: ");
  }

  if (inactivity_timeout > 0) {
    // Give up if there is no progress for a long time.
    ::curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1);
    ::curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, inactivity_timeout);
  }

  // Need the progress helper's scope to last through the duration of
  // the curl_easy_perform call... so this object is declared at function
  // scope intentionally, rather than inside the "if(showProgress)"
  // block...
  //
  cURLProgressHelper helper(this, "download");

  if (showProgress) {
    res = ::curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
    check_curl_result(res, "DOWNLOAD cannot set noprogress value: ");

    res = ::curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION,
                             cmFileDownloadProgressCallback);
    check_curl_result(res, "DOWNLOAD cannot set progress function: ");

    res = ::curl_easy_setopt(curl, CURLOPT_PROGRESSDATA,
                             reinterpret_cast<void*>(&helper));
    check_curl_result(res, "DOWNLOAD cannot set progress data: ");
  }

  if (!userpwd.empty()) {
    res = ::curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd.c_str());
    check_curl_result(res, "DOWNLOAD cannot set user password: ");
  }

  struct curl_slist* headers = nullptr;
  for (std::string const& h : curl_headers) {
    headers = ::curl_slist_append(headers, h.c_str());
  }
  ::curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  res = ::curl_easy_perform(curl);

  ::curl_slist_free_all(headers);

  /* always cleanup */
  g_curl.release();
  ::curl_easy_cleanup(curl);

  if (!statusVar.empty()) {
    std::ostringstream result;
    result << static_cast<int>(res) << ";\"" << ::curl_easy_strerror(res)
           << "\"";
    this->Makefile->AddDefinition(statusVar, result.str().c_str());
  }

  ::curl_global_cleanup();

  // Explicitly flush/close so we can measure the md5 accurately.
  //
  fout.flush();
  fout.close();

  // Verify MD5 sum if requested:
  //
  if (hash) {
    std::string actualHash = hash->HashFile(file);
    if (actualHash.empty()) {
      this->SetError("DOWNLOAD cannot compute hash on downloaded file");
      return false;
    }

    if (expectedHash != actualHash) {
      std::ostringstream oss;
      oss << "DOWNLOAD HASH mismatch" << std::endl
          << "  for file: [" << file << "]" << std::endl
          << "    expected hash: [" << expectedHash << "]" << std::endl
          << "      actual hash: [" << actualHash << "]" << std::endl
          << "           status: [" << static_cast<int>(res) << ";\""
          << ::curl_easy_strerror(res) << "\"]" << std::endl;

      if (!statusVar.empty() && res == 0) {
        std::string status = "1;HASH mismatch: "
                             "expected: " +
          expectedHash + " actual: " + actualHash;
        this->Makefile->AddDefinition(statusVar, status.c_str());
      }

      this->SetError(oss.str());
      return false;
    }
  }

  if (!logVar.empty()) {
    chunkDebug.push_back(0);
    this->Makefile->AddDefinition(logVar, chunkDebug.data());
  }

  return true;
#else
  this->SetError("DOWNLOAD not supported by bootstrap cmake.");
  return false;
#endif
}

bool cmFileCommand::HandleUploadCommand(std::vector<std::string> const& args)
{
#if defined(CMAKE_BUILD_WITH_CMAKE)
  if (args.size() < 3) {
    this->SetError("UPLOAD must be called with at least three arguments.");
    return false;
  }
  std::vector<std::string>::const_iterator i = args.begin();
  ++i;
  std::string filename = *i;
  ++i;
  std::string url = *i;
  ++i;

  long timeout = 0;
  long inactivity_timeout = 0;
  std::string logVar;
  std::string statusVar;
  bool showProgress = false;
  std::string userpwd;
  std::string netrc_level = this->Makefile->GetSafeDefinition("CMAKE_NETRC");
  std::string netrc_file =
    this->Makefile->GetSafeDefinition("CMAKE_NETRC_FILE");

  std::vector<std::string> curl_headers;

  while (i != args.end()) {
    if (*i == "TIMEOUT") {
      ++i;
      if (i != args.end()) {
        timeout = atol(i->c_str());
      } else {
        this->SetError("UPLOAD missing time for TIMEOUT.");
        return false;
      }
    } else if (*i == "INACTIVITY_TIMEOUT") {
      ++i;
      if (i != args.end()) {
        inactivity_timeout = atol(i->c_str());
      } else {
        this->SetError("UPLOAD missing time for INACTIVITY_TIMEOUT.");
        return false;
      }
    } else if (*i == "LOG") {
      ++i;
      if (i == args.end()) {
        this->SetError("UPLOAD missing VAR for LOG.");
        return false;
      }
      logVar = *i;
    } else if (*i == "STATUS") {
      ++i;
      if (i == args.end()) {
        this->SetError("UPLOAD missing VAR for STATUS.");
        return false;
      }
      statusVar = *i;
    } else if (*i == "SHOW_PROGRESS") {
      showProgress = true;
    } else if (*i == "NETRC_FILE") {
      ++i;
      if (i != args.end()) {
        netrc_file = *i;
      } else {
        this->SetError("UPLOAD missing file value for NETRC_FILE.");
        return false;
      }
    } else if (*i == "NETRC") {
      ++i;
      if (i != args.end()) {
        netrc_level = *i;
      } else {
        this->SetError("UPLOAD missing level value for NETRC.");
        return false;
      }
    } else if (*i == "USERPWD") {
      ++i;
      if (i == args.end()) {
        this->SetError("UPLOAD missing string for USERPWD.");
        return false;
      }
      userpwd = *i;
    } else if (*i == "HTTPHEADER") {
      ++i;
      if (i == args.end()) {
        this->SetError("UPLOAD missing string for HTTPHEADER.");
        return false;
      }
      curl_headers.push_back(*i);
    } else {
      // Do not return error for compatibility reason.
      std::string err = "Unexpected argument: ";
      err += *i;
      this->Makefile->IssueMessage(MessageType::AUTHOR_WARNING, err);
    }

    ++i;
  }

  // Open file for reading:
  //
  FILE* fin = cmsys::SystemTools::Fopen(filename, "rb");
  if (!fin) {
    std::string errStr = "UPLOAD cannot open file '";
    errStr += filename + "' for reading.";
    this->SetError(errStr);
    return false;
  }

  unsigned long file_size = cmsys::SystemTools::FileLength(filename);

#  if defined(_WIN32)
  url = fix_file_url_windows(url);
#  endif

  ::CURL* curl;
  ::curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = ::curl_easy_init();
  if (!curl) {
    this->SetError("UPLOAD error initializing curl.");
    fclose(fin);
    return false;
  }

  cURLEasyGuard g_curl(curl);

  // enable HTTP ERROR parsing
  ::CURLcode res = ::curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
  check_curl_result(res, "UPLOAD cannot set fail on error flag: ");

  // enable uploading
  res = ::curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);
  check_curl_result(res, "UPLOAD cannot set upload flag: ");

  res = ::curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  check_curl_result(res, "UPLOAD cannot set url: ");

  res =
    ::curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cmWriteToMemoryCallback);
  check_curl_result(res, "UPLOAD cannot set write function: ");

  res = ::curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION,
                           cmFileCommandCurlDebugCallback);
  check_curl_result(res, "UPLOAD cannot set debug function: ");

  cmFileCommandVectorOfChar chunkResponse;
  cmFileCommandVectorOfChar chunkDebug;

  res = ::curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunkResponse);
  check_curl_result(res, "UPLOAD cannot set write data: ");

  res = ::curl_easy_setopt(curl, CURLOPT_DEBUGDATA, &chunkDebug);
  check_curl_result(res, "UPLOAD cannot set debug data: ");

  res = ::curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  check_curl_result(res, "UPLOAD cannot set follow-redirect option: ");

  if (!logVar.empty()) {
    res = ::curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    check_curl_result(res, "UPLOAD cannot set verbose: ");
  }

  if (timeout > 0) {
    res = ::curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
    check_curl_result(res, "UPLOAD cannot set timeout: ");
  }

  if (inactivity_timeout > 0) {
    // Give up if there is no progress for a long time.
    ::curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1);
    ::curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, inactivity_timeout);
  }

  // Need the progress helper's scope to last through the duration of
  // the curl_easy_perform call... so this object is declared at function
  // scope intentionally, rather than inside the "if(showProgress)"
  // block...
  //
  cURLProgressHelper helper(this, "upload");

  if (showProgress) {
    res = ::curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
    check_curl_result(res, "UPLOAD cannot set noprogress value: ");

    res = ::curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION,
                             cmFileUploadProgressCallback);
    check_curl_result(res, "UPLOAD cannot set progress function: ");

    res = ::curl_easy_setopt(curl, CURLOPT_PROGRESSDATA,
                             reinterpret_cast<void*>(&helper));
    check_curl_result(res, "UPLOAD cannot set progress data: ");
  }

  // now specify which file to upload
  res = ::curl_easy_setopt(curl, CURLOPT_INFILE, fin);
  check_curl_result(res, "UPLOAD cannot set input file: ");

  // and give the size of the upload (optional)
  res =
    ::curl_easy_setopt(curl, CURLOPT_INFILESIZE, static_cast<long>(file_size));
  check_curl_result(res, "UPLOAD cannot set input file size: ");

  if (!userpwd.empty()) {
    res = ::curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd.c_str());
    check_curl_result(res, "UPLOAD cannot set user password: ");
  }

  // check to see if netrc parameters have been specified
  // local command args takes precedence over CMAKE_NETRC*
  netrc_level = cmSystemTools::UpperCase(netrc_level);
  std::string const& netrc_option_err =
    cmCurlSetNETRCOption(curl, netrc_level, netrc_file);
  if (!netrc_option_err.empty()) {
    this->SetError(netrc_option_err);
    return false;
  }

  struct curl_slist* headers = nullptr;
  for (std::string const& h : curl_headers) {
    headers = ::curl_slist_append(headers, h.c_str());
  }
  ::curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  res = ::curl_easy_perform(curl);

  ::curl_slist_free_all(headers);

  /* always cleanup */
  g_curl.release();
  ::curl_easy_cleanup(curl);

  if (!statusVar.empty()) {
    std::ostringstream result;
    result << static_cast<int>(res) << ";\"" << ::curl_easy_strerror(res)
           << "\"";
    this->Makefile->AddDefinition(statusVar, result.str().c_str());
  }

  ::curl_global_cleanup();

  fclose(fin);
  fin = nullptr;

  if (!logVar.empty()) {
    std::string log;

    if (!chunkResponse.empty()) {
      chunkResponse.push_back(0);
      log += "Response:\n";
      log += chunkResponse.data();
      log += "\n";
    }

    if (!chunkDebug.empty()) {
      chunkDebug.push_back(0);
      log += "Debug:\n";
      log += chunkDebug.data();
      log += "\n";
    }

    this->Makefile->AddDefinition(logVar, log.c_str());
  }

  return true;
#else
  this->SetError("UPLOAD not supported by bootstrap cmake.");
  return false;
#endif
}

void cmFileCommand::AddEvaluationFile(const std::string& inputName,
                                      const std::string& outputExpr,
                                      const std::string& condition,
                                      bool inputIsContent)
{
  cmListFileBacktrace lfbt = this->Makefile->GetBacktrace();

  cmGeneratorExpression outputGe(lfbt);
  std::unique_ptr<cmCompiledGeneratorExpression> outputCge =
    outputGe.Parse(outputExpr);

  cmGeneratorExpression conditionGe(lfbt);
  std::unique_ptr<cmCompiledGeneratorExpression> conditionCge =
    conditionGe.Parse(condition);

  this->Makefile->AddEvaluationFile(inputName, std::move(outputCge),
                                    std::move(conditionCge), inputIsContent);
}

bool cmFileCommand::HandleGenerateCommand(std::vector<std::string> const& args)
{
  if (args.size() < 5) {
    this->SetError("Incorrect arguments to GENERATE subcommand.");
    return false;
  }
  if (args[1] != "OUTPUT") {
    this->SetError("Incorrect arguments to GENERATE subcommand.");
    return false;
  }
  std::string condition;
  if (args.size() > 5) {
    if (args[5] != "CONDITION") {
      this->SetError("Incorrect arguments to GENERATE subcommand.");
      return false;
    }
    if (args.size() != 7) {
      this->SetError("Incorrect arguments to GENERATE subcommand.");
      return false;
    }
    condition = args[6];
    if (condition.empty()) {
      this->SetError("CONDITION of sub-command GENERATE must not be empty if "
                     "specified.");
      return false;
    }
  }
  std::string output = args[2];
  const bool inputIsContent = args[3] != "INPUT";
  if (inputIsContent && args[3] != "CONTENT") {
    this->SetError("Incorrect arguments to GENERATE subcommand.");
    return false;
  }
  std::string input = args[4];

  this->AddEvaluationFile(input, output, condition, inputIsContent);
  return true;
}

bool cmFileCommand::HandleLockCommand(std::vector<std::string> const& args)
{
#if defined(CMAKE_BUILD_WITH_CMAKE)
  // Default values
  bool directory = false;
  bool release = false;
  enum Guard
  {
    GUARD_FUNCTION,
    GUARD_FILE,
    GUARD_PROCESS
  };
  Guard guard = GUARD_PROCESS;
  std::string resultVariable;
  unsigned long timeout = static_cast<unsigned long>(-1);

  // Parse arguments
  if (args.size() < 2) {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      "sub-command LOCK requires at least two arguments.");
    return false;
  }

  std::string path = args[1];
  for (unsigned i = 2; i < args.size(); ++i) {
    if (args[i] == "DIRECTORY") {
      directory = true;
    } else if (args[i] == "RELEASE") {
      release = true;
    } else if (args[i] == "GUARD") {
      ++i;
      const char* merr = "expected FUNCTION, FILE or PROCESS after GUARD";
      if (i >= args.size()) {
        this->Makefile->IssueMessage(MessageType::FATAL_ERROR, merr);
        return false;
      }
      if (args[i] == "FUNCTION") {
        guard = GUARD_FUNCTION;
      } else if (args[i] == "FILE") {
        guard = GUARD_FILE;
      } else if (args[i] == "PROCESS") {
        guard = GUARD_PROCESS;
      } else {
        std::ostringstream e;
        e << merr << ", but got:\n  \"" << args[i] << "\".";
        this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
        return false;
      }

    } else if (args[i] == "RESULT_VARIABLE") {
      ++i;
      if (i >= args.size()) {
        this->Makefile->IssueMessage(
          MessageType::FATAL_ERROR,
          "expected variable name after RESULT_VARIABLE");
        return false;
      }
      resultVariable = args[i];
    } else if (args[i] == "TIMEOUT") {
      ++i;
      if (i >= args.size()) {
        this->Makefile->IssueMessage(MessageType::FATAL_ERROR,
                                     "expected timeout value after TIMEOUT");
        return false;
      }
      long scanned;
      if (!cmSystemTools::StringToLong(args[i].c_str(), &scanned) ||
          scanned < 0) {
        std::ostringstream e;
        e << "TIMEOUT value \"" << args[i] << "\" is not an unsigned integer.";
        this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
        return false;
      }
      timeout = static_cast<unsigned long>(scanned);
    } else {
      std::ostringstream e;
      e << "expected DIRECTORY, RELEASE, GUARD, RESULT_VARIABLE or TIMEOUT\n";
      e << "but got: \"" << args[i] << "\".";
      this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
      return false;
    }
  }

  if (directory) {
    path += "/cmake.lock";
  }

  if (!cmsys::SystemTools::FileIsFullPath(path)) {
    path = this->Makefile->GetCurrentSourceDirectory() + "/" + path;
  }

  // Unify path (remove '//', '/../', ...)
  path = cmSystemTools::CollapseFullPath(path);

  // Create file and directories if needed
  std::string parentDir = cmSystemTools::GetParentDirectory(path);
  if (!cmSystemTools::MakeDirectory(parentDir)) {
    std::ostringstream e;
    e << "directory\n  \"" << parentDir << "\"\ncreation failed ";
    e << "(check permissions).";
    this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
    cmSystemTools::SetFatalErrorOccured();
    return false;
  }
  FILE* file = cmsys::SystemTools::Fopen(path, "w");
  if (!file) {
    std::ostringstream e;
    e << "file\n  \"" << path << "\"\ncreation failed (check permissions).";
    this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
    cmSystemTools::SetFatalErrorOccured();
    return false;
  }
  fclose(file);

  // Actual lock/unlock
  cmFileLockPool& lockPool =
    this->Makefile->GetGlobalGenerator()->GetFileLockPool();

  cmFileLockResult fileLockResult(cmFileLockResult::MakeOk());
  if (release) {
    fileLockResult = lockPool.Release(path);
  } else {
    switch (guard) {
      case GUARD_FUNCTION:
        fileLockResult = lockPool.LockFunctionScope(path, timeout);
        break;
      case GUARD_FILE:
        fileLockResult = lockPool.LockFileScope(path, timeout);
        break;
      case GUARD_PROCESS:
        fileLockResult = lockPool.LockProcessScope(path, timeout);
        break;
      default:
        cmSystemTools::SetFatalErrorOccured();
        return false;
    }
  }

  const std::string result = fileLockResult.GetOutputMessage();

  if (resultVariable.empty() && !fileLockResult.IsOk()) {
    std::ostringstream e;
    e << "error locking file\n  \"" << path << "\"\n" << result << ".";
    this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
    cmSystemTools::SetFatalErrorOccured();
    return false;
  }

  if (!resultVariable.empty()) {
    this->Makefile->AddDefinition(resultVariable, result.c_str());
  }

  return true;
#else
  static_cast<void>(args);
  this->SetError("sub-command LOCK not implemented in bootstrap cmake");
  return false;
#endif
}

bool cmFileCommand::HandleTimestampCommand(
  std::vector<std::string> const& args)
{
  if (args.size() < 3) {
    this->SetError("sub-command TIMESTAMP requires at least two arguments.");
    return false;
  }
  if (args.size() > 5) {
    this->SetError("sub-command TIMESTAMP takes at most four arguments.");
    return false;
  }

  unsigned int argsIndex = 1;

  const std::string& filename = args[argsIndex++];

  const std::string& outputVariable = args[argsIndex++];

  std::string formatString;
  if (args.size() > argsIndex && args[argsIndex] != "UTC") {
    formatString = args[argsIndex++];
  }

  bool utcFlag = false;
  if (args.size() > argsIndex) {
    if (args[argsIndex] == "UTC") {
      utcFlag = true;
    } else {
      std::string e = " TIMESTAMP sub-command does not recognize option " +
        args[argsIndex] + ".";
      this->SetError(e);
      return false;
    }
  }

  cmTimestamp timestamp;
  std::string result =
    timestamp.FileModificationTime(filename.c_str(), formatString, utcFlag);
  this->Makefile->AddDefinition(outputVariable, result.c_str());

  return true;
}

bool cmFileCommand::HandleSizeCommand(std::vector<std::string> const& args)
{
  if (args.size() != 3) {
    std::ostringstream e;
    e << args[0] << " requires a file name and output variable";
    this->SetError(e.str());
    return false;
  }

  unsigned int argsIndex = 1;

  const std::string& filename = args[argsIndex++];

  const std::string& outputVariable = args[argsIndex++];

  if (!cmSystemTools::FileExists(filename, true)) {
    std::ostringstream e;
    e << "SIZE requested of path that is not readable:\n  " << filename;
    this->SetError(e.str());
    return false;
  }

  this->Makefile->AddDefinition(
    outputVariable,
    std::to_string(cmSystemTools::FileLength(filename)).c_str());

  return true;
}

bool cmFileCommand::HandleReadSymlinkCommand(
  std::vector<std::string> const& args)
{
  if (args.size() != 3) {
    std::ostringstream e;
    e << args[0] << " requires a file name and output variable";
    this->SetError(e.str());
    return false;
  }

  const std::string& filename = args[1];
  const std::string& outputVariable = args[2];

  std::string result;
  if (!cmSystemTools::ReadSymlink(filename, result)) {
    std::ostringstream e;
    e << "READ_SYMLINK requested of path that is not a symlink:\n  "
      << filename;
    this->SetError(e.str());
    return false;
  }

  this->Makefile->AddDefinition(outputVariable, result.c_str());

  return true;
}

bool cmFileCommand::HandleCreateLinkCommand(
  std::vector<std::string> const& args)
{
  if (args.size() < 3) {
    this->SetError("CREATE_LINK must be called with at least two additional "
                   "arguments");
    return false;
  }

  std::string const& fileName = args[1];
  std::string const& newFileName = args[2];

  struct Arguments
  {
    std::string Result;
    bool CopyOnError = false;
    bool Symbolic = false;
  };

  static auto const parser =
    cmArgumentParser<Arguments>{}
      .Bind("RESULT"_s, &Arguments::Result)
      .Bind("COPY_ON_ERROR"_s, &Arguments::CopyOnError)
      .Bind("SYMBOLIC"_s, &Arguments::Symbolic);

  std::vector<std::string> unconsumedArgs;
  Arguments const arguments =
    parser.Parse(cmMakeRange(args).advance(3), &unconsumedArgs);

  if (!unconsumedArgs.empty()) {
    this->SetError("unknown argument: \"" + unconsumedArgs.front() + '\"');
    return false;
  }

  // The system error message generated in the operation.
  std::string result;

  // Check if the paths are distinct.
  if (fileName == newFileName) {
    result = "CREATE_LINK cannot use same file and newfile";
    if (!arguments.Result.empty()) {
      this->Makefile->AddDefinition(arguments.Result, result.c_str());
      return true;
    }
    this->SetError(result);
    return false;
  }

  // Hard link requires original file to exist.
  if (!arguments.Symbolic && !cmSystemTools::FileExists(fileName)) {
    result = "Cannot hard link \'" + fileName + "\' as it does not exist.";
    if (!arguments.Result.empty()) {
      this->Makefile->AddDefinition(arguments.Result, result.c_str());
      return true;
    }
    this->SetError(result);
    return false;
  }

  // Check if the new file already exists and remove it.
  if ((cmSystemTools::FileExists(newFileName) ||
       cmSystemTools::FileIsSymlink(newFileName)) &&
      !cmSystemTools::RemoveFile(newFileName)) {
    std::ostringstream e;
    e << "Failed to create link '" << newFileName
      << "' because existing path cannot be removed: "
      << cmSystemTools::GetLastSystemError() << "\n";

    if (!arguments.Result.empty()) {
      this->Makefile->AddDefinition(arguments.Result, e.str().c_str());
      return true;
    }
    this->SetError(e.str());
    return false;
  }

  // Whether the operation completed successfully.
  bool completed = false;

  // Check if the command requires a symbolic link.
  if (arguments.Symbolic) {
    completed = cmSystemTools::CreateSymlink(fileName, newFileName, &result);
  } else {
    completed = cmSystemTools::CreateLink(fileName, newFileName, &result);
  }

  // Check if copy-on-error is enabled in the arguments.
  if (!completed && arguments.CopyOnError) {
    completed = cmsys::SystemTools::CopyFileAlways(fileName, newFileName);
    if (!completed) {
      result = "Copy failed: " + cmSystemTools::GetLastSystemError();
    }
  }

  // Check if the operation was successful.
  if (completed) {
    result = "0";
  } else if (arguments.Result.empty()) {
    // The operation failed and the result is not reported in a variable.
    this->SetError(result);
    return false;
  }

  if (!arguments.Result.empty()) {
    this->Makefile->AddDefinition(arguments.Result, result.c_str());
  }

  return true;
}
