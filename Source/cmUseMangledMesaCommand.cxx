/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmUseMangledMesaCommand.h"

#include "cmsys/FStream.hxx"
#include "cmsys/RegularExpression.hxx"

#include "cmExecutionStatus.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

namespace {
void CopyAndFullPathMesaHeader(const std::string& source,
                               const std::string& outdir);
}

bool cmUseMangledMesaCommand(std::vector<std::string> const& args,
                             cmExecutionStatus& status)
{
  // expected two arguments:
  // argument one: the full path to gl_mangle.h
  // argument two : directory for output of edited headers
  if (args.size() != 2) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }
  const std::string& inputDir = args[0];
  std::string glh = cmStrCat(inputDir, "/gl.h");
  if (!cmSystemTools::FileExists(glh)) {
    std::string e = cmStrCat("Bad path to Mesa, could not find: ", glh, ' ');
    status.SetError(e);
    return false;
  }
  const std::string& destDir = args[1];
  std::vector<std::string> files;
  cmSystemTools::Glob(inputDir, "\\.h$", files);
  if (files.empty()) {
    cmSystemTools::Error("Could not open Mesa Directory " + inputDir);
    return false;
  }
  cmSystemTools::MakeDirectory(destDir);
  for (std::string const& f : files) {
    std::string path = cmStrCat(inputDir, '/', f);
    CopyAndFullPathMesaHeader(path, destDir);
  }

  return true;
}

namespace {
void CopyAndFullPathMesaHeader(const std::string& source,
                               const std::string& outdir)
{
  std::string dir;
  std::string file;
  cmSystemTools::SplitProgramPath(source, dir, file);
  std::string outFile = cmStrCat(outdir, '/', file);
  std::string tempOutputFile = cmStrCat(outFile, ".tmp");
  cmsys::ofstream fout(tempOutputFile.c_str());
  if (!fout) {
    cmSystemTools::Error("Could not open file for write in copy operation: " +
                         tempOutputFile + outdir);
    cmSystemTools::ReportLastSystemError("");
    return;
  }
  cmsys::ifstream fin(source.c_str());
  if (!fin) {
    cmSystemTools::Error("Could not open file for read in copy operation" +
                         source);
    return;
  }
  // now copy input to output and expand variables in the
  // input file at the same time
  std::string inLine;
  // regular expression for any #include line
  cmsys::RegularExpression includeLine(
    "^[ \t]*#[ \t]*include[ \t]*[<\"]([^\">]+)[\">]");
  // regular expression for gl/ or GL/ in a file (match(1) of above)
  cmsys::RegularExpression glDirLine(R"((gl|GL)(/|\\)([^<"]+))");
  // regular expression for gl GL or xmesa in a file (match(1) of above)
  cmsys::RegularExpression glLine("(gl|GL|xmesa)");
  while (cmSystemTools::GetLineFromStream(fin, inLine)) {
    if (includeLine.find(inLine)) {
      std::string includeFile = includeLine.match(1);
      if (glDirLine.find(includeFile)) {
        std::string gfile = glDirLine.match(3);
        fout << "#include \"" << outdir << "/" << gfile << "\"\n";
      } else if (glLine.find(includeFile)) {
        fout << "#include \"" << outdir << "/" << includeLine.match(1)
             << "\"\n";
      } else {
        fout << inLine << "\n";
      }
    } else {
      fout << inLine << "\n";
    }
  }
  // close the files before attempting to copy
  fin.close();
  fout.close();
  cmSystemTools::MoveFileIfDifferent(tempOutputFile, outFile);
}
}
