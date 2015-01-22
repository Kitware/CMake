/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCTestSubmitHandler.h"
#include "cmCTestScriptHandler.h"
#include "cmake.h"
#include "cmSystemTools.h"
#include "cmVersion.h"
#include "cmGeneratedFileStream.h"
#include "cmCTest.h"
#include "cmXMLParser.h"

#include <cmsys/Process.h>
#include <cmsys/Base64.h>

// For XML-RPC submission
#include "cm_xmlrpc.h"

#include <cm_jsoncpp_reader.h>
// For curl submission
#include "cm_curl.h"
#include "cmCTestCurl.h"

#include <sys/stat.h>

#define SUBMIT_TIMEOUT_IN_SECONDS_DEFAULT 120

typedef std::vector<char> cmCTestSubmitHandlerVectorOfChar;

//----------------------------------------------------------------------------
class cmCTestSubmitHandler::ResponseParser: public cmXMLParser
{
public:
  ResponseParser() { this->Status = STATUS_OK; }
  ~ResponseParser() {}

public:

  enum StatusType
    {
    STATUS_OK,
    STATUS_WARNING,
    STATUS_ERROR
    };

  StatusType Status;
  std::string CDashVersion;
  std::string Filename;
  std::string MD5;
  std::string Message;

private:

  std::vector<char> CurrentValue;

  std::string GetCurrentValue()
    {
    std::string val;
    if(!this->CurrentValue.empty())
      {
      val.assign(&this->CurrentValue[0], this->CurrentValue.size());
      }
    return val;
    }

  virtual void StartElement(const std::string& name, const char** atts)
    {
    this->CurrentValue.clear();
    if(name == "cdash")
      {
      this->CDashVersion = this->FindAttribute(atts, "version");
      }
    }

  virtual void CharacterDataHandler(const char* data, int length)
    {
    this->CurrentValue.insert(this->CurrentValue.end(), data, data+length);
    }

  virtual void EndElement(const std::string& name)
    {
    if(name == "status")
      {
      std::string status = cmSystemTools::UpperCase(this->GetCurrentValue());
      if(status == "OK" || status == "SUCCESS")
        {
        this->Status = STATUS_OK;
        }
      else if(status == "WARNING")
        {
        this->Status = STATUS_WARNING;
        }
      else
        {
        this->Status = STATUS_ERROR;
        }
      }
    else if(name == "filename")
      {
      this->Filename = this->GetCurrentValue();
      }
    else if(name == "md5")
      {
      this->MD5 = this->GetCurrentValue();
      }
    else if(name == "message")
      {
      this->Message = this->GetCurrentValue();
      }
    }
};


static size_t
cmCTestSubmitHandlerWriteMemoryCallback(void *ptr, size_t size, size_t nmemb,
  void *data)
{
  int realsize = (int)(size * nmemb);

  cmCTestSubmitHandlerVectorOfChar *vec
    = static_cast<cmCTestSubmitHandlerVectorOfChar*>(data);
  const char* chPtr = static_cast<char*>(ptr);
  vec->insert(vec->end(), chPtr, chPtr + realsize);

  return realsize;
}

static size_t
cmCTestSubmitHandlerCurlDebugCallback(CURL *, curl_infotype, char *chPtr,
  size_t size, void *data)
{
  cmCTestSubmitHandlerVectorOfChar *vec
    = static_cast<cmCTestSubmitHandlerVectorOfChar*>(data);
  vec->insert(vec->end(), chPtr, chPtr + size);

  return size;
}

//----------------------------------------------------------------------------
cmCTestSubmitHandler::cmCTestSubmitHandler() : HTTPProxy(), FTPProxy()
{
  this->Initialize();
}

//----------------------------------------------------------------------------
void cmCTestSubmitHandler::Initialize()
{
  // We submit all available parts by default.
  for(cmCTest::Part p = cmCTest::PartStart;
      p != cmCTest::PartCount; p = cmCTest::Part(p+1))
    {
    this->SubmitPart[p] = true;
    }
  this->CDash = false;
  this->HasWarnings = false;
  this->HasErrors = false;
  this->Superclass::Initialize();
  this->HTTPProxy = "";
  this->HTTPProxyType = 0;
  this->HTTPProxyAuth = "";
  this->FTPProxy = "";
  this->FTPProxyType = 0;
  this->LogFile = 0;
  this->Files.clear();
}

//----------------------------------------------------------------------------
bool cmCTestSubmitHandler::SubmitUsingFTP(const std::string& localprefix,
  const std::set<std::string>& files,
  const std::string& remoteprefix,
  const std::string& url)
{
  CURL *curl;
  CURLcode res;
  FILE* ftpfile;
  char error_buffer[1024];

  /* In windows, this will init the winsock stuff */
  ::curl_global_init(CURL_GLOBAL_ALL);

  cmCTest::SetOfStrings::const_iterator file;
  for ( file = files.begin(); file != files.end(); ++file )
    {
    /* get a curl handle */
    curl = curl_easy_init();
    if(curl)
      {
      // Using proxy
      if ( this->FTPProxyType > 0 )
        {
        curl_easy_setopt(curl, CURLOPT_PROXY, this->FTPProxy.c_str());
        switch (this->FTPProxyType)
          {
        case 2:
          curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4);
          break;
        case 3:
          curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);
          break;
        default:
          curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
          }
        }

      // enable uploading
      ::curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);

      // if there is little to no activity for too long stop submitting
      ::curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1);
      ::curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME,
        SUBMIT_TIMEOUT_IN_SECONDS_DEFAULT);

      ::curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);

      std::string local_file = *file;
      if ( !cmSystemTools::FileExists(local_file.c_str()) )
        {
        local_file = localprefix + "/" + *file;
        }
      std::string upload_as
        = url + "/" + remoteprefix + cmSystemTools::GetFilenameName(*file);


      if ( !cmSystemTools::FileExists(local_file.c_str()) )
        {
        cmCTestLog(this->CTest, ERROR_MESSAGE, "   Cannot find file: "
          << local_file << std::endl);
        ::curl_easy_cleanup(curl);
        ::curl_global_cleanup();
        return false;
        }
      unsigned long filelen = cmSystemTools::FileLength(local_file);

      ftpfile = cmsys::SystemTools::Fopen(local_file, "rb");
      *this->LogFile << "\tUpload file: " << local_file << " to "
          << upload_as << std::endl;
      cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "   Upload file: "
        << local_file << " to "
        << upload_as << std::endl);

      ::curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

      // specify target
      ::curl_easy_setopt(curl,CURLOPT_URL, upload_as.c_str());

      // now specify which file to upload
      ::curl_easy_setopt(curl, CURLOPT_INFILE, ftpfile);

      // and give the size of the upload (optional)
      ::curl_easy_setopt(curl, CURLOPT_INFILESIZE,
        static_cast<long>(filelen));

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
      ::curl_easy_setopt(curl, CURLOPT_FILE, (void *)&chunk);
      ::curl_easy_setopt(curl, CURLOPT_DEBUGDATA, (void *)&chunkDebug);

      // Now run off and do what you've been told!
      res = ::curl_easy_perform(curl);

      if (!chunk.empty())
        {
        cmCTestLog(this->CTest, DEBUG, "CURL output: ["
          << cmCTestLogWrite(&*chunk.begin(), chunk.size()) << "]"
          << std::endl);
        }
      if (!chunkDebug.empty())
        {
        cmCTestLog(this->CTest, DEBUG, "CURL debug output: ["
          << cmCTestLogWrite(&*chunkDebug.begin(), chunkDebug.size()) << "]"
          << std::endl);
        }

      fclose(ftpfile);
      if ( res )
        {
        cmCTestLog(this->CTest, ERROR_MESSAGE,
          "   Error when uploading file: "
          << local_file << std::endl);
        cmCTestLog(this->CTest, ERROR_MESSAGE, "   Error message was: "
          << error_buffer << std::endl);
        *this->LogFile << "   Error when uploading file: "
                       << local_file
                       << std::endl
                       << "   Error message was: "
                       << error_buffer << std::endl
                       << "   Curl output was: ";
        // avoid dereference of empty vector
        if(!chunk.empty())
          {
          *this->LogFile << cmCTestLogWrite(&*chunk.begin(), chunk.size());
          cmCTestLog(this->CTest, ERROR_MESSAGE, "CURL output: ["
                     << cmCTestLogWrite(&*chunk.begin(), chunk.size()) << "]"
                     << std::endl);
          }
        *this->LogFile << std::endl;
        ::curl_easy_cleanup(curl);
        ::curl_global_cleanup();
        return false;
        }
      // always cleanup
      ::curl_easy_cleanup(curl);
      cmCTestLog(this->CTest, HANDLER_OUTPUT, "   Uploaded: " + local_file
        << std::endl);
      }
    }
  ::curl_global_cleanup();
  return true;
}

