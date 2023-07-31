/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmFileAPI.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <utility>

#include "cmsys/Directory.hxx"
#include "cmsys/FStream.hxx"

#include "cmCryptoHash.h"
#include "cmFileAPICMakeFiles.h"
#include "cmFileAPICache.h"
#include "cmFileAPICodemodel.h"
#include "cmFileAPIConfigureLog.h"
#include "cmFileAPIToolchains.h"
#include "cmGlobalGenerator.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTimestamp.h"
#include "cmake.h"

cmFileAPI::cmFileAPI(cmake* cm)
  : CMakeInstance(cm)
{
  this->APIv1 =
    this->CMakeInstance->GetHomeOutputDirectory() + "/.cmake/api/v1";

  Json::CharReaderBuilder rbuilder;
  rbuilder["collectComments"] = false;
  rbuilder["failIfExtra"] = true;
  rbuilder["rejectDupKeys"] = false;
  rbuilder["strictRoot"] = true;
  this->JsonReader =
    std::unique_ptr<Json::CharReader>(rbuilder.newCharReader());

  Json::StreamWriterBuilder wbuilder;
  wbuilder["indentation"] = "\t";
  this->JsonWriter =
    std::unique_ptr<Json::StreamWriter>(wbuilder.newStreamWriter());
}

void cmFileAPI::ReadQueries()
{
  std::string const query_dir = this->APIv1 + "/query";
  this->QueryExists = cmSystemTools::FileIsDirectory(query_dir);
  if (!this->QueryExists) {
    return;
  }

  // Load queries at the top level.
  std::vector<std::string> queries = cmFileAPI::LoadDir(query_dir);

  // Read the queries and save for later.
  for (std::string& query : queries) {
    if (cmHasLiteralPrefix(query, "client-")) {
      this->ReadClient(query);
    } else if (!cmFileAPI::ReadQuery(query, this->TopQuery.Known)) {
      this->TopQuery.Unknown.push_back(std::move(query));
    }
  }
}

std::vector<unsigned long> cmFileAPI::GetConfigureLogVersions()
{
  std::vector<unsigned long> versions;
  auto getConfigureLogVersions = [&versions](Query const& q) {
    for (Object const& o : q.Known) {
      if (o.Kind == ObjectKind::ConfigureLog) {
        versions.emplace_back(o.Version);
      }
    }
  };
  getConfigureLogVersions(this->TopQuery);
  for (auto const& client : this->ClientQueries) {
    getConfigureLogVersions(client.second.DirQuery);
  }
  std::sort(versions.begin(), versions.end());
  versions.erase(std::unique(versions.begin(), versions.end()),
                 versions.end());
  return versions;
}

void cmFileAPI::WriteReplies()
{
  if (this->QueryExists) {
    cmSystemTools::MakeDirectory(this->APIv1 + "/reply");
    this->WriteJsonFile(this->BuildReplyIndex(), "index", ComputeSuffixTime);
  }

  this->RemoveOldReplyFiles();
}

std::vector<std::string> cmFileAPI::LoadDir(std::string const& dir)
{
  std::vector<std::string> files;
  cmsys::Directory d;
  d.Load(dir);
  for (unsigned long i = 0; i < d.GetNumberOfFiles(); ++i) {
    std::string f = d.GetFile(i);
    if (f != "." && f != "..") {
      files.push_back(std::move(f));
    }
  }
  std::sort(files.begin(), files.end());
  return files;
}

void cmFileAPI::RemoveOldReplyFiles()
{
  std::string const reply_dir = this->APIv1 + "/reply";
  std::vector<std::string> files = this->LoadDir(reply_dir);
  for (std::string const& f : files) {
    if (this->ReplyFiles.find(f) == this->ReplyFiles.end()) {
      std::string file = cmStrCat(reply_dir, "/", f);
      cmSystemTools::RemoveFile(file);
    }
  }
}

