#include "cmInstrumentation.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <set>
#include <sstream>
#include <stdexcept>
#include <utility>

#include <cm/memory>
#include <cm/optional>
#include <cmext/algorithm>

#include <cm3p/json/reader.h>
#include <cm3p/json/version.h>
#include <cm3p/json/writer.h>
#include <cm3p/uv.h>

#include "cmsys/Directory.hxx"
#include "cmsys/FStream.hxx"
#include "cmsys/RegularExpression.hxx"
#include "cmsys/SystemInformation.hxx"

#include "cmCMakePath.h"
#include "cmCryptoHash.h"
#include "cmExperimental.h"
#include "cmFileLock.h"
#include "cmFileLockResult.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmInstrumentationQuery.h"
#include "cmJSONState.h"
#include "cmList.h"
#include "cmLocalGenerator.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTimestamp.h"
#include "cmUVProcessChain.h"
#include "cmValue.h"

using LoadQueriesAfter = cmInstrumentation::LoadQueriesAfter;

std::map<std::string, std::string> cmInstrumentation::cdashSnippetsMap = {
  {
    "configure",
    "configure",
  },
  {
    "generate",
    "configure",
  },
  {
    "compile",
    "build",
  },
  {
    "link",
    "build",
  },
  {
    "custom",
    "build",
  },
  {
    "build",
    "skip",
  },
  {
    "cmakeBuild",
    "build",
  },
  {
    "cmakeInstall",
    "build",
  },
  {
    "install",
    "build",
  },
  {
    "ctest",
    "build",
  },
  {
    "test",
    "test",
  }
};

cmInstrumentation::cmInstrumentation(std::string const& binary_dir,
                                     LoadQueriesAfter loadQueries)
{
  std::string const uuid =
    cmExperimental::DataForFeature(cmExperimental::Feature::Instrumentation)
      .Uuid;
  this->binaryDir = binary_dir;
  this->timingDirv1 =
    cmStrCat(this->binaryDir, "/.cmake/instrumentation-", uuid, "/v1");
  this->cdashDir = cmStrCat(this->timingDirv1, "/cdash");
  this->dataDir = cmStrCat(this->timingDirv1, "/data");
  if (cm::optional<std::string> configDir =
        cmSystemTools::GetCMakeConfigDirectory()) {
    this->userTimingDirv1 =
      cmStrCat(configDir.value(), "/instrumentation-", uuid, "/v1");
  }
  if (loadQueries == LoadQueriesAfter::Yes) {
    this->LoadQueries();
  }
}

void cmInstrumentation::LoadQueries()
{
  auto const readJSONQueries = [this](std::string const& dir) {
    if (cmSystemTools::FileIsDirectory(dir) && this->ReadJSONQueries(dir)) {
      this->hasQuery = true;
    }
  };
  readJSONQueries(cmStrCat(this->timingDirv1, "/query"));
  readJSONQueries(cmStrCat(this->timingDirv1, "/query/generated"));
  if (!this->userTimingDirv1.empty()) {
    readJSONQueries(cmStrCat(this->userTimingDirv1, "/query"));
  }
}

void cmInstrumentation::CheckCDashVariable()
{
  std::string envVal;
  if (cmSystemTools::GetEnv("CTEST_USE_INSTRUMENTATION", envVal) &&
      !cmIsOff(envVal)) {
    if (cmSystemTools::GetEnv("CTEST_EXPERIMENTAL_INSTRUMENTATION", envVal)) {
      std::string const uuid = cmExperimental::DataForFeature(
                                 cmExperimental::Feature::Instrumentation)
                                 .Uuid;
      if (envVal == uuid) {
        std::set<cmInstrumentationQuery::Option> options_ = {
          cmInstrumentationQuery::Option::CDashSubmit
        };
        if (cmSystemTools::GetEnv("CTEST_USE_VERBOSE_INSTRUMENTATION",
                                  envVal) &&
            !cmIsOff(envVal)) {
          options_.insert(cmInstrumentationQuery::Option::CDashVerbose);
        }
        std::set<cmInstrumentationQuery::Hook> hooks_;
        this->WriteJSONQuery(options_, hooks_, {});
      }
    }
  }
}

cmsys::SystemInformation& cmInstrumentation::GetSystemInformation()
{
  if (!this->systemInformation) {
    this->systemInformation = cm::make_unique<cmsys::SystemInformation>();
  }
  return *this->systemInformation;
}

bool cmInstrumentation::ReadJSONQueries(std::string const& directory)
{
  cmsys::Directory d;
  bool result = false;
  if (d.Load(directory)) {
    for (unsigned int i = 0; i < d.GetNumberOfFiles(); i++) {
      std::string fpath = d.GetFilePath(i);
      if (cmHasLiteralSuffix(fpath, ".json")) {
        result = true;
        this->ReadJSONQuery(fpath);
      }
    }
  }
  return result;
}

