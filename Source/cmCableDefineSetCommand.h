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
#ifndef cmCableDefineSetCommand_h
#define cmCableDefineSetCommand_h

#include "cmStandardIncludes.h"
#include "cmCableCommand.h"

/** \class cmCableDefineSetCommand
 * \brief Define a command that adds a CABLE Set definition.
 *
 * cmCableDefineSetCommand is used to define a named CABLE Set.
 * The set can be referenced in other CABLE command arguments
 * with a '$' followed by the set name.
 */
class cmCableDefineSetCommand : public cmCableCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
      return new cmCableDefineSetCommand;
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
    { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "CABLE_DEFINE_SET";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Define a CABLE Set.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "CABLE_DEFINE_SET(name_of_set [[tag1]:]memeber1 [[tag2]:]member2 ...\n"
      "                 [SOURCE_FILES source1 source2 ...]] )\n"
      "Generates a Set definition in the CABLE configuration.  The sets are\n"
      "referenced in other CABLE commands by a '$' immediately followed by\n"
      "the set name (ex. $SetName).  If a the \"tag:\" syntax is not used,\n"
      "an attempt is made to auto-generate a meaningful tag.  If the\n"
      "SOURCE_FILES keyword is given, all arguments after it refer to header\n"
      "files to be included in any package referencing the set.\n";
    }

  cmTypeMacro(cmCableDefineSetCommand, cmCableCommand);
  
private:
  void WriteConfiguration() const;
  bool AddElement(const std::string&);
  bool GenerateTag(const std::string&, std::string&);
  bool AddSourceFile(const std::string&);
private:  
  typedef std::pair<std::string, std::string>  Element;
  typedef std::vector<Element>  Elements;
  
  /**
   * The name of the set.
   */
  std::string m_SetName;
  
  /**
   * The elements to be defined in the set (before $ expansion).
   */
  Elements  m_Elements;
  
  /**
   * The source headers associated with this set.
   */
  std::vector<std::string> m_SourceHeaders;
  
  /**
   * The instantiation sources associated with this set.
   */
  std::vector<std::string> m_InstantiationSources;
};



#endif
