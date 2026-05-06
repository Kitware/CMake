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
  cmInstallScriptHandler(std::string, std::vector<std::string>, std::string,
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
    std::vector<std::string> Command;
    std::vector<std::string> Output;
    std::string Name;
    std::unique_ptr<cmUVProcessChain> Chain;
    std::unique_ptr<cmUVStreamReadHandle> StreamHandler;
  };

private:
  std::vector<InstallScript> Scripts;
  std::vector<std::string> Configs;
  std::vector<std::string> Directories;
  std::vector<std::string> Components;
  std::string BinaryDir;
  bool Parallel;
};