void cmInstrumentation::ReadJSONQuery(std::string const& file)
{
  auto query = cmInstrumentationQuery();
  query.ReadJSON(file, this->errorMsg, this->options, this->hooks,
                 this->callbacks);
  if (this->HasOption(cmInstrumentationQuery::Option::CDashVerbose)) {
    this->AddOption(cmInstrumentationQuery::Option::CDashSubmit);
  }
  if (this->HasOption(cmInstrumentationQuery::Option::CDashSubmit)) {
    this->AddHook(cmInstrumentationQuery::Hook::PrepareForCDash);
    this->AddOption(cmInstrumentationQuery::Option::DynamicSystemInformation);
  }
  if (!this->errorMsg.empty()) {
    cmSystemTools::Error(cmStrCat(
      "Could not load instrumentation queries from ",
      cmSystemTools::GetParentDirectory(file), ":\n", this->errorMsg));
  }
}

bool cmInstrumentation::HasErrors() const
{
  return !this->errorMsg.empty();
}

void cmInstrumentation::WriteJSONQuery(
  std::set<cmInstrumentationQuery::Option> const& options_,
  std::set<cmInstrumentationQuery::Hook> const& hooks_,
  std::vector<std::vector<std::string>> const& callbacks_)
{
  Json::Value root;
  root["version"] = 1;
  root["options"] = Json::arrayValue;
  for (auto const& option : options_) {
    root["options"].append(cmInstrumentationQuery::OptionString[option]);
  }
  root["hooks"] = Json::arrayValue;
  for (auto const& hook : hooks_) {
    root["hooks"].append(cmInstrumentationQuery::HookString[hook]);
  }
  root["callbacks"] = Json::arrayValue;
  for (auto const& callback : callbacks_) {
    root["callbacks"].append(cmInstrumentation::GetCommandStr(callback));
  }
  this->WriteInstrumentationJson(
    root, "query/generated",
    cmStrCat("query-", this->writtenJsonQueries++, ".json"));
}

void cmInstrumentation::AddCustomContent(std::string const& name,
                                         Json::Value const& contents)
{
  this->customContent[name] = contents;
}

void cmInstrumentation::WriteCMakeContent(
  std::unique_ptr<cmGlobalGenerator> const& gg)
{
  Json::Value root;
  root["targets"] = this->DumpTargets(gg);
  root["custom"] = this->customContent;
  this->WriteInstrumentationJson(
    root, "data/content",
    cmStrCat("cmake-", this->ComputeSuffixTime(), ".json"));
}

Json::Value cmInstrumentation::DumpTargets(
  std::unique_ptr<cmGlobalGenerator> const& gg)
{
  Json::Value targets = Json::objectValue;
  std::vector<cmGeneratorTarget*> targetList;
  for (auto const& lg : gg->GetLocalGenerators()) {
    cm::append(targetList, lg->GetGeneratorTargets());
  }
  for (cmGeneratorTarget* gt : targetList) {
    if (this->IsInstrumentableTargetType(gt->GetType())) {
      Json::Value target = Json::objectValue;
      auto labels = gt->GetSafeProperty("LABELS");
      target["labels"] = Json::arrayValue;
      for (auto const& item : cmList(labels)) {
        target["labels"].append(item);
      }
      target["type"] = cmState::GetTargetTypeName(gt->GetType()).c_str();
      targets[gt->GetName()] = target;
    }
  }
  return targets;
}

std::string cmInstrumentation::GetFileByTimestamp(
  cmInstrumentation::LatestOrOldest order, std::string const& dataSubdir,
  std::string const& exclude)
{
  std::string fullDir = cmStrCat(this->dataDir, '/', dataSubdir);
  std::string result;
  if (cmSystemTools::FileExists(fullDir)) {
    cmsys::Directory d;
    if (d.Load(fullDir)) {
      for (unsigned int i = 0; i < d.GetNumberOfFiles(); i++) {
        std::string fname = d.GetFileName(i);
        if (fname != "." && fname != ".." && fname != exclude &&
            (result.empty() ||
             (order == LatestOrOldest::Latest && fname > result) ||
             (order == LatestOrOldest::Oldest && fname < result))) {
          result = fname;
        }
      }
    }
  }
  return result;
}

void cmInstrumentation::RemoveOldFiles(std::string const& dataSubdir)
{
  std::string const dataSubdirPath = cmStrCat(this->dataDir, '/', dataSubdir);
  std::string oldIndex =
    this->GetFileByTimestamp(LatestOrOldest::Oldest, "index");
  if (!oldIndex.empty()) {
    oldIndex = cmStrCat(this->dataDir, "/index/", oldIndex);
  }
  if (cmSystemTools::FileExists(dataSubdirPath)) {
    std::string latestFile =
      this->GetFileByTimestamp(LatestOrOldest::Latest, dataSubdir);
    cmsys::Directory d;
    if (d.Load(dataSubdirPath)) {
      for (unsigned int i = 0; i < d.GetNumberOfFiles(); i++) {
        std::string fname = d.GetFileName(i);
        std::string fpath = d.GetFilePath(i);
        if (fname != "." && fname != ".." && fname < latestFile) {
          if (!oldIndex.empty()) {
            int compare;
            cmSystemTools::FileTimeCompare(oldIndex, fpath, &compare);
            if (compare == 1) {
              continue;
            }
          }
          cmSystemTools::RemoveFile(fpath);
        }
      }
    }
  }
}

