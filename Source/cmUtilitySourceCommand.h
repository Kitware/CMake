/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) 2000 National Library of Medicine
  All rights reserved.

  See COPYRIGHT.txt for copyright details.

=========================================================================*/
#ifndef cmUtilitySourceCommand_h
#define cmUtilitySourceCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmUtilitySourceCommand
 * \brief A command to setup a cache entry with the location of a third-party
 * utility's source.
 *
 * cmUtilitySourceCommand is used when a third-party utility's source is
 * included in the project's source tree.  It specifies the location of
 * the executable's source, and any files that may be needed to confirm the
 * identity of the source.
 */
class cmUtilitySourceCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmUtilitySourceCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool Invoke(std::vector<std::string>& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "UTILITY_SOURCE";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Specify the source tree of a third-party utility.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "UTILITY_SOURCE(cache_entry executable_name path_to_source [file1 file2 ...])\n"
      "When a third-party utility's source is included in the distribution,\n"
      "this command specifies its location and name.  The cache entry will\n"
      "not be set unless the path_to_source and all listed files exist.  It\n"
      "is assumed that the source tree of the utility will have been built\n"
      "before it is needed.";
    }
  
  cmTypeMacro(cmUtilitySourceCommand, cmCommand);
};



#endif
