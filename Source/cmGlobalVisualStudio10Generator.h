/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGlobalVisualStudio10Generator_h
#define cmGlobalVisualStudio10Generator_h

#include "cmGlobalVisualStudio8Generator.h"


/** \class cmGlobalVisualStudio10Generator
 * \brief Write a Unix makefiles.
 *
 * cmGlobalVisualStudio10Generator manages UNIX build process for a tree
 */
class cmGlobalVisualStudio10Generator : 
  public cmGlobalVisualStudio8Generator
{
public:
  cmGlobalVisualStudio10Generator();
  static cmGlobalGenerator* New() { 
    return new cmGlobalVisualStudio10Generator; }
  
  virtual std::string 
  GenerateBuildCommand(const char* makeProgram,
                       const char *projectName, 
                       const char* additionalOptions, const char *targetName,
                       const char* config, bool ignoreErrors, bool);
  
  ///! Get the name for the generator.
  virtual const char* GetName() const {
    return cmGlobalVisualStudio10Generator::GetActualName();}
  static const char* GetActualName() {return "Visual Studio 10";}
  virtual void AddPlatformDefinitions(cmMakefile* mf);
  
  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry& entry) const;

  ///! create the correct local generator
  virtual cmLocalGenerator *CreateLocalGenerator();

  /**
   * Try to determine system infomation such as shared library
   * extension, pthreads, byte order etc.  
   */
  virtual void EnableLanguage(std::vector<std::string>const& languages, 
                              cmMakefile *, bool optional);
  virtual void WriteSLNHeader(std::ostream& fout);

  /**
   * Where does this version of Visual Studio look for macros for the
   * current user? Returns the empty string if this version of Visual
   * Studio does not implement support for VB macros.
   */
  virtual std::string GetUserMacrosDirectory();

  /**
   * What is the reg key path to "vsmacros" for this version of Visual
   * Studio?
   */
  virtual std::string GetUserMacrosRegKeyBase();
  virtual const char* GetCMakeCFGInitDirectory() 
    { return "$(Configuration)";}
protected:
  virtual const char* GetIDEVersion() { return "10.0"; }
};
#endif
