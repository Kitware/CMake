#include "curl/curl.h"

int GetFtpFile(void)
{
  int retVal = 0;
  CURL *curl;
  CURLcode res;
  curl = curl_easy_init();
  if(curl) 
    {
    /* Get curl 7.9.2 from sunet.se's FTP site: */
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(curl, CURLOPT_HEADER, 1);
    curl_easy_setopt(curl, CURLOPT_URL,
                     "ftp://public.kitware.com/pub/cmake/cygwin/setup.hint");
    res = curl_easy_perform(curl);
    if ( res != 0 )
      {
      printf("Error fetching: ftp://public.kitware.com/pub/cmake/cygwin/setup.hint\n");
      retVal = 1;
      }

    /* always cleanup */
    curl_easy_cleanup(curl);
    }
  else
    {
    printf("Cannot create curl object\n");
    retVal = 1;
    }
  return retVal;
}

int GetWebFile(void)
{
  int retVal = 0;
  CURL *curl;
  CURLcode res;
  curl = curl_easy_init();
  if(curl) 
    {
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(curl, CURLOPT_HEADER, 1);

    /* get the first document */
    curl_easy_setopt(curl, CURLOPT_URL, "http://www.cmake.org/HTML/Sponsors.html");
    res = curl_easy_perform(curl);
    if ( res != 0 )
      {
      printf("Error fetching: http://www.cmake.org/HTML/Sponsors.html\n");
      retVal = 1;
      }


    /* get another document from the same server using the same
       connection */
    curl_easy_setopt(curl, CURLOPT_URL, "http://www.cmake.org/HTML/Index.html");
    res = curl_easy_perform(curl);
    if ( res != 0 )
      {
      printf("Error fetching: http://www.cmake.org/HTML/Index.html\n");
      retVal = 1;
      }

    /* always cleanup */
    curl_easy_cleanup(curl);
    }
  else
    {
    printf("Cannot create curl object\n");
    retVal = 1;
    }

  return retVal;
}

int main(int argc, char **argv)
{
  int retVal = 0;
  curl_global_init(CURL_GLOBAL_DEFAULT);
  retVal += GetWebFile();
  retVal += GetFtpFile();
  curl_global_cleanup();
  return retVal;
}