bool cmFileAPI::ReadJsonFile(std::string const& file, Json::Value& value,
                             std::string& error)
{
  std::vector<char> content;

  cmsys::ifstream fin;
  if (!cmSystemTools::FileIsDirectory(file)) {
    fin.open(file.c_str(), std::ios::binary);
  }
  auto finEnd = fin.rdbuf()->pubseekoff(0, std::ios::end);
  if (finEnd > 0) {
    size_t finSize = finEnd;
    try {
      // Allocate a buffer to read the whole file.
      content.resize(finSize);

      // Now read the file from the beginning.
      fin.seekg(0, std::ios::beg);
      fin.read(content.data(), finSize);
    } catch (...) {
      fin.setstate(std::ios::failbit);
    }
  }
  fin.close();
  if (!fin) {
    value = Json::Value();
    error = "failed to read from file";
    return false;
  }

  // Parse our buffer as json.
  if (!this->JsonReader->parse(content.data(), content.data() + content.size(),
                               &value, &error)) {
    value = Json::Value();
    return false;
  }

  return true;
}

std::string cmFileAPI::WriteJsonFile(
  Json::Value const& value, std::string const& prefix,
  std::string (*computeSuffix)(std::string const&))
{
  std::string fileName;

  // Write the json file with a temporary name.
  std::string const& tmpFile = this->APIv1 + "/tmp.json";
  cmsys::ofstream ftmp(tmpFile.c_str());
  this->JsonWriter->write(value, &ftmp);
  ftmp << "\n";
  ftmp.close();
  if (!ftmp) {
    cmSystemTools::RemoveFile(tmpFile);
    return fileName;
  }

  // Compute the final name for the file.
  fileName = prefix + "-" + computeSuffix(tmpFile) + ".json";

  // Create the destination.
  std::string file = this->APIv1 + "/reply";
  cmSystemTools::MakeDirectory(file);
  file += "/";
  file += fileName;

  // If the final name already exists then assume it has proper content.
  // Otherwise, atomically place the reply file at its final name
  if (cmSystemTools::FileExists(file, true) ||
      !cmSystemTools::RenameFile(tmpFile, file)) {
    cmSystemTools::RemoveFile(tmpFile);
  }

  // Record this among files we have just written.
  this->ReplyFiles.insert(fileName);

  return fileName;
}

Json::Value cmFileAPI::MaybeJsonFile(Json::Value in, std::string const& prefix)
{
  Json::Value out;
  if (in.isObject() || in.isArray()) {
    out = Json::objectValue;
    out["jsonFile"] = this->WriteJsonFile(in, prefix);
  } else {
    out = std::move(in);
  }
  return out;
}

std::string cmFileAPI::ComputeSuffixHash(std::string const& file)
{
  cmCryptoHash hasher(cmCryptoHash::AlgoSHA3_256);
  std::string hash = hasher.HashFile(file);
  hash.resize(20, '0');
  return hash;
}

std::string cmFileAPI::ComputeSuffixTime(std::string const&)
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

bool cmFileAPI::ReadQuery(std::string const& query,
                          std::vector<Object>& objects)
{
  // Parse the "<kind>-" syntax.
  std::string::size_type sep_pos = query.find('-');
  if (sep_pos == std::string::npos) {
    return false;
  }
  std::string kindName = query.substr(0, sep_pos);
  std::string verStr = query.substr(sep_pos + 1);
  if (kindName == ObjectKindName(ObjectKind::CodeModel)) {
    Object o;
    o.Kind = ObjectKind::CodeModel;
    if (verStr == "v2") {
      o.Version = 2;
    } else {
      return false;
    }
    objects.push_back(o);
    return true;
  }
  if (kindName == ObjectKindName(ObjectKind::ConfigureLog)) {
    Object o;
    o.Kind = ObjectKind::ConfigureLog;
    if (verStr == "v1") {
      o.Version = 1;
    } else {
      return false;
    }
    objects.push_back(o);
    return true;
  }
  if (kindName == ObjectKindName(ObjectKind::Cache)) {
    Object o;
    o.Kind = ObjectKind::Cache;
    if (verStr == "v2") {
      o.Version = 2;
    } else {
      return false;
    }
    objects.push_back(o);
    return true;
  }
  if (kindName == ObjectKindName(ObjectKind::CMakeFiles)) {
    Object o;
    o.Kind = ObjectKind::CMakeFiles;
    if (verStr == "v1") {
      o.Version = 1;
    } else {
      return false;
    }
    objects.push_back(o);
    return true;
  }
  if (kindName == ObjectKindName(ObjectKind::Toolchains)) {
    Object o;
    o.Kind = ObjectKind::Toolchains;
    if (verStr == "v1") {
      o.Version = 1;
    } else {
      return false;
    }
    objects.push_back(o);
    return true;
  }
  if (kindName == ObjectKindName(ObjectKind::InternalTest)) {
    Object o;
    o.Kind = ObjectKind::InternalTest;
    if (verStr == "v1") {
      o.Version = 1;
    } else if (verStr == "v2") {
      o.Version = 2;
    } else {
      return false;
    }
    objects.push_back(o);
    return true;
  }
  return false;
}

