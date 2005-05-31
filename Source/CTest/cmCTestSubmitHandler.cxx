/*=========================================================================

Program:   CMake - Cross-Platform Makefile Generator
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmCTestSubmitHandler.h"

#include "cmSystemTools.h"
#include "cmVersion.h"
#include "cmGeneratedFileStream.h"
#include "cmCTest.h"

#include <cmsys/Process.h>
#include <cmsys/Base64.h>

// For XML-RPC submission
#include "xmlrpc.h"
#include "xmlrpc_client.h"

// For curl submission
#include "CTest/Curl/curl/curl.h"

#include <sys/stat.h>

typedef std::vector<char> cmCTestSubmitHandlerVectorOfChar;

static size_t
cmCTestSubmitHandlerWriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
  register int realsize = size * nmemb;

  cmCTestSubmitHandlerVectorOfChar *vec = static_cast<cmCTestSubmitHandlerVectorOfChar*>(data);
  const char* chPtr = static_cast<char*>(ptr);
  vec->insert(vec->end(), chPtr, chPtr + realsize);

  return realsize;
}

//----------------------------------------------------------------------------
cmCTestSubmitHandler::cmCTestSubmitHandler() : m_HTTPProxy(), m_FTPProxy()
{
  m_HTTPProxy = "";
  m_HTTPProxyType = 0;
  m_HTTPProxyAuth = "";
  if ( getenv("HTTP_PROXY") )
    {
    m_HTTPProxyType = 1;
    m_HTTPProxy = getenv("HTTP_PROXY");
    if ( getenv("HTTP_PROXY_PORT") )
      {
      m_HTTPProxy += ":";
      m_HTTPProxy += getenv("HTTP_PROXY_PORT");
      }
    if ( getenv("HTTP_PROXY_TYPE") )
      {
      cmStdString type = getenv("HTTP_PROXY_TYPE");
      // HTTP/SOCKS4/SOCKS5
      if ( type == "HTTP" )
        {
        m_HTTPProxyType = 1;
        }
      else if ( type == "SOCKS4" )
        {
        m_HTTPProxyType = 2;
        }
      else if ( type == "SOCKS5" )
        {
        m_HTTPProxyType = 3;
        }
      }
    if ( getenv("HTTP_PROXY_USER") )
      {
      m_HTTPProxyAuth = getenv("HTTP_PROXY_USER");
      }
    if ( getenv("HTTP_PROXY_PASSWD") )
      {
      m_HTTPProxyAuth += ":";
      m_HTTPProxyAuth += getenv("HTTP_PROXY_PASSWD");
      }
    }
  m_FTPProxy = "";
  m_FTPProxyType = 0;
  if ( getenv("FTP_PROXY") )
    {
    m_FTPProxyType = 1;
    m_FTPProxy = getenv("FTP_PROXY");
    if ( getenv("FTP_PROXY_PORT") )
      {
      m_FTPProxy += ":";
      m_FTPProxy += getenv("FTP_PROXY_PORT");
      }
    if ( getenv("FTP_PROXY_TYPE") )
      {
      cmStdString type = getenv("FTP_PROXY_TYPE");
      // HTTP/SOCKS4/SOCKS5
      if ( type == "HTTP" )
        {
        m_FTPProxyType = 1;
        }
      else if ( type == "SOCKS4" )
        {
        m_FTPProxyType = 2;
        }
      else if ( type == "SOCKS5" )
        {
        m_FTPProxyType = 3;
        }
      }
    }
  if ( m_HTTPProxy.size() > 0 )
    {
    cmCTestLog(m_CTest, HANDLER_OUTPUT, "  Use HTTP Proxy: " << m_HTTPProxy << std::endl);
    }
  if ( m_FTPProxy.size() > 0 )
    {
    cmCTestLog(m_CTest, HANDLER_OUTPUT, "  Use FTP Proxy: " << m_FTPProxy << std::endl);
    }
}

//----------------------------------------------------------------------------
bool cmCTestSubmitHandler::SubmitUsingFTP(const cmStdString& localprefix, 
  const std::vector<cmStdString>& files,
  const cmStdString& remoteprefix, 
  const cmStdString& url)
{
  CURL *curl;
  CURLcode res;
  FILE* ftpfile;
  char error_buffer[1024];

  /* In windows, this will init the winsock stuff */
  ::curl_global_init(CURL_GLOBAL_ALL);

  cmStdString::size_type cc;
  for ( cc = 0; cc < files.size(); cc ++ )
    {
    /* get a curl handle */
    curl = curl_easy_init();
    if(curl) 
      {
      // Using proxy
      if ( m_FTPProxyType > 0 )
        {
        curl_easy_setopt(curl, CURLOPT_PROXY, m_FTPProxy.c_str()); 
        switch (m_FTPProxyType)
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
      ::curl_easy_setopt(curl, CURLOPT_UPLOAD, 1) ;

      cmStdString local_file = localprefix + "/" + files[cc];
      cmStdString upload_as = url + "/" + remoteprefix + files[cc];

      struct stat st;
      if ( ::stat(local_file.c_str(), &st) )
        {
        return false;
        }

      ftpfile = ::fopen(local_file.c_str(), "rb");
      *m_LogFile << "\tUpload file: " << local_file.c_str() << " to "
          << upload_as.c_str() << std::endl;
      cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "  Upload file: " << local_file.c_str() << " to " 
        << upload_as.c_str() << std::endl);

      ::curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
      if ( m_HandlerVerbose )
        {
        ::curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        }
      // specify target
      ::curl_easy_setopt(curl,CURLOPT_URL, upload_as.c_str());

      // now specify which file to upload
      ::curl_easy_setopt(curl, CURLOPT_INFILE, ftpfile);

      // and give the size of the upload (optional)
      ::curl_easy_setopt(curl, CURLOPT_INFILESIZE, static_cast<long>(st.st_size));

      // and give curl the buffer for errors
      ::curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, &error_buffer);

      // specify handler for output
      ::curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cmCTestSubmitHandlerWriteMemoryCallback);

      /* we pass our 'chunk' struct to the callback function */
      cmCTestSubmitHandlerVectorOfChar chunk;
      ::curl_easy_setopt(curl, CURLOPT_FILE, (void *)&chunk);

      // Now run off and do what you've been told!
      res = ::curl_easy_perform(curl);

      cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "CURL output: ["
        << cmCTestLogWrite(&*chunk.begin(), chunk.size()) << "]" << std::endl);

      fclose(ftpfile);
      if ( res )
        {
        cmCTestLog(m_CTest, ERROR, "  Error when uploading file: " << local_file.c_str() << std::endl);
        cmCTestLog(m_CTest, ERROR, "  Error message was: " << error_buffer << std::endl);
        *m_LogFile << "  Error when uploading file: " << local_file.c_str() << std::endl
          << "  Error message was: " << error_buffer << std::endl
          << "  Curl output was: " << cmCTestLogWrite(&*chunk.begin(), chunk.size()) << std::endl;
        cmCTestLog(m_CTest, ERROR, "CURL output: ["
          << cmCTestLogWrite(&*chunk.begin(), chunk.size()) << "]" << std::endl);
        ::curl_easy_cleanup(curl);
        ::curl_global_cleanup(); 
        return false;
        }
      // always cleanup
      ::curl_easy_cleanup(curl);
      cmCTestLog(m_CTest, HANDLER_OUTPUT, "  Uploaded: " + local_file << std::endl);
      }
    }
  ::curl_global_cleanup(); 
  return true;
}

