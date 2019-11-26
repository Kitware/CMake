/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmAuxSourceDirectoryCommand.h"

#include <algorithm>
#include <cstddef>
#include <utility>

#include "cmsys/Directory.hxx"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmake.h"

bool cmAuxSourceDirectoryCommand(std::vector<std::string> const& args,
                                 cmExecutionStatus& status)
{
  if (args.size() != 2) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  cmMakefile& mf = status.GetMakefile();
  std::string sourceListValue;
  std::string const& templateDirectory = args[0];
  std::string tdir;
  if (!cmSystemTools::FileIsFullPath(templateDirectory)) {
    tdir = cmStrCat(mf.GetCurrentSourceDirectory(), '/', templateDirectory);
  } else {
    tdir = templateDirectory;
  }

  // was the list already populated
  const char* def = mf.GetDefinition(args[1]);
  if (def) {
    sourceListValue = def;
  }

  std::vector<std::string> files;

  // Load all the files in the directory
  cmsys::Directory dir;
  if (dir.Load(tdir)) {
    size_t numfiles = dir.GetNumberOfFiles();
    for (size_t i = 0; i < numfiles; ++i) {
      std::string file = dir.GetFile(static_cast<unsigned long>(i));
      // Split the filename into base and extension
      std::string::size_type dotpos = file.rfind('.');
      if (dotpos != std::string::npos) {
        std::string ext = file.substr(dotpos + 1);
        std::string base = file.substr(0, dotpos);
        // Process only source files
        auto cm = mf.GetCMakeInstance();
        if (!base.empty() && cm->IsSourceExtension(ext)) {
          std::string fullname = cmStrCat(templateDirectory, '/', file);
          // add the file as a class file so
          // depends can be done
          cmSourceFile* sf = mf.GetOrCreateSource(fullname);
          sf->SetProperty("ABSTRACT", "0");
          files.push_back(std::move(fullname));
        }
      }
    }
  }
  std::sort(files.begin(), files.end());
  if (!sourceListValue.empty()) {
    sourceListValue += ";";
  }
  sourceListValue += cmJoin(files, ";");
  mf.AddDefinition(args[1], sourceListValue);
  return true;
}