void cmFileAPI::ReadClient(std::string const& client)
{
  // Load queries for the client.
  std::string clientDir = this->APIv1 + "/query/" + client;
  std::vector<std::string> queries = this->LoadDir(clientDir);

  // Read the queries and save for later.
  ClientQuery& clientQuery = this->ClientQueries[client];
  for (std::string& query : queries) {
    if (query == "query.json") {
      clientQuery.HaveQueryJson = true;
      this->ReadClientQuery(client, clientQuery.QueryJson);
    } else if (!this->ReadQuery(query, clientQuery.DirQuery.Known)) {
      clientQuery.DirQuery.Unknown.push_back(std::move(query));
    }
  }
}

void cmFileAPI::ReadClientQuery(std::string const& client, ClientQueryJson& q)
{
  // Read the query.json file.
  std::string queryFile = this->APIv1 + "/query/" + client + "/query.json";
  Json::Value query;
  if (!this->ReadJsonFile(queryFile, query, q.Error)) {
    return;
  }
  if (!query.isObject()) {
    q.Error = "query root is not an object";
    return;
  }

  Json::Value const& clientValue = query["client"];
  if (!clientValue.isNull()) {
    q.ClientValue = clientValue;
  }
  q.RequestsValue = std::move(query["requests"]);
  q.Requests = this->BuildClientRequests(q.RequestsValue);
}

Json::Value cmFileAPI::BuildReplyIndex()
{
  Json::Value index(Json::objectValue);

  // Report information about this version of CMake.
  index["cmake"] = this->BuildCMake();

  // Reply to all queries that we loaded.
  Json::Value& reply = index["reply"] = this->BuildReply(this->TopQuery);
  for (auto const& client : this->ClientQueries) {
    std::string const& clientName = client.first;
    ClientQuery const& clientQuery = client.second;
    reply[clientName] = this->BuildClientReply(clientQuery);
  }

  // Move our index of generated objects into its field.
  Json::Value& objects = index["objects"] = Json::arrayValue;
  for (auto& entry : this->ReplyIndexObjects) {
    objects.append(std::move(entry.second)); // NOLINT(*)
  }

  return index;
}

Json::Value cmFileAPI::BuildCMake()
{
  Json::Value cmake = Json::objectValue;
  cmake["version"] = this->CMakeInstance->ReportVersionJson();
  Json::Value& cmake_paths = cmake["paths"] = Json::objectValue;
  cmake_paths["cmake"] = cmSystemTools::GetCMakeCommand();
  cmake_paths["ctest"] = cmSystemTools::GetCTestCommand();
  cmake_paths["cpack"] = cmSystemTools::GetCPackCommand();
  cmake_paths["root"] = cmSystemTools::GetCMakeRoot();
  cmake["generator"] = this->CMakeInstance->GetGlobalGenerator()->GetJson();
  return cmake;
}

Json::Value cmFileAPI::BuildReply(Query const& q)
{
  Json::Value reply = Json::objectValue;
  for (Object const& o : q.Known) {
    std::string const& name = ObjectName(o);
    reply[name] = this->AddReplyIndexObject(o);
  }

  for (std::string const& name : q.Unknown) {
    reply[name] = cmFileAPI::BuildReplyError("unknown query file");
  }
  return reply;
}

Json::Value cmFileAPI::BuildReplyError(std::string const& error)
{
  Json::Value e = Json::objectValue;
  e["error"] = error;
  return e;
}

Json::Value const& cmFileAPI::AddReplyIndexObject(Object const& o)
{
  Json::Value& indexEntry = this->ReplyIndexObjects[o];
  if (!indexEntry.isNull()) {
    // The reply object has already been generated.
    return indexEntry;
  }

  // Generate this reply object.
  Json::Value const& object = this->BuildObject(o);
  assert(object.isObject());

  // Populate this index entry.
  indexEntry = Json::objectValue;
  indexEntry["kind"] = object["kind"];
  indexEntry["version"] = object["version"];
  indexEntry["jsonFile"] = this->WriteJsonFile(object, ObjectName(o));
  return indexEntry;
}

