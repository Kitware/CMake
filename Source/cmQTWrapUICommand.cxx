/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmQTWrapUICommand.h"

#include <utility>

#include <cm/memory>

#include "cmCustomCommand.h"
#include "cmCustomCommandLines.h"
#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmRange.h"
#include "cmSourceFile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

bool cmQTWrapUICommand(std::vector<std::string> const& args,
                       cmExecutionStatus& status)
{
  if (args.size() < 4) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  cmMakefile& mf = status.GetMakefile();

  // Get the uic and moc executables to run in the custom commands.
  std::string const& uic_exe = mf.GetRequiredDefinition("QT_UIC_EXECUTABLE");
  std::string const& moc_exe = mf.GetRequiredDefinition("QT_MOC_EXECUTABLE");

  // Get the variable holding the list of sources.
  std::string const& headerList = args[1];
  std::string const& sourceList = args[2];
  std::string headerListValue = mf.GetSafeDefinition(headerList);
  std::string sourceListValue = mf.GetSafeDefinition(sourceList);

  // Create rules for all sources listed.
  for (std::string const& arg : cmMakeRange(args).advance(3)) {
    cmSourceFile* curr = mf.GetSource(arg);
    // if we should wrap the class
    if (!(curr && curr->GetPropertyAsBool("WRAP_EXCLUDE"))) {
      // Compute the name of the files to generate.
      std::string srcName =
        cmSystemTools::GetFilenameWithoutLastExtension(arg);
      std::string hName =
        cmStrCat(mf.GetCurrentBinaryDirectory(), '/', srcName, ".h");
      std::string cxxName =
        cmStrCat(mf.GetCurrentBinaryDirectory(), '/', srcName, ".cxx");
      std::string mocName =
        cmStrCat(mf.GetCurrentBinaryDirectory(), "/moc_", srcName, ".cxx");

      // Compute the name of the ui file from which to generate others.
      std::string uiName;
      if (cmSystemTools::FileIsFullPath(arg)) {
        uiName = arg;
      } else {
        if (curr && curr->GetIsGenerated()) {
          uiName = mf.GetCurrentBinaryDirectory();
        } else {
          uiName = mf.GetCurrentSourceDirectory();
        }
        uiName += "/";
        uiName += arg;
      }

      // create the list of headers
      if (!headerListValue.empty()) {
        headerListValue += ";";
      }
      headerListValue += hName;

      // create the list of sources
      if (!sourceListValue.empty()) {
        sourceListValue += ";";
      }
      sourceListValue += cxxName;
      sourceListValue += ";";
      sourceListValue += mocName;

      // set up .ui to .h and .cxx command
      cmCustomCommandLines hCommandLines =
        cmMakeSingleCommandLine({ uic_exe, "-o", hName, uiName });
      cmCustomCommandLines cxxCommandLines = cmMakeSingleCommandLine(
        { uic_exe, "-impl", hName, "-o", cxxName, uiName });
      cmCustomCommandLines mocCommandLines =
        cmMakeSingleCommandLine({ moc_exe, "-o", mocName, hName });

      std::vector<std::string> depends;
      depends.push_back(uiName);
      auto cc = cm::make_unique<cmCustomCommand>();
      cc->SetOutputs(hName);
      cc->SetDepends(depends);
      cc->SetCommandLines(hCommandLines);
      mf.AddCustomCommandToOutput(std::move(cc));

      depends.push_back(hName);
      cc = cm::make_unique<cmCustomCommand>();
      cc->SetOutputs(cxxName);
      cc->SetDepends(depends);
      cc->SetCommandLines(cxxCommandLines);
      mf.AddCustomCommandToOutput(std::move(cc));

      depends.clear();
      depends.push_back(hName);
      cc = cm::make_unique<cmCustomCommand>();
      cc->SetOutputs(mocName);
      cc->SetDepends(depends);
      cc->SetCommandLines(mocCommandLines);
      mf.AddCustomCommandToOutput(std::move(cc));
    }
  }

  // Store the final list of source files and headers.
  mf.AddDefinition(sourceList, sourceListValue);
  mf.AddDefinition(headerList, headerListValue);
  return true;
}
