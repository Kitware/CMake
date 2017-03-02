/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmQtAutoGeneratorCommon.h"
#include "cmAlgorithms.h"
#include "cmSystemTools.h"

#include <cmsys/FStream.hxx>
#include <cmsys/RegularExpression.hxx>

#include <sstream>

// - Static functions

static std::string utilStripCR(std::string const& line)
{
  // Strip CR characters rcc may have printed (possibly more than one!).
  std::string::size_type cr = line.find('\r');
  if (cr != line.npos) {
    return line.substr(0, cr);
  }
  return line;
}

/// @brief Reads the resource files list from from a .qrc file - Qt4 version
/// @return True if the .qrc file was successfully parsed
static bool RccListInputsQt4(const std::string& fileName,
                             std::vector<std::string>& files)
{
  // Qrc file directory
  std::string qrcDir(cmsys::SystemTools::GetFilenamePath(fileName));
  if (!qrcDir.empty()) {
    qrcDir += '/';
  }

  // Read file into string
  std::string qrcContents;
  {
    std::ostringstream stream;
    stream << cmsys::ifstream(fileName).rdbuf();
    qrcContents = stream.str();
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
  return true;
}

/// @brief Reads the resource files list from from a .qrc file - Qt5 version
/// @return True if the .qrc file was successfully parsed
static bool RccListInputsQt5(const std::string& rccCommand,
                             const std::string& fileName,
                             std::vector<std::string>& files)
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
                                      CM_NULLPTR, cmSystemTools::OUTPUT_NONE);
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
                                      CM_NULLPTR, cmSystemTools::OUTPUT_NONE);
  }
  if (!result || retVal) {
    std::ostringstream err;
    err << "AUTOGEN: error: Rcc list process for " << fileName << " failed:\n"
        << rccStdOut << "\n"
        << rccStdErr << std::endl;
    cmSystemTools::Error(err.str().c_str());
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
          std::ostringstream err;
          err << "AUTOGEN: error: Rcc lists unparsable output " << eline
              << std::endl;
          cmSystemTools::Error(err.str().c_str());
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

const char* cmQtAutoGeneratorCommon::listSep = "@LSEP@";

bool cmQtAutoGeneratorCommon::RccListInputs(const std::string& qtMajorVersion,
                                            const std::string& rccCommand,
                                            const std::string& fileName,
                                            std::vector<std::string>& files)
{
  if (qtMajorVersion == "4") {
    return RccListInputsQt4(fileName, files);
  }
  return RccListInputsQt5(rccCommand, fileName, files);
}