void cmInstrumentation::ClearGeneratedQueries()
{
  std::string dir = cmStrCat(this->timingDirv1, "/query/generated");
  if (cmSystemTools::FileIsDirectory(dir)) {
    cmSystemTools::RemoveADirectory(dir);
  }
}

bool cmInstrumentation::HasQuery() const
{
  return this->hasQuery;
}

bool cmInstrumentation::HasOption(cmInstrumentationQuery::Option option) const
{
  return (this->options.find(option) != this->options.end());
}

bool cmInstrumentation::HasHook(cmInstrumentationQuery::Hook hook) const
{
  return (this->hooks.find(hook) != this->hooks.end());
}

int cmInstrumentation::CollectTimingData(cmInstrumentationQuery::Hook hook)
{
  // Don't run collection if hook is disabled
  if (hook != cmInstrumentationQuery::Hook::Manual && !this->HasHook(hook)) {
    return 0;
  }

  // Touch index file immediately to claim snippets
  std::string suffix_time = ComputeSuffixTime();
  std::string const& index_name = cmStrCat("index-", suffix_time, ".json");
  std::string index_path = cmStrCat(this->dataDir, "/index/", index_name);
  cmSystemTools::Touch(index_path, true);

  // Gather Snippets
  using snippet = std::pair<std::string, std::string>;
  std::vector<snippet> files;
  cmsys::Directory d;
  std::string last_index_name =
    this->GetFileByTimestamp(LatestOrOldest::Latest, "index", index_name);
  if (d.Load(this->dataDir)) {
    for (unsigned int i = 0; i < d.GetNumberOfFiles(); i++) {
      std::string fpath = d.GetFilePath(i);
      std::string fname = d.GetFile(i);
      if (fname.rfind('.', 0) == 0 || d.FileIsDirectory(i)) {
        continue;
      }
      files.push_back(snippet(std::move(fname), std::move(fpath)));
    }
  }

  // Build Json Object
  Json::Value index(Json::objectValue);
  index["snippets"] = Json::arrayValue;
  index["hook"] = cmInstrumentationQuery::HookString[hook];
  index["dataDir"] = this->dataDir;
  index["buildDir"] = this->binaryDir;
  index["version"] = 1;
  if (this->HasOption(
        cmInstrumentationQuery::Option::StaticSystemInformation)) {
    this->InsertStaticSystemInformation(index);
  }
  for (auto const& file : files) {
    if (last_index_name.empty()) {
      index["snippets"].append(file.first);
    } else {
      int compare;
      std::string last_index_path =
        cmStrCat(this->dataDir, "/index/", last_index_name);
      cmSystemTools::FileTimeCompare(file.second, last_index_path, &compare);
      if (compare == 1) {
        index["snippets"].append(file.first);
      }
    }
  }

  // Parse snippets into the Google trace file
  if (this->HasOption(cmInstrumentationQuery::Option::Trace)) {
    std::string trace_name = cmStrCat("trace-", suffix_time, ".json");
    this->WriteTraceFile(index, trace_name);
    index["trace"] = cmStrCat("trace/", trace_name);
  }

  // Write index file
  this->WriteInstrumentationJson(index, "data/index", index_name);

  // Execute callbacks
  for (auto& cb : this->callbacks) {
    cmSystemTools::RunSingleCommand(cmStrCat(cb, " \"", index_path, '"'),
                                    nullptr, nullptr, nullptr, nullptr,
                                    cmSystemTools::OUTPUT_PASSTHROUGH);
  }

  // Special case for CDash collation
  if (this->HasOption(cmInstrumentationQuery::Option::CDashSubmit)) {
    this->PrepareDataForCDash(this->dataDir, index_path);
  }

  // Delete files
  for (auto const& f : index["snippets"]) {
    cmSystemTools::RemoveFile(cmStrCat(this->dataDir, '/', f.asString()));
  }
  cmSystemTools::RemoveFile(index_path);

  // Delete old content and trace files
  this->RemoveOldFiles("content");
  this->RemoveOldFiles("trace");

  return 0;
}

void cmInstrumentation::InsertDynamicSystemInformation(
  Json::Value& root, std::string const& prefix)
{
  Json::Value data;
  double memory;
  double load;
  this->GetDynamicSystemInformation(memory, load);
  if (!root.isMember("dynamicSystemInformation")) {
    root["dynamicSystemInformation"] = Json::objectValue;
  }
  root["dynamicSystemInformation"][cmStrCat(prefix, "HostMemoryUsed")] =
    memory;
  root["dynamicSystemInformation"][cmStrCat(prefix, "CPULoadAverage")] =
    load > 0 ? Json::Value(load) : Json::nullValue;
}

void cmInstrumentation::GetDynamicSystemInformation(double& memory,
                                                    double& load)
{
  cmsys::SystemInformation& info = this->GetSystemInformation();
  if (!this->ranSystemChecks) {
    info.RunCPUCheck();
    info.RunMemoryCheck();
    this->ranSystemChecks = true;
  }
  memory = (double)info.GetHostMemoryUsed();
  load = info.GetLoadAverage();
}

