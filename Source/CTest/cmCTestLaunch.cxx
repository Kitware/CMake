/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestLaunch.h"

#include <cstring>
#include <iostream>

#include "cmsys/FStream.hxx"
#include "cmsys/Process.h"
#include "cmsys/RegularExpression.hxx"

#include "cmCTestLaunchReporter.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmProcessOutput.h"
#include "cmState.h"
#include "cmStateSnapshot.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmake.h"

#ifdef _WIN32
#  include <cstdio> // for std{out,err} and fileno

#  include <fcntl.h> // for _O_BINARY
#  include <io.h>    // for _setmode
#endif

cmCTestLaunch::cmCTestLaunch(int argc, const char* const* argv)
{
  this->Process = nullptr;

  if (!this->ParseArguments(argc, argv)) {
    return;
  }

  this->Reporter.RealArgs = this->RealArgs;
  this->Reporter.ComputeFileNames();

  this->ScrapeRulesLoaded = false;
  this->HaveOut = false;
  this->HaveErr = false;
  this->Process = cmsysProcess_New();
}

cmCTestLaunch::~cmCTestLaunch()
{
  cmsysProcess_Delete(this->Process);
}

bool cmCTestLaunch::ParseArguments(int argc, const char* const* argv)
{
  // Launcher options occur first and are separated from the real
  // command line by a '--' option.
  enum Doing
  {
    DoingNone,
    DoingOutput,
    DoingSource,
    DoingLanguage,
    DoingTargetName,
    DoingTargetType,
    DoingBuildDir,
    DoingCount,
    DoingFilterPrefix
  };
  Doing doing = DoingNone;
  int arg0 = 0;
  for (int i = 1; !arg0 && i < argc; ++i) {
    const char* arg = argv[i];
    if (strcmp(arg, "--") == 0) {
      arg0 = i + 1;
    } else if (strcmp(arg, "--output") == 0) {
      doing = DoingOutput;
    } else if (strcmp(arg, "--source") == 0) {
      doing = DoingSource;
    } else if (strcmp(arg, "--language") == 0) {
      doing = DoingLanguage;
    } else if (strcmp(arg, "--target-name") == 0) {
      doing = DoingTargetName;
    } else if (strcmp(arg, "--target-type") == 0) {
      doing = DoingTargetType;
    } else if (strcmp(arg, "--build-dir") == 0) {
      doing = DoingBuildDir;
    } else if (strcmp(arg, "--filter-prefix") == 0) {
      doing = DoingFilterPrefix;
    } else if (doing == DoingOutput) {
      this->Reporter.OptionOutput = arg;
      doing = DoingNone;
    } else if (doing == DoingSource) {
      this->Reporter.OptionSource = arg;
      doing = DoingNone;
    } else if (doing == DoingLanguage) {
      this->Reporter.OptionLanguage = arg;
      if (this->Reporter.OptionLanguage == "CXX") {
        this->Reporter.OptionLanguage = "C++";
      }
      doing = DoingNone;
    } else if (doing == DoingTargetName) {
      this->Reporter.OptionTargetName = arg;
      doing = DoingNone;
    } else if (doing == DoingTargetType) {
      this->Reporter.OptionTargetType = arg;
      doing = DoingNone;
    } else if (doing == DoingBuildDir) {
      this->Reporter.OptionBuildDir = arg;
      doing = DoingNone;
    } else if (doing == DoingFilterPrefix) {
      this->Reporter.OptionFilterPrefix = arg;
      doing = DoingNone;
    }
  }

  // Extract the real command line.
  if (arg0) {
    this->RealArgC = argc - arg0;
    this->RealArgV = argv + arg0;
    for (int i = 0; i < this->RealArgC; ++i) {
      this->HandleRealArg(this->RealArgV[i]);
    }
    return true;
  }
  this->RealArgC = 0;
  this->RealArgV = nullptr;
  std::cerr << "No launch/command separator ('--') found!\n";
  return false;
}

void cmCTestLaunch::HandleRealArg(const char* arg)
{
#ifdef _WIN32
  // Expand response file arguments.
  if (arg[0] == '@' && cmSystemTools::FileExists(arg + 1)) {
    cmsys::ifstream fin(arg + 1);
    std::string line;
    while (cmSystemTools::GetLineFromStream(fin, line)) {
      cmSystemTools::ParseWindowsCommandLine(line.c_str(), this->RealArgs);
    }
    return;
  }
#endif
  this->RealArgs.emplace_back(arg);
}

