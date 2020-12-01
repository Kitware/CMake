/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once
#include <memory>
#include <string>

#include "cmsys/FStream.hxx"

namespace Json {
class StreamWriter;
}

class cmListFileContext;
class cmListFileFunction;

class cmMakefileProfilingData
{
public:
  cmMakefileProfilingData(const std::string&);
  ~cmMakefileProfilingData() noexcept;
  void StartEntry(const cmListFileFunction& lff, cmListFileContext const& lfc);
  void StopEntry();

private:
  cmsys::ofstream ProfileStream;
  std::unique_ptr<Json::StreamWriter> JsonWriter;
};