void cmInstrumentation::InsertStaticSystemInformation(Json::Value& root)
{
  cmsys::SystemInformation& info = this->GetSystemInformation();
  if (!this->ranOSCheck) {
    info.RunOSCheck();
    this->ranOSCheck = true;
  }
  if (!this->ranSystemChecks) {
    info.RunCPUCheck();
    info.RunMemoryCheck();
    this->ranSystemChecks = true;
  }
  Json::Value infoRoot;
  infoRoot["familyId"] = info.GetFamilyID();
  infoRoot["hostname"] = info.GetHostname();
  infoRoot["is64Bits"] = info.Is64Bits();
  infoRoot["modelId"] = info.GetModelID();
  infoRoot["modelName"] = info.GetModelName();
  infoRoot["numberOfLogicalCPU"] = info.GetNumberOfLogicalCPU();
  infoRoot["numberOfPhysicalCPU"] = info.GetNumberOfPhysicalCPU();
  infoRoot["OSName"] = info.GetOSName();
  infoRoot["OSPlatform"] = info.GetOSPlatform();
  infoRoot["OSRelease"] = info.GetOSRelease();
  infoRoot["OSVersion"] = info.GetOSVersion();
  infoRoot["processorAPICID"] = info.GetProcessorAPICID();
  infoRoot["processorCacheSize"] = info.GetProcessorCacheSize();
  infoRoot["processorClockFrequency"] =
    (double)info.GetProcessorClockFrequency();
  infoRoot["processorName"] = info.GetExtendedProcessorName();
  infoRoot["totalPhysicalMemory"] =
    static_cast<Json::Value::UInt64>(info.GetTotalPhysicalMemory());
  infoRoot["totalVirtualMemory"] =
    static_cast<Json::Value::UInt64>(info.GetTotalVirtualMemory());
  infoRoot["vendorID"] = info.GetVendorID();
  infoRoot["vendorString"] = info.GetVendorString();

  // Record fields unable to be determined as null JSON objects.
  for (std::string const& field : infoRoot.getMemberNames()) {
    if ((infoRoot[field].isNumeric() && infoRoot[field].asInt64() <= 0) ||
        (infoRoot[field].isString() && infoRoot[field].asString().empty())) {
      infoRoot[field] = Json::nullValue;
    }
  }
  root["staticSystemInformation"] = infoRoot;
}

void cmInstrumentation::InsertTimingData(
  Json::Value& root, std::chrono::steady_clock::time_point steadyStart,
  std::chrono::system_clock::time_point systemStart)
{
  uint64_t timeStart = std::chrono::duration_cast<std::chrono::milliseconds>(
                         systemStart.time_since_epoch())
                         .count();
  uint64_t duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - steadyStart)
                        .count();
  root["timeStart"] = static_cast<Json::Value::UInt64>(timeStart);
  root["duration"] = static_cast<Json::Value::UInt64>(duration);
}

Json::Value cmInstrumentation::ReadJsonSnippet(std::string const& file_name)
{
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  cmsys::ifstream ftmp(
    cmStrCat(this->timingDirv1, "/data/", file_name).c_str());
  Json::Value snippetData;
  builder["collectComments"] = false;

  if (!Json::parseFromStream(builder, ftmp, &snippetData, nullptr)) {
#if JSONCPP_VERSION_HEXA < 0x01070300
    snippetData = Json::Value::null;
#else
    snippetData = Json::Value::nullSingleton();
#endif
  }

  ftmp.close();
  return snippetData;
}

void cmInstrumentation::WriteInstrumentationJson(Json::Value& root,
                                                 std::string const& subdir,
                                                 std::string const& file_name)
{
  Json::StreamWriterBuilder wbuilder;
  wbuilder["indentation"] = "\t";
  std::unique_ptr<Json::StreamWriter> JsonWriter =
    std::unique_ptr<Json::StreamWriter>(wbuilder.newStreamWriter());
  std::string const& directory = cmStrCat(this->timingDirv1, '/', subdir);
  cmSystemTools::MakeDirectory(directory);

  cmsys::ofstream ftmp(cmStrCat(directory, '/', file_name).c_str());
  if (!ftmp.good()) {
    throw std::runtime_error(std::string("Unable to open: ") + file_name);
  }

  try {
    JsonWriter->write(root, &ftmp);
    ftmp << "\n";
    ftmp.close();
  } catch (std::ios_base::failure& fail) {
    cmSystemTools::Error(cmStrCat("Failed to write JSON: ", fail.what()));
  } catch (...) {
    cmSystemTools::Error("Error writing JSON output for instrumentation.");
  }
}

