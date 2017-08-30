/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmQtAutoGen.h"
#include "cmAlgorithms.h"
#include "cmSystemTools.h"

#include "cmsys/FStream.hxx"
#include "cmsys/RegularExpression.hxx"

#include <algorithm>
#include <sstream>
#include <stddef.h>

// - Static variables

const std::string genNameGen = "AutoGen";
const std::string genNameMoc = "AutoMoc";
const std::string genNameUic = "AutoUic";
const std::string genNameRcc = "AutoRcc";

// - Static functions

/// @brief Merges newOpts into baseOpts
/// @arg valueOpts list of options that accept a value
void MergeOptions(std::vector<std::string>& baseOpts,
                  const std::vector<std::string>& newOpts,
                  const std::vector<std::string>& valueOpts, bool isQt5)
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
    const std::string& newOpt = *fit;
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
        if (!optName.empty() && (std::find(valueOpts.begin(), valueOpts.end(),
                                           optName) != valueOpts.end())) {
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

static std::string utilStripCR(std::string const& line)
{
  // Strip CR characters rcc may have printed (possibly more than one!).
  std::string::size_type cr = line.find('\r');
  if (cr != std::string::npos) {
    return line.substr(0, cr);
  }
  return line;
}

/// @brief Reads the resource files list from from a .qrc file - Qt4 version
/// @return True if the .qrc file was successfully parsed
static bool RccListInputsQt4(const std::string& fileName,
                             std::vector<std::string>& files,
                             std::string* errorMessage)
{
  bool allGood = true;
  // Read qrc file content into string
  std::string qrcContents;
  {
    cmsys::ifstream ifs(fileName.c_str());
    if (ifs) {
      std::ostringstream osst;
      osst << ifs.rdbuf();
      qrcContents = osst.str();
    } else {
      if (errorMessage != nullptr) {
        std::ostringstream ost;
        ost << "AutoRcc: Error: Rcc file not readable:\n"
            << cmQtAutoGen::Quoted(fileName) << "\n";
        *errorMessage = ost.str();
      }
      allGood = false;
    }
  }
  if (allGood) {
    // qrc file directory
    std::string qrcDir(cmSystemTools::GetFilenamePath(fileName));
    if (!qrcDir.empty()) {
      qrcDir += '/';
    }

    cmsys::RegularExpression fileMatchRegex("(<file[^<]+)");
    cmsys::RegularExpression fileReplaceRegex("(^<file[^>]*>)");

    size_t offset = 0;
    while (fileMatchRegex.find(qrcContents.c_str() + offset)) {
      std::string qrcEntry = fileMatchRegex.match(1);
      offset += qrcEntry.size();
      {
        fileReplaceRegex.find(qrcEntry);
        std::string tag = fileReplaceRegex.match(1);
        qrcEntry = qrcEntry.substr(tag.size());
      }
      if (!cmSystemTools::FileIsFullPath(qrcEntry.c_str())) {
        qrcEntry = qrcDir + qrcEntry;
      }
      files.push_back(qrcEntry);
    }
  }
  return allGood;
}

/// @brief Reads the resource files list from from a .qrc file - Qt5 version
/// @return True if the .qrc file was successfully parsed
static bool RccListInputsQt5(const std::string& rccCommand,
                             const std::string& fileName,
                             std::vector<std::string>& files,
                             std::string* errorMessage)
{
  if (rccCommand.empty()) {
    cmSystemTools::Error("AutoRcc: Error: rcc executable not available\n");
    return false;
  }

  // Read rcc features
  bool hasDashDashList = false;
  {
    std::vector<std::string> command;
    command.push_back(rccCommand);
    command.push_back("--help");
    std::string rccStdOut;
    std::string rccStdErr;
    int retVal = 0;
    bool result =
      cmSystemTools::RunSingleCommand(command, &rccStdOut, &rccStdErr, &retVal,
                                      nullptr, cmSystemTools::OUTPUT_NONE);
    if (result && retVal == 0 &&
        rccStdOut.find("--list") != std::string::npos) {
      hasDashDashList = true;
    }
  }

  // Run rcc list command
  bool result = false;
  int retVal = 0;
  std::string rccStdOut;
  std::string rccStdErr;
  {
    std::vector<std::string> command;
    command.push_back(rccCommand);
    command.push_back(hasDashDashList ? "--list" : "-list");
    command.push_back(fileName);
    result =
      cmSystemTools::RunSingleCommand(command, &rccStdOut, &rccStdErr, &retVal,
                                      nullptr, cmSystemTools::OUTPUT_NONE);
  }
  if (!result || retVal) {
    if (errorMessage != nullptr) {
      std::ostringstream ost;
      ost << "AutoRcc: Error: Rcc list process for " << fileName
          << " failed:\n"
          << rccStdOut << "\n"
          << rccStdErr << "\n";
      *errorMessage = ost.str();
    }
    return false;
  }

  // Parse rcc std output
  {
    std::istringstream ostr(rccStdOut);
    std::string oline;
    while (std::getline(ostr, oline)) {
      oline = utilStripCR(oline);
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
      eline = utilStripCR(eline);
      if (cmHasLiteralPrefix(eline, "RCC: Error in")) {
        static std::string searchString = "Cannot find file '";

        std::string::size_type pos = eline.find(searchString);
        if (pos == std::string::npos) {
          if (errorMessage != nullptr) {
            std::ostringstream ost;
            ost << "AutoRcc: Error: Rcc lists unparsable output:\n"
                << cmQtAutoGen::Quoted(eline) << "\n";
            *errorMessage = ost.str();
          }
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

// - Class definitions

const std::string cmQtAutoGen::listSep = "@LSEP@";

const std::string& cmQtAutoGen::GeneratorName(GeneratorType type)
{
  switch (type) {
    case GeneratorType::GEN:
      return genNameGen;
    case GeneratorType::MOC:
      return genNameMoc;
    case GeneratorType::UIC:
      return genNameUic;
    case GeneratorType::RCC:
      return genNameRcc;
  }
  return genNameGen;
}

std::string cmQtAutoGen::GeneratorNameUpper(GeneratorType genType)
{
  return cmSystemTools::UpperCase(cmQtAutoGen::GeneratorName(genType));
}

std::string cmQtAutoGen::Quoted(const std::string& text)
{
  static const char* rep[18] = { "\\", "\\\\", "\"", "\\\"", "\a", "\\a",
                                 "\b", "\\b",  "\f", "\\f",  "\n", "\\n",
                                 "\r", "\\r",  "\t", "\\t",  "\v", "\\v" };

  std::string res = text;
  for (const char* const* it = cmArrayBegin(rep); it != cmArrayEnd(rep);
       it += 2) {
    cmSystemTools::ReplaceString(res, *it, *(it + 1));
  }
  res = '"' + res;
  res += '"';
  return res;
}

void cmQtAutoGen::UicMergeOptions(std::vector<std::string>& baseOpts,
                                  const std::vector<std::string>& newOpts,
                                  bool isQt5)
{
  static const std::vector<std::string> valueOpts = {
    "tr",      "translate", "postfix", "generator",
    "include", // Since Qt 5.3
    "g"
  };
  MergeOptions(baseOpts, newOpts, valueOpts, isQt5);
}

void cmQtAutoGen::RccMergeOptions(std::vector<std::string>& baseOpts,
                                  const std::vector<std::string>& newOpts,
                                  bool isQt5)
{
  static const std::vector<std::string> valueOpts = { "name", "root",
                                                      "compress",
                                                      "threshold" };
  MergeOptions(baseOpts, newOpts, valueOpts, isQt5);
}

bool cmQtAutoGen::RccListInputs(const std::string& qtMajorVersion,
                                const std::string& rccCommand,
                                const std::string& fileName,
                                std::vector<std::string>& files,
                                std::string* errorMessage)
{
  bool allGood = false;
  if (cmSystemTools::FileExists(fileName.c_str())) {
    if (qtMajorVersion == "4") {
      allGood = RccListInputsQt4(fileName, files, errorMessage);
    } else {
      allGood = RccListInputsQt5(rccCommand, fileName, files, errorMessage);
    }
  } else {
    if (errorMessage != nullptr) {
      std::ostringstream ost;
      ost << "AutoRcc: Error: Rcc file does not exist:\n"
          << cmQtAutoGen::Quoted(fileName) << "\n";
      *errorMessage = ost.str();
    }
  }
  return allGood;
}
