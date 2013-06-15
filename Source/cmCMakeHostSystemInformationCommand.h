/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2013 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCMakeHostSystemInformationCommand_h
#define cmCMakeHostSystemInformationCommand_h

#include "cmCommand.h"

#include <cmsys/SystemInformation.hxx>

/** \class cmCMakeHostSystemInformationCommand
 * \brief Query host system specific information
 *
 * cmCMakeHostSystemInformationCommand queries system information of
 * the sytem on which CMake runs.
 */
class cmCMakeHostSystemInformationCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmCMakeHostSystemInformationCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
               cmExecutionStatus &status);

   /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() const { return true; }

   /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() const
    {
    return "cmake_host_system_information";
    }

   /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Query host system specific information.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
    "  cmake_host_system_information(RESULT <variable> QUERY <key> ...)\n"
    "Queries system information of the host system on which cmake runs. "
    "One or more <key> can be provided to "
    "select the information to be queried. "
    "The list of queried values is stored in <variable>.\n"
    "<key> can be one of the following values:\n"
    "  NUMBER_OF_LOGICAL_CORES   = Number of logical cores.\n"
    "  NUMBER_OF_PHYSICAL_CORES  = Number of physical cores.\n"
    "  HOSTNAME                  = Hostname.\n"
    "  FQDN                      = Fully qualified domain name.\n"
    "  TOTAL_VIRTUAL_MEMORY      = "
      "Total virtual memory in megabytes.\n"
    "  AVAILABLE_VIRTUAL_MEMORY  = "
      "Available virtual memory in megabytes.\n"
    "  TOTAL_PHYSICAL_MEMORY     = "
      "Total physical memory in megabytes.\n"
    "  AVAILABLE_PHYSICAL_MEMORY = "
      "Available physical memory in megabytes.\n"
    ;
    }

  cmTypeMacro(cmCMakeHostSystemInformationCommand, cmCommand);

private:
  bool GetValue(cmsys::SystemInformation &info,
    std::string const& key, std::string &value);

  std::string ValueToString(std::size_t value) const;
  std::string ValueToString(const char *value) const;
  std::string ValueToString(std::string const& value) const;
};

#endif
