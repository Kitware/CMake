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
#ifndef cmCTestSubmit_h
#define cmCTestSubmit_h

#include "cmStandardIncludes.h"

/** \class cmCTestSubmit
 * \brief Helper class for CTest
 *
 * Submit testing results
 * 
 */
class cmCTestSubmit
{
public:
  cmCTestSubmit();
  ~cmCTestSubmit() {}
  
  /**
   * Submit file using various ways
   */
  bool SubmitUsingFTP(const std::string& localprefix, 
                      const std::vector<std::string>& files,
                      const std::string& remoteprefix, 
                      const std::string& url);
  bool SubmitUsingHTTP(const std::string& localprefix, 
                       const std::vector<std::string>& files,
                       const std::string& remoteprefix, 
                       const std::string& url);
  bool SubmitUsingSCP(const std::string& localprefix, 
                      const std::vector<std::string>& files,
                      const std::string& remoteprefix, 
                      const std::string& url);

  bool TriggerUsingHTTP(const std::vector<std::string>& files,
                        const std::string& remoteprefix, 
                        const std::string& url);

private:
  std::string m_HTTPProxy;
  int         m_HTTPProxyType;
  std::string m_FTPProxy;
  int         m_FTPProxyType;
};

#endif