//----------------------------------------------------------------------------
// Uploading files is simpler
bool cmCTestSubmitHandler::SubmitUsingHTTP(const std::string& localprefix,
  const std::set<std::string>& files,
  const std::string& remoteprefix,
  const std::string& url)
{
  CURL *curl;
  CURLcode res;
  FILE* ftpfile;
  char error_buffer[1024];

  /* In windows, this will init the winsock stuff */
  ::curl_global_init(CURL_GLOBAL_ALL);
  std::string dropMethod(this->CTest->GetCTestConfiguration("DropMethod"));
  std::string curlopt(this->CTest->GetCTestConfiguration("CurlOptions"));
  std::vector<std::string> args;
  cmSystemTools::ExpandListArgument(curlopt, args);
  bool verifyPeerOff = false;
  bool verifyHostOff = false;
  for( std::vector<std::string>::iterator i = args.begin();
       i != args.end(); ++i)
    {
    if(*i == "CURLOPT_SSL_VERIFYPEER_OFF")
      {
      verifyPeerOff = true;
      }
    if(*i == "CURLOPT_SSL_VERIFYHOST_OFF")
      {
      verifyHostOff = true;
      }
    }
  std::string::size_type kk;
  cmCTest::SetOfStrings::const_iterator file;
  for ( file = files.begin(); file != files.end(); ++file )
    {
    /* get a curl handle */
    curl = curl_easy_init();
    if(curl)
      {
      if(verifyPeerOff)
        {
        cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                   "  Set CURLOPT_SSL_VERIFYPEER to off\n");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        }
      if(verifyHostOff)
        {
        cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                   "  Set CURLOPT_SSL_VERIFYHOST to off\n");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
        }

      // Using proxy
      if ( this->HTTPProxyType > 0 )
        {
        curl_easy_setopt(curl, CURLOPT_PROXY, this->HTTPProxy.c_str());
        switch (this->HTTPProxyType)
          {
        case 2:
          curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4);
          break;
        case 3:
          curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);
          break;
        default:
          curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
          if (!this->HTTPProxyAuth.empty())
            {
            curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD,
              this->HTTPProxyAuth.c_str());
            }
          }
        }
      if(this->CTest->ShouldUseHTTP10())
        {
        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
        }
      // enable HTTP ERROR parsing
      curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
      /* enable uploading */
      curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);

      // if there is little to no activity for too long stop submitting
      ::curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1);
      ::curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME,
        SUBMIT_TIMEOUT_IN_SECONDS_DEFAULT);

      /* HTTP PUT please */
      ::curl_easy_setopt(curl, CURLOPT_PUT, 1);
      ::curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

      std::string local_file = *file;
      if ( !cmSystemTools::FileExists(local_file.c_str()) )
        {
        local_file = localprefix + "/" + *file;
        }
      std::string remote_file
        = remoteprefix + cmSystemTools::GetFilenameName(*file);

      *this->LogFile << "\tUpload file: " << local_file << " to "
          << remote_file << std::endl;

      std::string ofile = "";
      for ( kk = 0; kk < remote_file.size(); kk ++ )
        {
        char c = remote_file[kk];
        char hexCh[4] = { 0, 0, 0, 0 };
        hexCh[0] = c;
        switch ( c )
          {
        case '+':
        case '?':
        case '/':
        case '\\':
        case '&':
        case ' ':
        case '=':
        case '%':
          sprintf(hexCh, "%%%02X", (int)c);
          ofile.append(hexCh);
          break;
        default:
          ofile.append(hexCh);
          }
        }
      std::string upload_as
        = url + ((url.find("?",0) == std::string::npos) ? "?" : "&")
        + "FileName=" + ofile;

      upload_as += "&MD5=";

      if(cmSystemTools::IsOn(this->GetOption("InternalTest")))
        {
        upload_as += "bad_md5sum";
        }
      else
        {
        char md5[33];
        cmSystemTools::ComputeFileMD5(local_file, md5);
        md5[32] = 0;
        upload_as += md5;
        }

      if( !cmSystemTools::FileExists(local_file.c_str()) )
        {
        cmCTestLog(this->CTest, ERROR_MESSAGE, "   Cannot find file: "
          << local_file << std::endl);
        ::curl_easy_cleanup(curl);
        ::curl_global_cleanup();
        return false;
        }
      unsigned long filelen = cmSystemTools::FileLength(local_file);

      ftpfile = cmsys::SystemTools::Fopen(local_file, "rb");
      cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "   Upload file: "
        << local_file << " to "
        << upload_as << " Size: " << filelen << std::endl);

      // specify target
      ::curl_easy_setopt(curl,CURLOPT_URL, upload_as.c_str());

      // now specify which file to upload
      ::curl_easy_setopt(curl, CURLOPT_INFILE, ftpfile);

      // and give the size of the upload (optional)
      ::curl_easy_setopt(curl, CURLOPT_INFILESIZE,
        static_cast<long>(filelen));

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
      ::curl_easy_setopt(curl, CURLOPT_FILE, (void *)&chunk);
      ::curl_easy_setopt(curl, CURLOPT_DEBUGDATA, (void *)&chunkDebug);

      // Now run off and do what you've been told!
      res = ::curl_easy_perform(curl);

      if(cmSystemTools::IsOn(this->GetOption("InternalTest")) &&
         cmSystemTools::VersionCompare(cmSystemTools::OP_LESS,
         this->CTest->GetCDashVersion().c_str(), "1.7"))
        {
        // mock failure output for internal test case
        std::string mock_output = "<cdash version=\"1.7.0\">\n"
          "  <status>ERROR</status>\n"
          "  <message>Checksum failed for file.</message>\n"
          "</cdash>\n";
        chunk.clear();
        chunk.assign(mock_output.begin(), mock_output.end());
        }

      if (!chunk.empty())
        {
        cmCTestLog(this->CTest, DEBUG, "CURL output: ["
          << cmCTestLogWrite(&*chunk.begin(), chunk.size()) << "]"
          << std::endl);
        this->ParseResponse(chunk);
        }
      if (!chunkDebug.empty())
        {
        cmCTestLog(this->CTest, DEBUG, "CURL debug output: ["
          << cmCTestLogWrite(&*chunkDebug.begin(), chunkDebug.size()) << "]"
          << std::endl);
        }

      // If curl failed for any reason, or checksum fails, wait and retry
      //
      if(res != CURLE_OK || this->HasErrors)
        {
        std::string retryDelay = this->GetOption("RetryDelay") == NULL ?
          "" : this->GetOption("RetryDelay");
        std::string retryCount = this->GetOption("RetryCount") == NULL ?
          "" : this->GetOption("RetryCount");

        int delay = retryDelay == "" ? atoi(this->CTest->GetCTestConfiguration(
          "CTestSubmitRetryDelay").c_str()) : atoi(retryDelay.c_str());
        int count = retryCount == "" ? atoi(this->CTest->GetCTestConfiguration(
          "CTestSubmitRetryCount").c_str()) : atoi(retryCount.c_str());

        for(int i = 0; i < count; i++)
          {
          cmCTestLog(this->CTest, HANDLER_OUTPUT,
            "   Submit failed, waiting " << delay << " seconds...\n");

          double stop = cmSystemTools::GetTime() + delay;
          while(cmSystemTools::GetTime() < stop)
            {
            cmSystemTools::Delay(100);
            }

          cmCTestLog(this->CTest, HANDLER_OUTPUT,
            "   Retry submission: Attempt " << (i + 1) << " of "
            << count << std::endl);

          ::fclose(ftpfile);
          ftpfile = cmsys::SystemTools::Fopen(local_file, "rb");
          ::curl_easy_setopt(curl, CURLOPT_INFILE, ftpfile);

          chunk.clear();
          chunkDebug.clear();
          this->HasErrors = false;

          res = ::curl_easy_perform(curl);

          if (!chunk.empty())
            {
            cmCTestLog(this->CTest, DEBUG, "CURL output: ["
              << cmCTestLogWrite(&*chunk.begin(), chunk.size()) << "]"
              << std::endl);
            this->ParseResponse(chunk);
            }

          if(res == CURLE_OK && !this->HasErrors)
            {
            break;
            }
          }
        }

      fclose(ftpfile);
      if ( res )
        {
        cmCTestLog(this->CTest, ERROR_MESSAGE,
          "   Error when uploading file: "
          << local_file << std::endl);
        cmCTestLog(this->CTest, ERROR_MESSAGE, "   Error message was: "
          << error_buffer << std::endl);
        *this->LogFile << "   Error when uploading file: "
                       << local_file
                       << std::endl
                       << "   Error message was: " << error_buffer
                       << std::endl;
        // avoid deref of begin for zero size array
        if(!chunk.empty())
          {
          *this->LogFile << "   Curl output was: "
                         << cmCTestLogWrite(&*chunk.begin(), chunk.size())
                         << std::endl;
          cmCTestLog(this->CTest, ERROR_MESSAGE, "CURL output: ["
                     << cmCTestLogWrite(&*chunk.begin(), chunk.size()) << "]"
                     << std::endl);
          }
        ::curl_easy_cleanup(curl);
        ::curl_global_cleanup();
        return false;
        }
      // always cleanup
      ::curl_easy_cleanup(curl);
      cmCTestLog(this->CTest, HANDLER_OUTPUT, "   Uploaded: " + local_file
        << std::endl);
      }
    }
  ::curl_global_cleanup();
  return true;
}

