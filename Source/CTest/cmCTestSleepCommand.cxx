/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmCTestSleepCommand.h"

#include <chrono>
#include <cstdlib>
#include <thread>

#include "cmExecutionStatus.h"

bool cmCTestSleepCommand(std::vector<std::string> const& args,
                         cmExecutionStatus& status)
{
  // sleep for specified seconds
  if (args.size() == 1) {
    unsigned int duration = atoi(args[0].c_str());
    std::this_thread::sleep_for(std::chrono::seconds(duration));
    return true;
  }

  // sleep up to a duration
  if (args.size() == 3) {
    unsigned int time1 = atoi(args[0].c_str());
    unsigned int duration = atoi(args[1].c_str());
    unsigned int time2 = atoi(args[2].c_str());
    if (time1 + duration > time2) {
      duration = (time1 + duration - time2);
      std::this_thread::sleep_for(std::chrono::seconds(duration));
    }
    return true;
  }

  status.SetError("called with incorrect number of arguments");
  return false;
}
