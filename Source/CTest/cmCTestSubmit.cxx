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

#include "cmCTestSubmit.h"
#include "cmSystemTools.h"

#include <cmsys/Process.h>
#include "CTest/Curl/curl/curl.h"
#include <sys/stat.h>

cmCTestSubmit::cmCTestSubmit() : m_HTTPProxy(), m_FTPProxy()
{
  m_Verbose = false;
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
    std::cout << "  Use HTTP Proxy: " << m_HTTPProxy << std::endl;
    }
  if ( m_FTPProxy.size() > 0 )
    {
    std::cout << "  Use FTP Proxy: " << m_FTPProxy << std::endl;
    }
}

bool cmCTestSubmit::SubmitUsingFTP(const cmStdString& localprefix, 
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
      if ( m_Verbose )
        {
        std::cout << "  Upload file: " << local_file.c_str() << " to " 
          << upload_as.c_str() << std::endl;
        }

      if ( m_Verbose )
        {
        ::curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        }
      // specify target
      ::curl_easy_setopt(curl,CURLOPT_URL, upload_as.c_str());

      // now specify which file to upload
      ::curl_easy_setopt(curl, CURLOPT_INFILE, ftpfile);

      // and give the size of the upload (optional)
      ::curl_easy_setopt(curl, CURLOPT_INFILESIZE, static_cast<long>(st.st_size));

      ::curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, &error_buffer);

      // Now run off and do what you've been told!
      res = ::curl_easy_perform(curl);
      fclose(ftpfile);
      if ( res )
        {
        std::cerr << "  Error when uploading file: " << local_file.c_str() << std::endl;
        std::cerr << "  Error message was: " << error_buffer << std::endl;
        *m_LogFile << "  Error when uploading file: " << local_file.c_str() << std::endl
          << "  Error message was: " << error_buffer << std::endl;
        ::curl_easy_cleanup(curl);
        ::curl_global_cleanup(); 
        return false;
        }
      // always cleanup
      ::curl_easy_cleanup(curl);
      std::cout << "  Uploaded: " + local_file << std::endl;
      }
    }
  ::curl_global_cleanup(); 
  return true;
}

// Uploading files is simpler
bool cmCTestSubmit::SubmitUsingHTTP(const cmStdString& localprefix, 
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
      if ( m_Verbose )
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
      if ( m_Verbose )
        {
        std::cout << "  Upload file: " << local_file.c_str() << " to " 
          << upload_as.c_str() << " Size: " << st.st_size << std::endl;
        }


      // specify target
      ::curl_easy_setopt(curl,CURLOPT_URL, upload_as.c_str());

      // now specify which file to upload
      ::curl_easy_setopt(curl, CURLOPT_INFILE, ftpfile);

      // and give the size of the upload (optional)
      ::curl_easy_setopt(curl, CURLOPT_INFILESIZE, static_cast<long>(st.st_size));

      // and give curl the buffer for errors
      ::curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, &error_buffer);

      // Now run off and do what you've been told!
      res = ::curl_easy_perform(curl);

      fclose(ftpfile);
      if ( res )
        {
        std::cerr << "  Error when uploading file: " << local_file.c_str() << std::endl;
        *m_LogFile << "  Error when uploading file: " << local_file.c_str() << std::endl
          << "  Error message was: " << error_buffer << std::endl;
        ::curl_easy_cleanup(curl);
        ::curl_global_cleanup(); 
        return false;
        }
      // always cleanup
      ::curl_easy_cleanup(curl);
      std::cout << "  Uploaded: " + local_file << std::endl;
      }
    }
  ::curl_global_cleanup(); 
  return true;
}

bool cmCTestSubmit::TriggerUsingHTTP(const std::vector<cmStdString>& files,
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
      if ( m_Verbose )
        {
        ::curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        }

      // and give curl the buffer for errors
      ::curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, &error_buffer);

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
      if ( m_Verbose )
        {
        std::cout << "  Trigger url: " << turl.c_str() << std::endl;
        }
      curl_easy_setopt(curl, CURLOPT_URL, turl.c_str());
      if ( curl_easy_perform(curl) )
        {
        std::cerr << "  Error when triggering: " << turl.c_str() << std::endl;
        *m_LogFile << "\tTrigerring failed with error: " << error_buffer << std::endl;
        ::curl_easy_cleanup(curl);
        ::curl_global_cleanup(); 
        return false;
        }
      // always cleanup
      ::curl_easy_cleanup(curl);
      std::cout << std::endl;
      }
    }
  ::curl_global_cleanup(); 
  std::cout << "  Dart server triggered..." << std::endl;
  return true;
}

bool cmCTestSubmit::SubmitUsingSCP(
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
    if ( m_Verbose )
      {
      std::cout << "Execute \"" << argv[0] << "\" \"" << argv[1] << "\" \"" 
        << argv[2] << "\"" << std::endl;
      }
    *m_LogFile << "Execute \"" << argv[0] << "\" \"" << argv[1] << "\" \"" 
      << argv[2] << "\"" << std::endl;
    cmsysProcess_SetCommand(cp, &*argv.begin());
    cmsysProcess_Execute(cp);
    char* data;
    int length;
    while(cmsysProcess_WaitForData(cp, &data, &length, 0))
      {
      std::cout.write(data, length);
      }
    cmsysProcess_WaitForExit(cp, 0);
    int result = cmsysProcess_GetState(cp);

    if(result == cmsysProcess_State_Exited)
      {
      retVal = cmsysProcess_GetExitValue(cp);
      if ( retVal != 0 )
        {
        if ( m_Verbose )
          {
          std::cout << "\tSCP returned: " << retVal << std::endl;
          }
        *m_LogFile << "\tSCP returned: " << retVal << std::endl;
        problems ++;
        }
      }
    else if(result == cmsysProcess_State_Exception)
      {
      retVal = cmsysProcess_GetExitException(cp);
      if ( m_Verbose )
        {
        std::cerr << "\tThere was an exception: " << retVal << std::endl;
        }
      *m_LogFile << "\tThere was an exception: " << retVal << std::endl;
      problems ++;
      }
    else if(result == cmsysProcess_State_Expired)
      {
      if ( m_Verbose )
        {
        std::cerr << "\tThere was a timeout" << std::endl;
        }
      *m_LogFile << "\tThere was a timeout" << std::endl;
      problems ++;
      } 
    else if(result == cmsysProcess_State_Error)
      {
      if ( m_Verbose )
        {
        std::cerr << "\tError executing SCP: "
          << cmsysProcess_GetErrorString(cp) << std::endl;
        }
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
