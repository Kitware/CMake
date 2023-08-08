/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestSubmitHandler.h"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <sstream>

#include <cm/iomanip>
#include <cmext/algorithm>

#include <cm3p/curl/curl.h>
#include <cm3p/json/reader.h>
#include <cm3p/json/value.h>

#include "cmAlgorithms.h"
#include "cmCTest.h"
#include "cmCTestCurl.h"
#include "cmCTestScriptHandler.h"
#include "cmCryptoHash.h"
#include "cmCurl.h"
#include "cmDuration.h"
#include "cmGeneratedFileStream.h"
#include "cmList.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmXMLParser.h"
#include "cmake.h"

#define SUBMIT_TIMEOUT_IN_SECONDS_DEFAULT 120

using cmCTestSubmitHandlerVectorOfChar = std::vector<char>;

class cmCTestSubmitHandler::ResponseParser : public cmXMLParser
{
public:
  enum StatusType
  {
    STATUS_OK,
    STATUS_WARNING,
    STATUS_ERROR
  };

  StatusType Status = STATUS_OK;
  std::string Filename;
  std::string MD5;
  std::string Message;
  std::string BuildID;

private:
  std::vector<char> CurrentValue;

  std::string GetCurrentValue()
  {
    std::string val;
    if (!this->CurrentValue.empty()) {
      val.assign(this->CurrentValue.data(), this->CurrentValue.size());
    }
    return val;
  }

  void StartElement(const std::string& /*name*/,
                    const char** /*atts*/) override
  {
    this->CurrentValue.clear();
  }

  void CharacterDataHandler(const char* data, int length) override
  {
    cm::append(this->CurrentValue, data, data + length);
  }

  void EndElement(const std::string& name) override
  {
    if (name == "status") {
      std::string status = cmSystemTools::UpperCase(this->GetCurrentValue());
      if (status == "OK" || status == "SUCCESS") {
        this->Status = STATUS_OK;
      } else if (status == "WARNING") {
        this->Status = STATUS_WARNING;
      } else {
        this->Status = STATUS_ERROR;
      }
    } else if (name == "filename") {
      this->Filename = this->GetCurrentValue();
    } else if (name == "md5") {
      this->MD5 = this->GetCurrentValue();
    } else if (name == "message") {
      this->Message = this->GetCurrentValue();
    } else if (name == "buildId") {
      this->BuildID = this->GetCurrentValue();
    }
  }
};

static size_t cmCTestSubmitHandlerWriteMemoryCallback(void* ptr, size_t size,
                                                      size_t nmemb, void* data)
{
  int realsize = static_cast<int>(size * nmemb);
  const char* chPtr = static_cast<char*>(ptr);
  cm::append(*static_cast<cmCTestSubmitHandlerVectorOfChar*>(data), chPtr,
             chPtr + realsize);
  return realsize;
}

static size_t cmCTestSubmitHandlerCurlDebugCallback(CURL* /*unused*/,
                                                    curl_infotype /*unused*/,
                                                    char* chPtr, size_t size,
                                                    void* data)
{
  cm::append(*static_cast<cmCTestSubmitHandlerVectorOfChar*>(data), chPtr,
             chPtr + size);
  return 0;
}

cmCTestSubmitHandler::cmCTestSubmitHandler()
{
  this->Initialize();
}

void cmCTestSubmitHandler::Initialize()
{
  // We submit all available parts by default.
  for (cmCTest::Part p = cmCTest::PartStart; p != cmCTest::PartCount;
       p = static_cast<cmCTest::Part>(p + 1)) {
    this->SubmitPart[p] = true;
  }
  this->HasWarnings = false;
  this->HasErrors = false;
  this->Superclass::Initialize();
  this->HTTPProxy.clear();
  this->HTTPProxyType = 0;
  this->HTTPProxyAuth.clear();
  this->LogFile = nullptr;
  this->Files.clear();
}

