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
#include "cmEnableTestingCommand.h"
#include "cmCacheManager.h"

// we do this in the final pass so that we now the subdirs have all 
// been defined
void cmEnableTestingCommand::FinalPass()
{
  // Create a full path filename for output Testfile
  std::string fname;
  fname = m_Makefile->GetStartOutputDirectory();
  fname += "/";
  fname += "CMakeTestfile.txt";
  
  cmSystemTools::MakeDirectory(m_Makefile->GetStartOutputDirectory());

  // Open the output Testfile
  std::ofstream fout(fname.c_str());
  if (!fout)
    {
    cmSystemTools::Error("Error Writing ", fname.c_str());
    return;
    }
  
  fout << "# CMake generated Testfile for " << std::endl
       << "#\tSource directory: "
       << m_Makefile->GetStartDirectory()
       << std::endl
       << "#\tBuild directory: " << m_Makefile->GetStartOutputDirectory()
       << std::endl
       << "# " << std::endl
       << "# This file replicates the SUBDIRS() and ADD_TEST() commands from the source"
       << std::endl
       << "# tree CMakeLists.txt file, skipping any SUBDIRS() or ADD_TEST() commands"
       << std::endl
       << "# that are excluded by CMake control structures, i.e. IF() commands."
       << std::endl
       << "#" 
       << std::endl << std::endl;

  // write out the subdirs for the current directory
  if (!m_Makefile->GetSubDirectories().empty())
    {
    fout << "SUBDIRS(";
    const std::vector<std::string>& subdirs = m_Makefile->GetSubDirectories();
    std::vector<std::string>::const_iterator i = subdirs.begin();
    fout << (*i).c_str();
    ++i;
    for(; i != subdirs.end(); ++i)
      {
      fout << " " << (*i).c_str();
      }
    fout << ")" << std::endl << std::endl;;
    }
  fout.close();  

  return;
}

