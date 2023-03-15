/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <cm/string_view>

#include "cmsys/FStream.hxx"

namespace Json {
class StreamWriter;
}

class cmMakefile;

class cmConfigureLog
{
public:
  /** Construct with the log directory and a sorted list of enabled log
      versions.  The latest log version will be enabled regardless.  */
  cmConfigureLog(std::string logDir, std::vector<unsigned long> logVersions);
  ~cmConfigureLog();

  /** Return true if at least one of the log versions in the given sorted
      list is enabled.  */
  bool IsAnyLogVersionEnabled(std::vector<unsigned long> const& v) const;

  void EnsureInit();

  void BeginEvent(std::string const& kind, cmMakefile const& mf);
  void EndEvent();

  void BeginObject(cm::string_view key);
  void EndObject();

  // TODO other value types
  void WriteValue(cm::string_view key, std::nullptr_t);
  void WriteValue(cm::string_view key, bool value);
  void WriteValue(cm::string_view key, int value);
  void WriteValue(cm::string_view key, std::string const& value);
  void WriteValue(cm::string_view key, std::vector<std::string> const& list);
  void WriteValue(cm::string_view key,
                  std::map<std::string, std::string> const& map);

  void WriteTextBlock(cm::string_view key, cm::string_view text);
  void WriteLiteralTextBlock(cm::string_view key, cm::string_view text);

  void WriteLiteralTextBlock(cm::string_view key, std::string const& text)
  {
    this->WriteLiteralTextBlock(key, cm::string_view{ text });
  }

private:
  std::string LogDir;
  std::vector<unsigned long> LogVersions;
  cmsys::ofstream Stream;
  unsigned Indent = 0;
  bool Opened = false;

  std::unique_ptr<Json::StreamWriter> Encoder;

  void WriteBacktrace(cmMakefile const& mf);
  void WriteChecks(cmMakefile const& mf);

  cmsys::ofstream& BeginLine();
  void EndLine();
  void WriteEscape(unsigned char c);
};