//----------------------------------------------------------------------------
// Uploading files is simpler
bool cmCTestSubmitHandler::SubmitUsingHTTP(const cmStdString& localprefix, 
  const std::vector<cmStdString>& files,
  const cmStdString& remoteprefix, 
  const cmStdString& url)
{
  CURL *curl;
  CURLcode res;
  FILE* ftpfile;
  char error_buffer[1024];

  /* In windows, this will init the winsock stuff */
  ::curl_global_init(CURL_GLOBAL_ALL);

  cmStdString::size_type cc, kk;
  for ( cc = 0; cc < files.size(); cc ++ )
    {
    /* get a curl handle */
    curl = curl_easy_init();
    if(curl) 
      {

      // Using proxy
      if ( m_HTTPProxyType > 0 )
        {
        curl_easy_setopt(curl, CURLOPT_PROXY, m_HTTPProxy.c_str()); 
        switch (m_HTTPProxyType)
          {
        case 2:
          curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4);
          break;
        case 3:
          curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);
          break;
        default:
          curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
          if (m_HTTPProxyAuth.size() > 0)
            {
            curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD,
              m_HTTPProxyAuth.c_str());
            }
          }
        }

      /* enable uploading */
      curl_easy_setopt(curl, CURLOPT_UPLOAD, 1) ;

      /* HTTP PUT please */
      curl_easy_setopt(curl, CURLOPT_PUT, 1);
      ::curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
      if ( m_HandlerVerbose )
        {
        ::curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        }

      cmStdString local_file = localprefix + "/" + files[cc];
      cmStdString remote_file = remoteprefix + files[cc];

      *m_LogFile << "\tUpload file: " << local_file.c_str() << " to "
          << remote_file.c_str() << std::endl;

      cmStdString ofile = "";
      for ( kk = 0; kk < remote_file.size(); kk ++ )
        {
        char c = remote_file[kk];
        char hex[4] = { 0, 0, 0, 0 };
        hex[0] = c;
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
          sprintf(hex, "%%%02X", (int)c);
          ofile.append(hex);
          break;
        default: 
          ofile.append(hex);
          }
        }
      cmStdString upload_as 
        = url + ((url.find("?",0) == cmStdString::npos) ? "?" : "&") 
        + "FileName=" + ofile;

      struct stat st;
      if ( ::stat(local_file.c_str(), &st) )
        {
        return false;
        }

      ftpfile = ::fopen(local_file.c_str(), "rb");
      cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "  Upload file: " << local_file.c_str() << " to " 
        << upload_as.c_str() << " Size: " << st.st_size << std::endl);


      // specify target
      ::curl_easy_setopt(curl,CURLOPT_URL, upload_as.c_str());

      // now specify which file to upload
      ::curl_easy_setopt(curl, CURLOPT_INFILE, ftpfile);

      // and give the size of the upload (optional)
      ::curl_easy_setopt(curl, CURLOPT_INFILESIZE, static_cast<long>(st.st_size));

      // and give curl the buffer for errors
      ::curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, &error_buffer);

      // specify handler for output
      ::curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cmCTestSubmitHandlerWriteMemoryCallback);

      /* we pass our 'chunk' struct to the callback function */
      cmCTestSubmitHandlerVectorOfChar chunk;
      ::curl_easy_setopt(curl, CURLOPT_FILE, (void *)&chunk);

      // Now run off and do what you've been told!
      res = ::curl_easy_perform(curl);

      cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "CURL output: ["
        << cmCTestLogWrite(&*chunk.begin(), chunk.size()) << "]" << std::endl);

      fclose(ftpfile);
      if ( res )
        {
        cmCTestLog(m_CTest, ERROR, "  Error when uploading file: " << local_file.c_str() << std::endl);
        cmCTestLog(m_CTest, ERROR, "  Error message was: " << error_buffer << std::endl);
        *m_LogFile << "  Error when uploading file: " << local_file.c_str() << std::endl
          << "  Error message was: " << error_buffer << std::endl
          << "  Curl output was: " << cmCTestLogWrite(&*chunk.begin(), chunk.size()) << std::endl;
        cmCTestLog(m_CTest, ERROR, "CURL output: ["
          << cmCTestLogWrite(&*chunk.begin(), chunk.size()) << "]" << std::endl);
        ::curl_easy_cleanup(curl);
        ::curl_global_cleanup(); 
        return false;
        }
      // always cleanup
      ::curl_easy_cleanup(curl);
      cmCTestLog(m_CTest, HANDLER_OUTPUT, "  Uploaded: " + local_file << std::endl);
      }
    }
  ::curl_global_cleanup(); 
  return true;
}

