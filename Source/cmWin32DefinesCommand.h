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
#ifndef cmWin32DefinesCommand_h
#define cmWin32DefinesCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmWin32DefinesCommand
 * \brief Specify a list of compiler defines for Win32 platforms.
 *
 * cmWin32DefinesCommand specifies a list of compiler defines for Win32 platforms
 * only. This defines will be added to the compile command.
 */
class cmWin32DefinesCommand : public cmCommand
{
public:
  /**
   * Constructor.
   */
  cmWin32DefinesCommand();

  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmWin32DefinesCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool Invoke(std::vector<std::string>& args);

  /**
   * This determines if the command gets propagated down
   * to makefiles located in subdirectories.
   */
  virtual bool IsInherited() {return true;}

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() {return "WIN32_DEFINES";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Add -D define flags to command line for Win32 environments.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "WIN32_DEFINES(-DFOO -DBAR ...)\n"
      "Add -D define flags to command line for Win32 environments.";
    }
  
  cmTypeMacro(cmWin32DefinesCommand, cmCommand);
};



#endif
