/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmProcessTools.h"

#include <algorithm>
#include <iterator>
#include <ostream>

#include <cm3p/uv.h>

#include "cmProcessOutput.h"
#include "cmUVHandlePtr.h"
#include "cmUVStream.h"

std::vector<cmUVProcessChain::Status> cmProcessTools::RunProcess(
  cmUVProcessChainBuilder& builder, OutputParser* out, OutputParser* err,
  Encoding encoding)
{
  cmProcessOutput processOutput(encoding);

  builder.SetBuiltinStream(cmUVProcessChainBuilder::Stream_OUTPUT)
    .SetBuiltinStream(cmUVProcessChainBuilder::Stream_ERROR);

  auto chain = builder.Start();

  std::string strdata;
  cm::uv_pipe_ptr outputPipe;
  outputPipe.init(chain.GetLoop(), 0);
  uv_pipe_open(outputPipe, chain.OutputStream());
  auto outputHandle = cmUVStreamRead(
    outputPipe,
    [&out, &processOutput, &strdata](std::vector<char> data) {
      if (out) {
        processOutput.DecodeText(data.data(), data.size(), strdata, 1);
        if (!out->Process(strdata.c_str(), static_cast<int>(strdata.size()))) {
          out = nullptr;
        }
      }
    },
    [&out]() { out = nullptr; });
  cm::uv_pipe_ptr errorPipe;
  errorPipe.init(chain.GetLoop(), 0);
  uv_pipe_open(errorPipe, chain.ErrorStream());
  auto errorHandle = cmUVStreamRead(
    errorPipe,
    [&err, &processOutput, &strdata](std::vector<char> data) {
      if (err) {
        processOutput.DecodeText(data.data(), data.size(), strdata, 2);
        if (!err->Process(strdata.c_str(), static_cast<int>(strdata.size()))) {
          err = nullptr;
        }
      }
    },
    [&err]() { err = nullptr; });
  while (out || err || !chain.Finished()) {
    uv_run(&chain.GetLoop(), UV_RUN_ONCE);
  }

  std::vector<cmUVProcessChain::Status> result;
  auto status = chain.GetStatus();
  std::transform(
    status.begin(), status.end(), std::back_inserter(result),
    [](cmUVProcessChain::Status const* s) -> cmUVProcessChain::Status {
      return *s;
    });
  return result;
}

cmProcessTools::LineParser::LineParser(char sep, bool ignoreCR)
  : Separator(sep)
  , IgnoreCR(ignoreCR)
{
}

void cmProcessTools::LineParser::SetLog(std::ostream* log, char const* prefix)
{
  this->Log = log;
  this->Prefix = prefix ? prefix : "";
}

bool cmProcessTools::LineParser::ProcessChunk(char const* first, int length)
{
  char const* last = first + length;
  for (char const* c = first; c != last; ++c) {
    if (*c == this->Separator || *c == '\0') {
      this->LineEnd = *c;

      // Log this line.
      if (this->Log && this->Prefix) {
        *this->Log << this->Prefix << this->Line << "\n";
      }

      // Hand this line to the subclass implementation.
      if (!this->ProcessLine()) {
        this->Line.clear();
        return false;
      }

      this->Line.clear();
    } else if (*c != '\r' || !this->IgnoreCR) {
      // Append this character to the line under construction.
      this->Line.append(1, *c);
    }
  }
  return true;
}
