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
#ifndef cmConfigureFileNoAutoconf_h
#define cmConfigureFileNoAutoconf_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

class cmConfigureFileNoAutoconf : public cmCommand
{
public:
  virtual cmCommand* Clone() 
    {
      return new cmConfigureFileNoAutoconf;
    }

  /**
   * This is called when the command is first encountered in
   * the input file.
   */
  virtual bool Invoke(std::vector<std::string>& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "CONFIGURE_FILE_NOAUTOCONF";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Create a header file from an autoconf style header.h.in file.";
    }
  
  /**
   * Longer documentation.
   */
  virtual const char* GetFullDocumentation()
    {
      return
        "CONFIGURE_HEADER(InputFile OutputFile)\n"
	"The Input and Ouput files have to have full paths.\n"
	"They can also use variables like CMAKE_BINARY_DIR,CMAKE_SOURCE_DIR.\n"
        "This command is only run if autoconf was not used.\n";
    }

  /**
   * Create the header files in this pass.  This is so
   * all varibles can be expaned.
   */
  virtual void FinalPass();
private:
  std::string m_InputFile;
  std::string m_OuputFile;
};



#endif
