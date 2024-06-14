/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <string>
#include <vector>

#include "cmListFileCache.h"
#include "cmPolicies.h"
#include "cmPropertyMap.h"
#include "cmValue.h"

class cmMakefile;

/** \class cmTest
 * \brief Represent a test
 *
 * cmTest is representation of a test.
 */
class cmTest
{
public:
  /**
   */
  cmTest(cmMakefile* mf);
  ~cmTest();

  //! Set the test name
  void SetName(const std::string& name);
  std::string GetName() const { return this->Name; }

  void SetCommand(std::vector<std::string> const& command);
  std::vector<std::string> const& GetCommand() const { return this->Command; }

  //! Set/Get a property of this source file
  void SetProperty(const std::string& prop, cmValue value);
  void SetProperty(const std::string& prop, std::nullptr_t)
  {
    this->SetProperty(prop, cmValue{ nullptr });
  }
  void SetProperty(const std::string& prop, const std::string& value)
  {
    this->SetProperty(prop, cmValue(value));
  }
  void AppendProperty(const std::string& prop, const std::string& value,
                      bool asString = false);
  cmValue GetProperty(const std::string& prop) const;
  bool GetPropertyAsBool(const std::string& prop) const;
  cmPropertyMap& GetProperties() { return this->Properties; }

  /** Get the cmMakefile instance that owns this test.  */
  cmMakefile* GetMakefile() { return this->Makefile; }

  /** Get the backtrace of the command that created this test.  */
  cmListFileBacktrace const& GetBacktrace() const;

  /** Get/Set whether this is an old-style test.  */
  bool GetOldStyle() const { return this->OldStyle; }
  void SetOldStyle(bool b) { this->OldStyle = b; }

  /** Get/Set if CMP0158 policy is NEW */
  bool GetCMP0158IsNew() const
  {
    return this->PolicyStatusCMP0158 == cmPolicies::NEW;
  }

  /** Set/Get whether lists in command lines should be expanded. */
  bool GetCommandExpandLists() const;
  void SetCommandExpandLists(bool b);

private:
  cmPropertyMap Properties;
  std::string Name;
  std::vector<std::string> Command;
  bool CommandExpandLists = false;

  bool OldStyle;

  cmMakefile* Makefile;
  cmListFileBacktrace Backtrace;
  cmPolicies::PolicyStatus PolicyStatusCMP0158;
};
