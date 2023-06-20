/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmFileCommand.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <set>
#include <sstream>
#include <utility>
#include <vector>

#include <cm/memory>
#include <cm/optional>
#include <cm/string_view>
#include <cmext/algorithm>
#include <cmext/string_view>

#include <cm3p/kwiml/int.h>

#include "cmsys/FStream.hxx"
#include "cmsys/Glob.hxx"
#include "cmsys/RegularExpression.hxx"

#include "cm_sys_stat.h"

#include "cmArgumentParser.h"
#include "cmArgumentParserTypes.h"
#include "cmCryptoHash.h"
#include "cmELF.h"
#include "cmExecutionStatus.h"
#include "cmFSPermissions.h"
#include "cmFileCopier.h"
#include "cmFileInstaller.h"
#include "cmFileLockPool.h"
#include "cmFileTimes.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorExpression.h"
#include "cmGlobalGenerator.h"
#include "cmHexFileConverter.h"
#include "cmList.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmNewLineStyle.h"
#include "cmPolicies.h"
#include "cmRange.h"
#include "cmRuntimeDependencyArchive.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"
#include "cmSubcommandTable.h"
#include "cmSystemTools.h"
#include "cmTimestamp.h"
#include "cmValue.h"
#include "cmWorkingDirectory.h"
#include "cmake.h"

#if !defined(CMAKE_BOOTSTRAP)
#  include <cm3p/curl/curl.h>

#  include "cmCurl.h"
#  include "cmFileLockResult.h"
#endif

namespace {

bool HandleWriteImpl(std::vector<std::string> const& args, bool append,
                     cmExecutionStatus& status)
{
  auto i = args.begin();

  i++; // Get rid of subcommand

  std::string fileName = *i;
  if (!cmsys::SystemTools::FileIsFullPath(*i)) {
    fileName =
      cmStrCat(status.GetMakefile().GetCurrentSourceDirectory(), '/', *i);
  }

  i++;

  if (!status.GetMakefile().CanIWriteThisFile(fileName)) {
    std::string e =
      "attempted to write a file: " + fileName + " into a source directory.";
    status.SetError(e);
    cmSystemTools::SetFatalErrorOccurred();
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
    std::string error =
      cmStrCat("failed to open for writing (",
               cmSystemTools::GetLastSystemError(), "):\n  ", fileName);
    status.SetError(error);
    return false;
  }
  std::string message = cmJoin(cmMakeRange(i, args.end()), std::string());
  file << message;
  if (!file) {
    std::string error =
      cmStrCat("write failed (", cmSystemTools::GetLastSystemError(), "):\n  ",
               fileName);
    status.SetError(error);
    return false;
  }
  file.close();
  if (mode && !writable) {
    cmSystemTools::SetPermissions(fileName, mode);
  }
  return true;
}

bool HandleWriteCommand(std::vector<std::string> const& args,
                        cmExecutionStatus& status)
{
  return HandleWriteImpl(args, false, status);
}

bool HandleAppendCommand(std::vector<std::string> const& args,
                         cmExecutionStatus& status)
{
  return HandleWriteImpl(args, true, status);
}

bool HandleReadCommand(std::vector<std::string> const& args,
                       cmExecutionStatus& status)
{
  if (args.size() < 3) {
    status.SetError("READ must be called with at least two additional "
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

  Arguments const arguments = parser.Parse(cmMakeRange(args).advance(3),
                                           /*unparsedArguments=*/nullptr);

  std::string fileName = fileNameArg;
  if (!cmsys::SystemTools::FileIsFullPath(fileName)) {
    fileName = cmStrCat(status.GetMakefile().GetCurrentSourceDirectory(), '/',
                        fileNameArg);
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
    std::string error =
      cmStrCat("failed to open for reading (",
               cmSystemTools::GetLastSystemError(), "):\n  ", fileName);
    status.SetError(error);
    return false;
  }

  // is there a limit?
  std::string::size_type sizeLimit = std::string::npos;
  if (!arguments.Limit.empty()) {
    unsigned long long limit;
    if (cmStrToULongLong(arguments.Limit, &limit)) {
      sizeLimit = static_cast<std::string::size_type>(limit);
    }
  }

  // is there an offset?
  cmsys::ifstream::off_type offset = 0;
  if (!arguments.Offset.empty()) {
    long long off;
    if (cmStrToLongLong(arguments.Offset, &off)) {
      offset = static_cast<cmsys::ifstream::off_type>(off);
    }
  }

  file.seekg(offset, std::ios::beg); // explicit ios::beg for IBM VisualAge 6

  std::string output;

  if (arguments.Hex) {
    // Convert part of the file into hex code
    char c;
    while ((sizeLimit > 0) && (file.get(c))) {
      char hex[4];
      snprintf(hex, sizeof(hex), "%.2x", c & 0xff);
      output += hex;
      sizeLimit--;
    }
  } else {
    std::string line;
    bool has_newline = false;
    while (
      sizeLimit > 0 &&
      cmSystemTools::GetLineFromStream(file, line, &has_newline, sizeLimit)) {
      sizeLimit = sizeLimit - line.size();
      if (has_newline && sizeLimit > 0) {
        sizeLimit--;
      }
      output += line;
      if (has_newline) {
        output += "\n";
      }
    }
  }
  status.GetMakefile().AddDefinition(variable, output);
  return true;
}

bool HandleHashCommand(std::vector<std::string> const& args,
                       cmExecutionStatus& status)
{
#if !defined(CMAKE_BOOTSTRAP)
  if (args.size() != 3) {
    status.SetError(
      cmStrCat(args[0], " requires a file name and output variable"));
    return false;
  }

  std::unique_ptr<cmCryptoHash> hash(cmCryptoHash::New(args[0]));
  if (hash) {
    std::string out = hash->HashFile(args[1]);
    if (!out.empty()) {
      status.GetMakefile().AddDefinition(args[2], out);
      return true;
    }
    status.SetError(cmStrCat(args[0], " failed to read file \"", args[1],
                             "\": ", cmSystemTools::GetLastSystemError()));
  }
  return false;
#else
  status.SetError(cmStrCat(args[0], " not available during bootstrap"));
  return false;
#endif
}

bool HandleStringsCommand(std::vector<std::string> const& args,
                          cmExecutionStatus& status)
{
  if (args.size() < 3) {
    status.SetError("STRINGS requires a file name and output variable");
    return false;
  }

  // Get the file to read.
  std::string fileName = args[1];
  if (!cmsys::SystemTools::FileIsFullPath(fileName)) {
    fileName =
      cmStrCat(status.GetMakefile().GetCurrentSourceDirectory(), '/', args[1]);
  }

  // Get the variable in which to store the results.
  std::string const& outVar = args[2];

  // Parse the options.
  enum
  {
    arg_none,
    arg_limit_input,
    arg_limit_output,
    arg_limit_count,
    arg_length_minimum,
    arg_length_maximum,
    arg_maximum,
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
        status.SetError(cmStrCat("STRINGS option LIMIT_INPUT value \"",
                                 args[i], "\" is not an unsigned integer."));
        return false;
      }
      arg_mode = arg_none;
    } else if (arg_mode == arg_limit_output) {
      if (sscanf(args[i].c_str(), "%d", &limit_output) != 1 ||
          limit_output < 0) {
        status.SetError(cmStrCat("STRINGS option LIMIT_OUTPUT value \"",
                                 args[i], "\" is not an unsigned integer."));
        return false;
      }
      arg_mode = arg_none;
    } else if (arg_mode == arg_limit_count) {
      int count;
      if (sscanf(args[i].c_str(), "%d", &count) != 1 || count < 0) {
        status.SetError(cmStrCat("STRINGS option LIMIT_COUNT value \"",
                                 args[i], "\" is not an unsigned integer."));
        return false;
      }
      limit_count = count;
      arg_mode = arg_none;
    } else if (arg_mode == arg_length_minimum) {
      int len;
      if (sscanf(args[i].c_str(), "%d", &len) != 1 || len < 0) {
        status.SetError(cmStrCat("STRINGS option LENGTH_MINIMUM value \"",
                                 args[i], "\" is not an unsigned integer."));
        return false;
      }
      minlen = len;
      arg_mode = arg_none;
    } else if (arg_mode == arg_length_maximum) {
      int len;
      if (sscanf(args[i].c_str(), "%d", &len) != 1 || len < 0) {
        status.SetError(cmStrCat("STRINGS option LENGTH_MAXIMUM value \"",
                                 args[i], "\" is not an unsigned integer."));
        return false;
      }
      maxlen = len;
      arg_mode = arg_none;
    } else if (arg_mode == arg_regex) {
      if (!regex.compile(args[i])) {
        status.SetError(cmStrCat("STRINGS option REGEX value \"", args[i],
                                 "\" could not be compiled."));
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
        status.SetError(cmStrCat("STRINGS option ENCODING \"", args[i],
                                 "\" not recognized."));
        return false;
      }
      arg_mode = arg_none;
    } else {
      status.SetError(
        cmStrCat("STRINGS given unknown argument \"", args[i], "\""));
      return false;
    }
  }

  if (hex_conversion_enabled) {
    // TODO: should work without temp file, but just on a memory buffer
    std::string binaryFileName =
      cmStrCat(status.GetMakefile().GetCurrentBinaryDirectory(),
               "/CMakeFiles/FileCommandStringsBinaryFile");
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
    status.SetError(
      cmStrCat("STRINGS file \"", fileName, "\" cannot be read."));
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
          fin.putback(current_str[current_str.size() - 1 - j]);
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
  status.GetMakefile().AddDefinition(outVar, output);
  return true;
}

bool HandleGlobImpl(std::vector<std::string> const& args, bool recurse,
                    cmExecutionStatus& status)
{
  // File commands has at least one argument
  assert(args.size() > 1);

  auto i = args.begin();

  i++; // Get rid of subcommand

  std::string variable = *i;
  i++;
  cmsys::Glob g;
  g.SetRecurse(recurse);

  bool explicitFollowSymlinks = false;
  cmPolicies::PolicyStatus policyStatus =
    status.GetMakefile().GetPolicyStatus(cmPolicies::CMP0009);
  if (recurse) {
    switch (policyStatus) {
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
      case cmPolicies::NEW:
        g.RecurseThroughSymlinksOff();
        break;
      case cmPolicies::WARN:
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        g.RecurseThroughSymlinksOn();
        break;
    }
  }

  cmake* cm = status.GetMakefile().GetCMakeInstance();
  std::vector<std::string> files;
  bool configureDepends = false;
  bool warnConfigureLate = false;
  bool warnFollowedSymlinks = false;
  const cmake::WorkingMode workingMode = cm->GetWorkingMode();
  while (i != args.end()) {
    if (*i == "LIST_DIRECTORIES") {
      ++i; // skip LIST_DIRECTORIES
      if (i != args.end()) {
        if (cmIsOn(*i)) {
          g.SetListDirs(true);
          g.SetRecurseListDirs(true);
        } else if (cmIsOff(*i)) {
          g.SetListDirs(false);
          g.SetRecurseListDirs(false);
        } else {
          status.SetError("LIST_DIRECTORIES missing bool value.");
          return false;
        }
        ++i;
      } else {
        status.SetError("LIST_DIRECTORIES missing bool value.");
        return false;
      }
    } else if (*i == "FOLLOW_SYMLINKS") {
      ++i; // skip FOLLOW_SYMLINKS
      if (recurse) {
        explicitFollowSymlinks = true;
        g.RecurseThroughSymlinksOn();
        if (i == args.end()) {
          status.SetError(
            "GLOB_RECURSE requires a glob expression after FOLLOW_SYMLINKS.");
          return false;
        }
      }
    } else if (*i == "RELATIVE") {
      ++i; // skip RELATIVE
      if (i == args.end()) {
        status.SetError("GLOB requires a directory after the RELATIVE tag.");
        return false;
      }
      g.SetRelative(i->c_str());
      ++i;
      if (i == args.end()) {
        status.SetError(
          "GLOB requires a glob expression after the directory.");
        return false;
      }
    } else if (*i == "CONFIGURE_DEPENDS") {
      // Generated build system depends on glob results
      if (!configureDepends && warnConfigureLate) {
        status.GetMakefile().IssueMessage(
          MessageType::AUTHOR_WARNING,
          "CONFIGURE_DEPENDS flag was given after a glob expression was "
          "already evaluated.");
      }
      if (workingMode != cmake::NORMAL_MODE) {
        status.GetMakefile().IssueMessage(
          MessageType::FATAL_ERROR,
          "CONFIGURE_DEPENDS is invalid for script and find package modes.");
        return false;
      }
      configureDepends = true;
      ++i;
      if (i == args.end()) {
        status.SetError(
          "GLOB requires a glob expression after CONFIGURE_DEPENDS.");
        return false;
      }
    } else {
      std::string expr = *i;
      if (!cmsys::SystemTools::FileIsFullPath(*i)) {
        expr = status.GetMakefile().GetCurrentSourceDirectory();
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
            status.GetMakefile().IssueMessage(
              MessageType::AUTHOR_WARNING,
              "Cyclic recursion detected while globbing for '" + *i + "':\n" +
                globMessage.content);
          } else if (globMessage.type == cmsys::Glob::error) {
            status.GetMakefile().IssueMessage(
              MessageType::FATAL_ERROR,
              "Error has occurred while globbing for '" + *i + "' - " +
                globMessage.content);
            shouldExit = true;
          } else if (cm->GetDebugOutput() || cm->GetTrace()) {
            status.GetMakefile().IssueMessage(
              MessageType::LOG,
              cmStrCat("Globbing for\n  ", *i, "\nEncountered an error:\n ",
                       globMessage.content));
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
      cm::append(files, foundFiles);

      if (configureDepends) {
        std::sort(foundFiles.begin(), foundFiles.end());
        foundFiles.erase(std::unique(foundFiles.begin(), foundFiles.end()),
                         foundFiles.end());
        cm->AddGlobCacheEntry(
          recurse, (recurse ? g.GetRecurseListDirs() : g.GetListDirs()),
          (recurse ? g.GetRecurseThroughSymlinks() : false),
          (g.GetRelative() ? g.GetRelative() : ""), expr, foundFiles, variable,
          status.GetMakefile().GetBacktrace());
      } else {
        warnConfigureLate = true;
      }
      ++i;
    }
  }

  switch (policyStatus) {
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
        status.GetMakefile().IssueMessage(
          MessageType::AUTHOR_WARNING,
          cmPolicies::GetPolicyWarning(cmPolicies::CMP0009));
      }
      break;
  }

  std::sort(files.begin(), files.end());
  files.erase(std::unique(files.begin(), files.end()), files.end());
  status.GetMakefile().AddDefinition(variable, cmList::to_string(files));
  return true;
}

bool HandleGlobCommand(std::vector<std::string> const& args,
                       cmExecutionStatus& status)
{
  return HandleGlobImpl(args, false, status);
}

bool HandleGlobRecurseCommand(std::vector<std::string> const& args,
                              cmExecutionStatus& status)
{
  return HandleGlobImpl(args, true, status);
}

bool HandleMakeDirectoryCommand(std::vector<std::string> const& args,
                                cmExecutionStatus& status)
{
  // File command has at least one argument
  assert(args.size() > 1);

  std::string expr;
  for (std::string const& arg :
       cmMakeRange(args).advance(1)) // Get rid of subcommand
  {
    const std::string* cdir = &arg;
    if (!cmsys::SystemTools::FileIsFullPath(arg)) {
      expr =
        cmStrCat(status.GetMakefile().GetCurrentSourceDirectory(), '/', arg);
      cdir = &expr;
    }
    if (!status.GetMakefile().CanIWriteThisFile(*cdir)) {
      std::string e = "attempted to create a directory: " + *cdir +
        " into a source directory.";
      status.SetError(e);
      cmSystemTools::SetFatalErrorOccurred();
      return false;
    }
    cmsys::Status mkdirStatus = cmSystemTools::MakeDirectory(*cdir);
    if (!mkdirStatus) {
      std::string error = cmStrCat("failed to create directory:\n  ", *cdir,
                                   "\nbecause: ", mkdirStatus.GetString());
      status.SetError(error);
      return false;
    }
  }
  return true;
}

bool HandleTouchImpl(std::vector<std::string> const& args, bool create,
                     cmExecutionStatus& status)
{
  // File command has at least one argument
  assert(args.size() > 1);

  for (std::string const& arg :
       cmMakeRange(args).advance(1)) // Get rid of subcommand
  {
    std::string tfile = arg;
    if (!cmsys::SystemTools::FileIsFullPath(tfile)) {
      tfile =
        cmStrCat(status.GetMakefile().GetCurrentSourceDirectory(), '/', arg);
    }
    if (!status.GetMakefile().CanIWriteThisFile(tfile)) {
      std::string e =
        "attempted to touch a file: " + tfile + " in a source directory.";
      status.SetError(e);
      cmSystemTools::SetFatalErrorOccurred();
      return false;
    }
    if (!cmSystemTools::Touch(tfile, create)) {
      std::string error = "problem touching file: " + tfile;
      status.SetError(error);
      return false;
    }
  }
  return true;
}

bool HandleTouchCommand(std::vector<std::string> const& args,
                        cmExecutionStatus& status)
{
  return HandleTouchImpl(args, true, status);
}

bool HandleTouchNocreateCommand(std::vector<std::string> const& args,
                                cmExecutionStatus& status)
{
  return HandleTouchImpl(args, false, status);
}

bool HandleDifferentCommand(std::vector<std::string> const& args,
                            cmExecutionStatus& status)
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
      status.SetError(cmStrCat("DIFFERENT given unknown argument ", args[i]));
      return false;
    }
  }
  if (!var) {
    status.SetError("DIFFERENT not given result variable name.");
    return false;
  }
  if (!file_lhs || !file_rhs) {
    status.SetError("DIFFERENT not given FILES option with two file names.");
    return false;
  }

  // Compare the files.
  const char* result =
    cmSystemTools::FilesDiffer(file_lhs, file_rhs) ? "1" : "0";
  status.GetMakefile().AddDefinition(var, result);
  return true;
}