//----------------------------------------------------------------------------
void cmCTestSubmitHandler
::ParseResponse(cmCTestSubmitHandlerVectorOfChar chunk)
{
  std::string output = "";
  output.append(chunk.begin(), chunk.end());

  if(output.find("<cdash") != output.npos)
    {
    ResponseParser parser;
    parser.Parse(output.c_str());

    if(parser.Status != ResponseParser::STATUS_OK)
      {
      this->HasErrors = true;
      cmCTestLog(this->CTest, HANDLER_OUTPUT, "   Submission failed: " <<
        parser.Message << std::endl);
      return;
      }
    }
  output = cmSystemTools::UpperCase(output);
  if(output.find("WARNING") != std::string::npos)
    {
    this->HasWarnings = true;
    }
  if(output.find("ERROR") != std::string::npos)
    {
    this->HasErrors = true;
    }

  if(this->HasWarnings || this->HasErrors)
    {
    cmCTestLog(this->CTest, HANDLER_OUTPUT, "   Server Response:\n" <<
          cmCTestLogWrite(&*chunk.begin(), chunk.size()) << "\n");
    }
}

//----------------------------------------------------------------------------
bool cmCTestSubmitHandler::TriggerUsingHTTP(
  const std::set<std::string>& files,
  const std::string& remoteprefix,
  const std::string& url)
{
  CURL *curl;
  char error_buffer[1024];
  /* In windows, this will init the winsock stuff */
  ::curl_global_init(CURL_GLOBAL_ALL);

  cmCTest::SetOfStrings::const_iterator file;
  for ( file = files.begin(); file != files.end(); ++file )
    {
    /* get a curl handle */
    curl = curl_easy_init();
    if(curl)
      {
      // Using proxy
      if ( this->HTTPProxyType > 0 )
        {
        curl_easy_setopt(curl, CURLOPT_PROXY, this->HTTPProxy.c_str());
        switch (this->HTTPProxyType)
          {
        case 2:
          curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4);
          break;
        case 3:
          curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);
          break;
        default:
          curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
          if (!this->HTTPProxyAuth.empty())
            {
            curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD,
              this->HTTPProxyAuth.c_str());
            }
          }
        }

      ::curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

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
      ::curl_easy_setopt(curl, CURLOPT_FILE, (void *)&chunk);
      ::curl_easy_setopt(curl, CURLOPT_DEBUGDATA, (void *)&chunkDebug);

      std::string rfile
        = remoteprefix + cmSystemTools::GetFilenameName(*file);
      std::string ofile = "";
      std::string::iterator kk;
      for ( kk = rfile.begin(); kk < rfile.end(); ++ kk)
        {
        char c = *kk;
        char hexCh[4] = { 0, 0, 0, 0 };
        hexCh[0] = c;
        switch ( c )
          {
        case '+':
        case '?':
        case '/':
        case '\\':
        case '&':
        case ' ':
        case '=':
        case '%':
          sprintf(hexCh, "%%%02X", (int)c);
          ofile.append(hexCh);
          break;
        default:
          ofile.append(hexCh);
          }
        }
      std::string turl
        = url + ((url.find("?",0) == std::string::npos) ? "?" : "&")
        + "xmlfile=" + ofile;
      *this->LogFile << "Trigger url: " << turl << std::endl;
      cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "   Trigger url: "
        << turl << std::endl);
      curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
      curl_easy_setopt(curl, CURLOPT_URL, turl.c_str());
      if ( curl_easy_perform(curl) )
        {
        cmCTestLog(this->CTest, ERROR_MESSAGE, "   Error when triggering: "
          << turl << std::endl);
        cmCTestLog(this->CTest, ERROR_MESSAGE, "   Error message was: "
          << error_buffer << std::endl);
        *this->LogFile << "\tTriggering failed with error: " << error_buffer
                       << std::endl
                       << "   Error message was: " << error_buffer
                       << std::endl;
        if(!chunk.empty())
          {
          *this->LogFile
            << "   Curl output was: "
            << cmCTestLogWrite(&*chunk.begin(), chunk.size()) << std::endl;
          cmCTestLog(this->CTest, ERROR_MESSAGE, "CURL output: ["
                     << cmCTestLogWrite(&*chunk.begin(), chunk.size()) << "]"
                     << std::endl);
          }
        ::curl_easy_cleanup(curl);
        ::curl_global_cleanup();
        return false;
        }

      if (!chunk.empty())
        {
        cmCTestLog(this->CTest, DEBUG, "CURL output: ["
          << cmCTestLogWrite(&*chunk.begin(), chunk.size()) << "]"
          << std::endl);
        }
      if (!chunkDebug.empty())
        {
        cmCTestLog(this->CTest, DEBUG, "CURL debug output: ["
          << cmCTestLogWrite(&*chunkDebug.begin(), chunkDebug.size())
          << "]" << std::endl);
        }

      // always cleanup
      ::curl_easy_cleanup(curl);
      cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, std::endl);
      }
    }
  ::curl_global_cleanup();
  cmCTestLog(this->CTest, HANDLER_OUTPUT, "   Dart server triggered..."
    << std::endl);
  return true;
}