std::string cmInstrumentation::InstrumentTest(
  std::string const& name, std::string const& command,
  std::vector<std::string> const& args, int64_t result,
  std::chrono::steady_clock::time_point steadyStart,
  std::chrono::system_clock::time_point systemStart, std::string config)
{
  // Store command info
  Json::Value root(this->preTestStats);
  std::string command_str = cmStrCat(command, ' ', GetCommandStr(args));
  root["version"] = 1;
  root["command"] = command_str;
  root["role"] = "test";
  root["testName"] = name;
  root["result"] = static_cast<Json::Value::Int64>(result);
  root["config"] = config;
  root["workingDir"] = cmSystemTools::GetLogicalWorkingDirectory();

  // Post-Command
  this->InsertTimingData(root, steadyStart, systemStart);
  if (this->HasOption(
        cmInstrumentationQuery::Option::DynamicSystemInformation)) {
    this->InsertDynamicSystemInformation(root, "after");
  }

  cmsys::SystemInformation& info = this->GetSystemInformation();
  std::chrono::system_clock::time_point endTime =
    systemStart + std::chrono::milliseconds(root["duration"].asUInt64());
  std::string file_name = cmStrCat(
    "test-",
    this->ComputeSuffixHash(cmStrCat(command_str, info.GetProcessId())), '-',
    this->ComputeSuffixTime(endTime), ".json");
  this->WriteInstrumentationJson(root, "data", file_name);
  return file_name;
}

void cmInstrumentation::GetPreTestStats()
{
  if (this->HasOption(
        cmInstrumentationQuery::Option::DynamicSystemInformation)) {
    this->InsertDynamicSystemInformation(this->preTestStats, "before");
  }
}

int cmInstrumentation::InstrumentCommand(
  std::string command_type, std::vector<std::string> const& command,
  std::function<int()> const& callback,
  cm::optional<std::map<std::string, std::string>> data,
  cm::optional<std::map<std::string, std::string>> arrayData,
  LoadQueriesAfter reloadQueriesAfterCommand)
{

  // Always begin gathering data for configure in case cmake_instrumentation
  // command creates a query
  if (!this->hasQuery && reloadQueriesAfterCommand == LoadQueriesAfter::No) {
    return callback();
  }

  // Store command info
  Json::Value root(Json::objectValue);
  Json::Value commandInfo(Json::objectValue);
  std::string command_str = GetCommandStr(command);

  if (!command_str.empty()) {
    root["command"] = command_str;
  }
  root["version"] = 1;

  // Pre-Command
  auto steady_start = std::chrono::steady_clock::now();
  auto system_start = std::chrono::system_clock::now();
  double preConfigureMemory = 0;
  double preConfigureLoad = 0;
  if (this->HasOption(
        cmInstrumentationQuery::Option::DynamicSystemInformation)) {
    this->InsertDynamicSystemInformation(root, "before");
  } else if (reloadQueriesAfterCommand == LoadQueriesAfter::Yes) {
    this->GetDynamicSystemInformation(preConfigureMemory, preConfigureLoad);
  }

  // Execute Command
  int ret = callback();

  // Exit early if configure didn't generate a query
  if (reloadQueriesAfterCommand == LoadQueriesAfter::Yes) {
    this->LoadQueries();
    if (!this->HasQuery()) {
      return ret;
    }
    if (this->HasOption(
          cmInstrumentationQuery::Option::DynamicSystemInformation)) {
      root["dynamicSystemInformation"] = Json::objectValue;
      root["dynamicSystemInformation"]["beforeHostMemoryUsed"] =
        preConfigureMemory;
      root["dynamicSystemInformation"]["beforeCPULoadAverage"] =
        preConfigureLoad;
    }
  }

  // Post-Command
  this->InsertTimingData(root, steady_start, system_start);
  if (this->HasOption(
        cmInstrumentationQuery::Option::DynamicSystemInformation)) {
    this->InsertDynamicSystemInformation(root, "after");
  }

  // Gather additional data
  if (data.has_value()) {
    for (auto const& item : data.value()) {
      if (item.first == "role" && !item.second.empty()) {
        command_type = item.second;
      } else if (item.first == "showOnly") {
        root[item.first] = item.second == "1" ? true : false;
      } else if (!item.second.empty()) {
        root[item.first] = item.second;
      }
    }
  }

  // See SpawnBuildDaemon(); this data is currently meaningless for build.
  root["result"] = command_type == "build" ? Json::nullValue : ret;

  // Create empty config entry if config not found
  if (!root.isMember("config") &&
      (command_type == "compile" || command_type == "link" ||
       command_type == "custom" || command_type == "install")) {
    root["config"] = "";
  }

  if (arrayData.has_value()) {
    for (auto const& item : arrayData.value()) {
      if (item.first == "targetLabels" && command_type != "link") {
        continue;
      }
      root[item.first] = Json::arrayValue;
      std::stringstream ss(item.second);
      std::string element;
      while (getline(ss, element, ',')) {
        root[item.first].append(element);
      }
      if (item.first == "outputs") {
        root["outputSizes"] = Json::arrayValue;
        for (auto const& output : root["outputs"]) {
          root["outputSizes"].append(
            static_cast<Json::Value::UInt64>(cmSystemTools::FileLength(
              cmStrCat(this->binaryDir, '/', output.asCString()))));
        }
      }
    }
  }
  root["role"] = command_type;
  root["workingDir"] = cmSystemTools::GetLogicalWorkingDirectory();

  auto addCMakeContent = [this](Json::Value& root_) -> void {
    std::string contentFile =
      this->GetFileByTimestamp(LatestOrOldest::Latest, "content");
    if (!contentFile.empty()) {
      root_["cmakeContent"] = cmStrCat("content/", contentFile);
    }
  };
  // Don't insert path to CMake content until generate time
  if (command_type != "configure") {
    addCMakeContent(root);
  }

  // Write Json
  cmsys::SystemInformation& info = this->GetSystemInformation();
  std::chrono::system_clock::time_point endTime =
    system_start + std::chrono::milliseconds(root["duration"].asUInt64());
  std::string const& file_name = cmStrCat(
    command_type, '-',
    this->ComputeSuffixHash(cmStrCat(command_str, info.GetProcessId())), '-',
    this->ComputeSuffixTime(endTime), ".json");

  // Don't write configure snippet until generate time
  if (command_type == "configure") {
    this->configureSnippetData = root;
    this->configureSnippetName = file_name;
  } else {
    // Add reference to CMake content and write out configure snippet after
    // generate
    if (command_type == "generate") {
      addCMakeContent(this->configureSnippetData);
      this->WriteInstrumentationJson(this->configureSnippetData, "data",
                                     this->configureSnippetName);
    }
    this->WriteInstrumentationJson(root, "data", file_name);
  }
  return ret;
}

