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
#ifndef cmCreateTestSourceList_h
#define cmCreateTestSourceList_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmCreateTestSourceList
 * \brief 
 *
 */

class cmCreateTestSourceList : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmCreateTestSourceList;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * This determines if the command gets propagated down
   * to makefiles located in subdirectories.
   */
  virtual bool IsInherited() {return true;}

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() {return "CREATE_TEST_SOURCELIST";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Create a test driver and source list for building test programs.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "CREATE_TEST_SOURCELIST(SourceListName DriverName test1 test2 test3"
      "The list of source files needed to build the testdriver will be in SourceListName.\n"
      "DriverName.cxx is the name of the test driver program.\n"
      "The rest of the arguments consist of a list of test source files, can be "
      "; separated.  Each test source file should have a function in it that "
      "is the same name as the file with no extension (foo.cxx should have int foo();) "
      "DriverName.cxx will be able to call each of the tests by name on the command line.";
    }
  
  cmTypeMacro(cmCreateTestSourceList, cmCommand);
};



#endif