//----------------------------------------------------------------------------
bool cmCTestSubmitHandler::SubmitUsingSCP(
  const std::string& scp_command,
  const std::string& localprefix,
  const std::set<std::string>& files,
  const std::string& remoteprefix,
  const std::string& url)
{
  if ( scp_command.empty() || localprefix.empty() ||
    files.empty() || remoteprefix.empty() || url.empty() )
    {
    return 0;
    }
  std::vector<const char*> argv;
  argv.push_back(scp_command.c_str()); // Scp command
  argv.push_back(scp_command.c_str()); // Dummy string for file
  argv.push_back(scp_command.c_str()); // Dummy string for remote url
  argv.push_back(0);

  cmsysProcess* cp = cmsysProcess_New();
  cmsysProcess_SetOption(cp, cmsysProcess_Option_HideWindow, 1);
  //cmsysProcess_SetTimeout(cp, timeout);

  int problems = 0;

  cmCTest::SetOfStrings::const_iterator file;
  for ( file = files.begin(); file != files.end(); ++file )
    {
    int retVal;

    std::string lfname = localprefix;
    cmSystemTools::ConvertToUnixSlashes(lfname);
    lfname += "/" + *file;
    lfname = cmSystemTools::ConvertToOutputPath(lfname.c_str());
    argv[1] = lfname.c_str();
    std::string rfname = url + "/" + remoteprefix + *file;
    argv[2] = rfname.c_str();
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "Execute \"" << argv[0]
      << "\" \"" << argv[1] << "\" \""
      << argv[2] << "\"" << std::endl);
    *this->LogFile << "Execute \"" << argv[0] << "\" \"" << argv[1] << "\" \""
      << argv[2] << "\"" << std::endl;

    cmsysProcess_SetCommand(cp, &*argv.begin());
    cmsysProcess_Execute(cp);
    char* data;
    int length;

    while(cmsysProcess_WaitForData(cp, &data, &length, 0))
      {
      cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
        cmCTestLogWrite(data, length));
      }

    cmsysProcess_WaitForExit(cp, 0);

    int result = cmsysProcess_GetState(cp);

    if(result == cmsysProcess_State_Exited)
      {
      retVal = cmsysProcess_GetExitValue(cp);
      if ( retVal != 0 )
        {
        cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "\tSCP returned: "
          << retVal << std::endl);
        *this->LogFile << "\tSCP returned: " << retVal << std::endl;
        problems ++;
        }
      }
    else if(result == cmsysProcess_State_Exception)
      {
      retVal = cmsysProcess_GetExitException(cp);
      cmCTestLog(this->CTest, ERROR_MESSAGE, "\tThere was an exception: "
        << retVal << std::endl);
      *this->LogFile << "\tThere was an exception: " << retVal << std::endl;
      problems ++;
      }
    else if(result == cmsysProcess_State_Expired)
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE, "\tThere was a timeout"
        << std::endl);
      *this->LogFile << "\tThere was a timeout" << std::endl;
      problems ++;
      }
    else if(result == cmsysProcess_State_Error)
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE, "\tError executing SCP: "
        << cmsysProcess_GetErrorString(cp) << std::endl);
      *this->LogFile << "\tError executing SCP: "
        << cmsysProcess_GetErrorString(cp) << std::endl;
      problems ++;
      }
    }
  cmsysProcess_Delete(cp);
  if ( problems )
    {
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool cmCTestSubmitHandler::SubmitUsingCP(
  const std::string& localprefix,
  const std::set<std::string>& files,
  const std::string& remoteprefix,
  const std::string& destination)
{
  if ( localprefix.empty() ||
    files.empty() || remoteprefix.empty() || destination.empty() )
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Missing arguments for submit via cp:\n"
               << "\tlocalprefix: " << localprefix << "\n"
               << "\tNumber of files: " << files.size() << "\n"
               << "\tremoteprefix: " << remoteprefix << "\n"
               << "\tdestination: " << destination << std::endl);
    return 0;
    }
  cmCTest::SetOfStrings::const_iterator file;
  bool problems = false;
  for ( file = files.begin(); file != files.end(); ++file )
    {
    std::string lfname = localprefix;
    cmSystemTools::ConvertToUnixSlashes(lfname);
    lfname += "/" + *file;
    std::string rfname = destination + "/" + remoteprefix + *file;
    cmSystemTools::CopyFileAlways(lfname, rfname);
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "   Copy file: "
        << lfname << " to "
        << rfname << std::endl);
    }
  std::string tagDoneFile = destination + "/" + remoteprefix + "DONE";
  cmSystemTools::Touch(tagDoneFile, true);
  if ( problems )
    {
    return false;
    }
  return true;
}


