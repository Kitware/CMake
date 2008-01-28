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
#ifndef cmInstallGenerator_h
#define cmInstallGenerator_h

#include "cmStandardIncludes.h"

class cmLocalGenerator;

class cmInstallGeneratorIndent
{
public:
  cmInstallGeneratorIndent(): Level(0) {}
  cmInstallGeneratorIndent(int level): Level(level) {}
  void Write(std::ostream& os) const
    {
    for(int i=0; i < this->Level; ++i)
      {
      os << " ";
      }
    }
  cmInstallGeneratorIndent Next(int step = 2) const
    {
    return cmInstallGeneratorIndent(this->Level + step);
    }
private:
  int Level;
};
inline std::ostream& operator<<(std::ostream& os,
                                cmInstallGeneratorIndent const& indent)
{
  indent.Write(os);
  return os;
}

/** \class cmInstallGenerator
 * \brief Support class for generating install scripts.
 *
 */
class cmInstallGenerator
{
public:
  cmInstallGenerator(const char* destination,
                     std::vector<std::string> const& configurations,
                     const char* component);
  virtual ~cmInstallGenerator();

  void Generate(std::ostream& os, const char* config,
                std::vector<std::string> const& configurationTypes);

  void AddInstallRule(
    std::ostream& os, int type,
    std::vector<std::string> const& files,
    bool optional = false,
    const char* properties = 0,
    const char* permissions_file = 0,
    const char* permissions_dir = 0,
    const char* rename = 0,
    const char* literal_args = 0,
    cmInstallGeneratorIndent const& indent = cmInstallGeneratorIndent()
    );

  const char* GetDestination() const
    { return this->Destination.c_str(); }
  const std::vector<std::string>& GetConfigurations() const
    { return this->Configurations; }

  /** Get the install destination as it should appear in the
      installation script.  */
  std::string GetInstallDestination() const;

  /** Test if this generator installs something for a given configuration.  */
  bool InstallsForConfig(const char*);

protected:
  typedef cmInstallGeneratorIndent Indent;
  virtual void GenerateScript(std::ostream& os);
  virtual void GenerateScriptConfigs(std::ostream& os, Indent const& indent);
  virtual void GenerateScriptActions(std::ostream& os, Indent const& indent);

  std::string CreateConfigTest(const char* config);
  std::string CreateConfigTest(std::vector<std::string> const& configs);
  std::string CreateComponentTest(const char* component);

  // Information shared by most generator types.
  std::string Destination;
  std::vector<std::string> const Configurations;
  std::string Component;

  // Information used during generation.
  const char* ConfigurationName;
  std::vector<std::string> const* ConfigurationTypes;
};

#endif
