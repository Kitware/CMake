/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include <functional>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <cm3p/cppdap/protocol.h>
#include <cm3p/cppdap/types.h>
#include <stddef.h>
#include <stdint.h>

#include "cmDebuggerStackFrame.h"
#include "cmDebuggerVariables.h"
#include "cmDebuggerVariablesHelper.h"
#include "cmDebuggerVariablesManager.h"
#include "cmFileSet.h"
#include "cmGlobalGenerator.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmPolicies.h"
#include "cmPropertyMap.h"
#include "cmState.h"
#include "cmStateDirectory.h"
#include "cmStateSnapshot.h"
#include "cmStateTypes.h"
#include "cmTarget.h"
#include "cmTest.h"
#include "cmake.h"

#include "testCommon.h"
#include "testDebugger.h"

static dap::VariablesRequest CreateVariablesRequest(int64_t reference)
{
  dap::VariablesRequest variableRequest;
  variableRequest.variablesReference = reference;
  return variableRequest;
}

struct Dummies
{
  std::shared_ptr<cmake> CMake;
  std::shared_ptr<cmMakefile> Makefile;
  std::shared_ptr<cmGlobalGenerator> GlobalGenerator;
};

static Dummies CreateDummies(
  std::string targetName,
  std::string currentSourceDirectory = "c:/CurrentSourceDirectory",
  std::string currentBinaryDirectory = "c:/CurrentBinaryDirectory")
{
  Dummies dummies;
  dummies.CMake =
    std::make_shared<cmake>(cmake::RoleProject, cmState::Project);
  cmState* state = dummies.CMake->GetState();
  dummies.GlobalGenerator =
    std::make_shared<cmGlobalGenerator>(dummies.CMake.get());
  cmStateSnapshot snapshot = state->CreateBaseSnapshot();
  snapshot.GetDirectory().SetCurrentSource(currentSourceDirectory);
  snapshot.GetDirectory().SetCurrentBinary(currentBinaryDirectory);
  dummies.Makefile =
    std::make_shared<cmMakefile>(dummies.GlobalGenerator.get(), snapshot);
  dummies.Makefile->CreateNewTarget(targetName, cmStateEnums::EXECUTABLE);
  return dummies;
}

static bool testCreateFromPolicyMap()
{
  auto variablesManager =
    std::make_shared<cmDebugger::cmDebuggerVariablesManager>();

  cmPolicies::PolicyMap policyMap;
  policyMap.Set(cmPolicies::CMP0000, cmPolicies::NEW);
  policyMap.Set(cmPolicies::CMP0003, cmPolicies::WARN);
  policyMap.Set(cmPolicies::CMP0005, cmPolicies::OLD);
  auto vars = cmDebugger::cmDebuggerVariablesHelper::Create(
    variablesManager, "Locals", true, policyMap);

  dap::array<dap::Variable> variables =
    variablesManager->HandleVariablesRequest(
      CreateVariablesRequest(vars->GetId()));
  ASSERT_TRUE(variables.size() == 3);
  ASSERT_VARIABLE(variables[0], "CMP0000", "NEW", "string");
  ASSERT_VARIABLE(variables[1], "CMP0003", "WARN", "string");
  ASSERT_VARIABLE(variables[2], "CMP0005", "OLD", "string");

  return true;
}

static bool testCreateFromPairVector()
{
  auto variablesManager =
    std::make_shared<cmDebugger::cmDebuggerVariablesManager>();

  std::vector<std::pair<std::string, std::string>> pairs = {
    { "Foo1", "Bar1" }, { "Foo2", "Bar2" }
  };

  auto vars = cmDebugger::cmDebuggerVariablesHelper::CreateIfAny(
    variablesManager, "Locals", true, pairs);

  dap::array<dap::Variable> variables =
    variablesManager->HandleVariablesRequest(
      CreateVariablesRequest(vars->GetId()));

  ASSERT_TRUE(vars->GetValue() == std::to_string(pairs.size()));
  ASSERT_TRUE(variables.size() == 2);
  ASSERT_VARIABLE(variables[0], "Foo1", "Bar1", "string");
  ASSERT_VARIABLE(variables[1], "Foo2", "Bar2", "string");

  auto none = cmDebugger::cmDebuggerVariablesHelper::CreateIfAny(
    variablesManager, "Locals", true,
    std::vector<std::pair<std::string, std::string>>());

  ASSERT_TRUE(none == nullptr);

  return true;
}

