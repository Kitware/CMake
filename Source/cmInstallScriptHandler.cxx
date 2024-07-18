/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

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

#include "cmJSONState.h"
#include "cmProcessOutput.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmUVHandlePtr.h"
#include "cmUVProcessChain.h"
#include "cmUVStream.h"

using InstallScript = cmInstallScriptHandler::InstallScript;

cmInstallScriptHandler::cmInstallScriptHandler(const std::string& binary_dir,
                                               std::vector<std::string>& args)
{
  const std::string& file =
    cmStrCat(binary_dir, "/CMakeFiles/InstallScripts.json");
  if (cmSystemTools::FileExists(file)) {
    int compare;
    cmSystemTools::FileTimeCompare(
      cmStrCat(binary_dir, "/CMakeFiles/cmake.check_cache"), file, &compare);
    if (compare < 1) {
      args.insert(args.end() - 1, "-DCMAKE_INSTALL_LOCAL_ONLY=1");
      Json::CharReaderBuilder rbuilder;
      auto JsonReader =
        std::unique_ptr<Json::CharReader>(rbuilder.newCharReader());
      std::vector<char> content;
      Json::Value value;
      cmJSONState state(file, &value);
      for (auto const& script : value["InstallScripts"]) {
        this->commands.push_back(args);
        this->commands.back().emplace_back(script.asCString());
      }
    }
  }
}

bool cmInstallScriptHandler::isParallel()
{
  return !this->commands.empty();
}

int cmInstallScriptHandler::install(unsigned int j)
{
  cm::uv_loop_ptr loop;
  loop.init();
  std::vector<InstallScript> scripts;
  for (auto const& cmd : this->commands) {
    scripts.push_back(InstallScript(cmd));
  }
  std::size_t working = 0;
  std::size_t installed = 0;
  std::size_t i = 0;

  while (installed < scripts.size()) {
    for (auto queue = std::min(j - working, scripts.size() - i); queue > 0;
         --queue) {
      scripts[i].start(loop, [&scripts, &working, &installed, i]() {
        scripts[i].printResult(++installed, scripts.size());
        --working;
      });
      ++i;
    }
    uv_run(loop, UV_RUN_DEFAULT);
  }
  return 0;
}

InstallScript::InstallScript(const std::vector<std::string>& cmd)
{
  this->name = cmSystemTools::RelativePath(
    cmSystemTools::GetCurrentWorkingDirectory(), cmd.back());
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
  cmSystemTools::Stdout(cmStrCat("[", n, "/", total, "] ", this->name, "\n"));
  for (auto const& line : this->output) {
    cmSystemTools::Stdout(line);
  }
}
