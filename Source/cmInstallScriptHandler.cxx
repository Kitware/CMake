/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include "cmInstallScriptHandler.h"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <cm/memory>

#include <cm3p/json/reader.h>
#include <cm3p/json/value.h>
#include <cm3p/uv.h>

#include "cmsys/FStream.hxx"
#include "cmsys/RegularExpression.hxx"

#include "cmCryptoHash.h"
#include "cmGeneratedFileStream.h"
#include "cmInstrumentation.h"
#include "cmJSONState.h"
#include "cmProcessOutput.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmUVHandlePtr.h"
#include "cmUVProcessChain.h"
#include "cmUVStream.h"

using InstallScript = cmInstallScriptHandler::InstallScript;

cmInstallScriptHandler::cmInstallScriptHandler(std::string _binaryDir,
                                               std::string _component,
                                               std::string _config,
                                               std::vector<std::string>& args)
  : binaryDir(std::move(_binaryDir))
  , component(std::move(_component))
{
  std::string const& file =
    cmStrCat(this->binaryDir, "/CMakeFiles/InstallScripts.json");
  this->parallel = false;

  auto addScript = [this, &args](std::string script,
                                 std::string config) -> void {
    this->commands.push_back(args);
    if (!config.empty()) {
      this->commands.back().insert(
        this->commands.back().end() - 1,
        cmStrCat("-DCMAKE_INSTALL_CONFIG_NAME=", config));
    }
    this->commands.back().emplace_back(script);
    this->directories.push_back(cmSystemTools::GetFilenamePath(script));
  };

  int compare = 1;
  if (cmSystemTools::FileExists(file)) {
    cmSystemTools::FileTimeCompare(
      cmStrCat(this->binaryDir, "/CMakeFiles/cmake.check_cache"), file,
      &compare);
  }
  if (compare < 1) {
    Json::CharReaderBuilder rbuilder;
    auto JsonReader =
      std::unique_ptr<Json::CharReader>(rbuilder.newCharReader());
    std::vector<char> content;
    Json::Value value;
    cmJSONState state(file, &value);
    this->parallel = value["Parallel"].asBool();
    if (this->parallel) {
      args.insert(args.end() - 1, "-DCMAKE_INSTALL_LOCAL_ONLY=1");
    }
    if (_config.empty() && value.isMember("Configs")) {
      for (auto const& config : value["Configs"]) {
        this->configs.push_back(config.asCString());
      }
    } else {
      this->configs.push_back(_config);
    }
    for (auto const& script : value["InstallScripts"]) {
      for (auto const& config : configs) {
        addScript(script.asCString(), config);
      }
      if (!this->parallel) {
        break;
      }
    }
  } else {
    addScript(cmStrCat(this->binaryDir, "/cmake_install.cmake"), _config);
  }
}

bool cmInstallScriptHandler::IsParallel()
{
  return this->parallel;
}

std::vector<std::vector<std::string>> cmInstallScriptHandler::GetCommands()
  const
{
  return this->commands;
}

int cmInstallScriptHandler::Install(unsigned int j,
                                    cmInstrumentation& instrumentation)
{
  cm::uv_loop_ptr loop;
  loop.init();
  std::vector<InstallScript> scripts;
  scripts.reserve(this->commands.size());

  std::vector<std::string> instrument_arg;
  if (instrumentation.HasQuery()) {
    instrument_arg = { cmSystemTools::GetCTestCommand(),
                       "--instrument",
                       "--command-type",
                       "install",
                       "--build-dir",
                       this->binaryDir,
                       "--" };
  }

  for (auto& cmd : this->commands) {
    cmd.insert(cmd.begin(), instrument_arg.begin(), instrument_arg.end());
    scripts.emplace_back(cmd);
  }
  std::size_t working = 0;
  std::size_t installed = 0;
  std::size_t i = 0;

  std::function<void()> queueScripts;
  queueScripts = [&scripts, &working, &installed, &i, &loop, j,
                  &queueScripts]() {
    for (auto queue = std::min(j - working, scripts.size() - i); queue > 0;
         --queue) {
      ++working;
      scripts[i].start(loop,
                       [&scripts, &working, &installed, i, &queueScripts]() {
                         scripts[i].printResult(++installed, scripts.size());
                         --working;
                         queueScripts();
                       });
      ++i;
    }
  };
  queueScripts();
  uv_run(loop, UV_RUN_DEFAULT);

  // Write install manifest
  std::string install_manifest;
  if (this->component.empty()) {
    install_manifest = "install_manifest.txt";
  } else {
    cmsys::RegularExpression regEntry;
    if (regEntry.compile("^[a-zA-Z0-9_.+-]+$") &&
        regEntry.find(this->component)) {
      install_manifest =
        cmStrCat("install_manifest_", this->component, ".txt");
    } else {
      cmCryptoHash md5(cmCryptoHash::AlgoMD5);
      md5.Initialize();
      install_manifest =
        cmStrCat("install_manifest_", md5.HashString(this->component), ".txt");
    }
  }
  cmGeneratedFileStream fout(cmStrCat(this->binaryDir, "/", install_manifest));
  fout.SetCopyIfDifferent(true);
  for (auto const& dir : this->directories) {
    auto local_manifest = cmStrCat(dir, "/install_local_manifest.txt");
    if (cmSystemTools::FileExists(local_manifest)) {
      cmsys::ifstream fin(local_manifest.c_str());
      std::string line;
      while (std::getline(fin, line)) {
        fout << line << "\n";
      }
    }
  }
  return 0;
}

InstallScript::InstallScript(std::vector<std::string> const& cmd)
{
  this->name = cmSystemTools::RelativePath(
    cmSystemTools::GetLogicalWorkingDirectory(), cmd.back());
  this->command = cmd;
}

void InstallScript::start(cm::uv_loop_ptr& loop,
                          std::function<void()> callback)
{
  cmUVProcessChainBuilder builder;
  builder.AddCommand(this->command)
    .SetExternalLoop(*loop)
    .SetMergedBuiltinStreams();
  this->chain = cm::make_unique<cmUVProcessChain>(builder.Start());
  this->pipe.init(this->chain->GetLoop(), 0);
  uv_pipe_open(this->pipe, this->chain->OutputStream());
  this->streamHandler = cmUVStreamRead(
    this->pipe,
    [this](std::vector<char> data) {
      std::string strdata;
      cmProcessOutput(cmProcessOutput::Auto)
        .DecodeText(data.data(), data.size(), strdata);
      this->output.push_back(strdata);
    },
    std::move(callback));
}

void InstallScript::printResult(std::size_t n, std::size_t total)
{
  cmSystemTools::Stdout(cmStrCat('[', n, '/', total, "] ", this->name, '\n'));
  for (auto const& line : this->output) {
    cmSystemTools::Stdout(line);
  }
}