//----------------------------------------------------------------------------
bool cmCTestSubmitHandler::TriggerUsingHTTP(const std::vector<cmStdString>& files,
  const cmStdString& remoteprefix, 
  const cmStdString& url)
{
  CURL *curl;
  char error_buffer[1024];

  /* In windows, this will init the winsock stuff */
  ::curl_global_init(CURL_GLOBAL_ALL);

  cmStdString::size_type cc, kk;
  for ( cc = 0; cc < files.size(); cc ++ )
    {
    /* get a curl handle */
    curl = curl_easy_init();
    if(curl) 
      {
      // Using proxy
      if ( m_HTTPProxyType > 0 )
        {
        curl_easy_setopt(curl, CURLOPT_PROXY, m_HTTPProxy.c_str()); 
        switch (m_HTTPProxyType)
          {
        case 2:
          curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4);
          break;
        case 3:
          curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);
          break;
        default:
          curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);           
          if (m_HTTPProxyAuth.size() > 0)
            {
            curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD,
              m_HTTPProxyAuth.c_str());
            }
          }
        }

      ::curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
      if ( m_HandlerVerbose )
        {
        ::curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        }

      // and give curl the buffer for errors
      ::curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, &error_buffer);

      // specify handler for output
      ::curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cmCTestSubmitHandlerWriteMemoryCallback);

      /* we pass our 'chunk' struct to the callback function */
      cmCTestSubmitHandlerVectorOfChar chunk;
      ::curl_easy_setopt(curl, CURLOPT_FILE, (void *)&chunk);

      cmStdString file = remoteprefix + files[cc];
      cmStdString ofile = "";
      for ( kk = 0; kk < file.size(); kk ++ )
        {
        char c = file[kk];
        char hex[4] = { 0, 0, 0, 0 };
        hex[0] = c;
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
          sprintf(hex, "%%%02X", (int)c);
          ofile.append(hex);
          break;
        default: 
          ofile.append(hex);
          }
        }
      cmStdString turl 
        = url + ((url.find("?",0) == cmStdString::npos) ? "?" : "&") 
        + "xmlfile=" + ofile;
      *m_LogFile << "Trigger url: " << turl.c_str() << std::endl;
      cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "  Trigger url: " << turl.c_str() << std::endl);
      curl_easy_setopt(curl, CURLOPT_URL, turl.c_str());
      if ( curl_easy_perform(curl) )
        {
        cmCTestLog(m_CTest, ERROR, "  Error when triggering: " << turl.c_str() << std::endl);
        cmCTestLog(m_CTest, ERROR, "  Error message was: " << error_buffer << std::endl);
        *m_LogFile << "\tTrigerring failed with error: " << error_buffer << std::endl
          << "  Error message was: " << error_buffer << std::endl
          << "  Curl output was: " << cmCTestLogWrite(&*chunk.begin(), chunk.size()) << std::endl;
        cmCTestLog(m_CTest, ERROR, "CURL output: ["
          << cmCTestLogWrite(&*chunk.begin(), chunk.size()) << "]" << std::endl);
        ::curl_easy_cleanup(curl);
        ::curl_global_cleanup(); 
        return false;
        }

      cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "CURL output: ["
        << cmCTestLogWrite(&*chunk.begin(), chunk.size()) << "]" << std::endl);

      // always cleanup
      ::curl_easy_cleanup(curl);
      cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, std::endl);
      }
    }
  ::curl_global_cleanup(); 
  cmCTestLog(m_CTest, HANDLER_OUTPUT, "  Dart server triggered..." << std::endl);
  return true;
}