std::string cmInstrumentation::GetCommandStr(
  std::vector<std::string> const& args)
{
  std::string command_str;
  for (size_t i = 0; i < args.size(); ++i) {
    if (args[i].find(' ') != std::string::npos) {
      command_str = cmStrCat(command_str, '"', args[i], '"');
    } else {
      command_str = cmStrCat(command_str, args[i]);
    }
    if (i < args.size() - 1) {
      command_str = cmStrCat(command_str, ' ');
    }
  }
  return command_str;
}

std::string cmInstrumentation::ComputeSuffixHash(
  std::string const& command_str)
{
  cmCryptoHash hasher(cmCryptoHash::AlgoSHA3_256);
  std::string hash = hasher.HashString(command_str);
  hash.resize(20, '0');
  return hash;
}

std::string cmInstrumentation::ComputeSuffixTime(
  cm::optional<std::chrono::system_clock::time_point> time)
{
  std::chrono::milliseconds ms =
    std::chrono::duration_cast<std::chrono::milliseconds>(
      (time.has_value() ? time.value() : std::chrono::system_clock::now())
        .time_since_epoch());
  std::chrono::seconds s =
    std::chrono::duration_cast<std::chrono::seconds>(ms);

  std::time_t ts = s.count();
  std::size_t tms = ms.count() % 1000;

  cmTimestamp cmts;
  std::ostringstream ss;
  ss << cmts.CreateTimestampFromTimeT(ts, "%Y-%m-%dT%H-%M-%S", true) << '-'
     << std::setfill('0') << std::setw(4) << tms;
  return ss.str();
}

bool cmInstrumentation::IsInstrumentableTargetType(
  cmStateEnums::TargetType type)
{
  return type == cmStateEnums::TargetType::EXECUTABLE ||
    type == cmStateEnums::TargetType::SHARED_LIBRARY ||
    type == cmStateEnums::TargetType::STATIC_LIBRARY ||
    type == cmStateEnums::TargetType::MODULE_LIBRARY ||
    type == cmStateEnums::TargetType::OBJECT_LIBRARY;
}

/*
 * Called by ctest --start-instrumentation.
 *
 * This creates a detached process which waits for the parent process (i.e.,
 * the build system) to die before running the postBuild hook. In this way, the
 * postBuild hook triggers after every invocation of the build system,
 * regardless of whether the build passed or failed.
 */
int cmInstrumentation::SpawnBuildDaemon()
{
  // Do not inherit handles from the parent process, so that the daemon is
  // fully detached. This helps prevent deadlock between the two.
  uv_disable_stdio_inheritance();

  // preBuild Hook
  if (this->LockBuildDaemon()) {
    // Release lock before spawning the build daemon, to prevent blocking it.
    this->lock.Release();
    this->CollectTimingData(cmInstrumentationQuery::Hook::PreBuild);
  }

  // postBuild Hook
  auto ppid = uv_os_getppid();
  if (ppid) {
    std::vector<std::string> args;
    args.push_back(cmSystemTools::GetCTestCommand());
    args.push_back("--wait-and-collect-instrumentation");
    args.push_back(this->binaryDir);
    args.push_back(std::to_string(ppid));
    auto builder = cmUVProcessChainBuilder().SetDetached().AddCommand(args);
    auto chain = builder.Start();
    uv_run(&chain.GetLoop(), UV_RUN_DEFAULT);
  }
  return 0;
}

// Prevent multiple build daemons from running simultaneously
bool cmInstrumentation::LockBuildDaemon()
{
  std::string const lockFile = cmStrCat(this->timingDirv1, "/.build.lock");
  if (!cmSystemTools::FileExists(lockFile)) {
    cmSystemTools::Touch(lockFile, true);
  }
  return this->lock.Lock(lockFile, 0).IsOk();
}

/*
 * Always called by ctest --wait-and-collect-instrumentation in a detached
 * process. Waits for the given PID to end before running the postBuild hook.
 *
 * See SpawnBuildDaemon()
 */
