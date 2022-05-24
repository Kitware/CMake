#include <cassert>
#include <chrono>
#include <cstddef> // IWYU pragma: keep
#include <cstdlib>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "cmsys/Encoding.hxx"
#include "cmsys/FStream.hxx"

#include "cmCTestMultiProcessHandler.h"
#include "cmCTestResourceAllocator.h"
#include "cmCTestResourceSpec.h"
#include "cmCTestTestHandler.h"
#include "cmFileLock.h"
#include "cmFileLockResult.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

/*
 * This helper program is used to verify that the CTest resource allocation
 * feature is working correctly. It consists of two stages:
 *
 * 1) write - This stage receives the RESOURCE_GROUPS property of the test and
 *    compares it with the values passed in the CTEST_RESOURCE_GROUP_*
 *    environment variables. If it received all of the resources it expected,
 *    then it writes this information to a log file, which will be read in
 *    the verify stage.
 * 2) verify - This stage compares the log file with the resource spec file to
 *    make sure that no resources were over-subscribed, deallocated without
 *    being allocated, or allocated without being deallocated.
 */

static int usage(const char* argv0)
{
  std::cout << "Usage: " << argv0 << " (write|verify) <args...>" << std::endl;
  return 1;
}

static int usageWrite(const char* argv0)
{
  std::cout << "Usage: " << argv0
            << " write <log-file> <test-name> <sleep-time-secs>"
               " [<resource-groups-property>]"
            << std::endl;
  return 1;
}

static int usageVerify(const char* argv0)
{
  std::cout << "Usage: " << argv0
            << " verify <log-file> <resource-spec-file> [<test-names>]"
            << std::endl;
  return 1;
}

