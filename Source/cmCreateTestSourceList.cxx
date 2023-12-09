/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCreateTestSourceList.h"

#include <algorithm>

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

bool cmCreateTestSourceList(std::vector<std::string> const& args,
                            cmExecutionStatus& status)
{
  if (args.size() < 3) {
    status.SetError("called with wrong number of arguments.");
    return false;
  }

  auto i = args.begin();
  std::string extraInclude;
  std::string function;
  std::vector<std::string> tests;
  // extract extra include and function ot
  for (; i != args.end(); i++) {
    if (*i == "EXTRA_INCLUDE") {
      ++i;
      if (i == args.end()) {
        status.SetError("incorrect arguments to EXTRA_INCLUDE");
        return false;
      }
      extraInclude = cmStrCat("#include \"", *i, "\"\n");
    } else if (*i == "FUNCTION") {
      ++i;
      if (i == args.end()) {
        status.SetError("incorrect arguments to FUNCTION");
        return false;
      }
      function = cmStrCat(*i, "(&ac, &av);\n");
    } else {
      tests.push_back(*i);
    }
  }
  i = tests.begin();

  // Name of the source list

  const char* sourceList = i->c_str();
  ++i;

  // Name of the test driver
  // make sure they specified an extension
  if (cmSystemTools::GetFilenameExtension(*i).size() < 2) {
    status.SetError(
      "You must specify a file extension for the test driver file.");
    return false;
  }
  cmMakefile& mf = status.GetMakefile();
  std::string driver = cmStrCat(mf.GetCurrentBinaryDirectory(), '/', *i);
  ++i;

  std::string configFile = cmSystemTools::GetCMakeRoot();

  configFile += "/Templates/TestDriver.cxx.in";
  // Create the test driver file

  auto testsBegin = i;
  std::vector<std::string> tests_func_name;

  // The rest of the arguments consist of a list of test source files.
  // Sadly, they can be in directories. Let's find a unique function
  // name for the corresponding test, and push it to the tests_func_name
  // list.
  // For the moment:
  //   - replace spaces ' ', ':' and '/' with underscores '_'
  std::string forwardDeclareCode;
  for (i = testsBegin; i != tests.end(); ++i) {
    if (*i == "EXTRA_INCLUDE") {
      break;
    }
    std::string func_name;
    if (!cmSystemTools::GetFilenamePath(*i).empty()) {
      func_name = cmStrCat(cmSystemTools::GetFilenamePath(*i), '/',
                           cmSystemTools::GetFilenameWithoutLastExtension(*i));
    } else {
      func_name = cmSystemTools::GetFilenameWithoutLastExtension(*i);
    }
    cmSystemTools::ConvertToUnixSlashes(func_name);
    func_name = cmSystemTools::MakeCidentifier(func_name);
    bool already_declared =
      std::find(tests_func_name.begin(), tests_func_name.end(), func_name) !=
      tests_func_name.end();
    tests_func_name.push_back(func_name);
    if (!already_declared) {
      forwardDeclareCode += cmStrCat("int ", func_name, "(int, char*[]);\n");
    }
  }

  std::string functionMapCode;
  std::vector<std::string>::iterator j;
  for (i = testsBegin, j = tests_func_name.begin(); i != tests.end();
       ++i, ++j) {
    std::string func_name;
    if (!cmSystemTools::GetFilenamePath(*i).empty()) {
      func_name = cmStrCat(cmSystemTools::GetFilenamePath(*i), '/',
                           cmSystemTools::GetFilenameWithoutLastExtension(*i));
    } else {
      func_name = cmSystemTools::GetFilenameWithoutLastExtension(*i);
    }
    functionMapCode += "  {\n"
                       "    \"";
    functionMapCode += func_name;
    functionMapCode += "\",\n"
                       "    ";
    functionMapCode += *j;
    functionMapCode += "\n"
                       "  },\n";
  }
  if (!extraInclude.empty()) {
    mf.AddDefinition("CMAKE_TESTDRIVER_EXTRA_INCLUDES", extraInclude);
  }
  if (!function.empty()) {
    mf.AddDefinition("CMAKE_TESTDRIVER_ARGVC_FUNCTION", function);
  }
  mf.AddDefinition("CMAKE_FORWARD_DECLARE_TESTS", forwardDeclareCode);
  mf.AddDefinition("CMAKE_FUNCTION_TABLE_ENTRIES", functionMapCode);
  bool res = true;
  if (!mf.ConfigureFile(configFile, driver, false, true, false)) {
    res = false;
  }

  // Construct the source list.
  std::string sourceListValue;
  {
    cmSourceFile* sf = mf.GetOrCreateSource(driver);
    sf->SetProperty("ABSTRACT", "0");
    sourceListValue = driver;
  }
  for (i = testsBegin; i != tests.end(); ++i) {
    cmSourceFile* sf = mf.GetOrCreateSource(*i);
    sf->SetProperty("ABSTRACT", "0");
    sourceListValue += ';';
    sourceListValue += *i;
  }

  mf.AddDefinition(sourceList, sourceListValue);
  return res;
}
