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

#ifndef cmInstallCommandArguments_h
#define cmInstallCommandArguments_h

#include "cmStandardIncludes.h"
#include "cmCommandArgumentsHelper.h"

class cmInstallCommandArguments
{
  public:
    cmInstallCommandArguments();
    void SetGenericArguments(cmInstallCommandArguments* args) 
                                               {this->GenericArguments = args;}
    void Parse(const std::vector<std::string>* args, 
               std::vector<std::string>* unconsumedArgs);

    // Compute destination path.and check permissions
    bool Finalize();

    const std::string& GetDestination() const;
    const std::string& GetComponent() const;
    const std::string& GetRename() const;
    const std::string& GetPermissions() const;
    const std::vector<std::string>& GetConfigurations() const;
    bool GetOptional() const;

    // once HandleDirectoryMode() is also switched to using 
    // cmInstallCommandArguments then these two functions can become non-static
    // private member functions without arguments
    static bool CheckPermissions(const std::string& onePerm, 
                                 std::string& perm);
    cmCommandArgumentsHelper Parser;
    cmCommandArgumentGroup ArgumentGroup;
  private:
    cmCAString Destination;
    cmCAString Component;
    cmCAString Rename;
    cmCAStringVector Permissions;
    cmCAStringVector Configurations;
    cmCAEnabler Optional;

    std::string DestinationString;
    std::string PermissionsString;

    cmInstallCommandArguments* GenericArguments;
    static const char* PermissionsTable[];
    static const std::string EmptyString;
    bool CheckPermissions();
};

#endif
