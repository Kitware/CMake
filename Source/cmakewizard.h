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


#include "cmMakefile.h"

class cmakewizard
{
public:
  cmakewizard();
  /** 
   * Prompt the user to see if they want to see advanced entries.
   */
  virtual bool AskAdvanced();
  
  /**
   * Prompt the User for a new value for key, the answer is put in entry.
   */
  virtual void AskUser(const char* key, cmCacheManager::CacheIterator& iter);
  ///! Show a message to wait for cmake to run.
  virtual void ShowMessage(const char*);
  
  /**  
   *  Run cmake in wizard mode.  This will coninue to ask the user questions 
   *  until there are no more entries in the cache.
   */
  int RunWizard(std::vector<std::string>const& args);
  
private:
  bool m_ShowAdvanced;
};

