/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmDocumentation.h"

#include "cmSystemTools.h"
#include "cmVersion.h"
#include <cmsys/Directory.hxx>
#include <cmsys/Glob.hxx>

#include <algorithm>

//----------------------------------------------------------------------------
static const char *cmDocumentationStandardOptions[][3] =
{
  {"--copyright [file]", "Print the CMake copyright and exit.",
   "If a file is specified, the copyright is written into it."},
  {"--help,-help,-usage,-h,-H,/?", "Print usage information and exit.",
   "Usage describes the basic command line interface and its options."},
  {"--help-full [file]", "Print full help and exit.",
   "Full help displays most of the documentation provided by the UNIX "
   "man page.  It is provided for use on non-UNIX platforms, but is "
   "also convenient if the man page is not installed.  If a file is "
   "specified, the help is written into it."},
  {"--help-html [file]", "Print full help in HTML format.",
   "This option is used by CMake authors to help produce web pages.  "
   "If a file is specified, the help is written into it."},
  {"--help-man [file]", "Print full help as a UNIX man page and exit.",
   "This option is used by the cmake build to generate the UNIX man page.  "
   "If a file is specified, the help is written into it."},
  {"--version,-version,/V [file]",
   "Show program name/version banner and exit.",
   "If a file is specified, the version is written into it."},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char *cmModulesDocumentationDescription[][3] =
{
  {0,
  "  CMake Modules - Modules coming with CMake, the Cross-Platform Makefile "
  "Generator.", 0},
//  CMAKE_DOCUMENTATION_OVERVIEW,
  {0,
  "This is the documentation for the modules and scripts coming with CMake. "
  "Using these modules you can check the computer system for "
  "installed software packages, features of the compiler and the "
  "existence of headers to name just a few.", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char *cmCustomModulesDocumentationDescription[][3] =
{
  {0,
  "  Custom CMake Modules - Additional Modules for CMake.", 0},
//  CMAKE_DOCUMENTATION_OVERVIEW,
  {0,
  "This is the documentation for additional modules and scripts for CMake. "
  "Using these modules you can check the computer system for "
  "installed software packages, features of the compiler and the "
  "existence of headers to name just a few.", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char *cmPropertiesDocumentationDescription[][3] =
{
  {0,
   "  CMake Properties - Properties supported by CMake, "
   "the Cross-Platform Makefile Generator.", 0},
//  CMAKE_DOCUMENTATION_OVERVIEW,
  {0,
   "This is the documentation for the properties supported by CMake. "
   "Properties can have different scopes. They can either be assigned to a "
   "source file, a directory, a target or globally to CMake. By modifying the "
   "values of properties the behaviour of the build system can be customized.",
   0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char *cmCompatCommandsDocumentationDescription[][3] =
{
  {0,
   "  CMake Compatibility Listfile Commands - "
   "Obsolete commands supported by CMake for compatibility.", 0},
//  CMAKE_DOCUMENTATION_OVERVIEW,
  {0,
  "This is the documentation for now obsolete listfile commands from previous "
  "CMake versions, which are still supported for compatibility reasons. You "
  "should instead use the newer, faster and shinier new commands. ;-)", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char *cmDocumentationModulesHeader[][3] =
{
  {0,
   "The following modules are provided with CMake. "
   "They can be used with INCLUDE(ModuleName).", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char *cmDocumentationCustomModulesHeader[][3] =
{
  {0,
   "The following modules are also available for CMake. "
   "They can be used with INCLUDE(ModuleName).", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char *cmDocumentationGeneratorsHeader[][3] =
{
  {0,
   "The following generators are available on this platform:", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char *cmDocumentationStandardSeeAlso[][3] =
{
  {0,
   "The following resources are available to get help using CMake:", 0},
  {"Home Page",
   "http://www.cmake.org",
   "The primary starting point for learning about CMake."},
  {"Frequently Asked Questions",
   "http://www.cmake.org/Wiki/CMake_FAQ",
   "A Wiki is provided containing answers to frequently asked questions. "},
  {"Online Documentation",
   "http://www.cmake.org/HTML/Documentation.html",
   "Links to available documentation may be found on this web page."},
  {"Mailing List",
   "http://www.cmake.org/HTML/MailingLists.html",
   "For help and discussion about using cmake, a mailing list is provided at "
   "cmake@cmake.org. "
   "The list is member-post-only but one may sign up on the CMake web page. "
   "Please first read the full documentation at "
   "http://www.cmake.org before posting questions to the list."},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char *cmDocumentationCopyright[][3] =
{
  {0,
   "Copyright 2000-2012 Kitware, Inc., Insight Software Consortium.  "
   "All rights reserved.", 0},
  {0,
   "Redistribution and use in source and binary forms, with or without "
   "modification, are permitted provided that the following conditions are "
   "met:", 0},
  {"",
   "Redistributions of source code must retain the above copyright notice, "
   "this list of conditions and the following disclaimer.", 0},
  {"",
   "Redistributions in binary form must reproduce the above copyright "
   "notice, this list of conditions and the following disclaimer in the "
   "documentation and/or other materials provided with the distribution.",
   0},
  {"",
   "Neither the names of Kitware, Inc., the Insight Software Consortium, "
   "nor the names of their contributors may be used to endorse or promote "
   "products derived from this software without specific prior written "
   "permission.", 0},
  {0,
   "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "
   "\"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT "
   "LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR "
   "A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT "
   "HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, "
   "SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT "
   "LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, "
   "DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY "
   "THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT "
   "(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE "
   "OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.",
   0},
  {0, 0, 0}
};

//----------------------------------------------------------------------------
#define DOCUMENT_INTRO(type, default_name, desc) \
  static char const *cmDocumentation##type##Intro[2] = { default_name, desc };
#define GET_DOCUMENT_INTRO(type) cmDocumentation##type##Intro

DOCUMENT_INTRO(Modules, "cmakemodules",
  "Reference of available CMake modules.");
DOCUMENT_INTRO(CustomModules, "cmakecustommodules",
  "Reference of available CMake custom modules.");
DOCUMENT_INTRO(Policies, "cmakepolicies",
  "Reference of CMake policies.");
DOCUMENT_INTRO(Properties, "cmakeprops",
  "Reference of CMake properties.");
DOCUMENT_INTRO(Variables, "cmakevars",
  "Reference of CMake variables.");
DOCUMENT_INTRO(Commands, "cmakecommands",
  "Reference of available CMake commands.");
DOCUMENT_INTRO(CompatCommands, "cmakecompat",
  "Reference of CMake compatibility commands.");

//----------------------------------------------------------------------------
cmDocumentation::cmDocumentation()
:CurrentFormatter(0)
{
  this->SetForm(TextForm, 0);
  this->addCommonStandardDocSections();
  this->ShowGenerators = true;
}

//----------------------------------------------------------------------------
cmDocumentation::~cmDocumentation()
{
  for(std::vector< char* >::iterator i = this->ModuleStrings.begin();
      i != this->ModuleStrings.end(); ++i)
    {
    delete [] *i;
    }
  for(std::map<std::string,cmDocumentationSection *>::iterator i =
        this->AllSections.begin();
      i != this->AllSections.end(); ++i)
    {
    delete i->second;
    }
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintCopyright(std::ostream& os)
{
  cmDocumentationSection *sec = this->AllSections["Copyright"];
  const std::vector<cmDocumentationEntry> &entries = sec->GetEntries();
  for(std::vector<cmDocumentationEntry>::const_iterator op = entries.begin();
      op != entries.end(); ++op)
    {
    if(op->Name.size())
      {
      os << " * ";
      this->TextFormatter.SetIndent("    ");
      this->TextFormatter.PrintColumn(os, op->Brief.c_str());
      }
    else
      {
      this->TextFormatter.SetIndent("");
      this->TextFormatter.PrintColumn(os, op->Brief.c_str());
      }
    os << "\n";
    }
  return true;
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintVersion(std::ostream& os)
{
  os << this->GetNameString() << " version "
     << cmVersion::GetCMakeVersion() << "\n";
  return true;
}

//----------------------------------------------------------------------------
void cmDocumentation::AddSectionToPrint(const char *section)
{
  if (this->AllSections.find(section) != this->AllSections.end())
    {
    this->PrintSections.push_back(this->AllSections[section]);
    }
}

//----------------------------------------------------------------------------
void cmDocumentation::ClearSections()
{
  this->PrintSections.erase(this->PrintSections.begin(),
                            this->PrintSections.end());
  this->ModulesFound.clear();
}

//----------------------------------------------------------------------------
void cmDocumentation::AddDocumentIntroToPrint(const char* intro[2])
{
  const char* docname = this->GetDocName(false);
  if(intro && docname)
    {
    cmDocumentationSection* section;
    std::string desc("");

    desc += docname;
    desc += " - ";
    desc += intro[1];

    section = new cmDocumentationSection("Introduction", "NAME");
    section->Append(0, desc.c_str(), 0);
    this->PrintSections.push_back(section);
    }
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintDocumentation(Type ht, std::ostream& os,
                                         const char* docname)
{
  // Handle Document Name. docname==0 disables intro.
  this->SetDocName("");
  if (docname)
    {
    if (*docname)
      this->SetDocName(docname);
    else // empty string was given. select default if possible
      this->SetDocName(this->GetDefaultDocName(ht));
    }

  switch (ht)
    {
    case cmDocumentation::Usage:
      return this->PrintDocumentationUsage(os);
    case cmDocumentation::Single:
      return this->PrintDocumentationSingle(os);
    case cmDocumentation::SingleModule:
      return this->PrintDocumentationSingleModule(os);
    case cmDocumentation::SinglePolicy:
      return this->PrintDocumentationSinglePolicy(os);
    case cmDocumentation::SingleProperty:
      return this->PrintDocumentationSingleProperty(os);
    case cmDocumentation::SingleVariable:
      return this->PrintDocumentationSingleVariable(os);
    case cmDocumentation::List:
      this->PrintDocumentationList(os,"Commands");
      this->PrintDocumentationList(os,"Compatibility Commands");
      return true;
    case cmDocumentation::ModuleList:
      // find the modules first, print the custom module docs only if
      // any custom modules have been found actually, Alex
      this->CreateCustomModulesSection();
      this->CreateModulesSection();
      if (this->AllSections.find("Custom CMake Modules")
         != this->AllSections.end())
        {
        this->PrintDocumentationList(os,"Custom CMake Modules");
        }
      this->PrintDocumentationList(os,"Modules");
      return true;
    case cmDocumentation::PropertyList:
      this->PrintDocumentationList(os,"Properties Description");
      for (std::vector<std::string>::iterator i =
             this->PropertySections.begin();
           i != this->PropertySections.end(); ++i)
        {
        this->PrintDocumentationList(os,i->c_str());
        }
      return true;
    case cmDocumentation::VariableList:
      for (std::vector<std::string>::iterator i =
             this->VariableSections.begin();
           i != this->VariableSections.end(); ++i)
        {
        this->PrintDocumentationList(os,i->c_str());
        }
      return true;
    case cmDocumentation::PolicyList:
      this->PrintDocumentationList(os,"Policies");
      return true;
    case cmDocumentation::Full:
      return this->PrintDocumentationFull(os);
    case cmDocumentation::Modules:
      return this->PrintDocumentationModules(os);
    case cmDocumentation::CustomModules:
      return this->PrintDocumentationCustomModules(os);
    case cmDocumentation::Policies:
      return this->PrintDocumentationPolicies(os);
    case cmDocumentation::Properties:
      return this->PrintDocumentationProperties(os);
    case cmDocumentation::Variables:
      return this->PrintDocumentationVariables(os);
    case cmDocumentation::Commands:
      return this->PrintDocumentationCurrentCommands(os);
    case cmDocumentation::CompatCommands:
      return this->PrintDocumentationCompatCommands(os);

    case cmDocumentation::Copyright:
      return this->PrintCopyright(os);
    case cmDocumentation::Version:
      return this->PrintVersion(os);
    default: return false;
    }
}

//----------------------------------------------------------------------------
bool cmDocumentation::CreateModulesSection()
{
  cmDocumentationSection *sec =
    new cmDocumentationSection("Standard CMake Modules", "MODULES");
  this->AllSections["Modules"] = sec;
  std::string cmakeModules = this->CMakeRoot;
  cmakeModules += "/Modules";
  cmsys::Directory dir;
  dir.Load(cmakeModules.c_str());
  if (dir.GetNumberOfFiles() > 0)
    {
    sec->Append(cmDocumentationModulesHeader[0]);
    sec->Append(cmModulesDocumentationDescription);
    this->CreateModuleDocsForDir(dir, *this->AllSections["Modules"]);
    }
  return true;
}

//----------------------------------------------------------------------------
bool cmDocumentation::CreateCustomModulesSection()
{
  bool sectionHasHeader = false;

  std::vector<std::string> dirs;
  cmSystemTools::ExpandListArgument(this->CMakeModulePath, dirs);

  for(std::vector<std::string>::const_iterator dirIt = dirs.begin();
      dirIt != dirs.end();
      ++dirIt)
    {
    cmsys::Directory dir;
    dir.Load(dirIt->c_str());
    if (dir.GetNumberOfFiles() > 0)
      {
      if (!sectionHasHeader)
        {
        cmDocumentationSection *sec =
          new cmDocumentationSection("Custom CMake Modules","CUSTOM MODULES");
        this->AllSections["Custom CMake Modules"] = sec;
        sec->Append(cmDocumentationCustomModulesHeader[0]);
        sec->Append(cmCustomModulesDocumentationDescription);
        sectionHasHeader = true;
        }
      this->CreateModuleDocsForDir
        (dir, *this->AllSections["Custom CMake Modules"]);
      }
    }

  return true;
}

//----------------------------------------------------------------------------
void cmDocumentation
::CreateModuleDocsForDir(cmsys::Directory& dir,
                         cmDocumentationSection &moduleSection)
{
  // sort the files alphabetically, so the docs for one module are easier
  // to find than if they are in random order
  std::vector<std::string> sortedFiles;
  for(unsigned int i = 0; i < dir.GetNumberOfFiles(); ++i)
  {
    sortedFiles.push_back(dir.GetFile(i));
  }
  std::sort(sortedFiles.begin(), sortedFiles.end());

  for(std::vector<std::string>::const_iterator fname = sortedFiles.begin();
      fname!=sortedFiles.end(); ++fname)
    {
    if(fname->length() > 6)
      {
      if(fname->substr(fname->length()-6, 6) == ".cmake")
        {
        std::string moduleName = fname->substr(0, fname->length()-6);
        // this check is to avoid creating documentation for the modules with
        // the same name in multiple directories of CMAKE_MODULE_PATH
        if (this->ModulesFound.find(moduleName) == this->ModulesFound.end())
          {
          this->ModulesFound.insert(moduleName);
          std::string path = dir.GetPath();
          path += "/";
          path += (*fname);
          this->CreateSingleModule(path.c_str(), moduleName.c_str(),
                                   moduleSection);
          }
        }
      }
    }
}

//----------------------------------------------------------------------------
bool cmDocumentation::CreateSingleModule(const char* fname,
                                         const char* moduleName,
                                         cmDocumentationSection &moduleSection)
{
  std::ifstream fin(fname);
  if(!fin)
    {
    std::cerr << "Internal error: can not open module." << fname << std::endl;
    return false;
    }
  std::string line;
  std::string text;
  std::string brief;
  brief = " ";
  bool newParagraph = true;
  while ( fin && cmSystemTools::GetLineFromStream(fin, line) )
    {
    if(line.size() && line[0] == '#')
      {
      // blank line
      if(line.size() <= 2)
        {
        text += "\n";
        newParagraph = true;
        }
      else if(line[2] == '-')
        {
        brief = line.c_str()+4;
        }
      else
        {
        // two spaces
        if(line[1] == ' ' && line[2] == ' ')
          {
          if(!newParagraph)
            {
            text += "\n";
            newParagraph = true;
            }
          // Skip #, and leave space for preformatted
          text += line.c_str()+1;
          text += "\n";
          }
        else if(line[1] == ' ')
          {
          if(!newParagraph)
            {
            text += " ";
            }
          newParagraph = false;
          // skip # and space
          text += line.c_str()+2;
          }
        else
          {
          if(!newParagraph)
            {
            text += " ";
            }
          newParagraph = false;
          // skip #
          text += line.c_str()+1;
          }
        }
      }
    else
      {
      break;
      }
    }

  if(text.length() < 2 && brief.length() == 1)
    {
    return false;
    }

  char* pname = strcpy(new char[strlen(moduleName)+1], moduleName);
  char* ptext = strcpy(new char[text.length()+1], text.c_str());
  this->ModuleStrings.push_back(pname);
  this->ModuleStrings.push_back(ptext);
  char* pbrief = strcpy(new char[brief.length()+1], brief.c_str());
  this->ModuleStrings.push_back(pbrief);
  moduleSection.Append(pname, pbrief, ptext);
  return true;
}


//----------------------------------------------------------------------------
bool cmDocumentation::PrintRequestedDocumentation(std::ostream& os)
{
  bool result = true;

  // Loop over requested documentation types.
  for(std::vector<RequestedHelpItem>::const_iterator
      i = this->RequestedHelpItems.begin();
      i != this->RequestedHelpItems.end();
      ++i)
    {
    this->SetForm(i->HelpForm, i->ManSection);
    this->CurrentArgument = i->Argument;
    // If a file name was given, use it.  Otherwise, default to the
    // given stream.
    std::ofstream* fout = 0;
    std::ostream* s = &os;
    std::string docname("");
    if(i->Filename.length() > 0)
      {
      fout = new std::ofstream(i->Filename.c_str(), std::ios::out);
      if(fout)
        {
        s = fout;
        }
      else
        {
        result = false;
        }
      if(i->Filename != "-")
        {
        docname = cmSystemTools::GetFilenameWithoutLastExtension(i->Filename);
        }
      }

    // Print this documentation type to the stream.
    if(!this->PrintDocumentation(i->HelpType, *s, docname.c_str()) || !*s)
      {
      result = false;
      }

    // Close the file if we wrote one.
    if(fout)
      {
      delete fout;
      }
    }
  return result;
}

#define GET_OPT_ARGUMENT(target)                      \
     if((i+1 < argc) && !this->IsOption(argv[i+1]))   \
        {                                             \
        target = argv[i+1];                           \
        i = i+1;                                      \
        };


cmDocumentation::Form cmDocumentation::GetFormFromFilename(
                                                   const std::string& filename,
                                                   int* manSection)
{
  std::string ext = cmSystemTools::GetFilenameLastExtension(filename);
  ext = cmSystemTools::UpperCase(ext);
  if ((ext == ".HTM") || (ext == ".HTML"))
    {
    return cmDocumentation::HTMLForm;
    }

  if (ext == ".DOCBOOK")
    {
    return cmDocumentation::DocbookForm;
    }

  // ".1" to ".9" should be manpages
  if ((ext.length()==2) && (ext[1] >='1') && (ext[1]<='9'))
    {
    if (manSection)
      {
      *manSection = ext[1] - '0';
      }
    return cmDocumentation::ManForm;
    }

  if (ext == ".RST")
    {
    return cmDocumentation::RSTForm;
    }

  return cmDocumentation::TextForm;
}

//----------------------------------------------------------------------------
void cmDocumentation::addCommonStandardDocSections()
{
    cmDocumentationSection *sec;

    sec = new cmDocumentationSection("Author","AUTHOR");
    sec->Append(cmDocumentationEntry
               (0,
                "This manual page was generated by the \"--help-man\" option.",
                0));
    this->AllSections["Author"] = sec;

    sec = new cmDocumentationSection("Copyright","COPYRIGHT");
    sec->Append(cmDocumentationCopyright);
    this->AllSections["Copyright"] = sec;

    sec = new cmDocumentationSection("See Also","SEE ALSO");
    sec->Append(cmDocumentationStandardSeeAlso);
    this->AllSections["Standard See Also"] = sec;

    sec = new cmDocumentationSection("Options","OPTIONS");
    sec->Append(cmDocumentationStandardOptions);
    this->AllSections["Options"] = sec;

    sec = new cmDocumentationSection("Compatibility Commands",
                                     "COMPATIBILITY COMMANDS");
    sec->Append(cmCompatCommandsDocumentationDescription);
    this->AllSections["Compatibility Commands"] = sec;
}

//----------------------------------------------------------------------------
void cmDocumentation::addCMakeStandardDocSections()
{
    cmDocumentationSection *sec;

    sec = new cmDocumentationSection("Properties","PROPERTIES");
    sec->Append(cmPropertiesDocumentationDescription);
    this->AllSections["Properties Description"] = sec;

    sec = new cmDocumentationSection("Generators","GENERATORS");
    sec->Append(cmDocumentationGeneratorsHeader);
    this->AllSections["Generators"] = sec;

    this->PropertySections.push_back("Properties of Global Scope");
    this->PropertySections.push_back("Properties on Directories");
    this->PropertySections.push_back("Properties on Targets");
    this->PropertySections.push_back("Properties on Tests");
    this->PropertySections.push_back("Properties on Source Files");
    this->PropertySections.push_back("Properties on Cache Entries");

    this->VariableSections.push_back("Variables that Provide Information");
    this->VariableSections.push_back("Variables That Change Behavior");
    this->VariableSections.push_back("Variables That Describe the System");
    this->VariableSections.push_back("Variables that Control the Build");
    this->VariableSections.push_back("Variables for Languages");

}

//----------------------------------------------------------------------------
void cmDocumentation::addCTestStandardDocSections()
{
    // This is currently done for backward compatibility reason
    // We may suppress some of these.
    addCMakeStandardDocSections();
}

//----------------------------------------------------------------------------
void cmDocumentation::addCPackStandardDocSections()
{
    cmDocumentationSection *sec;

    sec = new cmDocumentationSection("Generators","GENERATORS");
    sec->Append(cmDocumentationGeneratorsHeader);
    this->AllSections["Generators"] = sec;

    this->VariableSections.push_back(
            "Variables common to all CPack generators");
}

void cmDocumentation::addAutomaticVariableSections(const std::string& section)
{
  std::vector<std::string>::iterator it;
  it = std::find(this->VariableSections.begin(),
                 this->VariableSections.end(),
                 section);
  /* if the section does not exist then add it */
  if (it==this->VariableSections.end())
    {
    this->VariableSections.push_back(section);
    }
}

//----------------------------------------------------------------------------
bool cmDocumentation::CheckOptions(int argc, const char* const* argv,
                                   const char* exitOpt)
{
  // Providing zero arguments gives usage information.
  if(argc == 1)
    {
    RequestedHelpItem help;
    help.HelpType = cmDocumentation::Usage;
    help.HelpForm = cmDocumentation::UsageForm;
    this->RequestedHelpItems.push_back(help);
    return true;
    }

  // Search for supported help options.

  bool result = false;
  for(int i=1; i < argc; ++i)
    {
    if(exitOpt && strcmp(argv[i], exitOpt) == 0)
      {
      return result;
      }
    RequestedHelpItem help;
    // Check if this is a supported help option.
    if((strcmp(argv[i], "-help") == 0) ||
       (strcmp(argv[i], "--help") == 0) ||
       (strcmp(argv[i], "/?") == 0) ||
       (strcmp(argv[i], "-usage") == 0) ||
       (strcmp(argv[i], "-h") == 0) ||
       (strcmp(argv[i], "-H") == 0))
      {
      help.HelpType = cmDocumentation::Usage;
      help.HelpForm = cmDocumentation::UsageForm;
      GET_OPT_ARGUMENT(help.Argument);
      help.Argument = cmSystemTools::LowerCase(help.Argument);
      // special case for single command
      if (!help.Argument.empty())
        {
        help.HelpType = cmDocumentation::Single;
        }
      }
    else if(strcmp(argv[i], "--help-properties") == 0)
      {
      help.HelpType = cmDocumentation::Properties;
      GET_OPT_ARGUMENT(help.Filename);
      help.HelpForm = this->GetFormFromFilename(help.Filename,
                                                &help.ManSection);
      }
    else if(strcmp(argv[i], "--help-policies") == 0)
      {
      help.HelpType = cmDocumentation::Policies;
      GET_OPT_ARGUMENT(help.Filename);
      help.HelpForm = this->GetFormFromFilename(help.Filename,
                                                &help.ManSection);
      }
    else if(strcmp(argv[i], "--help-variables") == 0)
      {
      help.HelpType = cmDocumentation::Variables;
      GET_OPT_ARGUMENT(help.Filename);
      help.HelpForm = this->GetFormFromFilename(help.Filename,
                                                &help.ManSection);
      }
    else if(strcmp(argv[i], "--help-modules") == 0)
      {
      help.HelpType = cmDocumentation::Modules;
      GET_OPT_ARGUMENT(help.Filename);
      help.HelpForm = this->GetFormFromFilename(help.Filename,
                                                &help.ManSection);
      }
    else if(strcmp(argv[i], "--help-custom-modules") == 0)
      {
      help.HelpType = cmDocumentation::CustomModules;
      GET_OPT_ARGUMENT(help.Filename);
      help.HelpForm = this->GetFormFromFilename(help.Filename,
                                                &help.ManSection);
      }
    else if(strcmp(argv[i], "--help-commands") == 0)
      {
      help.HelpType = cmDocumentation::Commands;
      GET_OPT_ARGUMENT(help.Filename);
      help.HelpForm = this->GetFormFromFilename(help.Filename,
                                                &help.ManSection);
      }
    else if(strcmp(argv[i], "--help-compatcommands") == 0)
      {
      help.HelpType = cmDocumentation::CompatCommands;
      GET_OPT_ARGUMENT(help.Filename);
      help.HelpForm = this->GetFormFromFilename(help.Filename,
                                                &help.ManSection);
      }
    else if(strcmp(argv[i], "--help-full") == 0)
      {
      help.HelpType = cmDocumentation::Full;
      GET_OPT_ARGUMENT(help.Filename);
      help.HelpForm = this->GetFormFromFilename(help.Filename,
                                                &help.ManSection);
      }
    else if(strcmp(argv[i], "--help-html") == 0)
      {
      help.HelpType = cmDocumentation::Full;
      GET_OPT_ARGUMENT(help.Filename);
      help.HelpForm = cmDocumentation::HTMLForm;
      }
    else if(strcmp(argv[i], "--help-man") == 0)
      {
      help.HelpType = cmDocumentation::Full;
      GET_OPT_ARGUMENT(help.Filename);
      help.HelpForm = cmDocumentation::ManForm;
      help.ManSection = 1;
      }
    else if(strcmp(argv[i], "--help-command") == 0)
      {
      help.HelpType = cmDocumentation::Single;
      GET_OPT_ARGUMENT(help.Argument);
      GET_OPT_ARGUMENT(help.Filename);
      help.Argument = cmSystemTools::LowerCase(help.Argument);
      help.HelpForm = this->GetFormFromFilename(help.Filename,
                                                &help.ManSection);
      }
    else if(strcmp(argv[i], "--help-module") == 0)
      {
      help.HelpType = cmDocumentation::SingleModule;
      GET_OPT_ARGUMENT(help.Argument);
      GET_OPT_ARGUMENT(help.Filename);
      help.HelpForm = this->GetFormFromFilename(help.Filename,
                                                &help.ManSection);
      }
    else if(strcmp(argv[i], "--help-property") == 0)
      {
      help.HelpType = cmDocumentation::SingleProperty;
      GET_OPT_ARGUMENT(help.Argument);
      GET_OPT_ARGUMENT(help.Filename);
      help.HelpForm = this->GetFormFromFilename(help.Filename,
                                                &help.ManSection);
      }
    else if(strcmp(argv[i], "--help-policy") == 0)
      {
      help.HelpType = cmDocumentation::SinglePolicy;
      GET_OPT_ARGUMENT(help.Argument);
      GET_OPT_ARGUMENT(help.Filename);
      help.HelpForm = this->GetFormFromFilename(help.Filename,
                                                &help.ManSection);
      }
    else if(strcmp(argv[i], "--help-variable") == 0)
      {
      help.HelpType = cmDocumentation::SingleVariable;
      GET_OPT_ARGUMENT(help.Argument);
      GET_OPT_ARGUMENT(help.Filename);
      help.HelpForm = this->GetFormFromFilename(help.Filename,
                                                &help.ManSection);
      }
    else if(strcmp(argv[i], "--help-command-list") == 0)
      {
      help.HelpType = cmDocumentation::List;
      GET_OPT_ARGUMENT(help.Filename);
      help.HelpForm = cmDocumentation::TextForm;
      }
    else if(strcmp(argv[i], "--help-module-list") == 0)
      {
      help.HelpType = cmDocumentation::ModuleList;
      GET_OPT_ARGUMENT(help.Filename);
      help.HelpForm = cmDocumentation::TextForm;
      }
    else if(strcmp(argv[i], "--help-property-list") == 0)
      {
      help.HelpType = cmDocumentation::PropertyList;
      GET_OPT_ARGUMENT(help.Filename);
      help.HelpForm = cmDocumentation::TextForm;
      }
    else if(strcmp(argv[i], "--help-variable-list") == 0)
      {
      help.HelpType = cmDocumentation::VariableList;
      GET_OPT_ARGUMENT(help.Filename);
      help.HelpForm = cmDocumentation::TextForm;
      }
    else if(strcmp(argv[i], "--help-policy-list") == 0)
      {
      help.HelpType = cmDocumentation::PolicyList;
      GET_OPT_ARGUMENT(help.Filename);
      help.HelpForm = cmDocumentation::TextForm;
      }
    else if(strcmp(argv[i], "--copyright") == 0)
      {
      help.HelpType = cmDocumentation::Copyright;
      GET_OPT_ARGUMENT(help.Filename);
      help.HelpForm = cmDocumentation::UsageForm;
      }
    else if((strcmp(argv[i], "--version") == 0) ||
            (strcmp(argv[i], "-version") == 0) ||
            (strcmp(argv[i], "/V") == 0))
      {
      help.HelpType = cmDocumentation::Version;
      GET_OPT_ARGUMENT(help.Filename);
      help.HelpForm = cmDocumentation::UsageForm;
      }
    if(help.HelpType != None)
      {
      // This is a help option.  See if there is a file name given.
      result = true;
      this->RequestedHelpItems.push_back(help);
      }
    }
  return result;
}

//----------------------------------------------------------------------------
void cmDocumentation::Print(Form f, int manSection, std::ostream& os)
{
  this->SetForm(f, manSection);
  this->Print(os);
}

//----------------------------------------------------------------------------
void cmDocumentation::Print(std::ostream& os)
{
  // if the formatter supports it, print a master index for
  // all sections
  this->CurrentFormatter->PrintIndex(os, this->PrintSections);
  for(unsigned int i=0; i < this->PrintSections.size(); ++i)
    {
    std::string name = this->PrintSections[i]->
      GetName((this->CurrentFormatter->GetForm()));
    this->CurrentFormatter->PrintSection(os,*this->PrintSections[i],
                                         name.c_str());
    }
}

//----------------------------------------------------------------------------
void cmDocumentation::SetName(const char* name)
{
  this->NameString = name?name:"";
}

//----------------------------------------------------------------------------
void cmDocumentation::SetDocName(const char *docname)
{
  this->DocName = docname?docname:"";
}

//----------------------------------------------------------------------------
void cmDocumentation::SetSection(const char *name,
                                 cmDocumentationSection *section)
{
  if (this->AllSections.find(name) != this->AllSections.end())
    {
    delete this->AllSections[name];
    }
  this->AllSections[name] = section;
}

//----------------------------------------------------------------------------
void cmDocumentation::SetSection(const char *name,
                                 std::vector<cmDocumentationEntry> &docs)
{
  cmDocumentationSection *sec =
    new cmDocumentationSection(name,
                               cmSystemTools::UpperCase(name).c_str());
  sec->Append(docs);
  this->SetSection(name,sec);
}

//----------------------------------------------------------------------------
void cmDocumentation::SetSection(const char *name,
                                 const char *docs[][3])
{
  cmDocumentationSection *sec =
    new cmDocumentationSection(name,
                               cmSystemTools::UpperCase(name).c_str());
  sec->Append(docs);
  this->SetSection(name,sec);
}

//----------------------------------------------------------------------------
void cmDocumentation
::SetSections(std::map<std::string,cmDocumentationSection *> &sections)
{
  for (std::map<std::string,cmDocumentationSection *>::const_iterator
         it = sections.begin(); it != sections.end(); ++it)
    {
    this->SetSection(it->first.c_str(),it->second);
    }
}

//----------------------------------------------------------------------------
void cmDocumentation::PrependSection(const char *name,
                                     const char *docs[][3])
{
  cmDocumentationSection *sec = 0;
  if (this->AllSections.find(name) == this->AllSections.end())
    {
    sec = new cmDocumentationSection
      (name, cmSystemTools::UpperCase(name).c_str());
    this->SetSection(name,sec);
    }
  else
    {
    sec = this->AllSections[name];
    }
  sec->Prepend(docs);
}

//----------------------------------------------------------------------------
void cmDocumentation::PrependSection(const char *name,
                                     std::vector<cmDocumentationEntry> &docs)
{
  cmDocumentationSection *sec = 0;
  if (this->AllSections.find(name) == this->AllSections.end())
    {
    sec = new cmDocumentationSection
      (name, cmSystemTools::UpperCase(name).c_str());
    this->SetSection(name,sec);
    }
  else
    {
    sec = this->AllSections[name];
    }
  sec->Prepend(docs);
}

//----------------------------------------------------------------------------
void cmDocumentation::AppendSection(const char *name,
                                    const char *docs[][3])
{
  cmDocumentationSection *sec = 0;
  if (this->AllSections.find(name) == this->AllSections.end())
    {
    sec = new cmDocumentationSection
      (name, cmSystemTools::UpperCase(name).c_str());
    this->SetSection(name,sec);
    }
  else
    {
    sec = this->AllSections[name];
    }
  sec->Append(docs);
}

//----------------------------------------------------------------------------
void cmDocumentation::AppendSection(const char *name,
                                    std::vector<cmDocumentationEntry> &docs)
{
  cmDocumentationSection *sec = 0;
  if (this->AllSections.find(name) == this->AllSections.end())
    {
    sec = new cmDocumentationSection
      (name, cmSystemTools::UpperCase(name).c_str());
    this->SetSection(name,sec);
    }
  else
    {
    sec = this->AllSections[name];
    }
  sec->Append(docs);
}

//----------------------------------------------------------------------------
void cmDocumentation::AppendSection(const char *name,
                                    cmDocumentationEntry &docs)
{

  std::vector<cmDocumentationEntry> docsVec;
  docsVec.push_back(docs);
  this->AppendSection(name,docsVec);
}

//----------------------------------------------------------------------------
void cmDocumentation::PrependSection(const char *name,
                                     cmDocumentationEntry &docs)
{

  std::vector<cmDocumentationEntry> docsVec;
  docsVec.push_back(docs);
  this->PrependSection(name,docsVec);
}

//----------------------------------------------------------------------------
void cmDocumentation::SetSeeAlsoList(const char *data[][3])
{
  cmDocumentationSection *sec =
    new cmDocumentationSection("See Also", "SEE ALSO");
  this->AllSections["See Also"] = sec;
  this->SeeAlsoString = ".B ";
  int i = 0;
  while(data[i][1])
    {
    this->SeeAlsoString += data[i][1];
    this->SeeAlsoString += data[i+1][1]? "(1), ":"(1)";
    ++i;
    }
  sec->Append(0,this->SeeAlsoString.c_str(),0);
  sec->Append(cmDocumentationStandardSeeAlso);
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintDocumentationGeneric(std::ostream& os,
                                                const char *section)
{
  if(this->AllSections.find(section) == this->AllSections.end())
    {
    os << "Internal error: " << section << " list is empty." << std::endl;
    return false;
    }
  if(this->CurrentArgument.length() == 0)
    {
    os << "Required argument missing.\n";
    return false;
    }
  const std::vector<cmDocumentationEntry> &entries =
    this->AllSections[section]->GetEntries();
  for(std::vector<cmDocumentationEntry>::const_iterator ei =
        entries.begin();
      ei != entries.end(); ++ei)
    {
    if(this->CurrentArgument == ei->Name)
      {
      this->PrintDocumentationCommand(os, *ei);
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintDocumentationSingle(std::ostream& os)
{
  if (this->PrintDocumentationGeneric(os,"Commands"))
    {
    return true;
    }
  if (this->PrintDocumentationGeneric(os,"Compatibility Commands"))
    {
    return true;
    }

  // Argument was not a command.  Complain.
  os << "Argument \"" << this->CurrentArgument.c_str()
     << "\" to --help-command is not a CMake command.  "
     << "Use --help-command-list to see all commands.\n";
  return false;
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintDocumentationSingleModule(std::ostream& os)
{
  if(this->CurrentArgument.length() == 0)
    {
    os << "Argument --help-module needs a module name.\n";
    return false;
    }

  std::string moduleName;
  // find the module
  std::vector<std::string> dirs;
  cmSystemTools::ExpandListArgument(this->CMakeModulePath, dirs);
  for(std::vector<std::string>::const_iterator dirIt = dirs.begin();
      dirIt != dirs.end();
      ++dirIt)
    {
    moduleName = *dirIt;
    moduleName += "/";
    moduleName += this->CurrentArgument;
    moduleName += ".cmake";
    if(cmSystemTools::FileExists(moduleName.c_str()))
      {
      break;
      }
    moduleName = "";
    }

  if (moduleName.empty())
    {
    moduleName = this->CMakeRoot;
    moduleName += "/Modules/";
    moduleName += this->CurrentArgument;
    moduleName += ".cmake";
    if(!cmSystemTools::FileExists(moduleName.c_str()))
      {
      moduleName = "";
      }
    }

  if(!moduleName.empty())
    {
    cmDocumentationSection *sec =
      new cmDocumentationSection("Standard CMake Modules", "MODULES");
    this->AllSections["Modules"] = sec;
    if (this->CreateSingleModule(moduleName.c_str(),
                                 this->CurrentArgument.c_str(),
                                 *this->AllSections["Modules"]))
      {
      if(this->AllSections["Modules"]->GetEntries().size())
        {
        this->PrintDocumentationCommand
          (os,  this->AllSections["Modules"]->GetEntries()[0]);
        os <<  "\n       Defined in: ";
        os << moduleName << "\n";
        return true;
        }
      else
        {
        return false;
        }
      }
    }

  // Argument was not a module.  Complain.
  os << "Argument \"" << this->CurrentArgument.c_str()
     << "\" to --help-module is not a CMake module.\n";
  return false;
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintDocumentationSingleProperty(std::ostream& os)
{
  bool done = false;
  for (std::vector<std::string>::iterator i =
         this->PropertySections.begin();
       !done && i != this->PropertySections.end(); ++i)
    {
    done = this->PrintDocumentationGeneric(os,i->c_str());
    }

  if (done)
    {
    return true;
    }

  // Argument was not a command.  Complain.
  os << "Argument \"" << this->CurrentArgument.c_str()
     << "\" to --help-property is not a CMake property.  "
     << "Use --help-property-list to see all properties.\n";
  return false;
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintDocumentationSinglePolicy(std::ostream& os)
{
  if (this->PrintDocumentationGeneric(os,"Policies"))
    {
    return true;
    }

  // Argument was not a policy.  Complain.
  os << "Argument \"" << this->CurrentArgument.c_str()
     << "\" to --help-policy is not a CMake policy.\n";
  return false;
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintDocumentationSingleVariable(std::ostream& os)
{
  bool done = false;
  for (std::vector<std::string>::iterator i =
         this->VariableSections.begin();
       !done && i != this->VariableSections.end(); ++i)
    {
    done = this->PrintDocumentationGeneric(os,i->c_str());
    }

  if (done)
    {
    return true;
    }

  // Argument was not a variable.  Complain.
  os << "Argument \"" << this->CurrentArgument.c_str()
     << "\" to --help-variable is not a defined variable.  "
     << "Use --help-variable-list to see all defined variables.\n";
  return false;
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintDocumentationList(std::ostream& os,
                                             const char *section)
{
  if(this->AllSections.find(section) == this->AllSections.end())
    {
    os << "Internal error: " << section << " list is empty." << std::endl;
    return false;
    }

  const std::vector<cmDocumentationEntry> &entries =
    this->AllSections[section]->GetEntries();
  for(std::vector<cmDocumentationEntry>::const_iterator ei =
        entries.begin();
      ei != entries.end(); ++ei)
    {
    if(ei->Name.size())
      {
      os << ei->Name << std::endl;
      }
    }
  return true;
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintDocumentationUsage(std::ostream& os)
{
  this->ClearSections();
  this->AddSectionToPrint("Usage");
  this->AddSectionToPrint("Options");
  if(this->ShowGenerators)
    {
    this->AddSectionToPrint("Generators");
    }
  this->Print(os);
  return true;
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintDocumentationFull(std::ostream& os)
{
  this->CreateFullDocumentation();
  this->CurrentFormatter->PrintHeader(GetNameString(), GetNameString(), os);
  this->Print(os);
  this->CurrentFormatter->PrintFooter(os);
  return true;
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintDocumentationModules(std::ostream& os)
{
  this->ClearSections();
  this->CreateModulesSection();
  this->AddDocumentIntroToPrint(GET_DOCUMENT_INTRO(Modules));
  this->AddSectionToPrint("Description");
  this->AddSectionToPrint("Modules");
  this->AddSectionToPrint("Copyright");
  this->AddSectionToPrint("See Also");
  this->CurrentFormatter->PrintHeader(GetDocName(), GetNameString(), os);
  this->Print(os);
  this->CurrentFormatter->PrintFooter(os);
  return true;
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintDocumentationCustomModules(std::ostream& os)
{
  this->ClearSections();
  this->CreateCustomModulesSection();
  this->AddDocumentIntroToPrint(GET_DOCUMENT_INTRO(CustomModules));
  this->AddSectionToPrint("Description");
  this->AddSectionToPrint("Custom CMake Modules");
// the custom modules are most probably not under Kitware's copyright, Alex
//  this->AddSectionToPrint("Copyright");
  this->AddSectionToPrint("See Also");

  this->CurrentFormatter->PrintHeader(GetDocName(), GetNameString(), os);
  this->Print(os);
  this->CurrentFormatter->PrintFooter(os);
  return true;
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintDocumentationPolicies(std::ostream& os)
{
  this->ClearSections();
  this->AddDocumentIntroToPrint(GET_DOCUMENT_INTRO(Policies));
  this->AddSectionToPrint("Description");
  this->AddSectionToPrint("Policies");
  this->AddSectionToPrint("Copyright");
  this->AddSectionToPrint("See Also");

  this->CurrentFormatter->PrintHeader(GetDocName(), GetNameString(), os);
  this->Print(os);
  this->CurrentFormatter->PrintFooter(os);
  return true;
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintDocumentationProperties(std::ostream& os)
{
  this->ClearSections();
  this->AddDocumentIntroToPrint(GET_DOCUMENT_INTRO(Properties));
  this->AddSectionToPrint("Properties Description");
  for (std::vector<std::string>::iterator i =
         this->PropertySections.begin();
       i != this->PropertySections.end(); ++i)
    {
    this->AddSectionToPrint(i->c_str());
    }
  this->AddSectionToPrint("Copyright");
  this->AddSectionToPrint("Standard See Also");
  this->CurrentFormatter->PrintHeader(GetDocName(), GetNameString(), os);
  this->Print(os);
  this->CurrentFormatter->PrintFooter(os);
  return true;
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintDocumentationVariables(std::ostream& os)
{
  this->ClearSections();
  this->AddDocumentIntroToPrint(GET_DOCUMENT_INTRO(Variables));
  for (std::vector<std::string>::iterator i =
         this->VariableSections.begin();
       i != this->VariableSections.end(); ++i)
    {
    this->AddSectionToPrint(i->c_str());
    }
  this->AddSectionToPrint("Copyright");
  this->AddSectionToPrint("Standard See Also");
  this->CurrentFormatter->PrintHeader(GetDocName(), GetNameString(), os);
  this->Print(os);
  this->CurrentFormatter->PrintFooter(os);
  return true;
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintDocumentationCurrentCommands(std::ostream& os)
{
  this->ClearSections();
  this->AddDocumentIntroToPrint(GET_DOCUMENT_INTRO(Commands));
  this->AddSectionToPrint("Commands");
  this->AddSectionToPrint("Copyright");
  this->AddSectionToPrint("Standard See Also");
  this->CurrentFormatter->PrintHeader(GetDocName(), GetNameString(), os);
  this->Print(os);
  this->CurrentFormatter->PrintFooter(os);
  return true;
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintDocumentationCompatCommands(std::ostream& os)
{
  this->ClearSections();
  this->AddDocumentIntroToPrint(GET_DOCUMENT_INTRO(CompatCommands));
  this->AddSectionToPrint("Compatibility Commands Description");
  this->AddSectionToPrint("Compatibility Commands");
  this->AddSectionToPrint("Copyright");
  this->AddSectionToPrint("Standard See Also");
  this->CurrentFormatter->PrintHeader(GetDocName(), GetNameString(), os);
  this->Print(os);
  this->CurrentFormatter->PrintFooter(os);
  return true;
}

//----------------------------------------------------------------------------
void cmDocumentation
::PrintDocumentationCommand(std::ostream& os,
                            const cmDocumentationEntry &entry)
{
  // the string "SingleItem" will be used in a few places to detect the case
  // that only the documentation for a single item is printed
  cmDocumentationSection *sec = new cmDocumentationSection("SingleItem","");
  sec->Append(entry);
  this->AllSections["temp"] = sec;
  this->ClearSections();
  this->AddSectionToPrint("temp");
  this->Print(os);
  this->AllSections.erase("temp");
  delete sec;
}

//----------------------------------------------------------------------------
void cmDocumentation::CreateFullDocumentation()
{
  this->ClearSections();
  this->CreateCustomModulesSection();
  this->CreateModulesSection();

  std::set<std::string> emitted;
  this->AddSectionToPrint("Name");
  emitted.insert("Name");
  this->AddSectionToPrint("Usage");
  emitted.insert("Usage");
  this->AddSectionToPrint("Description");
  emitted.insert("Description");
  this->AddSectionToPrint("Options");
  emitted.insert("Options");
  this->AddSectionToPrint("Generators");
  emitted.insert("Generators");
  this->AddSectionToPrint("Commands");
  emitted.insert("Commands");


  this->AddSectionToPrint("Properties Description");
  emitted.insert("Properties Description");
  for (std::vector<std::string>::iterator i =
         this->PropertySections.begin();
       i != this->PropertySections.end(); ++i)
    {
    this->AddSectionToPrint(i->c_str());
    emitted.insert(i->c_str());
    }

  emitted.insert("Copyright");
  emitted.insert("See Also");
  emitted.insert("Standard See Also");
  emitted.insert("Author");

  // add any sections not yet written out, or to be written out
  for (std::map<std::string, cmDocumentationSection*>::iterator i =
         this->AllSections.begin();
       i != this->AllSections.end(); ++i)
    {
    if (emitted.find(i->first) == emitted.end())
      {
      this->AddSectionToPrint(i->first.c_str());
      }
    }

  this->AddSectionToPrint("Copyright");

  if(this->CurrentFormatter->GetForm() == ManForm)
    {
    this->AddSectionToPrint("See Also");
    this->AddSectionToPrint("Author");
    }
  else
    {
    this->AddSectionToPrint("Standard See Also");
    }
}

//----------------------------------------------------------------------------
void cmDocumentation::SetForm(Form f, int manSection)
{
  switch(f)
  {
    case HTMLForm:
      this->CurrentFormatter = &this->HTMLFormatter;
      break;
    case DocbookForm:
      this->CurrentFormatter = &this->DocbookFormatter;
      break;
    case ManForm:
      this->ManFormatter.SetManSection(manSection);
      this->CurrentFormatter = &this->ManFormatter;
      break;
    case RSTForm:
      this->CurrentFormatter = &this->RSTFormatter;
      break;
    case TextForm:
      this->CurrentFormatter = &this->TextFormatter;
      break;
    case UsageForm:
      this->CurrentFormatter = & this->UsageFormatter;
      break;
  }
}


//----------------------------------------------------------------------------
const char* cmDocumentation::GetNameString() const
{
  if(this->NameString.length() > 0)
    {
    return this->NameString.c_str();
    }
  else
    {
    return "CMake";
    }
}

//----------------------------------------------------------------------------
const char* cmDocumentation::GetDocName(bool fallbackToNameString) const
{
  if (this->DocName.length() > 0)
    {
    return this->DocName.c_str();
    }
  else if (fallbackToNameString)
    {
    return this->GetNameString();
    }
  else
    return 0;
}

//----------------------------------------------------------------------------
#define CASE_DEFAULT_DOCNAME(doctype) \
  case cmDocumentation::doctype : \
    return GET_DOCUMENT_INTRO(doctype)[0];
const char* cmDocumentation::GetDefaultDocName(Type ht) const
{
  switch (ht)
    {
    CASE_DEFAULT_DOCNAME(Modules)
    CASE_DEFAULT_DOCNAME(CustomModules)
    CASE_DEFAULT_DOCNAME(Policies)
    CASE_DEFAULT_DOCNAME(Properties)
    CASE_DEFAULT_DOCNAME(Variables)
    CASE_DEFAULT_DOCNAME(Commands)
    CASE_DEFAULT_DOCNAME(CompatCommands)
    default: break;
    }
  return 0;
}

//----------------------------------------------------------------------------
bool cmDocumentation::IsOption(const char* arg) const
{
  return ((arg[0] == '-') || (strcmp(arg, "/V") == 0) ||
          (strcmp(arg, "/?") == 0));
}
