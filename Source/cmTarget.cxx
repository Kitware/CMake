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
#include "cmTarget.h"
#include "cmMakefile.h"

void cmTarget::GenerateSourceFilesFromSourceLists( cmMakefile &mf)
{
  // this is only done for non install targets
  if ((this->m_TargetType == cmTarget::INSTALL_FILES)
      || (this->m_TargetType == cmTarget::INSTALL_PROGRAMS))
    {
    return;
    }

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
      const std::vector<cmSourceFile*> &clsList = 
        mf.GetSources().find(temps)->second;
      // if we ahave a limited build list, use it
      m_SourceFiles.insert(m_SourceFiles.end(), 
                           clsList.begin(), 
                           clsList.end());
      }
    // if one wasn't found then assume it is a single class
    else
      {
      cmSourceFile file;
      file.SetIsAnAbstractClass(false);
      file.SetName(temps.c_str(), mf.GetCurrentDirectory(),
                   mf.GetSourceExtensions(),
                   mf.GetHeaderExtensions());
      m_SourceFiles.push_back(mf.AddSource(file));
      }
    }

  // expand any link library variables whle we are at it
  LinkLibraries::iterator p = m_LinkLibraries.begin();
  for (;p != m_LinkLibraries.end(); ++p)
    {
    mf.ExpandVariablesInString(p->first);    
    }
}

void cmTarget::MergeLibraries(const LinkLibraries &ll)
{
  typedef std::vector<std::pair<std::string,LinkLibraryType> > LinkLibraries;

  LinkLibraries::const_iterator p = ll.begin();
  for (;p != ll.end(); ++p)
    {
    m_LinkLibraries.push_back(*p);
    }

}

bool cmTarget::HasCxx() const
{
  for(std::vector<cmSourceFile*>::const_iterator i =  m_SourceFiles.begin();
      i != m_SourceFiles.end(); ++i)
    {
    if((*i)->GetSourceExtension() != "c")
      {
      return true;
      }
    }
  return false;
}