static int doWrite(int argc, char const* const* argv)
{
  if (argc < 5 || argc > 6) {
    return usageWrite(argv[0]);
  }
  std::string logFile = argv[2];
  std::string testName = argv[3];
  unsigned int sleepTime = std::atoi(argv[4]);
  std::vector<std::map<
    std::string, std::vector<cmCTestMultiProcessHandler::ResourceAllocation>>>
    resources;
  if (argc == 6) {
    // Parse RESOURCE_GROUPS property
    std::string resourceGroupsProperty = argv[5];
    std::vector<
      std::vector<cmCTestTestHandler::cmCTestTestResourceRequirement>>
      resourceGroups;
    bool result = cmCTestTestHandler::ParseResourceGroupsProperty(
      resourceGroupsProperty, resourceGroups);
    (void)result;
    assert(result);

    // Verify group count
    const char* resourceGroupCountEnv =
      cmSystemTools::GetEnv("CTEST_RESOURCE_GROUP_COUNT");
    if (!resourceGroupCountEnv) {
      std::cout << "CTEST_RESOURCE_GROUP_COUNT should be defined" << std::endl;
      return 1;
    }
    int resourceGroupCount = std::atoi(resourceGroupCountEnv);
    if (resourceGroups.size() !=
        static_cast<std::size_t>(resourceGroupCount)) {
      std::cout
        << "CTEST_RESOURCE_GROUP_COUNT does not match expected resource groups"
        << std::endl
        << "Expected: " << resourceGroups.size() << std::endl
        << "Actual: " << resourceGroupCount << std::endl;
      return 1;
    }

    if (!cmSystemTools::Touch(logFile + ".lock", true)) {
      std::cout << "Could not create lock file" << std::endl;
      return 1;
    }
    cmFileLock lock;
    auto lockResult =
      lock.Lock(logFile + ".lock", static_cast<unsigned long>(-1));
    if (!lockResult.IsOk()) {
      std::cout << "Could not lock file" << std::endl;
      return 1;
    }
    std::size_t i = 0;
    cmsys::ofstream fout(logFile.c_str(), std::ios::app);
    fout << "begin " << testName << std::endl;
    for (auto& resourceGroup : resourceGroups) {
      try {
        // Build and verify set of expected resources
        std::set<std::string> expectedResources;
        for (auto const& it : resourceGroup) {
          expectedResources.insert(it.ResourceType);
        }

        std::string prefix = "CTEST_RESOURCE_GROUP_";
        prefix += std::to_string(i);
        const char* actualResourcesCStr = cmSystemTools::GetEnv(prefix);
        if (!actualResourcesCStr) {
          std::cout << prefix << " should be defined" << std::endl;
          return 1;
        }

        auto actualResourcesVec =
          cmSystemTools::SplitString(actualResourcesCStr, ',');
        std::set<std::string> actualResources;
        for (auto const& r : actualResourcesVec) {
          if (!r.empty()) {
            actualResources.insert(r);
          }
        }

        if (actualResources != expectedResources) {
          std::cout << prefix << " did not list expected resources"
                    << std::endl;
          return 1;
        }

        // Verify that we got what we asked for and write it to the log
        prefix += '_';
        std::map<std::string,
                 std::vector<cmCTestMultiProcessHandler::ResourceAllocation>>
          resEntry;
        for (auto const& type : actualResources) {
          auto it = resourceGroup.begin();

          std::string varName = prefix;
          varName += cmSystemTools::UpperCase(type);
          const char* varVal = cmSystemTools::GetEnv(varName);
          if (!varVal) {
            std::cout << varName << " should be defined" << std::endl;
            return 1;
          }

          auto received = cmSystemTools::SplitString(varVal, ';');
          for (auto const& r : received) {
            while (it->ResourceType != type || it->UnitsNeeded == 0) {
              ++it;
              if (it == resourceGroup.end()) {
                std::cout << varName << " did not list expected resources"
                          << std::endl;
                return 1;
              }
            }
            auto split = cmSystemTools::SplitString(r, ',');
            if (split.size() != 2) {
              std::cout << varName << " was ill-formed" << std::endl;
              return 1;
            }
            if (!cmHasLiteralPrefix(split[0], "id:")) {
              std::cout << varName << " was ill-formed" << std::endl;
              return 1;
            }
            auto id = split[0].substr(3);
            if (!cmHasLiteralPrefix(split[1], "slots:")) {
              std::cout << varName << " was ill-formed" << std::endl;
              return 1;
            }
            auto slots = split[1].substr(6);
            unsigned int amount = std::atoi(slots.c_str());
            if (amount != static_cast<unsigned int>(it->SlotsNeeded)) {
              std::cout << varName << " did not list expected resources"
                        << std::endl;
              return 1;
            }
            --it->UnitsNeeded;

            fout << "alloc " << type << " " << id << " " << amount
                 << std::endl;
            resEntry[type].push_back({ id, amount });
          }

          bool ended = false;
          while (it->ResourceType != type || it->UnitsNeeded == 0) {
            ++it;
            if (it == resourceGroup.end()) {
              ended = true;
              break;
            }
          }

          if (!ended) {
            std::cout << varName << " did not list expected resources"
                      << std::endl;
            return 1;
          }
        }
        resources.push_back(resEntry);

        ++i;
      } catch (...) {
        std::cout << "Unknown error while processing resources" << std::endl;
        return 1;
      }
    }

    auto unlockResult = lock.Release();
    if (!unlockResult.IsOk()) {
      std::cout << "Could not unlock file" << std::endl;
      return 1;
    }
  } else {
    if (cmSystemTools::GetEnv("CTEST_RESOURCE_GROUP_COUNT")) {
      std::cout << "CTEST_RESOURCE_GROUP_COUNT should not be defined"
                << std::endl;
      return 1;
    }
  }

  std::this_thread::sleep_for(std::chrono::seconds(sleepTime));

  if (argc == 6) {
    if (!cmSystemTools::Touch(logFile + ".lock", true)) {
      std::cout << "Could not create lock file" << std::endl;
      return 1;
    }
    cmFileLock lock;
    auto lockResult =
      lock.Lock(logFile + ".lock", static_cast<unsigned long>(-1));
    if (!lockResult.IsOk()) {
      std::cout << "Could not lock file" << std::endl;
      return 1;
    }
    cmsys::ofstream fout(logFile.c_str(), std::ios::app);
    for (auto const& group : resources) {
      for (auto const& it : group) {
        for (auto const& it2 : it.second) {
          fout << "dealloc " << it.first << " " << it2.Id << " " << it2.Slots
               << std::endl;
        }
      }
    }

    fout << "end " << testName << std::endl;

    auto unlockResult = lock.Release();
    if (!unlockResult.IsOk()) {
      std::cout << "Could not unlock file" << std::endl;
      return 1;
    }
  }

  return 0;
}