//----------------------------------------------------------------------------
#if defined(CTEST_USE_XMLRPC)
bool cmCTestSubmitHandler::SubmitUsingXMLRPC(const std::string& localprefix,
  const std::set<std::string>& files,
  const std::string& remoteprefix,
  const std::string& url)
{
  xmlrpc_env env;
  char ctestString[] = "CTest";
  std::string ctestVersionString = cmVersion::GetCMakeVersion();
  char* ctestVersion = const_cast<char*>(ctestVersionString.c_str());

  std::string realURL = url + "/" + remoteprefix + "/Command/";

  /* Start up our XML-RPC client library. */
  xmlrpc_client_init(XMLRPC_CLIENT_NO_FLAGS, ctestString, ctestVersion);

  /* Initialize our error-handling environment. */
  xmlrpc_env_init(&env);

  /* Call the famous server at UserLand. */
  cmCTestLog(this->CTest, HANDLER_OUTPUT, "   Submitting to: "
    << realURL.c_str() << " (" << remoteprefix.c_str() << ")" << std::endl);
  cmCTest::SetOfStrings::const_iterator file;
  for ( file = files.begin(); file != files.end(); ++file )
    {
    xmlrpc_value *result;

    std::string local_file = *file;
    if ( !cmSystemTools::FileExists(local_file.c_str()) )
      {
      local_file = localprefix + "/" + *file;
      }
    cmCTestLog(this->CTest, HANDLER_OUTPUT, "   Submit file: "
      << local_file.c_str() << std::endl);
    struct stat st;
    if ( ::stat(local_file.c_str(), &st) )
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE, "  Cannot find file: "
        << local_file.c_str() << std::endl);
      return false;
      }

    // off_t can be bigger than size_t.  fread takes size_t.
    // make sure the file is not too big.
    if(static_cast<off_t>(static_cast<size_t>(st.st_size)) !=
       static_cast<off_t>(st.st_size))
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE, "  File too big: "
        << local_file.c_str() << std::endl);
      return false;
      }
    size_t fileSize = static_cast<size_t>(st.st_size);
    FILE* fp = cmsys::SystemTools::Fopen(local_file.c_str(), "rb");
    if ( !fp )
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE, "  Cannot open file: "
        << local_file.c_str() << std::endl);
      return false;
      }

    unsigned char *fileBuffer = new unsigned char[fileSize];
    if ( fread(fileBuffer, 1, fileSize, fp) != fileSize )
      {
      delete [] fileBuffer;
      fclose(fp);
      cmCTestLog(this->CTest, ERROR_MESSAGE, "  Cannot read file: "
        << local_file.c_str() << std::endl);
      return false;
      }
    fclose(fp);

    char remoteCommand[] = "Submit.put";
    char* pRealURL = const_cast<char*>(realURL.c_str());
    result = xmlrpc_client_call(&env, pRealURL, remoteCommand,
      "(6)", fileBuffer, (xmlrpc_int32)fileSize );

    delete [] fileBuffer;

    if ( env.fault_occurred )
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE, " Submission problem: "
        << env.fault_string << " (" << env.fault_code << ")" << std::endl);
      xmlrpc_env_clean(&env);
      xmlrpc_client_cleanup();
      return false;
      }

    /* Dispose of our result value. */
    xmlrpc_DECREF(result);
    }

  /* Clean up our error-handling environment. */
  xmlrpc_env_clean(&env);

  /* Shutdown our XML-RPC client library. */
  xmlrpc_client_cleanup();
  return true;
}
#else
bool cmCTestSubmitHandler::SubmitUsingXMLRPC(std::string const&,
                                             std::set<std::string> const&,
                                             std::string const&,
                                             std::string const&)
{
  return false;
}
#endif

void cmCTestSubmitHandler::ConstructCDashURL(std::string& dropMethod,
                                             std::string& url)
{
  dropMethod = this->CTest->GetCTestConfiguration("DropMethod");
  url = dropMethod;
  url += "://";
  if ( this->CTest->GetCTestConfiguration("DropSiteUser").size() > 0 )
    {
    url += this->CTest->GetCTestConfiguration("DropSiteUser");
    cmCTestLog(this->CTest, HANDLER_OUTPUT,
               this->CTest->GetCTestConfiguration("DropSiteUser").c_str());
    if ( this->CTest->GetCTestConfiguration("DropSitePassword").size() > 0 )
      {
      url += ":" + this->CTest->GetCTestConfiguration("DropSitePassword");
      cmCTestLog(this->CTest, HANDLER_OUTPUT, ":******");
      }
    url += "@";
    }
  url += this->CTest->GetCTestConfiguration("DropSite") +
    this->CTest->GetCTestConfiguration("DropLocation");
}


