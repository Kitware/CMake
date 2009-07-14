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
#ifndef cmGlobalVisualStudioGenerator_h
#define cmGlobalVisualStudioGenerator_h

#include "cmGlobalGenerator.h"

/** \class cmGlobalVisualStudioGenerator
 * \brief Base class for global Visual Studio generators.
 *
 * cmGlobalVisualStudioGenerator provides functionality common to all
 * global Visual Studio generators.
 */
class cmGlobalVisualStudioGenerator : public cmGlobalGenerator
{
public:
  cmGlobalVisualStudioGenerator();
  virtual ~cmGlobalVisualStudioGenerator();

  /**
   * Basic generate implementation for all VS generators.
   */
  virtual void Generate();

  /**
   * Configure CMake's Visual Studio macros file into the user's Visual
   * Studio macros directory.
   */
  virtual void ConfigureCMakeVisualStudioMacros();

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

  enum MacroName {MacroReload, MacroStop};

  /**
   * Call the ReloadProjects macro if necessary based on
   * GetFilesReplacedDuringGenerate results.
   */
  virtual void CallVisualStudioMacro(MacroName m,
                                     const char* vsSolutionFile = 0);
  
  // return true if target is fortran only
  bool TargetIsFortranOnly(cmTarget& t);
  const char* GetUtilityForTarget(cmTarget& target, const char*);

protected:
  virtual void CreateGUID(const char*) {}
  void FixUtilityDepends();

  // Does this VS version link targets to each other if there are
  // dependencies in the SLN file?  This was done for VS versions
  // below 8.
  virtual bool VSLinksDependencies() const { return true; }

private:
  void FixUtilityDependsForTarget(cmTarget& target);
  void CreateUtilityDependTarget(cmTarget& target);
  bool CheckTargetLinks(cmTarget& target, const char* name);
};

#endif
