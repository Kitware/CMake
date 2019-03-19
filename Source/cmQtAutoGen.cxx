/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmQtAutoGen.h"
#include "cmAlgorithms.h"
#include "cmSystemTools.h"

#include "cmsys/RegularExpression.hxx"

#include <algorithm>
#include <array>
#include <sstream>
#include <utility>

// - Static functions

/// @brief Merges newOpts into baseOpts
/// @arg valueOpts list of options that accept a value
void MergeOptions(std::vector<std::string>& baseOpts,
                  std::vector<std::string> const& newOpts,
                  std::vector<std::string> const& valueOpts, bool isQt5)
{
  typedef std::vector<std::string>::iterator Iter;
  typedef std::vector<std::string>::const_iterator CIter;
  if (newOpts.empty()) {
    return;
  }
  if (baseOpts.empty()) {
    baseOpts = newOpts;
    return;
  }

  std::vector<std::string> extraOpts;
  for (CIter fit = newOpts.begin(), fitEnd = newOpts.end(); fit != fitEnd;
       ++fit) {
    std::string const& newOpt = *fit;
    Iter existIt = std::find(baseOpts.begin(), baseOpts.end(), newOpt);
    if (existIt != baseOpts.end()) {
      if (newOpt.size() >= 2) {
        // Acquire the option name
        std::string optName;
        {
          auto oit = newOpt.begin();
          if (*oit == '-') {
            ++oit;
            if (isQt5 && (*oit == '-')) {
              ++oit;
            }
            optName.assign(oit, newOpt.end());
          }
        }
        // Test if this is a value option and change the existing value
        if (!optName.empty() &&
            (std::find(valueOpts.begin(), valueOpts.end(), optName) !=
             valueOpts.end())) {
          const Iter existItNext(existIt + 1);
          const CIter fitNext(fit + 1);
          if ((existItNext != baseOpts.end()) && (fitNext != fitEnd)) {
            *existItNext = *fitNext;
            ++fit;
          }
        }
      }
    } else {
      extraOpts.push_back(newOpt);
    }
  }
  // Append options
  baseOpts.insert(baseOpts.end(), extraOpts.begin(), extraOpts.end());
}

// - Class definitions

unsigned int const cmQtAutoGen::ParallelMax = 64;
std::string const cmQtAutoGen::ListSep = "<<<S>>>";

std::string const cmQtAutoGen::GenAutoGen = "AutoGen";
std::string const cmQtAutoGen::GenAutoMoc = "AutoMoc";
std::string const cmQtAutoGen::GenAutoUic = "AutoUic";
std::string const cmQtAutoGen::GenAutoRcc = "AutoRcc";

std::string const cmQtAutoGen::GenAUTOGEN = "AUTOGEN";
std::string const cmQtAutoGen::GenAUTOMOC = "AUTOMOC";
std::string const cmQtAutoGen::GenAUTOUIC = "AUTOUIC";
std::string const cmQtAutoGen::GenAUTORCC = "AUTORCC";

std::string const& cmQtAutoGen::GeneratorName(GenT genType)
{
  switch (genType) {
    case GenT::GEN:
      return GenAutoGen;
    case GenT::MOC:
      return GenAutoMoc;
    case GenT::UIC:
      return GenAutoUic;
    case GenT::RCC:
      return GenAutoRcc;
  }
  return GenAutoGen;
}

std::string const& cmQtAutoGen::GeneratorNameUpper(GenT genType)
{
  switch (genType) {
    case GenT::GEN:
      return GenAUTOGEN;
    case GenT::MOC:
      return GenAUTOMOC;
    case GenT::UIC:
      return GenAUTOUIC;
    case GenT::RCC:
      return GenAUTORCC;
  }
  return GenAUTOGEN;
}

std::string cmQtAutoGen::Tools(bool moc, bool uic, bool rcc)
{
  std::string res;
  std::vector<std::string> lst;
  if (moc) {
    lst.emplace_back(GenAUTOMOC);
  }
  if (uic) {
    lst.emplace_back(GenAUTOUIC);
  }
  if (rcc) {
    lst.emplace_back(GenAUTORCC);
  }
  switch (lst.size()) {
    case 1:
      res += lst.at(0);
      break;
    case 2:
      res += lst.at(0);
      res += " and ";
      res += lst.at(1);
      break;
    case 3:
      res += lst.at(0);
      res += ", ";
      res += lst.at(1);
      res += " and ";
      res += lst.at(2);
      break;
    default:
      break;
  }
  return res;
}

