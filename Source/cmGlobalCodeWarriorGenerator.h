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
#ifndef cmGlobalCodeWarriorGenerator_h
#define cmGlobalCodeWarriorGenerator_h

#include "cmGlobalGenerator.h"

class cmTarget;

/** \class cmGlobalCodeWarriorGenerator
 * \brief Write a Unix makefiles.
 *
 * cmGlobalCodeWarriorGenerator manages UNIX build process for a tree
 */
class cmGlobalCodeWarriorGenerator : public cmGlobalGenerator
{
public:
  ///! Get the name for the generator.
  virtual const char* GetName() {
    return cmGlobalCodeWarriorGenerator::GetActualName();}
  static const char* GetActualName() {return "Code Warrior";}

  ///! Create a local generator appropriate to this Global Generator
  virtual cmLocalGenerator *CreateLocalGenerator();

  /**
   * Try to determine system infomation such as shared library
   * extension, pthreads, byte order etc.  
   */
  virtual void EnableLanguage(const char*, cmMakefile *mf);

  /**
   * Try running cmake and building a file. This is used for dynalically
   * loaded commands, not as part of the usual build process.
   */
  virtual int TryCompile(const char *srcdir, const char *bindir,
                         const char *projectName, const char *targetName,
                         std::string *output);

  /**
   * Generate the all required files for building this project/tree. This
   * basically creates a series of LocalGenerators for each directory and
   * requests that they Generate.  
   */
  virtual void Generate();

  /**
   * Generate the required files for building this directory. This
   * basically creates a single LocalGenerators and
   * requests that it Generate.  
   */
  virtual void LocalGenerate();

  /**
   * Generate the DSW workspace file.
   */
  virtual void OutputProject();

private:
  cmTarget *GetTargetFromName(const char *tgtName);
  void WriteProject(std::ostream & fout);
  void WriteProjectHeader(std::ostream & fout);
  void WriteTargetList(std::ostream & fout);
  void WriteTargetOrder(std::ostream & fout);
  void WriteGroupList(std::ostream & fout);
  void ComputeTargetOrder(std::vector<std::string> &tgtOrder,
                          const char *tgtName, cmTarget const *tgt);
};

#endif
