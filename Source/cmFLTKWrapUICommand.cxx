/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmFLTKWrapUICommand.h"

#include <cstddef>

#include "cmCustomCommandLines.h"
#include "cmExecutionStatus.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmRange.h"
#include "cmSourceFile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmake.h"

class cmTarget;

static void FinalAction(cmMakefile& makefile, std::string const& name,
                        const cmListFileBacktrace& lfbt)
{
  // people should add the srcs to the target themselves, but the old command
  // didn't support that, so check and see if they added the files in and if
  // they didn;t then print a warning and add then anyhow
  cmTarget* target = makefile.FindLocalNonAliasTarget(name);
  if (!target) {
    std::string msg = cmStrCat(
      "FLTK_WRAP_UI was called with a target that was never created: ", name,
      ".  The problem was found while processing the source directory: ",
      makefile.GetCurrentSourceDirectory(),
      ".  This FLTK_WRAP_UI call will be ignored.");
    makefile.GetCMakeInstance()->IssueMessage(MessageType::AUTHOR_ERROR, msg,
                                              lfbt);
  }
}

bool cmFLTKWrapUICommand(std::vector<std::string> const& args,
                         cmExecutionStatus& status)
{
  if (args.size() < 2) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  cmMakefile& mf = status.GetMakefile();

  // what is the current source dir
  std::string cdir = mf.GetCurrentSourceDirectory();
  std::string const& fluid_exe =
    mf.GetRequiredDefinition("FLTK_FLUID_EXECUTABLE");

  // Target that will use the generated files
  std::string const& target = args[0];

  // get the list of GUI files from which .cxx and .h will be generated
  std::string outputDirectory = mf.GetCurrentBinaryDirectory();

  {
    // Some of the generated files are *.h so the directory "GUI"
    // where they are created have to be added to the include path
    std::vector<std::string> outputDirectories;
    outputDirectories.push_back(outputDirectory);
    mf.AddIncludeDirectories(outputDirectories);
  }

  // List of produced files.
  std::vector<cmSourceFile*> generatedSourcesClasses;

  for (std::string const& arg : cmMakeRange(args).advance(1)) {
    cmSourceFile* curr = mf.GetSource(arg);
    // if we should use the source GUI
    // to generate .cxx and .h files
    if (!curr || !curr->GetPropertyAsBool("WRAP_EXCLUDE")) {
      std::string outName = cmStrCat(
        outputDirectory, "/", cmSystemTools::GetFilenameWithoutExtension(arg));
      std::string hname = cmStrCat(outName, ".h");
      std::string origname = cmStrCat(cdir, "/", arg);
      // add starting depends
      std::vector<std::string> depends;
      depends.push_back(origname);
      depends.push_back(fluid_exe);
      std::string cxxres = cmStrCat(outName, ".cxx");

      cmCustomCommandLines commandLines = cmMakeSingleCommandLine({
        fluid_exe,
        "-c", // instructs Fluid to run in command line
        "-h", // optionally rename .h files
        hname,
        "-o", // optionally rename .cxx files
        cxxres,
        origname // name of the GUI fluid file
      });

      // Add command for generating the .h and .cxx files
      std::string no_main_dependency;
      const char* no_comment = nullptr;
      const char* no_working_dir = nullptr;
      mf.AddCustomCommandToOutput(cxxres, depends, no_main_dependency,
                                  commandLines, no_comment, no_working_dir);
      mf.AddCustomCommandToOutput(hname, depends, no_main_dependency,
                                  commandLines, no_comment, no_working_dir);

      cmSourceFile* sf = mf.GetSource(cxxres);
      sf->AddDepend(hname);
      sf->AddDepend(origname);
      generatedSourcesClasses.push_back(sf);
    }
  }

  // create the variable with the list of sources in it
  size_t lastHeadersClass = generatedSourcesClasses.size();
  std::string sourceListValue;
  for (size_t classNum = 0; classNum < lastHeadersClass; classNum++) {
    if (classNum) {
      sourceListValue += ";";
    }
    sourceListValue += generatedSourcesClasses[classNum]->ResolveFullPath();
  }

  std::string const varName = target + "_FLTK_UI_SRCS";
  mf.AddDefinition(varName, sourceListValue);

  mf.AddGeneratorAction(
    [target](cmLocalGenerator& lg, const cmListFileBacktrace& lfbt) {
      FinalAction(*lg.GetMakefile(), target, lfbt);
    });
  return true;
}
