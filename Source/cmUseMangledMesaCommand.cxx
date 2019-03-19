/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmUseMangledMesaCommand.h"

#include "cmsys/FStream.hxx"
#include "cmsys/RegularExpression.hxx"

#include "cmSystemTools.h"

class cmExecutionStatus;

bool cmUseMangledMesaCommand::InitialPass(std::vector<std::string> const& args,
                                          cmExecutionStatus&)
{
  // expected two arguments:
  // argument one: the full path to gl_mangle.h
  // argument two : directory for output of edited headers
  if (args.size() != 2) {
    this->SetError("called with incorrect number of arguments");
    return false;
  }
  const std::string& inputDir = args[0];
  std::string glh = inputDir;
  glh += "/";
  glh += "gl.h";
  if (!cmSystemTools::FileExists(glh)) {
    std::string e = "Bad path to Mesa, could not find: ";
    e += glh;
    e += " ";
    this->SetError(e);
    return false;
  }
  const char* destDir = args[1].c_str();
  std::vector<std::string> files;
  cmSystemTools::Glob(inputDir, "\\.h$", files);
  if (files.empty()) {
    cmSystemTools::Error("Could not open Mesa Directory " + inputDir);
    return false;
  }
  cmSystemTools::MakeDirectory(destDir);
  for (std::string const& f : files) {
    std::string path = inputDir;
    path += "/";
    path += f;
    this->CopyAndFullPathMesaHeader(path.c_str(), destDir);
  }

  return true;
}

void cmUseMangledMesaCommand::CopyAndFullPathMesaHeader(const char* source,
                                                        const char* outdir)
{
  std::string dir, file;
  cmSystemTools::SplitProgramPath(source, dir, file);
  std::string outFile = outdir;
  outFile += "/";
  outFile += file;
  std::string tempOutputFile = outFile;
  tempOutputFile += ".tmp";
  cmsys::ofstream fout(tempOutputFile.c_str());
  if (!fout) {
    cmSystemTools::Error("Could not open file for write in copy operation: " +
                         tempOutputFile + outdir);
    cmSystemTools::ReportLastSystemError("");
    return;
  }
  cmsys::ifstream fin(source);
  if (!fin) {
    cmSystemTools::Error("Could not open file for read in copy operation",
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
  cmsys::RegularExpression glDirLine("(gl|GL)(/|\\\\)([^<\"]+)");
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
  cmSystemTools::CopyFileIfDifferent(tempOutputFile, outFile);
  cmSystemTools::RemoveFile(tempOutputFile);
}
