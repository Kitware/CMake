/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmInstallFilesCommand.h"
#include "cmCacheManager.h"

// cmExecutableCommand
bool cmInstallFilesCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // Create an INSTALL_FILES target specifically for this path.
  m_TargetName = "INSTALL_FILES_"+args[0];
  cmTarget target;
  target.SetInAll(false);
  target.SetType(cmTarget::INSTALL_FILES);
  target.SetInstallPath(args[0].c_str());
  m_Makefile->GetTargets().insert(cmTargets::value_type(m_TargetName, target));

  std::vector<std::string>::const_iterator s = args.begin();
  for (++s;s != args.end(); ++s)
    {
    m_FinalArgs.push_back(*s);
    }
  
  return true;
}

void cmInstallFilesCommand::FinalPass() 
{
  std::string testf;
  std::string ext = m_FinalArgs[0];
  std::vector<std::string>& targetSourceLists =
    m_Makefile->GetTargets()[m_TargetName].GetSourceLists();
  
  // two different options
  if (m_FinalArgs.size() > 1)
    {
    // now put the files into the list
    std::vector<std::string>::iterator s = m_FinalArgs.begin();
    ++s;
    // for each argument, get the files 
    for (;s != m_FinalArgs.end(); ++s)
      {
      // replace any variables
      std::string temps = *s;
      // look for a srclist
      if (m_Makefile->GetSources().find(temps) != m_Makefile->GetSources().end())
        {
        const std::vector<cmSourceFile*> &clsList = 
          m_Makefile->GetSources().find(temps)->second;
        std::vector<cmSourceFile*>::const_iterator c = clsList.begin();
        for (; c != clsList.end(); ++c)
          {
          testf = (*c)->GetSourceName() + ext;
          // add to the result
          targetSourceLists.push_back(testf);
          }
        }
      // if one wasn't found then assume it is a single class
      else
        {
        testf = temps + ext;
        // add to the result
        targetSourceLists.push_back(testf);
        }
      }
    }
  else     // reg exp list
    {
    std::vector<std::string> files;
    std::string regex = m_FinalArgs[0].c_str();
    cmSystemTools::Glob(m_Makefile->GetCurrentDirectory(),
                        regex.c_str(), files);
    
    std::vector<std::string>::iterator s = files.begin();
    // for each argument, get the files 
    for (;s != files.end(); ++s)
      {
      targetSourceLists.push_back(*s);
      }
    }
}

      
