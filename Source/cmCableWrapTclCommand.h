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
#ifndef cmCableWrapTclCommand_h
#define cmCableWrapTclCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"
#include "cmCableClassSet.h"

class cmMakeDepend;

/** \class cmCableWrapTclCommand
 * \brief Define a command that wraps a set of classes in Tcl.
 */
class cmCableWrapTclCommand : public cmCommand
{
public:
  cmCableWrapTclCommand();
  virtual ~cmCableWrapTclCommand();

  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
      return new cmCableWrapTclCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);  
  
  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "CABLE_WRAP_TCL";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Wrap a set of classes in Tcl.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "CABLE_WRAP_TCL(target class1 class2 ...)\n"
      "Wrap the given set of classes in Tcl using the CABLE tool.  The set\n"
      "of source files produced for the given package name will be added to\n"
      "a source list with the given name.";
    }
  
  cmTypeMacro(cmCableWrapTclCommand, cmCommand);

protected:
  void GenerateCableFiles() const;
  void GenerateCableClassFiles(const char*, const cmCableClass&, const char*) const;
  std::string GetGccXmlFromCache() const;
  std::string GetGccXmlFlagsFromCache() const;
  std::string GetCableFromCache() const;
  void AddGccXmlFlagsFromCache(std::vector<std::string>&) const;
  
  class cmGccXmlFlagsParser;

private:
  /**
   * The name of the package of wrappers to produce.
   */
  std::string m_TargetName;
  
  /**
   * The name of the source list into which the files needed for the package
   * will be placed.
   */
  std::string m_SourceListName;
  
  /**
   * The set of classes to be wrapped in the package.  This is also implicitly
   * added to the makefile as another set.
   */
  cmCableClassSet* m_CableClassSet;

  ///! The dependency generator.
  cmMakeDepend* m_MakeDepend;
};

#endif