static bool testCreateFromSet()
{
  auto variablesManager =
    std::make_shared<cmDebugger::cmDebuggerVariablesManager>();

  std::set<std::string> set = { "Foo", "Bar" };

  auto vars = cmDebugger::cmDebuggerVariablesHelper::CreateIfAny(
    variablesManager, "Locals", true, set);

  dap::array<dap::Variable> variables =
    variablesManager->HandleVariablesRequest(
      CreateVariablesRequest(vars->GetId()));

  ASSERT_TRUE(vars->GetValue() == std::to_string(set.size()));
  ASSERT_TRUE(variables.size() == 2);
  ASSERT_VARIABLE(variables[0], "[0]", "Bar", "string");
  ASSERT_VARIABLE(variables[1], "[1]", "Foo", "string");

  auto none = cmDebugger::cmDebuggerVariablesHelper::CreateIfAny(
    variablesManager, "Locals", true, std::set<std::string>());

  ASSERT_TRUE(none == nullptr);

  return true;
}

static bool testCreateFromStringVector()
{
  auto variablesManager =
    std::make_shared<cmDebugger::cmDebuggerVariablesManager>();

  std::vector<std::string> list = { "Foo", "Bar" };

  auto vars = cmDebugger::cmDebuggerVariablesHelper::CreateIfAny(
    variablesManager, "Locals", true, list);

  dap::array<dap::Variable> variables =
    variablesManager->HandleVariablesRequest(
      CreateVariablesRequest(vars->GetId()));

  ASSERT_TRUE(vars->GetValue() == std::to_string(list.size()));
  ASSERT_TRUE(variables.size() == 2);
  ASSERT_VARIABLE(variables[0], "[0]", "Foo", "string");
  ASSERT_VARIABLE(variables[1], "[1]", "Bar", "string");

  auto none = cmDebugger::cmDebuggerVariablesHelper::CreateIfAny(
    variablesManager, "Locals", true, std::vector<std::string>());

  ASSERT_TRUE(none == nullptr);

  return true;
}

static bool testCreateFromTarget()
{
  auto variablesManager =
    std::make_shared<cmDebugger::cmDebuggerVariablesManager>();

  auto dummies = CreateDummies("Foo");

  auto vars = cmDebugger::cmDebuggerVariablesHelper::CreateIfAny(
    variablesManager, "Locals", true, dummies.Makefile->GetOrderedTargets());

  dap::array<dap::Variable> variables =
    variablesManager->HandleVariablesRequest(
      CreateVariablesRequest(vars->GetId()));

  ASSERT_TRUE(variables.size() == 1);
  ASSERT_VARIABLE(variables[0], "Foo", "EXECUTABLE", "collection");

  variables = variablesManager->HandleVariablesRequest(
    CreateVariablesRequest(variables[0].variablesReference));

  ASSERT_TRUE(variables.size() == 15);
  ASSERT_VARIABLE(variables[0], "GlobalGenerator", "Generic", "collection");
  ASSERT_VARIABLE(variables[1], "IsAIX", "FALSE", "bool");
  ASSERT_VARIABLE(variables[2], "IsAndroidGuiExecutable", "FALSE", "bool");
  ASSERT_VARIABLE(variables[3], "IsAppBundleOnApple", "FALSE", "bool");
  ASSERT_VARIABLE(variables[4], "IsDLLPlatform", "FALSE", "bool");
  ASSERT_VARIABLE(variables[5], "IsExecutableWithExports", "FALSE", "bool");
  ASSERT_VARIABLE(variables[6], "IsFrameworkOnApple", "FALSE", "bool");
  ASSERT_VARIABLE(variables[7], "IsImported", "FALSE", "bool");
  ASSERT_VARIABLE(variables[8], "IsImportedGloballyVisible", "FALSE", "bool");
  ASSERT_VARIABLE(variables[9], "IsPerConfig", "TRUE", "bool");
  ASSERT_VARIABLE(variables[10], "Makefile",
                  dummies.Makefile->GetDirectoryId().String, "collection");
  ASSERT_VARIABLE(variables[11], "Name", "Foo", "string");
  ASSERT_VARIABLE(variables[12], "PolicyMap", "", "collection");
  ASSERT_VARIABLE(variables[13], "Properties",
                  std::to_string(dummies.Makefile->GetOrderedTargets()[0]
                                   ->GetProperties()
                                   .GetList()
                                   .size()),
                  "collection");
  ASSERT_VARIABLE(variables[14], "Type", "EXECUTABLE", "string");

  auto none = cmDebugger::cmDebuggerVariablesHelper::CreateIfAny(
    variablesManager, "Locals", true, std::vector<cmTarget*>());

  ASSERT_TRUE(none == nullptr);

  return true;
}