const char* cmFileAPI::ObjectKindName(ObjectKind kind)
{
  // Keep in sync with ObjectKind enum.
  static const char* objectKindNames[] = {
    "codemodel",    //
    "configureLog", //
    "cache",        //
    "cmakeFiles",   //
    "toolchains",   //
    "__test"        //
  };
  return objectKindNames[static_cast<size_t>(kind)];
}

std::string cmFileAPI::ObjectName(Object const& o)
{
  std::string name = cmStrCat(ObjectKindName(o.Kind), "-v", o.Version);
  return name;
}

Json::Value cmFileAPI::BuildVersion(unsigned int major, unsigned int minor)
{
  Json::Value version;
  version["major"] = major;
  version["minor"] = minor;
  return version;
}

Json::Value cmFileAPI::BuildObject(Object const& object)
{
  Json::Value value;

  switch (object.Kind) {
    case ObjectKind::CodeModel:
      value = this->BuildCodeModel(object);
      break;
    case ObjectKind::ConfigureLog:
      value = this->BuildConfigureLog(object);
      break;
    case ObjectKind::Cache:
      value = this->BuildCache(object);
      break;
    case ObjectKind::CMakeFiles:
      value = this->BuildCMakeFiles(object);
      break;
    case ObjectKind::Toolchains:
      value = this->BuildToolchains(object);
      break;
    case ObjectKind::InternalTest:
      value = this->BuildInternalTest(object);
      break;
  }

  return value;
}

cmFileAPI::ClientRequests cmFileAPI::BuildClientRequests(
  Json::Value const& requests)
{
  ClientRequests result;
  if (requests.isNull()) {
    result.Error = "'requests' member missing";
    return result;
  }
  if (!requests.isArray()) {
    result.Error = "'requests' member is not an array";
    return result;
  }

  result.reserve(requests.size());
  for (Json::Value const& request : requests) {
    result.emplace_back(this->BuildClientRequest(request));
  }

  return result;
}

cmFileAPI::ClientRequest cmFileAPI::BuildClientRequest(
  Json::Value const& request)
{
  ClientRequest r;

  if (!request.isObject()) {
    r.Error = "request is not an object";
    return r;
  }

  Json::Value const& kind = request["kind"];
  if (kind.isNull()) {
    r.Error = "'kind' member missing";
    return r;
  }
  if (!kind.isString()) {
    r.Error = "'kind' member is not a string";
    return r;
  }
  std::string const& kindName = kind.asString();

  if (kindName == this->ObjectKindName(ObjectKind::CodeModel)) {
    r.Kind = ObjectKind::CodeModel;
  } else if (kindName == this->ObjectKindName(ObjectKind::ConfigureLog)) {
    r.Kind = ObjectKind::ConfigureLog;
  } else if (kindName == this->ObjectKindName(ObjectKind::Cache)) {
    r.Kind = ObjectKind::Cache;
  } else if (kindName == this->ObjectKindName(ObjectKind::CMakeFiles)) {
    r.Kind = ObjectKind::CMakeFiles;
  } else if (kindName == this->ObjectKindName(ObjectKind::Toolchains)) {
    r.Kind = ObjectKind::Toolchains;
  } else if (kindName == this->ObjectKindName(ObjectKind::InternalTest)) {
    r.Kind = ObjectKind::InternalTest;
  } else {
    r.Error = "unknown request kind '" + kindName + "'";
    return r;
  }

  Json::Value const& version = request["version"];
  if (version.isNull()) {
    r.Error = "'version' member missing";
    return r;
  }
  std::vector<RequestVersion> versions;
  if (!cmFileAPI::ReadRequestVersions(version, versions, r.Error)) {
    return r;
  }

  switch (r.Kind) {
    case ObjectKind::CodeModel:
      this->BuildClientRequestCodeModel(r, versions);
      break;
    case ObjectKind::ConfigureLog:
      this->BuildClientRequestConfigureLog(r, versions);
      break;
    case ObjectKind::Cache:
      this->BuildClientRequestCache(r, versions);
      break;
    case ObjectKind::CMakeFiles:
      this->BuildClientRequestCMakeFiles(r, versions);
      break;
    case ObjectKind::Toolchains:
      this->BuildClientRequestToolchains(r, versions);
      break;
    case ObjectKind::InternalTest:
      this->BuildClientRequestInternalTest(r, versions);
      break;
  }

  return r;
}