bool cmCTestSubmitHandler::SubmitUsingHTTP(
  const std::string& localprefix, const std::vector<std::string>& files,
  const std::string& remoteprefix, const std::string& url)
{
  CURL* curl;
  FILE* ftpfile;
  char error_buffer[1024];
  // Set Content-Type to satisfy fussy modsecurity rules.
  struct curl_slist* headers =
    ::curl_slist_append(nullptr, "Content-Type: text/xml");

  // Add any additional headers that the user specified.
  for (std::string const& h : this->HttpHeaders) {
    cmCTestOptionalLog(this->CTest, DEBUG,
                       "   Add HTTP Header: \"" << h << "\"" << std::endl,
                       this->Quiet);
    headers = ::curl_slist_append(headers, h.c_str());
  }

  /* In windows, this will init the winsock stuff */
  ::curl_global_init(CURL_GLOBAL_ALL);
  std::string curlopt(this->CTest->GetCTestConfiguration("CurlOptions"));
  cmList args{ curlopt };
  bool verifyPeerOff = false;
  bool verifyHostOff = false;
  for (std::string const& arg : args) {
    if (arg == "CURLOPT_SSL_VERIFYPEER_OFF") {
      verifyPeerOff = true;
    }
    if (arg == "CURLOPT_SSL_VERIFYHOST_OFF") {
      verifyHostOff = true;
    }
  }
  for (std::string const& file : files) {
    /* get a curl handle */
    curl = curl_easy_init();
    if (curl) {
      cmCurlSetCAInfo(curl);
      if (verifyPeerOff) {
        cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                           "  Set CURLOPT_SSL_VERIFYPEER to off\n",
                           this->Quiet);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
      }
      if (verifyHostOff) {
        cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                           "  Set CURLOPT_SSL_VERIFYHOST to off\n",
                           this->Quiet);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
      }

      // Using proxy
      if (this->HTTPProxyType > 0) {
        curl_easy_setopt(curl, CURLOPT_PROXY, this->HTTPProxy.c_str());
        switch (this->HTTPProxyType) {
          case 2:
            curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4);
            break;
          case 3:
            curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);
            break;
          default:
            curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
            if (!this->HTTPProxyAuth.empty()) {
              curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD,
                               this->HTTPProxyAuth.c_str());
            }
        }
      }
      if (this->CTest->ShouldUseHTTP10()) {
        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
      }
      /* enable uploading */
      curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);

      // if there is little to no activity for too long stop submitting
      ::curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1);
      auto submitInactivityTimeout = this->GetSubmitInactivityTimeout();
      if (submitInactivityTimeout != 0) {
        ::curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME,
                           submitInactivityTimeout);
      }

      ::curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

      ::curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

      std::string local_file = file;
      bool initialize_cdash_buildid = false;
      if (!cmSystemTools::FileExists(local_file)) {
        local_file = cmStrCat(localprefix, "/", file);
        // If this file exists within the local Testing directory we assume
        // that it will be associated with the current build in CDash.
        initialize_cdash_buildid = true;
      }
      std::string remote_file =
        remoteprefix + cmSystemTools::GetFilenameName(file);

      *this->LogFile << "\tUpload file: " << local_file << " to "
                     << remote_file << std::endl;

      std::string ofile = cmSystemTools::EncodeURL(remote_file);
      std::string upload_as =
        cmStrCat(url, ((url.find('?') == std::string::npos) ? '?' : '&'),
                 "FileName=", ofile);

      if (initialize_cdash_buildid) {
        // Provide extra arguments to CDash so that it can initialize and
        // return a buildid.
        cmCTestCurl ctest_curl(this->CTest);
        upload_as += "&build=";
        upload_as +=
          ctest_curl.Escape(this->CTest->GetCTestConfiguration("BuildName"));
        upload_as += "&site=";
        upload_as +=
          ctest_curl.Escape(this->CTest->GetCTestConfiguration("Site"));
        upload_as += "&stamp=";
        upload_as += ctest_curl.Escape(this->CTest->GetCurrentTag());
        upload_as += "-";
        upload_as += ctest_curl.Escape(this->CTest->GetTestModelString());
        cmCTestScriptHandler* ch = this->CTest->GetScriptHandler();
        cmake* cm = ch->GetCMake();
        if (cm) {
          cmValue subproject = cm->GetState()->GetGlobalProperty("SubProject");
          if (subproject) {
            upload_as += "&subproject=";
            upload_as += ctest_curl.Escape(*subproject);
          }
        }
      }

      // Generate Done.xml right before it is submitted.
      // The reason for this is two-fold:
      // 1) It must be generated after some other part has been submitted
      //    so we have a buildId to refer to in its contents.
      // 2) By generating Done.xml here its timestamp will be as late as
      //    possible. This gives us a more accurate record of how long the
      //    entire build took to complete.
      if (file == "Done.xml") {
        this->CTest->GenerateDoneFile();
      }

      upload_as += "&MD5=";

      if (cmIsOn(this->GetOption("InternalTest"))) {
        upload_as += "ffffffffffffffffffffffffffffffff";
      } else {
        cmCryptoHash hasher(cmCryptoHash::AlgoMD5);
        upload_as += hasher.HashFile(local_file);
      }

      if (!cmSystemTools::FileExists(local_file)) {
        cmCTestLog(this->CTest, ERROR_MESSAGE,
                   "   Cannot find file: " << local_file << std::endl);
        ::curl_easy_cleanup(curl);
        ::curl_slist_free_all(headers);
        ::curl_global_cleanup();
        return false;
      }
      unsigned long filelen = cmSystemTools::FileLength(local_file);

      ftpfile = cmsys::SystemTools::Fopen(local_file, "rb");
      cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                         "   Upload file: " << local_file << " to "
                                            << upload_as << " Size: "
                                            << filelen << std::endl,
                         this->Quiet);

      // specify target
      ::curl_easy_setopt(curl, CURLOPT_URL, upload_as.c_str());

      // CURLAUTH_BASIC is default, and here we allow additional methods,
      // including more secure ones
      ::curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_ANY);

      // now specify which file to upload
      ::curl_easy_setopt(curl, CURLOPT_INFILE, ftpfile);

      // and give the size of the upload (optional)
      ::curl_easy_setopt(curl, CURLOPT_INFILESIZE, static_cast<long>(filelen));

      // and give curl the buffer for errors
      ::curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, &error_buffer);

      // specify handler for output
      ::curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                         cmCTestSubmitHandlerWriteMemoryCallback);
      ::curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION,
                         cmCTestSubmitHandlerCurlDebugCallback);

      /* we pass our 'chunk' struct to the callback function */
      cmCTestSubmitHandlerVectorOfChar chunk;
      cmCTestSubmitHandlerVectorOfChar chunkDebug;
      ::curl_easy_setopt(curl, CURLOPT_FILE, &chunk);
      ::curl_easy_setopt(curl, CURLOPT_DEBUGDATA, &chunkDebug);

      // Now run off and do what you've been told!
      ::curl_easy_perform(curl);

      if (!chunk.empty()) {
        cmCTestOptionalLog(this->CTest, DEBUG,
                           "CURL output: ["
                             << cmCTestLogWrite(chunk.data(), chunk.size())
                             << "]" << std::endl,
                           this->Quiet);
        this->ParseResponse(chunk);
      }
      if (!chunkDebug.empty()) {
        cmCTestOptionalLog(
          this->CTest, DEBUG,
          "CURL debug output: ["
            << cmCTestLogWrite(chunkDebug.data(), chunkDebug.size()) << "]"
            << std::endl,
          this->Quiet);
      }

      // If curl failed for any reason, or checksum fails, wait and retry
      //
      long response_code;
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
      bool successful_submission = response_code == 200;

      if (!successful_submission || this->HasErrors) {
        std::string retryDelay = *this->GetOption("RetryDelay");
        std::string retryCount = *this->GetOption("RetryCount");

        auto delay = cmDuration(
          retryDelay.empty()
            ? atoi(this->CTest->GetCTestConfiguration("CTestSubmitRetryDelay")
                     .c_str())
            : atoi(retryDelay.c_str()));
        int count = retryCount.empty()
          ? atoi(this->CTest->GetCTestConfiguration("CTestSubmitRetryCount")
                   .c_str())
          : atoi(retryCount.c_str());

        for (int i = 0; i < count; i++) {
          cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                             "   Submit failed, waiting " << delay.count()
                                                          << " seconds...\n",
                             this->Quiet);

          auto stop = std::chrono::steady_clock::now() + delay;
          while (std::chrono::steady_clock::now() < stop) {
            cmSystemTools::Delay(100);
          }

          cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                             "   Retry submission: Attempt "
                               << (i + 1) << " of " << count << std::endl,
                             this->Quiet);

          ::fclose(ftpfile);
          ftpfile = cmsys::SystemTools::Fopen(local_file, "rb");
          ::curl_easy_setopt(curl, CURLOPT_INFILE, ftpfile);

          chunk.clear();
          chunkDebug.clear();
          this->HasErrors = false;

          ::curl_easy_perform(curl);

          if (!chunk.empty()) {
            cmCTestOptionalLog(this->CTest, DEBUG,
                               "CURL output: ["
                                 << cmCTestLogWrite(chunk.data(), chunk.size())
                                 << "]" << std::endl,
                               this->Quiet);
            this->ParseResponse(chunk);
          }

          curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
          if (response_code == 200 && !this->HasErrors) {
            successful_submission = true;
            break;
          }
        }
      }

      fclose(ftpfile);
      if (!successful_submission) {
        cmCTestLog(this->CTest, ERROR_MESSAGE,
                   "   Error when uploading file: " << local_file
                                                    << std::endl);
        cmCTestLog(this->CTest, ERROR_MESSAGE,
                   "   Error message was: " << error_buffer << std::endl);
        *this->LogFile << "   Error when uploading file: " << local_file
                       << std::endl
                       << "   Error message was: " << error_buffer
                       << std::endl;
        // avoid deref of begin for zero size array
        if (!chunk.empty()) {
          *this->LogFile << "   Curl output was: "
                         << cmCTestLogWrite(chunk.data(), chunk.size())
                         << std::endl;
          cmCTestLog(this->CTest, ERROR_MESSAGE,
                     "CURL output: ["
                       << cmCTestLogWrite(chunk.data(), chunk.size()) << "]"
                       << std::endl);
        }
        ::curl_easy_cleanup(curl);
        ::curl_slist_free_all(headers);
        ::curl_global_cleanup();
        return false;
      }
      // always cleanup
      ::curl_easy_cleanup(curl);
      cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                         "   Uploaded: " + local_file << std::endl,
                         this->Quiet);
    }
  }
  ::curl_slist_free_all(headers);
  ::curl_global_cleanup();
  return true;
}

