/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "cmUVProcessChain.h"
#include "cmUVStream.h"

namespace cm {
class uv_loop_ptr;
}

class cmInstrumentation;

class cmInstallScriptHandler
{
public:
  cmInstallScriptHandler() = default;
  cmInstallScriptHandler(std::string, std::string, std::string,
                         std::vector<std::string>&);
  bool IsParallel();
  int Install(unsigned int j, cmInstrumentation& instrumentation);
  struct InstallScript
  {
    std::string path;
    std::string config;
    std::vector<std::string> command;
  };
  std::vector<InstallScript> GetScripts() const;
  class InstallScriptRunner
  {
  public:
    InstallScriptRunner(InstallScript const&);
    void start(cm::uv_loop_ptr&, std::function<void()>);
    void printResult(std::size_t n, std::size_t total);

  private:
    std::vector<std::string> command;
    std::vector<std::string> output;
    std::string name;
    std::unique_ptr<cmUVProcessChain> chain;
    std::unique_ptr<cmUVStreamReadHandle> streamHandler;
  };

private:
  std::vector<InstallScript> scripts;
  std::vector<std::string> configs;
  std::vector<std::string> directories;
  std::string binaryDir;
  std::string component;
  bool parallel;
};
