#include "cmInstrumentation.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <memory>
#include <set>
#include <sstream>
#include <utility>

#include <cm/optional>

#include <cm3p/json/writer.h>
#include <cm3p/uv.h>

#include "cmsys/Directory.hxx"
#include "cmsys/FStream.hxx"
#include <cmsys/SystemInformation.hxx>

#include "cmCryptoHash.h"
#include "cmExperimental.h"
#include "cmInstrumentationQuery.h"
#include "cmJSONState.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTimestamp.h"
#include "cmUVProcessChain.h"
#include "cmValue.h"

cmInstrumentation::cmInstrumentation(std::string const& binary_dir)
{
  std::string const uuid =
    cmExperimental::DataForFeature(cmExperimental::Feature::Instrumentation)
      .Uuid;
  this->binaryDir = binary_dir;
  this->timingDirv1 =
    cmStrCat(this->binaryDir, "/.cmake/instrumentation-", uuid, "/v1");
  if (cm::optional<std::string> configDir =
        cmSystemTools::GetCMakeConfigDirectory()) {
    this->userTimingDirv1 =
      cmStrCat(configDir.value(), "/instrumentation-", uuid, "/v1");
  }
  this->LoadQueries();
}

void cmInstrumentation::LoadQueries()
{
  if (cmSystemTools::FileExists(cmStrCat(this->timingDirv1, "/query"))) {
    this->hasQuery =
      this->ReadJSONQueries(cmStrCat(this->timingDirv1, "/query")) ||
      this->ReadJSONQueries(cmStrCat(this->timingDirv1, "/query/generated"));
  }
  if (!this->userTimingDirv1.empty() &&
      cmSystemTools::FileExists(cmStrCat(this->userTimingDirv1, "/query"))) {
    this->hasQuery = this->hasQuery ||
      this->ReadJSONQueries(cmStrCat(this->userTimingDirv1, "/query"));
  }

  std::string envVal;
  if (cmSystemTools::GetEnv("CTEST_USE_INSTRUMENTATION", envVal) &&
      !cmIsOff(envVal)) {
    if (cmSystemTools::GetEnv("CTEST_EXPERIMENTAL_INSTRUMENTATION", envVal)) {
      std::string const uuid = cmExperimental::DataForFeature(
                                 cmExperimental::Feature::Instrumentation)
                                 .Uuid;
      if (envVal == uuid) {
        this->AddHook(cmInstrumentationQuery::Hook::PrepareForCDash);
        this->AddQuery(
          cmInstrumentationQuery::Query::DynamicSystemInformation);
        this->cdashDir = cmStrCat(this->timingDirv1, "/cdash");
        cmSystemTools::MakeDirectory(this->cdashDir);
        cmSystemTools::MakeDirectory(cmStrCat(this->cdashDir, "/configure"));
        cmSystemTools::MakeDirectory(cmStrCat(this->cdashDir, "/build"));
        cmSystemTools::MakeDirectory(
          cmStrCat(this->cdashDir, "/build/commands"));
        cmSystemTools::MakeDirectory(
          cmStrCat(this->cdashDir, "/build/targets"));
        cmSystemTools::MakeDirectory(cmStrCat(this->cdashDir, "/test"));
        this->cdashSnippetsMap = { {
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
                                   } };
        this->hasQuery = true;
      }
    }
  }
}

