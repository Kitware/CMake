/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmLcovTracefile.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <utility>
#include <vector>

#include <cm/string_view>
#include <cmext/algorithm>
#include <cmext/string_view>

#include "cmsys/FStream.hxx"

#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

cmLcovFunction::cmLcovFunction(std::string name, unsigned long long hits)
  : Name(std::move(name))
  , Hits(hits)
{
}

bool cmLcovSourceFile::HasData() const
{
  return !this->Lines.empty() || !this->Functions.empty();
}

namespace {

void MergeSourceFile(cmLcovSourceFile& into, cmLcovSourceFile const& from)
{
  for (auto const& line : from.Lines) {
    into.Lines[line.first] += line.second;
  }
  for (auto const& fn : from.Functions) {
    auto it = into.Functions.find(fn.first);
    if (it == into.Functions.end()) {
      into.Functions.emplace(fn.first, fn.second);
    } else {
      it->second.Hits += fn.second.Hits;
    }
  }
}

// Percent-encode, as in URLs, the characters that a name can not hold in
// lcov records: "," and line breaks.  Also encode "%" and the "@" of the
// "@<line>" suffix, so that every name reads back as written.
std::string EncodeName(std::string const& name)
{
  std::string out;
  for (char c : name) {
    switch (c) {
      case '%':
      case ',':
      case '\n':
      case '\r':
      case '@': {
        std::array<char, 4> hex{};
        std::snprintf(hex.data(), hex.size(), "%%%02X",
                      static_cast<unsigned char>(c));
        out += hex.data();
      } break;
      default:
        out += c;
        break;
    }
  }
  return out;
}

// The exact inverse of EncodeName.
std::string DecodeName(std::string const& name)
{
  return cmSystemTools::DecodeURL(name);
}

bool ParseLine(std::string const& s, long* line)
{
  return cmStrToLong(s, line) && *line > 0;
}

// Split "<a>,<b>" at the first comma.
bool SplitPair(std::string const& value, std::string* first,
               std::string* second)
{
  std::string::size_type const comma = value.find(',');
  if (comma == std::string::npos) {
    return false;
  }
  *first = value.substr(0, comma);
  *second = value.substr(comma + 1);
  return true;
}

// If NAME ends with "@<line>" for the given definition line, strip that
// suffix.  The writer adds it to keep duplicate names unique.
std::string StripLineSuffix(std::string const& name, long line)
{
  std::string const suffix = cmStrCat('@', line);
  if (name.size() > suffix.size() && cmHasSuffix(name, suffix)) {
    return name.substr(0, name.size() - suffix.size());
  }
  return name;
}
}

void cmLcovTracefile::Merge(cmLcovTracefile const& other)
{
  for (auto const& file : other.Files) {
    MergeSourceFile(this->Files[file.first], file.second);
  }
}

bool cmLcovTracefile::MergeFiles(
  std::vector<std::string> const& paths,
  std::function<void(std::string const&)> const& warn)
{
  bool allMerged = true;
  for (std::string const& path : paths) {
    // Read into a fresh object, so that a bad file adds nothing.
    cmLcovTracefile tracefile;
    std::string error;
    cmsys::ifstream fin(path.c_str());
    if (!fin || !tracefile.Read(fin, error)) {
      std::string warning =
        cmStrCat("Skipping unreadable CMake listfile coverage file: ", path);
      if (!error.empty()) {
        warning = cmStrCat(std::move(warning), "\n  ", error);
      }
      warn(warning);
      allMerged = false;
      continue;
    }
    this->Merge(tracefile);
  }
  return allMerged;
}

void cmLcovTracefile::RemoveBuildTreeFiles(
  std::string const& sourceDir, std::string const& buildDir,
  std::function<void(std::string const&)> const& removed)
{
  if (buildDir.empty()) {
    return;
  }
  std::string const build = cmSystemTools::CollapseFullPath(buildDir);
  if (!sourceDir.empty() &&
      cmSystemTools::ComparePath(cmSystemTools::CollapseFullPath(sourceDir),
                                 build)) {
    return;
  }
  for (auto it = this->Files.begin(); it != this->Files.end();) {
    std::string const path = cmSystemTools::CollapseFullPath(it->first);
    if (cmSystemTools::IsSubDirectory(path, build)) {
      removed(path);
      it = this->Files.erase(it);
    } else {
      ++it;
    }
  }
}

bool cmLcovTracefile::HasData() const
{
  return std::any_of(
    this->Files.begin(), this->Files.end(),
    [](std::pair<std::string const, cmLcovSourceFile> const& file) {
      return file.second.HasData();
    });
}