int cmInstrumentation::CollectTimingAfterBuild(int ppid)
{
  // Check if another process is already instrumenting the build.
  // This lock will be released when the process exits at the end of the build.
  if (!this->LockBuildDaemon()) {
    return 0;
  }
  std::function<int()> waitForBuild = [ppid]() -> int {
    while (0 == uv_kill(ppid, 0)) {
      cmSystemTools::Delay(100);
    };
    // FIXME(#27331): Investigate a cross-platform solution to obtain the exit
    // code given the `ppid` above.
    return 0;
  };
  int ret = this->InstrumentCommand(
    "build", {}, [waitForBuild]() { return waitForBuild(); }, cm::nullopt,
    cm::nullopt, LoadQueriesAfter::Yes);
  this->CollectTimingData(cmInstrumentationQuery::Hook::PostBuild);
  return ret;
}

void cmInstrumentation::AddHook(cmInstrumentationQuery::Hook hook)
{
  this->hooks.insert(hook);
}

void cmInstrumentation::AddOption(cmInstrumentationQuery::Option option)
{
  this->options.insert(option);
}

std::string const& cmInstrumentation::GetCDashDir() const
{
  return this->cdashDir;
}

std::string const& cmInstrumentation::GetDataDir() const
{
  return this->dataDir;
}

/** Copy the snippets referred to by an index file to a separate
 * directory where they will be parsed for submission to CDash.
 **/
void cmInstrumentation::PrepareDataForCDash(std::string const& data_dir,
                                            std::string const& index_path)
{
  cmSystemTools::MakeDirectory(this->cdashDir);
  cmSystemTools::MakeDirectory(cmStrCat(this->cdashDir, "/configure"));
  cmSystemTools::MakeDirectory(cmStrCat(this->cdashDir, "/build"));
  cmSystemTools::MakeDirectory(cmStrCat(this->cdashDir, "/build/commands"));
  cmSystemTools::MakeDirectory(cmStrCat(this->cdashDir, "/build/targets"));
  cmSystemTools::MakeDirectory(cmStrCat(this->cdashDir, "/test"));

  Json::Value root;
  std::string error_msg;
  cmJSONState parseState = cmJSONState(index_path, &root);
  if (!parseState.errors.empty()) {
    cmSystemTools::Error(parseState.GetErrorMessage(true));
    return;
  }

  if (!root.isObject()) {
    error_msg =
      cmStrCat("Expected index file ", index_path, " to contain an object");
    cmSystemTools::Error(error_msg);
    return;
  }

  if (!root.isMember("snippets")) {
    error_msg = cmStrCat("Expected index file ", index_path,
                         " to have a key 'snippets'");
    cmSystemTools::Error(error_msg);
    return;
  }

  std::string dst_dir;
  Json::Value snippets = root["snippets"];
  for (auto const& snippet : snippets) {
    // Parse the role of this snippet.
    std::string snippet_str = snippet.asString();
    std::string snippet_path = cmStrCat(data_dir, '/', snippet_str);
    Json::Value snippet_root;
    parseState = cmJSONState(snippet_path, &snippet_root);
    if (!parseState.errors.empty()) {
      cmSystemTools::Error(parseState.GetErrorMessage(true));
      continue;
    }
    if (!snippet_root.isObject()) {
      error_msg = cmStrCat("Expected snippet file ", snippet_path,
                           " to contain an object");
      cmSystemTools::Error(error_msg);
      continue;
    }
    if (!snippet_root.isMember("role")) {
      error_msg = cmStrCat("Expected snippet file ", snippet_path,
                           " to have a key 'role'");
      cmSystemTools::Error(error_msg);
      continue;
    }

    std::string snippet_role = snippet_root["role"].asString();
    auto map_element = this->cdashSnippetsMap.find(snippet_role);
    if (map_element == this->cdashSnippetsMap.end()) {
      std::string message =
        "Unexpected snippet type encountered: " + snippet_role;
      cmSystemTools::Message(message, "Warning");
      continue;
    }

    if (map_element->second == "skip") {
      continue;
    }

    if (map_element->second == "build") {
      // We organize snippets on a per-target basis (when possible)
      // for Build.xml.
      if (snippet_root.isMember("target")) {
        dst_dir = cmStrCat(this->cdashDir, "/build/targets/",
                           snippet_root["target"].asString());
        cmSystemTools::MakeDirectory(dst_dir);
      } else {
        dst_dir = cmStrCat(this->cdashDir, "/build/commands");
      }
    } else {
      dst_dir = cmStrCat(this->cdashDir, '/', map_element->second);
    }

    std::string dst = cmStrCat(dst_dir, '/', snippet_str);
    cmsys::Status copied = cmSystemTools::CopyFileAlways(snippet_path, dst);
    if (!copied) {
      error_msg = cmStrCat("Failed to copy ", snippet_path, " to ", dst);
      cmSystemTools::Error(error_msg);
    }
  }
}