bool HandleCopyCommand(std::vector<std::string> const& args,
                       cmExecutionStatus& status)
{
  cmFileCopier copier(status);
  return copier.Run(args);
}

bool HandleRPathChangeCommand(std::vector<std::string> const& args,
                              cmExecutionStatus& status)
{
  // Evaluate arguments.
  std::string file;
  cm::optional<std::string> oldRPath;
  cm::optional<std::string> newRPath;
  bool removeEnvironmentRPath = false;
  cmArgumentParser<void> parser;
  std::vector<std::string> unknownArgs;
  parser.Bind("FILE"_s, file)
    .Bind("OLD_RPATH"_s, oldRPath)
    .Bind("NEW_RPATH"_s, newRPath)
    .Bind("INSTALL_REMOVE_ENVIRONMENT_RPATH"_s, removeEnvironmentRPath);
  ArgumentParser::ParseResult parseResult =
    parser.Parse(cmMakeRange(args).advance(1), &unknownArgs);
  if (!unknownArgs.empty()) {
    status.SetError(
      cmStrCat("RPATH_CHANGE given unknown argument ", unknownArgs.front()));
    return false;
  }
  if (parseResult.MaybeReportError(status.GetMakefile())) {
    return true;
  }
  if (file.empty()) {
    status.SetError("RPATH_CHANGE not given FILE option.");
    return false;
  }
  if (!oldRPath) {
    status.SetError("RPATH_CHANGE not given OLD_RPATH option.");
    return false;
  }
  if (!newRPath) {
    status.SetError("RPATH_CHANGE not given NEW_RPATH option.");
    return false;
  }
  if (!cmSystemTools::FileExists(file, true)) {
    status.SetError(
      cmStrCat("RPATH_CHANGE given FILE \"", file, "\" that does not exist."));
    return false;
  }
  bool success = true;
  cmFileTimes const ft(file);
  std::string emsg;
  bool changed;

  if (!cmSystemTools::ChangeRPath(file, *oldRPath, *newRPath,
                                  removeEnvironmentRPath, &emsg, &changed)) {
    status.SetError(cmStrCat("RPATH_CHANGE could not write new RPATH:\n  ",
                             *newRPath, "\nto the file:\n  ", file, "\n",
                             emsg));
    success = false;
  }
  if (success) {
    if (changed) {
      std::string message =
        cmStrCat("Set runtime path of \"", file, "\" to \"", *newRPath, '"');
      status.GetMakefile().DisplayStatus(message, -1);
    }
    ft.Store(file);
  }
  return success;
}

bool HandleRPathSetCommand(std::vector<std::string> const& args,
                           cmExecutionStatus& status)
{
  // Evaluate arguments.
  std::string file;
  cm::optional<std::string> newRPath;
  cmArgumentParser<void> parser;
  std::vector<std::string> unknownArgs;
  parser.Bind("FILE"_s, file).Bind("NEW_RPATH"_s, newRPath);
  ArgumentParser::ParseResult parseResult =
    parser.Parse(cmMakeRange(args).advance(1), &unknownArgs);
  if (!unknownArgs.empty()) {
    status.SetError(cmStrCat("RPATH_SET given unrecognized argument \"",
                             unknownArgs.front(), "\"."));
    return false;
  }
  if (parseResult.MaybeReportError(status.GetMakefile())) {
    return true;
  }
  if (file.empty()) {
    status.SetError("RPATH_SET not given FILE option.");
    return false;
  }
  if (!newRPath) {
    status.SetError("RPATH_SET not given NEW_RPATH option.");
    return false;
  }
  if (!cmSystemTools::FileExists(file, true)) {
    status.SetError(
      cmStrCat("RPATH_SET given FILE \"", file, "\" that does not exist."));
    return false;
  }
  bool success = true;
  cmFileTimes const ft(file);
  std::string emsg;
  bool changed;

  if (!cmSystemTools::SetRPath(file, *newRPath, &emsg, &changed)) {
    status.SetError(cmStrCat("RPATH_SET could not write new RPATH:\n  ",
                             *newRPath, "\nto the file:\n  ", file, "\n",
                             emsg));
    success = false;
  }
  if (success) {
    if (changed) {
      std::string message =
        cmStrCat("Set runtime path of \"", file, "\" to \"", *newRPath, '"');
      status.GetMakefile().DisplayStatus(message, -1);
    }
    ft.Store(file);
  }
  return success;
}

bool HandleRPathRemoveCommand(std::vector<std::string> const& args,
                              cmExecutionStatus& status)
{
  // Evaluate arguments.
  std::string file;
  cmArgumentParser<void> parser;
  std::vector<std::string> unknownArgs;
  parser.Bind("FILE"_s, file);
  ArgumentParser::ParseResult parseResult =
    parser.Parse(cmMakeRange(args).advance(1), &unknownArgs);
  if (!unknownArgs.empty()) {
    status.SetError(
      cmStrCat("RPATH_REMOVE given unknown argument ", unknownArgs.front()));
    return false;
  }
  if (parseResult.MaybeReportError(status.GetMakefile())) {
    return true;
  }
  if (file.empty()) {
    status.SetError("RPATH_REMOVE not given FILE option.");
    return false;
  }
  if (!cmSystemTools::FileExists(file, true)) {
    status.SetError(
      cmStrCat("RPATH_REMOVE given FILE \"", file, "\" that does not exist."));
    return false;
  }
  bool success = true;
  cmFileTimes const ft(file);
  std::string emsg;
  bool removed;
  if (!cmSystemTools::RemoveRPath(file, &emsg, &removed)) {
    status.SetError(
      cmStrCat("RPATH_REMOVE could not remove RPATH from file: \n  ", file,
               "\n", emsg));
    success = false;
  }
  if (success) {
    if (removed) {
      std::string message =
        cmStrCat("Removed runtime path from \"", file, '"');
      status.GetMakefile().DisplayStatus(message, -1);
    }
    ft.Store(file);
  }
  return success;
}

bool HandleRPathCheckCommand(std::vector<std::string> const& args,
                             cmExecutionStatus& status)
{
  // Evaluate arguments.
  std::string file;
  cm::optional<std::string> rpath;
  cmArgumentParser<void> parser;
  std::vector<std::string> unknownArgs;
  parser.Bind("FILE"_s, file).Bind("RPATH"_s, rpath);
  ArgumentParser::ParseResult parseResult =
    parser.Parse(cmMakeRange(args).advance(1), &unknownArgs);
  if (!unknownArgs.empty()) {
    status.SetError(
      cmStrCat("RPATH_CHECK given unknown argument ", unknownArgs.front()));
    return false;
  }
  if (parseResult.MaybeReportError(status.GetMakefile())) {
    return true;
  }
  if (file.empty()) {
    status.SetError("RPATH_CHECK not given FILE option.");
    return false;
  }
  if (!rpath) {
    status.SetError("RPATH_CHECK not given RPATH option.");
    return false;
  }

  // If the file exists but does not have the desired RPath then
  // delete it.  This is used during installation to re-install a file
  // if its RPath will change.
  if (cmSystemTools::FileExists(file, true) &&
      !cmSystemTools::CheckRPath(file, *rpath)) {
    cmSystemTools::RemoveFile(file);
  }

  return true;
}

