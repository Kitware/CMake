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
#ifndef cmUnixDefinesCommand_h
#define cmUnixDefinesCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmUnixDefinesCommand
 * \brief Specify a list of compiler defines for Unix platforms.
 *
 * cmUnixDefinesCommand specifies a list of compiler defines for Unix platforms
 * only. This defines will be added to the compile command.
 */
class cmUnixDefinesCommand : public cmCommand
{
public:
  /**
   * Constructor.
   */
  cmUnixDefinesCommand();

  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmUnixDefinesCommand;
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
  virtual const char* GetName() { return "UNIX_DEFINES";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Add -D flags to the command line for Unix only.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "UNIX_DEFINES(-DFOO -DBAR)\n"
      "Add -D flags to the command line for Unix only.";
    }
  
  cmTypeMacro(cmUnixDefinesCommand, cmCommand);
};



#endif
