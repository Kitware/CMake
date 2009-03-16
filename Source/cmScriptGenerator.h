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
#ifndef cmScriptGenerator_h
#define cmScriptGenerator_h

#include "cmStandardIncludes.h"

class cmScriptGeneratorIndent
{
public:
  cmScriptGeneratorIndent(): Level(0) {}
  cmScriptGeneratorIndent(int level): Level(level) {}
  void Write(std::ostream& os) const
    {
    for(int i=0; i < this->Level; ++i)
      {
      os << " ";
      }
    }
  cmScriptGeneratorIndent Next(int step = 2) const
    {
    return cmScriptGeneratorIndent(this->Level + step);
    }
private:
  int Level;
};
inline std::ostream& operator<<(std::ostream& os,
                                cmScriptGeneratorIndent const& indent)
{
  indent.Write(os);
  return os;
}

/** \class cmScriptGenerator
 * \brief Support class for generating install and test scripts.
 *
 */
class cmScriptGenerator
{
public:
  cmScriptGenerator(const char* config_var,
                    std::vector<std::string> const& configurations);
  virtual ~cmScriptGenerator();

  void Generate(std::ostream& os, const char* config,
                std::vector<std::string> const& configurationTypes);

  const std::vector<std::string>& GetConfigurations() const
    { return this->Configurations; }

protected:
  typedef cmScriptGeneratorIndent Indent;
  virtual void GenerateScript(std::ostream& os);
  virtual void GenerateScriptConfigs(std::ostream& os, Indent const& indent);
  virtual void GenerateScriptActions(std::ostream& os, Indent const& indent);
  virtual void GenerateScriptForConfig(std::ostream& os,
                                       const char* config,
                                       Indent const& indent);

  // Test if this generator does something for a given configuration.
  bool GeneratesForConfig(const char*);

  std::string CreateConfigTest(const char* config);
  std::string CreateConfigTest(std::vector<std::string> const& configs);
  std::string CreateComponentTest(const char* component);

  // Information shared by most generator types.
  std::string RuntimeConfigVariable;
  std::vector<std::string> const Configurations;

  // Information used during generation.
  const char* ConfigurationName;
  std::vector<std::string> const* ConfigurationTypes;

  // True if the subclass needs to generate an explicit rule for each
  // configuration.  False if the subclass only generates one rule for
  // all enabled configurations.
  bool ActionsPerConfig;

private:
  void GenerateScriptActionsOnce(std::ostream& os, Indent const& indent);
  void GenerateScriptActionsPerConfig(std::ostream& os, Indent const& indent);
};

#endif