int cmCTestSubmitHandler::HandleCDashUploadFile(std::string const& file,
                                                std::string const& typeString)
{
  if (file.empty())
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Upload file not specified\n");
    return -1;
    }
  if (!cmSystemTools::FileExists(file))
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Upload file not found: '" << file << "'\n");
    return -1;
    }
  cmCTestCurl curl(this->CTest);
  std::string curlopt(this->CTest->GetCTestConfiguration("CurlOptions"));
  std::vector<std::string> args;
  cmSystemTools::ExpandListArgument(curlopt, args);
  curl.SetCurlOptions(args);
  curl.SetTimeOutSeconds(SUBMIT_TIMEOUT_IN_SECONDS_DEFAULT);
  std::string dropMethod;
  std::string url;
  this->ConstructCDashURL(dropMethod, url);
  std::string::size_type pos = url.find("submit.php?");
  url = url.substr(0, pos+10);
  if ( ! (dropMethod == "http" || dropMethod == "https" ) )
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Only http and https are supported for CDASH_UPLOAD\n");
    return -1;
    }
  char md5sum[33];
  md5sum[32] = 0;
  cmSystemTools::ComputeFileMD5(file, md5sum);
  // 1. request the buildid and check to see if the file
  //    has already been uploaded
  // TODO I added support for subproject. You would need to add
  // a "&subproject=subprojectname" to the first POST.
  cmCTestScriptHandler* ch =
    static_cast<cmCTestScriptHandler*>(this->CTest->GetHandler("script"));
  cmake* cm =  ch->GetCMake();
  const char* subproject = cm->GetProperty("SubProject", cmProperty::GLOBAL);
  // TODO: Encode values for a URL instead of trusting caller.
  std::ostringstream str;
  str << "project="
      << this->CTest->GetCTestConfiguration("ProjectName") << "&";
  if(subproject)
    {
    str << "subproject=" << subproject << "&";
    }
  str << "stamp=" << this->CTest->GetCurrentTag() << "-"
      << this->CTest->GetTestModelString() << "&"
      << "model=" << this->CTest->GetTestModelString() << "&"
      << "build=" << this->CTest->GetCTestConfiguration("BuildName") << "&"
      << "site=" << this->CTest->GetCTestConfiguration("Site") << "&"
      << "track=" << this->CTest->GetTestModelString() << "&"
      << "starttime=" << (int)cmSystemTools::GetTime() << "&"
      << "endtime=" << (int)cmSystemTools::GetTime() << "&"
      << "datafilesmd5[0]=" << md5sum << "&"
      << "type=" << typeString;
  std::string fields = str.str();
  cmCTestLog(this->CTest, DEBUG, "fields: " << fields << "\nurl:"
             << url << "\nfile: " << file << "\n");
  std::string response;
  if(!curl.HttpRequest(url, fields, response))
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Error in HttpRequest\n" << response);
    return -1;
    }
  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
             "Request upload response: [" << response << "]\n");
  Json::Value json;
  Json::Reader reader;
  if(!reader.parse(response, json))
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "error parsing json string [" << response << "]\n"
               << reader.getFormattedErrorMessages() << "\n");
    return -1;
    }
  if(json["status"].asInt() != 0)
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Bad status returned from CDash: "
               << json["status"].asInt());
    return -1;
    }
  if(json["datafilesmd5"].isArray())
    {
    int datares = json["datafilesmd5"][0].asInt();
    if(datares == 1)
      {
      cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                 "File already exists on CDash, skip upload "
                 << file << "\n");
      return 0;
      }
    }
  else
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "bad datafilesmd5 value in response "
               << response << "\n");
    return -1;
    }

  std::string upload_as = cmSystemTools::GetFilenameName(file);
  std::ostringstream fstr;
  fstr << "type=" << typeString << "&"
       << "md5=" << md5sum << "&"
       << "filename=" << upload_as << "&"
       << "buildid=" << json["buildid"].asString();
  if(!curl.UploadFile(file, url, fstr.str(), response))
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "error uploading to CDash. "
               << file << " " << url << " " << fstr.str());
    return -1;
    }
  if(!reader.parse(response, json))
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "error parsing json string [" << response << "]\n"
               << reader.getFormattedErrorMessages() << "\n");
    return -1;
    }
  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
             "Upload file response: [" << response << "]\n");
  return 0;
}

