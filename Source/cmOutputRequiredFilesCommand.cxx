/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmOutputRequiredFilesCommand.h"
#include "cmMakeDepend.h"

class cmLBDepend : public cmMakeDepend
{
  /**
   * Compute the depend information for this class.
   */
  virtual void DependWalk(cmDependInformation* info);
};

void cmLBDepend::DependWalk(cmDependInformation* info)
{
  std::ifstream fin(info->m_FullPath.c_str());
  if(!fin)
    {
    cmSystemTools::Error("error can not open ", info->m_FullPath.c_str());
    return;
    }
  
  std::string line;
  while(cmSystemTools::GetLineFromStream(fin, line))
    {
    if(!strncmp(line.c_str(), "#include", 8))
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
          message += info->m_FullPath.c_str();
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
bool cmOutputRequiredFilesCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() != 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // store the arg for final pass
  m_File = args[0];
  m_OutputFile = args[1];
  
  return true;
}

void cmOutputRequiredFilesCommand::
ListDependencies(cmDependInformation const *info,
                 FILE *fout,
                 std::set<cmDependInformation const*> *visited)
{
  // add info to the visited set
  visited->insert(info);
  
  // now recurse with info's dependencies
  for(cmDependInformation::DependencySet::const_iterator d = 
        info->m_DependencySet.begin();
      d != info->m_DependencySet.end(); ++d)
    {
    if (visited->find(*d) == visited->end())
      {
      if(info->m_FullPath != "")
        {
        std::string tmp = (*d)->m_FullPath;
        std::string::size_type pos = tmp.rfind('.');
        if(pos != std::string::npos && tmp.substr(pos) == ".cxx")
          {
          tmp = tmp.substr(0, pos);
          fprintf(fout,"%s\n",(*d)->m_FullPath.c_str());
          }
        }
      this->ListDependencies(*d,fout,visited);
      }
    }
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

  // find the depends for a file
  const cmDependInformation *info = md.FindDependencies(m_File.c_str());
  if (info)
    {
    // write them out
    FILE *fout = fopen(m_OutputFile.c_str(),"w");
    std::set<cmDependInformation const*> visited;
    this->ListDependencies(info,fout, &visited);
    fclose(fout);
    }
}
