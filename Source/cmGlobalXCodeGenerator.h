/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmGlobalXCodeGenerator_h
#define cmGlobalXCodeGenerator_h

#include "cmGlobalGenerator.h"
#include "cmXCodeObject.h"
class cmTarget;

/** \class cmGlobalXCodeGenerator
 * \brief Write a Unix makefiles.
 *
 * cmGlobalXCodeGenerator manages UNIX build process for a tree
 */
class cmGlobalXCodeGenerator : public cmGlobalGenerator
{
public:
  cmGlobalXCodeGenerator();
  static cmGlobalGenerator* New() { return new cmGlobalXCodeGenerator; }
  
  ///! Get the name for the generator.
  virtual const char* GetName() const {
    return cmGlobalXCodeGenerator::GetActualName();}
  static const char* GetActualName() {return "XCode";}

  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry& entry) const;
  
  ///! Create a local generator appropriate to this Global Generator
  virtual cmLocalGenerator *CreateLocalGenerator();

  /**
   * Try to determine system infomation such as shared library
   * extension, pthreads, byte order etc.  
   */
  virtual void EnableLanguage(std::vector<std::string>const& languages, cmMakefile *);
  /**
   * Try running cmake and building a file. This is used for dynalically
   * loaded commands, not as part of the usual build process.
   */
  virtual int TryCompile(const char *srcdir, const char *bindir,
                         const char *projectName, const char *targetName,
                         std::string *output, cmMakefile* mf);

  /**
   * Generate the all required files for building this project/tree. This
   * basically creates a series of LocalGenerators for each directory and
   * requests that they Generate.  
   */
  virtual void Generate();

private:
  cmXCodeObject* CreateObject(cmXCodeObject::PBXType ptype, cmXCodeObject::Type type);
  void ClearXCodeObjects();
  void CreateXCodeObjects(cmLocalGenerator* root,
                          std::vector<cmLocalGenerator*>& generators);
  void OutputXCodeProject(cmLocalGenerator* root,
                          std::vector<cmLocalGenerator*>& generators);
  void  WriteXCodePBXProj(std::ostream& fout,
                          cmLocalGenerator* root,
                          std::vector<cmLocalGenerator*>& generators);
  
  std::vector<cmXCodeObject*> m_XCodeObjects;
  cmXCodeObject* m_RootObject;
};

#endif