bool cmInstrumentation::ReadJSONQueries(std::string const& directory)
{
  cmsys::Directory d;
  std::string json = ".json";
  bool result = false;
  if (d.Load(directory)) {
    for (unsigned int i = 0; i < d.GetNumberOfFiles(); i++) {
      std::string fpath = d.GetFilePath(i);
      if (fpath.rfind(json) == (fpath.size() - json.size())) {
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
  query.ReadJSON(file, this->errorMsg, this->queries, this->hooks,
                 this->callbacks);
}

void cmInstrumentation::WriteJSONQuery(
  std::set<cmInstrumentationQuery::Query> const& queries_,
  std::set<cmInstrumentationQuery::Hook> const& hooks_,
  std::vector<std::vector<std::string>> const& callbacks_)
{
  Json::Value root;
  root["version"] = 1;
  root["queries"] = Json::arrayValue;
  for (auto const& query : queries_) {
    root["queries"].append(cmInstrumentationQuery::QueryString[query]);
  }
  root["hooks"] = Json::arrayValue;
  for (auto const& hook : hooks_) {
    root["hooks"].append(cmInstrumentationQuery::HookString[hook]);
  }
  root["callbacks"] = Json::arrayValue;
  for (auto const& callback : callbacks_) {
    root["callbacks"].append(cmInstrumentation::GetCommandStr(callback));
  }
  cmsys::Directory d;
  int n = 0;
  if (d.Load(cmStrCat(this->timingDirv1, "/query/generated"))) {
    n = (int)d.GetNumberOfFiles() - 2; // Don't count '.' or '..'
  }
  this->WriteInstrumentationJson(root, "query/generated",
                                 cmStrCat("query-", n, ".json"));
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

bool cmInstrumentation::HasQuery(cmInstrumentationQuery::Query query) const
{
  return (this->queries.find(query) != this->queries.end());
}

bool cmInstrumentation::HasHook(cmInstrumentationQuery::Hook hook) const
{
  return (this->hooks.find(hook) != this->hooks.end());
}

bool cmInstrumentation::HasPreOrPostBuildHook() const
{
  return (this->HasHook(cmInstrumentationQuery::Hook::PreBuild) ||
          this->HasHook(cmInstrumentationQuery::Hook::PostBuild));
}

int cmInstrumentation::CollectTimingData(cmInstrumentationQuery::Hook hook)
{
  // Don't run collection if hook is disabled
  if (hook != cmInstrumentationQuery::Hook::Manual &&
      this->hooks.find(hook) == this->hooks.end()) {
    return 0;
  }

  // Touch index file immediately to claim snippets
  std::string const& directory = cmStrCat(this->timingDirv1, "/data");
  std::string const& file_name =
    cmStrCat("index-", ComputeSuffixTime(), ".json");
  std::string index_path = cmStrCat(directory, "/", file_name);
  cmSystemTools::Touch(index_path, true);

  // Gather Snippets
  using snippet = std::pair<std::string, std::string>;
  std::vector<snippet> files;
  cmsys::Directory d;
  std::string last_index;
  if (d.Load(directory)) {
    for (unsigned int i = 0; i < d.GetNumberOfFiles(); i++) {
      std::string fpath = d.GetFilePath(i);
      std::string fname = d.GetFile(i);
      if (fname.rfind('.', 0) == 0) {
        continue;
      }
      if (fname == file_name) {
        continue;
      }
      if (fname.rfind("index-", 0) == 0) {
        if (last_index.empty()) {
          last_index = fpath;
        } else {
          int compare;
          cmSystemTools::FileTimeCompare(fpath, last_index, &compare);
          if (compare == 1) {
            last_index = fpath;
          }
        }
      }
      files.push_back(snippet(std::move(fname), std::move(fpath)));
    }
  }

  // Build Json Object
  Json::Value index(Json::objectValue);
  index["snippets"] = Json::arrayValue;
  index["hook"] = cmInstrumentationQuery::HookString[hook];
  index["dataDir"] = directory;
  index["buildDir"] = this->binaryDir;
  index["version"] = 1;
  if (this->HasQuery(cmInstrumentationQuery::Query::StaticSystemInformation)) {
    this->InsertStaticSystemInformation(index);
  }
  for (auto const& file : files) {
    if (last_index.empty()) {
      index["snippets"].append(file.first);
    } else {
      int compare;
      cmSystemTools::FileTimeCompare(file.second, last_index, &compare);
      if (compare == 1) {
        index["snippets"].append(file.first);
      }
    }
  }
  this->WriteInstrumentationJson(index, "data", file_name);

  // Execute callbacks
  for (auto& cb : this->callbacks) {
    cmSystemTools::RunSingleCommand(cmStrCat(cb, " \"", index_path, "\""),
                                    nullptr, nullptr, nullptr, nullptr,
                                    cmSystemTools::OUTPUT_PASSTHROUGH);
  }

  // Special case for CDash collation
  if (this->HasHook(cmInstrumentationQuery::Hook::PrepareForCDash)) {
    this->PrepareDataForCDash(directory, index_path);
  }

  // Delete files
  for (auto const& f : index["snippets"]) {
    cmSystemTools::RemoveFile(cmStrCat(directory, "/", f.asString()));
  }
  cmSystemTools::RemoveFile(index_path);

  return 0;
}

void cmInstrumentation::InsertDynamicSystemInformation(
  Json::Value& root, std::string const& prefix)
{
  cmsys::SystemInformation info;
  Json::Value data;
  info.RunCPUCheck();
  info.RunMemoryCheck();
  if (!root.isMember("dynamicSystemInformation")) {
    root["dynamicSystemInformation"] = Json::objectValue;
  }
  root["dynamicSystemInformation"][cmStrCat(prefix, "HostMemoryUsed")] =
    (double)info.GetHostMemoryUsed();
  root["dynamicSystemInformation"][cmStrCat(prefix, "CPULoadAverage")] =
    info.GetLoadAverage();
}

void cmInstrumentation::GetDynamicSystemInformation(double& memory,
                                                    double& load)
{
  cmsys::SystemInformation info;
  Json::Value data;
  info.RunCPUCheck();
  info.RunMemoryCheck();
  memory = (double)info.GetHostMemoryUsed();
  load = info.GetLoadAverage();
}

void cmInstrumentation::InsertStaticSystemInformation(Json::Value& root)
{
  cmsys::SystemInformation info;
  info.RunCPUCheck();
  info.RunOSCheck();
  info.RunMemoryCheck();
  Json::Value infoRoot;
  infoRoot["familyId"] = info.GetFamilyID();
  infoRoot["hostname"] = info.GetHostname();
  infoRoot["is64Bits"] = info.Is64Bits();
  infoRoot["modelId"] = info.GetModelID();
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

void cmInstrumentation::WriteInstrumentationJson(Json::Value& root,
                                                 std::string const& subdir,
                                                 std::string const& file_name)
{
  Json::StreamWriterBuilder wbuilder;
  wbuilder["indentation"] = "\t";
  std::unique_ptr<Json::StreamWriter> JsonWriter =
    std::unique_ptr<Json::StreamWriter>(wbuilder.newStreamWriter());
  std::string const& directory = cmStrCat(this->timingDirv1, "/", subdir);
  cmSystemTools::MakeDirectory(directory);
  cmsys::ofstream ftmp(cmStrCat(directory, "/", file_name).c_str());
  JsonWriter->write(root, &ftmp);
  ftmp << "\n";
  ftmp.close();
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
  if (this->HasQuery(
        cmInstrumentationQuery::Query::DynamicSystemInformation)) {
    this->InsertDynamicSystemInformation(root, "after");
  }

  cmsys::SystemInformation info;
  std::string file_name = cmStrCat(
    "test-",
    this->ComputeSuffixHash(cmStrCat(command_str, info.GetProcessId())),
    this->ComputeSuffixTime(), ".json");
  this->WriteInstrumentationJson(root, "data", file_name);
  return file_name;
}

void cmInstrumentation::GetPreTestStats()
{
  if (this->HasQuery(
        cmInstrumentationQuery::Query::DynamicSystemInformation)) {
    this->InsertDynamicSystemInformation(this->preTestStats, "before");
  }
}

int cmInstrumentation::InstrumentCommand(
  std::string command_type, std::vector<std::string> const& command,
  std::function<int()> const& callback,
  cm::optional<std::map<std::string, std::string>> options,
  cm::optional<std::map<std::string, std::string>> arrayOptions,
  bool reloadQueriesAfterCommand)
{

  // Always begin gathering data for configure in case cmake_instrumentation
  // command creates a query
  if (!this->hasQuery && !reloadQueriesAfterCommand) {
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
  if (this->HasQuery(
        cmInstrumentationQuery::Query::DynamicSystemInformation)) {
    this->InsertDynamicSystemInformation(root, "before");
  } else if (reloadQueriesAfterCommand) {
    this->GetDynamicSystemInformation(preConfigureMemory, preConfigureLoad);
  }

  // Execute Command
  int ret = callback();
  root["result"] = ret;

  // Exit early if configure didn't generate a query
  if (reloadQueriesAfterCommand) {
    this->LoadQueries();
    if (!this->hasQuery) {
      return ret;
    }
    if (this->HasQuery(
          cmInstrumentationQuery::Query::DynamicSystemInformation)) {
      root["dynamicSystemInformation"] = Json::objectValue;
      root["dynamicSystemInformation"]["beforeHostMemoryUsed"] =
        preConfigureMemory;
      root["dynamicSystemInformation"]["beforeCPULoadAverage"] =
        preConfigureLoad;
    }
  }

  // Post-Command
  this->InsertTimingData(root, steady_start, system_start);
  if (this->HasQuery(
        cmInstrumentationQuery::Query::DynamicSystemInformation)) {
    this->InsertDynamicSystemInformation(root, "after");
  }

  // Gather additional data
  if (options.has_value()) {
    for (auto const& item : options.value()) {
      if (item.first == "role" && !item.second.empty()) {
        command_type = item.second;
      } else if (!item.second.empty()) {
        root[item.first] = item.second;
      }
    }
  }

  // Create empty config entry if config not found
  if (!root.isMember("config") &&
      (command_type == "compile" || command_type == "link")) {
    root["config"] = "";
  }

  if (arrayOptions.has_value()) {
    for (auto const& item : arrayOptions.value()) {
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
              cmStrCat(this->binaryDir, "/", output.asCString()))));
        }
      }
    }
  }
  root["role"] = command_type;
  root["workingDir"] = cmSystemTools::GetLogicalWorkingDirectory();

  // Write Json
  cmsys::SystemInformation info;
  std::string const& file_name = cmStrCat(
    command_type, "-",
    this->ComputeSuffixHash(cmStrCat(command_str, info.GetProcessId())),
    this->ComputeSuffixTime(), ".json");
  this->WriteInstrumentationJson(root, "data", file_name);
  return ret;
}

std::string cmInstrumentation::GetCommandStr(
  std::vector<std::string> const& args)
{
  std::string command_str;
  for (size_t i = 0; i < args.size(); ++i) {
    command_str = cmStrCat(command_str, '"', args[i], '"');
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

std::string cmInstrumentation::ComputeSuffixTime()
{
  std::chrono::milliseconds ms =
    std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch());
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

/*
 * Called by ctest --start-instrumentation as part of the START_INSTRUMENTATION
 * rule when using the Ninja generator.
 * This creates a detached process which waits for the Ninja process to die
 * before running the postBuild hook. In this way, the postBuild hook triggers
 * after every ninja invocation, regardless of whether the build passed or
 * failed.
 */
int cmInstrumentation::SpawnBuildDaemon()
{
  // preBuild Hook
  this->CollectTimingData(cmInstrumentationQuery::Hook::PreBuild);

  // postBuild Hook
  if (this->HasHook(cmInstrumentationQuery::Hook::PostBuild)) {
    auto ninja_pid = uv_os_getppid();
    if (ninja_pid) {
      std::vector<std::string> args;
      args.push_back(cmSystemTools::GetCTestCommand());
      args.push_back("--wait-and-collect-instrumentation");
      args.push_back(this->binaryDir);
      args.push_back(std::to_string(ninja_pid));
      auto builder = cmUVProcessChainBuilder().SetDetached().AddCommand(args);
      auto chain = builder.Start();
      uv_run(&chain.GetLoop(), UV_RUN_DEFAULT);
    }
  }
  return 0;
}

/*
 * Always called by ctest --wait-and-collect-instrumentation in a detached
 * process. Waits for the given PID to end before running the postBuild hook.
 *
 * See SpawnBuildDaemon()
 */
int cmInstrumentation::CollectTimingAfterBuild(int ppid)
{
  std::function<int()> waitForBuild = [ppid]() -> int {
    while (0 == uv_kill(ppid, 0)) {
      cmSystemTools::Delay(100);
    };
    return 0;
  };
  int ret = this->InstrumentCommand(
    "build", {}, [waitForBuild]() { return waitForBuild(); }, cm::nullopt,
    cm::nullopt, false);
  this->CollectTimingData(cmInstrumentationQuery::Hook::PostBuild);
  return ret;
}

void cmInstrumentation::AddHook(cmInstrumentationQuery::Hook hook)
{
  this->hooks.insert(hook);
}

void cmInstrumentation::AddQuery(cmInstrumentationQuery::Query query)
{
  this->queries.insert(query);
}

std::string const& cmInstrumentation::GetCDashDir()
{
  return this->cdashDir;
}

/** Copy the snippets referred to by an index file to a separate
 * directory where they will be parsed for submission to CDash.
 **/
void cmInstrumentation::PrepareDataForCDash(std::string const& data_dir,
                                            std::string const& index_path)
{
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
