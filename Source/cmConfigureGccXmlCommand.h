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
#ifndef cmConfigureGccXmlCommand_h
#define cmConfigureGccXmlCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmConfigureGccXmlCommand
 * \brief Define a command that configures flags for GCC-XML to run.
 */
class cmConfigureGccXmlCommand : public cmCommand
{
public:
  cmConfigureGccXmlCommand();
  virtual ~cmConfigureGccXmlCommand();

  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
      return new cmConfigureGccXmlCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);  
  
  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "CONFIGURE_GCCXML";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Configure the flags needed for GCC-XML to run.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "CONFIGURE_GCCXML(exe_location flags_def)\n"
      "Configures the flags GCC-XML needs to parse source code just as\n"
      "the current compiler would.  This includes using the compiler's\n"
      "standard header files.  First argument is input of the full path to\n"
      "the GCC-XML executable.  The second argument should be the name of\n"
      "a cache entry to set with the flags chosen.\n";
    }
  
  cmTypeMacro(cmConfigureGccXmlCommand, cmCommand);
  
protected:
  bool GetSupportDirectory(const char*);
  bool FindVcIncludeFlags();
  bool FindGccIncludeFlags();
  bool FindMproIncludeFlags();
  bool CompilerIsGCC() const;
  bool CompilerIsMipsPro() const;
  
private:
  std::string m_SupportDir;
  std::string m_Flags;
};

#endif