bool HandleReadElfCommand(std::vector<std::string> const& args,
                          cmExecutionStatus& status)
{
  if (args.size() < 4) {
    status.SetError("READ_ELF must be called with at least three additional "
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
  Arguments const arguments = parser.Parse(cmMakeRange(args).advance(2),
                                           /*unparsedArguments=*/nullptr);

  if (!cmSystemTools::FileExists(fileNameArg, true)) {
    status.SetError(cmStrCat("READ_ELF given FILE \"", fileNameArg,
                             "\" that does not exist."));
    return false;
  }

  cmELF elf(fileNameArg.c_str());
  if (!elf) {
    if (arguments.Error.empty()) {
      status.SetError(cmStrCat("READ_ELF given FILE:\n  ", fileNameArg,
                               "\nthat is not a valid ELF file."));
      return false;
    }
    status.GetMakefile().AddDefinition(arguments.Error,
                                       "not a valid ELF file");
    return true;
  }

  if (!arguments.RPath.empty()) {
    if (cmELF::StringEntry const* se_rpath = elf.GetRPath()) {
      std::string rpath(se_rpath->Value);
      std::replace(rpath.begin(), rpath.end(), ':', ';');
      status.GetMakefile().AddDefinition(arguments.RPath, rpath);
    }
  }
  if (!arguments.RunPath.empty()) {
    if (cmELF::StringEntry const* se_runpath = elf.GetRunPath()) {
      std::string runpath(se_runpath->Value);
      std::replace(runpath.begin(), runpath.end(), ':', ';');
      status.GetMakefile().AddDefinition(arguments.RunPath, runpath);
    }
  }

  return true;
}

bool HandleInstallCommand(std::vector<std::string> const& args,
                          cmExecutionStatus& status)
{
  cmFileInstaller installer(status);
  return installer.Run(args);
}

bool HandleRealPathCommand(std::vector<std::string> const& args,
                           cmExecutionStatus& status)
{
  if (args.size() < 3) {
    status.SetError("REAL_PATH requires a path and an output variable");
    return false;
  }

  struct Arguments : public ArgumentParser::ParseResult
  {
    cm::optional<std::string> BaseDirectory;
    bool ExpandTilde = false;
  };
  static auto const parser =
    cmArgumentParser<Arguments>{}
      .Bind("BASE_DIRECTORY"_s, &Arguments::BaseDirectory)
      .Bind("EXPAND_TILDE"_s, &Arguments::ExpandTilde);

  std::vector<std::string> unparsedArguments;
  auto arguments =
    parser.Parse(cmMakeRange(args).advance(3), &unparsedArguments);

  if (!unparsedArguments.empty()) {
    status.SetError("REAL_PATH called with unexpected arguments");
    return false;
  }
  if (arguments.MaybeReportError(status.GetMakefile())) {
    return true;
  }

  if (!arguments.BaseDirectory) {
    arguments.BaseDirectory = status.GetMakefile().GetCurrentSourceDirectory();
  }

  auto input = args[1];
  if (arguments.ExpandTilde && !input.empty()) {
    if (input[0] == '~' && (input.length() == 1 || input[1] == '/')) {
      std::string home;
      if (
#if defined(_WIN32) && !defined(__CYGWIN__)
        cmSystemTools::GetEnv("USERPROFILE", home) ||
#endif
        cmSystemTools::GetEnv("HOME", home)) {
        input.replace(0, 1, home);
      }
    }
  }

  auto realPath =
    cmSystemTools::CollapseFullPath(input, *arguments.BaseDirectory);
  realPath = cmSystemTools::GetRealPath(realPath);

  status.GetMakefile().AddDefinition(args[2], realPath);

  return true;
}

bool HandleRelativePathCommand(std::vector<std::string> const& args,
                               cmExecutionStatus& status)
{
  if (args.size() != 4) {
    status.SetError("RELATIVE_PATH called with incorrect number of arguments");
    return false;
  }

  const std::string& outVar = args[1];
  const std::string& directoryName = args[2];
  const std::string& fileName = args[3];

  if (!cmSystemTools::FileIsFullPath(directoryName)) {
    std::string errstring =
      "RELATIVE_PATH must be passed a full path to the directory: " +
      directoryName;
    status.SetError(errstring);
    return false;
  }
  if (!cmSystemTools::FileIsFullPath(fileName)) {
    std::string errstring =
      "RELATIVE_PATH must be passed a full path to the file: " + fileName;
    status.SetError(errstring);
    return false;
  }

  std::string res = cmSystemTools::RelativePath(directoryName, fileName);
  status.GetMakefile().AddDefinition(outVar, res);
  return true;
}

bool HandleRename(std::vector<std::string> const& args,
                  cmExecutionStatus& status)
{
  if (args.size() < 3) {
    status.SetError("RENAME must be called with at least two additional "
                    "arguments");
    return false;
  }

  // Compute full path for old and new names.
  std::string oldname = args[1];
  if (!cmsys::SystemTools::FileIsFullPath(oldname)) {
    oldname =
      cmStrCat(status.GetMakefile().GetCurrentSourceDirectory(), '/', args[1]);
  }
  std::string newname = args[2];
  if (!cmsys::SystemTools::FileIsFullPath(newname)) {
    newname =
      cmStrCat(status.GetMakefile().GetCurrentSourceDirectory(), '/', args[2]);
  }

  struct Arguments
  {
    bool NoReplace = false;
    std::string Result;
  };

  static auto const parser = cmArgumentParser<Arguments>{}
                               .Bind("NO_REPLACE"_s, &Arguments::NoReplace)
                               .Bind("RESULT"_s, &Arguments::Result);

  std::vector<std::string> unconsumedArgs;
  Arguments const arguments =
    parser.Parse(cmMakeRange(args).advance(3), &unconsumedArgs);
  if (!unconsumedArgs.empty()) {
    status.SetError("RENAME unknown argument:\n  " + unconsumedArgs.front());
    return false;
  }

  std::string err;
  switch (cmSystemTools::RenameFile(oldname, newname,
                                    arguments.NoReplace
                                      ? cmSystemTools::Replace::No
                                      : cmSystemTools::Replace::Yes,
                                    &err)) {
    case cmSystemTools::RenameResult::Success:
      if (!arguments.Result.empty()) {
        status.GetMakefile().AddDefinition(arguments.Result, "0");
      }
      return true;
    case cmSystemTools::RenameResult::NoReplace:
      if (!arguments.Result.empty()) {
        err = "NO_REPLACE";
      } else {
        err = "path not replaced";
      }
      CM_FALLTHROUGH;
    case cmSystemTools::RenameResult::Failure:
      if (!arguments.Result.empty()) {
        status.GetMakefile().AddDefinition(arguments.Result, err);
        return true;
      }
      break;
  }
  status.SetError(cmStrCat("RENAME failed to rename\n  ", oldname, "\nto\n  ",
                           newname, "\nbecause: ", err, "\n"));
  return false;
}

bool HandleCopyFile(std::vector<std::string> const& args,
                    cmExecutionStatus& status)
{
  if (args.size() < 3) {
    status.SetError("COPY_FILE must be called with at least two additional "
                    "arguments");
    return false;
  }

  // Compute full path for old and new names.
  std::string oldname = args[1];
  if (!cmsys::SystemTools::FileIsFullPath(oldname)) {
    oldname =
      cmStrCat(status.GetMakefile().GetCurrentSourceDirectory(), '/', args[1]);
  }
  std::string newname = args[2];
  if (!cmsys::SystemTools::FileIsFullPath(newname)) {
    newname =
      cmStrCat(status.GetMakefile().GetCurrentSourceDirectory(), '/', args[2]);
  }

  struct Arguments
  {
    bool InputMayBeRecent = false;
    bool OnlyIfDifferent = false;
    std::string Result;
  };

  static auto const parser =
    cmArgumentParser<Arguments>{}
      .Bind("INPUT_MAY_BE_RECENT"_s, &Arguments::InputMayBeRecent)
      .Bind("ONLY_IF_DIFFERENT"_s, &Arguments::OnlyIfDifferent)
      .Bind("RESULT"_s, &Arguments::Result);

  std::vector<std::string> unconsumedArgs;
  Arguments const arguments =
    parser.Parse(cmMakeRange(args).advance(3), &unconsumedArgs);
  if (!unconsumedArgs.empty()) {
    status.SetError("COPY_FILE unknown argument:\n  " +
                    unconsumedArgs.front());
    return false;
  }

  bool result = true;
  if (cmsys::SystemTools::FileIsDirectory(oldname)) {
    if (!arguments.Result.empty()) {
      status.GetMakefile().AddDefinition(arguments.Result,
                                         "cannot copy a directory");
    } else {
      status.SetError(
        cmStrCat("COPY_FILE cannot copy a directory\n  ", oldname));
      result = false;
    }
    return result;
  }
  if (cmsys::SystemTools::FileIsDirectory(newname)) {
    if (!arguments.Result.empty()) {
      status.GetMakefile().AddDefinition(arguments.Result,
                                         "cannot copy to a directory");
    } else {
      status.SetError(
        cmStrCat("COPY_FILE cannot copy to a directory\n  ", newname));
      result = false;
    }
    return result;
  }

  cmSystemTools::CopyWhen when;
  if (arguments.OnlyIfDifferent) {
    when = cmSystemTools::CopyWhen::OnlyIfDifferent;
  } else {
    when = cmSystemTools::CopyWhen::Always;
  }
  cmSystemTools::CopyInputRecent const inputRecent = arguments.InputMayBeRecent
    ? cmSystemTools::CopyInputRecent::Yes
    : cmSystemTools::CopyInputRecent::No;

  std::string err;
  if (cmSystemTools::CopySingleFile(oldname, newname, when, inputRecent,
                                    &err) ==
      cmSystemTools::CopyResult::Success) {
    if (!arguments.Result.empty()) {
      status.GetMakefile().AddDefinition(arguments.Result, "0");
    }
  } else {
    if (!arguments.Result.empty()) {
      status.GetMakefile().AddDefinition(arguments.Result, err);
    } else {
      status.SetError(cmStrCat("COPY_FILE failed to copy\n  ", oldname,
                               "\nto\n  ", newname, "\nbecause: ", err, "\n"));
      result = false;
    }
  }

  return result;
}

bool HandleRemoveImpl(std::vector<std::string> const& args, bool recurse,
                      cmExecutionStatus& status)
{
  for (std::string const& arg :
       cmMakeRange(args).advance(1)) // Get rid of subcommand
  {
    std::string fileName = arg;
    if (fileName.empty()) {
      std::string const r = recurse ? "REMOVE_RECURSE" : "REMOVE";
      status.GetMakefile().IssueMessage(
        MessageType::AUTHOR_WARNING, "Ignoring empty file name in " + r + ".");
      continue;
    }
    if (!cmsys::SystemTools::FileIsFullPath(fileName)) {
      fileName =
        cmStrCat(status.GetMakefile().GetCurrentSourceDirectory(), '/', arg);
    }

    if (cmSystemTools::FileIsDirectory(fileName) &&
        !cmSystemTools::FileIsSymlink(fileName) && recurse) {
      cmSystemTools::RepeatedRemoveDirectory(fileName);
    } else {
      cmSystemTools::RemoveFile(fileName);
    }
  }
  return true;
}

bool HandleRemove(std::vector<std::string> const& args,
                  cmExecutionStatus& status)
{
  return HandleRemoveImpl(args, false, status);
}

bool HandleRemoveRecurse(std::vector<std::string> const& args,
                         cmExecutionStatus& status)
{
  return HandleRemoveImpl(args, true, status);
}

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

bool HandlePathCommand(std::vector<std::string> const& args,
                       std::string (*convert)(std::string const&),
                       cmExecutionStatus& status)
{
  if (args.size() != 3) {
    status.SetError("FILE([TO_CMAKE_PATH|TO_NATIVE_PATH] path result) must be "
                    "called with exactly three arguments.");
    return false;
  }
#if defined(_WIN32) && !defined(__CYGWIN__)
  char pathSep = ';';
#else
  char pathSep = ':';
#endif
  std::vector<std::string> path = cmSystemTools::SplitString(args[1], pathSep);

  std::string value = cmList::to_string(cmMakeRange(path).transform(convert));
  status.GetMakefile().AddDefinition(args[2], value);
  return true;
}

bool HandleCMakePathCommand(std::vector<std::string> const& args,
                            cmExecutionStatus& status)
{
  return HandlePathCommand(args, ToCMakePath, status);
}

bool HandleNativePathCommand(std::vector<std::string> const& args,
                             cmExecutionStatus& status)
{
  return HandlePathCommand(args, ToNativePath, status);
}

#if !defined(CMAKE_BOOTSTRAP)

// Stuff for curl download/upload
using cmFileCommandVectorOfChar = std::vector<char>;

size_t cmWriteToFileCallback(void* ptr, size_t size, size_t nmemb, void* data)
{
  int realsize = static_cast<int>(size * nmemb);
  cmsys::ofstream* fout = static_cast<cmsys::ofstream*>(data);
  if (fout) {
    const char* chPtr = static_cast<char*>(ptr);
    fout->write(chPtr, realsize);
  }
  return realsize;
}

size_t cmWriteToMemoryCallback(void* ptr, size_t size, size_t nmemb,
                               void* data)
{
  int realsize = static_cast<int>(size * nmemb);
  const char* chPtr = static_cast<char*>(ptr);
  cm::append(*static_cast<cmFileCommandVectorOfChar*>(data), chPtr,
             chPtr + realsize);
  return realsize;
}

int cmFileCommandCurlDebugCallback(CURL*, curl_infotype type, char* chPtr,
                                   size_t size, void* data)
{
  cmFileCommandVectorOfChar& vec =
    *static_cast<cmFileCommandVectorOfChar*>(data);
  switch (type) {
    case CURLINFO_TEXT:
    case CURLINFO_HEADER_IN:
    case CURLINFO_HEADER_OUT:
      cm::append(vec, chPtr, chPtr + size);
      break;
    case CURLINFO_DATA_IN:
    case CURLINFO_DATA_OUT:
    case CURLINFO_SSL_DATA_IN:
    case CURLINFO_SSL_DATA_OUT: {
      char buf[128];
      int n =
        snprintf(buf, sizeof(buf), "[%" KWIML_INT_PRIu64 " bytes data]\n",
                 static_cast<KWIML_INT_uint64_t>(size));
      if (n > 0) {
        cm::append(vec, buf, buf + n);
      }
    } break;
    default:
      break;
  }
  return 0;
}

#  if defined(LIBCURL_VERSION_NUM) && LIBCURL_VERSION_NUM >= 0x072000
const CURLoption CM_CURLOPT_XFERINFOFUNCTION = CURLOPT_XFERINFOFUNCTION;
using cm_curl_off_t = curl_off_t;
#  else
const CURLoption CM_CURLOPT_XFERINFOFUNCTION = CURLOPT_PROGRESSFUNCTION;
using cm_curl_off_t = double;
#  endif

class cURLProgressHelper
{
public:
  cURLProgressHelper(cmMakefile* mf, const char* text)
    : Makefile(mf)
    , Text(text)
  {
  }

  bool UpdatePercentage(cm_curl_off_t value, cm_curl_off_t total,
                        std::string& status)
  {
    long OldPercentage = this->CurrentPercentage;

    if (total > 0) {
      this->CurrentPercentage = std::lround(
        static_cast<double>(value) / static_cast<double>(total) * 100.0);
      if (this->CurrentPercentage > 100) {
        // Avoid extra progress reports for unexpected data beyond total.
        this->CurrentPercentage = 100;
      }
    }

    bool updated = (OldPercentage != this->CurrentPercentage);

    if (updated) {
      status =
        cmStrCat("[", this->Text, " ", this->CurrentPercentage, "% complete]");
    }

    return updated;
  }

  cmMakefile* GetMakefile() { return this->Makefile; }

private:
  long CurrentPercentage = -1;
  cmMakefile* Makefile;
  std::string Text;
};

int cmFileDownloadProgressCallback(void* clientp, cm_curl_off_t dltotal,
                                   cm_curl_off_t dlnow, cm_curl_off_t ultotal,
                                   cm_curl_off_t ulnow)
{
  cURLProgressHelper* helper = reinterpret_cast<cURLProgressHelper*>(clientp);

  static_cast<void>(ultotal);
  static_cast<void>(ulnow);

  std::string status;
  if (helper->UpdatePercentage(dlnow, dltotal, status)) {
    cmMakefile* mf = helper->GetMakefile();
    mf->DisplayStatus(status, -1);
  }

  return 0;
}

int cmFileUploadProgressCallback(void* clientp, cm_curl_off_t dltotal,
                                 cm_curl_off_t dlnow, cm_curl_off_t ultotal,
                                 cm_curl_off_t ulnow)
{
  cURLProgressHelper* helper = reinterpret_cast<cURLProgressHelper*>(clientp);

  static_cast<void>(dltotal);
  static_cast<void>(dlnow);

  std::string status;
  if (helper->UpdatePercentage(ulnow, ultotal, status)) {
    cmMakefile* mf = helper->GetMakefile();
    mf->DisplayStatus(status, -1);
  }

  return 0;
}

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

#endif

#define check_curl_result(result, errstr)                                     \
  do {                                                                        \
    if (result != CURLE_OK) {                                                 \
      std::string e(errstr);                                                  \
      e += ::curl_easy_strerror(result);                                      \
      status.SetError(e);                                                     \
      return false;                                                           \
    }                                                                         \
  } while (false)

bool HandleDownloadCommand(std::vector<std::string> const& args,
                           cmExecutionStatus& status)
{
#if !defined(CMAKE_BOOTSTRAP)
  auto i = args.begin();
  if (args.size() < 2) {
    status.SetError("DOWNLOAD must be called with at least two arguments.");
    return false;
  }
  ++i; // Get rid of subcommand
  std::string url = *i;
  ++i;
  std::string file;

  long timeout = 0;
  long inactivity_timeout = 0;
  std::string logVar;
  std::string statusVar;
  bool tls_verify = status.GetMakefile().IsOn("CMAKE_TLS_VERIFY");
  cmValue cainfo = status.GetMakefile().GetDefinition("CMAKE_TLS_CAINFO");
  std::string netrc_level =
    status.GetMakefile().GetSafeDefinition("CMAKE_NETRC");
  std::string netrc_file =
    status.GetMakefile().GetSafeDefinition("CMAKE_NETRC_FILE");
  std::string expectedHash;
  std::string hashMatchMSG;
  std::unique_ptr<cmCryptoHash> hash;
  bool showProgress = false;
  std::string userpwd;

  std::vector<std::string> curl_headers;
  std::vector<std::pair<std::string, cm::optional<std::string>>> curl_ranges;

  while (i != args.end()) {
    if (*i == "TIMEOUT") {
      ++i;
      if (i != args.end()) {
        timeout = atol(i->c_str());
      } else {
        status.SetError("DOWNLOAD missing time for TIMEOUT.");
        return false;
      }
    } else if (*i == "INACTIVITY_TIMEOUT") {
      ++i;
      if (i != args.end()) {
        inactivity_timeout = atol(i->c_str());
      } else {
        status.SetError("DOWNLOAD missing time for INACTIVITY_TIMEOUT.");
        return false;
      }
    } else if (*i == "LOG") {
      ++i;
      if (i == args.end()) {
        status.SetError("DOWNLOAD missing VAR for LOG.");
        return false;
      }
      logVar = *i;
    } else if (*i == "STATUS") {
      ++i;
      if (i == args.end()) {
        status.SetError("DOWNLOAD missing VAR for STATUS.");
        return false;
      }
      statusVar = *i;
    } else if (*i == "TLS_VERIFY") {
      ++i;
      if (i != args.end()) {
        tls_verify = cmIsOn(*i);
      } else {
        status.SetError("DOWNLOAD missing bool value for TLS_VERIFY.");
        return false;
      }
    } else if (*i == "TLS_CAINFO") {
      ++i;
      if (i != args.end()) {
        cainfo = cmValue(*i);
      } else {
        status.SetError("DOWNLOAD missing file value for TLS_CAINFO.");
        return false;
      }
    } else if (*i == "NETRC_FILE") {
      ++i;
      if (i != args.end()) {
        netrc_file = *i;
      } else {
        status.SetError("DOWNLOAD missing file value for NETRC_FILE.");
        return false;
      }
    } else if (*i == "NETRC") {
      ++i;
      if (i != args.end()) {
        netrc_level = *i;
      } else {
        status.SetError("DOWNLOAD missing level value for NETRC.");
        return false;
      }
    } else if (*i == "EXPECTED_MD5") {
      ++i;
      if (i == args.end()) {
        status.SetError("DOWNLOAD missing sum value for EXPECTED_MD5.");
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
        status.SetError("DOWNLOAD missing ALGO=value for EXPECTED_HASH.");
        return false;
      }
      std::string::size_type pos = i->find("=");
      if (pos == std::string::npos) {
        std::string err =
          cmStrCat("DOWNLOAD EXPECTED_HASH expects ALGO=value but got: ", *i);
        status.SetError(err);
        return false;
      }
      std::string algo = i->substr(0, pos);
      expectedHash = cmSystemTools::LowerCase(i->substr(pos + 1));
      hash = cmCryptoHash::New(algo);
      if (!hash) {
        std::string err =
          cmStrCat("DOWNLOAD EXPECTED_HASH given unknown ALGO: ", algo);
        status.SetError(err);
        return false;
      }
      hashMatchMSG = algo + " hash";
    } else if (*i == "USERPWD") {
      ++i;
      if (i == args.end()) {
        status.SetError("DOWNLOAD missing string for USERPWD.");
        return false;
      }
      userpwd = *i;
    } else if (*i == "HTTPHEADER") {
      ++i;
      if (i == args.end()) {
        status.SetError("DOWNLOAD missing string for HTTPHEADER.");
        return false;
      }
      curl_headers.push_back(*i);
    } else if (*i == "RANGE_START") {
      ++i;
      if (i == args.end()) {
        status.SetError("DOWNLOAD missing value for RANGE_START.");
        return false;
      }
      curl_ranges.emplace_back(*i, cm::nullopt);
    } else if (*i == "RANGE_END") {
      ++i;
      if (curl_ranges.empty()) {
        curl_ranges.emplace_back("0", *i);
      } else {
        auto& last_range = curl_ranges.back();
        if (!last_range.second.has_value()) {
          last_range.second = *i;
        } else {
          status.SetError("Multiple RANGE_END values is provided without "
                          "the corresponding RANGE_START.");
          return false;
        }
      }
    } else if (file.empty()) {
      file = *i;
    } else {
      // Do not return error for compatibility reason.
      std::string err = cmStrCat("Unexpected argument: ", *i);
      status.GetMakefile().IssueMessage(MessageType::AUTHOR_WARNING, err);
    }
    ++i;
  }

  // Can't calculate hash if we don't save the file.
  // TODO Incrementally calculate hash in the write callback as the file is
  // being downloaded so this check can be relaxed.
  if (file.empty() && hash) {
    status.SetError("DOWNLOAD cannot calculate hash if file is not saved.");
    return false;
  }
  // If file exists already, and caller specified an expected md5 or sha,
  // and the existing file already has the expected hash, then simply
  // return.
  //
  if (!file.empty() && cmSystemTools::FileExists(file) && hash.get()) {
    std::string msg;
    std::string actualHash = hash->HashFile(file);
    if (actualHash == expectedHash) {
      msg = cmStrCat("skipping download as file already exists with expected ",
                     hashMatchMSG, '"');
      if (!statusVar.empty()) {
        status.GetMakefile().AddDefinition(statusVar, cmStrCat(0, ";\"", msg));
      }
      return true;
    }
  }
  // Make sure parent directory exists so we can write to the file
  // as we receive downloaded bits from curl...
  //
  if (!file.empty()) {
    std::string dir = cmSystemTools::GetFilenamePath(file);
    if (!dir.empty() && !cmSystemTools::FileExists(dir) &&
        !cmSystemTools::MakeDirectory(dir)) {
      std::string errstring = "DOWNLOAD error: cannot create directory '" +
        dir +
        "' - Specify file by full path name and verify that you "
        "have directory creation and file write privileges.";
      status.SetError(errstring);
      return false;
    }
  }

  cmsys::ofstream fout;
  if (!file.empty()) {
    fout.open(file.c_str(), std::ios::binary);
    if (!fout) {
      status.SetError("DOWNLOAD cannot open file for write.");
      return false;
    }
  }

  url = cmCurlFixFileURL(url);

  ::CURL* curl;
  ::curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = ::curl_easy_init();
  if (!curl) {
    status.SetError("DOWNLOAD error initializing curl.");
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
    check_curl_result(res, "DOWNLOAD cannot set TLS/SSL Verify on: ");
  } else {
    res = ::curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    check_curl_result(res, "DOWNLOAD cannot set TLS/SSL Verify off: ");
  }

  for (const auto& range : curl_ranges) {
    std::string curl_range = range.first + '-' +
      (range.second.has_value() ? range.second.value() : "");
    res = ::curl_easy_setopt(curl, CURLOPT_RANGE, curl_range.c_str());
    check_curl_result(res, "DOWNLOAD cannot set range: ");
  }

  // check to see if a CAINFO file has been specified
  // command arg comes first
  std::string const& cainfo_err = cmCurlSetCAInfo(curl, cainfo);
  if (!cainfo_err.empty()) {
    status.SetError(cainfo_err);
    return false;
  }

  // check to see if netrc parameters have been specified
  // local command args takes precedence over CMAKE_NETRC*
  netrc_level = cmSystemTools::UpperCase(netrc_level);
  std::string const& netrc_option_err =
    cmCurlSetNETRCOption(curl, netrc_level, netrc_file);
  if (!netrc_option_err.empty()) {
    status.SetError(netrc_option_err);
    return false;
  }

  cmFileCommandVectorOfChar chunkDebug;

  res = ::curl_easy_setopt(curl, CURLOPT_WRITEDATA,
                           file.empty() ? nullptr : &fout);
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
  cURLProgressHelper helper(&status.GetMakefile(), "download");

  if (showProgress) {
    res = ::curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
    check_curl_result(res, "DOWNLOAD cannot set noprogress value: ");

    res = ::curl_easy_setopt(curl, CM_CURLOPT_XFERINFOFUNCTION,
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
    status.GetMakefile().AddDefinition(
      statusVar,
      cmStrCat(static_cast<int>(res), ";\"", ::curl_easy_strerror(res), "\""));
  }

  ::curl_global_cleanup();

  // Ensure requested curl logs are returned (especially in case of failure)
  //
  if (!logVar.empty()) {
    chunkDebug.push_back(0);
    status.GetMakefile().AddDefinition(logVar, chunkDebug.data());
  }

  // Explicitly flush/close so we can measure the md5 accurately.
  //
  if (!file.empty()) {
    fout.flush();
    fout.close();
  }

  // Verify MD5 sum if requested:
  //
  if (hash) {
    if (res != CURLE_OK) {
      status.SetError(cmStrCat(
        "DOWNLOAD cannot compute hash on failed download\n"
        "  status: [",
        static_cast<int>(res), ";\"", ::curl_easy_strerror(res), "\"]"));
      return false;
    }

    std::string actualHash = hash->HashFile(file);
    if (actualHash.empty()) {
      status.SetError("DOWNLOAD cannot compute hash on downloaded file");
      return false;
    }

    if (expectedHash != actualHash) {
      if (!statusVar.empty() && res == 0) {
        status.GetMakefile().AddDefinition(statusVar,
                                           "1;HASH mismatch: "
                                           "expected: " +
                                             expectedHash +
                                             " actual: " + actualHash);
      }

      status.SetError(cmStrCat("DOWNLOAD HASH mismatch\n"
                               "  for file: [",
                               file,
                               "]\n"
                               "    expected hash: [",
                               expectedHash,
                               "]\n"
                               "      actual hash: [",
                               actualHash, "]\n"));
      return false;
    }
  }

  return true;
#else
  status.SetError("DOWNLOAD not supported by bootstrap cmake.");
  return false;
#endif
}

bool HandleUploadCommand(std::vector<std::string> const& args,
                         cmExecutionStatus& status)
{
#if !defined(CMAKE_BOOTSTRAP)
  if (args.size() < 3) {
    status.SetError("UPLOAD must be called with at least three arguments.");
    return false;
  }
  auto i = args.begin();
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
  bool tls_verify = status.GetMakefile().IsOn("CMAKE_TLS_VERIFY");
  cmValue cainfo = status.GetMakefile().GetDefinition("CMAKE_TLS_CAINFO");
  std::string userpwd;
  std::string netrc_level =
    status.GetMakefile().GetSafeDefinition("CMAKE_NETRC");
  std::string netrc_file =
    status.GetMakefile().GetSafeDefinition("CMAKE_NETRC_FILE");

  std::vector<std::string> curl_headers;

  while (i != args.end()) {
    if (*i == "TIMEOUT") {
      ++i;
      if (i != args.end()) {
        timeout = atol(i->c_str());
      } else {
        status.SetError("UPLOAD missing time for TIMEOUT.");
        return false;
      }
    } else if (*i == "INACTIVITY_TIMEOUT") {
      ++i;
      if (i != args.end()) {
        inactivity_timeout = atol(i->c_str());
      } else {
        status.SetError("UPLOAD missing time for INACTIVITY_TIMEOUT.");
        return false;
      }
    } else if (*i == "LOG") {
      ++i;
      if (i == args.end()) {
        status.SetError("UPLOAD missing VAR for LOG.");
        return false;
      }
      logVar = *i;
    } else if (*i == "STATUS") {
      ++i;
      if (i == args.end()) {
        status.SetError("UPLOAD missing VAR for STATUS.");
        return false;
      }
      statusVar = *i;
    } else if (*i == "SHOW_PROGRESS") {
      showProgress = true;
    } else if (*i == "TLS_VERIFY") {
      ++i;
      if (i != args.end()) {
        tls_verify = cmIsOn(*i);
      } else {
        status.SetError("UPLOAD missing bool value for TLS_VERIFY.");
        return false;
      }
    } else if (*i == "TLS_CAINFO") {
      ++i;
      if (i != args.end()) {
        cainfo = cmValue(*i);
      } else {
        status.SetError("UPLOAD missing file value for TLS_CAINFO.");
        return false;
      }
    } else if (*i == "NETRC_FILE") {
      ++i;
      if (i != args.end()) {
        netrc_file = *i;
      } else {
        status.SetError("UPLOAD missing file value for NETRC_FILE.");
        return false;
      }
    } else if (*i == "NETRC") {
      ++i;
      if (i != args.end()) {
        netrc_level = *i;
      } else {
        status.SetError("UPLOAD missing level value for NETRC.");
        return false;
      }
    } else if (*i == "USERPWD") {
      ++i;
      if (i == args.end()) {
        status.SetError("UPLOAD missing string for USERPWD.");
        return false;
      }
      userpwd = *i;
    } else if (*i == "HTTPHEADER") {
      ++i;
      if (i == args.end()) {
        status.SetError("UPLOAD missing string for HTTPHEADER.");
        return false;
      }
      curl_headers.push_back(*i);
    } else {
      // Do not return error for compatibility reason.
      std::string err = cmStrCat("Unexpected argument: ", *i);
      status.GetMakefile().IssueMessage(MessageType::AUTHOR_WARNING, err);
    }

    ++i;
  }

  // Open file for reading:
  //
  FILE* fin = cmsys::SystemTools::Fopen(filename, "rb");
  if (!fin) {
    std::string errStr =
      cmStrCat("UPLOAD cannot open file '", filename, "' for reading.");
    status.SetError(errStr);
    return false;
  }

  unsigned long file_size = cmsys::SystemTools::FileLength(filename);

  url = cmCurlFixFileURL(url);

  ::CURL* curl;
  ::curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = ::curl_easy_init();
  if (!curl) {
    status.SetError("UPLOAD error initializing curl.");
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

  // check to see if TLS verification is requested
  if (tls_verify) {
    res = ::curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
    check_curl_result(res, "UPLOAD cannot set TLS/SSL Verify on: ");
  } else {
    res = ::curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    check_curl_result(res, "UPLOAD cannot set TLS/SSL Verify off: ");
  }

  // check to see if a CAINFO file has been specified
  // command arg comes first
  std::string const& cainfo_err = cmCurlSetCAInfo(curl, cainfo);
  if (!cainfo_err.empty()) {
    status.SetError(cainfo_err);
    return false;
  }

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
  cURLProgressHelper helper(&status.GetMakefile(), "upload");

  if (showProgress) {
    res = ::curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
    check_curl_result(res, "UPLOAD cannot set noprogress value: ");

    res = ::curl_easy_setopt(curl, CM_CURLOPT_XFERINFOFUNCTION,
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
    status.SetError(netrc_option_err);
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
    status.GetMakefile().AddDefinition(
      statusVar,
      cmStrCat(static_cast<int>(res), ";\"", ::curl_easy_strerror(res), "\""));
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

    status.GetMakefile().AddDefinition(logVar, log);
  }

  return true;
#else
  status.SetError("UPLOAD not supported by bootstrap cmake.");
  return false;
#endif
}

void AddEvaluationFile(const std::string& inputName,
                       const std::string& targetName,
                       const std::string& outputExpr,
                       const std::string& condition, bool inputIsContent,
                       const std::string& newLineCharacter, mode_t permissions,
                       cmExecutionStatus& status)
{
  cmListFileBacktrace lfbt = status.GetMakefile().GetBacktrace();

  cmGeneratorExpression outputGe(*status.GetMakefile().GetCMakeInstance(),
                                 lfbt);
  std::unique_ptr<cmCompiledGeneratorExpression> outputCge =
    outputGe.Parse(outputExpr);

  cmGeneratorExpression conditionGe(*status.GetMakefile().GetCMakeInstance(),
                                    lfbt);
  std::unique_ptr<cmCompiledGeneratorExpression> conditionCge =
    conditionGe.Parse(condition);

  status.GetMakefile().AddEvaluationFile(
    inputName, targetName, std::move(outputCge), std::move(conditionCge),
    newLineCharacter, permissions, inputIsContent);
}

bool HandleGenerateCommand(std::vector<std::string> const& args,
                           cmExecutionStatus& status)
{
  if (args.size() < 5) {
    status.SetError("Incorrect arguments to GENERATE subcommand.");
    return false;
  }

  struct Arguments : public ArgumentParser::ParseResult
  {
    cm::optional<std::string> Output;
    cm::optional<std::string> Input;
    cm::optional<std::string> Content;
    cm::optional<std::string> Condition;
    cm::optional<std::string> Target;
    cm::optional<std::string> NewLineStyle;
    bool NoSourcePermissions = false;
    bool UseSourcePermissions = false;
    ArgumentParser::NonEmpty<std::vector<std::string>> FilePermissions;
    std::vector<cm::string_view> ParsedKeywords;
  };

  static auto const parser =
    cmArgumentParser<Arguments>{}
      .Bind("OUTPUT"_s, &Arguments::Output)
      .Bind("INPUT"_s, &Arguments::Input)
      .Bind("CONTENT"_s, &Arguments::Content)
      .Bind("CONDITION"_s, &Arguments::Condition)
      .Bind("TARGET"_s, &Arguments::Target)
      .Bind("NO_SOURCE_PERMISSIONS"_s, &Arguments::NoSourcePermissions)
      .Bind("USE_SOURCE_PERMISSIONS"_s, &Arguments::UseSourcePermissions)
      .Bind("FILE_PERMISSIONS"_s, &Arguments::FilePermissions)
      .Bind("NEWLINE_STYLE"_s, &Arguments::NewLineStyle)
      .BindParsedKeywords(&Arguments::ParsedKeywords);

  std::vector<std::string> unparsedArguments;
  Arguments const arguments =
    parser.Parse(cmMakeRange(args).advance(1), &unparsedArguments);

  if (arguments.MaybeReportError(status.GetMakefile())) {
    return true;
  }

  if (!unparsedArguments.empty()) {
    status.SetError("Unknown argument to GENERATE subcommand.");
    return false;
  }

  if (!arguments.Output || arguments.ParsedKeywords[0] != "OUTPUT"_s) {
    status.SetError("GENERATE requires OUTPUT as first option.");
    return false;
  }
  std::string const& output = *arguments.Output;

  if (!arguments.Input && !arguments.Content) {
    status.SetError("GENERATE requires INPUT or CONTENT option.");
    return false;
  }
  const bool inputIsContent = arguments.ParsedKeywords[1] == "CONTENT"_s;
  if (!inputIsContent && arguments.ParsedKeywords[1] == "INPUT") {
    status.SetError("Unknown argument to GENERATE subcommand.");
  }
  std::string const& input =
    inputIsContent ? *arguments.Content : *arguments.Input;

  if (arguments.Condition && arguments.Condition->empty()) {
    status.SetError("CONDITION of sub-command GENERATE must not be empty "
                    "if specified.");
    return false;
  }
  std::string const& condition =
    arguments.Condition ? *arguments.Condition : std::string();

  if (arguments.Target && arguments.Target->empty()) {
    status.SetError("TARGET of sub-command GENERATE must not be empty "
                    "if specified.");
    return false;
  }
  std::string const& target =
    arguments.Target ? *arguments.Target : std::string();

  cmNewLineStyle newLineStyle;
  if (arguments.NewLineStyle) {
    std::string errorMessage;
    if (!newLineStyle.ReadFromArguments(args, errorMessage)) {
      status.SetError(cmStrCat("GENERATE ", errorMessage));
      return false;
    }
  }

  if (arguments.NoSourcePermissions && arguments.UseSourcePermissions) {
    status.SetError("given both NO_SOURCE_PERMISSIONS and "
                    "USE_SOURCE_PERMISSIONS. Only one option allowed.");
    return false;
  }

  if (!arguments.FilePermissions.empty()) {
    if (arguments.NoSourcePermissions) {
      status.SetError("given both NO_SOURCE_PERMISSIONS and "
                      "FILE_PERMISSIONS. Only one option allowed.");
      return false;
    }
    if (arguments.UseSourcePermissions) {
      status.SetError("given both USE_SOURCE_PERMISSIONS and "
                      "FILE_PERMISSIONS. Only one option allowed.");
      return false;
    }
  }

  if (arguments.UseSourcePermissions) {
    if (inputIsContent) {
      status.SetError("given USE_SOURCE_PERMISSIONS without a file INPUT.");
      return false;
    }
  }

  mode_t permissions = 0;
  if (arguments.NoSourcePermissions) {
    permissions |= cmFSPermissions::mode_owner_read;
    permissions |= cmFSPermissions::mode_owner_write;
    permissions |= cmFSPermissions::mode_group_read;
    permissions |= cmFSPermissions::mode_world_read;
  }

  if (!arguments.FilePermissions.empty()) {
    std::vector<std::string> invalidOptions;
    for (auto const& e : arguments.FilePermissions) {
      if (!cmFSPermissions::stringToModeT(e, permissions)) {
        invalidOptions.push_back(e);
      }
    }
    if (!invalidOptions.empty()) {
      std::ostringstream oss;
      oss << "given invalid permission ";
      for (auto i = 0u; i < invalidOptions.size(); i++) {
        if (i == 0u) {
          oss << "\"" << invalidOptions[i] << "\"";
        } else {
          oss << ",\"" << invalidOptions[i] << "\"";
        }
      }
      oss << ".";
      status.SetError(oss.str());
      return false;
    }
  }

  AddEvaluationFile(input, target, output, condition, inputIsContent,
                    newLineStyle.GetCharacters(), permissions, status);
  return true;
}

bool HandleLockCommand(std::vector<std::string> const& args,
                       cmExecutionStatus& status)
{
#if !defined(CMAKE_BOOTSTRAP)
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
    status.GetMakefile().IssueMessage(
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
        status.GetMakefile().IssueMessage(MessageType::FATAL_ERROR, merr);
        return false;
      }
      if (args[i] == "FUNCTION") {
        guard = GUARD_FUNCTION;
      } else if (args[i] == "FILE") {
        guard = GUARD_FILE;
      } else if (args[i] == "PROCESS") {
        guard = GUARD_PROCESS;
      } else {
        status.GetMakefile().IssueMessage(
          MessageType::FATAL_ERROR,
          cmStrCat(merr, ", but got:\n  \"", args[i], "\"."));
        return false;
      }

    } else if (args[i] == "RESULT_VARIABLE") {
      ++i;
      if (i >= args.size()) {
        status.GetMakefile().IssueMessage(
          MessageType::FATAL_ERROR,
          "expected variable name after RESULT_VARIABLE");
        return false;
      }
      resultVariable = args[i];
    } else if (args[i] == "TIMEOUT") {
      ++i;
      if (i >= args.size()) {
        status.GetMakefile().IssueMessage(
          MessageType::FATAL_ERROR, "expected timeout value after TIMEOUT");
        return false;
      }
      long scanned;
      if (!cmStrToLong(args[i], &scanned) || scanned < 0) {
        status.GetMakefile().IssueMessage(
          MessageType::FATAL_ERROR,
          cmStrCat("TIMEOUT value \"", args[i],
                   "\" is not an unsigned integer."));
        return false;
      }
      timeout = static_cast<unsigned long>(scanned);
    } else {
      status.GetMakefile().IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat("expected DIRECTORY, RELEASE, GUARD, RESULT_VARIABLE or ",
                 "TIMEOUT\nbut got: \"", args[i], "\"."));
      return false;
    }
  }

  if (directory) {
    path += "/cmake.lock";
  }

  // Unify path (remove '//', '/../', ...)
  path = cmSystemTools::CollapseFullPath(
    path, status.GetMakefile().GetCurrentSourceDirectory());

  // Create file and directories if needed
  std::string parentDir = cmSystemTools::GetParentDirectory(path);
  if (!cmSystemTools::MakeDirectory(parentDir)) {
    status.GetMakefile().IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("directory\n  \"", parentDir,
               "\"\ncreation failed (check permissions)."));
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }
  FILE* file = cmsys::SystemTools::Fopen(path, "w");
  if (!file) {
    status.GetMakefile().IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("file\n  \"", path,
               "\"\ncreation failed (check permissions)."));
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }
  fclose(file);

  // Actual lock/unlock
  cmFileLockPool& lockPool =
    status.GetMakefile().GetGlobalGenerator()->GetFileLockPool();

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
        cmSystemTools::SetFatalErrorOccurred();
        return false;
    }
  }

  const std::string result = fileLockResult.GetOutputMessage();

  if (resultVariable.empty() && !fileLockResult.IsOk()) {
    status.GetMakefile().IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("error locking file\n  \"", path, "\"\n", result, "."));
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }

  if (!resultVariable.empty()) {
    status.GetMakefile().AddDefinition(resultVariable, result);
  }

  return true;
