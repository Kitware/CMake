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
static const char *cmDocumentationGeneratorsHeader[][3] =
{
  {0,
   "The following generators are available on this platform:", 0},
  {0,0,0}
};

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
  for(std::map<std::string,cmDocumentationSection *>::iterator i =
        this->AllSections.begin();
      i != this->AllSections.end(); ++i)
    {
    delete i->second;
    }
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
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintDocumentation(Type ht, std::ostream& os)
{
  switch (ht)
    {
    case cmDocumentation::Usage:
      return this->PrintDocumentationUsage(os);
    case cmDocumentation::Single:
    case cmDocumentation::SingleModule:
    case cmDocumentation::SinglePolicy:
    case cmDocumentation::SingleProperty:
    case cmDocumentation::SingleVariable:
    case cmDocumentation::List:
    case cmDocumentation::ModuleList:
    case cmDocumentation::PropertyList:
    case cmDocumentation::VariableList:
    case cmDocumentation::PolicyList:
    case cmDocumentation::Modules:
    case cmDocumentation::Policies:
    case cmDocumentation::Properties:
    case cmDocumentation::Variables:
    case cmDocumentation::Commands:
      return false;
    case cmDocumentation::Version:
      return this->PrintVersion(os);
    default: return false;
    }
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
      }

    // Print this documentation type to the stream.
    if(!this->PrintDocumentation(i->HelpType, *s) || !*s)
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

    sec = new cmDocumentationSection("Options","OPTIONS");
    sec->Append(cmDocumentationStandardOptions);
    this->AllSections["Options"] = sec;
}

//----------------------------------------------------------------------------
void cmDocumentation::addCMakeStandardDocSections()
{
    cmDocumentationSection *sec;

    sec = new cmDocumentationSection("Generators","GENERATORS");
    sec->Append(cmDocumentationGeneratorsHeader);
    this->AllSections["Generators"] = sec;
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
      GET_OPT_ARGUMENT(help.Filename);
      cmSystemTools::Message(
        "Warning: --help-custom-modules no longer supported");
      return true;
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
      GET_OPT_ARGUMENT(help.Filename);
      cmSystemTools::Message(
        "Warning: --help-compatcommands no longer supported");
      return true;
      }
    else if(strcmp(argv[i], "--help-full") == 0)
      {
      GET_OPT_ARGUMENT(help.Filename);
      cmSystemTools::Message("Warning: --help-full no longer supported");
      return true;
      }
    else if(strcmp(argv[i], "--help-html") == 0)
      {
      GET_OPT_ARGUMENT(help.Filename);
      cmSystemTools::Message("Warning: --help-html no longer supported");
      return true;
      }
    else if(strcmp(argv[i], "--help-man") == 0)
      {
      GET_OPT_ARGUMENT(help.Filename);
      cmSystemTools::Message("Warning: --help-man no longer supported");
      return true;
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
      GET_OPT_ARGUMENT(help.Filename);
      cmSystemTools::Message("Warning: --copyright no longer supported");
      return true;
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
bool cmDocumentation::IsOption(const char* arg) const
{
  return ((arg[0] == '-') || (strcmp(arg, "/V") == 0) ||
          (strcmp(arg, "/?") == 0));
}
