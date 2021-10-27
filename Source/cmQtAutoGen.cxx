/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmQtAutoGen.h"

#include <algorithm>
#include <array>
#include <initializer_list>
#include <sstream>
#include <utility>

#include <cmext/algorithm>

#include "cmsys/FStream.hxx"
#include "cmsys/RegularExpression.hxx"

#include "cmDuration.h"
#include "cmProcessOutput.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

// - Static functions

/// @brief Merges newOpts into baseOpts
/// @arg valueOpts list of options that accept a value
static void MergeOptions(std::vector<std::string>& baseOpts,
                         std::vector<std::string> const& newOpts,
                         std::initializer_list<cm::string_view> valueOpts,
                         bool isQt5OrLater)
{
  if (newOpts.empty()) {
    return;
  }
  if (baseOpts.empty()) {
    baseOpts = newOpts;
    return;
  }

  std::vector<std::string> extraOpts;
  for (auto fit = newOpts.begin(), fitEnd = newOpts.end(); fit != fitEnd;
       ++fit) {
    std::string const& newOpt = *fit;
    auto existIt = std::find(baseOpts.begin(), baseOpts.end(), newOpt);
    if (existIt != baseOpts.end()) {
      if (newOpt.size() >= 2) {
        // Acquire the option name
        std::string optName;
        {
          auto oit = newOpt.begin();
          if (*oit == '-') {
            ++oit;
            if (isQt5OrLater && (*oit == '-')) {
              ++oit;
            }
            optName.assign(oit, newOpt.end());
          }
        }
        // Test if this is a value option and change the existing value
        if (!optName.empty() && cm::contains(valueOpts, optName)) {
          const auto existItNext(existIt + 1);
          const auto fitNext(fit + 1);
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
  cm::append(baseOpts, extraOpts);
}

// - Class definitions

unsigned int const cmQtAutoGen::ParallelMax = 64;

cm::string_view cmQtAutoGen::GeneratorName(GenT genType)
{
  switch (genType) {
    case GenT::GEN:
      return "AutoGen";
    case GenT::MOC:
      return "AutoMoc";
    case GenT::UIC:
      return "AutoUic";
    case GenT::RCC:
      return "AutoRcc";
  }
  return "AutoGen";
}

cm::string_view cmQtAutoGen::GeneratorNameUpper(GenT genType)
{
  switch (genType) {
    case GenT::GEN:
      return "AUTOGEN";
    case GenT::MOC:
      return "AUTOMOC";
    case GenT::UIC:
      return "AUTOUIC";
    case GenT::RCC:
      return "AUTORCC";
  }
  return "AUTOGEN";
}

std::string cmQtAutoGen::Tools(bool moc, bool uic, bool rcc)
{
  std::array<cm::string_view, 3> lst;
  decltype(lst)::size_type num = 0;
  if (moc) {
    lst.at(num++) = "AUTOMOC";
  }
  if (uic) {
    lst.at(num++) = "AUTOUIC";
  }
  if (rcc) {
    lst.at(num++) = "AUTORCC";
  }
  switch (num) {
    case 1:
      return std::string(lst[0]);
    case 2:
      return cmStrCat(lst[0], " and ", lst[1]);
    case 3:
      return cmStrCat(lst[0], ", ", lst[1], " and ", lst[2]);
    default:
      break;
  }
  return std::string();
}

std::string cmQtAutoGen::Quoted(cm::string_view text)
{
  static std::initializer_list<std::pair<const char*, const char*>> const
    replacements = { { "\\", "\\\\" }, { "\"", "\\\"" }, { "\a", "\\a" },
                     { "\b", "\\b" },  { "\f", "\\f" },  { "\n", "\\n" },
                     { "\r", "\\r" },  { "\t", "\\t" },  { "\v", "\\v" } };

  std::string res(text);
  for (auto const& pair : replacements) {
    cmSystemTools::ReplaceString(res, pair.first, pair.second);
  }
  return cmStrCat('"', res, '"');
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

std::string cmQtAutoGen::FileNameWithoutLastExtension(cm::string_view filename)
{
  auto slashPos = filename.rfind('/');
  if (slashPos != cm::string_view::npos) {
    filename.remove_prefix(slashPos + 1);
  }
  auto dotPos = filename.rfind('.');
  return std::string(filename.substr(0, dotPos));
}

std::string cmQtAutoGen::ParentDir(cm::string_view filename)
{
  auto slashPos = filename.rfind('/');
  if (slashPos == cm::string_view::npos) {
    return std::string();
  }
  return std::string(filename.substr(0, slashPos));
}

std::string cmQtAutoGen::SubDirPrefix(cm::string_view filename)
{
  auto slashPos = filename.rfind('/');
  if (slashPos == cm::string_view::npos) {
    return std::string();
  }
  return std::string(filename.substr(0, slashPos + 1));
}

std::string cmQtAutoGen::AppendFilenameSuffix(cm::string_view filename,
                                              cm::string_view suffix)
{
  auto dotPos = filename.rfind('.');
  if (dotPos == cm::string_view::npos) {
    return cmStrCat(filename, suffix);
  }
  return cmStrCat(filename.substr(0, dotPos), suffix,
                  filename.substr(dotPos, filename.size() - dotPos));
}

void cmQtAutoGen::UicMergeOptions(std::vector<std::string>& baseOpts,
                                  std::vector<std::string> const& newOpts,
                                  bool isQt5OrLater)
{
  static std::initializer_list<cm::string_view> const valueOpts = {
    "tr",      "translate", "postfix", "generator",
    "include", // Since Qt 5.3
    "g"
  };
  MergeOptions(baseOpts, newOpts, valueOpts, isQt5OrLater);
}

void cmQtAutoGen::RccMergeOptions(std::vector<std::string>& baseOpts,
                                  std::vector<std::string> const& newOpts,
                                  bool isQt5OrLater)
{
  static std::initializer_list<cm::string_view> const valueOpts = {
    "name", "root", "compress", "threshold"
  };
  MergeOptions(baseOpts, newOpts, valueOpts, isQt5OrLater);
}

static void RccListParseContent(std::string const& content,
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

static bool RccListParseOutput(std::string const& rccStdOut,
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
          error = cmStrCat("rcc lists unparsable output:\n",
                           cmQtAutoGen::Quoted(eline), '\n');
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

cmQtAutoGen::RccLister::RccLister() = default;

cmQtAutoGen::RccLister::RccLister(std::string rccExecutable,
                                  std::vector<std::string> listOptions)
  : RccExcutable_(std::move(rccExecutable))
  , ListOptions_(std::move(listOptions))
{
}

bool cmQtAutoGen::RccLister::list(std::string const& qrcFile,
                                  std::vector<std::string>& files,
                                  std::string& error, bool verbose) const
{
  error.clear();

  if (!cmSystemTools::FileExists(qrcFile, true)) {
    error =
      cmStrCat("The resource file ", Quoted(qrcFile), " does not exist.");
    return false;
  }

  // Run rcc list command in the directory of the qrc file with the pathless
  // qrc file name argument.  This way rcc prints relative paths.
  // This avoids issues on Windows when the qrc file is in a path that
  // contains non-ASCII characters.
  std::string const fileDir = cmSystemTools::GetFilenamePath(qrcFile);

  if (!this->RccExcutable_.empty() &&
      cmSystemTools::FileExists(this->RccExcutable_, true) &&
      !this->ListOptions_.empty()) {

    bool result = false;
    int retVal = 0;
    std::string rccStdOut;
    std::string rccStdErr;
    {
      std::vector<std::string> cmd;
      cmd.emplace_back(this->RccExcutable_);
      cm::append(cmd, this->ListOptions_);
      cmd.emplace_back(cmSystemTools::GetFilenameName(qrcFile));

      // Log command
      if (verbose) {
        cmSystemTools::Stdout(
          cmStrCat("Running command:\n", QuotedCommand(cmd), '\n'));
      }

      result = cmSystemTools::RunSingleCommand(
        cmd, &rccStdOut, &rccStdErr, &retVal, fileDir.c_str(),
        cmSystemTools::OUTPUT_NONE, cmDuration::zero(), cmProcessOutput::Auto);
    }
    if (!result || retVal) {
      error =
        cmStrCat("The rcc list process failed for ", Quoted(qrcFile), '\n');
      if (!rccStdOut.empty()) {
        error += cmStrCat(rccStdOut, '\n');
      }
      if (!rccStdErr.empty()) {
        error += cmStrCat(rccStdErr, '\n');
      }
      return false;
    }
    if (!RccListParseOutput(rccStdOut, rccStdErr, files, error)) {
      return false;
    }
  } else {
    // We can't use rcc for the file listing.
    // Read the qrc file content into string and parse it.
    {
      std::string qrcContents;
      {
        cmsys::ifstream ifs(qrcFile.c_str());
        if (ifs) {
          std::ostringstream osst;
          osst << ifs.rdbuf();
          qrcContents = osst.str();
        } else {
          error = cmStrCat("The resource file ", Quoted(qrcFile),
                           " is not readable\n");
          return false;
        }
      }
      // Parse string content
      RccListParseContent(qrcContents, files);
    }
  }

  // Convert relative paths to absolute paths
  for (std::string& entry : files) {
    entry = cmSystemTools::CollapseFullPath(entry, fileDir);
  }
  return true;
}
