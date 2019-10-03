/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCTestHardwareSpec_h
#define cmCTestHardwareSpec_h

#include <map>
#include <string>
#include <vector>

class cmCTestHardwareSpec
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

  bool operator==(const cmCTestHardwareSpec& other) const;
  bool operator!=(const cmCTestHardwareSpec& other) const;
};

#endif