static bool testCreateFromGlobalGenerator()
{
  auto variablesManager =
    std::make_shared<cmDebugger::cmDebuggerVariablesManager>();

  auto dummies = CreateDummies("Foo");

  auto vars = cmDebugger::cmDebuggerVariablesHelper::CreateIfAny(
    variablesManager, "Locals", true, dummies.GlobalGenerator.get());

  dap::array<dap::Variable> variables =
    variablesManager->HandleVariablesRequest(
      CreateVariablesRequest(vars->GetId()));

  ASSERT_TRUE(variables.size() == 10);
  ASSERT_VARIABLE(variables[0], "AllTargetName", "ALL_BUILD", "string");
  ASSERT_VARIABLE(variables[1], "ForceUnixPaths", "FALSE", "bool");
  ASSERT_VARIABLE(variables[2], "InstallTargetName", "INSTALL", "string");
  ASSERT_VARIABLE(variables[3], "IsMultiConfig", "FALSE", "bool");
  ASSERT_VARIABLE(variables[4], "MakefileEncoding", "None", "string");
  ASSERT_VARIABLE(variables[5], "Name", "Generic", "string");
  ASSERT_VARIABLE(variables[6], "NeedSymbolicMark", "FALSE", "bool");
  ASSERT_VARIABLE(variables[7], "PackageTargetName", "PACKAGE", "string");
  ASSERT_VARIABLE(variables[8], "TestTargetName", "RUN_TESTS", "string");
  ASSERT_VARIABLE(variables[9], "UseLinkScript", "FALSE", "bool");

  auto none = cmDebugger::cmDebuggerVariablesHelper::CreateIfAny(
    variablesManager, "Locals", true,
    static_cast<cmGlobalGenerator*>(nullptr));

  ASSERT_TRUE(none == nullptr);

  return true;
}

