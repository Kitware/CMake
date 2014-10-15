/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmSearchPath_h
#define cmSearchPath_h

#include "cmStandardIncludes.h"

class cmFindCommon;

/** \class cmSearchPath
 * \brief Base class for FIND_XXX implementations.
 *
 * cmSearchPath is a container that encapsulates search path construction and
 * management
 */
class cmSearchPath
{
public:
  cmSearchPath(cmFindCommon* findCmd, const std::string& groupLabel);
  ~cmSearchPath();

  const std::string& GetLabel() const { return this->Label; }
  const std::vector<std::string>& GetPaths() const { return this->Paths; }

  void ExtractWithout(const std::set<std::string>& ignore,
                      std::vector<std::string>& outPaths,
                      bool clear = false) const;

  void AddPath(const std::string& path);
  void AddUserPath(const std::string& path);
  void AddCMakePath(const std::string& variable);
  void AddEnvPath(const std::string& variable);
  void AddCMakePrefixPath(const std::string& variable);
  void AddEnvPrefixPath(const std::string& variable);
  void AddSuffixes(const std::vector<std::string>& suffixes);

protected:
  void AddPrefixPaths(const std::vector<std::string>& paths,
                      const char *base = 0);
  void AddPathInternal(const std::string& path, const char *base = 0);

  // Members collected from the calling find command
  const std::string& FindName;
  cmMakefile* const& Makefile;
  std::set<std::string>& Emitted;

  std::string Label;
  std::vector<std::string> Paths;
};

#endif
