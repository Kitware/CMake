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
  ~cmCTestSubmit() { m_LogFile = 0; }

  /**
   * Set verbosity of send
   */
  void SetVerbose(bool i) { m_Verbose = i; }
  void VerboseOn() { this->SetVerbose(1); }
  void VerboseOff() { this->SetVerbose(0); }

  void SetLogFile(std::ostream* ost) { m_LogFile = ost; }
  
  /**
   * Submit file using various ways
   */
  bool SubmitUsingFTP(const cmStdString& localprefix, 
                      const std::vector<cmStdString>& files,
                      const cmStdString& remoteprefix, 
                      const cmStdString& url);
  bool SubmitUsingHTTP(const cmStdString& localprefix, 
                       const std::vector<cmStdString>& files,
                       const cmStdString& remoteprefix, 
                       const cmStdString& url);
  bool SubmitUsingSCP(const cmStdString& scp_command,
                      const cmStdString& localprefix, 
                      const std::vector<cmStdString>& files,
                      const cmStdString& remoteprefix, 
                      const cmStdString& url);

  bool TriggerUsingHTTP(const std::vector<cmStdString>& files,
                        const cmStdString& remoteprefix, 
                        const cmStdString& url);

private:
  cmStdString   m_HTTPProxy;
  int           m_HTTPProxyType;
  cmStdString   m_FTPProxy;
  int           m_FTPProxyType;
  bool          m_Verbose;
  std::ostream* m_LogFile;
};

#endif
