/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCTestResourceSpec_h
#define cmCTestResourceSpec_h

#include <map>
#include <string>
#include <vector>

class cmCTestResourceSpec
{
public:
  class Resource
  {
  public:
    std::string Id;
    unsigned int Capacity;

    bool operator==(const Resource& other) const;
    bool operator!=(const Resource& other) const;
  };

  class Socket
  {
  public:
    std::map<std::string, std::vector<Resource>> Resources;

    bool operator==(const Socket& other) const;
    bool operator!=(const Socket& other) const;
  };

  Socket LocalSocket;

  bool ReadFromJSONFile(const std::string& filename);

  bool operator==(const cmCTestResourceSpec& other) const;
  bool operator!=(const cmCTestResourceSpec& other) const;
};

#endif