void cmInstrumentation::WriteTraceFile(Json::Value const& index,
                                       std::string const& trace_name)
{
  std::vector<std::string> snippets = std::vector<std::string>();
  for (auto const& f : index["snippets"]) {
    snippets.push_back(f.asString());
  }
  // Reverse-sort snippets by timeEnd (timeStart + duration) as a
  // prerequisite for AssignTargetToTraceThread().
  auto extractSnippetTimestamp = [](std::string file) -> std::string {
    cmsys::RegularExpression snippetTimeRegex(
      "[A-Za-z]+-[A-Za-z0-9]+-([0-9T\\-]+)\\.json");
    cmsys::RegularExpressionMatch matchA;
    if (snippetTimeRegex.find(file.c_str(), matchA)) {
      return matchA.match(1);
    }
    return "";
  };
  std::sort(
    snippets.begin(), snippets.end(),
    [extractSnippetTimestamp](std::string snippetA, std::string snippetB) {
      return extractSnippetTimestamp(snippetA) >
        extractSnippetTimestamp(snippetB);
    });

  std::string traceDir = cmStrCat(this->timingDirv1, "/data/trace/");
  std::string traceFile = cmStrCat(traceDir, trace_name);
  cmSystemTools::MakeDirectory(traceDir);
  cmsys::ofstream traceStream;
  Json::StreamWriterBuilder wbuilder;
  wbuilder["indentation"] = "\t";
  std::unique_ptr<Json::StreamWriter> jsonWriter =
    std::unique_ptr<Json::StreamWriter>(wbuilder.newStreamWriter());
  traceStream.open(traceFile.c_str(), std::ios::out | std::ios::trunc);
  if (!traceStream.good()) {
    throw std::runtime_error(std::string("Unable to open: ") + traceFile);
  }
  traceStream << "[";

  // Append trace events from single snippets. Prefer writing to the output
  // stream incrementally over building up a Json::arrayValue in memory for
  // large traces.
  std::vector<uint64_t> workers = std::vector<uint64_t>();
  Json::Value traceEvent;
  Json::Value snippetData;
  for (size_t i = 0; i < snippets.size(); i++) {
    snippetData = this->ReadJsonSnippet(snippets[i]);
    traceEvent = this->BuildTraceEvent(workers, snippetData);
    try {
      if (i > 0) {
        traceStream << ",";
      }
      jsonWriter->write(traceEvent, &traceStream);
      if (i % 50 == 0 || i == snippets.size() - 1) {
        traceStream.flush();
        traceStream.clear();
      }
    } catch (std::ios_base::failure& fail) {
      cmSystemTools::Error(
        cmStrCat("Failed to write to Google trace file: ", fail.what()));
    } catch (...) {
      cmSystemTools::Error("Error writing Google trace output.");
    }
  }

  try {
    traceStream << "]\n";
    traceStream.close();
  } catch (...) {
    cmSystemTools::Error("Error writing Google trace output.");
  }
}

Json::Value cmInstrumentation::BuildTraceEvent(std::vector<uint64_t>& workers,
                                               Json::Value const& snippetData)
{
  Json::Value snippetTraceEvent;

  // Provide a useful trace event name depending on what data is available
  // from the snippet.
  std::string nameSuffix;
  if (snippetData["role"] == "compile") {
    nameSuffix = snippetData["source"].asString();
  } else if (snippetData["role"] == "link") {
    nameSuffix = snippetData["target"].asString();
  } else if (snippetData["role"] == "install") {
    cmCMakePath workingDir(snippetData["workingDir"].asCString());
    nameSuffix = workingDir.GetFileName().String();
  } else if (snippetData["role"] == "custom") {
    nameSuffix = snippetData["command"].asString();
  } else if (snippetData["role"] == "test") {
    nameSuffix = snippetData["testName"].asString();
  }
  if (!nameSuffix.empty()) {
    snippetTraceEvent["name"] =
      cmStrCat(snippetData["role"].asString(), ": ", nameSuffix);
  } else {
    snippetTraceEvent["name"] = snippetData["role"].asString();
  }

  snippetTraceEvent["cat"] = snippetData["role"];
  snippetTraceEvent["ph"] = "X";
  snippetTraceEvent["args"] = snippetData;

  // Time in the Trace Event Format is stored in microseconds
  // but the snippet files store time in milliseconds.
  snippetTraceEvent["ts"] = snippetData["timeStart"].asUInt64() * 1000;
  snippetTraceEvent["dur"] = snippetData["duration"].asUInt64() * 1000;

  // Assign an arbitrary PID, since this data isn't useful for the
  // visualization in our case.
  snippetTraceEvent["pid"] = 0;
  // Assign TID of 0 for snippets which will have other snippet data
  // visualized "underneath" them. (For others, start from 1.)
  if (snippetData["role"] == "build" || snippetData["role"] == "cmakeBuild" ||
      snippetData["role"] == "ctest" ||
      snippetData["role"] == "cmakeInstall") {
    snippetTraceEvent["tid"] = 0;
  } else {
    snippetTraceEvent["tid"] = static_cast<Json::Value::UInt64>(
      AssignTargetToTraceThread(workers, snippetData["timeStart"].asUInt64(),
                                snippetData["duration"].asUInt64()));
  }

  return snippetTraceEvent;
}

size_t cmInstrumentation::AssignTargetToTraceThread(
  std::vector<uint64_t>& workers, uint64_t timeStart, uint64_t duration)
{
  for (size_t i = 0; i < workers.size(); i++) {
    if (workers[i] >= timeStart + duration) {
      workers[i] = timeStart;
      return i + 1;
    }
  }
  workers.push_back(timeStart);
  return workers.size();
}
