/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCustomCommandGenerator_h
#define cmCustomCommandGenerator_h

#include "cmConfigure.h" // IWYU pragma: keep
#include "cmCustomCommandLines.h"

#include <string>
#include <vector>

class cmCustomCommand;
class cmGeneratorExpression;
class cmLocalGenerator;

class cmCustomCommandGenerator
{
  cmCustomCommand const& CC;
  std::string Config;
  cmLocalGenerator* LG;
  bool OldStyle;
  bool MakeVars;
  cmGeneratorExpression* GE;
  cmCustomCommandLines CommandLines;
  std::vector<std::string> Depends;
  std::string WorkingDirectory;

  const char* GetCrossCompilingEmulator(unsigned int c) const;
  const char* GetArgv0Location(unsigned int c) const;

public:
  cmCustomCommandGenerator(cmCustomCommand const& cc, std::string config,
                           cmLocalGenerator* lg);
  ~cmCustomCommandGenerator();
  cmCustomCommandGenerator(const cmCustomCommandGenerator&) = delete;
  cmCustomCommandGenerator& operator=(const cmCustomCommandGenerator&) =
    delete;
  cmCustomCommand const& GetCC() const { return this->CC; }
  unsigned int GetNumberOfCommands() const;
  std::string GetCommand(unsigned int c) const;
  void AppendArguments(unsigned int c, std::string& cmd) const;
  const char* GetComment() const;
  std::string GetWorkingDirectory() const;
  std::vector<std::string> const& GetOutputs() const;
  std::vector<std::string> const& GetByproducts() const;
  std::vector<std::string> const& GetDepends() const;
  bool HasOnlyEmptyCommandLines() const;
};

#endif
