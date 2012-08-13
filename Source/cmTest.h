/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmTest_h
#define cmTest_h

#include "cmCustomCommand.h"
#include "cmPropertyMap.h"
class cmMakefile;
class cmListFileBacktrace;

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

  ///! Set the test name
  void SetName(const char* name);
  const char* GetName() const { return this->Name.c_str(); }

  void SetCommand(std::vector<std::string> const& command);
  std::vector<std::string> const& GetCommand() const
    {
    return this->Command;
    }

  /**
   * Print the structure to std::cout.
   */
  void Print() const;

  ///! Set/Get a property of this source file
  void SetProperty(const char *prop, const char *value);
  void AppendProperty(const char* prop, const char* value,bool asString=false);
  const char *GetProperty(const char *prop) const;
  bool GetPropertyAsBool(const char *prop) const;
  cmPropertyMap &GetProperties() { return this->Properties; };

  // Define the properties
  static void DefineProperties(cmake *cm);

  /** Get the cmMakefile instance that owns this test.  */
  cmMakefile *GetMakefile() { return this->Makefile;};

  /** Get the backtrace of the command that created this test.  */
  cmListFileBacktrace const& GetBacktrace() const;

  /** Get/Set whether this is an old-style test.  */
  bool GetOldStyle() const { return this->OldStyle; }
  void SetOldStyle(bool b) { this->OldStyle = b; }

private:
  cmPropertyMap Properties;
  cmStdString Name;
  std::vector<std::string> Command;

  bool OldStyle;

  cmMakefile* Makefile;
  cmListFileBacktrace* Backtrace;
};

#endif

