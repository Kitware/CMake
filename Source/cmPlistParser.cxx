/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmPlistParser.h"

#include <iterator>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <cm3p/json/value.h>

#include "cmJSONState.h"
#include "cmSystemTools.h"
#include "cmUVProcessChain.h"
#include "cmUVStream.h"

namespace {
std::vector<std::string> PlistToJsonCommand(std::string const& filename)
{
  static std::string const plutil = cmSystemTools::FindProgram("plutil");
  if (!plutil.empty()) {
    return { plutil, "-convert", "json", "-o", "-", filename };
  }

  static std::string const plistutil = cmSystemTools::FindProgram("plistutil");
  if (!plistutil.empty()) {
    return { plistutil, "-f", "json", "-o", "-", "-i", filename };
  }

  return {};
}
}

cm::optional<Json::Value> cmParsePlist(std::string const& filename)
{
  std::vector<std::string> command = PlistToJsonCommand(filename);
  if (command.empty()) {
    return cm::nullopt;
  }

  cmUVProcessChainBuilder builder;
  builder.AddCommand(std::move(command));
  builder.SetBuiltinStream(cmUVProcessChainBuilder::Stream_OUTPUT);

  auto chain = builder.Start();
  chain.Wait();

  auto const& status = chain.GetStatus(0);
  if (status.ExitStatus != 0) {
    return cm::nullopt;
  }

  // Buffer plutil's output into a seekable stream. cmJSONState must be able
  // peek at a leading BOM, which the process pipe behind cmUVIStream can't
  // handle.
  cmUVIStream outputStream(chain.OutputStream());
  std::string output{ std::istreambuf_iterator<char>(outputStream),
                      std::istreambuf_iterator<char>() };
  std::istringstream jsonStream(output);

  Json::Value value;
  cmJSONState parseState(jsonStream, &value, cmJSONState::StrictMode::Relaxed);
  if (!parseState.errors.empty()) {
    return cm::nullopt;
  }
  return cm::optional<Json::Value>(value);
}
