/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) 2000 National Library of Medicine
  All rights reserved.

  See COPYRIGHT.txt for copyright details.

=========================================================================*/
#include "cmTarget.h"
#include "cmMakefile.h"

void cmTarget::GenerateSourceFilesFromSourceLists(const cmMakefile &mf)
{
  // for each src lists add the classes
  for (std::vector<std::string>::const_iterator s = m_SourceLists.begin();
       s != m_SourceLists.end(); ++s)
    {
    // replace any variables
    std::string temps = *s;
    mf.ExpandVariablesInString(temps);
    // look for a srclist
    if (mf.GetSources().find(temps) != mf.GetSources().end())
      {
      const std::vector<cmSourceFile> &clsList = 
        mf.GetSources().find(temps)->second;
      m_SourceFiles.insert(m_SourceFiles.end(), clsList.begin(), clsList.end());
      }
    // if one wasn't found then assume it is a single class
    else
      {
      cmSourceFile file;
      file.SetIsAnAbstractClass(false);
      file.SetName(temps.c_str(), mf.GetCurrentDirectory());
      m_SourceFiles.push_back(file);
      }
    }
}