static bool testCreateFromTests()
{
  auto variablesManager =
    std::make_shared<cmDebugger::cmDebuggerVariablesManager>();

  auto dummies = CreateDummies("Foo");
  cmTest test1 = cmTest(dummies.Makefile.get());
  test1.SetName("Test1");
  test1.SetOldStyle(false);
  test1.SetCommandExpandLists(true);
  test1.SetCommand(std::vector<std::string>{ "Foo1", "arg1" });
  test1.SetProperty("Prop1", "Prop1");
  cmTest test2 = cmTest(dummies.Makefile.get());
  test2.SetName("Test2");
  test2.SetOldStyle(false);
  test2.SetCommandExpandLists(false);
  test2.SetCommand(std::vector<std::string>{ "Bar1", "arg1", "arg2" });
  test2.SetProperty("Prop2", "Prop2");
  test2.SetProperty("Prop3", "Prop3");

  std::vector<cmTest*> tests = { &test1, &test2 };

  auto vars = cmDebugger::cmDebuggerVariablesHelper::CreateIfAny(
    variablesManager, "Locals", true, tests);

  dap::array<dap::Variable> variables =
    variablesManager->HandleVariablesRequest(
      CreateVariablesRequest(vars->GetId()));

  ASSERT_TRUE(vars->GetValue() == std::to_string(tests.size()));
  ASSERT_TRUE(variables.size() == 2);
  ASSERT_VARIABLE_REFERENCE_NOT_ZERO(variables[0], test1.GetName(), "",
                                     "collection");
  ASSERT_VARIABLE_REFERENCE_NOT_ZERO(variables[1], test2.GetName(), "",
                                     "collection");

  dap::array<dap::Variable> testVariables =
    variablesManager->HandleVariablesRequest(
      CreateVariablesRequest(variables[0].variablesReference));
  ASSERT_TRUE(testVariables.size() == 5);
  ASSERT_VARIABLE_REFERENCE_NOT_ZERO(testVariables[0], "Command",
                                     std::to_string(test1.GetCommand().size()),
                                     "collection");
  ASSERT_VARIABLE(testVariables[1], "CommandExpandLists",
                  BOOL_STRING(test1.GetCommandExpandLists()), "bool");
  ASSERT_VARIABLE(testVariables[2], "Name", test1.GetName(), "string");
  ASSERT_VARIABLE(testVariables[3], "OldStyle",
                  BOOL_STRING(test1.GetOldStyle()), "bool");
  ASSERT_VARIABLE_REFERENCE_NOT_ZERO(testVariables[4], "Properties", "1",
                                     "collection");

  dap::array<dap::Variable> commandVariables =
    variablesManager->HandleVariablesRequest(
      CreateVariablesRequest(testVariables[0].variablesReference));
  ASSERT_TRUE(commandVariables.size() == test1.GetCommand().size());
  for (size_t i = 0; i < commandVariables.size(); ++i) {
    ASSERT_VARIABLE(commandVariables[i], "[" + std::to_string(i) + "]",
                    test1.GetCommand()[i], "string");
  }

  dap::array<dap::Variable> propertiesVariables =
    variablesManager->HandleVariablesRequest(
      CreateVariablesRequest(testVariables[4].variablesReference));
  ASSERT_TRUE(propertiesVariables.size() == 1);
  ASSERT_VARIABLE(propertiesVariables[0], "Prop1", "Prop1", "string");

  testVariables = variablesManager->HandleVariablesRequest(
    CreateVariablesRequest(variables[1].variablesReference));
  ASSERT_TRUE(testVariables.size() == 5);
  ASSERT_VARIABLE_REFERENCE_NOT_ZERO(testVariables[0], "Command",
                                     std::to_string(test2.GetCommand().size()),
                                     "collection");
  ASSERT_VARIABLE(testVariables[1], "CommandExpandLists",
                  BOOL_STRING(test2.GetCommandExpandLists()), "bool");
  ASSERT_VARIABLE(testVariables[2], "Name", test2.GetName(), "string");
  ASSERT_VARIABLE(testVariables[3], "OldStyle",
                  BOOL_STRING(test2.GetOldStyle()), "bool");
  ASSERT_VARIABLE_REFERENCE_NOT_ZERO(testVariables[4], "Properties", "2",
                                     "collection");

  commandVariables = variablesManager->HandleVariablesRequest(
    CreateVariablesRequest(testVariables[0].variablesReference));
  ASSERT_TRUE(commandVariables.size() == test2.GetCommand().size());
  for (size_t i = 0; i < commandVariables.size(); ++i) {
    ASSERT_VARIABLE(commandVariables[i], "[" + std::to_string(i) + "]",
                    test2.GetCommand()[i], "string");
  }

  propertiesVariables = variablesManager->HandleVariablesRequest(
    CreateVariablesRequest(testVariables[4].variablesReference));
  ASSERT_TRUE(propertiesVariables.size() == 2);
  ASSERT_VARIABLE(propertiesVariables[0], "Prop2", "Prop2", "string");
  ASSERT_VARIABLE(propertiesVariables[1], "Prop3", "Prop3", "string");

  auto none = cmDebugger::cmDebuggerVariablesHelper::CreateIfAny(
    variablesManager, "Locals", true, std::vector<cmTest*>());

  ASSERT_TRUE(none == nullptr);

  return true;
}

static bool testCreateFromMakefile()
{
  auto variablesManager =
    std::make_shared<cmDebugger::cmDebuggerVariablesManager>();

  auto dummies = CreateDummies("Foo");
  auto snapshot = dummies.Makefile->GetStateSnapshot();
  auto state = dummies.Makefile->GetState();
  state->SetSourceDirectory("c:/HomeDirectory");
  state->SetBinaryDirectory("c:/HomeOutputDirectory");
  auto vars = cmDebugger::cmDebuggerVariablesHelper::CreateIfAny(
    variablesManager, "Locals", true, dummies.Makefile.get());

  dap::array<dap::Variable> variables =
    variablesManager->HandleVariablesRequest(
      CreateVariablesRequest(vars->GetId()));

  ASSERT_TRUE(variables.size() == 12);
  ASSERT_VARIABLE(variables[0], "AppleSDKType", "MacOS", "string");
  ASSERT_VARIABLE(variables[1], "CurrentBinaryDirectory",
                  snapshot.GetDirectory().GetCurrentBinary(), "string");
  ASSERT_VARIABLE(variables[2], "CurrentSourceDirectory",
                  snapshot.GetDirectory().GetCurrentSource(), "string");
  ASSERT_VARIABLE(variables[3], "DefineFlags", " ", "string");
  ASSERT_VARIABLE(variables[4], "DirectoryId",
                  dummies.Makefile->GetDirectoryId().String, "string");
  ASSERT_VARIABLE(variables[5], "HomeDirectory", state->GetSourceDirectory(),
                  "string");
  ASSERT_VARIABLE(variables[6], "HomeOutputDirectory",
                  state->GetBinaryDirectory(), "string");
  ASSERT_VARIABLE(variables[7], "IsRootMakefile", "TRUE", "bool");
  ASSERT_VARIABLE(variables[8], "PlatformIs32Bit", "FALSE", "bool");
  ASSERT_VARIABLE(variables[9], "PlatformIs64Bit", "FALSE", "bool");
  ASSERT_VARIABLE(variables[10], "PlatformIsAppleEmbedded", "FALSE", "bool");
  ASSERT_VARIABLE(variables[11], "PlatformIsx32", "FALSE", "bool");

  auto none = cmDebugger::cmDebuggerVariablesHelper::CreateIfAny(
    variablesManager, "Locals", true, static_cast<cmMakefile*>(nullptr));

  ASSERT_TRUE(none == nullptr);

  return true;
}

