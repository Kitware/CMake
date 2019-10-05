/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmVariableWatch_h
#define cmVariableWatch_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <memory>
#include <string>
#include <vector>

class cmMakefile;

/** \class cmVariableWatch
 * \brief Helper class for watching of variable accesses.
 *
 * Calls function when variable is accessed
 */
class cmVariableWatch
{
public:
  using WatchMethod = void (*)(const std::string&, int, void*, const char*,
                               const cmMakefile*);
  using DeleteData = void (*)(void*);

  cmVariableWatch();
  ~cmVariableWatch();

  /**
   * Add watch to the variable
   */
  bool AddWatch(const std::string& variable, WatchMethod method,
                void* client_data = nullptr, DeleteData delete_data = nullptr);
  void RemoveWatch(const std::string& variable, WatchMethod method,
                   void* client_data = nullptr);

  /**
   * This method is called when variable is accessed
   */
  bool VariableAccessed(const std::string& variable, int access_type,
                        const char* newValue, const cmMakefile* mf) const;

  /**
   * Different access types.
   */
  enum
  {
    VARIABLE_READ_ACCESS,
    UNKNOWN_VARIABLE_READ_ACCESS,
    UNKNOWN_VARIABLE_DEFINED_ACCESS,
    VARIABLE_MODIFIED_ACCESS,
    VARIABLE_REMOVED_ACCESS,
    NO_ACCESS
  };

  /**
   * Return the access as string
   */
  static const std::string& GetAccessAsString(int access_type);

protected:
  struct Pair
  {
    WatchMethod Method = nullptr;
    void* ClientData = nullptr;
    DeleteData DeleteDataCall = nullptr;
    ~Pair()
    {
      if (this->DeleteDataCall && this->ClientData) {
        this->DeleteDataCall(this->ClientData);
      }
    }
    Pair() = default;
    Pair(const Pair&) = delete;
    Pair& operator=(const Pair&) = delete;
  };

  using VectorOfPairs = std::vector<std::shared_ptr<Pair>>;
  using StringToVectorOfPairs = std::map<std::string, VectorOfPairs>;

  StringToVectorOfPairs WatchMap;
};

#endif