void cmCTestSubmitHandler::ParseResponse(
  cmCTestSubmitHandlerVectorOfChar chunk)
{
  std::string output;
  output.append(chunk.begin(), chunk.end());

  if (output.find("<cdash") != std::string::npos) {
    ResponseParser parser;
    parser.Parse(output.c_str());

    if (parser.Status != ResponseParser::STATUS_OK) {
      this->HasErrors = true;
      cmCTestLog(this->CTest, HANDLER_OUTPUT,
                 "   Submission failed: " << parser.Message << std::endl);
      return;
    }
    this->CTest->SetBuildID(parser.BuildID);
  }
  output = cmSystemTools::UpperCase(output);
  if (output.find("WARNING") != std::string::npos) {
    this->HasWarnings = true;
  }
  if (output.find("ERROR") != std::string::npos) {
    this->HasErrors = true;
  }

  if (this->HasWarnings || this->HasErrors) {
    cmCTestLog(this->CTest, HANDLER_OUTPUT,
               "   Server Response:\n"
                 << cmCTestLogWrite(chunk.data(), chunk.size()) << "\n");
  }
}

int cmCTestSubmitHandler::HandleCDashUploadFile(std::string const& file,
                                                std::string const& typeString)
{
  if (file.empty()) {
    cmCTestLog(this->CTest, ERROR_MESSAGE, "Upload file not specified\n");
    return -1;
  }
  if (!cmSystemTools::FileExists(file)) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Upload file not found: '" << file << "'\n");
    return -1;
  }
  cmCTestCurl curl(this->CTest);
  curl.SetQuiet(this->Quiet);
  std::string curlopt(this->CTest->GetCTestConfiguration("CurlOptions"));
  cmList args{ curlopt };
  curl.SetCurlOptions(args);
  auto submitInactivityTimeout = this->GetSubmitInactivityTimeout();
  if (submitInactivityTimeout != 0) {
    curl.SetTimeOutSeconds(submitInactivityTimeout);
  }
  curl.SetHttpHeaders(this->HttpHeaders);
  std::string url = this->CTest->GetSubmitURL();
  if (!cmHasLiteralPrefix(url, "http://") &&
      !cmHasLiteralPrefix(url, "https://")) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Only http and https are supported for CDASH_UPLOAD\n");
    return -1;
  }

  std::string fields;
  std::string::size_type pos = url.find('?');
  if (pos != std::string::npos) {
    fields = url.substr(pos + 1);
    url.erase(pos);
  }
  bool internalTest = cmIsOn(this->GetOption("InternalTest"));

  // Get RETRY_COUNT and RETRY_DELAY values if they were set.
  std::string retryDelayString = *this->GetOption("RetryDelay");
  std::string retryCountString = *this->GetOption("RetryCount");
  auto retryDelay = std::chrono::seconds(0);
  if (!retryDelayString.empty()) {
    unsigned long retryDelayValue = 0;
    if (!cmStrToULong(retryDelayString, &retryDelayValue)) {
      cmCTestLog(this->CTest, WARNING,
                 "Invalid value for 'RETRY_DELAY' : " << retryDelayString
                                                      << std::endl);
    } else {
      retryDelay = std::chrono::seconds(retryDelayValue);
    }
  }
  unsigned long retryCount = 0;
  if (!retryCountString.empty()) {
    if (!cmStrToULong(retryCountString, &retryCount)) {
      cmCTestLog(this->CTest, WARNING,
                 "Invalid value for 'RETRY_DELAY' : " << retryCountString
                                                      << std::endl);
    }
  }

  cmCryptoHash hasher(cmCryptoHash::AlgoMD5);
  std::string md5sum = hasher.HashFile(file);
  // 1. request the buildid and check to see if the file
  //    has already been uploaded
  // TODO I added support for subproject. You would need to add
  // a "&subproject=subprojectname" to the first POST.
  cmCTestScriptHandler* ch = this->CTest->GetScriptHandler();
  cmake* cm = ch->GetCMake();
  cmValue subproject = cm->GetState()->GetGlobalProperty("SubProject");
  // TODO: Encode values for a URL instead of trusting caller.
  std::ostringstream str;
  if (subproject) {
    str << "subproject=" << curl.Escape(*subproject) << "&";
  }
  auto timeNow =
    std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  str << "stamp=" << curl.Escape(this->CTest->GetCurrentTag()) << "-"
      << curl.Escape(this->CTest->GetTestModelString()) << "&"
      << "model=" << curl.Escape(this->CTest->GetTestModelString()) << "&"
      << "build="
      << curl.Escape(this->CTest->GetCTestConfiguration("BuildName")) << "&"
      << "site=" << curl.Escape(this->CTest->GetCTestConfiguration("Site"))
      << "&"
      << "group=" << curl.Escape(this->CTest->GetTestModelString())
      << "&"
      // For now, we send both "track" and "group" to CDash in case we're
      // submitting to an older instance that still expects the prior
      // terminology.
      << "track=" << curl.Escape(this->CTest->GetTestModelString()) << "&"
      << "starttime=" << timeNow << "&"
      << "endtime=" << timeNow << "&"
      << "datafilesmd5[0]=" << md5sum << "&"
      << "type=" << curl.Escape(typeString);
  if (!fields.empty()) {
    fields += '&';
  }
  fields += str.str();
  cmCTestOptionalLog(this->CTest, DEBUG,
                     "fields: " << fields << "\nurl:" << url
                                << "\nfile: " << file << "\n",
                     this->Quiet);
  std::string response;

  bool requestSucceeded = curl.HttpRequest(url, fields, response);
  if (!internalTest && !requestSucceeded) {
    // If request failed, wait and retry.
    for (unsigned long i = 0; i < retryCount; i++) {
      cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                         "   Request failed, waiting " << retryDelay.count()
                                                       << " seconds...\n",
                         this->Quiet);

      auto stop = std::chrono::steady_clock::now() + retryDelay;
      while (std::chrono::steady_clock::now() < stop) {
        cmSystemTools::Delay(100);
      }

      cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                         "   Retry request: Attempt "
                           << (i + 1) << " of " << retryCount << std::endl,
                         this->Quiet);

      requestSucceeded = curl.HttpRequest(url, fields, response);
      if (requestSucceeded) {
        break;
      }
    }
  }
  if (!internalTest && !requestSucceeded) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Error in HttpRequest\n"
                 << response);
    return -1;
  }
  cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                     "Request upload response: [" << response << "]\n",
                     this->Quiet);
  Json::Value json;
  Json::Reader reader;
  if (!internalTest && !reader.parse(response, json)) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "error parsing json string ["
                 << response << "]\n"
                 << reader.getFormattedErrorMessages() << "\n");
    return -1;
  }
  if (!internalTest && json["status"].asInt() != 0) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Bad status returned from CDash: " << json["status"].asInt());
    return -1;
  }
  if (!internalTest) {
    if (json["datafilesmd5"].isArray()) {
      int datares = json["datafilesmd5"][0].asInt();
      if (datares == 1) {
        cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                           "File already exists on CDash, skip upload "
                             << file << "\n",
                           this->Quiet);
        return 0;
      }
    } else {
      cmCTestLog(this->CTest, ERROR_MESSAGE,
                 "bad datafilesmd5 value in response " << response << "\n");
      return -1;
    }
  }

  std::string upload_as = cmSystemTools::GetFilenameName(file);
  std::ostringstream fstr;
  fstr << "type=" << curl.Escape(typeString) << "&"
       << "md5=" << md5sum << "&"
       << "filename=" << curl.Escape(upload_as) << "&"
       << "buildid=" << json["buildid"].asString();

  bool uploadSucceeded = false;
  if (!internalTest) {
    uploadSucceeded = curl.UploadFile(file, url, fstr.str(), response);
  }

  if (!uploadSucceeded) {
    // If upload failed, wait and retry.
    for (unsigned long i = 0; i < retryCount; i++) {
      cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                         "   Upload failed, waiting " << retryDelay.count()
                                                      << " seconds...\n",
                         this->Quiet);

      auto stop = std::chrono::steady_clock::now() + retryDelay;
      while (std::chrono::steady_clock::now() < stop) {
        cmSystemTools::Delay(100);
      }

      cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                         "   Retry upload: Attempt "
                           << (i + 1) << " of " << retryCount << std::endl,
                         this->Quiet);

      if (!internalTest) {
        uploadSucceeded = curl.UploadFile(file, url, fstr.str(), response);
      }
      if (uploadSucceeded) {
        break;
      }
    }
  }

  if (!uploadSucceeded) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "error uploading to CDash. " << file << " " << url << " "
                                            << fstr.str());
    return -1;
  }
  if (!reader.parse(response, json)) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "error parsing json string ["
                 << response << "]\n"
                 << reader.getFormattedErrorMessages() << "\n");
    return -1;
  }
  cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                     "Upload file response: [" << response << "]\n",
                     this->Quiet);
  return 0;
}

