/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmConfigureLog.h"

#include <cassert>
#include <cstdio>
#include <iterator>
#include <sstream>
#include <utility>

#include <cmext/algorithm>
#include <cmext/string_view>

#include <cm3p/json/writer.h>

#include "cm_utf8.h"

#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmRange.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmake.h"

cmConfigureLog::cmConfigureLog(std::string logDir,
                               std::vector<unsigned long> logVersions)
  : LogDir(std::move(logDir))
  , LogVersions(std::move(logVersions))
{
  // Always emit events for the latest log version.
  static const unsigned long LatestLogVersion = 1;
  if (!cm::contains(this->LogVersions, LatestLogVersion)) {
    this->LogVersions.emplace_back(LatestLogVersion);
  }

  Json::StreamWriterBuilder builder;
  this->Encoder.reset(builder.newStreamWriter());
}

cmConfigureLog::~cmConfigureLog()
{
  if (this->Opened) {
    this->EndObject();
    this->Stream << "...\n";
  }
}

bool cmConfigureLog::IsAnyLogVersionEnabled(
  std::vector<unsigned long> const& v) const
{
  // Both input lists are sorted.  Look for a matching element.
  auto i1 = v.cbegin();
  auto i2 = this->LogVersions.cbegin();
  while (i1 != v.cend() && i2 != this->LogVersions.cend()) {
    if (*i1 < *i2) {
      ++i1;
    } else if (*i2 < *i1) {
      ++i2;
    } else {
      return true;
    }
  }
  return false;
}

void cmConfigureLog::WriteBacktrace(cmMakefile const& mf)
{
  std::vector<std::string> backtrace;
  auto root = mf.GetCMakeInstance()->GetHomeDirectory();
  for (auto bt = mf.GetBacktrace(); !bt.Empty(); bt = bt.Pop()) {
    auto t = bt.Top();
    if (!t.Name.empty() || t.Line == cmListFileContext::DeferPlaceholderLine) {
      t.FilePath = cmSystemTools::RelativeIfUnder(root, t.FilePath);
      std::ostringstream s;
      s << t;
      backtrace.emplace_back(s.str());
    }
  }
  this->WriteValue("backtrace"_s, backtrace);
}

void cmConfigureLog::WriteChecks(cmMakefile const& mf)
{
  if (!mf.GetCMakeInstance()->HasCheckInProgress()) {
    return;
  }
  this->BeginObject("checks"_s);
  for (auto const& value :
       cmReverseRange(mf.GetCMakeInstance()->GetCheckInProgressMessages())) {
    this->BeginLine() << "- ";
    this->Encoder->write(value, &this->Stream);
    this->EndLine();
  }
  this->EndObject();
}

void cmConfigureLog::EnsureInit()
{
  if (this->Opened) {
    return;
  }
  assert(!this->Stream.is_open());

  std::string name = cmStrCat(this->LogDir, "/CMakeConfigureLog.yaml");
  this->Stream.open(name.c_str(), std::ios::out | std::ios::app);

  this->Opened = true;

  this->Stream << "\n---\n";
  this->BeginObject("events"_s);
}

cmsys::ofstream& cmConfigureLog::BeginLine()
{
  for (unsigned i = 0; i < this->Indent; ++i) {
    this->Stream << "  ";
  }
  return this->Stream;
}

void cmConfigureLog::EndLine()
{
  this->Stream << std::endl;
}

void cmConfigureLog::BeginObject(cm::string_view key)
{
  this->BeginLine() << key << ':';
  this->EndLine();
  ++this->Indent;
}

void cmConfigureLog::EndObject()
{
  assert(this->Indent);
  --this->Indent;
}

void cmConfigureLog::BeginEvent(std::string const& kind, cmMakefile const& mf)
{
  this->EnsureInit();

  this->BeginLine() << '-';
  this->EndLine();

  ++this->Indent;

  this->WriteValue("kind"_s, kind);
  this->WriteBacktrace(mf);
  this->WriteChecks(mf);
}

void cmConfigureLog::EndEvent()
{
  assert(this->Indent);
  --this->Indent;
}