#else
  static_cast<void>(args);
  status.SetError("sub-command LOCK not implemented in bootstrap cmake");
  return false;
#endif
}

bool HandleTimestampCommand(std::vector<std::string> const& args,
                            cmExecutionStatus& status)
{
  if (args.size() < 3) {
    status.SetError("sub-command TIMESTAMP requires at least two arguments.");
    return false;
  }
  if (args.size() > 5) {
    status.SetError("sub-command TIMESTAMP takes at most four arguments.");
    return false;
  }

  unsigned int argsIndex = 1;

  std::string filename = args[argsIndex++];
  if (!cmsys::SystemTools::FileIsFullPath(filename)) {
    filename = cmStrCat(status.GetMakefile().GetCurrentSourceDirectory(), '/',
                        filename);
  }

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
      status.SetError(e);
      return false;
    }
  }

  cmTimestamp timestamp;
  std::string result =
    timestamp.FileModificationTime(filename.c_str(), formatString, utcFlag);
  status.GetMakefile().AddDefinition(outputVariable, result);

  return true;
}

bool HandleSizeCommand(std::vector<std::string> const& args,
                       cmExecutionStatus& status)
{
  if (args.size() != 3) {
    status.SetError(
      cmStrCat(args[0], " requires a file name and output variable"));
    return false;
  }

  unsigned int argsIndex = 1;

  const std::string& filename = args[argsIndex++];

  const std::string& outputVariable = args[argsIndex++];

  if (!cmSystemTools::FileExists(filename, true)) {
    status.SetError(
      cmStrCat("SIZE requested of path that is not readable:\n  ", filename));
    return false;
  }

  status.GetMakefile().AddDefinition(
    outputVariable, std::to_string(cmSystemTools::FileLength(filename)));

  return true;
}

