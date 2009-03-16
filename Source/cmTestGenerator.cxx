/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmTestGenerator.h"

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
    std::ostream& fout = os;
    cmPropertyMap::const_iterator pit;
    cmPropertyMap* mpit = &test->GetProperties();
    if ( mpit->size() )
      {
      fout << "SET_TESTS_PROPERTIES(" << test->GetName() << " PROPERTIES ";
      for ( pit = mpit->begin(); pit != mpit->end(); ++ pit )
        {
        fout << " " << pit->first.c_str() << " \"";
        const char* value = pit->second.GetValue();
        for ( ; *value; ++ value )
          {
          switch ( *value )
            {
            case '\\':
            case '"':
            case ' ':
            case '#':
            case '(':
            case ')':
            case '$':
            case '^':
              fout << "\\" << *value;
              break;
            case '\t':
              fout << "\\t";
              break;
            case '\n':
              fout << "\\n";
              break;
            case '\r':
              fout << "\\r";
              break;
            default:
              fout << *value;
            }
          }
        fout << "\"";
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

  // Start the test command.
  os << indent << "ADD_TEST(" << this->Test->GetName() << " ";

  // Get the test command line to be executed.
  std::vector<std::string> const& command = this->Test->GetCommand();

  // Check whether the command executable is a target whose name is to
  // be translated.
  std::string exe = command[0];
  cmMakefile* mf = this->Test->GetMakefile();
  cmTarget* target = mf->FindTargetToUse(exe.c_str());
  if(target && target->GetType() == cmTarget::EXECUTABLE)
    {
    // Use the target file on disk.
    exe = target->GetFullPath(config);
    }
  else
    {
    // Use the command name given.
    cmSystemTools::ConvertToUnixSlashes(exe);
    }

  // Generate the command line with full escapes.
  cmLocalGenerator* lg = mf->GetLocalGenerator();
  os << lg->EscapeForCMake(exe.c_str());
  for(std::vector<std::string>::const_iterator ci = command.begin()+1;
      ci != command.end(); ++ci)
    {
    os << " " << lg->EscapeForCMake(ci->c_str());
    }

  // Finish the test command.
  os << ")\n";
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
