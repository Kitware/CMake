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


#include "cmMakefile.h"
#include "cmStandardIncludes.h"

class cmakewizard
{
public:
  cmakewizard();
  /** 
   * Prompt the user to see if they want to see advanced entires.
   */
  virtual bool AskAdvanced();
  
  /**
   * Prompt the User for a new value for key, the answer is put in entry.
   */
  virtual void AskUser(const char* key, cmCacheManager::CacheIterator& iter,
                       cmCacheManager *cm);
  ///! Show a message to wait for cmake to run.
  virtual void ShowMessage(const char*);
  
  /**  
   *  Run cmake in wizard mode.  This will coninue to ask the user questions 
   *  until there are no more entries in the cache.
   */
  void RunWizard(std::vector<std::string>const& args);
  
private:
  bool m_ShowAdvanced;
};

