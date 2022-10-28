/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once
#include <memory>
#include <string>
#include <utility>

#include <cm/optional>

#include <cm3p/json/value.h> // IWYU pragma: keep

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
  void StartEntry(const std::string& category, const std::string& name,
                  cm::optional<Json::Value> args = cm::nullopt);
  void StopEntry();

  class RAII
  {
  public:
    RAII() = delete;
    RAII(const RAII&) = delete;
    RAII(RAII&&) noexcept;

    template <typename... Args>
    RAII(cmMakefileProfilingData& data, Args&&... args)
      : Data(&data)
    {
      this->Data->StartEntry(std::forward<Args>(args)...);
    }

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