//----------------------------------------------------------------------------
bool cmCTestSubmitHandler::SubmitUsingSCP(
  const cmStdString& scp_command, 
  const cmStdString& localprefix, 
  const std::vector<cmStdString>& files,
  const cmStdString& remoteprefix, 
  const cmStdString& url)
{
  if ( !scp_command.size() || !localprefix.size() ||
    !files.size() || !remoteprefix.size() || !url.size() )
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

  std::vector<cmStdString>::const_iterator it;
  for ( it = files.begin();
    it != files.end();
    it ++ )
    {
    int retVal;

    std::string lfname = localprefix;
    cmSystemTools::ConvertToUnixSlashes(lfname);
    lfname += "/" + *it;
    lfname = cmSystemTools::ConvertToOutputPath(lfname.c_str());
    argv[1] = lfname.c_str();
    std::string rfname = url + "/" + remoteprefix + *it;
    argv[2] = rfname.c_str();
    cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "Execute \"" << argv[0] << "\" \"" << argv[1] << "\" \"" 
      << argv[2] << "\"" << std::endl);
    *m_LogFile << "Execute \"" << argv[0] << "\" \"" << argv[1] << "\" \"" 
      << argv[2] << "\"" << std::endl;
    cmsysProcess_SetCommand(cp, &*argv.begin());
    cmsysProcess_Execute(cp);
    char* data;
    int length;
    while(cmsysProcess_WaitForData(cp, &data, &length, 0))
      {
      cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, cmCTestLogWrite(data, length));
      }
    cmsysProcess_WaitForExit(cp, 0);
    int result = cmsysProcess_GetState(cp);

    if(result == cmsysProcess_State_Exited)
      {
      retVal = cmsysProcess_GetExitValue(cp);
      if ( retVal != 0 )
        {
        cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "\tSCP returned: " << retVal << std::endl);
        *m_LogFile << "\tSCP returned: " << retVal << std::endl;
        problems ++;
        }
      }
    else if(result == cmsysProcess_State_Exception)
      {
      retVal = cmsysProcess_GetExitException(cp);
      cmCTestLog(m_CTest, ERROR, "\tThere was an exception: " << retVal << std::endl);
      *m_LogFile << "\tThere was an exception: " << retVal << std::endl;
      problems ++;
      }
    else if(result == cmsysProcess_State_Expired)
      {
      cmCTestLog(m_CTest, ERROR, "\tThere was a timeout" << std::endl);
      *m_LogFile << "\tThere was a timeout" << std::endl;
      problems ++;
      } 
    else if(result == cmsysProcess_State_Error)
      {
      cmCTestLog(m_CTest, ERROR, "\tError executing SCP: "
        << cmsysProcess_GetErrorString(cp) << std::endl);
      *m_LogFile << "\tError executing SCP: "
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
bool cmCTestSubmitHandler::SubmitUsingXMLRPC(const cmStdString& localprefix, 
  const std::vector<cmStdString>& files,
  const cmStdString& remoteprefix, 
  const cmStdString& url)
{
  xmlrpc_env env;
  std::string ctestVersion = cmVersion::GetCMakeVersion();

  cmStdString realURL = url + "/" + remoteprefix + "/Command/";

  /* Start up our XML-RPC client library. */
  xmlrpc_client_init(XMLRPC_CLIENT_NO_FLAGS, "CTest", ctestVersion.c_str());

  /* Initialize our error-handling environment. */
  xmlrpc_env_init(&env);

  /* Call the famous server at UserLand. */
  cmCTestLog(m_CTest, HANDLER_OUTPUT, "  Submitting to: " << realURL.c_str() << " (" << remoteprefix.c_str() << ")" << std::endl);
  std::vector<cmStdString>::const_iterator it;
  for ( it = files.begin(); it != files.end(); ++it )
    {
    xmlrpc_value *result;

    std::string local_file = localprefix + "/" + *it;
    cmCTestLog(m_CTest, HANDLER_OUTPUT, "  Submit file: " << local_file.c_str() << std::endl);
    struct stat st;
    if ( ::stat(local_file.c_str(), &st) )
      {
      return false;
      }

    size_t fileSize = st.st_size;
    FILE* fp = fopen(local_file.c_str(), "r");
    if ( !fp )
      {
      return false;
      }

    unsigned char *fileBuffer = new unsigned char[fileSize];
    if ( fread(fileBuffer, 1, fileSize, fp) != fileSize )
      {
      delete [] fileBuffer;
      fclose(fp);
      return false;
      }
    fclose(fp);

    std::string remoteCommand = "Submit.put";
    result = xmlrpc_client_call(&env, realURL.c_str(),
      remoteCommand.c_str(),
      "(6)", fileBuffer, (xmlrpc_int32)fileSize );

    delete [] fileBuffer;

    if ( env.fault_occurred )
      {
      cmCTestLog(m_CTest, ERROR, " Submission problem: " << env.fault_string << " (" << env.fault_code << ")" << std::endl);
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

//----------------------------------------------------------------------------
int cmCTestSubmitHandler::ProcessHandler()
{
  const std::string &buildDirectory = m_CTest->GetCTestConfiguration("BuildDirectory");
  if ( buildDirectory.size() == 0 )
    {
    cmCTestLog(m_CTest, ERROR, "Cannot find BuildDirectory  key in the DartConfiguration.tcl" << std::endl);
    return -1;
    }

  cmGeneratedFileStream ofs;
  m_CTest->OpenOutputFile("Temporary", "LastSubmit.log", ofs);

  cmCTest::tm_VectorOfStrings files;
  std::string prefix = this->GetSubmitResultsPrefix();
  // TODO:
  // Check if test is enabled
  m_CTest->AddIfExists(files, "Update.xml");
  m_CTest->AddIfExists(files, "Configure.xml");
  m_CTest->AddIfExists(files, "Build.xml");
  m_CTest->AddIfExists(files, "Test.xml");
  if ( m_CTest->AddIfExists(files, "Coverage.xml") )
    {
    cmCTest::tm_VectorOfStrings gfiles;
    std::string gpath = buildDirectory + "/Testing/" + m_CTest->GetCurrentTag();
    std::string::size_type glen = gpath.size() + 1;
    gpath = gpath + "/CoverageLog*";
    cmCTestLog(m_CTest, DEBUG, "Globbing for: " << gpath.c_str() << std::endl);
    if ( cmSystemTools::SimpleGlob(gpath, gfiles, 1) )
      {
      size_t cc;
      for ( cc = 0; cc < gfiles.size(); cc ++ )
        {
        gfiles[cc] = gfiles[cc].substr(glen);
        cmCTestLog(m_CTest, DEBUG, "Glob file: " << gfiles[cc].c_str() << std::endl);
        files.push_back(gfiles[cc]);
        }
      }
    else
      {
      cmCTestLog(m_CTest, ERROR, "Problem globbing" << std::endl);
      }
    }
  m_CTest->AddIfExists(files, "DynamicAnalysis.xml");
  m_CTest->AddIfExists(files, "Purify.xml");
  m_CTest->AddIfExists(files, "Notes.xml");

  if ( ofs )
    {
    ofs << "Upload files:" << std::endl;
    int cnt = 0;
    cmCTest::tm_VectorOfStrings::iterator it;
    for ( it = files.begin(); it != files.end(); ++ it )
      {
      ofs << cnt << "\t" << it->c_str() << std::endl;
      cnt ++;
      }
    }
  cmCTestLog(m_CTest, HANDLER_OUTPUT, "Submit files (using " << m_CTest->GetCTestConfiguration("DropMethod") << ")"
    << std::endl);
  this->SetLogFile(&ofs);
  if ( m_CTest->GetCTestConfiguration("DropMethod") == "" ||
    m_CTest->GetCTestConfiguration("DropMethod") ==  "ftp" )
    {
    ofs << "Using drop method: FTP" << std::endl;
    cmCTestLog(m_CTest, HANDLER_OUTPUT, "  Using FTP submit method" << std::endl);
    std::string url = "ftp://";
    url += cmCTest::MakeURLSafe(m_CTest->GetCTestConfiguration("DropSiteUser")) + ":" + 
      cmCTest::MakeURLSafe(m_CTest->GetCTestConfiguration("DropSitePassword")) + "@" + 
      m_CTest->GetCTestConfiguration("DropSite") + 
      cmCTest::MakeURLSafe(m_CTest->GetCTestConfiguration("DropLocation"));
    if ( !this->SubmitUsingFTP(buildDirectory+"/Testing/"+m_CTest->GetCurrentTag(), 
        files, prefix, url) )
      {
      cmCTestLog(m_CTest, ERROR, "  Problems when submitting via FTP" << std::endl);
      ofs << "  Problems when submitting via FTP" << std::endl;
      return -1;
      }
    if ( !this->TriggerUsingHTTP(files, prefix, m_CTest->GetCTestConfiguration("TriggerSite")) )
      {
      cmCTestLog(m_CTest, ERROR, "  Problems when triggering via HTTP" << std::endl);
      ofs << "  Problems when triggering via HTTP" << std::endl;
      return -1;
      }
    cmCTestLog(m_CTest, HANDLER_OUTPUT, "  Submission successful" << std::endl);
    ofs << "  Submission successful" << std::endl;
    return 0;
    }
  else if ( m_CTest->GetCTestConfiguration("DropMethod") == "http" )
    {
    ofs << "Using drop method: HTTP" << std::endl;
    cmCTestLog(m_CTest, HANDLER_OUTPUT, "  Using HTTP submit method" << std::endl);
    std::string url = "http://";
    if ( m_CTest->GetCTestConfiguration("DropSiteUser").size() > 0 )
      {
      url += m_CTest->GetCTestConfiguration("DropSiteUser");
      if ( m_CTest->GetCTestConfiguration("DropSitePassword").size() > 0 )
        {
        url += ":" + m_CTest->GetCTestConfiguration("DropSitePassword");
        }
      url += "@";
      }
    url += m_CTest->GetCTestConfiguration("DropSite") + m_CTest->GetCTestConfiguration("DropLocation");
    if ( !this->SubmitUsingHTTP(buildDirectory +"/Testing/"+m_CTest->GetCurrentTag(), files, prefix, url) )
      {
      cmCTestLog(m_CTest, ERROR, "  Problems when submitting via HTTP" << std::endl);
      ofs << "  Problems when submitting via HTTP" << std::endl;
      return -1;
      }
    if ( !this->TriggerUsingHTTP(files, prefix, m_CTest->GetCTestConfiguration("TriggerSite")) )
      {
      cmCTestLog(m_CTest, ERROR, "  Problems when triggering via HTTP" << std::endl);
      ofs << "  Problems when triggering via HTTP" << std::endl;
      return -1;
      }
    cmCTestLog(m_CTest, HANDLER_OUTPUT, "  Submission successful" << std::endl);
    ofs << "  Submission successful" << std::endl;
    return 0;
    }
  else if ( m_CTest->GetCTestConfiguration("DropMethod") == "xmlrpc" )
    {
    ofs << "Using drop method: XML-RPC" << std::endl;
    cmCTestLog(m_CTest, HANDLER_OUTPUT, "  Using XML-RPC submit method" << std::endl);
    std::string url = m_CTest->GetCTestConfiguration("DropSite");
    prefix = m_CTest->GetCTestConfiguration("DropLocation");
    if ( !this->SubmitUsingXMLRPC(buildDirectory+"/Testing/"+m_CTest->GetCurrentTag(), files, prefix, url) )
      {
      cmCTestLog(m_CTest, ERROR, "  Problems when submitting via XML-RPC" << std::endl);
      ofs << "  Problems when submitting via XML-RPC" << std::endl;
      return -1;
      }
    cmCTestLog(m_CTest, HANDLER_OUTPUT, "  Submission successful" << std::endl);
    ofs << "  Submission successful" << std::endl;
    return 0;
    }
  else if ( m_CTest->GetCTestConfiguration("DropMethod") == "scp" )
    {
    std::string url;
    if ( m_CTest->GetCTestConfiguration("DropSiteUser").size() > 0 )
      {
      url += m_CTest->GetCTestConfiguration("DropSiteUser") + "@";
      }
    url += m_CTest->GetCTestConfiguration("DropSite") + ":" + m_CTest->GetCTestConfiguration("DropLocation");

    if ( !this->SubmitUsingSCP(m_CTest->GetCTestConfiguration("ScpCommand"),
        buildDirectory+"/Testing/"+m_CTest->GetCurrentTag(), files, prefix, url) )
      {
      cmCTestLog(m_CTest, ERROR, "  Problems when submitting via SCP" << std::endl);
      ofs << "  Problems when submitting via SCP" << std::endl;
      return -1;
      }
    cmCTestLog(m_CTest, HANDLER_OUTPUT, "  Submission successful" << std::endl);
    ofs << "  Submission successful" << std::endl;
    }
  cmCTestLog(m_CTest, ERROR, "   Unknown submission method: \"" << m_CTest->GetCTestConfiguration("DropMethod") << "\"" << std::endl);
  return -1;
}

//----------------------------------------------------------------------------
std::string cmCTestSubmitHandler::GetSubmitResultsPrefix()
{
  std::string name = m_CTest->GetCTestConfiguration("Site") +
    "___" + m_CTest->GetCTestConfiguration("BuildName") +
    "___" + m_CTest->GetCurrentTag() + "-" +
    m_CTest->GetTestModelString() + "___XML___";
  return name;
}


