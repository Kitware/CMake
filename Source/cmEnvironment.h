/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <string>
#include <vector>

#include <cm/optional>

/**
 * Helper class to represent an environment.
 */
class cmEnvironment
{
public:
  /** Initialize from `NAME=VALUE` strings. */
  cmEnvironment(std::vector<std::string> const& env = {});

  /**
   * Add a single variable (or remove if no = sign) to the current
   * environment.
   */
  void PutEnv(std::string const& env);

  /** Remove a single variable from the current environment. */
  void UnPutEnv(std::string const& env);

  /**
   * Move the environment entries from `other` into this one, overwriting
   * existing variables when a key is already present. */
  void Update(cmEnvironment&& other);

  /** Modify the value of a variable in-place using a transform function. */
  template <typename F>
  void Modify(std::string const& name, F&& transform)
  {
    cm::optional<std::string>& val = this->Map[name];
    if (!val.has_value()) {
      val = std::string{};
    }
    transform(*val);
  }

  std::vector<std::string> GetVariables() const;

  std::string RecordDifference(cmEnvironment const& original) const;

protected:
  struct EnvNameLess
  {
    bool operator()(std::string const& lhs, std::string const& rhs) const;
  };
  std::map<std::string, cm::optional<std::string>, EnvNameLess> Map;
};

class cmEnvironmentModification
{
public:
  bool Add(std::string const& envmod);
  bool Add(std::vector<std::string> const& envmod);

  void ApplyTo(cmEnvironment& env);

private:
  struct Entry
  {
    std::string Name;
    std::string Value;
    std::string Op;
  };

  std::vector<Entry> Entries;
};
