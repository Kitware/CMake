/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmStringTable.h"

#include <list>
#include <unordered_map>

struct string_hasher
{
  size_t operator()(std::string * k) const
  {
    std::hash<std::string> h;
    return h(*k);
  }
};

struct string_comparer
{
  bool operator()(const std::string * l, const std::string * r) const
  {
    return (*l) == (*r);
  }
};

std::list<std::string> _string_storage;
std::unordered_map<std::string *, size_t, string_hasher, string_comparer> _string_to_id_map;
std::unordered_map<size_t, std::string *> _id_to_string_map;

size_t cmStringTable::GetStringId(const std::string & str)
{
  auto it = _string_to_id_map.find(const_cast<std::string *>(&str));
  if (it == _string_to_id_map.end()) {
    _string_storage.emplace_back(str);
    // As long as we never delete anything this iterator should remain in memory
    std::string * addr = &(_string_storage.back()); 
    it = _string_to_id_map.emplace(addr, _string_to_id_map.size() + 1).first;
    _id_to_string_map.emplace(it->second, it->first);
  }
  return it->second;
}

size_t cmStringTable::GetStringId(const char * str)
{
  return GetStringId(std::string(str));
}

const std::string & cmStringTable::GetString(size_t id)
{
  auto it = _id_to_string_map.find(id);
  if (it != _id_to_string_map.end()) {
    return *it->second;
  }

  static std::string empty;
  return empty;
}
