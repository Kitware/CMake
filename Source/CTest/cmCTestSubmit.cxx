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
  
  /* get a curl handle */
  curl = curl_easy_init();
  if(curl) 
    {
    // enable uploading
    ::curl_easy_setopt(curl, CURLOPT_UPLOAD, TRUE) ;
    
    std::string::size_type cc;
    for ( cc = 0; cc < files.size(); cc ++ )
      {
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
      }
    // always cleanup
    ::curl_easy_cleanup(curl);
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
