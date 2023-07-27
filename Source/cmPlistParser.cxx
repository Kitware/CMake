/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmPlistParser.h"

#include <cm3p/json/reader.h>
#include <cm3p/json/value.h>

#include "cmUVProcessChain.h"
#include "cmUVStream.h"

cm::optional<Json::Value> cmParsePlist(const std::string& filename)
{
  cmUVProcessChainBuilder builder;
  builder.AddCommand(
    { "/usr/bin/plutil", "-convert", "json", "-o", "-", filename });
  builder.SetBuiltinStream(cmUVProcessChainBuilder::Stream_OUTPUT);

  auto chain = builder.Start();
  chain.Wait();

  auto const& status = chain.GetStatus(0);
  if (status.ExitStatus != 0) {
    return cm::nullopt;
  }

  Json::Reader reader;
  Json::Value value;
  cmUVPipeIStream outputStream(chain.GetLoop(), chain.OutputStream());
  if (!reader.parse(outputStream, value)) {
    return cm::nullopt;
  }
  return cm::optional<Json::Value>(value);
}