bool HandleReadSymlinkCommand(std::vector<std::string> const& args,
                              cmExecutionStatus& status)
{
  if (args.size() != 3) {
    status.SetError(
      cmStrCat(args[0], " requires a file name and output variable"));
    return false;
  }

  const std::string& filename = args[1];
  const std::string& outputVariable = args[2];

  std::string result;
  if (!cmSystemTools::ReadSymlink(filename, result)) {
    status.SetError(cmStrCat(
      "READ_SYMLINK requested of path that is not a symlink:\n  ", filename));
    return false;
  }

  status.GetMakefile().AddDefinition(outputVariable, result);

  return true;
}

bool HandleCreateLinkCommand(std::vector<std::string> const& args,
                             cmExecutionStatus& status)
{
  if (args.size() < 3) {
    status.SetError("CREATE_LINK must be called with at least two additional "
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
    status.SetError("unknown argument: \"" + unconsumedArgs.front() + '\"');
    return false;
  }

  // The system error message generated in the operation.
  std::string result;

  // Check if the paths are distinct.
  if (fileName == newFileName) {
    result = "CREATE_LINK cannot use same file and newfile";
    if (!arguments.Result.empty()) {
      status.GetMakefile().AddDefinition(arguments.Result, result);
      return true;
    }
    status.SetError(result);
    return false;
  }

  // Hard link requires original file to exist.
  if (!arguments.Symbolic && !cmSystemTools::FileExists(fileName)) {
    result = "Cannot hard link \'" + fileName + "\' as it does not exist.";
    if (!arguments.Result.empty()) {
      status.GetMakefile().AddDefinition(arguments.Result, result);
      return true;
    }
    status.SetError(result);
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
      status.GetMakefile().AddDefinition(arguments.Result, e.str());
      return true;
    }
    status.SetError(e.str());
    return false;
  }

  // Whether the operation completed successfully.
  bool completed = false;

  // Check if the command requires a symbolic link.
  if (arguments.Symbolic) {
    cmsys::Status linked =
      cmSystemTools::CreateSymlinkQuietly(fileName, newFileName);
    if (linked) {
      completed = true;
    } else {
      result = cmStrCat("failed to create symbolic link '", newFileName,
                        "': ", linked.GetString());
    }
  } else {
    cmsys::Status linked =
      cmSystemTools::CreateLinkQuietly(fileName, newFileName);
    if (linked) {
      completed = true;
    } else {
      result = cmStrCat("failed to create link '", newFileName,
                        "': ", linked.GetString());
    }
  }

  // Check if copy-on-error is enabled in the arguments.
  if (!completed && arguments.CopyOnError) {
    cmsys::Status copied =
      cmsys::SystemTools::CopyFileAlways(fileName, newFileName);
    if (copied) {
      completed = true;
    } else {
      result = "Copy failed: " + copied.GetString();
    }
  }

  // Check if the operation was successful.
  if (completed) {
    result = "0";
  } else if (arguments.Result.empty()) {
    // The operation failed and the result is not reported in a variable.
    status.SetError(result);
    return false;
  }

  if (!arguments.Result.empty()) {
    status.GetMakefile().AddDefinition(arguments.Result, result);
  }

  return true;
}

