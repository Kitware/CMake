/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once
#include <memory>
#include <string>

#include <cm/optional>

#include <cm3p/json/value.h> // IWYU pragma: keep

#include "cmsys/FStream.hxx"

namespace Json {
class StreamWriter;
}

class cmMakefileProfilingData
{
public:
  cmMakefileProfilingData(std::string const&);
  ~cmMakefileProfilingData() noexcept;
  void StartEntry(std::string const& category, std::string const& name,
                  cm::optional<Json::Value> args = cm::nullopt);
  void StopEntry();

  class RAII
  {
  public:
    RAII() = delete;
    RAII(const RAII&) = delete;
    RAII(RAII&&) noexcept;

    RAII(cmMakefileProfilingData& data, std::string const& category,
         std::string const& name,
         cm::optional<Json::Value> args = cm::nullopt);

    ~RAII();

    RAII& operator=(const RAII&) = delete;
    RAII& operator=(RAII&&) noexcept;

  private:
    cmMakefileProfilingData* Data = nullptr;
  };

private:
  cmsys::ofstream ProfileStream;
  std::unique_ptr<Json::StreamWriter> JsonWriter;
};