static bool testCreateFromStackFrame()
{
  auto variablesManager =
    std::make_shared<cmDebugger::cmDebuggerVariablesManager>();
  auto dummies = CreateDummies("Foo");

  cmListFileFunction lff = cmListFileFunction("set", 99, 99, {});
  auto frame = std::make_shared<cmDebugger::cmDebuggerStackFrame>(
    dummies.Makefile.get(), "c:/CMakeLists.txt", lff);

  dummies.CMake->AddCacheEntry("CMAKE_BUILD_TYPE", "Debug", "Build Type",
                               cmStateEnums::CacheEntryType::STRING);

  auto locals = cmDebugger::cmDebuggerVariablesHelper::Create(
    variablesManager, "Locals", true, frame);

  dap::array<dap::Variable> variables =
    variablesManager->HandleVariablesRequest(
      CreateVariablesRequest(locals->GetId()));

  ASSERT_TRUE(variables.size() == 5);
  ASSERT_VARIABLE(variables[0], "CacheVariables", "1", "collection");
  ASSERT_VARIABLE(variables[1], "CurrentLine", std::to_string(lff.Line()),
                  "int");
  ASSERT_VARIABLE(variables[2], "Directories", "2", "collection");
  ASSERT_VARIABLE(variables[3], "Locals", "2", "collection");
  ASSERT_VARIABLE(variables[4], "Targets", "1", "collection");

  dap::array<dap::Variable> cacheVariables =
    variablesManager->HandleVariablesRequest(
      CreateVariablesRequest(variables[0].variablesReference));
  ASSERT_TRUE(cacheVariables.size() == 1);
  ASSERT_VARIABLE(cacheVariables[0], "CMAKE_BUILD_TYPE:STRING", "Debug",
                  "collection");

  dap::array<dap::Variable> directoriesVariables =
    variablesManager->HandleVariablesRequest(
      CreateVariablesRequest(variables[2].variablesReference));
  ASSERT_TRUE(directoriesVariables.size() == 2);
  ASSERT_VARIABLE(
    directoriesVariables[0], "CMAKE_CURRENT_BINARY_DIR",
    dummies.Makefile->GetStateSnapshot().GetDirectory().GetCurrentBinary(),
    "string");
  ASSERT_VARIABLE(
    directoriesVariables[1], "CMAKE_CURRENT_SOURCE_DIR",
    dummies.Makefile->GetStateSnapshot().GetDirectory().GetCurrentSource(),
    "string");

  dap::array<dap::Variable> propertiesVariables =
    variablesManager->HandleVariablesRequest(
      CreateVariablesRequest(cacheVariables[0].variablesReference));
  ASSERT_TRUE(propertiesVariables.size() == 3);
  ASSERT_VARIABLE(propertiesVariables[0], "HELPSTRING", "Build Type",
                  "string");
  ASSERT_VARIABLE(propertiesVariables[1], "TYPE", "STRING", "string");
  ASSERT_VARIABLE(propertiesVariables[2], "VALUE", "Debug", "string");

  return true;
}

