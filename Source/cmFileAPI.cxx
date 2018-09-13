/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmFileAPI.h"

#include "cmCryptoHash.h"
#include "cmSystemTools.h"
#include "cmTimestamp.h"
#include "cmake.h"
#include "cmsys/Directory.hxx"
#include "cmsys/FStream.hxx"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <utility>

cmFileAPI::cmFileAPI(cmake* cm)
  : CMakeInstance(cm)
{
  this->APIv1 =
    this->CMakeInstance->GetHomeOutputDirectory() + "/.cmake/api/v1";

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
    if (!cmFileAPI::ReadQuery(query, this->TopQuery.Known)) {
      this->TopQuery.Unknown.push_back(std::move(query));
    }
  }
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
      std::string file = reply_dir + "/" + f;
      cmSystemTools::RemoveFile(file);
    }
  }
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
      !cmSystemTools::RenameFile(tmpFile.c_str(), file.c_str())) {
    cmSystemTools::RemoveFile(tmpFile);
  }

  // Record this among files we have just written.
  this->ReplyFiles.insert(fileName);

  return fileName;
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

Json::Value cmFileAPI::BuildReplyIndex()
{
  Json::Value index(Json::objectValue);

  // Report information about this version of CMake.
  index["cmake"] = this->BuildCMake();

  // Reply to all queries that we loaded.
  index["reply"] = this->BuildReply(this->TopQuery);

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
    "__test" //
  };
  return objectKindNames[size_t(kind)];
}

std::string cmFileAPI::ObjectName(Object const& o)
{
  std::string name = ObjectKindName(o.Kind);
  name += "-v";
  name += std::to_string(o.Version);
  return name;
}

Json::Value cmFileAPI::BuildObject(Object const& object)
{
  Json::Value value;

  switch (object.Kind) {
    case ObjectKind::InternalTest:
      value = this->BuildInternalTest(object);
      break;
  }

  return value;
}

// The "__test" object kind is for internal testing of CMake.

static unsigned int const InternalTestV1Minor = 3;
static unsigned int const InternalTestV2Minor = 0;

Json::Value cmFileAPI::BuildInternalTest(Object const& object)
{
  Json::Value test = Json::objectValue;
  test["kind"] = this->ObjectKindName(object.Kind);
  Json::Value& version = test["version"] = Json::objectValue;
  if (object.Version == 2) {
    version["major"] = 2;
    version["minor"] = InternalTestV2Minor;
  } else {
    version["major"] = 1;
    version["minor"] = InternalTestV1Minor;
  }
  return test;
}