void cmCTestLaunch::RunChild()
{
  // Ignore noopt make rules
  if (this->RealArgs.empty() || this->RealArgs[0] == ":") {
    this->Reporter.ExitCode = 0;
    return;
  }

  // Prepare to run the real command.
  cmsysProcess* cp = this->Process;
  cmsysProcess_SetCommand(cp, this->RealArgV);

  cmsys::ofstream fout;
  cmsys::ofstream ferr;
  if (this->Reporter.Passthru) {
    // In passthru mode we just share the output pipes.
    cmsysProcess_SetPipeShared(cp, cmsysProcess_Pipe_STDOUT, 1);
    cmsysProcess_SetPipeShared(cp, cmsysProcess_Pipe_STDERR, 1);
  } else {
    // In full mode we record the child output pipes to log files.
    fout.open(this->Reporter.LogOut.c_str(), std::ios::out | std::ios::binary);
    ferr.open(this->Reporter.LogErr.c_str(), std::ios::out | std::ios::binary);
  }

#ifdef _WIN32
  // Do this so that newline transformation is not done when writing to cout
  // and cerr below.
  _setmode(fileno(stdout), _O_BINARY);
  _setmode(fileno(stderr), _O_BINARY);
#endif

  // Run the real command.
  cmsysProcess_Execute(cp);

  // Record child stdout and stderr if necessary.
  if (!this->Reporter.Passthru) {
    char* data = nullptr;
    int length = 0;
    cmProcessOutput processOutput;
    std::string strdata;
    while (int p = cmsysProcess_WaitForData(cp, &data, &length, nullptr)) {
      if (p == cmsysProcess_Pipe_STDOUT) {
        processOutput.DecodeText(data, length, strdata, 1);
        fout.write(strdata.c_str(), strdata.size());
        std::cout.write(strdata.c_str(), strdata.size());
        this->HaveOut = true;
      } else if (p == cmsysProcess_Pipe_STDERR) {
        processOutput.DecodeText(data, length, strdata, 2);
        ferr.write(strdata.c_str(), strdata.size());
        std::cerr.write(strdata.c_str(), strdata.size());
        this->HaveErr = true;
      }
    }
    processOutput.DecodeText(std::string(), strdata, 1);
    if (!strdata.empty()) {
      fout.write(strdata.c_str(), strdata.size());
      std::cout.write(strdata.c_str(), strdata.size());
    }
    processOutput.DecodeText(std::string(), strdata, 2);
    if (!strdata.empty()) {
      ferr.write(strdata.c_str(), strdata.size());
      std::cerr.write(strdata.c_str(), strdata.size());
    }
  }

  // Wait for the real command to finish.
  cmsysProcess_WaitForExit(cp, nullptr);
  this->Reporter.ExitCode = cmsysProcess_GetExitValue(cp);
}

int cmCTestLaunch::Run()
{
  if (!this->Process) {
    std::cerr << "Could not allocate cmsysProcess instance!\n";
    return -1;
  }

  this->RunChild();

  if (this->CheckResults()) {
    return this->Reporter.ExitCode;
  }

  this->LoadConfig();
  this->Reporter.Process = this->Process;
  this->Reporter.WriteXML();

  return this->Reporter.ExitCode;
}

bool cmCTestLaunch::CheckResults()
{
  // Skip XML in passthru mode.
  if (this->Reporter.Passthru) {
    return true;
  }

  // We always report failure for error conditions.
  if (this->Reporter.IsError()) {
    return false;
  }

  // Scrape the output logs to look for warnings.
  if ((this->HaveErr && this->ScrapeLog(this->Reporter.LogErr)) ||
      (this->HaveOut && this->ScrapeLog(this->Reporter.LogOut))) {
    return false;
  }
  return true;
}

void cmCTestLaunch::LoadScrapeRules()
{
  if (this->ScrapeRulesLoaded) {
    return;
  }
  this->ScrapeRulesLoaded = true;

  // Load custom match rules given to us by CTest.
  this->LoadScrapeRules("Warning", this->Reporter.RegexWarning);
  this->LoadScrapeRules("WarningSuppress",
                        this->Reporter.RegexWarningSuppress);
}

void cmCTestLaunch::LoadScrapeRules(
  const char* purpose, std::vector<cmsys::RegularExpression>& regexps) const
{
  std::string fname =
    cmStrCat(this->Reporter.LogDir, "Custom", purpose, ".txt");
  cmsys::ifstream fin(fname.c_str(), std::ios::in | std::ios::binary);
  std::string line;
  cmsys::RegularExpression rex;
  while (cmSystemTools::GetLineFromStream(fin, line)) {
    if (rex.compile(line)) {
      regexps.push_back(rex);
    }
  }
}

bool cmCTestLaunch::ScrapeLog(std::string const& fname)
{
  this->LoadScrapeRules();

  // Look for log file lines matching warning expressions but not
  // suppression expressions.
  cmsys::ifstream fin(fname.c_str(), std::ios::in | std::ios::binary);
  std::string line;
  while (cmSystemTools::GetLineFromStream(fin, line)) {
    if (this->Reporter.MatchesFilterPrefix(line)) {
      continue;
    }

    if (this->Reporter.Match(line, this->Reporter.RegexWarning) &&
        !this->Reporter.Match(line, this->Reporter.RegexWarningSuppress)) {
      return true;
    }
  }
  return false;
}

int cmCTestLaunch::Main(int argc, const char* const argv[])
{
  if (argc == 2) {
    std::cerr << "ctest --launch: this mode is for internal CTest use only"
              << std::endl;
    return 1;
  }
  cmCTestLaunch self(argc, argv);
  return self.Run();
}

void cmCTestLaunch::LoadConfig()
{
  cmake cm(cmake::RoleScript, cmState::CTest);
  cm.SetHomeDirectory("");
  cm.SetHomeOutputDirectory("");
  cm.GetCurrentSnapshot().SetDefaultDefinitions();
  cmGlobalGenerator gg(&cm);
  cmMakefile mf(&gg, cm.GetCurrentSnapshot());
  std::string fname =
    cmStrCat(this->Reporter.LogDir, "CTestLaunchConfig.cmake");
  if (cmSystemTools::FileExists(fname) && mf.ReadListFile(fname)) {
    this->Reporter.SourceDir = mf.GetSafeDefinition("CTEST_SOURCE_DIRECTORY");
    cmSystemTools::ConvertToUnixSlashes(this->Reporter.SourceDir);
  }
}
