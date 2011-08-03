/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmTestGenerator.h"

#include "cmGeneratorExpression.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTest.h"

//----------------------------------------------------------------------------
cmTestGenerator
::cmTestGenerator(cmTest* test,
                  std::vector<std::string> const& configurations):
  cmScriptGenerator("CTEST_CONFIGURATION_TYPE", configurations),
  Test(test)
{
  this->ActionsPerConfig = !test->GetOldStyle();
  this->TestGenerated = false;
}

//----------------------------------------------------------------------------
cmTestGenerator
::~cmTestGenerator()
{
}

//----------------------------------------------------------------------------
void cmTestGenerator::GenerateScriptConfigs(std::ostream& os,
                                            Indent const& indent)
{
  // First create the tests.
  this->cmScriptGenerator::GenerateScriptConfigs(os, indent);

  // Now generate the test properties.
  if(this->TestGenerated)
    {
    cmTest* test = this->Test;
    cmMakefile* mf = test->GetMakefile();
    cmLocalGenerator* lg = mf->GetLocalGenerator();
    std::ostream& fout = os;
    cmPropertyMap::const_iterator pit;
    cmPropertyMap* mpit = &test->GetProperties();
    if ( mpit->size() )
      {
      fout << "SET_TESTS_PROPERTIES(" << test->GetName() << " PROPERTIES ";
      for ( pit = mpit->begin(); pit != mpit->end(); ++ pit )
        {
        fout << " " << pit->first
             << " " << lg->EscapeForCMake(pit->second.GetValue());
        }
      fout << ")" << std::endl;
      }
    }
}

//----------------------------------------------------------------------------
void cmTestGenerator::GenerateScriptActions(std::ostream& os,
                                            Indent const& indent)
{
  if(this->ActionsPerConfig)
    {
    // This is the per-config generation in a single-configuration
    // build generator case.  The superclass will call our per-config
    // method.
    this->cmScriptGenerator::GenerateScriptActions(os, indent);
    }
  else
    {
    // This is an old-style test, so there is only one config.
    //assert(this->Test->GetOldStyle());
    this->GenerateOldStyle(os, indent);
    }
}

//----------------------------------------------------------------------------
void cmTestGenerator::GenerateScriptForConfig(std::ostream& os,
                                              const char* config,
                                              Indent const& indent)
{
  this->TestGenerated = true;

  // Set up generator expression evaluation context.
  cmMakefile* mf = this->Test->GetMakefile();
  cmGeneratorExpression ge(mf, config, this->Test->GetBacktrace());

  // Start the test command.
  os << indent << "ADD_TEST(" << this->Test->GetName() << " ";

  // Get the test command line to be executed.
  std::vector<std::string> const& command = this->Test->GetCommand();

  // Check whether the command executable is a target whose name is to
  // be translated.
  std::string exe = command[0];
  cmTarget* target = mf->FindTargetToUse(exe.c_str());
  if(target && target->GetType() == cmTarget::EXECUTABLE)
    {
    // Use the target file on disk.
    exe = target->GetFullPath(config);
    }
  else
    {
    // Use the command name given.
    exe = ge.Process(exe.c_str());
    cmSystemTools::ConvertToUnixSlashes(exe);
    }

  // Generate the command line with full escapes.
  cmLocalGenerator* lg = mf->GetLocalGenerator();
  os << lg->EscapeForCMake(exe.c_str());
  for(std::vector<std::string>::const_iterator ci = command.begin()+1;
      ci != command.end(); ++ci)
    {
    os << " " << lg->EscapeForCMake(ge.Process(*ci));
    }

  // Finish the test command.
  os << ")\n";
}

//----------------------------------------------------------------------------
void cmTestGenerator::GenerateScriptNoConfig(std::ostream& os,
                                             Indent const& indent)
{
  os << indent << "ADD_TEST(" << this->Test->GetName() << " NOT_AVAILABLE)\n";
}

//----------------------------------------------------------------------------
bool cmTestGenerator::NeedsScriptNoConfig() const
{
  return (this->TestGenerated && // test generated for at least one config
          this->ActionsPerConfig && // test is config-aware
          this->Configurations.empty() && // test runs in all configs
          !this->ConfigurationTypes->empty()); // config-dependent command
}

//----------------------------------------------------------------------------
void cmTestGenerator::GenerateOldStyle(std::ostream& fout,
                                       Indent const& indent)
{
  this->TestGenerated = true;

  // Get the test command line to be executed.
  std::vector<std::string> const& command = this->Test->GetCommand();

  std::string exe = command[0];
  cmSystemTools::ConvertToUnixSlashes(exe);
  fout << indent;
  fout << "ADD_TEST(";
  fout << this->Test->GetName() << " \"" << exe << "\"";

  for(std::vector<std::string>::const_iterator argit = command.begin()+1;
      argit != command.end(); ++argit)
    {
    // Just double-quote all arguments so they are re-parsed
    // correctly by the test system.
    fout << " \"";
    for(std::string::const_iterator c = argit->begin();
        c != argit->end(); ++c)
      {
      // Escape quotes within arguments.  We should escape
      // backslashes too but we cannot because it makes the result
      // inconsistent with previous behavior of this command.
      if((*c == '"'))
        {
        fout << '\\';
        }
      fout << *c;
      }
    fout << "\"";
    }
  fout << ")" << std::endl;
}
