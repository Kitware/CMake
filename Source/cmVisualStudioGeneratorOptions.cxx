#include "cmVisualStudioGeneratorOptions.h"
#include "cmSystemTools.h"
#include <cmsys/System.h>
#include "cmVisualStudio10TargetGenerator.h"

inline std::string cmVisualStudio10GeneratorOptionsEscapeForXML(const char* s)
{
  std::string ret = s;
  cmSystemTools::ReplaceString(ret, "&", "&amp;");
  cmSystemTools::ReplaceString(ret, "<", "&lt;");
  cmSystemTools::ReplaceString(ret, ">", "&gt;");
  return ret;
}

inline std::string cmVisualStudioGeneratorOptionsEscapeForXML(const char* s)
{
  std::string ret = s;
  cmSystemTools::ReplaceString(ret, "&", "&amp;");
  cmSystemTools::ReplaceString(ret, "\"", "&quot;");
  cmSystemTools::ReplaceString(ret, "<", "&lt;");
  cmSystemTools::ReplaceString(ret, ">", "&gt;");
  cmSystemTools::ReplaceString(ret, "\n", "&#x0D;&#x0A;");
  return ret;
}

//----------------------------------------------------------------------------
cmVisualStudioGeneratorOptions
::cmVisualStudioGeneratorOptions(cmLocalGenerator* lg,
                                 int version,
                                 Tool tool,
                                 cmVS7FlagTable const* table,
                                 cmVS7FlagTable const* extraTable,
                                 cmVisualStudio10TargetGenerator* g):
  LocalGenerator(lg), Version(version), CurrentTool(tool),
  DoingDefine(false), FlagTable(table), ExtraFlagTable(extraTable),
  TargetGenerator(g)
{
}

//----------------------------------------------------------------------------
void cmVisualStudioGeneratorOptions::FixExceptionHandlingDefault()
{
  // Exception handling is on by default because the platform file has
  // "/EHsc" in the flags.  Normally, that will override this
  // initialization to off, but the user has the option of removing
  // the flag to disable exception handling.  When the user does
  // remove the flag we need to override the IDE default of on.
  switch (this->Version)
    {
    case 7:
    case 71:
      this->FlagMap["ExceptionHandling"] = "FALSE";
      break;
    case 10:
      // by default VS puts <ExceptionHandling></ExceptionHandling> empty
      // for a project, to make our projects look the same put a new line
      // and space over for the closing </ExceptionHandling> as the default
      // value
      this->FlagMap["ExceptionHandling"] = "\n      ";
      break;
    default:
      this->FlagMap["ExceptionHandling"] = "0";
    break;
    }
}

//----------------------------------------------------------------------------
void cmVisualStudioGeneratorOptions::SetVerboseMakefile(bool verbose)
{
  // If verbose makefiles have been requested and the /nologo option
  // was not given explicitly in the flags we want to add an attribute
  // to the generated project to disable logo suppression.  Otherwise
  // the GUI default is to enable suppression.
  if(verbose &&
     this->FlagMap.find("SuppressStartupBanner") == this->FlagMap.end())
    {
    if(this->Version == 10)
      {
      this->FlagMap["SuppressStartupBanner"] = "false";
      }
    else
      {
      this->FlagMap["SuppressStartupBanner"] = "FALSE";
      }
    }
}

//----------------------------------------------------------------------------
void cmVisualStudioGeneratorOptions::AddDefine(const std::string& def)
{
  this->Defines.push_back(def);
}

//----------------------------------------------------------------------------
void cmVisualStudioGeneratorOptions::AddDefines(const char* defines)
{
  if(defines)
    {
    // Expand the list of definitions.
    cmSystemTools::ExpandListArgument(defines, this->Defines);
    }
}

//----------------------------------------------------------------------------
void cmVisualStudioGeneratorOptions::AddFlag(const char* flag,
                                                   const char* value)
{
  this->FlagMap[flag] = value;
}


bool cmVisualStudioGeneratorOptions::IsDebug()
{
  return this->FlagMap.find("DebugInformationFormat") != this->FlagMap.end();
}

