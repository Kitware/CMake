/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmInstallFilesCommand_h
#define cmInstallFilesCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmInstallFilesCommand
 * \brief Specifies where to install some files
 *
 * cmInstallFilesCommand specifies the relative path where a list of
 * files should be installed.  
 */
class cmInstallFilesCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmInstallFilesCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "INSTALL_FILES";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Create install rules for files";
    }
  
  /**
   * This is called at the end after all the information
   * specified by the command is accumulated. Most commands do
   * not implement this method.  At this point, reading and
   * writing to the cache can be done.
   */
  virtual void FinalPass();

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "INSTALL_FILES(path extension file file ...)\n"
      "INSTALL_FILES(path regexp)\n"
      "Create rules to install the listed files into the path. Path is relative to the variable CMAKE_INSTALL_PREFIX. There are two forms for this command. In the first the files can be specified explicitly. If a file specified already has an extension, that extension will be removed first. This is useful for providing lists of source files such as foo.cxx when you want the corresponding foo.h to be installed. A typical extension is .h etc... In the second form any files in the current directory that match the regular expression will be installed.";
    }
  
  cmTypeMacro(cmInstallFilesCommand, cmCommand);

 private:
  std::string m_TargetName;
  std::vector<std::string> m_FinalArgs;
};


#endif