bool HandleGetRuntimeDependenciesCommand(std::vector<std::string> const& args,
                                         cmExecutionStatus& status)
{
  std::string platform =
    status.GetMakefile().GetSafeDefinition("CMAKE_HOST_SYSTEM_NAME");
  if (!cmRuntimeDependencyArchive::PlatformSupportsRuntimeDependencies(
        platform)) {
    status.SetError(
      cmStrCat("GET_RUNTIME_DEPENDENCIES is not supported on system \"",
               platform, "\""));
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }

  if (status.GetMakefile().GetState()->GetMode() == cmState::Project) {
    status.GetMakefile().IssueMessage(
      MessageType::AUTHOR_WARNING,
      "You have used file(GET_RUNTIME_DEPENDENCIES)"
      " in project mode. This is probably not what "
      "you intended to do. Instead, please consider"
      " using it in an install(CODE) or "
      "install(SCRIPT) command. For example:"
      "\n  install(CODE [["
      "\n    file(GET_RUNTIME_DEPENDENCIES"
      "\n      # ..."
      "\n      )"
      "\n    ]])");
  }

  struct Arguments : public ArgumentParser::ParseResult
  {
    std::string ResolvedDependenciesVar;
    std::string UnresolvedDependenciesVar;
    std::string ConflictingDependenciesPrefix;
    std::string RPathPrefix;
    std::string BundleExecutable;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> Executables;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> Libraries;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> Directories;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> Modules;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> PreIncludeRegexes;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> PreExcludeRegexes;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> PostIncludeRegexes;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> PostExcludeRegexes;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> PostIncludeFiles;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> PostExcludeFiles;
    ArgumentParser::MaybeEmpty<std::vector<std::string>>
      PostExcludeFilesStrict;
  };

  static auto const parser =
    cmArgumentParser<Arguments>{}
      .Bind("RESOLVED_DEPENDENCIES_VAR"_s, &Arguments::ResolvedDependenciesVar)
      .Bind("UNRESOLVED_DEPENDENCIES_VAR"_s,
            &Arguments::UnresolvedDependenciesVar)
      .Bind("CONFLICTING_DEPENDENCIES_PREFIX"_s,
            &Arguments::ConflictingDependenciesPrefix)
      .Bind("RPATH_PREFIX"_s, &Arguments::RPathPrefix)
      .Bind("BUNDLE_EXECUTABLE"_s, &Arguments::BundleExecutable)
      .Bind("EXECUTABLES"_s, &Arguments::Executables)
      .Bind("LIBRARIES"_s, &Arguments::Libraries)
      .Bind("MODULES"_s, &Arguments::Modules)
      .Bind("DIRECTORIES"_s, &Arguments::Directories)
      .Bind("PRE_INCLUDE_REGEXES"_s, &Arguments::PreIncludeRegexes)
      .Bind("PRE_EXCLUDE_REGEXES"_s, &Arguments::PreExcludeRegexes)
      .Bind("POST_INCLUDE_REGEXES"_s, &Arguments::PostIncludeRegexes)
      .Bind("POST_EXCLUDE_REGEXES"_s, &Arguments::PostExcludeRegexes)
      .Bind("POST_INCLUDE_FILES"_s, &Arguments::PostIncludeFiles)
      .Bind("POST_EXCLUDE_FILES"_s, &Arguments::PostExcludeFiles)
      .Bind("POST_EXCLUDE_FILES_STRICT"_s, &Arguments::PostExcludeFilesStrict);

  std::vector<std::string> unrecognizedArguments;
  auto parsedArgs =
    parser.Parse(cmMakeRange(args).advance(1), &unrecognizedArguments);
  auto argIt = unrecognizedArguments.begin();
  if (argIt != unrecognizedArguments.end()) {
    status.SetError(cmStrCat("Unrecognized argument: \"", *argIt, "\""));
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }

  if (parsedArgs.MaybeReportError(status.GetMakefile())) {
    cmSystemTools::SetFatalErrorOccurred();
    return true;
  }

  cmRuntimeDependencyArchive archive(
    status, parsedArgs.Directories, parsedArgs.BundleExecutable,
    parsedArgs.PreIncludeRegexes, parsedArgs.PreExcludeRegexes,
    parsedArgs.PostIncludeRegexes, parsedArgs.PostExcludeRegexes,
    std::move(parsedArgs.PostIncludeFiles),
    std::move(parsedArgs.PostExcludeFiles),
    std::move(parsedArgs.PostExcludeFilesStrict));
  if (!archive.Prepare()) {
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }

  if (!archive.GetRuntimeDependencies(
        parsedArgs.Executables, parsedArgs.Libraries, parsedArgs.Modules)) {
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }

  std::vector<std::string> deps;
  std::vector<std::string> unresolvedDeps;
  std::vector<std::string> conflictingDeps;
  for (auto const& val : archive.GetResolvedPaths()) {
    bool unique = true;
    auto it = val.second.begin();
    assert(it != val.second.end());
    auto const& firstPath = *it;
    while (++it != val.second.end()) {
      if (!cmSystemTools::SameFile(firstPath, *it)) {
        unique = false;
        break;
      }
    }

    if (unique) {
      deps.push_back(firstPath);
      if (!parsedArgs.RPathPrefix.empty()) {
        status.GetMakefile().AddDefinition(
          parsedArgs.RPathPrefix + "_" + firstPath,
          cmList::to_string(archive.GetRPaths().at(firstPath)));
      }
    } else if (!parsedArgs.ConflictingDependenciesPrefix.empty()) {
      conflictingDeps.push_back(val.first);
      std::vector<std::string> paths;
      paths.insert(paths.begin(), val.second.begin(), val.second.end());
      std::string varName =
        parsedArgs.ConflictingDependenciesPrefix + "_" + val.first;
      std::string pathsStr = cmList::to_string(paths);
      status.GetMakefile().AddDefinition(varName, pathsStr);
    } else {
      std::ostringstream e;
      e << "Multiple conflicting paths found for " << val.first << ":";
      for (auto const& path : val.second) {
        e << "\n  " << path;
      }
      status.SetError(e.str());
      cmSystemTools::SetFatalErrorOccurred();
      return false;
    }
  }
  if (!archive.GetUnresolvedPaths().empty()) {
    if (!parsedArgs.UnresolvedDependenciesVar.empty()) {
      unresolvedDeps.insert(unresolvedDeps.begin(),
                            archive.GetUnresolvedPaths().begin(),
                            archive.GetUnresolvedPaths().end());
    } else {
      std::ostringstream e;
      e << "Could not resolve runtime dependencies:";
      for (auto const& path : archive.GetUnresolvedPaths()) {
        e << "\n  " << path;
      }
      status.SetError(e.str());
      cmSystemTools::SetFatalErrorOccurred();
      return false;
    }
  }

  if (!parsedArgs.ResolvedDependenciesVar.empty()) {
    std::string val = cmList::to_string(deps);
    status.GetMakefile().AddDefinition(parsedArgs.ResolvedDependenciesVar,
                                       val);
  }
  if (!parsedArgs.UnresolvedDependenciesVar.empty()) {
    std::string val = cmList::to_string(unresolvedDeps);
    status.GetMakefile().AddDefinition(parsedArgs.UnresolvedDependenciesVar,
                                       val);
  }
  if (!parsedArgs.ConflictingDependenciesPrefix.empty()) {
    std::string val = cmList::to_string(conflictingDeps);
    status.GetMakefile().AddDefinition(
      parsedArgs.ConflictingDependenciesPrefix + "_FILENAMES", val);
  }
  return true;
}

