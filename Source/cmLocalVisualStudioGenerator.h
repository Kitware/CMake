/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmLocalVisualStudioGenerator_h
#define cmLocalVisualStudioGenerator_h

#include "cmLocalGenerator.h"

#include <cmsys/auto_ptr.hxx>

class cmSourceFile;
class cmSourceGroup;

/** \class cmLocalVisualStudioGenerator
 * \brief Base class for Visual Studio generators.
 *
 * cmLocalVisualStudioGenerator provides functionality common to all
 * Visual Studio generators.
 */
class cmLocalVisualStudioGenerator : public cmLocalGenerator
{
public:
  cmLocalVisualStudioGenerator();
  virtual ~cmLocalVisualStudioGenerator();
  /** Construct a script from the given list of command lines.  */
  std::string ConstructScript(const cmCustomCommandLines& commandLines,
                              const char* workingDirectory,
                              const char* configName,
                              bool escapeOldStyle,
                              bool escapeAllowMakeVars,
                              const char* newline = "\n");

protected:

  /** Construct a custom command to make exe import lib dir.  */
  cmsys::auto_ptr<cmCustomCommand>
  MaybeCreateImplibDir(cmTarget& target, const char* config);

  // Safe object file name generation.
  void ComputeObjectNameRequirements(std::vector<cmSourceGroup> const&);
  bool SourceFileCompiles(const cmSourceFile* sf);
  void CountObjectNames(const std::vector<cmSourceGroup>& groups,
                        std::map<cmStdString, int>& count);
  void InsertNeedObjectNames(const std::vector<cmSourceGroup>& groups,
                             std::map<cmStdString, int>& count);
  std::set<const cmSourceFile*> NeedObjectName;
  friend class cmVisualStudio10TargetGenerator;
};

#endif
