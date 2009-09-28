/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
// Program extracts documentation describing commands from
// the CMake system.
// 
#include "cmake.h"

#include "cmDocumentation.h"
#include "cmVersion.h"

//----------------------------------------------------------------------------
static const char *cmDocumentationName[][3] =
{
  {0,
   "  DumpDocumentation - Dump documentation for CMake.", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char *cmDocumentationUsage[][3] =
{
  {0,
   "  DumpDocumentation [filename]", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char *cmDocumentationDescription[][3] =
{
  {0,
   "The \"DumpDocumentation\" executable is only available in the build "
   "tree.  It is used for testing, coverage, and documentation.", 0},
  CMAKE_STANDARD_INTRODUCTION,
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char *cmDocumentationOptions[][3] =
{
  {"--all-for-coverage", 
   "Dump all documentation to stdout.  For testing.", 0},
  {0,0,0}
};


int DumpHTML(const char* outname)
{
  std::ofstream fout(outname);
  if(!fout)
    {
    std::cerr << "failed to open output file: " << outname << "\n";  
    cmSystemTools::ReportLastSystemError("");
    return -1;
    }

  cmake cmi;
  cmDocumentation doc;
  std::vector<cmDocumentationEntry> commands;
  cmi.GetCommandDocumentation(commands);
  cmOStringStream str;
  str << "Documentation for Commands of CMake " 
    << cmVersion::GetCMakeVersion();
  doc.SetSection(str.str().c_str(), commands);
  doc.Print(cmDocumentation::HTMLForm, fout);
  
  return 0;
}

int DumpForCoverageToStream(std::ostream& out)
{
  cmake cmi;
  cmDocumentation doc;
  std::vector<cmDocumentationEntry> commands;
  std::vector<cmDocumentationEntry> generators;
  cmi.GetCommandDocumentation(commands);
  cmi.GetGeneratorDocumentation(generators);
  doc.SetSection("Name",cmDocumentationName);
  doc.SetSection("Usage",cmDocumentationUsage);
  doc.SetSection("Description",cmDocumentationDescription);
  doc.SetSection("options",cmDocumentationOptions);
  doc.SetSection("Commands",commands);
  doc.SetSection("Generators",generators);
  doc.PrintDocumentation(cmDocumentation::Usage, out);
  doc.PrintDocumentation(cmDocumentation::Full, out);
  return 0;
}

int DumpForCoverage(const char* outname)
{
  if(outname)
    {
    std::ofstream fout(outname);
    if(!fout)
      {
      std::cerr << "failed to open output file: " << outname << "\n";
      cmSystemTools::ReportLastSystemError("");
      return -1;
      }
    return DumpForCoverageToStream(fout);
    }
  else
    {
    return DumpForCoverageToStream(std::cout);
    }
}

int main(int ac, char** av)
{
  cmSystemTools::EnableMSVCDebugHook();
  cmSystemTools::FindExecutableDirectory(av[0]);
  const char* outname = "cmake.html";
  bool coverage = false;
  if(ac > 1)
    {
    if(strcmp(av[1], "--all-for-coverage") == 0)
      {
      coverage = true;
      if(ac > 2)
        {
        outname = av[2];
        }
      else
        {
        outname = 0;
        }
      }
    else
      {
      outname = av[1];
      }
    }
  
  if(coverage)
    {
    return DumpForCoverage(outname);
    }
  else
    {
    return DumpHTML(outname);
    }
}
