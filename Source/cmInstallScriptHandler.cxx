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
using InstallScriptRunner = cmInstallScriptHandler::InstallScriptRunner;

cmInstallScriptHandler::cmInstallScriptHandler(
  std::string binaryDir, std::vector<std::string> components,
  std::string installConfig, std::vector<std::string>& args)
  : Components(std::move(components))
  , BinaryDir(std::move(binaryDir))
{
  if (this->Components.empty()) {
    this->Components.emplace_back(std::string{});
  }

  std::string const& file =
    cmStrCat(this->BinaryDir, "/CMakeFiles/InstallScripts.json");
  this->Parallel = false;

  auto addScript = [this, &args](std::string script, std::string component,
                                 std::string config) -> void {
    this->Scripts.push_back({ script, config, args });
    if (!component.empty()) {
      this->Scripts.back().command.insert(
        this->Scripts.back().command.end() - 1,
        cmStrCat("-DCMAKE_INSTALL_COMPONENT=", component));
    }
    if (!config.empty()) {
      this->Scripts.back().command.insert(
        this->Scripts.back().command.end() - 1,
        cmStrCat("-DCMAKE_INSTALL_CONFIG_NAME=", config));
    }
    this->Scripts.back().command.emplace_back(script);
    this->Directories.push_back(cmSystemTools::GetFilenamePath(script));
  };

  int compare = 1;
  if (cmSystemTools::FileExists(file)) {
    cmSystemTools::FileTimeCompare(
      cmStrCat(this->BinaryDir, "/CMakeFiles/cmake.check_cache"), file,
      &compare);
  }
  if (compare < 1) {
    Json::CharReaderBuilder rbuilder;
    auto jsonReader =
      std::unique_ptr<Json::CharReader>(rbuilder.newCharReader());
    std::vector<char> content;
    Json::Value value;
    cmJSONState state(file, &value);
    this->Parallel = value["Parallel"].asBool();
    if (this->Parallel) {
      args.insert(args.end() - 1, "-DCMAKE_INSTALL_LOCAL_ONLY=1");
    }
    if (installConfig.empty() && value.isMember("Configs")) {
      for (auto const& config : value["Configs"]) {
        this->Configs.push_back(config.asCString());
      }
    } else {
      this->Configs.push_back(installConfig);
    }
    for (auto const& script : value["InstallScripts"]) {
      for (auto const& component : Components) {
        for (auto const& config : Configs) {
          addScript(script.asCString(), component, config);
        }
      }
      if (!this->Parallel) {
        break;
      }
    }
  } else {
    for (auto const& component : Components) {
      addScript(cmStrCat(this->BinaryDir, "/cmake_install.cmake"), component,
                installConfig);
    }
  }
}

bool cmInstallScriptHandler::IsParallel()
{
  return this->Parallel;
}

std::vector<InstallScript> cmInstallScriptHandler::GetScripts() const
{
  return this->Scripts;
}

int cmInstallScriptHandler::Install(unsigned int j,
                                    cmInstrumentation& instrumentation)
{
  cm::uv_loop_ptr loop;
  loop.init();
  std::vector<InstallScriptRunner> runners;
  runners.reserve(this->Scripts.size());

  std::vector<std::string> instrumentArg;
  if (instrumentation.HasQuery()) {
    instrumentArg = { cmSystemTools::GetCTestCommand(),
                      "--instrument",
                      "--command-type",
                      "install",
                      "--build-dir",
                      this->BinaryDir,
                      "--config",
                      "",
                      "--" };
  }

  for (auto& script : this->Scripts) {
    if (!instrumentArg.empty()) {
      instrumentArg[7] = script.config; // --config <script.config>
    }
    script.command.insert(script.command.begin(), instrumentArg.begin(),
                          instrumentArg.end());
    runners.emplace_back(script);
  }
  std::size_t working = 0;
  std::size_t installed = 0;
  std::size_t i = 0;

  std::function<void()> queueScripts;
  queueScripts = [&runners, &working, &installed, &i, &loop, j,
                  &queueScripts]() {
    for (auto queue = std::min(j - working, runners.size() - i); queue > 0;
         --queue) {
      ++working;
      runners[i].start(loop,
                       [&runners, &working, &installed, i, &queueScripts]() {
                         runners[i].printResult(++installed, runners.size());
                         --working;
                         queueScripts();
                       });
      ++i;
    }
  };
  queueScripts();
  uv_run(loop, UV_RUN_DEFAULT);

  // Write install manifest
  std::string installManifest;
  for (auto const& component : this->Components) {
    if (component.empty()) {
      installManifest = "install_manifest.txt";
    } else {
      cmsys::RegularExpression regEntry;
      if (regEntry.compile("^[a-zA-Z0-9_.+-]+$") && regEntry.find(component)) {
        installManifest = cmStrCat("install_manifest_", component, ".txt");
      } else {
        cmCryptoHash md5(cmCryptoHash::AlgoMD5);
        md5.Initialize();
        installManifest =
          cmStrCat("install_manifest_", md5.HashString(component), ".txt");
      }
    }
    cmGeneratedFileStream fout(
      cmStrCat(this->BinaryDir, '/', installManifest));
    fout.SetCopyIfDifferent(true);
    for (auto const& dir : this->Directories) {
      auto localManifest = cmStrCat(dir, "/install_local_manifest.txt");
      if (cmSystemTools::FileExists(localManifest)) {
        cmsys::ifstream fin(localManifest.c_str());
        std::string line;
        while (std::getline(fin, line)) {
          fout << line << "\n";
        }
      }
    }
  }
  return 0;
}

InstallScriptRunner::InstallScriptRunner(InstallScript const& script)
{
  this->Name = cmSystemTools::RelativePath(
    cmSystemTools::GetLogicalWorkingDirectory(), script.path);
  this->Command = script.command;
}

void InstallScriptRunner::start(cm::uv_loop_ptr& loop,
                                std::function<void()> callback)
{
  cmUVProcessChainBuilder builder;
  builder.AddCommand(this->Command)
    .SetExternalLoop(*loop)
    .SetMergedBuiltinStreams();
  this->Chain = cm::make_unique<cmUVProcessChain>(builder.Start());
  this->StreamHandler = cmUVStreamRead(
    this->Chain->OutputStream(),
    [this](std::vector<char> data) {
      std::string strdata;
      cmProcessOutput(cmProcessOutput::Auto)
        .DecodeText(data.data(), data.size(), strdata);
      this->Output.push_back(strdata);
    },
    std::move(callback));
}

void InstallScriptRunner::printResult(std::size_t n, std::size_t total)
{
  cmSystemTools::Stdout(cmStrCat('[', n, '/', total, "] ", this->Name, '\n'));
  for (auto const& line : this->Output) {
    cmSystemTools::Stdout(line);
  }
}
