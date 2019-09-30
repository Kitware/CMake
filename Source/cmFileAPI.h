/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmFileAPI_h
#define cmFileAPI_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "cm_jsoncpp_reader.h"
#include "cm_jsoncpp_value.h"
#include "cm_jsoncpp_writer.h"

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

  /** Convert a JSON object or array into an object with a single
      "jsonFile" member specifying a file named with the given prefix
      and holding the original object.  Other JSON types are unchanged.  */
  Json::Value MaybeJsonFile(Json::Value in, std::string const& prefix);

  /** Report file-api capabilities for cmake -E capabilities.  */
  static Json::Value ReportCapabilities();

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
    CodeModel,
    Cache,
    CMakeFiles,
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

  /** Represent one request in a client 'query.json'.  */
  struct ClientRequest : public Object
  {
    /** Empty if request is valid, else the error string.  */
    std::string Error;
  };

  /** Represent the "requests" in a client 'query.json'.  */
  struct ClientRequests : public std::vector<ClientRequest>
  {
    /** Empty if requests field is valid, else the error string.  */
    std::string Error;
  };

  /** Represent the content of a client query.json file.  */
  struct ClientQueryJson
  {
    /** The error string if parsing failed, else empty.  */
    std::string Error;

    /** The 'query.json' object "client" member if it exists, else null.  */
    Json::Value ClientValue;

    /** The 'query.json' object "requests" member if it exists, else null.  */
    Json::Value RequestsValue;

    /** Requests extracted from 'query.json'.  */
    ClientRequests Requests;
  };

  /** Represent content of a client query directory.  */
  struct ClientQuery
  {
    /** The content of the client query directory except 'query.json'.  */
    Query DirQuery;

    /** True if 'query.json' exists.  */
    bool HaveQueryJson = false;

    /** The 'query.json' content.  */
    ClientQueryJson QueryJson;
  };

  /** Whether the top-level query directory exists at all.  */
  bool QueryExists = false;

  /** The content of the top-level query directory.  */
  Query TopQuery;

  /** The content of each "client-$client" query directory.  */
  std::map<std::string, ClientQuery> ClientQueries;

  /** Reply index object generated for object kind/version.
      This populates the "objects" field of the reply index.  */
  std::map<Object, Json::Value> ReplyIndexObjects;

  std::unique_ptr<Json::CharReader> JsonReader;
  std::unique_ptr<Json::StreamWriter> JsonWriter;

  bool ReadJsonFile(std::string const& file, Json::Value& value,
                    std::string& error);

  std::string WriteJsonFile(
    Json::Value const& value, std::string const& prefix,
    std::string (*computeSuffix)(std::string const&) = ComputeSuffixHash);
  static std::string ComputeSuffixHash(std::string const&);
  static std::string ComputeSuffixTime(std::string const&);

  static bool ReadQuery(std::string const& query,
                        std::vector<Object>& objects);
  void ReadClient(std::string const& client);
  void ReadClientQuery(std::string const& client, ClientQueryJson& q);

  Json::Value BuildReplyIndex();
  Json::Value BuildCMake();
  Json::Value BuildReply(Query const& q);
  static Json::Value BuildReplyError(std::string const& error);
  Json::Value const& AddReplyIndexObject(Object const& o);

  static const char* ObjectKindName(ObjectKind kind);
  static std::string ObjectName(Object const& o);

  static Json::Value BuildVersion(unsigned int major, unsigned int minor);

  Json::Value BuildObject(Object const& object);

  ClientRequests BuildClientRequests(Json::Value const& requests);
  ClientRequest BuildClientRequest(Json::Value const& request);
  Json::Value BuildClientReply(ClientQuery const& q);
  Json::Value BuildClientReplyResponses(ClientRequests const& requests);
  Json::Value BuildClientReplyResponse(ClientRequest const& request);

  struct RequestVersion
  {
    unsigned int Major = 0;
    unsigned int Minor = 0;
  };
  static bool ReadRequestVersions(Json::Value const& version,
                                  std::vector<RequestVersion>& versions,
                                  std::string& error);
  static bool ReadRequestVersion(Json::Value const& version, bool inArray,
                                 std::vector<RequestVersion>& result,
                                 std::string& error);
  static std::string NoSupportedVersion(
    std::vector<RequestVersion> const& versions);

  void BuildClientRequestCodeModel(
    ClientRequest& r, std::vector<RequestVersion> const& versions);
  Json::Value BuildCodeModel(Object const& object);

  void BuildClientRequestCache(ClientRequest& r,
                               std::vector<RequestVersion> const& versions);
  Json::Value BuildCache(Object const& object);

  void BuildClientRequestCMakeFiles(
    ClientRequest& r, std::vector<RequestVersion> const& versions);
  Json::Value BuildCMakeFiles(Object const& object);

  void BuildClientRequestInternalTest(
    ClientRequest& r, std::vector<RequestVersion> const& versions);
  Json::Value BuildInternalTest(Object const& object);
};

#endif
