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
#ifndef cmCablePackageCommand_h
#define cmCablePackageCommand_h

#include "cmStandardIncludes.h"
#include "cmCableCommand.h"

/** \class cmCablePackageCommand
 * \brief Define a command that begins a CABLE Package definition.
 *
 * cmCablePackageCommand is used to generate a new CABLE Package.
 * All subsequent commands that require a package will refer to that
 * setup by this command, until another package is started.
 */
class cmCablePackageCommand : public cmCableCommand
{
public:
  cmCablePackageCommand() {}
  virtual ~cmCablePackageCommand() {}

  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
      return new cmCablePackageCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool Invoke(std::vector<std::string>& args);  

  /**
   * This is called at the end after all the information
   * specified by the command is accumulated. Most commands do
   * not implement this method.  At this point, reading and
   * writing to the cache can be done.
   */
  virtual void FinalPass();
  
  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "CABLE_PACKAGE";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Begin a package definition.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "CABLE_PACKAGE(package_name)\n"
      "Close current package (if any), and open a new package definition.";
    }  

  void WritePackageHeader() const;
  void WritePackageFooter() const;  
  
  cmTypeMacro(cmCablePackageCommand, cmCableCommand);
private:
  /**
   * The name of the package.
   */
  std::string m_PackageName;  
};



#endif
