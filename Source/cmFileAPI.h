/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmFileAPI_h
#define cmFileAPI_h

#include "cmConfigure.h" // IWYU pragma: keep

#include "cm_jsoncpp_value.h"
#include "cm_jsoncpp_writer.h"

#include <map>
#include <memory> // IWYU pragma: keep
#include <string>
#include <unordered_set>
#include <vector>

class cmake;

class cmFileAPI
{
public:
  cmFileAPI(cmake* cm);

  /** Read fileapi queries from disk.  */
  void ReadQueries();

  /** Write fileapi replies to disk.  */
  void WriteReplies();

  /** Get the "cmake" instance with which this was constructed.  */
  cmake* GetCMakeInstance() const { return this->CMakeInstance; }

private:
  cmake* CMakeInstance;

  /** The api/v1 directory location.  */
  std::string APIv1;

  /** The set of files we have just written to the reply directory.  */
  std::unordered_set<std::string> ReplyFiles;

  static std::vector<std::string> LoadDir(std::string const& dir);
  void RemoveOldReplyFiles();

  // Keep in sync with ObjectKindName.
  enum class ObjectKind
  {
    InternalTest
  };

  /** Identify one object kind and major version.  */
  struct Object
  {
    ObjectKind Kind;
    unsigned long Version = 0;
    friend bool operator<(Object const& l, Object const& r)
    {
      if (l.Kind != r.Kind) {
        return l.Kind < r.Kind;
      }
      return l.Version < r.Version;
    }
  };

  /** Represent content of a query directory.  */
  struct Query
  {
    /** Known object kind-version pairs.  */
    std::vector<Object> Known;
    /** Unknown object kind names.  */
    std::vector<std::string> Unknown;
  };

  /** Whether the top-level query directory exists at all.  */
  bool QueryExists = false;

  /** The content of the top-level query directory.  */
  Query TopQuery;

  /** Reply index object generated for object kind/version.
      This populates the "objects" field of the reply index.  */
  std::map<Object, Json::Value> ReplyIndexObjects;

  std::unique_ptr<Json::StreamWriter> JsonWriter;

  std::string WriteJsonFile(
    Json::Value const& value, std::string const& prefix,
    std::string (*computeSuffix)(std::string const&) = ComputeSuffixHash);
  static std::string ComputeSuffixHash(std::string const&);
  static std::string ComputeSuffixTime(std::string const&);

  static bool ReadQuery(std::string const& query,
                        std::vector<Object>& objects);

  Json::Value BuildReplyIndex();
  Json::Value BuildCMake();
  Json::Value BuildReply(Query const& q);
  static Json::Value BuildReplyError(std::string const& error);
  Json::Value const& AddReplyIndexObject(Object const& o);

  static const char* ObjectKindName(ObjectKind kind);
  static std::string ObjectName(Object const& o);

  Json::Value BuildObject(Object const& object);

  Json::Value BuildInternalTest(Object const& object);
};

#endif
