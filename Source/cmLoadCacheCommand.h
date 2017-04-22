/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmLoadCacheCommand_h
#define cmLoadCacheCommand_h

#include "cmConfigure.h"

#include <set>
#include <string>
#include <vector>

#include "cmCommand.h"

class cmExecutionStatus;

/** \class cmLoadCacheCommand
 * \brief load a cache file
 *
 * cmLoadCacheCommand loads the non internal values of a cache file
 */
class cmLoadCacheCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  cmCommand* Clone() CM_OVERRIDE { return new cmLoadCacheCommand; }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) CM_OVERRIDE;

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  std::string GetName() const CM_OVERRIDE { return "load_cache"; }

protected:
  std::set<std::string> VariablesToRead;
  std::string Prefix;

  bool ReadWithPrefix(std::vector<std::string> const& args);
  void CheckLine(const char* line);
};

#endif
