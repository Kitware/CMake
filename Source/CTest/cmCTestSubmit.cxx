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

#include "curl/curl.h"
#include <sys/stat.h>

cmCTestSubmit::cmCTestSubmit() : m_HTTPProxy(), m_FTPProxy()
{
  std::cout << "Setup proxy" << std::endl;
  m_HTTPProxy = "";
  m_HTTPProxyType = 0;
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
      std::string type = getenv("HTTP_PROXY_TYPE");
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
      std::string type = getenv("FTP_PROXY_TYPE");
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
  std::cout << this << " HTTP Proxy: " << m_HTTPProxy << std::endl;

}

bool cmCTestSubmit::SubmitUsingFTP(const std::string& localprefix, 
                                   const std::vector<std::string>& files,
                                   const std::string& remoteprefix, 
                                   const std::string& url)
{
  CURL *curl;
  CURLcode res;
  FILE* ftpfile;

  /* In windows, this will init the winsock stuff */
  ::curl_global_init(CURL_GLOBAL_ALL);
  
  std::string::size_type cc;
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
      ::curl_easy_setopt(curl, CURLOPT_UPLOAD, TRUE) ;
      
      std::string local_file = localprefix + "/" + files[cc];
      std::string upload_as = url + "/" + remoteprefix + files[cc];
      
      struct stat st;
      if ( ::stat(local_file.c_str(), &st) )
        {
        return false;
        }

      ftpfile = ::fopen(local_file.c_str(), "rb");
      std::cout << "upload file: " << local_file.c_str() << " to " 
                << upload_as.c_str() << std::endl;

      ::curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
      // specify target
      ::curl_easy_setopt(curl,CURLOPT_URL, upload_as.c_str());
      
      // now specify which file to upload
      ::curl_easy_setopt(curl, CURLOPT_INFILE, ftpfile);

      // and give the size of the upload (optional)
      ::curl_easy_setopt(curl, CURLOPT_INFILESIZE, st.st_size);

      // Now run off and do what you've been told!
      res = ::curl_easy_perform(curl);
      fclose(ftpfile);
      if ( res )
        {
        std::cout << "Error when uploading" << std::endl;
        ::curl_easy_cleanup(curl);
        ::curl_global_cleanup(); 
        return false;
        }
      // always cleanup
      ::curl_easy_cleanup(curl);
      }
    }
  ::curl_global_cleanup(); 
  return true;
}

// Uploading files is simpler
bool cmCTestSubmit::SubmitUsingHTTP(const std::string& localprefix, 
                                   const std::vector<std::string>& files,
                                   const std::string& remoteprefix, 
                                   const std::string& url)
{
  std::cout << this << " Using proxy: " << m_HTTPProxy << std::endl;
  CURL *curl;
  CURLcode res;
  FILE* ftpfile;

  /* In windows, this will init the winsock stuff */
  ::curl_global_init(CURL_GLOBAL_ALL);

  std::string::size_type cc, kk;
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
          }
        }

      /* enable uploading */
      curl_easy_setopt(curl, CURLOPT_UPLOAD, TRUE) ;

      /* HTTP PUT please */
      curl_easy_setopt(curl, CURLOPT_PUT, TRUE);
      ::curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

      std::string local_file = localprefix + "/" + files[cc];
      std::string remote_file = remoteprefix + files[cc];
      std::string ofile = "";
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
            break;
          default: 
            ofile.append(hex);
          }
        }
      std::string upload_as = url + "?FileName=" + ofile;

      struct stat st;
      if ( ::stat(local_file.c_str(), &st) )
        {
        return false;
        }

      ftpfile = ::fopen(local_file.c_str(), "rb");
      
      std::cout << "upload file: " << local_file.c_str() << " to " 
                << upload_as.c_str() << " Size: " << st.st_size << std::endl;

                
      // specify target
      ::curl_easy_setopt(curl,CURLOPT_URL, upload_as.c_str());

      // now specify which file to upload
      ::curl_easy_setopt(curl, CURLOPT_INFILE, ftpfile);

      // and give the size of the upload (optional)
      ::curl_easy_setopt(curl, CURLOPT_INFILESIZE, st.st_size);

      // Now run off and do what you've been told!
      res = ::curl_easy_perform(curl);

      fclose(ftpfile);
      if ( res )
        {
        std::cout << "Error when uploading" << std::endl;
        ::curl_easy_cleanup(curl);
        ::curl_global_cleanup(); 
        return false;
        }
      // always cleanup
      ::curl_easy_cleanup(curl);
      }
    }
  ::curl_global_cleanup(); 
  return true;
}

bool cmCTestSubmit::TriggerUsingHTTP(const std::vector<std::string>& files,
                                     const std::string& remoteprefix, 
                                     const std::string& url)
{
  CURL *curl;
  CURLcode res = CURLcode();
  FILE* ftpfile;

  /* In windows, this will init the winsock stuff */
  ::curl_global_init(CURL_GLOBAL_ALL);
  
  std::string::size_type cc, kk;
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
          }
        }

      curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
      std::string file = remoteprefix + files[cc];
      std::string ofile = "";
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
            break;
          default: 
            ofile.append(hex);
          }
        }
      std::string turl = url + "?xmlfile=" + ofile;
      std::cout << "Trigger url: " << turl.c_str() << std::endl;
      curl_easy_setopt(curl, CURLOPT_URL, turl.c_str());
      res = curl_easy_perform(curl);
      if ( res )
        {
        std::cout << "Error when uploading" << std::endl;
        ::curl_easy_cleanup(curl);
        ::curl_global_cleanup(); 
        return false;
        }
      // always cleanup
      ::curl_easy_cleanup(curl);
      }
    }
  ::curl_global_cleanup(); 
  return true;
}

bool cmCTestSubmit::SubmitUsingSCP(const std::string& localprefix, 
                                   const std::vector<std::string>& files,
                                   const std::string& remoteprefix, 
                                   const std::string& url)
{
  std::cout << "SubmitUsingSCP is not yet implemented" << std::endl;
  return false;
}
