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
bool cmInstallFilesCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  if(argsIn.size() < 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::vector<std::string> args;
  m_Makefile->ExpandSourceListArguments(argsIn, args, 2);

  // Create an INSTALL_FILES target specifically for this path.
  m_TargetName = "INSTALL_FILES_"+args[0];
  cmTarget& target = m_Makefile->GetTargets()[m_TargetName];
  target.SetInAll(false);
  target.SetType(cmTarget::INSTALL_FILES);
  target.SetInstallPath(args[0].c_str());
  
  if((args.size() > 1) && (args[1] == "FILES"))
    {
    m_IsFilesForm = true;    
    for(std::vector<std::string>::const_iterator s = args.begin()+2;
        s != args.end(); ++s)
      {
      // Find the source location for each file listed.
      std::string f = this->FindInstallSource(s->c_str());
      target.GetSourceLists().push_back(f);
      }
    }
  else
    {
    m_IsFilesForm = false;
    std::vector<std::string>::const_iterator s = args.begin();
    for (++s;s != args.end(); ++s)
      {
      m_FinalArgs.push_back(*s);
      }
    }
  
  return true;
}

void cmInstallFilesCommand::FinalPass() 
{
  // No final pass for "FILES" form of arguments.
  if(m_IsFilesForm)
    {
    return;
    }
  
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
      if (cmSystemTools::GetFilenamePath(temps).size() > 0)
	{
	  testf = cmSystemTools::GetFilenamePath(temps) + "/" + 
	    cmSystemTools::GetFilenameWithoutLastExtension(temps) + ext;
	}
      else
	{
	  testf = cmSystemTools::GetFilenameWithoutLastExtension(temps) + ext;
	}
      
      // add to the result
      targetSourceLists.push_back(this->FindInstallSource(testf.c_str()));
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
      targetSourceLists.push_back(this->FindInstallSource(s->c_str()));
      }
    }
}

/**
 * Find a file in the build or source tree for installation given a
 * relative path from the CMakeLists.txt file.  This will favor files
 * present in the build tree.  If a full path is given, it is just
 * returned.
 */
std::string cmInstallFilesCommand::FindInstallSource(const char* name) const
{
  if(cmSystemTools::FileIsFullPath(name))
    {
    // This is a full path.
    return name;
    }
  
  // This is a relative path.
  std::string tb = m_Makefile->GetCurrentOutputDirectory();
  tb += "/";
  tb += name;
  std::string ts = m_Makefile->GetCurrentDirectory();
  ts += "/";
  ts += name;
  
  if(cmSystemTools::FileExists(tb.c_str()))
    {
    // The file exists in the binary tree.  Use it.
    return tb;
    }
  else if(cmSystemTools::FileExists(ts.c_str()))
    {
    // The file exists in the source tree.  Use it.
    return ts;
    }
  else
    {
    // The file doesn't exist.  Assume it will be present in the
    // binary tree when the install occurs.
    return tb;
    }
}