void cmConfigureLog::WriteValue(cm::string_view key, std::nullptr_t)
{
  this->BeginLine() << key << ": null";
  this->EndLine();
}

void cmConfigureLog::WriteValue(cm::string_view key, bool value)
{
  this->BeginLine() << key << ": " << (value ? "true" : "false");
  this->EndLine();
}

void cmConfigureLog::WriteValue(cm::string_view key, int value)
{
  this->BeginLine() << key << ": " << value;
  this->EndLine();
}

void cmConfigureLog::WriteValue(cm::string_view key, std::string const& value)
{
  this->BeginLine() << key << ": ";
  this->Encoder->write(value, &this->Stream);
  this->EndLine();
}

void cmConfigureLog::WriteValue(cm::string_view key,
                                std::vector<std::string> const& list)
{
  this->BeginObject(key);
  for (auto const& value : list) {
    this->BeginLine() << "- ";
    this->Encoder->write(value, &this->Stream);
    this->EndLine();
  }
  this->EndObject();
}

void cmConfigureLog::WriteValue(cm::string_view key,
                                std::map<std::string, std::string> const& map)
{
  static const std::string rawKeyChars = //
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"         //
    "abcdefghijklmnopqrstuvwxyz"         //
    "0123456789"                         //
    "-_"                                 //
    ;
  this->BeginObject(key);
  for (auto const& entry : map) {
    if (entry.first.find_first_not_of(rawKeyChars) == std::string::npos) {
      this->WriteValue(entry.first, entry.second);
    } else {
      this->BeginLine();
      this->Encoder->write(entry.first, &this->Stream);
      this->Stream << ": ";
      this->Encoder->write(entry.second, &this->Stream);
      this->EndLine();
    }
  }
  this->EndObject();
}

void cmConfigureLog::WriteLiteralTextBlock(cm::string_view key,
                                           cm::string_view text)
{
  this->BeginLine() << key << ": |";
  this->EndLine();

  auto const l = text.length();
  if (l) {
    ++this->Indent;
    this->BeginLine();

    auto i = decltype(l){ 0 };
    while (i < l) {
      // YAML allows ' ', '\t' and "printable characters", but NOT other
      // ASCII whitespace; those must be escaped, as must the upper UNICODE
      // control characters (U+0080 - U+009F)
      static constexpr unsigned int C1_LAST = 0x9F;
      auto const c = static_cast<unsigned char>(text[i]);
      switch (c) {
        case '\r':
          // Print a carriage return only if it is not followed by a line feed.
          ++i;
          if (i == l || text[i] != '\n') {
            this->WriteEscape(c);
          }
          break;
        case '\n':
          // Print any line feeds except the very last one
          if (i + 1 < l) {
            this->EndLine();
            this->BeginLine();
          }
          ++i;
          break;
        case '\t':
          // Print horizontal tab verbatim
          this->Stream.put('\t');
          ++i;
          break;
        case '\\':
          // Escape backslash for disambiguation
          this->Stream << "\\\\";
          ++i;
          break;
        default:
          if (c >= 32 && c < 127) {
            // Print ascii byte.
            this->Stream.put(text[i]);
            ++i;
            break;
          } else if (c > 127) {
            // Decode a UTF-8 sequence.
            unsigned int c32;
            auto const* const s = text.data() + i;
            auto const* const e = text.data() + l;
            auto const* const n = cm_utf8_decode_character(s, e, &c32);
            if (n > s && c32 > C1_LAST) {
              auto const k = std::distance(s, n);
              this->Stream.write(s, static_cast<std::streamsize>(k));
              i += static_cast<unsigned>(k);
              break;
            }
          }

          // Escape non-printable byte.
          this->WriteEscape(c);
          ++i;
          break;
      }
    }

    this->EndLine();
    --this->Indent;
  }
}

void cmConfigureLog::WriteEscape(unsigned char c)
{
  char buffer[6];
  int n = snprintf(buffer, sizeof(buffer), "\\x%02x", c);
  if (n > 0) {
    this->Stream.write(buffer, n);
  }
}
