/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmConfigureFileCommand.h"

#include "cmExecutionStatus.h"
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
    cmSystemTools::SetFatalErrorOccured();
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

  std::string unknown_args;
  bool atOnly = false;
  for (unsigned int i = 2; i < args.size(); ++i) {
    if (args[i] == "COPYONLY") {
      copyOnly = true;
      if (newLineStyle.IsValid()) {
        status.SetError("COPYONLY could not be used in combination "
                        "with NEWLINE_STYLE");
        return false;
      }
    } else if (args[i] == "ESCAPE_QUOTES") {
      escapeQuotes = true;
    } else if (args[i] == "@ONLY") {
      atOnly = true;
    } else if (args[i] == "IMMEDIATE") {
      /* Ignore legacy option.  */
    } else if (args[i] == "NEWLINE_STYLE" || args[i] == "LF" ||
               args[i] == "UNIX" || args[i] == "CRLF" || args[i] == "WIN32" ||
               args[i] == "DOS") {
      /* Options handled by NewLineStyle member above.  */
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

  if (!status.GetMakefile().ConfigureFile(
        inputFile, outputFile, copyOnly, atOnly, escapeQuotes, newLineStyle)) {
    status.SetError("Problem configuring file");
    return false;
  }

  return true;
}
