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
#include "cmOutputRequiredFilesCommand.h"
#include "cmMakeDepend.h"

class cmLBDepend : public cmMakeDepend
{
  /**
   * Compute the depend information for this class.
   */
  void DependWalk(cmDependInformation* info, const char* file);
};

void cmLBDepend::DependWalk(cmDependInformation* info, const char* file)
{
  std::ifstream fin(file);
  if(!fin)
    {
    cmSystemTools::Error("error can not open ", file);
    return;
    }
  
  char line[255];
  while(!fin.eof() && !fin.fail())
    {
    fin.getline(line, 255);
    if(!strncmp(line, "#include", 8))
      {
      // if it is an include line then create a string class
      std::string currentline = line;
      size_t qstart = currentline.find('\"', 8);
      size_t qend;
      // if a quote is not found look for a <
      if(qstart == std::string::npos)
	{
	qstart = currentline.find('<', 8);
	// if a < is not found then move on
	if(qstart == std::string::npos)
	  {
	  cmSystemTools::Error("unknown include directive ", 
                               currentline.c_str() );
	  continue;
	  }
	else
	  {
	  qend = currentline.find('>', qstart+1);
	  }
	}
      else
	{
	qend = currentline.find('\"', qstart+1);
	}
      // extract the file being included
      std::string includeFile = currentline.substr(qstart+1, qend - qstart-1);
      // see if the include matches the regular expression
      if(!m_IncludeFileRegularExpression.find(includeFile))
	{
	if(m_Verbose)
	  {
          std::string message = "Skipping ";
          message += includeFile;
          message += " for file ";
          message += file;
	  cmSystemTools::Error(message.c_str(), 0);
	  }
	continue;
	}
      
      // Add this file and all its dependencies.
      this->AddDependency(info, includeFile.c_str());
      /// add the cxx file if it exists
      std::string cxxFile = includeFile;
      std::string::size_type pos = cxxFile.rfind('.');
      if(pos != std::string::npos)
        {
        std::string root = cxxFile.substr(0, pos);
        cxxFile = root + ".cxx";
        bool found = false;
        // try jumping to .cxx .cpp and .c in order
        if(cmSystemTools::FileExists(cxxFile.c_str()))
          {
          found = true;
          }
        for(std::vector<std::string>::iterator i = 
              m_IncludeDirectories.begin();
            i != m_IncludeDirectories.end(); ++i)
          {
          std::string path = *i;
          path = path + "/";
          path = path + cxxFile;
          if(cmSystemTools::FileExists(path.c_str()))
            {
            found = true;
            }
          }
        if (!found)
          {
          cxxFile = root + ".cpp";
          if(cmSystemTools::FileExists(cxxFile.c_str()))
            {
            found = true;
            }
          for(std::vector<std::string>::iterator i = 
                m_IncludeDirectories.begin();
              i != m_IncludeDirectories.end(); ++i)
            {
            std::string path = *i;
            path = path + "/";
            path = path + cxxFile;
            if(cmSystemTools::FileExists(path.c_str()))
              {
              found = true;
              }
            }
          }
        if (!found)
          {
          cxxFile = root + ".c";
          if(cmSystemTools::FileExists(cxxFile.c_str()))
            {
            found = true;
            }
          for(std::vector<std::string>::iterator i = 
                m_IncludeDirectories.begin();
              i != m_IncludeDirectories.end(); ++i)
            {
            std::string path = *i;
            path = path + "/";
            path = path + cxxFile;
            if(cmSystemTools::FileExists(path.c_str()))
              {
              found = true;
              }
            }
          }
        if (found)
          {
          this->AddDependency(info, cxxFile.c_str());
          }
        }
      }
    }
}

// cmOutputRequiredFilesCommand
bool cmOutputRequiredFilesCommand::InitialPass(std::vector<std::string>& args)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // store the arg for final pass
  m_File = args[0];
  m_OutputFile = args[1];
  
  return true;
}

void cmOutputRequiredFilesCommand::FinalPass()
{
  
  cmTargets &tgts = m_Makefile->GetTargets();
  for (cmTargets::iterator l = tgts.begin(); l != tgts.end(); l++)
    {
    l->second.GenerateSourceFilesFromSourceLists(*m_Makefile);
    }

  // compute the list of files
  cmLBDepend md;
  md.SetMakefile(m_Makefile);
  md.GenerateDependInformation();

  // always expand the first argument
  m_Makefile->ExpandVariablesInString(m_File);
  m_Makefile->ExpandVariablesInString(m_OutputFile);

  // find the depends for a file
  const cmMakeDepend::DependArray &da = md.GetDependInformation();
  const cmDependInformation *info = md.GetDependInformationForSourceFile(m_File.c_str());
  if (info)
    {
    // write them out
    FILE *fout = fopen(m_OutputFile.c_str(),"w");
    for( cmDependInformation::IndexSet::const_iterator indx = 
           info->m_IndexSet.begin();
         indx != info->m_IndexSet.end(); ++indx)
      {
      std::string tmp = md.GetDependInformation()[*indx]->m_FullPath;
      std::string::size_type pos = tmp.rfind('.');
      if(pos != std::string::npos && tmp.substr(pos) == ".cxx")
        {
        tmp = tmp.substr(0, pos);
        fprintf(fout,"%s\n",md.GetDependInformation()[*indx]->m_FullPath.c_str());
        }
      }
    fclose(fout);
    }
}
