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
#ifndef cmCabilDefineSetCommand_h
#define cmCabilDefineSetCommand_h

#include "cmStandardIncludes.h"
#include "cmCabilCommand.h"

/** \class cmCabilDefineSetCommand
 * \brief Define a command that adds a CABIL Set definition.
 *
 * cmCabilDefineSetCommand is used to define a named CABIL Set.
 * The set can be referenced in other CABIL command arguments
 * with a '$' followed by the set name.
 */
class cmCabilDefineSetCommand : public cmCabilCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
      return new cmCabilDefineSetCommand;
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
  virtual bool IsInherited() 
    {return true;}

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "CABIL_DEFINE_SET";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Define a CABIL Set.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "CABIL_DEFINE_SET(name_of_set member1 member2 ...)";
    }

  virtual void WriteConfiguration(std::ostream&) const;
  
  cmTypeMacro(cmCabilDefineSetCommand, cmCabilCommand);
private:  
  typedef std::vector<std::string>  Elements;
  
  /**
   * The name of the set.
   */
  std::string m_SetName;
  
  /**
   * The elements to be defined in the set (before $ expansion).
   */
  Elements  m_Elements;
};



#endif
