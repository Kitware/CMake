/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmQtAutoGen.h"

#include "cmAlgorithms.h"
#include "cmDuration.h"
#include "cmProcessOutput.h"
#include "cmSystemTools.h"
#include "cmsys/FStream.hxx"
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
  cmAppend(baseOpts, extraOpts);
}

// - Class definitions

unsigned int const cmQtAutoGen::ParallelMax = 64;
std::string const cmQtAutoGen::ListSep = "<<<S>>>";

std::string const& cmQtAutoGen::GeneratorName(GenT genType)
{
  static const std::string AutoGen("AutoGen");
  static const std::string AutoMoc("AutoMoc");
  static const std::string AutoUic("AutoUic");
  static const std::string AutoRcc("AutoRcc");

  switch (genType) {
    case GenT::GEN:
      return AutoGen;
    case GenT::MOC:
      return AutoMoc;
    case GenT::UIC:
      return AutoUic;
    case GenT::RCC:
      return AutoRcc;
  }
  return AutoGen;
}

std::string const& cmQtAutoGen::GeneratorNameUpper(GenT genType)
{
  static const std::string AUTOGEN("AUTOGEN");
  static const std::string AUTOMOC("AUTOMOC");
  static const std::string AUTOUIC("AUTOUIC");
  static const std::string AUTORCC("AUTORCC");

  switch (genType) {
    case GenT::GEN:
      return AUTOGEN;
    case GenT::MOC:
      return AUTOMOC;
    case GenT::UIC:
      return AUTOUIC;
    case GenT::RCC:
      return AUTORCC;
  }
  return AUTOGEN;
}

std::string cmQtAutoGen::Tools(bool moc, bool uic, bool rcc)
{
  std::string res;
  std::vector<std::string> lst;
  if (moc) {
    lst.emplace_back("AUTOMOC");
  }
  if (uic) {
    lst.emplace_back("AUTOUIC");
  }
  if (rcc) {
    lst.emplace_back("AUTORCC");
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
  std::string::size_type slash_pos = filename.rfind('/');
  if (slash_pos == std::string::npos) {
    return std::string();
  }
  return filename.substr(0, slash_pos + 1);
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
    error = "The resource file ";
    error += Quoted(qrcFile);
    error += " does not exist.";
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
      cmAppend(cmd, this->ListOptions_);
      cmd.emplace_back(cmSystemTools::GetFilenameName(qrcFile));

      // Log command
      if (verbose) {
        std::string msg = "Running command:\n";
        msg += QuotedCommand(cmd);
        msg += '\n';
        cmSystemTools::Stdout(msg);
      }

      result = cmSystemTools::RunSingleCommand(
        cmd, &rccStdOut, &rccStdErr, &retVal, fileDir.c_str(),
        cmSystemTools::OUTPUT_NONE, cmDuration::zero(), cmProcessOutput::Auto);
    }
    if (!result || retVal) {
      error = "The rcc list process failed for ";
      error += Quoted(qrcFile);
      error += "\n";
      if (!rccStdOut.empty()) {
        error += rccStdOut;
        error += "\n";
      }
      if (!rccStdErr.empty()) {
        error += rccStdErr;
        error += "\n";
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
          error = "The resource file ";
          error += Quoted(qrcFile);
          error += " is not readable\n";
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
