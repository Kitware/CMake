/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) 2001 Insight Consortium
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * The name of the Insight Consortium, nor the names of any consortium members,
   nor of any contributors, may be used to endorse or promote products derived
   from this software without specific prior written permission.

  * Modified source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "cmUseMangledMesaCommand.h"
#include "cmSystemTools.h"

// cmUseMangledMesaCommand
bool cmUseMangledMesaCommand::InitialPass(std::vector<std::string> const& argsIn)
{ 
  // expected two arguments:
  // arguement one: the full path to gl_mangle.h
  // arguement two : directory for output of edited headers
  if(argsIn.size() < 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::vector<std::string>  args = argsIn;
  m_Makefile->ExpandVariablesInString(args[0]);
  m_Makefile->ExpandVariablesInString(args[1]);
  const char* inputDir = args[0].c_str();
  const char* destDir = args[1].c_str();
  std::vector<std::string> files;
  cmSystemTools::Glob(inputDir, "\\.h$", files);
  if(files.size() == 0)
    {
    cmSystemTools::Error("Could not open Mesa Directory ", inputDir);
    return false;
    }
  cmSystemTools::MakeDirectory(destDir);
  for(std::vector<std::string>::iterator i = files.begin();
      i != files.end(); ++i)
    {
    std::string path = inputDir;
    path += "/";
    path += *i;
    this->CopyAndFullPathMesaHeader(path.c_str(), destDir);
    }
  
  return true;
}

void 
cmUseMangledMesaCommand::
CopyAndFullPathMesaHeader(const char* source,
                          const char* outdir)
{
  std::string dir, file;
  cmSystemTools::SplitProgramPath(source, dir, file);
  std::string outFile = outdir;
  outFile += "/";
  outFile += file;
  std::string tempOutputFile = outFile;
  tempOutputFile += ".tmp";
  std::ofstream fout(tempOutputFile.c_str());
  if(!fout)
    {
    cmSystemTools::Error("Could not open file for write in copy operatation: ", 
                         tempOutputFile.c_str(), outdir);
    return;
    }
  std::ifstream fin(source);
  if(!fin)
    {
    cmSystemTools::Error("Could not open file for read in copy operatation",
                         source);
    return;
    }
  // now copy input to output and expand varibles in the
  // input file at the same time
  const int bufSize = 4096;
  char buffer[bufSize];
  std::string inLine;  
  // regular expression for any #include line
  cmRegularExpression includeLine("^[ \t]*#[ \t]*include[ \t]*[<\"]([^\">]+)[\">]");
  // regular expression for gl/ or GL/ in a file (match(1) of above)
  cmRegularExpression glDirLine("(gl|GL)(/|\\\\)([^<\"]+)");
  // regular expression for gl GL or xmesa in a file (match(1) of above)
  cmRegularExpression glLine("(gl|GL|xmesa)");
  while(fin)
    {
    fin.getline(buffer, bufSize);
    if(fin)
      {
      inLine = buffer;
      if(includeLine.find(inLine.c_str()))
        {
        std::string includeFile = includeLine.match(1);
        if(glDirLine.find(includeFile.c_str()))
          {
          std::string file = glDirLine.match(3);
          fout << "#include \"" << outdir << "/" << file.c_str() << "\"\n";
          }
        else if(glLine.find(includeFile.c_str()))
          {
          fout << "#include \"" << outdir << "/" << includeLine.match(1).c_str() << "\"\n";
          }
        else
          {
          fout << inLine << "\n";
          }
        }
      else
        {
        fout << inLine << "\n";
        }
      }
    }
  // close the files before attempting to copy
  fin.close();
  fout.close();
  cmSystemTools::CopyFileIfDifferent(tempOutputFile.c_str(),
                                     outFile.c_str());
  cmSystemTools::RemoveFile(tempOutputFile.c_str());
}