int cmCTestSubmitHandler::ProcessHandler()
{
  cmValue cdashUploadFile = this->GetOption("CDashUploadFile");
  cmValue cdashUploadType = this->GetOption("CDashUploadType");
  if (cdashUploadFile && cdashUploadType) {
    return this->HandleCDashUploadFile(*cdashUploadFile, *cdashUploadType);
  }

  const std::string& buildDirectory =
    this->CTest->GetCTestConfiguration("BuildDirectory");
  if (buildDirectory.empty()) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Cannot find BuildDirectory  key in the DartConfiguration.tcl"
                 << std::endl);
    return -1;
  }

  if (char const* proxy = getenv("HTTP_PROXY")) {
    this->HTTPProxyType = 1;
    this->HTTPProxy = proxy;
    if (getenv("HTTP_PROXY_PORT")) {
      this->HTTPProxy += ":";
      this->HTTPProxy += getenv("HTTP_PROXY_PORT");
    }
    if (char const* proxy_type = getenv("HTTP_PROXY_TYPE")) {
      std::string type = proxy_type;
      // HTTP/SOCKS4/SOCKS5
      if (type == "HTTP") {
        this->HTTPProxyType = 1;
      } else if (type == "SOCKS4") {
        this->HTTPProxyType = 2;
      } else if (type == "SOCKS5") {
        this->HTTPProxyType = 3;
      }
    }
    if (getenv("HTTP_PROXY_USER")) {
      this->HTTPProxyAuth = getenv("HTTP_PROXY_USER");
    }
    if (getenv("HTTP_PROXY_PASSWD")) {
      this->HTTPProxyAuth += ":";
      this->HTTPProxyAuth += getenv("HTTP_PROXY_PASSWD");
    }
  }

  if (!this->HTTPProxy.empty()) {
    cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                       "   Use HTTP Proxy: " << this->HTTPProxy << std::endl,
                       this->Quiet);
  }
  cmGeneratedFileStream ofs;
  this->StartLogFile("Submit", ofs);

  std::vector<std::string> files;
  std::string prefix = this->GetSubmitResultsPrefix();

  if (!this->Files.empty()) {
    // Submit the explicitly selected files:
    cm::append(files, this->Files);
  }

  // Add to the list of files to submit from any selected, existing parts:
  //

  // TODO:
  // Check if test is enabled

  this->CTest->AddIfExists(cmCTest::PartUpdate, "Update.xml");
  this->CTest->AddIfExists(cmCTest::PartConfigure, "Configure.xml");
  this->CTest->AddIfExists(cmCTest::PartBuild, "Build.xml");
  this->CTest->AddIfExists(cmCTest::PartTest, "Test.xml");
  if (this->CTest->AddIfExists(cmCTest::PartCoverage, "Coverage.xml")) {
    std::vector<std::string> gfiles;
    std::string gpath =
      buildDirectory + "/Testing/" + this->CTest->GetCurrentTag();
    std::string::size_type glen = gpath.size() + 1;
    gpath = gpath + "/CoverageLog*";
    cmCTestOptionalLog(this->CTest, DEBUG,
                       "Globbing for: " << gpath << std::endl, this->Quiet);
    if (cmSystemTools::SimpleGlob(gpath, gfiles, 1)) {
      for (std::string& gfile : gfiles) {
        gfile = gfile.substr(glen);
        cmCTestOptionalLog(this->CTest, DEBUG,
                           "Glob file: " << gfile << std::endl, this->Quiet);
        this->CTest->AddSubmitFile(cmCTest::PartCoverage, gfile);
      }
    } else {
      cmCTestLog(this->CTest, ERROR_MESSAGE, "Problem globbing" << std::endl);
    }
  }
  this->CTest->AddIfExists(cmCTest::PartMemCheck, "DynamicAnalysis.xml");
  this->CTest->AddIfExists(cmCTest::PartMemCheck, "DynamicAnalysis-Test.xml");
  this->CTest->AddIfExists(cmCTest::PartMemCheck, "Purify.xml");
  this->CTest->AddIfExists(cmCTest::PartNotes, "Notes.xml");
  this->CTest->AddIfExists(cmCTest::PartUpload, "Upload.xml");

  // Query parts for files to submit.
  for (cmCTest::Part p = cmCTest::PartStart; p != cmCTest::PartCount;
       p = static_cast<cmCTest::Part>(p + 1)) {
    // Skip parts we are not submitting.
    if (!this->SubmitPart[p]) {
      continue;
    }

    // Submit files from this part.
    cm::append(files, this->CTest->GetSubmitFiles(p));
  }

  // Make sure files are unique, but preserve order.
  {
    // This endPos intermediate is needed to work around non-conformant C++11
    // standard libraries that have erase(iterator,iterator) instead of
    // erase(const_iterator,const_iterator).
    size_t endPos = cmRemoveDuplicates(files) - files.cbegin();
    files.erase(files.begin() + endPos, files.end());
  }

  // Submit Done.xml last
  if (this->SubmitPart[cmCTest::PartDone]) {
    files.emplace_back("Done.xml");
  }

  if (ofs) {
    ofs << "Upload files:" << std::endl;
    int cnt = 0;
    for (std::string const& file : files) {
      ofs << cnt << "\t" << file << std::endl;
      cnt++;
    }
  }
  cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT, "Submit files\n",
                     this->Quiet);
  const char* specificGroup = this->CTest->GetSpecificGroup();
  if (specificGroup) {
    cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                       "   Send to group: " << specificGroup << std::endl,
                       this->Quiet);
  }
  this->SetLogFile(&ofs);

  std::string url = this->CTest->GetSubmitURL();
  cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                     "   SubmitURL: " << url << '\n', this->Quiet);
  if (!this->SubmitUsingHTTP(buildDirectory + "/Testing/" +
                               this->CTest->GetCurrentTag(),
                             files, prefix, url)) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "   Problems when submitting via HTTP\n");
    ofs << "   Problems when submitting via HTTP\n";
    return -1;
  }
  if (this->HasErrors) {
    cmCTestLog(this->CTest, HANDLER_OUTPUT,
               "   Errors occurred during submission.\n");
    ofs << "   Errors occurred during submission.\n";
  } else {
    cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                       "   Submission successful"
                         << (this->HasWarnings ? ", with warnings." : "")
                         << std::endl,
                       this->Quiet);
    ofs << "   Submission successful"
        << (this->HasWarnings ? ", with warnings." : "") << std::endl;
  }

  return 0;
}

