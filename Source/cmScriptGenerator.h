/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <ostream>
#include <string>
#include <vector>

class cmScriptGeneratorIndent
{
public:
  cmScriptGeneratorIndent() = default;
  cmScriptGeneratorIndent(int level)
    : Level(level)
  {
  }
  void Write(std::ostream& os) const
  {
    for (int i = 0; i < this->Level; ++i) {
      os << " ";
    }
  }
  cmScriptGeneratorIndent Next(int step = 2) const
  {
    return { this->Level + step };
  }

private:
  int Level = 0;
};
inline std::ostream& operator<<(std::ostream& os,
                                cmScriptGeneratorIndent indent)
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
  cmScriptGenerator(std::string config_var,
                    std::vector<std::string> configurations);
  virtual ~cmScriptGenerator();

  cmScriptGenerator(cmScriptGenerator const&) = delete;
  cmScriptGenerator& operator=(cmScriptGenerator const&) = delete;

  void Generate(std::ostream& os, const std::string& config,
                std::vector<std::string> const& configurationTypes);

protected:
  using Indent = cmScriptGeneratorIndent;
  virtual void GenerateScript(std::ostream& os);
  virtual void GenerateScriptConfigs(std::ostream& os, Indent indent);
  virtual void GenerateScriptActions(std::ostream& os, Indent indent);
  virtual void GenerateScriptForConfig(std::ostream& os,
                                       const std::string& config,
                                       Indent indent);
  virtual void GenerateScriptNoConfig(std::ostream&, Indent) {}
  virtual bool NeedsScriptNoConfig() const { return false; }

  // Test if this generator does something for a given configuration.
  bool GeneratesForConfig(const std::string&);

  std::string CreateConfigTest(const std::string& config);
  std::string CreateConfigTest(std::vector<std::string> const& configs);
  std::string CreateComponentTest(const std::string& component);

  // Information shared by most generator types.
  std::string RuntimeConfigVariable;
  std::vector<std::string> const Configurations;

  // Information used during generation.
  std::string ConfigurationName;
  std::vector<std::string> const* ConfigurationTypes;

  // True if the subclass needs to generate an explicit rule for each
  // configuration.  False if the subclass only generates one rule for
  // all enabled configurations.
  bool ActionsPerConfig;

private:
  void GenerateScriptActionsOnce(std::ostream& os, Indent indent);
  void GenerateScriptActionsPerConfig(std::ostream& os, Indent indent);
};