static int doVerify(int argc, char const* const* argv)
{
  if (argc < 4 || argc > 5) {
    return usageVerify(argv[0]);
  }
  std::string logFile = argv[2];
  std::string resFile = argv[3];
  std::string testNames;
  if (argc == 5) {
    testNames = argv[4];
  }
  auto testNameList = cmExpandedList(testNames, false);
  std::set<std::string> testNameSet(testNameList.begin(), testNameList.end());

  cmCTestResourceSpec spec;
  if (spec.ReadFromJSONFile(resFile) !=
      cmCTestResourceSpec::ReadFileResult::READ_OK) {
    std::cout << "Could not read resource spec " << resFile << std::endl;
    return 1;
  }

  cmCTestResourceAllocator allocator;
  allocator.InitializeFromResourceSpec(spec);

  cmsys::ifstream fin(logFile.c_str(), std::ios::in);
  if (!fin) {
    std::cout << "Could not open log file " << logFile << std::endl;
    return 1;
  }

  std::string command;
  std::string resourceName;
  std::string resourceId;
  std::string testName;
  unsigned int amount;
  std::set<std::string> inProgressTests;
  std::set<std::string> completedTests;
  try {
    while (fin >> command) {
      if (command == "begin") {
        if (!(fin >> testName)) {
          std::cout << "Could not read begin line" << std::endl;
          return 1;
        }
        if (!testNameSet.count(testName) || inProgressTests.count(testName) ||
            completedTests.count(testName)) {
          std::cout << "Could not begin test" << std::endl;
          return 1;
        }
        inProgressTests.insert(testName);
      } else if (command == "alloc") {
        if (!(fin >> resourceName) || !(fin >> resourceId) ||
            !(fin >> amount)) {
          std::cout << "Could not read alloc line" << std::endl;
          return 1;
        }
        if (!allocator.AllocateResource(resourceName, resourceId, amount)) {
          std::cout << "Could not allocate resources" << std::endl;
          return 1;
        }
      } else if (command == "dealloc") {
        if (!(fin >> resourceName) || !(fin >> resourceId) ||
            !(fin >> amount)) {
          std::cout << "Could not read dealloc line" << std::endl;
          return 1;
        }
        if (!allocator.DeallocateResource(resourceName, resourceId, amount)) {
          std::cout << "Could not deallocate resources" << std::endl;
          return 1;
        }
      } else if (command == "end") {
        if (!(fin >> testName)) {
          std::cout << "Could not read end line" << std::endl;
          return 1;
        }
        if (!inProgressTests.erase(testName)) {
          std::cout << "Could not end test" << std::endl;
          return 1;
        }
        if (!completedTests.insert(testName).second) {
          std::cout << "Could not end test" << std::endl;
          return 1;
        }
      }
    }
  } catch (...) {
    std::cout << "Unknown error while reading log file" << std::endl;
    return 1;
  }

  auto const& avail = allocator.GetResources();
  for (auto const& it : avail) {
    for (auto const& it2 : it.second) {
      if (it2.second.Locked != 0) {
        std::cout << "Resource was not unlocked" << std::endl;
        return 1;
      }
    }
  }

  if (completedTests != testNameSet) {
    std::cout << "Tests were not ended" << std::endl;
    return 1;
  }

  return 0;
}

int main(int argc, char const* const* argv)
{
  cmsys::Encoding::CommandLineArguments args =
    cmsys::Encoding::CommandLineArguments::Main(argc, argv);
  argc = args.argc();
  argv = args.argv();

  if (argc < 2) {
    return usage(argv[0]);
  }

  std::string argv1 = argv[1];
  if (argv1 == "write") {
    return doWrite(argc, argv);
  }
  if (argv1 == "verify") {
    return doVerify(argc, argv);
  }
  return usage(argv[0]);
}
