/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmCreateTestSourceList.h"


// cmCreateTestSourceList
bool cmCreateTestSourceList::InitialPass(std::vector<std::string> const& argsIn)
{
  if (argsIn.size() < 3)
    {
    this->SetError("called with wrong number of arguments.");
    return false;
    }

  std::vector<std::string> args;
  cmSystemTools::ExpandListArguments(argsIn, args, true);
  
  std::vector<std::string>::iterator i = args.begin();

  // Name of the source list

  const char* sourceList = i->c_str();
  ++i;

  // Name of the test driver

  std::string driver = m_Makefile->GetCurrentOutputDirectory();
  driver += "/";
  driver += *i;
  driver += ".cxx";
  ++i;

  std::ofstream fout(driver.c_str());
  if (!fout)
    {
    std::string err = "Could not create file ";
    err += driver;
    err += " for cmCreateTestSourceList command.";
    this->SetError(err.c_str());
    return false;
    }

  // Create the test driver file

  fout << 
    "#include <stdio.h>\n"
    "#include <string.h>\n"
    "\n"
    "// Forward declare test functions\n"
    "\n";

  std::vector<std::string>::iterator testsBegin = i;
  std::vector<std::string> tests_func_name;

  // The rest of the arguments consist of a list of test source files.
  // Sadly, they can be in directories. Let's find a unique function 
  // name for the corresponding test, and push it to the tests_func_name
  // list. 
  // For the moment:
  //   - replace spaces ' ', ':' and '/' with underscores '_'

  for(i = testsBegin; i != args.end(); ++i)
    {
    std::string func_name = *i;
    cmSystemTools::ConvertToUnixSlashes(func_name);
    cmSystemTools::ReplaceString(func_name, " ", "_");
    cmSystemTools::ReplaceString(func_name, "/", "_");
    cmSystemTools::ReplaceString(func_name, ":", "_");
    tests_func_name.push_back(func_name);
    fout << "int " << func_name << "(int, char**);\n";
    }

  fout << 
    "\n"
    "// Create map\n"
    "\n"
    "typedef int (*MainFuncPointer)(int , char**);\n"
    "struct functionMapEntry\n"
    "{\n"
    "  const char* name;\n"
    "  MainFuncPointer func;\n"
    "};\n"
    "\n"
    "functionMapEntry cmakeGeneratedFunctionMapEntries[] = {\n";

  int numTests = 0;
  std::vector<std::string>::iterator j;
  for(i = testsBegin, j = tests_func_name.begin(); i != args.end(); ++i, ++j)
    {
    fout << 
      "  {\n"
      "    \"" << *i << "\",\n"
      "    " << *j << "\n"
      "  },\n";
    numTests++;
    }

  fout << 
    "};\n"
    "\n"
    "int main(int ac, char** av)\n"
    "{\n"
    "  int NumTests = " << numTests << ";\n"
    "  int i;\n"
    "  \n"
    "  // If no test name was given\n"
    "  if (ac < 2)\n"
    "    {\n"
    "    // If there is only one test, then run it with the arguments\n"
    "    if (NumTests == 1)\n"
    "      {\n"
    "      return (*cmakeGeneratedFunctionMapEntries[0].func)(ac, av);\n"
    "      }\n"
    "    \n"
    "    // Ask for a test\n"
    "    printf(\"Available tests:\\n\");\n"
    "    for (i =0; i < NumTests; ++i)\n"
    "      {\n"
    "      printf(\"%d. %s\\n\", i, cmakeGeneratedFunctionMapEntries[i].name);\n"
    "      }\n"
    "    printf(\"To run a test, enter the test number: \");\n"
    "    int testNum = 0;\n"
    "    scanf(\"%d\", &testNum);\n"
    "    if (testNum >= NumTests)\n"
    "      {\n"
    "      printf(\"%d is an invalid test number.\\n\", testNum);\n"
    "      return -1;\n"
    "      }\n"
    "    return (*cmakeGeneratedFunctionMapEntries[testNum].func)(ac-1, av+1);\n"
    "    }\n"
    "  \n"
    "  // If partial match is requested\n"
    "  int partial_match = (strcmp(av[1], \"-R\") == 0) ? 1 : 0;\n"
    "  if (partial_match && ac < 3)\n"
    "    {\n"
    "    printf(\"-R needs an additional parameter.\\n\");\n"
    "    return -1;\n"
    "    }\n"
    "  \n"
    "  // A test name was given, try to find it\n"
    "  for (i =0; i < NumTests; ++i)\n"
    "    {\n"
    "    if (partial_match && \n"
    "        strstr(cmakeGeneratedFunctionMapEntries[i].name, av[2]) != NULL)\n"
    "      {\n"
    "      return (*cmakeGeneratedFunctionMapEntries[i].func)(ac-2, av+2);\n"
    "      }\n"
    "    else if (!partial_match && \n"
    "             strcmp(cmakeGeneratedFunctionMapEntries[i].name, av[1]) == 0)\n"
    "      {\n"
    "      return (*cmakeGeneratedFunctionMapEntries[i].func)(ac-1, av+1);\n"
    "      }\n"
    "    }\n"
    "  \n"
    "  // If the test was not found but there is only one test, then\n"
    "  // run it with the arguments\n"
    "  if (NumTests == 1)\n"
    "    {\n"
    "    return (*cmakeGeneratedFunctionMapEntries[0].func)(ac, av);\n"
    "    }\n"
    "  \n"
    "  // Nothing was run, display the test names\n"
    "  printf(\"Available tests:\\n\");\n"
    "  for (i =0; i < NumTests; ++i)\n"
    "    {\n"
    "    printf(\"%d. %s\\n\", i, cmakeGeneratedFunctionMapEntries[i].name);\n"
    "    }\n"
    "  printf(\"Failed: %s is an invalid test name.\\n\", av[1]);\n"
    "  \n"
    "  return -1;\n"
    "}\n";

  fout.close();

  // Create the source list

  cmSourceFile cfile;
  cfile.SetIsAnAbstractClass(false);
  cfile.SetName(args[1].c_str(), 
                m_Makefile->GetCurrentOutputDirectory(),
                "cxx", 
                false);
  m_Makefile->AddSource(cfile, sourceList);
  
  for(i = testsBegin; i != args.end(); ++i)
    {
    cmSourceFile cfile;
    cfile.SetIsAnAbstractClass(false);
    cfile.SetName(i->c_str(), 
                  m_Makefile->GetCurrentDirectory(),
                  "cxx", 
                  false);
    m_Makefile->AddSource(cfile, sourceList);
    }

  return true;
}



