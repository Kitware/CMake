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
#include "cmAddTestCommand.h"
#include "cmCacheManager.h"

// cmExecutableCommand
bool cmAddTestCommand::InitialPass(std::vector<std::string> const& args)
{
  // First argument is the name of the test
  // Second argument is the name of the executable to run (a target or external
  //    program)
  // Remaining arguments are the arguments to pass to the executable
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  // store the arguments for the final pass
  m_Args.erase(m_Args.begin(), m_Args.end());
  std::copy(args.begin(),args.end(),std::back_inserter(m_Args));
  return true;
}

// we append to the file in the final pass because Enable Testing command
// creates the file in the final pass.
void cmAddTestCommand::FinalPass()
{

  // Expand any CMake variables
  std::vector<std::string>::iterator s;
  for (s = m_Args.begin(); s != m_Args.end(); ++s)
    {
    m_Makefile->ExpandVariablesInString(*s);
    }

  // Create a full path filename for output Testfile
  std::string fname;
  fname = m_Makefile->GetStartOutputDirectory();
  fname += "/";
  fname += "DartTestfile.txt";
  

  // If the file doesn't exist, then ENABLE_TESTING hasn't been run
  if (cmSystemTools::FileExists(fname.c_str()))
    {
    // Open the output Testfile
    std::ofstream fout(fname.c_str(), std::ios::app);
    if (!fout)
      {
        cmSystemTools::Error("Error Writing ", fname.c_str());
        return;
      }

    std::vector<std::string>::iterator it;

  // for each arg in the test
    fout << "ADD_TEST(";
    it = m_Args.begin();
    fout << (*it).c_str();
    ++it;
    for (; it != m_Args.end(); ++it)
      {
        fout << " " << (*it).c_str();
      }
    fout << ")" << std::endl;
    fout.close();
    }  
  return;
}