std::string cmCTestSubmitHandler::GetSubmitResultsPrefix()
{
  std::string buildname =
    cmCTest::SafeBuildIdField(this->CTest->GetCTestConfiguration("BuildName"));
  std::string name = this->CTest->GetCTestConfiguration("Site") + "___" +
    buildname + "___" + this->CTest->GetCurrentTag() + "-" +
    this->CTest->GetTestModelString() + "___XML___";
  return name;
}

void cmCTestSubmitHandler::SelectParts(std::set<cmCTest::Part> const& parts)
{
  // Check whether each part is selected.
  for (cmCTest::Part p = cmCTest::PartStart; p != cmCTest::PartCount;
       p = static_cast<cmCTest::Part>(p + 1)) {
    this->SubmitPart[p] = parts.find(p) != parts.end();
  }
}

int cmCTestSubmitHandler::GetSubmitInactivityTimeout()
{
  int submitInactivityTimeout = SUBMIT_TIMEOUT_IN_SECONDS_DEFAULT;
  std::string const& timeoutStr =
    this->CTest->GetCTestConfiguration("SubmitInactivityTimeout");
  if (!timeoutStr.empty()) {
    unsigned long timeout;
    if (cmStrToULong(timeoutStr, &timeout)) {
      submitInactivityTimeout = static_cast<int>(timeout);
    } else {
      cmCTestLog(this->CTest, ERROR_MESSAGE,
                 "SubmitInactivityTimeout is invalid: "
                   << cm::quoted(timeoutStr) << "."
                   << " Using a default value of "
                   << SUBMIT_TIMEOUT_IN_SECONDS_DEFAULT << "." << std::endl);
    }
  }
  return submitInactivityTimeout;
}

void cmCTestSubmitHandler::SelectFiles(std::set<std::string> const& files)
{
  this->Files.insert(files.begin(), files.end());
}