void cmLcovTracefile::Write(std::ostream& os) const
{
  if (!this->TestName.empty()) {
    os << "TN:" << this->TestName << '\n';
  }
  for (auto const& file : this->Files) {
    cmLcovSourceFile const& data = file.second;
    // lcov rejects a tracefile that has only such records.
    if (!data.HasData()) {
      continue;
    }
    os << "SF:" << file.first << '\n';

    // Give each record a name that is unique in this block.
    std::map<std::string, unsigned int> nameCounts;
    std::vector<std::pair<long, std::string>> names;
    names.reserve(data.Functions.size());
    for (auto const& fn : data.Functions) {
      names.emplace_back(fn.first, EncodeName(fn.second.Name));
      ++nameCounts[names.back().second];
    }
    for (auto& name : names) {
      if (nameCounts[name.second] > 1) {
        name.second = cmStrCat(name.second, '@', name.first);
      }
    }

    for (auto const& name : names) {
      os << "FN:" << name.first << ',' << name.second << '\n';
    }
    std::size_t functionsHit = 0;
    for (auto const& name : names) {
      unsigned long long const hits = data.Functions.at(name.first).Hits;
      os << "FNDA:" << hits << ',' << name.second << '\n';
      if (hits > 0) {
        ++functionsHit;
      }
    }
    os << "FNF:" << data.Functions.size() << '\n';
    os << "FNH:" << functionsHit << '\n';

    std::size_t linesHit = 0;
    for (auto const& line : data.Lines) {
      os << "DA:" << line.first << ',' << line.second << '\n';
      if (line.second > 0) {
        ++linesHit;
      }
    }
    os << "LF:" << data.Lines.size() << '\n';
    os << "LH:" << linesHit << '\n';
    os << "end_of_record\n";
  }
}

bool cmLcovTracefile::Read(std::istream& is, std::string& error)
{
  bool inBlock = false;
  bool sawFile = false;
  std::string path;
  cmLcovSourceFile block;
  // Written function name -> definition line, for FNDA records.
  std::map<std::string, long> nameToLine;

  std::string text;
  unsigned long lineNumber = 0;
  auto fail = [&](cm::string_view message) -> bool {
    error = cmStrCat("line ", lineNumber, ": ", message);
    return false;
  };

  while (cmSystemTools::GetLineFromStream(is, text)) {
    ++lineNumber;
    if (!text.empty() && text.back() == '\r') {
      text.pop_back();
    }
    if (text.empty()) {
      continue;
    }

    std::string key;
    std::string value;
    std::string::size_type const colon = text.find(':');
    if (colon == std::string::npos) {
      key = text;
    } else {
      key = text.substr(0, colon);
      value = text.substr(colon + 1);
    }

    if (key == "TN") {
      this->TestName = value;
      continue;
    }
    if (key == "SF") {
      if (inBlock) {
        return fail("SF record before end_of_record");
      }
      inBlock = true;
      sawFile = true;
      path = value;
      continue;
    }

    static constexpr std::array<cm::string_view, 8> blockRecords{
      { "FN"_s, "FNDA"_s, "DA"_s, "FNF"_s, "FNH"_s, "LF"_s, "LH"_s,
        "end_of_record"_s }
    };
    if (!cm::contains(blockRecords, key)) {
      // Unknown records and the branch records BRDA, BRF and BRH.
      continue;
    }
    if (!inBlock) {
      return fail(cmStrCat(key, " record before SF"));
    }

    if (key == "FN") {
      std::string lineText;
      std::string name;
      long line = 0;
      if (!SplitPair(value, &lineText, &name) || !ParseLine(lineText, &line)) {
        return fail(cmStrCat("malformed FN record \"", value, '"'));
      }
      nameToLine[name] = line;
      block.Functions[line].Name = DecodeName(StripLineSuffix(name, line));
    } else if (key == "FNDA") {
      std::string hitsText;
      std::string name;
      unsigned long long hits = 0;
      if (!SplitPair(value, &hitsText, &name) ||
          !cmStrToULongLong(hitsText, &hits)) {
        return fail(cmStrCat("malformed FNDA record \"", value, '"'));
      }
      auto it = nameToLine.find(name);
      if (it == nameToLine.end()) {
        return fail(cmStrCat("FNDA record for \"", name, "\" with no FN"));
      }
      block.Functions[it->second].Hits += hits;
    } else if (key == "DA") {
      // A third field, the checksum, is ignored.
      std::string lineText;
      std::string rest;
      long line = 0;
      unsigned long long hits = 0;
      if (!SplitPair(value, &lineText, &rest) || !ParseLine(lineText, &line) ||
          !cmStrToULongLong(rest.substr(0, rest.find(',')), &hits)) {
        return fail(cmStrCat("malformed DA record \"", value, '"'));
      }
      block.Lines[line] += hits;
    } else if (key == "end_of_record") {
      MergeSourceFile(this->Files[path], block);
      block = cmLcovSourceFile();
      nameToLine.clear();
      inBlock = false;
    }
    // FNF, FNH, LF and LH are computed again by Write.
  }

  if (inBlock) {
    return fail("missing end_of_record");
  }
  if (!sawFile) {
    return fail("no SF record");
  }
  return true;
}