bool HandleConfigureCommand(std::vector<std::string> const& args,
                            cmExecutionStatus& status)
{
  struct Arguments : public ArgumentParser::ParseResult
  {
    cm::optional<std::string> Output;
    cm::optional<std::string> Content;
    bool EscapeQuotes = false;
    bool AtOnly = false;
    // "NEWLINE_STYLE" requires one value, but we use a custom check below.
    ArgumentParser::Maybe<std::string> NewlineStyle;
  };

  static auto const parser =
    cmArgumentParser<Arguments>{}
      .Bind("OUTPUT"_s, &Arguments::Output)
      .Bind("CONTENT"_s, &Arguments::Content)
      .Bind("ESCAPE_QUOTES"_s, &Arguments::EscapeQuotes)
      .Bind("@ONLY"_s, &Arguments::AtOnly)
      .Bind("NEWLINE_STYLE"_s, &Arguments::NewlineStyle);

  std::vector<std::string> unrecognizedArguments;
  auto parsedArgs =
    parser.Parse(cmMakeRange(args).advance(1), &unrecognizedArguments);

  auto argIt = unrecognizedArguments.begin();
  if (argIt != unrecognizedArguments.end()) {
    status.SetError(
      cmStrCat("CONFIGURE Unrecognized argument: \"", *argIt, "\""));
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }

  if (parsedArgs.MaybeReportError(status.GetMakefile())) {
    cmSystemTools::SetFatalErrorOccurred();
    return true;
  }

  if (!parsedArgs.Output) {
    status.SetError("CONFIGURE OUTPUT option is mandatory.");
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }
  if (!parsedArgs.Content) {
    status.SetError("CONFIGURE CONTENT option is mandatory.");
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }

  std::string errorMessage;
  cmNewLineStyle newLineStyle;
  if (!newLineStyle.ReadFromArguments(args, errorMessage)) {
    status.SetError(cmStrCat("CONFIGURE ", errorMessage));
    return false;
  }

  // Check for generator expressions
  std::string outputFile = cmSystemTools::CollapseFullPath(
    *parsedArgs.Output, status.GetMakefile().GetCurrentBinaryDirectory());

  std::string::size_type pos = outputFile.find_first_of("<>");
  if (pos != std::string::npos) {
    status.SetError(cmStrCat("CONFIGURE called with OUTPUT containing a \"",
                             outputFile[pos],
                             "\".  This character is not allowed."));
    return false;
  }

  cmMakefile& makeFile = status.GetMakefile();
  if (!makeFile.CanIWriteThisFile(outputFile)) {
    cmSystemTools::Error("Attempt to write file: " + outputFile +
                         " into a source directory.");
    return false;
  }

  cmSystemTools::ConvertToUnixSlashes(outputFile);

  // Re-generate if non-temporary outputs are missing.
  // when we finalize the configuration we will remove all
  // output files that now don't exist.
  makeFile.AddCMakeOutputFile(outputFile);

  // Create output directory
  const std::string::size_type slashPos = outputFile.rfind('/');
  if (slashPos != std::string::npos) {
    const std::string path = outputFile.substr(0, slashPos);
    cmSystemTools::MakeDirectory(path);
  }

  std::string newLineCharacters = "\n";
  bool open_with_binary_flag = false;
  if (newLineStyle.IsValid()) {
    newLineCharacters = newLineStyle.GetCharacters();
    open_with_binary_flag = true;
  }

  cmGeneratedFileStream fout;
  fout.Open(outputFile, false, open_with_binary_flag);
  if (!fout) {
    cmSystemTools::Error("Could not open file for write in copy operation " +
                         outputFile);
    cmSystemTools::ReportLastSystemError("");
    return false;
  }
  fout.SetCopyIfDifferent(true);

  // copy input to output and expand variables from input at the same time
  std::stringstream sin(*parsedArgs.Content, std::ios::in);
  std::string inLine;
  std::string outLine;
  bool hasNewLine = false;
  while (cmSystemTools::GetLineFromStream(sin, inLine, &hasNewLine)) {
    outLine.clear();
    makeFile.ConfigureString(inLine, outLine, parsedArgs.AtOnly,
                             parsedArgs.EscapeQuotes);
    fout << outLine;
    if (hasNewLine || newLineStyle.IsValid()) {
      fout << newLineCharacters;
    }
  }

  // close file before attempting to copy
  fout.close();

  return true;
}

bool HandleArchiveCreateCommand(std::vector<std::string> const& args,
                                cmExecutionStatus& status)
{
  struct Arguments : public ArgumentParser::ParseResult
  {
    std::string Output;
    std::string Format;
    std::string Compression;
    std::string CompressionLevel;
    // "MTIME" should require one value, but it has long been accidentally
    // accepted without one and treated as if an empty value were given.
    // Fixing this would require a policy.
    ArgumentParser::Maybe<std::string> MTime;
    bool Verbose = false;
    // "PATHS" requires at least one value, but use a custom check below.
    ArgumentParser::MaybeEmpty<std::vector<std::string>> Paths;
  };

  static auto const parser =
    cmArgumentParser<Arguments>{}
      .Bind("OUTPUT"_s, &Arguments::Output)
      .Bind("FORMAT"_s, &Arguments::Format)
      .Bind("COMPRESSION"_s, &Arguments::Compression)
      .Bind("COMPRESSION_LEVEL"_s, &Arguments::CompressionLevel)
      .Bind("MTIME"_s, &Arguments::MTime)
      .Bind("VERBOSE"_s, &Arguments::Verbose)
      .Bind("PATHS"_s, &Arguments::Paths);

  std::vector<std::string> unrecognizedArguments;
  auto parsedArgs =
    parser.Parse(cmMakeRange(args).advance(1), &unrecognizedArguments);
  auto argIt = unrecognizedArguments.begin();
  if (argIt != unrecognizedArguments.end()) {
    status.SetError(cmStrCat("Unrecognized argument: \"", *argIt, "\""));
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }

  if (parsedArgs.MaybeReportError(status.GetMakefile())) {
    cmSystemTools::SetFatalErrorOccurred();
    return true;
  }

  const char* knownFormats[] = {
    "7zip", "gnutar", "pax", "paxr", "raw", "zip"
  };

  if (!parsedArgs.Format.empty() &&
      !cm::contains(knownFormats, parsedArgs.Format)) {
    status.SetError(
      cmStrCat("archive format ", parsedArgs.Format, " not supported"));
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }

  const char* zipFileFormats[] = { "7zip", "zip" };
  if (!parsedArgs.Compression.empty() &&
      cm::contains(zipFileFormats, parsedArgs.Format)) {
    status.SetError(cmStrCat("archive format ", parsedArgs.Format,
                             " does not support COMPRESSION arguments"));
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }

  static std::map<std::string, cmSystemTools::cmTarCompression>
    compressionTypeMap = { { "None", cmSystemTools::TarCompressNone },
                           { "BZip2", cmSystemTools::TarCompressBZip2 },
                           { "GZip", cmSystemTools::TarCompressGZip },
                           { "XZ", cmSystemTools::TarCompressXZ },
                           { "Zstd", cmSystemTools::TarCompressZstd } };

  cmSystemTools::cmTarCompression compress = cmSystemTools::TarCompressNone;
  auto typeIt = compressionTypeMap.find(parsedArgs.Compression);
  if (typeIt != compressionTypeMap.end()) {
    compress = typeIt->second;
  } else if (!parsedArgs.Compression.empty()) {
    status.SetError(cmStrCat("compression type ", parsedArgs.Compression,
                             " is not supported"));
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }

  int compressionLevel = 0;
  int minCompressionLevel = 0;
  int maxCompressionLevel = 9;
  if (compress == cmSystemTools::TarCompressZstd) {
    maxCompressionLevel = 19;
  }

  if (!parsedArgs.CompressionLevel.empty()) {
    if (parsedArgs.CompressionLevel.size() != 1 &&
        !std::isdigit(parsedArgs.CompressionLevel[0])) {
      status.SetError(
        cmStrCat("compression level ", parsedArgs.CompressionLevel, " for ",
                 parsedArgs.Compression, " should be in range ",
                 minCompressionLevel, " to ", maxCompressionLevel));
      cmSystemTools::SetFatalErrorOccurred();
      return false;
    }
    compressionLevel = std::stoi(parsedArgs.CompressionLevel);
    if (compressionLevel < minCompressionLevel ||
        compressionLevel > maxCompressionLevel) {
      status.SetError(
        cmStrCat("compression level ", parsedArgs.CompressionLevel, " for ",
                 parsedArgs.Compression, " should be in range ",
                 minCompressionLevel, " to ", maxCompressionLevel));
      cmSystemTools::SetFatalErrorOccurred();
      return false;
    }
    if (compress == cmSystemTools::TarCompressNone) {
      status.SetError(cmStrCat("compression level is not supported for "
                               "compression \"None\"",
                               parsedArgs.Compression));
      cmSystemTools::SetFatalErrorOccurred();
      return false;
    }
  }

  if (parsedArgs.Paths.empty()) {
    status.SetError("ARCHIVE_CREATE requires a non-empty list of PATHS");
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }

  if (!cmSystemTools::CreateTar(parsedArgs.Output, parsedArgs.Paths, compress,
                                parsedArgs.Verbose, parsedArgs.MTime,
                                parsedArgs.Format, compressionLevel)) {
    status.SetError(cmStrCat("failed to compress: ", parsedArgs.Output));
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }

  return true;
}

