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
#ifndef cmLocalUnixMakefileGenerator2_h
#define cmLocalUnixMakefileGenerator2_h

#include "cmLocalUnixMakefileGenerator.h"

class cmDependInformation;
class cmMakeDepend;
class cmTarget;
class cmSourceFile;

/** \class cmLocalUnixMakefileGenerator2
 * \brief Write a LocalUnix makefiles.
 *
 * cmLocalUnixMakefileGenerator2 produces a LocalUnix makefile from its
 * member m_Makefile.
 */
class cmLocalUnixMakefileGenerator2 : public cmLocalUnixMakefileGenerator
{
public:
  ///! Set cache only and recurse to false by default.
  cmLocalUnixMakefileGenerator2();

  virtual ~cmLocalUnixMakefileGenerator2();

  /**
   * Generate the makefile for this directory. fromTheTop indicates if this
   * is being invoked as part of a global Generate or specific to this
   * directory. The difference is that when done from the Top we might skip
   * some steps to save time, such as dependency generation for the
   * makefiles. This is done by a direct invocation from make.
   */
  virtual void Generate(bool fromTheTop);

  /** Called from command-line hook to scan dependencies.  */
  static bool ScanDependencies(std::vector<std::string> const& args);

protected:

  void GenerateMakefile();
  void GenerateCMakefile();
  void GenerateTargetRuleFile(const cmTarget& target);
  void GenerateObjectRuleFile(const cmTarget& target,
                              const cmSourceFile& source);
  void WriteDisclaimer(std::ostream& os);

  std::string GetTargetDirectory(const cmTarget& target);
  std::string GetObjectFileName(const cmSourceFile& source);

  static bool ScanDependenciesC(const char* objFile, const char* srcFile,
                                std::vector<std::string> const& includes);
private:
};

#endif