std::string cmQtAutoGen::Quoted(std::string const& text)
{
  const std::array<std::pair<const char*, const char*>, 9> replaces = {
    { { "\\", "\\\\" },
      { "\"", "\\\"" },
      { "\a", "\\a" },
      { "\b", "\\b" },
      { "\f", "\\f" },
      { "\n", "\\n" },
      { "\r", "\\r" },
      { "\t", "\\t" },
      { "\v", "\\v" } }
  };

  std::string res = text;
  for (auto const& pair : replaces) {
    cmSystemTools::ReplaceString(res, pair.first, pair.second);
  }
  res = '"' + res;
  res += '"';
  return res;
}

std::string cmQtAutoGen::QuotedCommand(std::vector<std::string> const& command)
{
  std::string res;
  for (std::string const& item : command) {
    if (!res.empty()) {
      res.push_back(' ');
    }
    std::string const cesc = cmQtAutoGen::Quoted(item);
    if (item.empty() || (cesc.size() > (item.size() + 2)) ||
        (cesc.find(' ') != std::string::npos)) {
      res += cesc;
    } else {
      res += item;
    }
  }
  return res;
}

std::string cmQtAutoGen::SubDirPrefix(std::string const& filename)
{
  std::string res(cmSystemTools::GetFilenamePath(filename));
  if (!res.empty()) {
    res += '/';
  }
  return res;
}

std::string cmQtAutoGen::AppendFilenameSuffix(std::string const& filename,
                                              std::string const& suffix)
{
  std::string res;
  auto pos = filename.rfind('.');
  if (pos != std::string::npos) {
    const auto it_dot = filename.begin() + pos;
    res.assign(filename.begin(), it_dot);
    res.append(suffix);
    res.append(it_dot, filename.end());
  } else {
    res = filename;
    res.append(suffix);
  }
  return res;
}

void cmQtAutoGen::UicMergeOptions(std::vector<std::string>& baseOpts,
                                  std::vector<std::string> const& newOpts,
                                  bool isQt5)
{
  static std::vector<std::string> const valueOpts = {
    "tr",      "translate", "postfix", "generator",
    "include", // Since Qt 5.3
    "g"
  };
  MergeOptions(baseOpts, newOpts, valueOpts, isQt5);
}

void cmQtAutoGen::RccMergeOptions(std::vector<std::string>& baseOpts,
                                  std::vector<std::string> const& newOpts,
                                  bool isQt5)
{
  static std::vector<std::string> const valueOpts = { "name", "root",
                                                      "compress",
                                                      "threshold" };
  MergeOptions(baseOpts, newOpts, valueOpts, isQt5);
}

void cmQtAutoGen::RccListParseContent(std::string const& content,
                                      std::vector<std::string>& files)
{
  cmsys::RegularExpression fileMatchRegex("(<file[^<]+)");
  cmsys::RegularExpression fileReplaceRegex("(^<file[^>]*>)");

  const char* contentChars = content.c_str();
  while (fileMatchRegex.find(contentChars)) {
    std::string const qrcEntry = fileMatchRegex.match(1);
    contentChars += qrcEntry.size();
    {
      fileReplaceRegex.find(qrcEntry);
      std::string const tag = fileReplaceRegex.match(1);
      files.push_back(qrcEntry.substr(tag.size()));
    }
  }
}

bool cmQtAutoGen::RccListParseOutput(std::string const& rccStdOut,
                                     std::string const& rccStdErr,
                                     std::vector<std::string>& files,
                                     std::string& error)
{
  // Lambda to strip CR characters
  auto StripCR = [](std::string& line) {
    std::string::size_type cr = line.find('\r');
    if (cr != std::string::npos) {
      line = line.substr(0, cr);
    }
  };

  // Parse rcc std output
  {
    std::istringstream ostr(rccStdOut);
    std::string oline;
    while (std::getline(ostr, oline)) {
      StripCR(oline);
      if (!oline.empty()) {
        files.push_back(oline);
      }
    }
  }
  // Parse rcc error output
  {
    std::istringstream estr(rccStdErr);
    std::string eline;
    while (std::getline(estr, eline)) {
      StripCR(eline);
      if (cmHasLiteralPrefix(eline, "RCC: Error in")) {
        static std::string const searchString = "Cannot find file '";

        std::string::size_type pos = eline.find(searchString);
        if (pos == std::string::npos) {
          error = "rcc lists unparsable output:\n";
          error += cmQtAutoGen::Quoted(eline);
          error += "\n";
          return false;
        }
        pos += searchString.length();
        std::string::size_type sz = eline.size() - pos - 1;
        files.push_back(eline.substr(pos, sz));
      }
    }
  }

  return true;
}

void cmQtAutoGen::RccListConvertFullPath(std::string const& qrcFileDir,
                                         std::vector<std::string>& files)
{
  for (std::string& entry : files) {
    std::string tmp = cmSystemTools::CollapseCombinedPath(qrcFileDir, entry);
    entry = std::move(tmp);
  }
}