Json::Value cmFileAPI::BuildClientReply(ClientQuery const& q)
{
  Json::Value reply = this->BuildReply(q.DirQuery);

  if (!q.HaveQueryJson) {
    return reply;
  }

  Json::Value& reply_query_json = reply["query.json"];
  ClientQueryJson const& qj = q.QueryJson;

  if (!qj.Error.empty()) {
    reply_query_json = this->BuildReplyError(qj.Error);
    return reply;
  }

  if (!qj.ClientValue.isNull()) {
    reply_query_json["client"] = qj.ClientValue;
  }

  if (!qj.RequestsValue.isNull()) {
    reply_query_json["requests"] = qj.RequestsValue;
  }

  reply_query_json["responses"] = this->BuildClientReplyResponses(qj.Requests);

  return reply;
}

Json::Value cmFileAPI::BuildClientReplyResponses(
  ClientRequests const& requests)
{
  Json::Value responses;

  if (!requests.Error.empty()) {
    responses = this->BuildReplyError(requests.Error);
    return responses;
  }

  responses = Json::arrayValue;
  for (ClientRequest const& request : requests) {
    responses.append(this->BuildClientReplyResponse(request));
  }

  return responses;
}

Json::Value cmFileAPI::BuildClientReplyResponse(ClientRequest const& request)
{
  Json::Value response;
  if (!request.Error.empty()) {
    response = this->BuildReplyError(request.Error);
    return response;
  }
  response = this->AddReplyIndexObject(request);
  return response;
}

bool cmFileAPI::ReadRequestVersions(Json::Value const& version,
                                    std::vector<RequestVersion>& versions,
                                    std::string& error)
{
  if (version.isArray()) {
    for (Json::Value const& v : version) {
      if (!ReadRequestVersion(v, /*inArray=*/true, versions, error)) {
        return false;
      }
    }
  } else {
    if (!ReadRequestVersion(version, /*inArray=*/false, versions, error)) {
      return false;
    }
  }
  return true;
}

bool cmFileAPI::ReadRequestVersion(Json::Value const& version, bool inArray,
                                   std::vector<RequestVersion>& result,
                                   std::string& error)
{
  if (version.isUInt()) {
    RequestVersion v;
    v.Major = version.asUInt();
    result.push_back(v);
    return true;
  }

  if (!version.isObject()) {
    if (inArray) {
      error = "'version' array entry is not a non-negative integer or object";
    } else {
      error =
        "'version' member is not a non-negative integer, object, or array";
    }
    return false;
  }

  Json::Value const& major = version["major"];
  if (major.isNull()) {
    error = "'version' object 'major' member missing";
    return false;
  }
  if (!major.isUInt()) {
    error = "'version' object 'major' member is not a non-negative integer";
    return false;
  }

  RequestVersion v;
  v.Major = major.asUInt();

  Json::Value const& minor = version["minor"];
  if (minor.isUInt()) {
    v.Minor = minor.asUInt();
  } else if (!minor.isNull()) {
    error = "'version' object 'minor' member is not a non-negative integer";
    return false;
  }

  result.push_back(v);

  return true;
}

std::string cmFileAPI::NoSupportedVersion(
  std::vector<RequestVersion> const& versions)
{
  std::ostringstream msg;
  msg << "no supported version specified";
  if (!versions.empty()) {
    msg << " among:";
    for (RequestVersion const& v : versions) {
      msg << " " << v.Major << "." << v.Minor;
    }
  }
  return msg.str();
}

// The "codemodel" object kind.

// Update Help/manual/cmake-file-api.7.rst when updating this constant.
static unsigned int const CodeModelV2Minor = 6;

void cmFileAPI::BuildClientRequestCodeModel(
  ClientRequest& r, std::vector<RequestVersion> const& versions)
{
  // Select a known version from those requested.
  for (RequestVersion const& v : versions) {
    if ((v.Major == 2 && v.Minor <= CodeModelV2Minor)) {
      r.Version = v.Major;
      break;
    }
  }
  if (!r.Version) {
    r.Error = NoSupportedVersion(versions);
  }
}