//----------------------------------------------------------------------------
int cmCTestSubmitHandler::ProcessHandler()
{
  const char* cdashUploadFile = this->GetOption("CDashUploadFile");
  const char* cdashUploadType = this->GetOption("CDashUploadType");
  if(cdashUploadFile && cdashUploadType)
    {
    return this->HandleCDashUploadFile(cdashUploadFile, cdashUploadType);
    }
  std::string iscdash = this->CTest->GetCTestConfiguration("IsCDash");
  // cdash does not need to trigger so just return true
  if(!iscdash.empty())
    {
    this->CDash = true;
    }

  const std::string &buildDirectory
    = this->CTest->GetCTestConfiguration("BuildDirectory");
  if (buildDirectory.empty())
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
      "Cannot find BuildDirectory  key in the DartConfiguration.tcl"
      << std::endl);
    return -1;
    }

  if ( getenv("HTTP_PROXY") )
    {
    this->HTTPProxyType = 1;
    this->HTTPProxy = getenv("HTTP_PROXY");
    if ( getenv("HTTP_PROXY_PORT") )
      {
      this->HTTPProxy += ":";
      this->HTTPProxy += getenv("HTTP_PROXY_PORT");
      }
    if ( getenv("HTTP_PROXY_TYPE") )
      {
      std::string type = getenv("HTTP_PROXY_TYPE");
      // HTTP/SOCKS4/SOCKS5
      if ( type == "HTTP" )
        {
        this->HTTPProxyType = 1;
        }
      else if ( type == "SOCKS4" )
        {
        this->HTTPProxyType = 2;
        }
      else if ( type == "SOCKS5" )
        {
        this->HTTPProxyType = 3;
        }
      }
    if ( getenv("HTTP_PROXY_USER") )
      {
      this->HTTPProxyAuth = getenv("HTTP_PROXY_USER");
      }
    if ( getenv("HTTP_PROXY_PASSWD") )
      {
      this->HTTPProxyAuth += ":";
      this->HTTPProxyAuth += getenv("HTTP_PROXY_PASSWD");
      }
    }

  if ( getenv("FTP_PROXY") )
    {
    this->FTPProxyType = 1;
    this->FTPProxy = getenv("FTP_PROXY");
    if ( getenv("FTP_PROXY_PORT") )
      {
      this->FTPProxy += ":";
      this->FTPProxy += getenv("FTP_PROXY_PORT");
      }
    if ( getenv("FTP_PROXY_TYPE") )
      {
      std::string type = getenv("FTP_PROXY_TYPE");
      // HTTP/SOCKS4/SOCKS5
      if ( type == "HTTP" )
        {
        this->FTPProxyType = 1;
        }
      else if ( type == "SOCKS4" )
        {
        this->FTPProxyType = 2;
        }
      else if ( type == "SOCKS5" )
        {
        this->FTPProxyType = 3;
        }
      }
    }

  if (!this->HTTPProxy.empty())
    {
    cmCTestLog(this->CTest, HANDLER_OUTPUT, "   Use HTTP Proxy: "
      << this->HTTPProxy << std::endl);
    }
  if (!this->FTPProxy.empty())
    {
    cmCTestLog(this->CTest, HANDLER_OUTPUT, "   Use FTP Proxy: "
      << this->FTPProxy << std::endl);
    }
  cmGeneratedFileStream ofs;
  this->StartLogFile("Submit", ofs);

  cmCTest::SetOfStrings files;
  std::string prefix = this->GetSubmitResultsPrefix();

  if (!this->Files.empty())
    {
    // Submit the explicitly selected files:
    //
    files.insert(this->Files.begin(), this->Files.end());
    }

  // Add to the list of files to submit from any selected, existing parts:
  //

  // TODO:
  // Check if test is enabled

  this->CTest->AddIfExists(cmCTest::PartUpdate, "Update.xml");
  this->CTest->AddIfExists(cmCTest::PartConfigure, "Configure.xml");
  this->CTest->AddIfExists(cmCTest::PartBuild, "Build.xml");
  this->CTest->AddIfExists(cmCTest::PartTest, "Test.xml");
  if(this->CTest->AddIfExists(cmCTest::PartCoverage, "Coverage.xml"))
    {
    std::vector<std::string> gfiles;
    std::string gpath
      = buildDirectory + "/Testing/" + this->CTest->GetCurrentTag();
    std::string::size_type glen = gpath.size() + 1;
    gpath = gpath + "/CoverageLog*";
    cmCTestLog(this->CTest, DEBUG, "Globbing for: " << gpath
      << std::endl);
    if ( cmSystemTools::SimpleGlob(gpath, gfiles, 1) )
      {
      size_t cc;
      for ( cc = 0; cc < gfiles.size(); cc ++ )
        {
        gfiles[cc] = gfiles[cc].substr(glen);
        cmCTestLog(this->CTest, DEBUG, "Glob file: " << gfiles[cc]
          << std::endl);
        this->CTest->AddSubmitFile(cmCTest::PartCoverage, gfiles[cc].c_str());
        }
      }
    else
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE, "Problem globbing" << std::endl);
      }
    }
  this->CTest->AddIfExists(cmCTest::PartMemCheck, "DynamicAnalysis.xml");
  this->CTest->AddIfExists(cmCTest::PartMemCheck, "Purify.xml");
  this->CTest->AddIfExists(cmCTest::PartNotes, "Notes.xml");
  this->CTest->AddIfExists(cmCTest::PartUpload, "Upload.xml");

  // Query parts for files to submit.
  for(cmCTest::Part p = cmCTest::PartStart;
      p != cmCTest::PartCount; p = cmCTest::Part(p+1))
    {
    // Skip parts we are not submitting.
    if(!this->SubmitPart[p])
      {
      continue;
      }

    // Submit files from this part.
    std::vector<std::string> const& pfiles = this->CTest->GetSubmitFiles(p);
    files.insert(pfiles.begin(), pfiles.end());
    }

  if ( ofs )
    {
    ofs << "Upload files:" << std::endl;
    int cnt = 0;
    cmCTest::SetOfStrings::iterator it;
    for ( it = files.begin(); it != files.end(); ++ it )
      {
      ofs << cnt << "\t" << *it << std::endl;
      cnt ++;
      }
    }
  cmCTestLog(this->CTest, HANDLER_OUTPUT, "Submit files (using "
    << this->CTest->GetCTestConfiguration("DropMethod") << ")"
    << std::endl);
  const char* specificTrack = this->CTest->GetSpecificTrack();
  if ( specificTrack )
    {
    cmCTestLog(this->CTest, HANDLER_OUTPUT, "   Send to track: "
      << specificTrack << std::endl);
    }
  this->SetLogFile(&ofs);

  std::string dropMethod(this->CTest->GetCTestConfiguration("DropMethod"));

  if ( dropMethod == "" || dropMethod == "ftp" )
    {
    ofs << "Using drop method: FTP" << std::endl;
    cmCTestLog(this->CTest, HANDLER_OUTPUT, "   Using FTP submit method"
      << std::endl
      << "   Drop site: ftp://");
    std::string url = "ftp://";
    url += cmCTest::MakeURLSafe(
      this->CTest->GetCTestConfiguration("DropSiteUser")) + ":" +
      cmCTest::MakeURLSafe(this->CTest->GetCTestConfiguration(
          "DropSitePassword")) + "@" +
      this->CTest->GetCTestConfiguration("DropSite") +
      cmCTest::MakeURLSafe(
        this->CTest->GetCTestConfiguration("DropLocation"));
    if (!this->CTest->GetCTestConfiguration("DropSiteUser").empty())
      {
      cmCTestLog(this->CTest, HANDLER_OUTPUT,
        this->CTest->GetCTestConfiguration(
          "DropSiteUser").c_str());
      if (!this->CTest->GetCTestConfiguration("DropSitePassword").empty())
        {
        cmCTestLog(this->CTest, HANDLER_OUTPUT, ":******");
        }
      cmCTestLog(this->CTest, HANDLER_OUTPUT, "@");
      }
    cmCTestLog(this->CTest, HANDLER_OUTPUT,
      this->CTest->GetCTestConfiguration("DropSite")
      << this->CTest->GetCTestConfiguration("DropLocation") << std::endl);
    if ( !this->SubmitUsingFTP(buildDirectory + "/Testing/"
        + this->CTest->GetCurrentTag(),
        files, prefix, url) )
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE,
        "   Problems when submitting via FTP"
        << std::endl);
      ofs << "   Problems when submitting via FTP" << std::endl;
      return -1;
      }
    if(!this->CDash)
      {
      cmCTestLog(this->CTest, HANDLER_OUTPUT, "   Using HTTP trigger method"
                 << std::endl
                 << "   Trigger site: "
                 << this->CTest->GetCTestConfiguration("TriggerSite")
                 << std::endl);
      if ( !this->
           TriggerUsingHTTP(files, prefix,
                            this->CTest->GetCTestConfiguration("TriggerSite")))
        {
        cmCTestLog(this->CTest, ERROR_MESSAGE,
                   "   Problems when triggering via HTTP" << std::endl);
        ofs << "   Problems when triggering via HTTP" << std::endl;
        return -1;
        }
      cmCTestLog(this->CTest, HANDLER_OUTPUT, "   Submission successful"
                 << std::endl);
      ofs << "   Submission successful" << std::endl;
      return 0;
      }
    }
  else if ( dropMethod == "http" || dropMethod == "https" )
    {
    std::string url = dropMethod;
    url += "://";
    ofs << "Using drop method: " << dropMethod << std::endl;
    cmCTestLog(this->CTest, HANDLER_OUTPUT, "   Using HTTP submit method"
      << std::endl
      << "   Drop site:" << url);
     if (!this->CTest->GetCTestConfiguration("DropSiteUser").empty())
      {
      url += this->CTest->GetCTestConfiguration("DropSiteUser");
      cmCTestLog(this->CTest, HANDLER_OUTPUT,
        this->CTest->GetCTestConfiguration("DropSiteUser").c_str());
      if (!this->CTest->GetCTestConfiguration("DropSitePassword").empty())
        {
        url += ":" + this->CTest->GetCTestConfiguration("DropSitePassword");
        cmCTestLog(this->CTest, HANDLER_OUTPUT, ":******");
        }
      url += "@";
      cmCTestLog(this->CTest, HANDLER_OUTPUT, "@");
      }
    url += this->CTest->GetCTestConfiguration("DropSite") +
      this->CTest->GetCTestConfiguration("DropLocation");
    cmCTestLog(this->CTest, HANDLER_OUTPUT,
      this->CTest->GetCTestConfiguration("DropSite")
      << this->CTest->GetCTestConfiguration("DropLocation") << std::endl);
    if ( !this->SubmitUsingHTTP(buildDirectory + "/Testing/" +
        this->CTest->GetCurrentTag(), files, prefix, url) )
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE,
        "   Problems when submitting via HTTP" << std::endl);
      ofs << "   Problems when submitting via HTTP" << std::endl;
      return -1;
      }
    if(!this->CDash)
      {
      cmCTestLog(this->CTest, HANDLER_OUTPUT, "   Using HTTP trigger method"
                 << std::endl
                 << "   Trigger site: "
                 << this->CTest->GetCTestConfiguration("TriggerSite")
                 << std::endl);
      if ( !this->
           TriggerUsingHTTP(files, prefix,
                            this->CTest->GetCTestConfiguration("TriggerSite")))
        {
        cmCTestLog(this->CTest, ERROR_MESSAGE,
                   "   Problems when triggering via HTTP" << std::endl);
        ofs << "   Problems when triggering via HTTP" << std::endl;
        return -1;
        }
      }
    if(this->HasErrors)
      {
      cmCTestLog(this->CTest, HANDLER_OUTPUT, "   Errors occurred during "
        "submission." << std::endl);
      ofs << "   Errors occurred during submission. " << std::endl;
      }
    else
      {
      cmCTestLog(this->CTest, HANDLER_OUTPUT, "   Submission successful" <<
        (this->HasWarnings ? ", with warnings." : "") << std::endl);
      ofs << "   Submission successful" <<
        (this->HasWarnings ? ", with warnings." : "") << std::endl;
      }

    return 0;
    }
  else if ( dropMethod == "xmlrpc" )
    {
#if defined(CTEST_USE_XMLRPC)
    ofs << "Using drop method: XML-RPC" << std::endl;
    cmCTestLog(this->CTest, HANDLER_OUTPUT, "   Using XML-RPC submit method"
      << std::endl);
    std::string url = this->CTest->GetCTestConfiguration("DropSite");
    prefix = this->CTest->GetCTestConfiguration("DropLocation");
    if ( !this->SubmitUsingXMLRPC(buildDirectory + "/Testing/" +
        this->CTest->GetCurrentTag(), files, prefix, url) )
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE,
        "   Problems when submitting via XML-RPC" << std::endl);
      ofs << "   Problems when submitting via XML-RPC" << std::endl;
      return -1;
      }
    cmCTestLog(this->CTest, HANDLER_OUTPUT, "   Submission successful"
      << std::endl);
    ofs << "   Submission successful" << std::endl;
    return 0;
