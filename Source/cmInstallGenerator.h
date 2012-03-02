/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmInstallGenerator_h
#define cmInstallGenerator_h

#include "cmInstallType.h"
#include "cmScriptGenerator.h"

class cmLocalGenerator;

/** \class cmInstallGenerator
 * \brief Support class for generating install scripts.
 *
 */
class cmInstallGenerator: public cmScriptGenerator
{
public:
  cmInstallGenerator(const char* destination,
                     std::vector<std::string> const& configurations,
                     const char* component);
  virtual ~cmInstallGenerator();

  void AddInstallRule(
    std::ostream& os, cmInstallType type,
    std::vector<std::string> const& files,
    bool optional = false,
    const char* permissions_file = 0,
    const char* permissions_dir = 0,
    const char* rename = 0,
    const char* literal_args = 0,
    Indent const& indent = Indent()
    );

  const char* GetDestination() const
    { return this->Destination.c_str(); }

  /** Get the install destination as it should appear in the
      installation script.  */
  std::string GetInstallDestination() const;

  /** Test if this generator installs something for a given configuration.  */
  bool InstallsForConfig(const char*);

protected:
  virtual void GenerateScript(std::ostream& os);

  std::string CreateComponentTest(const char* component);

  // Information shared by most generator types.
  std::string Destination;
  std::string Component;
};

#endif
