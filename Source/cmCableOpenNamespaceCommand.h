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
#ifndef cmCableOpenNamespaceCommand_h
#define cmCableOpenNamespaceCommand_h

#include "cmStandardIncludes.h"
#include "cmCableCommand.h"

/** \class cmCableOpenNamespaceCommand
 * \brief Define a command that opens a CABLE Namespace.
 *
 * cmCableOpenNamespaceCommand is used to generate CABLE Namespace
 * open tags in the configuration file.
 */
class cmCableOpenNamespaceCommand : public cmCableCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
      return new cmCableOpenNamespaceCommand;
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
  virtual const char* GetName() { return "CABLE_OPEN_NAMESPACE";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Open a CABLE Namespace";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "CABLE_OPEN_NAMESPACE(namespace_name)\n"
      "Open the given namespace in the generated configuration file.\n"
      "There must be a matching CABLE_CLOSE_NAMESPACE(namespace_name)\n"
      "called with the same name.";
    }

  cmTypeMacro(cmCableOpenNamespaceCommand, cmCableCommand);
private:
  void WriteNamespaceHeader() const;
private:
  /**
   * The name of the namespace to setup.
   */
  std::string m_NamespaceName;
};



#endif
