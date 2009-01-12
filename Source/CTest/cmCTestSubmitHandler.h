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
#ifndef cmCTestSubmitHandler_h
#define cmCTestSubmitHandler_h

#include "cmCTestGenericHandler.h"

/** \class cmCTestSubmitHandler
 * \brief Helper class for CTest
 *
 * Submit testing results
 * 
 */
class cmCTestSubmitHandler : public cmCTestGenericHandler
{
public:
  cmTypeMacro(cmCTestSubmitHandler, cmCTestGenericHandler);

  cmCTestSubmitHandler();
  ~cmCTestSubmitHandler() { this->LogFile = 0; }

  /*
   * The main entry point for this class
   */
  int ProcessHandler();

  void Initialize();

  /** Specify a set of parts (by name) to submit.  */
  void SelectParts(std::set<cmCTest::Part> const& parts);
private:
  void SetLogFile(std::ostream* ost) { this->LogFile = ost; }

  /**
   * Submit file using various ways
   */
  bool SubmitUsingFTP(const cmStdString& localprefix, 
                      const std::set<cmStdString>& files,
                      const cmStdString& remoteprefix, 
                      const cmStdString& url);
  bool SubmitUsingHTTP(const cmStdString& localprefix, 
                       const std::set<cmStdString>& files,
                       const cmStdString& remoteprefix, 
                       const cmStdString& url);
  bool SubmitUsingSCP(const cmStdString& scp_command,
                      const cmStdString& localprefix, 
                      const std::set<cmStdString>& files,
                      const cmStdString& remoteprefix, 
                      const cmStdString& url);

  bool TriggerUsingHTTP(const std::set<cmStdString>& files,
                        const cmStdString& remoteprefix, 
                        const cmStdString& url);

  bool SubmitUsingXMLRPC(const cmStdString& localprefix, 
                       const std::set<cmStdString>& files,
                       const cmStdString& remoteprefix, 
                       const cmStdString& url);

  std::string GetSubmitResultsPrefix();

  cmStdString   HTTPProxy;
  int           HTTPProxyType;
  cmStdString   HTTPProxyAuth;
  cmStdString   FTPProxy;
  int           FTPProxyType;
  std::ostream* LogFile;
  bool SubmitPart[cmCTest::PartCount];
  bool CDash;
};

#endif
