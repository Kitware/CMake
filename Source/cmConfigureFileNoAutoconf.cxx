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
#include "cmConfigureFileNoAutoconf.h"

// cmConfigureFileNoAutoconf
bool cmConfigureFileNoAutoconf::InitialPass(std::vector<std::string>& args)
{
  if(args.size() != 2 )
    {
    this->SetError("called with incorrect number of arguments, expected 2");
    return false;
    }
  m_InputFile = args[0];
  m_OuputFile = args[1];
  return true;
}

void cmConfigureFileNoAutoconf::FinalPass()
{
#ifdef CMAKE_HAS_AUTOCONF
  return;
#else  
  m_Makefile->ExpandVariablesInString(m_InputFile);
  m_Makefile->ExpandVariablesInString(m_OuputFile);
  std::ifstream fin(m_InputFile.c_str());
  if(!fin)
    {
    cmSystemTools::Error("Could not open file for read in copy operatation",
                         m_InputFile.c_str());
    return;
    }
  cmSystemTools::ConvertToUnixSlashes(m_OuputFile);
  std::string::size_type pos = m_OuputFile.rfind('/');
  if(pos != std::string::npos)
    {
    std::string path = m_OuputFile.substr(0, pos);
    cmSystemTools::MakeDirectory(path.c_str());
    }
  std::string tempOutputFile = m_OuputFile;
  tempOutputFile += ".tmp";
  std::ofstream fout(tempOutputFile.c_str());
  if(!fout)
    {
    cmSystemTools::Error("Could not open file for write in copy operatation", 
                         tempOutputFile.c_str());
    return;
    }
  // now copy input to output and expand varibles in the
  // input file at the same time
  const int bufSize = 4096;
  char buffer[bufSize];
  std::string inLine;
  while(fin)
    {
    fin.getline(buffer, bufSize);
    if(fin)
      {
      inLine = buffer;
      m_Makefile->ExpandVariablesInString(inLine);
      fout << inLine << "\n";
      }
    }
  // close the files before attempting to copy
  fin.close();
  fout.close();
  cmSystemTools::CopyFileIfDifferent(tempOutputFile.c_str(),
                                     m_OuputFile.c_str());
  cmSystemTools::RemoveFile(tempOutputFile.c_str());
#endif
}

  
