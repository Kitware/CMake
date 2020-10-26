/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <map>
#include <string>

class cmCTestResourceSpec;

class cmCTestResourceAllocator
{
public:
  struct Resource
  {
    unsigned int Total;
    unsigned int Locked;

    unsigned int Free() const { return this->Total - this->Locked; }

    bool operator==(const Resource& other) const;
    bool operator!=(const Resource& other) const;
  };

  void InitializeFromResourceSpec(const cmCTestResourceSpec& spec);

  const std::map<std::string, std::map<std::string, Resource>>& GetResources()
    const;

  bool AllocateResource(const std::string& name, const std::string& id,
                        unsigned int slots);
  bool DeallocateResource(const std::string& name, const std::string& id,
                          unsigned int slots);

private:
  std::map<std::string, std::map<std::string, Resource>> Resources;
};