static bool testCreateFromBTStringVector()
{
  auto variablesManager =
    std::make_shared<cmDebugger::cmDebuggerVariablesManager>();

  std::vector<BT<std::string>> list(2);
  list[0].Value = "Foo";
  list[1].Value = "Bar";

  auto vars = cmDebugger::cmDebuggerVariablesHelper::CreateIfAny(
    variablesManager, "Locals", true, list);

  dap::array<dap::Variable> variables =
    variablesManager->HandleVariablesRequest(
      CreateVariablesRequest(vars->GetId()));

  ASSERT_TRUE(vars->GetValue() == std::to_string(list.size()));
  ASSERT_TRUE(variables.size() == 2);
  ASSERT_VARIABLE(variables[0], "[0]", "Foo", "string");
  ASSERT_VARIABLE(variables[1], "[1]", "Bar", "string");

  auto none = cmDebugger::cmDebuggerVariablesHelper::CreateIfAny(
    variablesManager, "Locals", true, std::vector<std::string>());

  ASSERT_TRUE(none == nullptr);

  return true;
}

static bool testCreateFromFileSet()
{
  auto variablesManager =
    std::make_shared<cmDebugger::cmDebuggerVariablesManager>();

  cmake cm(cmake::RoleScript, cmState::Unknown);
  cmFileSet fileSet(cm, "Foo", "HEADERS", cmFileSetVisibility::Public);
  BT<std::string> directory;
  directory.Value = "c:/";
  fileSet.AddDirectoryEntry(directory);
  BT<std::string> file;
  file.Value = "c:/foo.cxx";
  fileSet.AddFileEntry(file);

  auto vars = cmDebugger::cmDebuggerVariablesHelper::CreateIfAny(
    variablesManager, "Locals", true, &fileSet);

  dap::array<dap::Variable> variables =
    variablesManager->HandleVariablesRequest(
      CreateVariablesRequest(vars->GetId()));

  ASSERT_TRUE(variables.size() == 5);
  ASSERT_VARIABLE_REFERENCE_NOT_ZERO(variables[0], "Directories", "1",
                                     "collection");
  ASSERT_VARIABLE_REFERENCE_NOT_ZERO(variables[1], "Files", "1", "collection");
  ASSERT_VARIABLE(variables[2], "Name", "Foo", "string");
  ASSERT_VARIABLE(variables[3], "Type", "HEADERS", "string");
  ASSERT_VARIABLE(variables[4], "Visibility", "Public", "string");

  dap::array<dap::Variable> directoriesVariables =
    variablesManager->HandleVariablesRequest(
      CreateVariablesRequest(variables[0].variablesReference));
  ASSERT_TRUE(directoriesVariables.size() == 1);
  ASSERT_VARIABLE(directoriesVariables[0], "[0]", directory.Value, "string");

  dap::array<dap::Variable> filesVariables =
    variablesManager->HandleVariablesRequest(
      CreateVariablesRequest(variables[1].variablesReference));
  ASSERT_TRUE(filesVariables.size() == 1);
  ASSERT_VARIABLE(filesVariables[0], "[0]", file.Value, "string");

  return true;
}

static bool testCreateFromFileSets()
{
  auto variablesManager =
    std::make_shared<cmDebugger::cmDebuggerVariablesManager>();

  cmake cm(cmake::RoleScript, cmState::Unknown);
  cmFileSet fileSet(cm, "Foo", "HEADERS", cmFileSetVisibility::Public);
  BT<std::string> directory;
  directory.Value = "c:/";
  fileSet.AddDirectoryEntry(directory);
  BT<std::string> file;
  file.Value = "c:/foo.cxx";
  fileSet.AddFileEntry(file);

  auto fileSets = std::vector<cmFileSet*>{ &fileSet };
  auto vars = cmDebugger::cmDebuggerVariablesHelper::CreateIfAny(
    variablesManager, "Locals", true, fileSets);

  dap::array<dap::Variable> variables =
    variablesManager->HandleVariablesRequest(
      CreateVariablesRequest(vars->GetId()));

  ASSERT_TRUE(variables.size() == 1);
  ASSERT_VARIABLE_REFERENCE_NOT_ZERO(variables[0], "Foo", "", "collection");

  return true;
}

int testDebuggerVariablesHelper(int, char*[])
{
  return runTests(std::vector<std::function<bool()>>{
    testCreateFromPolicyMap,
    testCreateFromPairVector,
    testCreateFromSet,
    testCreateFromStringVector,
    testCreateFromTarget,
    testCreateFromGlobalGenerator,
    testCreateFromMakefile,
    testCreateFromStackFrame,
    testCreateFromTests,
    testCreateFromBTStringVector,
    testCreateFromFileSet,
    testCreateFromFileSets,
  });
}