Json::Value cmFileAPI::BuildCodeModel(Object const& object)
{
  Json::Value codemodel = cmFileAPICodemodelDump(*this, object.Version);
  codemodel["kind"] = this->ObjectKindName(object.Kind);

  Json::Value& version = codemodel["version"];
  if (object.Version == 2) {
    version = BuildVersion(2, CodeModelV2Minor);
  } else {
    return codemodel; // should be unreachable
  }

  return codemodel;
}

// The "configureLog" object kind.

// Update Help/manual/cmake-file-api.7.rst when updating this constant.
static unsigned int const ConfigureLogV1Minor = 0;

void cmFileAPI::BuildClientRequestConfigureLog(
  ClientRequest& r, std::vector<RequestVersion> const& versions)
{
  // Select a known version from those requested.
  for (RequestVersion const& v : versions) {
    if ((v.Major == 1 && v.Minor <= ConfigureLogV1Minor)) {
      r.Version = v.Major;
      break;
    }
  }
  if (!r.Version) {
    r.Error = NoSupportedVersion(versions);
  }
}

Json::Value cmFileAPI::BuildConfigureLog(Object const& object)
{
  Json::Value configureLog = cmFileAPIConfigureLogDump(*this, object.Version);
  configureLog["kind"] = this->ObjectKindName(object.Kind);

  Json::Value& version = configureLog["version"];
  if (object.Version == 1) {
    version = BuildVersion(1, ConfigureLogV1Minor);
  } else {
    return configureLog; // should be unreachable
  }

  return configureLog;
}

// The "cache" object kind.

static unsigned int const CacheV2Minor = 0;

void cmFileAPI::BuildClientRequestCache(
  ClientRequest& r, std::vector<RequestVersion> const& versions)
{
  // Select a known version from those requested.
  for (RequestVersion const& v : versions) {
    if ((v.Major == 2 && v.Minor <= CacheV2Minor)) {
      r.Version = v.Major;
      break;
    }
  }
  if (!r.Version) {
    r.Error = NoSupportedVersion(versions);
  }
}

Json::Value cmFileAPI::BuildCache(Object const& object)
{
  Json::Value cache = cmFileAPICacheDump(*this, object.Version);
  cache["kind"] = this->ObjectKindName(object.Kind);

  Json::Value& version = cache["version"];
  if (object.Version == 2) {
    version = BuildVersion(2, CacheV2Minor);
  } else {
    return cache; // should be unreachable
  }

  return cache;
}

// The "cmakeFiles" object kind.

static unsigned int const CMakeFilesV1Minor = 0;

void cmFileAPI::BuildClientRequestCMakeFiles(
  ClientRequest& r, std::vector<RequestVersion> const& versions)
{
  // Select a known version from those requested.
  for (RequestVersion const& v : versions) {
    if ((v.Major == 1 && v.Minor <= CMakeFilesV1Minor)) {
      r.Version = v.Major;
      break;
    }
  }
  if (!r.Version) {
    r.Error = NoSupportedVersion(versions);
  }
}

Json::Value cmFileAPI::BuildCMakeFiles(Object const& object)
{
  Json::Value cmakeFiles = cmFileAPICMakeFilesDump(*this, object.Version);
  cmakeFiles["kind"] = this->ObjectKindName(object.Kind);

  Json::Value& version = cmakeFiles["version"];
  if (object.Version == 1) {
    version = BuildVersion(1, CMakeFilesV1Minor);
  } else {
    return cmakeFiles; // should be unreachable
  }

  return cmakeFiles;
}

// The "toolchains" object kind.

static unsigned int const ToolchainsV1Minor = 0;

void cmFileAPI::BuildClientRequestToolchains(
  ClientRequest& r, std::vector<RequestVersion> const& versions)
{
  // Select a known version from those requested.
  for (RequestVersion const& v : versions) {
    if ((v.Major == 1 && v.Minor <= ToolchainsV1Minor)) {
      r.Version = v.Major;
      break;
    }
  }
  if (!r.Version) {
    r.Error = NoSupportedVersion(versions);
  }
}

Json::Value cmFileAPI::BuildToolchains(Object const& object)
{
  Json::Value toolchains = cmFileAPIToolchainsDump(*this, object.Version);
  toolchains["kind"] = this->ObjectKindName(object.Kind);

  Json::Value& version = toolchains["version"];
  if (object.Version == 1) {
    version = BuildVersion(1, ToolchainsV1Minor);
  } else {
    return toolchains; // should be unreachable
  }

  return toolchains;
}

