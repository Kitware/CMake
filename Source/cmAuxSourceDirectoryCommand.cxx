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
#include "cmAuxSourceDirectoryCommand.h"
#include "cmDirectory.h"

// cmAuxSourceDirectoryCommand
bool cmAuxSourceDirectoryCommand::InitialPass(std::vector<std::string>& args)
{
  if(args.size() < 2 || args.size() > 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  std::string templateDirectory = args[0];
  m_Makefile->AddExtraDirectory(templateDirectory.c_str());
  std::string tdir = m_Makefile->GetCurrentDirectory();
  tdir += "/";
  tdir += templateDirectory;
  // Load all the files in the directory
  cmDirectory dir;
  if(dir.Load(tdir.c_str()))
    {
    int numfiles = dir.GetNumberOfFiles();
    for(int i =0; i < numfiles; ++i)
      {
      std::string file = dir.GetFile(i);
      // Split the filename into base and extension
      std::string::size_type dotpos = file.rfind(".");
      if( dotpos != std::string::npos )
        {
        std::string ext = file.substr(dotpos+1);
        file = file.substr(0, dotpos);
        // Process only source files
        if( file.size() != 0
            && std::find( m_Makefile->GetSourceExtensions().begin(),
                          m_Makefile->GetSourceExtensions().end(), ext )
                 != m_Makefile->GetSourceExtensions().end() )
          {
          std::string fullname = templateDirectory;
          fullname += "/";
          fullname += file;
          // add the file as a class file so 
          // depends can be done
          cmSourceFile cmfile;
          cmfile.SetName(fullname.c_str(), m_Makefile->GetCurrentDirectory(),
                         m_Makefile->GetSourceExtensions(),
                         m_Makefile->GetHeaderExtensions());
          cmfile.SetIsAnAbstractClass(false);
          m_Makefile->AddSource(cmfile,args[1].c_str());
          }
        }
      }
    }
  return true;
}