//----------------------------------------------------------------------------
bool cmVisualStudioGeneratorOptions::UsingUnicode()
{
  // Look for the a _UNICODE definition.
  for(std::vector<std::string>::const_iterator di = this->Defines.begin();
      di != this->Defines.end(); ++di)
    {
    if(*di == "_UNICODE")
      {
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
void cmVisualStudioGeneratorOptions::Parse(const char* flags)
{
  // Parse the input string as a windows command line since the string
  // is intended for writing directly into the build files.
  std::vector<std::string> args;
  cmSystemTools::ParseWindowsCommandLine(flags, args);

  // Process flags that need to be represented specially in the IDE
  // project file.
  for(std::vector<std::string>::iterator ai = args.begin();
      ai != args.end(); ++ai)
    {
    this->HandleFlag(ai->c_str());
    }
}

//----------------------------------------------------------------------------
void cmVisualStudioGeneratorOptions::HandleFlag(const char* flag)
{
  // If the last option was -D then this option is the definition.
  if(this->DoingDefine)
    {
    this->DoingDefine = false;
    this->Defines.push_back(flag);
    return;
    }

  // Look for known arguments.
  if(flag[0] == '-' || flag[0] == '/')
    {
    // Look for preprocessor definitions.
    if(this->CurrentTool == Compiler && flag[1] == 'D')
      {
      if(flag[2] == '\0')
        {
        // The next argument will have the definition.
        this->DoingDefine = true;
        }
      else
        {
        // Store this definition.
        this->Defines.push_back(flag+2);
        }
      return;
      }

    // Look through the available flag tables.
    bool flag_handled = false;
    if(this->FlagTable &&
       this->CheckFlagTable(this->FlagTable, flag, flag_handled))
      {
      return;
      }
    if(this->ExtraFlagTable &&
       this->CheckFlagTable(this->ExtraFlagTable, flag, flag_handled))
      {
      return;
      }

    // If any map entry handled the flag we are done.
    if(flag_handled)
      {
      return;
      }
    }
  // This option is not known.  Store it in the output flags.
  this->FlagString += " ";
  this->FlagString +=
    cmSystemTools::EscapeWindowsShellArgument(
      flag,
      cmsysSystem_Shell_Flag_AllowMakeVariables |
      cmsysSystem_Shell_Flag_VSIDE);
}

//----------------------------------------------------------------------------
bool
cmVisualStudioGeneratorOptions
::CheckFlagTable(cmVS7FlagTable const* table, const char* flag,
                 bool& flag_handled)
{
  // Look for an entry in the flag table matching this flag.
  for(cmVS7FlagTable const* entry = table; entry->IDEName; ++entry)
    {
    bool entry_found = false;
    if(entry->special & cmVS7FlagTable::UserValue)
      {
      // This flag table entry accepts a user-specified value.  If
      // the entry specifies UserRequired we must match only if a
      // non-empty value is given.
      int n = static_cast<int>(strlen(entry->commandFlag));
      if(strncmp(flag+1, entry->commandFlag, n) == 0 &&
         (!(entry->special & cmVS7FlagTable::UserRequired) ||
          static_cast<int>(strlen(flag+1)) > n))
        {
        if(entry->special & cmVS7FlagTable::UserIgnored)
          {
          // Ignore the user-specified value.
          this->FlagMap[entry->IDEName] = entry->value;
          }
        else if(entry->special & cmVS7FlagTable::SemicolonAppendable)
          {
          const char *new_value = flag+1+n;
          
          std::map<cmStdString,cmStdString>::iterator itr;
          itr = this->FlagMap.find(entry->IDEName);
          if(itr != this->FlagMap.end())
            {
            // Append to old value (if present) with semicolons;
            itr->second += ";";
            itr->second += new_value;
            }
          else
            {
            this->FlagMap[entry->IDEName] = new_value;
            }
          }
        else
          {
          // Use the user-specified value.
          this->FlagMap[entry->IDEName] = flag+1+n;
          }
        entry_found = true;
        }
      }
    else if(strcmp(flag+1, entry->commandFlag) == 0)
      {
      // This flag table entry provides a fixed value.
      this->FlagMap[entry->IDEName] = entry->value;
      entry_found = true;
      }
    
    // If the flag has been handled by an entry not requesting a
    // search continuation we are done.
    if(entry_found && !(entry->special & cmVS7FlagTable::Continue))
      {
      return true;
      }
    
    // If the entry was found the flag has been handled.
    flag_handled = flag_handled || entry_found;
    }
  
  return false;
}


void cmVisualStudioGeneratorOptions::SetConfiguration(const char* config)
{
  this->Configuration = config;
}

//----------------------------------------------------------------------------
void
cmVisualStudioGeneratorOptions
::OutputPreprocessorDefinitions(std::ostream& fout,
                                const char* prefix,
                                const char* suffix)
{
  if(this->Defines.empty())
    {
    return;
    }
  if(this->Version == 10)
    {
    // if there are configuration specifc flags, then
    // use the configuration specific tag for PreprocessorDefinitions
    if(this->Configuration.size())
      {
      fout << prefix;
      this->TargetGenerator->WritePlatformConfigTag(
        "PreprocessorDefinitions",
        this->Configuration.c_str(),
        0,
        0, 0, &fout);
      }
    else
      {
      fout << prefix << "<PreprocessorDefinitions>";
      }
    }
  else
    {
    fout << prefix <<  "PreprocessorDefinitions=\"";
    }
  const char* comma = "";
  for(std::vector<std::string>::const_iterator di = this->Defines.begin();
      di != this->Defines.end(); ++di)
    {
    // Escape the definition for the compiler.
    std::string define;
    if(this->Version != 10)
      {
      define =
        this->LocalGenerator->EscapeForShell(di->c_str(), true);
      }
    else
      {
      define = *di;
      }
    // Escape this flag for the IDE.
    if(this->Version == 10)
      {
      define = cmVisualStudio10GeneratorOptionsEscapeForXML(define.c_str());
      }
    else
      {
      define = cmVisualStudioGeneratorOptionsEscapeForXML(define.c_str());
      }
    // Store the flag in the project file.
    fout << comma << define;
    if(this->Version == 10)
      {
      comma = ";";
      }
    else
      {
      comma = ",";
      }
    }
  if(this->Version == 10)
    {
    fout <<  ";%(PreprocessorDefinitions)</PreprocessorDefinitions>" << suffix;
    }
  else
    {
    fout << "\"" << suffix;
    }
}

//----------------------------------------------------------------------------
void
cmVisualStudioGeneratorOptions
::OutputFlagMap(std::ostream& fout, const char* indent)
{
  if(this->Version == 10)
    {
    for(std::map<cmStdString, cmStdString>::iterator m = this->FlagMap.begin();
        m != this->FlagMap.end(); ++m)
      {
      fout << indent;
      if(this->Configuration.size())
        {
        this->TargetGenerator->WritePlatformConfigTag(
          m->first.c_str(),
          this->Configuration.c_str(),
          0,
          0, 0, &fout);
        }
      else
        {
        fout << "<" << m->first << ">";
        }
      fout  << m->second << "</" << m->first << ">\n";
      }
    }
  else
    {
    for(std::map<cmStdString, cmStdString>::iterator m = this->FlagMap.begin();
        m != this->FlagMap.end(); ++m)
      {
      fout << indent << m->first << "=\"" << m->second << "\"\n";
      }
    }
}

//----------------------------------------------------------------------------
void
cmVisualStudioGeneratorOptions
::OutputAdditionalOptions(std::ostream& fout,
                          const char* prefix,
                          const char* suffix)
{
  if(!this->FlagString.empty())
    {
    if(this->Version == 10)
      { 
      fout << prefix;
      if(this->Configuration.size())
        { 
        this->TargetGenerator->WritePlatformConfigTag(
          "AdditionalOptions",
          this->Configuration.c_str(),
          0,
          0, 0, &fout);
        }
      else
        {
        fout << "<AdditionalOptions>";
        }
      fout << this->FlagString.c_str()
           << " %(AdditionalOptions)</AdditionalOptions>\n";
      }
    else
      {
      fout << prefix << "AdditionalOptions=\"";
      fout <<
        cmVisualStudioGeneratorOptionsEscapeForXML(this->FlagString.c_str());
      fout << "\"" << suffix;
      }
    }
}
