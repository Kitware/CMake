/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "cmFileAPI.h"

namespace {

// Identity oracle over a name->id map.  A name absent from the map models a
// file whose identity cannot be obtained (GetFileId failure).
std::function<bool(std::string const&, int&)> makeOracle(
  std::map<std::string, int> const& ids)
{
  return [ids](std::string const& name, int& id) -> bool {
    auto const it = ids.find(name);
    if (it == ids.end()) {
      return false;
    }
    id = it->second;
    return true;
  };
}

bool checkCase(char const* label, std::vector<std::string> const& entries,
               std::unordered_set<std::string> const& replyNames,
               std::map<std::string, int> const& ids,
               std::set<std::string> const& expected)
{
  std::vector<std::string> const removed =
    cmFileAPI::FilesToRemove<int>(entries, replyNames, makeOracle(ids));
  std::set<std::string> const actual(removed.begin(), removed.end());
  if (actual != expected) {
    std::cout << "FAILED: " << label << "\n  expected removals:";
    for (std::string const& e : expected) {
      std::cout << ' ' << e;
    }
    std::cout << "\n  actual removals:  ";
    for (std::string const& a : actual) {
      std::cout << ' ' << a;
    }
    std::cout << '\n';
    return false;
  }
  return true;
}

} // namespace

int testCMFileAPI(int /*unused*/, char* /*unused*/[])
{
  bool ok = true;

  // An on-disk entry that resolves to the same identity as a just-written
  // reply (a case-only-different name aliasing one inode) is kept.
  ok &= checkCase(
    "case-variant alias kept", { "target-foo-Debug-H.json" },
    { "target-foo-debug-H.json" },
    { { "target-foo-debug-H.json", 1 }, { "target-foo-Debug-H.json", 1 } },
    {});

  // A genuinely stale entry with a distinct identity is removed.
  ok &= checkCase("distinct stale removed", { "target-old-H.json" },
                  { "target-new-H.json" },
                  { { "target-new-H.json", 1 }, { "target-old-H.json", 2 } },
                  { "target-old-H.json" });

  // An entry whose exact name was just written is kept without consulting the
  // identity oracle (its name is intentionally absent from the id map).
  ok &= checkCase("exact-name match kept", { "index-x.json" },
                  { "index-x.json" }, {}, {});

  // Fail-safe: an entry whose identity cannot be obtained is retained.
  ok &= checkCase("candidate id failure retained", { "unreadable.json" },
                  { "target-new-H.json" }, { { "target-new-H.json", 1 } }, {});

  // No kept-side fail-safe is needed: an alias shares its inode, so when a
  // kept reply's identity is unknown the candidate's is too and the
  // candidate-side fail-safe retains it.  Both names are absent to model it.
  ok &= checkCase("unknown kept id: correlated alias retained",
                  { "alias-of-unidentified.json" },
                  { "target-new-H.json", "unidentified-kept.json" },
                  { { "target-new-H.json", 1 } }, {});

  // Mixed: an aliasing entry is kept while an unrelated stale entry is
  // removed.
  ok &= checkCase("mixed alias and stale", { "a-alias.json", "stale.json" },
                  { "a.json", "b.json" },
                  { { "a.json", 1 },
                    { "b.json", 2 },
                    { "a-alias.json", 1 },
                    { "stale.json", 3 } },
                  { "stale.json" });

  return ok ? 0 : 1;
}
