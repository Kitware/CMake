/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmVariableWatch.h"

#include <array>
#include <memory>
#include <utility>
#include <vector>

const std::string& cmVariableWatch::GetAccessAsString(int access_type)
{
  static const std::array<std::string, 6> cmVariableWatchAccessStrings = {
    { "READ_ACCESS", "UNKNOWN_READ_ACCESS", "UNKNOWN_DEFINED_ACCESS",
      "MODIFIED_ACCESS", "REMOVED_ACCESS", "NO_ACCESS" }
  };
  if (access_type < 0 || access_type >= cmVariableWatch::NO_ACCESS) {
    access_type = cmVariableWatch::NO_ACCESS;
  }
  return cmVariableWatchAccessStrings[access_type];
}

cmVariableWatch::cmVariableWatch() = default;

cmVariableWatch::~cmVariableWatch() = default;

bool cmVariableWatch::AddWatch(const std::string& variable, WatchMethod method,
                               void* client_data /*=0*/,
                               DeleteData delete_data /*=0*/)
{
  auto p = std::make_shared<cmVariableWatch::Pair>();
  p->Method = method;
  p->ClientData = client_data;
  p->DeleteDataCall = delete_data;
  cmVariableWatch::VectorOfPairs& vp = this->WatchMap[variable];
  for (auto& pair : vp) {
    if (pair->Method == method && client_data &&
        client_data == pair->ClientData) {
      // Callback already exists
      return false;
    }
  }
  vp.push_back(std::move(p));
  return true;
}

void cmVariableWatch::RemoveWatch(const std::string& variable,
                                  WatchMethod method, void* client_data /*=0*/)
{
  if (!this->WatchMap.count(variable)) {
    return;
  }
  cmVariableWatch::VectorOfPairs* vp = &this->WatchMap[variable];
  cmVariableWatch::VectorOfPairs::iterator it;
  for (it = vp->begin(); it != vp->end(); ++it) {
    if ((*it)->Method == method &&
        // If client_data is NULL, we want to disconnect all watches against
        // the given method; otherwise match ClientData as well.
        (!client_data || (client_data == (*it)->ClientData))) {
      vp->erase(it);
      return;
    }
  }
}

bool cmVariableWatch::VariableAccessed(const std::string& variable,
                                       int access_type, const char* newValue,
                                       const cmMakefile* mf) const
{
  auto mit = this->WatchMap.find(variable);
  if (mit != this->WatchMap.end()) {
    // The strategy here is to copy the list of callbacks, and ignore
    // new callbacks that existing ones may add.
    std::vector<std::weak_ptr<Pair>> vp(mit->second.begin(),
                                        mit->second.end());
    for (auto& weak_it : vp) {
      // In the case where a callback was removed, the weak_ptr will not be
      // lockable, and so this ensures we don't attempt to call into freed
      // memory
      if (auto it = weak_it.lock()) {
        it->Method(variable, access_type, it->ClientData, newValue, mf);
      }
    }
    return true;
  }
  return false;
}
