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


#include "cmStandardIncludes.h"


class ctest
{
public:

  /**
   * Run the test for a directory and any subdirectories
   */
  void ProcessDirectory(std::vector<std::string> &passed, 
                        std::vector<std::string> &failed);

  /**
   * Find the executable for a test
   */
  std::string FindExecutable(const char *exe);

  /**
   * constructor
   */
  ctest() {
    m_UseIncludeRegExp = false;
    m_UseExcludeRegExp = false;
    m_UseExcludeRegExpFirst = false;
  }

  bool m_UseIncludeRegExp;
  std::string m_IncludeRegExp;

  bool m_UseExcludeRegExp;
  bool m_UseExcludeRegExpFirst;
  std::string m_ExcludeRegExp;

  std::string m_ConfigType;
private:
};

