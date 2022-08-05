/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmConfigureFileCommand.h"

#include <set>
#include <sstream>

#include <cm/string_view>
#include <cmext/string_view>

#include <sys/types.h>

#include "cmExecutionStatus.h"
#include "cmFSPermissions.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmNewLineStyle.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

// cmConfigureFileCommand
bool cmConfigureFileCommand(std::vector<std::string> const& args,
                            cmExecutionStatus& status)
{
  if (args.size() < 2) {
    status.SetError("called with incorrect number of arguments, expected 2");
    return false;
  }

  std::string const& inFile = args[0];
  const std::string inputFile = cmSystemTools::CollapseFullPath(
    inFile, status.GetMakefile().GetCurrentSourceDirectory());

  // If the input location is a directory, error out.
  if (cmSystemTools::FileIsDirectory(inputFile)) {
    status.SetError(cmStrCat("input location\n  ", inputFile,
                             "\n"
                             "is a directory but a file was expected."));
    return false;
  }

  std::string const& outFile = args[1];
  std::string outputFile = cmSystemTools::CollapseFullPath(
    outFile, status.GetMakefile().GetCurrentBinaryDirectory());

  // If the output location is already a directory put the file in it.
  if (cmSystemTools::FileIsDirectory(outputFile)) {
    outputFile += "/";
    outputFile += cmSystemTools::GetFilenameName(inFile);
  }

  if (!status.GetMakefile().CanIWriteThisFile(outputFile)) {
    std::string e = "attempted to configure a file: " + outputFile +
      " into a source directory.";
    status.SetError(e);
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }
  std::string errorMessage;
  cmNewLineStyle newLineStyle;
  if (!newLineStyle.ReadFromArguments(args, errorMessage)) {
    status.SetError(errorMessage);
    return false;
  }
  bool copyOnly = false;
  bool escapeQuotes = false;
  bool useSourcePermissions = false;
  bool noSourcePermissions = false;
  bool filePermissions = false;
  std::vector<std::string> filePermissionOptions;

  enum class Doing
  {
    DoingNone,
    DoingFilePermissions,
    DoneFilePermissions
  };

  Doing doing = Doing::DoingNone;

  static std::set<cm::string_view> noopOptions = {
    /* Legacy.  */
    "IMMEDIATE"_s,
    /* Handled by NewLineStyle member.  */
    "NEWLINE_STYLE"_s,
    "LF"_s,
    "UNIX"_s,
    "CRLF"_s,
    "WIN32"_s,
    "DOS"_s,
  };

  std::string unknown_args;
  bool atOnly = false;
  for (unsigned int i = 2; i < args.size(); ++i) {
    if (args[i] == "COPYONLY") {
      if (doing == Doing::DoingFilePermissions) {
        doing = Doing::DoneFilePermissions;
      }
      copyOnly = true;
      if (newLineStyle.IsValid()) {
        status.SetError("COPYONLY could not be used in combination "
                        "with NEWLINE_STYLE");
        return false;
      }
    } else if (args[i] == "ESCAPE_QUOTES") {
      if (doing == Doing::DoingFilePermissions) {
        doing = Doing::DoneFilePermissions;
      }
      escapeQuotes = true;
    } else if (args[i] == "@ONLY") {
      if (doing == Doing::DoingFilePermissions) {
        doing = Doing::DoneFilePermissions;
      }
      atOnly = true;
    } else if (args[i] == "NO_SOURCE_PERMISSIONS") {
      if (doing == Doing::DoingFilePermissions) {
        status.SetError(" given both FILE_PERMISSIONS and "
                        "NO_SOURCE_PERMISSIONS. Only one option allowed.");
        return false;
      }
      noSourcePermissions = true;
    } else if (args[i] == "USE_SOURCE_PERMISSIONS") {
      if (doing == Doing::DoingFilePermissions) {
        status.SetError(" given both FILE_PERMISSIONS and "
                        "USE_SOURCE_PERMISSIONS. Only one option allowed.");
        return false;
      }
      useSourcePermissions = true;
    } else if (args[i] == "FILE_PERMISSIONS") {
      if (useSourcePermissions) {
        status.SetError(" given both FILE_PERMISSIONS and "
                        "USE_SOURCE_PERMISSIONS. Only one option allowed.");
        return false;
      }
      if (noSourcePermissions) {
        status.SetError(" given both FILE_PERMISSIONS and "
                        "NO_SOURCE_PERMISSIONS. Only one option allowed.");
        return false;
      }

      if (doing == Doing::DoingNone) {
        doing = Doing::DoingFilePermissions;
        filePermissions = true;
      }
    } else if (noopOptions.find(args[i]) != noopOptions.end()) {
      /* Ignore no-op options.  */
    } else if (doing == Doing::DoingFilePermissions) {
      filePermissionOptions.push_back(args[i]);
    } else {
      unknown_args += " ";
      unknown_args += args[i];
      unknown_args += "\n";
    }
  }
  if (!unknown_args.empty()) {
    std::string msg = cmStrCat(
      "configure_file called with unknown argument(s):\n", unknown_args);
    status.GetMakefile().IssueMessage(MessageType::AUTHOR_WARNING, msg);
  }

  if (useSourcePermissions && noSourcePermissions) {
    status.SetError(" given both USE_SOURCE_PERMISSIONS and "
                    "NO_SOURCE_PERMISSIONS. Only one option allowed.");
    return false;
  }

  mode_t permissions = 0;

  if (filePermissions) {
    if (filePermissionOptions.empty()) {
      status.SetError(" given FILE_PERMISSIONS without any options.");
      return false;
    }

    std::vector<std::string> invalidOptions;
    for (auto const& e : filePermissionOptions) {
      if (!cmFSPermissions::stringToModeT(e, permissions)) {
        invalidOptions.push_back(e);
      }
    }

    if (!invalidOptions.empty()) {
      std::ostringstream oss;
      oss << " given invalid permission ";
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

  if (noSourcePermissions) {
    permissions |= cmFSPermissions::mode_owner_read;
    permissions |= cmFSPermissions::mode_owner_write;
    permissions |= cmFSPermissions::mode_group_read;
    permissions |= cmFSPermissions::mode_world_read;
  }

  if (!status.GetMakefile().ConfigureFile(inputFile, outputFile, copyOnly,
                                          atOnly, escapeQuotes, permissions,
                                          newLineStyle)) {
    status.SetError("Problem configuring file");
    return false;
  }

  return true;
}
