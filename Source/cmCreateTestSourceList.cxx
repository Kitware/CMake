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
#include "cmCreateTestSourceList.h"
#include "cmSourceFile.h"

// cmCreateTestSourceList
bool cmCreateTestSourceList::InitialPass(std::vector<std::string> const& args)
{
  if (args.size() < 3)
    {
    this->SetError("called with wrong number of arguments.");
    return false;
    }

  
  std::vector<std::string>::const_iterator i = args.begin();
  std::string extraInclude;
  std::string function;
  std::vector<std::string> tests;
  // extract extra include and function ot
  for(; i != args.end(); i++)
    {
    if(*i == "EXTRA_INCLUDE")
      {
      ++i;
      if(i == args.end())
        {
        this->SetError("incorrect arguments to EXTRA_INCLUDE");
        return false;
        }
      extraInclude = "#include \"";
      extraInclude += *i;
      extraInclude += "\"\n";
      }
    else if(*i == "FUNCTION")
      {
      ++i;
      if(i == args.end())
        {
        this->SetError("incorrect arguments to FUNCTION");
        return false;
        }
      function = *i;
      function += "(&ac, &av);\n";
      }
    else
      {
      tests.push_back(*i);
      }
    }
  i = tests.begin();
  
  // Name of the source list

  const char* sourceList = i->c_str();
  ++i;

  // Name of the test driver
  // make sure they specified an extension
  if (cmSystemTools::GetFilenameExtension(*i).size() < 2)
    {
    this->SetError("You must specify a file extenion for the test driver file.");
    return false;
    }
  std::string driver = m_Makefile->GetCurrentOutputDirectory();
  driver += "/";
  driver += *i;
  ++i;

  std::string configFile = 
    m_Makefile->GetRequiredDefinition("CMAKE_ROOT");
  configFile += "/Templates/TestDriver.cxx.in";
  // Create the test driver file
  
  std::vector<std::string>::const_iterator testsBegin = i;
  std::vector<std::string> tests_func_name;

  // The rest of the arguments consist of a list of test source files.
  // Sadly, they can be in directories. Let's find a unique function 
  // name for the corresponding test, and push it to the tests_func_name
  // list. 
  // For the moment:
  //   - replace spaces ' ', ':' and '/' with underscores '_'
  std::string forwardDeclareCode;
  for(i = testsBegin; i != tests.end(); ++i)
    {
    if(*i == "EXTRA_INCLUDE")
      {
      break;
      }
    std::string func_name;
    if (cmSystemTools::GetFilenamePath(*i).size() > 0)
      {
      func_name = cmSystemTools::GetFilenamePath(*i) + "/" + 
        cmSystemTools::GetFilenameWithoutLastExtension(*i);
      }
    else
      {
      func_name = cmSystemTools::GetFilenameWithoutLastExtension(*i);
      }
    cmSystemTools::ConvertToUnixSlashes(func_name);
    cmSystemTools::ReplaceString(func_name, " ", "_");
    cmSystemTools::ReplaceString(func_name, "/", "_");
    cmSystemTools::ReplaceString(func_name, ":", "_");
    tests_func_name.push_back(func_name);
    forwardDeclareCode += "int ";
    forwardDeclareCode += func_name;
    forwardDeclareCode += "(int, char*[]);\n";
    }
  
  std::string functionMapCode;
  int numTests = 0;
  std::vector<std::string>::iterator j;
  for(i = testsBegin, j = tests_func_name.begin(); i != tests.end(); ++i, ++j)
    {
    std::string func_name;
    if (cmSystemTools::GetFilenamePath(*i).size() > 0)
      {
      func_name = cmSystemTools::GetFilenamePath(*i) + "/" + 
        cmSystemTools::GetFilenameWithoutLastExtension(*i);
      }
    else
      {
      func_name = cmSystemTools::GetFilenameWithoutLastExtension(*i);
      }
    functionMapCode += "  {\n"
      "    \"";
    functionMapCode += func_name;
    functionMapCode += "\",\n"
      "    ";
    functionMapCode +=  *j;
    functionMapCode += "\n"
      "  },\n";
    numTests++;
    }
  if(extraInclude.size())
    {
    m_Makefile->AddDefinition("CMAKE_TESTDRIVER_EXTRA_INCLUDES", extraInclude.c_str());
    }
  if(function.size())
    {
    m_Makefile->AddDefinition("CMAKE_TESTDRIVER_ARGVC_FUNCTION", function.c_str());
    }
  m_Makefile->AddDefinition("CMAKE_FORWARD_DECLARE_TESTS", forwardDeclareCode.c_str());
  m_Makefile->AddDefinition("CMAKE_FUNCTION_TABLE_ENTIRES", functionMapCode.c_str());
  bool res = true;
  if ( !m_Makefile->ConfigureFile(configFile.c_str(), driver.c_str(), false, true, false) )
    {
    res = false;
    }

  // Create the source list
  cmSourceFile cfile;
  std::string sourceListValue;
  
  cfile.SetProperty("ABSTRACT","0");
  cfile.SetName(cmSystemTools::GetFilenameWithoutExtension(args[1]).c_str(), 
                m_Makefile->GetCurrentOutputDirectory(),
                cmSystemTools::GetFilenameExtension(args[1]).c_str()+1, 
                false);
  m_Makefile->AddSource(cfile);
  sourceListValue = args[1];
    
  for(i = testsBegin; i != tests.end(); ++i)
    {
    cmSourceFile icfile;
    icfile.SetProperty("ABSTRACT","0");
    icfile.SetName(i->c_str(), 
                  m_Makefile->GetCurrentDirectory(),
                  m_Makefile->GetSourceExtensions(), 
                  m_Makefile->GetHeaderExtensions());
    m_Makefile->AddSource(icfile);
    sourceListValue += ";";
    sourceListValue += *i;
    }

  m_Makefile->AddDefinition(sourceList, sourceListValue.c_str());
  return res;
}