bool HandleArchiveExtractCommand(std::vector<std::string> const& args,
                                 cmExecutionStatus& status)
{
  struct Arguments : public ArgumentParser::ParseResult
  {
    std::string Input;
    bool Verbose = false;
    bool ListOnly = false;
    std::string Destination;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> Patterns;
    bool Touch = false;
  };

  static auto const parser = cmArgumentParser<Arguments>{}
                               .Bind("INPUT"_s, &Arguments::Input)
                               .Bind("VERBOSE"_s, &Arguments::Verbose)
                               .Bind("LIST_ONLY"_s, &Arguments::ListOnly)
                               .Bind("DESTINATION"_s, &Arguments::Destination)
                               .Bind("PATTERNS"_s, &Arguments::Patterns)
                               .Bind("TOUCH"_s, &Arguments::Touch);

  std::vector<std::string> unrecognizedArguments;
  auto parsedArgs =
    parser.Parse(cmMakeRange(args).advance(1), &unrecognizedArguments);
  auto argIt = unrecognizedArguments.begin();
  if (argIt != unrecognizedArguments.end()) {
    status.SetError(cmStrCat("Unrecognized argument: \"", *argIt, "\""));
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }

  if (parsedArgs.MaybeReportError(status.GetMakefile())) {
    cmSystemTools::SetFatalErrorOccurred();
    return true;
  }

  std::string inFile = parsedArgs.Input;

  if (parsedArgs.ListOnly) {
    if (!cmSystemTools::ListTar(inFile, parsedArgs.Patterns,
                                parsedArgs.Verbose)) {
      status.SetError(cmStrCat("failed to list: ", inFile));
      cmSystemTools::SetFatalErrorOccurred();
      return false;
    }
  } else {
    std::string destDir = status.GetMakefile().GetCurrentBinaryDirectory();
    if (!parsedArgs.Destination.empty()) {
      if (cmSystemTools::FileIsFullPath(parsedArgs.Destination)) {
        destDir = parsedArgs.Destination;
      } else {
        destDir = cmStrCat(destDir, "/", parsedArgs.Destination);
      }

      if (!cmSystemTools::MakeDirectory(destDir)) {
        status.SetError(cmStrCat("failed to create directory: ", destDir));
        cmSystemTools::SetFatalErrorOccurred();
        return false;
      }

      if (!cmSystemTools::FileIsFullPath(inFile)) {
        inFile =
          cmStrCat(cmSystemTools::GetCurrentWorkingDirectory(), "/", inFile);
      }
    }

    cmWorkingDirectory workdir(destDir);
    if (workdir.Failed()) {
      status.SetError(
        cmStrCat("failed to change working directory to: ", destDir));
      cmSystemTools::SetFatalErrorOccurred();
      return false;
    }

    if (!cmSystemTools::ExtractTar(
          inFile, parsedArgs.Patterns,
          parsedArgs.Touch ? cmSystemTools::cmTarExtractTimestamps::No
                           : cmSystemTools::cmTarExtractTimestamps::Yes,
          parsedArgs.Verbose)) {
      status.SetError(cmStrCat("failed to extract: ", inFile));
      cmSystemTools::SetFatalErrorOccurred();
      return false;
    }
  }

  return true;
}

bool ValidateAndConvertPermissions(
  cm::optional<ArgumentParser::NonEmpty<std::vector<std::string>>> const&
    permissions,
  mode_t& perms, cmExecutionStatus& status)
{
  if (!permissions) {
    return true;
  }
  for (const auto& i : *permissions) {
    if (!cmFSPermissions::stringToModeT(i, perms)) {
      status.SetError(i + " is an invalid permission specifier");
      cmSystemTools::SetFatalErrorOccurred();
      return false;
    }
  }
  return true;
}

bool SetPermissions(const std::string& filename, const mode_t& perms,
                    cmExecutionStatus& status)
{
  if (!cmSystemTools::SetPermissions(filename, perms)) {
    status.SetError("Failed to set permissions for " + filename);
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }
  return true;
}

bool HandleChmodCommandImpl(std::vector<std::string> const& args, bool recurse,
                            cmExecutionStatus& status)
{
  mode_t perms = 0;
  mode_t fperms = 0;
  mode_t dperms = 0;
  cmsys::Glob globber;

  globber.SetRecurse(recurse);
  globber.SetRecurseListDirs(recurse);

  struct Arguments : public ArgumentParser::ParseResult
  {
    cm::optional<ArgumentParser::NonEmpty<std::vector<std::string>>>
      Permissions;
    cm::optional<ArgumentParser::NonEmpty<std::vector<std::string>>>
      FilePermissions;
    cm::optional<ArgumentParser::NonEmpty<std::vector<std::string>>>
      DirectoryPermissions;
  };

  static auto const parser =
    cmArgumentParser<Arguments>{}
      .Bind("PERMISSIONS"_s, &Arguments::Permissions)
      .Bind("FILE_PERMISSIONS"_s, &Arguments::FilePermissions)
      .Bind("DIRECTORY_PERMISSIONS"_s, &Arguments::DirectoryPermissions);

  std::vector<std::string> pathEntries;
  Arguments parsedArgs =
    parser.Parse(cmMakeRange(args).advance(1), &pathEntries);

  // check validity of arguments
  if (!parsedArgs.Permissions && !parsedArgs.FilePermissions &&
      !parsedArgs.DirectoryPermissions) // no permissions given
  {
    status.SetError("No permissions given");
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }

  if (parsedArgs.Permissions && parsedArgs.FilePermissions &&
      parsedArgs.DirectoryPermissions) // all keywords are used
  {
    status.SetError("Remove either PERMISSIONS or FILE_PERMISSIONS or "
                    "DIRECTORY_PERMISSIONS from the invocation");
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }

  if (parsedArgs.MaybeReportError(status.GetMakefile())) {
    cmSystemTools::SetFatalErrorOccurred();
    return true;
  }

  // validate permissions
  bool validatePermissions =
    ValidateAndConvertPermissions(parsedArgs.Permissions, perms, status) &&
    ValidateAndConvertPermissions(parsedArgs.FilePermissions, fperms,
                                  status) &&
    ValidateAndConvertPermissions(parsedArgs.DirectoryPermissions, dperms,
                                  status);
  if (!validatePermissions) {
    return false;
  }

  std::vector<std::string> allPathEntries;

  if (recurse) {
    std::vector<std::string> tempPathEntries;
    for (const auto& i : pathEntries) {
      if (cmSystemTools::FileIsDirectory(i)) {
        globber.FindFiles(i + "/*");
        tempPathEntries = globber.GetFiles();
        allPathEntries.insert(allPathEntries.end(), tempPathEntries.begin(),
                              tempPathEntries.end());
        allPathEntries.emplace_back(i);
      } else {
        allPathEntries.emplace_back(i); // We validate path entries below
      }
    }
  } else {
    allPathEntries = std::move(pathEntries);
  }

  // chmod
  for (const auto& i : allPathEntries) {
    if (!(cmSystemTools::FileExists(i) || cmSystemTools::FileIsDirectory(i))) {
      status.SetError(cmStrCat("does not exist:\n  ", i));
      cmSystemTools::SetFatalErrorOccurred();
      return false;
    }

    if (cmSystemTools::FileExists(i, true)) {
      bool success = true;
      const mode_t& filePermissions =
        parsedArgs.FilePermissions ? fperms : perms;
      if (filePermissions) {
        success = SetPermissions(i, filePermissions, status);
      }
      if (!success) {
        return false;
      }
    }

    else if (cmSystemTools::FileIsDirectory(i)) {
      bool success = true;
      const mode_t& directoryPermissions =
        parsedArgs.DirectoryPermissions ? dperms : perms;
      if (directoryPermissions) {
        success = SetPermissions(i, directoryPermissions, status);
      }
      if (!success) {
        return false;
      }
    }
  }

  return true;
}

bool HandleChmodCommand(std::vector<std::string> const& args,
                        cmExecutionStatus& status)
{
  return HandleChmodCommandImpl(args, false, status);
}

bool HandleChmodRecurseCommand(std::vector<std::string> const& args,
                               cmExecutionStatus& status)
{
  return HandleChmodCommandImpl(args, true, status);
}

} // namespace

bool cmFileCommand(std::vector<std::string> const& args,
                   cmExecutionStatus& status)
{
  if (args.size() < 2) {
    status.SetError("must be called with at least two arguments.");
    return false;
  }

  static cmSubcommandTable const subcommand{
    { "WRITE"_s, HandleWriteCommand },
    { "APPEND"_s, HandleAppendCommand },
    { "DOWNLOAD"_s, HandleDownloadCommand },
    { "UPLOAD"_s, HandleUploadCommand },
    { "READ"_s, HandleReadCommand },
    { "MD5"_s, HandleHashCommand },
    { "SHA1"_s, HandleHashCommand },
    { "SHA224"_s, HandleHashCommand },
    { "SHA256"_s, HandleHashCommand },
    { "SHA384"_s, HandleHashCommand },
    { "SHA512"_s, HandleHashCommand },
    { "SHA3_224"_s, HandleHashCommand },
    { "SHA3_256"_s, HandleHashCommand },
    { "SHA3_384"_s, HandleHashCommand },
    { "SHA3_512"_s, HandleHashCommand },
    { "STRINGS"_s, HandleStringsCommand },
    { "GLOB"_s, HandleGlobCommand },
    { "GLOB_RECURSE"_s, HandleGlobRecurseCommand },
    { "MAKE_DIRECTORY"_s, HandleMakeDirectoryCommand },
    { "RENAME"_s, HandleRename },
    { "COPY_FILE"_s, HandleCopyFile },
    { "REMOVE"_s, HandleRemove },
    { "REMOVE_RECURSE"_s, HandleRemoveRecurse },
    { "COPY"_s, HandleCopyCommand },
    { "INSTALL"_s, HandleInstallCommand },
    { "DIFFERENT"_s, HandleDifferentCommand },
    { "RPATH_CHANGE"_s, HandleRPathChangeCommand },
    { "CHRPATH"_s, HandleRPathChangeCommand },
    { "RPATH_SET"_s, HandleRPathSetCommand },
    { "RPATH_CHECK"_s, HandleRPathCheckCommand },
    { "RPATH_REMOVE"_s, HandleRPathRemoveCommand },
    { "READ_ELF"_s, HandleReadElfCommand },
    { "REAL_PATH"_s, HandleRealPathCommand },
    { "RELATIVE_PATH"_s, HandleRelativePathCommand },
    { "TO_CMAKE_PATH"_s, HandleCMakePathCommand },
    { "TO_NATIVE_PATH"_s, HandleNativePathCommand },
    { "TOUCH"_s, HandleTouchCommand },
    { "TOUCH_NOCREATE"_s, HandleTouchNocreateCommand },
    { "TIMESTAMP"_s, HandleTimestampCommand },
    { "GENERATE"_s, HandleGenerateCommand },
    { "LOCK"_s, HandleLockCommand },
    { "SIZE"_s, HandleSizeCommand },
    { "READ_SYMLINK"_s, HandleReadSymlinkCommand },
    { "CREATE_LINK"_s, HandleCreateLinkCommand },
    { "GET_RUNTIME_DEPENDENCIES"_s, HandleGetRuntimeDependenciesCommand },
    { "CONFIGURE"_s, HandleConfigureCommand },
    { "ARCHIVE_CREATE"_s, HandleArchiveCreateCommand },
    { "ARCHIVE_EXTRACT"_s, HandleArchiveExtractCommand },
    { "CHMOD"_s, HandleChmodCommand },
    { "CHMOD_RECURSE"_s, HandleChmodRecurseCommand },
  };

  return subcommand(args[0], args, status);
}