// The "__test" object kind is for internal testing of CMake.

static unsigned int const InternalTestV1Minor = 3;
static unsigned int const InternalTestV2Minor = 0;

void cmFileAPI::BuildClientRequestInternalTest(
  ClientRequest& r, std::vector<RequestVersion> const& versions)
{
  // Select a known version from those requested.
  for (RequestVersion const& v : versions) {
    if ((v.Major == 1 && v.Minor <= InternalTestV1Minor) || //
        (v.Major == 2 && v.Minor <= InternalTestV2Minor)) {
      r.Version = v.Major;
      break;
    }
  }
  if (!r.Version) {
    r.Error = NoSupportedVersion(versions);
  }
}

Json::Value cmFileAPI::BuildInternalTest(Object const& object)
{
  Json::Value test = Json::objectValue;
  test["kind"] = this->ObjectKindName(object.Kind);
  Json::Value& version = test["version"];
  if (object.Version == 2) {
    version = BuildVersion(2, InternalTestV2Minor);
  } else {
    version = BuildVersion(1, InternalTestV1Minor);
  }
  return test;
}

Json::Value cmFileAPI::ReportCapabilities()
{
  Json::Value capabilities = Json::objectValue;
  Json::Value& requests = capabilities["requests"] = Json::arrayValue;

  {
    Json::Value request = Json::objectValue;
    request["kind"] = ObjectKindName(ObjectKind::CodeModel);
    Json::Value& versions = request["version"] = Json::arrayValue;
    versions.append(BuildVersion(2, CodeModelV2Minor));
    requests.append(std::move(request)); // NOLINT(*)
  }

  {
    Json::Value request = Json::objectValue;
    request["kind"] = ObjectKindName(ObjectKind::ConfigureLog);
    Json::Value& versions = request["version"] = Json::arrayValue;
    versions.append(BuildVersion(1, ConfigureLogV1Minor));
    requests.append(std::move(request)); // NOLINT(*)
  }

  {
    Json::Value request = Json::objectValue;
    request["kind"] = ObjectKindName(ObjectKind::Cache);
    Json::Value& versions = request["version"] = Json::arrayValue;
    versions.append(BuildVersion(2, CacheV2Minor));
    requests.append(std::move(request)); // NOLINT(*)
  }

  {
    Json::Value request = Json::objectValue;
    request["kind"] = ObjectKindName(ObjectKind::CMakeFiles);
    Json::Value& versions = request["version"] = Json::arrayValue;
    versions.append(BuildVersion(1, CMakeFilesV1Minor));
    requests.append(std::move(request)); // NOLINT(*)
  }

  {
    Json::Value request = Json::objectValue;
    request["kind"] = ObjectKindName(ObjectKind::Toolchains);
    Json::Value& versions = request["version"] = Json::arrayValue;
    versions.append(BuildVersion(1, ToolchainsV1Minor));
    requests.append(std::move(request)); // NOLINT(*)
  }

  return capabilities;
}

bool cmFileAPI::AddProjectQuery(cmFileAPI::ObjectKind kind,
                                unsigned majorVersion, unsigned minorVersion)
{
  switch (kind) {
    case ObjectKind::CodeModel:
      if (majorVersion != 2 || minorVersion > CodeModelV2Minor) {
        return false;
      }
      break;
    case ObjectKind::Cache:
      if (majorVersion != 2 || minorVersion > CacheV2Minor) {
        return false;
      }
      break;
    case ObjectKind::CMakeFiles:
      if (majorVersion != 1 || minorVersion > CMakeFilesV1Minor) {
        return false;
      }
      break;
    case ObjectKind::Toolchains:
      if (majorVersion != 1 || minorVersion > ToolchainsV1Minor) {
        return false;
      }
      break;
    // These cannot be requested by the project
    case ObjectKind::ConfigureLog:
    case ObjectKind::InternalTest:
      return false;
  }

  Object query;
  query.Kind = kind;
  query.Version = majorVersion;
  if (std::find(this->TopQuery.Known.begin(), this->TopQuery.Known.end(),
                query) == this->TopQuery.Known.end()) {
    this->TopQuery.Known.emplace_back(query);
    this->QueryExists = true;
  }

  return true;
}
