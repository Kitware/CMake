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
#include "cmSourceFilesCommand.h"

// cmSourceFilesCommand
bool cmSourceFilesCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  std::string name = args[0];
  m_Makefile->ExpandVariablesInString(name);
  
  int generated = 0;

  for(std::vector<std::string>::const_iterator i = (args.begin() + 1);
      i != args.end(); ++i)
    {
    std::string copy = *i;
    // Keyword GENERATED in the source file list means that 
    // from here on files will be generated
    if ( copy == "GENERATED" )
      {
      generated = 1;
      continue;
      }
    cmSourceFile file;
    m_Makefile->ExpandVariablesInString(copy);
    file.SetIsAnAbstractClass(false);
    std::string path = cmSystemTools::GetFilenamePath(copy);
    if ( generated )
      {
      // This file will be generated, so we should not check
      // if it exist. 
      std::string ext = cmSystemTools::GetFilenameExtension(copy);
      std::string name_no_ext = cmSystemTools::GetFilenameName(copy.c_str());
      name_no_ext = name_no_ext.substr(0, name_no_ext.length()-ext.length());
      if ( ext.length() && ext[0] == '.' )
	{
	ext = ext.substr(1);
	}
      if((path.size() && path[0] == '/') ||
	 (path.size() > 1 && path[1] == ':'))
	{
	file.SetName(name_no_ext.c_str(), path.c_str(), ext.c_str(), false);
	}
      else
	{
	file.SetName(name_no_ext.c_str(), m_Makefile->GetCurrentOutputDirectory(), 
		     ext.c_str(), false);
	}
      }
    else
      // if this is a full path then 
      if((path.size() && path[0] == '/') ||
	 (path.size() > 1 && path[1] == ':'))
	{
	file.SetName(cmSystemTools::GetFilenameName(copy.c_str()).c_str(), 
		     path.c_str(),
		     m_Makefile->GetSourceExtensions(),
		     m_Makefile->GetHeaderExtensions());
	}
      else
	{
	file.SetName(copy.c_str(), m_Makefile->GetCurrentDirectory(),
		     m_Makefile->GetSourceExtensions(),
		     m_Makefile->GetHeaderExtensions());
	}    
    m_Makefile->AddSource(file, name.c_str());
    }

  return true;
}

