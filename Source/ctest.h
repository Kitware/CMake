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
#include "cmSystemTools.h"

class ctest
{
public:

  /**
   * Run the test for a directory and any subdirectories
   */
  void ProcessDirectory(int &passed, std::vector<std::string> &failed);

  /**
   * Find the executable for a test
   */
  std::string FindExecutable(const char *exe);

  /**
   * constructor
   */
  ctest() {m_UseRegExp = false;}

  bool m_UseRegExp;
  std::string m_RegExp;

private:
};

