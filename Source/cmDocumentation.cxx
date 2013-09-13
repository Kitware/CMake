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
#include "cmRST.h"

#include <cmsys/Directory.hxx>
#include <cmsys/Glob.hxx>

#include <ctype.h>

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
{
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
bool cmDocumentation::PrintDocumentation(Type ht, std::ostream& os)
{
  switch (ht)
    {
    case cmDocumentation::Usage:
      return this->PrintDocumentationUsage(os);
    case cmDocumentation::OneManual:
      return this->PrintHelpOneManual(os);
    case cmDocumentation::OneCommand:
      return this->PrintHelpOneCommand(os);
    case cmDocumentation::OneModule:
      return this->PrintHelpOneModule(os);
    case cmDocumentation::OnePolicy:
      return this->PrintHelpOnePolicy(os);
    case cmDocumentation::OneProperty:
      return this->PrintHelpOneProperty(os);
    case cmDocumentation::OneVariable:
      return this->PrintHelpOneVariable(os);
    case cmDocumentation::ListManuals:
      return this->PrintHelpListManuals(os);
    case cmDocumentation::ListCommands:
      return this->PrintHelpListCommands(os);
    case cmDocumentation::ListModules:
      return this->PrintHelpListModules(os);
    case cmDocumentation::ListProperties:
      return this->PrintHelpListProperties(os);
    case cmDocumentation::ListVariables:
      return this->PrintHelpListVariables(os);
    case cmDocumentation::ListPolicies:
      return this->PrintHelpListPolicies(os);
    case cmDocumentation::Version:
      return this->PrintVersion(os);
    default: return false;
    }
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintRequestedDocumentation(std::ostream& os)
{
  int count = 0;
  bool result = true;

  // Loop over requested documentation types.
  for(std::vector<RequestedHelpItem>::const_iterator
      i = this->RequestedHelpItems.begin();
      i != this->RequestedHelpItems.end();
      ++i)
    {
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
    else if(++count > 1)
      {
      os << "\n\n";
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


void cmDocumentation::WarnFormFromFilename(
  cmDocumentation::RequestedHelpItem& request)
{
  std::string ext = cmSystemTools::GetFilenameLastExtension(request.Filename);
  ext = cmSystemTools::UpperCase(ext);
  if ((ext == ".HTM") || (ext == ".HTML"))
    {
    request.HelpType = cmDocumentation::None;
    cmSystemTools::Message("Warning: HTML help format no longer supported");
    }
  else if (ext == ".DOCBOOK")
    {
    request.HelpType = cmDocumentation::None;
    cmSystemTools::Message("Warning: Docbook help format no longer supported");
    }
  // ".1" to ".9" should be manpages
  else if ((ext.length()==2) && (ext[1] >='1') && (ext[1]<='9'))
    {
    request.HelpType = cmDocumentation::None;
    cmSystemTools::Message("Warning: Man help format no longer supported");
    }
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
      GET_OPT_ARGUMENT(help.Argument);
      help.Argument = cmSystemTools::LowerCase(help.Argument);
      // special case for single command
      if (!help.Argument.empty())
        {
        help.HelpType = cmDocumentation::OneCommand;
        }
      }
    else if(strcmp(argv[i], "--help-properties") == 0)
      {
      help.HelpType = cmDocumentation::OneManual;
      help.Argument = "cmake-properties.7";
      GET_OPT_ARGUMENT(help.Filename);
      this->WarnFormFromFilename(help);
      }
    else if(strcmp(argv[i], "--help-policies") == 0)
      {
      help.HelpType = cmDocumentation::OneManual;
      help.Argument = "cmake-policies.7";
      GET_OPT_ARGUMENT(help.Filename);
      this->WarnFormFromFilename(help);
      }
    else if(strcmp(argv[i], "--help-variables") == 0)
      {
      help.HelpType = cmDocumentation::OneManual;
      help.Argument = "cmake-variables.7";
      GET_OPT_ARGUMENT(help.Filename);
      this->WarnFormFromFilename(help);
      }
    else if(strcmp(argv[i], "--help-modules") == 0)
      {
      help.HelpType = cmDocumentation::OneManual;
      help.Argument = "cmake-modules.7";
      GET_OPT_ARGUMENT(help.Filename);
      this->WarnFormFromFilename(help);
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
      help.HelpType = cmDocumentation::OneManual;
      help.Argument = "cmake-commands.7";
      GET_OPT_ARGUMENT(help.Filename);
      this->WarnFormFromFilename(help);
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
      help.HelpType = cmDocumentation::OneCommand;
      GET_OPT_ARGUMENT(help.Argument);
      GET_OPT_ARGUMENT(help.Filename);
      help.Argument = cmSystemTools::LowerCase(help.Argument);
      this->WarnFormFromFilename(help);
      }
    else if(strcmp(argv[i], "--help-module") == 0)
      {
      help.HelpType = cmDocumentation::OneModule;
      GET_OPT_ARGUMENT(help.Argument);
      GET_OPT_ARGUMENT(help.Filename);
      this->WarnFormFromFilename(help);
      }
    else if(strcmp(argv[i], "--help-property") == 0)
      {
      help.HelpType = cmDocumentation::OneProperty;
      GET_OPT_ARGUMENT(help.Argument);
      GET_OPT_ARGUMENT(help.Filename);
      this->WarnFormFromFilename(help);
      }
    else if(strcmp(argv[i], "--help-policy") == 0)
      {
      help.HelpType = cmDocumentation::OnePolicy;
      GET_OPT_ARGUMENT(help.Argument);
      GET_OPT_ARGUMENT(help.Filename);
      this->WarnFormFromFilename(help);
      }
    else if(strcmp(argv[i], "--help-variable") == 0)
      {
      help.HelpType = cmDocumentation::OneVariable;
      GET_OPT_ARGUMENT(help.Argument);
      GET_OPT_ARGUMENT(help.Filename);
      this->WarnFormFromFilename(help);
      }
    else if(strcmp(argv[i], "--help-manual") == 0)
      {
      help.HelpType = cmDocumentation::OneManual;
      GET_OPT_ARGUMENT(help.Argument);
      GET_OPT_ARGUMENT(help.Filename);
      this->WarnFormFromFilename(help);
      }
    else if(strcmp(argv[i], "--help-command-list") == 0)
      {
      help.HelpType = cmDocumentation::ListCommands;
      GET_OPT_ARGUMENT(help.Filename);
      }
    else if(strcmp(argv[i], "--help-module-list") == 0)
      {
      help.HelpType = cmDocumentation::ListModules;
      GET_OPT_ARGUMENT(help.Filename);
      }
    else if(strcmp(argv[i], "--help-property-list") == 0)
      {
      help.HelpType = cmDocumentation::ListProperties;
      GET_OPT_ARGUMENT(help.Filename);
      }
    else if(strcmp(argv[i], "--help-variable-list") == 0)
      {
      help.HelpType = cmDocumentation::ListVariables;
      GET_OPT_ARGUMENT(help.Filename);
      }
    else if(strcmp(argv[i], "--help-policy-list") == 0)
      {
      help.HelpType = cmDocumentation::ListPolicies;
      GET_OPT_ARGUMENT(help.Filename);
      }
    else if(strcmp(argv[i], "--help-manual-list") == 0)
      {
      help.HelpType = cmDocumentation::ListManuals;
      GET_OPT_ARGUMENT(help.Filename);
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
void cmDocumentation::GlobHelp(std::vector<std::string>& files,
                               std::string const& pattern)
{
  cmsys::Glob gl;
  std::string findExpr = this->CMakeRoot + "/Help/" + pattern + ".rst";
  if(gl.FindFiles(findExpr))
    {
    files = gl.GetFiles();
    }
}

//----------------------------------------------------------------------------
void cmDocumentation::PrintNames(std::ostream& os,
                                 std::string const& pattern)
{
  std::vector<std::string> files;
  this->GlobHelp(files, pattern);
  std::vector<std::string> names;
  for (std::vector<std::string>::const_iterator i = files.begin();
       i != files.end(); ++i)
    {
    std::string line;
    std::ifstream fin(i->c_str());
    while(fin && cmSystemTools::GetLineFromStream(fin, line))
      {
      if(!line.empty() && (isalnum(line[0]) || line[0] == '<'))
        {
        names.push_back(line);
        break;
        }
      }
    }
  std::sort(names.begin(), names.end());
  for (std::vector<std::string>::iterator i = names.begin();
       i != names.end(); ++i)
    {
    os << *i << "\n";
    }
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintFiles(std::ostream& os,
                                 std::string const& pattern)
{
  bool found = false;
  std::vector<std::string> files;
  this->GlobHelp(files, pattern);
  std::sort(files.begin(), files.end());
  cmRST r(os, this->CMakeRoot + "/Help");
  for (std::vector<std::string>::const_iterator i = files.begin();
       i != files.end(); ++i)
    {
    found = r.ProcessFile(i->c_str()) || found;
    }
  return found;
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintHelpOneManual(std::ostream& os)
{
  std::string mname = this->CurrentArgument;
  std::string::size_type mlen = mname.length();
  if(mlen > 3 && mname[mlen-3] == '(' &&
                 mname[mlen-1] == ')')
    {
    mname = mname.substr(0, mlen-3) + "." + mname[mlen-2];
    }
  if(this->PrintFiles(os, "manual/" + mname) ||
     this->PrintFiles(os, "manual/" + mname + ".[0-9]"))
    {
    return true;
    }
  // Argument was not a manual.  Complain.
  os << "Argument \"" << this->CurrentArgument.c_str()
     << "\" to --help-manual is not an available manual.  "
     << "Use --help-manual-list to see all available manuals.\n";
  return false;
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintHelpListManuals(std::ostream& os)
{
  this->PrintNames(os, "manual/*");
  return true;
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintHelpOneCommand(std::ostream& os)
{
  std::string cname = cmSystemTools::LowerCase(this->CurrentArgument);
  if(this->PrintFiles(os, "command/" + cname))
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
bool cmDocumentation::PrintHelpListCommands(std::ostream& os)
{
  this->PrintNames(os, "command/*");
  return true;
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintHelpOneModule(std::ostream& os)
{
  std::string mname = this->CurrentArgument;
  if(this->PrintFiles(os, "module/" + mname))
    {
    return true;
    }
  // Argument was not a module.  Complain.
  os << "Argument \"" << this->CurrentArgument.c_str()
     << "\" to --help-module is not a CMake module.\n";
  return false;
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintHelpListModules(std::ostream& os)
{
  std::vector<std::string> files;
  this->GlobHelp(files, "module/*");
  std::vector<std::string> modules;
  for (std::vector<std::string>::iterator fi = files.begin();
       fi != files.end(); ++fi)
    {
    std::string module = cmSystemTools::GetFilenameName(*fi);
    modules.push_back(module.substr(0, module.size()-4));
    }
  std::sort(modules.begin(), modules.end());
  for (std::vector<std::string>::iterator i = modules.begin();
       i != modules.end(); ++i)
    {
    os << *i << "\n";
    }
  return true;
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintHelpOneProperty(std::ostream& os)
{
  std::string pname = cmSystemTools::HelpFileName(this->CurrentArgument);
  if(this->PrintFiles(os, "prop_*/" + pname))
    {
    return true;
    }
  // Argument was not a property.  Complain.
  os << "Argument \"" << this->CurrentArgument.c_str()
     << "\" to --help-property is not a CMake property.  "
     << "Use --help-property-list to see all properties.\n";
  return false;
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintHelpListProperties(std::ostream& os)
{
  this->PrintNames(os, "prop_*/*");
  return true;
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintHelpOnePolicy(std::ostream& os)
{
  std::string pname = this->CurrentArgument;
  std::vector<std::string> files;
  if(this->PrintFiles(os, "policy/" + pname))
    {
    return true;
    }

  // Argument was not a policy.  Complain.
  os << "Argument \"" << this->CurrentArgument.c_str()
     << "\" to --help-policy is not a CMake policy.\n";
  return false;
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintHelpListPolicies(std::ostream& os)
{
  this->PrintNames(os, "policy/*");
  return true;
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintHelpOneVariable(std::ostream& os)
{
  std::string vname = cmSystemTools::HelpFileName(this->CurrentArgument);
  if(this->PrintFiles(os, "variable/" + vname))
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
bool cmDocumentation::PrintHelpListVariables(std::ostream& os)
{
  this->PrintNames(os, "variable/*");
  return true;
}

//----------------------------------------------------------------------------
bool cmDocumentation::PrintDocumentationUsage(std::ostream& os)
{
  std::map<std::string,cmDocumentationSection*>::iterator si;
  si = this->AllSections.find("Usage");
  if(si != this->AllSections.end())
    {
    this->Formatter.PrintSection(os, *si->second);
    }
  si = this->AllSections.find("Options");
  if(si != this->AllSections.end())
    {
    this->Formatter.PrintSection(os, *si->second);
    }
  if(this->ShowGenerators)
    {
    si = this->AllSections.find("Generators");
    if(si != this->AllSections.end())
      {
      this->Formatter.PrintSection(os, *si->second);
      }
    }
  return true;
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