#else
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "   Submission method \"xmlrpc\" not compiled into CTest!"
               << std::endl);
    return -1;
#endif
    }
  else if ( dropMethod == "scp" )
    {
    std::string url;
    std::string oldWorkingDirectory;
    if (!this->CTest->GetCTestConfiguration("DropSiteUser").empty())
      {
      url += this->CTest->GetCTestConfiguration("DropSiteUser") + "@";
      }
    url += this->CTest->GetCTestConfiguration("DropSite") + ":" +
      this->CTest->GetCTestConfiguration("DropLocation");

    // change to the build directory so that we can uses a relative path
    // on windows since scp dosn't support "c:" a drive in the path
    oldWorkingDirectory = cmSystemTools::GetCurrentWorkingDirectory();
    cmSystemTools::ChangeDirectory(buildDirectory);

    if ( !this->SubmitUsingSCP(
        this->CTest->GetCTestConfiguration("ScpCommand"),
        "Testing/"+this->CTest->GetCurrentTag(), files, prefix, url) )
      {
      cmSystemTools::ChangeDirectory(oldWorkingDirectory);
      cmCTestLog(this->CTest, ERROR_MESSAGE,
        "   Problems when submitting via SCP"
        << std::endl);
      ofs << "   Problems when submitting via SCP" << std::endl;
      return -1;
      }
    cmSystemTools::ChangeDirectory(oldWorkingDirectory);
    cmCTestLog(this->CTest, HANDLER_OUTPUT, "   Submission successful"
      << std::endl);
    ofs << "   Submission successful" << std::endl;
    return 0;
    }
  else if ( dropMethod == "cp" )
    {
    std::string location
      = this->CTest->GetCTestConfiguration("DropLocation");


    // change to the build directory so that we can uses a relative path
    // on windows since scp dosn't support "c:" a drive in the path
    std::string
      oldWorkingDirectory = cmSystemTools::GetCurrentWorkingDirectory();
    cmSystemTools::ChangeDirectory(buildDirectory);
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "   Change directory: "
               << buildDirectory << std::endl);

    if ( !this->SubmitUsingCP(
           "Testing/"+this->CTest->GetCurrentTag(),
           files,
           prefix,
           location) )
      {
      cmSystemTools::ChangeDirectory(oldWorkingDirectory);
      cmCTestLog(this->CTest, ERROR_MESSAGE,
        "   Problems when submitting via CP"
        << std::endl);
      ofs << "   Problems when submitting via cp" << std::endl;
      return -1;
      }
    cmSystemTools::ChangeDirectory(oldWorkingDirectory);
    cmCTestLog(this->CTest, HANDLER_OUTPUT, "   Submission successful"
      << std::endl);
    ofs << "   Submission successful" << std::endl;
    return 0;
    }

  cmCTestLog(this->CTest, ERROR_MESSAGE, "   Unknown submission method: \""
    << dropMethod << "\"" << std::endl);
  return -1;
}

//----------------------------------------------------------------------------
std::string cmCTestSubmitHandler::GetSubmitResultsPrefix()
{
  std::string buildname = cmCTest::SafeBuildIdField(
    this->CTest->GetCTestConfiguration("BuildName"));
  std::string name = this->CTest->GetCTestConfiguration("Site") +
    "___" + buildname +
    "___" + this->CTest->GetCurrentTag() + "-" +
    this->CTest->GetTestModelString() + "___XML___";
  return name;
}

//----------------------------------------------------------------------------
void cmCTestSubmitHandler::SelectParts(std::set<cmCTest::Part> const& parts)
{
  // Check whether each part is selected.
  for(cmCTest::Part p = cmCTest::PartStart;
      p != cmCTest::PartCount; p = cmCTest::Part(p+1))
    {
    this->SubmitPart[p] =
      (std::set<cmCTest::Part>::const_iterator(parts.find(p)) != parts.end());
    }
}

//----------------------------------------------------------------------------
void cmCTestSubmitHandler::SelectFiles(cmCTest::SetOfStrings const& files)
{
  this->Files.insert(files.begin(), files.end());
}
