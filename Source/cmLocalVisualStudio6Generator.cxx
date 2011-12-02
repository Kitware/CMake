/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmGlobalGenerator.h"
#include "cmLocalVisualStudio6Generator.h"
#include "cmMakefile.h"
#include "cmSystemTools.h"
#include "cmSourceFile.h"
#include "cmCacheManager.h"
#include "cmake.h"

#include "cmComputeLinkInformation.h"

#include <cmsys/RegularExpression.hxx>

cmLocalVisualStudio6Generator::cmLocalVisualStudio6Generator():
  cmLocalVisualStudioGenerator(VS6)
{
}

cmLocalVisualStudio6Generator::~cmLocalVisualStudio6Generator()
{
}

//----------------------------------------------------------------------------
// Helper class to write build events.
class cmLocalVisualStudio6Generator::EventWriter
{
public:
  EventWriter(cmLocalVisualStudio6Generator* lg,
              const char* config, std::string& code):
    LG(lg), Config(config), Code(code), First(true) {}
  void Start(const char* event)
    {
    this->First = true;
    this->Event = event;
    }
  void Finish()
    {
    this->Code += (this->First? "" : "\n");
    }
  void Write(std::vector<cmCustomCommand> const& ccs)
    {
    for(std::vector<cmCustomCommand>::const_iterator ci = ccs.begin();
        ci != ccs.end(); ++ci)
      {
      this->Write(*ci);
      }
    }
  void Write(cmCustomCommand const& cc)
    {
    if(this->First)
      {
      this->Code += this->Event + "_Cmds=";
      this->First = false;
      }
    else
      {
      this->Code += "\\\n\t";
      }
    this->Code += this->LG->ConstructScript(cc, this->Config, "\\\n\t");
    }
private:
  cmLocalVisualStudio6Generator* LG;
  const char* Config;
  std::string& Code;
  bool First;
  std::string Event;
};

void cmLocalVisualStudio6Generator::AddHelperCommands()
{
  std::set<cmStdString> lang;
  lang.insert("C");
  lang.insert("CXX");
  this->CreateCustomTargetsAndCommands(lang);
}

void cmLocalVisualStudio6Generator::Generate()
{
  this->OutputDSPFile();
}

void cmLocalVisualStudio6Generator::OutputDSPFile()
{ 
  // If not an in source build, then create the output directory
  if(strcmp(this->Makefile->GetStartOutputDirectory(),
            this->Makefile->GetHomeDirectory()) != 0)
    {
    if(!cmSystemTools::MakeDirectory
       (this->Makefile->GetStartOutputDirectory()))
      {
      cmSystemTools::Error("Error creating directory ",
                           this->Makefile->GetStartOutputDirectory());
      }
    }

  // Setup /I and /LIBPATH options for the resulting DSP file.  VS 6
  // truncates long include paths so make it as short as possible if
  // the length threatens this problem.
  unsigned int maxIncludeLength = 3000;
  bool useShortPath = false;
  for(int j=0; j < 2; ++j)
    {
    std::vector<std::string> includes;
    this->GetIncludeDirectories(includes);
    std::vector<std::string>::iterator i;
    for(i = includes.begin(); i != includes.end(); ++i)
      {
      std::string tmp = 
        this->ConvertToOptionallyRelativeOutputPath(i->c_str());
      if(useShortPath)
        {
        cmSystemTools::GetShortPath(tmp.c_str(), tmp);
        }
      this->IncludeOptions +=  " /I ";

      // quote if not already quoted
      if (tmp[0] != '"')
        {
        this->IncludeOptions += "\"";
        this->IncludeOptions += tmp;
        this->IncludeOptions += "\"";
        }
      else
        {
        this->IncludeOptions += tmp;
        }
      }
    if(j == 0 && this->IncludeOptions.size() > maxIncludeLength)
      {
      this->IncludeOptions = "";
      useShortPath = true;
      }
    else
      {
      break;
      }
    }
  
  // Create the DSP or set of DSP's for libraries and executables

  cmTargets &tgts = this->Makefile->GetTargets(); 
  for(cmTargets::iterator l = tgts.begin(); 
      l != tgts.end(); l++)
    {
    // Add a rule to regenerate the build system when the target
    // specification source changes.
    const char* suppRegenRule =
      this->Makefile->GetDefinition("CMAKE_SUPPRESS_REGENERATION");
    if (!cmSystemTools::IsOn(suppRegenRule))
      {
      this->AddDSPBuildRule(l->second);
      }
    }

  // build any targets
  for(cmTargets::iterator l = tgts.begin(); 
      l != tgts.end(); l++)
    {
    switch(l->second.GetType())
      {
      case cmTarget::STATIC_LIBRARY:
        this->SetBuildType(STATIC_LIBRARY, l->first.c_str(), l->second);
        break;
      case cmTarget::SHARED_LIBRARY:
      case cmTarget::MODULE_LIBRARY:
        this->SetBuildType(DLL, l->first.c_str(), l->second);
        break;
      case cmTarget::EXECUTABLE:
        this->SetBuildType(EXECUTABLE,l->first.c_str(), l->second);
        break;
      case cmTarget::UTILITY:
      case cmTarget::GLOBAL_TARGET:
        this->SetBuildType(UTILITY, l->first.c_str(), l->second);
        break;
      default:
        cmSystemTools::Error("Bad target type", l->first.c_str());
        break;
      }
    // INCLUDE_EXTERNAL_MSPROJECT command only affects the workspace
    // so don't build a projectfile for it
    const char* path = 
      l->second.GetProperty("EXTERNAL_MSPROJECT");
    if(!path)
      {
      // check to see if the dsp is going into a sub-directory
      std::string::size_type pos = l->first.rfind('/');
      if(pos != std::string::npos)
        {
        std::string dir = this->Makefile->GetStartOutputDirectory();
        dir += "/";
        dir += l->first.substr(0, pos);
        if(!cmSystemTools::MakeDirectory(dir.c_str()))
          {
          cmSystemTools::Error("Error creating directory ", dir.c_str());
          }
        }
      this->CreateSingleDSP(l->first.c_str(),l->second);
      }
    }
}

// Utility function to make a valid VS6 *.dsp filename out
// of a CMake target name:
//
extern std::string GetVS6TargetName(const std::string& targetName);

void cmLocalVisualStudio6Generator::CreateSingleDSP(const char *lname, 
                                                    cmTarget &target)
{
  // add to the list of projects
  std::string pname = GetVS6TargetName(lname);

  // create the dsp.cmake file
  std::string fname;
  fname = this->Makefile->GetStartOutputDirectory();
  fname += "/";
  fname += pname;
  fname += ".dsp";
  // save the name of the real dsp file
  std::string realDSP = fname;
  fname += ".cmake";
  std::ofstream fout(fname.c_str());
  if(!fout)
    {
    cmSystemTools::Error("Error Writing ", fname.c_str());
    cmSystemTools::ReportLastSystemError("");
    }
  this->WriteDSPFile(fout,pname.c_str(),target);
  fout.close();
  // if the dsp file has changed, then write it.
  cmSystemTools::CopyFileIfDifferent(fname.c_str(), realDSP.c_str());
}


void cmLocalVisualStudio6Generator::AddDSPBuildRule(cmTarget& tgt)
{
  std::string dspname = GetVS6TargetName(tgt.GetName());
  dspname += ".dsp.cmake";
  const char* dsprule = 
    this->Makefile->GetRequiredDefinition("CMAKE_COMMAND");
  cmCustomCommandLine commandLine;
  commandLine.push_back(dsprule);
  std::string makefileIn = this->Makefile->GetStartDirectory();
  makefileIn += "/";
  makefileIn += "CMakeLists.txt";
  if(!cmSystemTools::FileExists(makefileIn.c_str()))
    {
    return;
    }
  std::string comment = "Building Custom Rule ";
  comment += makefileIn;
  std::string args;
  args = "-H";
  args += this->Convert(this->Makefile->GetHomeDirectory(),
                        START_OUTPUT, UNCHANGED, true);
  commandLine.push_back(args);
  args = "-B";
  args += 
    this->Convert(this->Makefile->GetHomeOutputDirectory(), 
                  START_OUTPUT, UNCHANGED, true);
  commandLine.push_back(args);

  std::vector<std::string> const& listFiles = this->Makefile->GetListFiles();

  cmCustomCommandLines commandLines;
  commandLines.push_back(commandLine);
  const char* no_working_directory = 0;
  this->Makefile->AddCustomCommandToOutput(dspname.c_str(), listFiles,
                                           makefileIn.c_str(), commandLines,
                                           comment.c_str(),
                                           no_working_directory, true);
  if(cmSourceFile* file = this->Makefile->GetSource(makefileIn.c_str()))
    {
    tgt.AddSourceFile(file);
    }
  else
    {
    cmSystemTools::Error("Error adding rule for ", makefileIn.c_str());
    }
}


void cmLocalVisualStudio6Generator::WriteDSPFile(std::ostream& fout, 
                                                 const char *libName,
                                                 cmTarget &target)
{
  // For utility targets need custom command since pre- and post-
  // build does not do anything in Visual Studio 6.  In order for the
  // rules to run in the correct order as custom commands, we need
  // special care for dependencies.  The first rule must depend on all
  // the dependencies of all the rules.  The later rules must each
  // depend only on the previous rule.
  if ((target.GetType() == cmTarget::UTILITY ||
      target.GetType() == cmTarget::GLOBAL_TARGET) &&
      (!target.GetPreBuildCommands().empty() ||
       !target.GetPostBuildCommands().empty()))
    {
    // Accumulate the dependencies of all the commands.
    std::vector<std::string> depends;
    for (std::vector<cmCustomCommand>::const_iterator cr =
           target.GetPreBuildCommands().begin();
         cr != target.GetPreBuildCommands().end(); ++cr)
      {
      depends.insert(depends.end(),
                     cr->GetDepends().begin(), cr->GetDepends().end());
      }
    for (std::vector<cmCustomCommand>::const_iterator cr =
           target.GetPostBuildCommands().begin();
         cr != target.GetPostBuildCommands().end(); ++cr)
      {
      depends.insert(depends.end(),
                     cr->GetDepends().begin(), cr->GetDepends().end());
      }

    // Add the pre- and post-build commands in order.
    int count = 1;
    for (std::vector<cmCustomCommand>::const_iterator cr =
           target.GetPreBuildCommands().begin();
         cr != target.GetPreBuildCommands().end(); ++cr)
      {
      this->AddUtilityCommandHack(target, count++, depends, *cr);
      }
    for (std::vector<cmCustomCommand>::const_iterator cr =
           target.GetPostBuildCommands().begin();
         cr != target.GetPostBuildCommands().end(); ++cr)
      {
      this->AddUtilityCommandHack(target, count++, depends, *cr);
      }
    }
  
  // We may be modifying the source groups temporarily, so make a copy.
  std::vector<cmSourceGroup> sourceGroups = this->Makefile->GetSourceGroups();
  
  // get the classes from the source lists then add them to the groups
  std::vector<cmSourceFile*> const & classes = target.GetSourceFiles();

  // now all of the source files have been properly assigned to the target
  // now stick them into source groups using the reg expressions
  for(std::vector<cmSourceFile*>::const_iterator i = classes.begin(); 
      i != classes.end(); i++)
    {
    // Add the file to the list of sources.
    std::string source = (*i)->GetFullPath();
    cmSourceGroup& sourceGroup = 
      this->Makefile->FindSourceGroup(source.c_str(), sourceGroups);
    sourceGroup.AssignSource(*i);
    // while we are at it, if it is a .rule file then for visual studio 6 we
    // must generate it
    if ((*i)->GetExtension() == "rule")
      {
      if(!cmSystemTools::FileExists(source.c_str()))
        {
        cmSystemTools::ReplaceString(source, "$(IntDir)/", "");
#if defined(_WIN32) || defined(__CYGWIN__)
        std::ofstream fout(source.c_str(), 
                           std::ios::binary | std::ios::out 
                           | std::ios::trunc);
#else
        std::ofstream fout(source.c_str(), 
                           std::ios::out | std::ios::trunc);
#endif
        if(fout)
          {
          fout.write("# generated from CMake",22);
          fout.flush();
          fout.close();
          }
        }
      }
    }

  // Compute which sources need unique object computation.
  this->ComputeObjectNameRequirements(sourceGroups);
  
  // Write the DSP file's header.
  this->WriteDSPHeader(fout, libName, target, sourceGroups);
  

  // Loop through every source group.
  for(std::vector<cmSourceGroup>::const_iterator sg = sourceGroups.begin();
      sg != sourceGroups.end(); ++sg)
    {
    this->WriteGroup(&(*sg), target, fout, libName);
    }  

  // Write the DSP file's footer.
  this->WriteDSPFooter(fout);
}

void cmLocalVisualStudio6Generator
::WriteGroup(const cmSourceGroup *sg, cmTarget& target,
             std::ostream &fout, const char *libName)
{
  const std::vector<const cmSourceFile *> &sourceFiles = 
    sg->GetSourceFiles();
  // If the group is empty, don't write it at all.
        
  if(sourceFiles.empty() && sg->GetGroupChildren().empty())
    { 
    return; 
    }
    
  // If the group has a name, write the header.
  std::string name = sg->GetName();
  if(name != "")
    {
    this->WriteDSPBeginGroup(fout, name.c_str(), "");
    }

  // Compute the maximum length configuration name.
  std::string config_max;
  for(std::vector<std::string>::iterator i = this->Configurations.begin();
      i != this->Configurations.end(); ++i)
    {
    // Strip the subdirectory name out of the configuration name.
    std::string config = this->GetConfigName(*i);
    if(config.size() > config_max.size())
      {
      config_max = config;
      }
    }

  // Compute the maximum length full path to the intermediate
  // files directory for any configuration.  This is used to construct
  // object file names that do not produce paths that are too long.
  std::string dir_max;
  dir_max += this->Makefile->GetCurrentOutputDirectory();
  dir_max += "/";
  dir_max += config_max;
  dir_max += "/";

  // Loop through each source in the source group.
  for(std::vector<const cmSourceFile *>::const_iterator sf =
        sourceFiles.begin(); sf != sourceFiles.end(); ++sf)
    {
    std::string source = (*sf)->GetFullPath();
    const cmCustomCommand *command = 
      (*sf)->GetCustomCommand();
    std::string compileFlags;
    std::vector<std::string> depends;
    std::string objectNameDir;
    if(this->NeedObjectName.find(*sf) != this->NeedObjectName.end())
      {
      objectNameDir =
        cmSystemTools::GetFilenamePath(
          this->GetObjectFileNameWithoutTarget(*(*sf), dir_max));
      }

    // Add per-source file flags.
    if(const char* cflags = (*sf)->GetProperty("COMPILE_FLAGS"))
      {
      compileFlags += cflags;
      }

    const char* lang = this->GetSourceFileLanguage(*(*sf));
    if(lang)
      {
      if(strcmp(lang, "CXX") == 0)
        {
        // force a C++ file type
        compileFlags += " /TP ";
        }
      else if(strcmp(lang, "C") == 0)
        {
        // force to c file type
        compileFlags += " /TC ";
        }
      }

    // Add per-source and per-configuration preprocessor definitions.
    std::map<cmStdString, cmStdString> cdmap;
    this->AppendDefines(compileFlags,
                        (*sf)->GetProperty("COMPILE_DEFINITIONS"), lang);
    if(const char* cdefs = (*sf)->GetProperty("COMPILE_DEFINITIONS_DEBUG"))
      {
      this->AppendDefines(cdmap["DEBUG"], cdefs, lang);
      }
    if(const char* cdefs = (*sf)->GetProperty("COMPILE_DEFINITIONS_RELEASE"))
      {
      this->AppendDefines(cdmap["RELEASE"], cdefs, lang);
      }
    if(const char* cdefs =
       (*sf)->GetProperty("COMPILE_DEFINITIONS_MINSIZEREL"))
      {
      this->AppendDefines(cdmap["MINSIZEREL"], cdefs, lang);
      }
    if(const char* cdefs =
       (*sf)->GetProperty("COMPILE_DEFINITIONS_RELWITHDEBINFO"))
      {
      this->AppendDefines(cdmap["RELWITHDEBINFO"], cdefs, lang);
      }

    bool excludedFromBuild =
      (lang && (*sf)->GetPropertyAsBool("HEADER_FILE_ONLY"));
      
    // Check for extra object-file dependencies.
    const char* dependsValue = (*sf)->GetProperty("OBJECT_DEPENDS");
    if(dependsValue)
      {
      cmSystemTools::ExpandListArgument(dependsValue, depends);
      }
    if (GetVS6TargetName(source) != libName ||
      target.GetType() == cmTarget::UTILITY ||
      target.GetType() == cmTarget::GLOBAL_TARGET)
      {
      fout << "# Begin Source File\n\n";
        
      // Tell MS-Dev what the source is.  If the compiler knows how to
      // build it, then it will.
      fout << "SOURCE=" << 
        this->ConvertToOptionallyRelativeOutputPath(source.c_str()) << "\n\n";
      if(!depends.empty())
        {
        // Write out the dependencies for the rule.
        fout << "USERDEP__HACK=";
        for(std::vector<std::string>::const_iterator d = depends.begin();
            d != depends.end(); ++d)
          { 
          fout << "\\\n\t" << 
            this->ConvertToOptionallyRelativeOutputPath(d->c_str());
          }
        fout << "\n";
        }
      if (command)
        {
        const char* flags = compileFlags.size() ? compileFlags.c_str(): 0;
        this->WriteCustomRule(fout, source.c_str(), *command, flags);
        }
      else if(!compileFlags.empty() || !objectNameDir.empty() ||
              excludedFromBuild || !cdmap.empty())
        {
        for(std::vector<std::string>::iterator i
              = this->Configurations.begin(); 
            i != this->Configurations.end(); ++i)
          { 
          // Strip the subdirectory name out of the configuration name.
          std::string config = this->GetConfigName(*i);
          if (i == this->Configurations.begin())
            {
            fout << "!IF  \"$(CFG)\" == " << i->c_str() << std::endl;
            }
          else 
            {
            fout << "!ELSEIF  \"$(CFG)\" == " << i->c_str() << std::endl;
            }
          if(excludedFromBuild)
            {
            fout << "# PROP Exclude_From_Build 1\n";
            }
          if(!compileFlags.empty())
            {
            fout << "\n# ADD CPP " << compileFlags << "\n\n";
            }
          std::map<cmStdString, cmStdString>::iterator cdi =
            cdmap.find(cmSystemTools::UpperCase(config));
          if(cdi != cdmap.end() && !cdi->second.empty())
            {
            fout << "\n# ADD CPP " << cdi->second << "\n\n";
            }
          if(!objectNameDir.empty())
            {
            // Setup an alternate object file directory.
            fout << "\n# PROP Intermediate_Dir \""
                 << config << "/" << objectNameDir << "\"\n\n";
            }
          } 
        fout << "!ENDIF\n\n";
        }
      fout << "# End Source File\n";
      }
    }

  std::vector<cmSourceGroup> const& children  = sg->GetGroupChildren();

  for(unsigned int i=0;i<children.size();++i)
    {
    this->WriteGroup(&children[i], target, fout, libName);
    }



    
  // If the group has a name, write the footer.
  if(name != "")
    {
    this->WriteDSPEndGroup(fout);
    }

}


void
cmLocalVisualStudio6Generator
::AddUtilityCommandHack(cmTarget& target, int count,
                        std::vector<std::string>& depends,
                        const cmCustomCommand& origCommand)
{
  // Create a fake output that forces the rule to run.
  char* output = new char[(strlen(this->Makefile->GetStartOutputDirectory()) +
                           strlen(target.GetName()) + 30)];
  sprintf(output,"%s/%s_force_%i", this->Makefile->GetStartOutputDirectory(),
          target.GetName(), count);
  std::string comment = this->ConstructComment(origCommand, "<hack>");

  // Add the rule with the given dependencies and commands.
  const char* no_main_dependency = 0;
  this->Makefile->AddCustomCommandToOutput(output,
                                       depends,
                                       no_main_dependency,
                                       origCommand.GetCommandLines(),
                                       comment.c_str(),
                                       origCommand.GetWorkingDirectory());

  // Replace the dependencies with the output of this rule so that the
  // next rule added will run after this one.
  depends.clear();
  depends.push_back(output);

  // Add a source file representing this output to the project.
  cmSourceFile* outsf = this->Makefile->GetSourceFileWithOutput(output);
  target.AddSourceFile(outsf);

  // Free the fake output name.
  delete [] output;
}

void
cmLocalVisualStudio6Generator
::WriteCustomRule(std::ostream& fout,
                  const char* source,
                  const cmCustomCommand& command,
                  const char* flags)
{
  std::string comment =
    this->ConstructComment(command, "Building Custom Rule $(InputPath)");
  if(comment == "<hack>")
    {
    comment = "";
    }
  
  // Write the rule for each configuration.
  std::vector<std::string>::iterator i;
  for(i = this->Configurations.begin(); i != this->Configurations.end(); ++i)
    {
    std::string config = this->GetConfigName(*i);
    std::string script =
      this->ConstructScript(command, config.c_str(), "\\\n\t");
      
    if (i == this->Configurations.begin())
      {
      fout << "!IF  \"$(CFG)\" == " << i->c_str() << std::endl;
      }
    else 
      {
      fout << "!ELSEIF  \"$(CFG)\" == " << i->c_str() << std::endl;
      }
    if(flags)
      {
      fout << "\n# ADD CPP " << flags << "\n\n";
      }
    // Write out the dependencies for the rule.
    fout << "USERDEP__HACK=";
    for(std::vector<std::string>::const_iterator d = 
          command.GetDepends().begin();
        d != command.GetDepends().end(); 
        ++d)
      {
      // Lookup the real name of the dependency in case it is a CMake target.
      std::string dep;
      if(this->GetRealDependency(d->c_str(), config.c_str(), dep))
        {
        fout << "\\\n\t" <<
          this->ConvertToOptionallyRelativeOutputPath(dep.c_str());
        }
      }
    fout << "\n";

    fout << "# PROP Ignore_Default_Tool 1\n";
    fout << "# Begin Custom Build -";
    if(!comment.empty())
      {
      fout << " " << comment.c_str();
      }
    fout << "\n\n";
    if(command.GetOutputs().empty())
      {
      fout << source 
           << "_force :  \"$(SOURCE)\" \"$(INTDIR)\" \"$(OUTDIR)\"\n\t";
      fout << script.c_str() << "\n\n";
      }
    else
      {
      for(std::vector<std::string>::const_iterator o = 
          command.GetOutputs().begin();
          o != command.GetOutputs().end(); 
          ++o)
        {
        // Write a rule for every output generated by this command.
        fout << this->ConvertToOptionallyRelativeOutputPath(o->c_str())
             << " :  \"$(SOURCE)\" \"$(INTDIR)\" \"$(OUTDIR)\"\n\t";
        fout << script.c_str() << "\n\n";
        }
      }
    fout << "# End Custom Build\n\n";
    }
  
  fout << "!ENDIF\n\n";
}


void cmLocalVisualStudio6Generator::WriteDSPBeginGroup(std::ostream& fout, 
                                                       const char* group,
                                                       const char* filter)
{
  fout << "# Begin Group \"" << group << "\"\n"
    "# PROP Default_Filter \"" << filter << "\"\n";
}


void cmLocalVisualStudio6Generator::WriteDSPEndGroup(std::ostream& fout)
{
  fout << "# End Group\n";
}




void cmLocalVisualStudio6Generator::SetBuildType(BuildType b,
                                                 const char* libName,
                                                 cmTarget& target)
{
  std::string root= this->Makefile->GetRequiredDefinition("CMAKE_ROOT");
  const char *def= 
    this->Makefile->GetDefinition( "MSPROJECT_TEMPLATE_DIRECTORY");

  if( def)
    {
    root = def;
    }
  else
    {
    root += "/Templates";
    }
  
  switch(b)
    {
    case STATIC_LIBRARY:
      this->DSPHeaderTemplate = root;
      this->DSPHeaderTemplate += "/staticLibHeader.dsptemplate";
      this->DSPFooterTemplate = root;
      this->DSPFooterTemplate += "/staticLibFooter.dsptemplate";
      break;
    case DLL:
      this->DSPHeaderTemplate =  root;
      this->DSPHeaderTemplate += "/DLLHeader.dsptemplate";
      this->DSPFooterTemplate =  root;
      this->DSPFooterTemplate += "/DLLFooter.dsptemplate";
      break;
    case EXECUTABLE:
      if ( target.GetPropertyAsBool("WIN32_EXECUTABLE") )
        {
        this->DSPHeaderTemplate = root;
        this->DSPHeaderTemplate += "/EXEWinHeader.dsptemplate";
        this->DSPFooterTemplate = root;
        this->DSPFooterTemplate += "/EXEFooter.dsptemplate";
        }
      else
        {
        this->DSPHeaderTemplate = root;
        this->DSPHeaderTemplate += "/EXEHeader.dsptemplate";
        this->DSPFooterTemplate = root;
        this->DSPFooterTemplate += "/EXEFooter.dsptemplate";
        }
      break;
    case UTILITY:
      this->DSPHeaderTemplate = root;
      this->DSPHeaderTemplate += "/UtilityHeader.dsptemplate";
      this->DSPFooterTemplate = root;
      this->DSPFooterTemplate += "/UtilityFooter.dsptemplate";
      break;
    }

  // once the build type is set, determine what configurations are
  // possible
  std::ifstream fin(this->DSPHeaderTemplate.c_str());

  cmsys::RegularExpression reg("# Name ");
  if(!fin)
    {
    cmSystemTools::Error("Error Reading ", this->DSPHeaderTemplate.c_str());
    }

  // reset this->Configurations
  this->Configurations.erase(this->Configurations.begin(), 
                             this->Configurations.end());

  // now add all the configurations possible
  std::string vs6name = GetVS6TargetName(libName);
  std::string line;
  while(cmSystemTools::GetLineFromStream(fin, line))
    {
    cmSystemTools::ReplaceString(line, "OUTPUT_LIBNAME", vs6name.c_str());
    if (reg.find(line))
      {
      this->Configurations.push_back(line.substr(reg.end()));
      }
    }
}

//----------------------------------------------------------------------------
cmsys::auto_ptr<cmCustomCommand>
cmLocalVisualStudio6Generator::MaybeCreateOutputDir(cmTarget& target,
                                                    const char* config)
{
  cmsys::auto_ptr<cmCustomCommand> pcc;

  // VS6 forgets to create the output directory for archives if it
  // differs from the intermediate directory.
  if(target.GetType() != cmTarget::STATIC_LIBRARY) { return pcc; }
  std::string outDir = target.GetDirectory(config, false);

  // Add a pre-link event to create the directory.
  cmCustomCommandLine command;
  command.push_back(this->Makefile->GetRequiredDefinition("CMAKE_COMMAND"));
  command.push_back("-E");
  command.push_back("make_directory");
  command.push_back(outDir);
  std::vector<std::string> no_output;
  std::vector<std::string> no_depends;
  cmCustomCommandLines commands;
  commands.push_back(command);
  pcc.reset(new cmCustomCommand(0, no_output, no_depends, commands, 0, 0));
  pcc->SetEscapeOldStyle(false);
  pcc->SetEscapeAllowMakeVars(true);
  return pcc;
}

// look for custom rules on a target and collect them together
std::string 
cmLocalVisualStudio6Generator::CreateTargetRules(cmTarget &target, 
                                                 const char* configName, 
                                                 const char * /* libName */)
{
  if (target.GetType() >= cmTarget::UTILITY )
    {
    return "";
    }

  std::string customRuleCode = "# Begin Special Build Tool\n";
  EventWriter event(this, configName, customRuleCode);

  // Write the pre-build and pre-link together (VS6 does not support both).
  event.Start("PreLink");
  event.Write(target.GetPreBuildCommands());
  event.Write(target.GetPreLinkCommands());
  cmsys::auto_ptr<cmCustomCommand> pcc(
    this->MaybeCreateImplibDir(target, configName, false));
  if(pcc.get())
    {
    event.Write(*pcc);
    }
  pcc = this->MaybeCreateOutputDir(target, configName);
  if(pcc.get())
    {
    event.Write(*pcc);
    }
  event.Finish();

  // Write the post-build rules.
  event.Start("PostBuild");
  event.Write(target.GetPostBuildCommands());
  event.Finish();

  customRuleCode += "# End Special Build Tool\n";
  return customRuleCode;
}


inline std::string removeQuotes(const std::string& s)
{
  if(s[0] == '\"' && s[s.size()-1] == '\"')
    {
    return s.substr(1, s.size()-2);
    }
  return s;
}

// Code in blocks surrounded by a test for this definition is needed
// only for compatibility with user project's replacement DSP
// templates.  The CMake templates no longer use them.
#define CM_USE_OLD_VS6

void cmLocalVisualStudio6Generator
::WriteDSPHeader(std::ostream& fout, 
                 const char *libName, cmTarget &target, 
                 std::vector<cmSourceGroup> &)
{
  bool targetBuilds = (target.GetType() >= cmTarget::EXECUTABLE &&
                       target.GetType() <= cmTarget::MODULE_LIBRARY);
#ifdef CM_USE_OLD_VS6
  // Lookup the library and executable output directories.
  std::string libPath;
  if(this->Makefile->GetDefinition("LIBRARY_OUTPUT_PATH"))
    {
    libPath = this->Makefile->GetDefinition("LIBRARY_OUTPUT_PATH");
    }
  std::string exePath;
  if(this->Makefile->GetDefinition("EXECUTABLE_OUTPUT_PATH"))
    {
    exePath = this->Makefile->GetDefinition("EXECUTABLE_OUTPUT_PATH");
    }

  // Make sure there are trailing slashes.
  if(!libPath.empty())
    {
    if(libPath[libPath.size()-1] != '/')
      {
      libPath += "/";
      }
    }
  if(!exePath.empty())
    {
    if(exePath[exePath.size()-1] != '/')
      {
      exePath += "/";
      }
    }

  std::set<std::string> pathEmitted;
  
  // determine the link directories
  std::string libOptions;
  std::string libDebugOptions;
  std::string libOptimizedOptions;

  std::string libMultiLineOptions;
  std::string libMultiLineOptionsForDebug;
  std::string libMultiLineDebugOptions;
  std::string libMultiLineOptimizedOptions;

  if(libPath.size())
    {
    std::string lpath = 
      this->ConvertToOptionallyRelativeOutputPath(libPath.c_str());
    if(lpath.size() == 0)
      {
      lpath = ".";
      }
    std::string lpathIntDir = libPath + "$(INTDIR)";
    lpathIntDir =  
      this->ConvertToOptionallyRelativeOutputPath(lpathIntDir.c_str());
    if(pathEmitted.insert(lpath).second)
      {
      libOptions += " /LIBPATH:";
      libOptions += lpathIntDir;
      libOptions += " ";
      libOptions += " /LIBPATH:";
      libOptions += lpath;
      libOptions += " ";
      libMultiLineOptions += "# ADD LINK32 /LIBPATH:";
      libMultiLineOptions += lpathIntDir;
      libMultiLineOptions += " ";
      libMultiLineOptions += " /LIBPATH:";
      libMultiLineOptions += lpath;
      libMultiLineOptions += " \n";
      libMultiLineOptionsForDebug += "# ADD LINK32 /LIBPATH:";
      libMultiLineOptionsForDebug += lpathIntDir;
      libMultiLineOptionsForDebug += " ";
      libMultiLineOptionsForDebug += " /LIBPATH:";
      libMultiLineOptionsForDebug += lpath;
      libMultiLineOptionsForDebug += " \n";
      }
    }
  if(exePath.size())
    {
    std::string lpath = 
      this->ConvertToOptionallyRelativeOutputPath(exePath.c_str());
    if(lpath.size() == 0)
      {
      lpath = ".";
      }
    std::string lpathIntDir = exePath + "$(INTDIR)";
    lpathIntDir =
      this->ConvertToOptionallyRelativeOutputPath(lpathIntDir.c_str());
    
    if(pathEmitted.insert(lpath).second)
      {
      libOptions += " /LIBPATH:";
      libOptions += lpathIntDir;
      libOptions += " ";
      libOptions += " /LIBPATH:";
      libOptions += lpath;
      libOptions += " ";
      libMultiLineOptions += "# ADD LINK32 /LIBPATH:";
      libMultiLineOptions += lpathIntDir;
      libMultiLineOptions += " ";
      libMultiLineOptions += " /LIBPATH:";
      libMultiLineOptions += lpath;
      libMultiLineOptions += " \n";
      libMultiLineOptionsForDebug += "# ADD LINK32 /LIBPATH:";
      libMultiLineOptionsForDebug += lpathIntDir;
      libMultiLineOptionsForDebug += " ";
      libMultiLineOptionsForDebug += " /LIBPATH:";
      libMultiLineOptionsForDebug += lpath;
      libMultiLineOptionsForDebug += " \n";
      }
    }
  std::vector<std::string>::const_iterator i;
  const std::vector<std::string>& libdirs = target.GetLinkDirectories();
  for(i = libdirs.begin(); i != libdirs.end(); ++i)
    {
    std::string path = *i;
    if(path[path.size()-1] != '/')
      {
      path += "/";
      }
    std::string lpath = 
      this->ConvertToOptionallyRelativeOutputPath(path.c_str());
    if(lpath.size() == 0)
      {
      lpath = ".";
      }
    std::string lpathIntDir = path + "$(INTDIR)";
    lpathIntDir =
      this->ConvertToOptionallyRelativeOutputPath(lpathIntDir.c_str());
    if(pathEmitted.insert(lpath).second)
      {
      libOptions += " /LIBPATH:";
      libOptions += lpathIntDir;
      libOptions += " ";
      libOptions += " /LIBPATH:";
      libOptions += lpath;
      libOptions += " ";
      
      libMultiLineOptions += "# ADD LINK32 /LIBPATH:";
      libMultiLineOptions += lpathIntDir;
      libMultiLineOptions += " ";
      libMultiLineOptions += " /LIBPATH:";
      libMultiLineOptions += lpath;
      libMultiLineOptions += " \n";
      libMultiLineOptionsForDebug += "# ADD LINK32 /LIBPATH:";
      libMultiLineOptionsForDebug += lpathIntDir;
      libMultiLineOptionsForDebug += " ";
      libMultiLineOptionsForDebug += " /LIBPATH:";
      libMultiLineOptionsForDebug += lpath;
      libMultiLineOptionsForDebug += " \n";
      }
    }
  // find link libraries
  const cmTarget::LinkLibraryVectorType& libs = target.GetLinkLibraries();
  cmTarget::LinkLibraryVectorType::const_iterator j;
  for(j = libs.begin(); j != libs.end(); ++j)
    {
    // add libraries to executables and dlls (but never include
    // a library in a library, bad recursion)
    // NEVER LINK STATIC LIBRARIES TO OTHER STATIC LIBRARIES
    if ((target.GetType() != cmTarget::SHARED_LIBRARY
         && target.GetType() != cmTarget::STATIC_LIBRARY 
         && target.GetType() != cmTarget::MODULE_LIBRARY) || 
        (target.GetType()==cmTarget::SHARED_LIBRARY
         && libName != GetVS6TargetName(j->first)) ||
        (target.GetType()==cmTarget::MODULE_LIBRARY
         && libName != GetVS6TargetName(j->first)))
      {
      // Compute the proper name to use to link this library.
      std::string lib;
      std::string libDebug;
      cmTarget* tgt = this->GlobalGenerator->FindTarget(0, j->first.c_str());
      if(tgt)
        {
        lib = cmSystemTools::GetFilenameWithoutExtension
          (tgt->GetFullName().c_str());
        libDebug = cmSystemTools::GetFilenameWithoutExtension
          (tgt->GetFullName("Debug").c_str());
        lib += ".lib";
        libDebug += ".lib";
        }
      else
        {
        lib = j->first.c_str();
        libDebug = j->first.c_str();
        if(j->first.find(".lib") == std::string::npos)
          {
          lib += ".lib";
          libDebug += ".lib";
          }
        }
      lib = this->ConvertToOptionallyRelativeOutputPath(lib.c_str());
      libDebug = 
        this->ConvertToOptionallyRelativeOutputPath(libDebug.c_str());

      if (j->second == cmTarget::GENERAL)
        {
        libOptions += " ";
        libOptions += lib;
        libMultiLineOptions += "# ADD LINK32 ";
        libMultiLineOptions +=  lib;
        libMultiLineOptions += "\n";
        libMultiLineOptionsForDebug += "# ADD LINK32 ";
        libMultiLineOptionsForDebug +=  libDebug;
        libMultiLineOptionsForDebug += "\n";
        }
      if (j->second == cmTarget::DEBUG)
        {
        libDebugOptions += " ";
        libDebugOptions += lib;

        libMultiLineDebugOptions += "# ADD LINK32 ";
        libMultiLineDebugOptions += libDebug;
        libMultiLineDebugOptions += "\n";
        }
      if (j->second == cmTarget::OPTIMIZED)
        {
        libOptimizedOptions += " ";
        libOptimizedOptions += lib;

        libMultiLineOptimizedOptions += "# ADD LINK32 ";
        libMultiLineOptimizedOptions += lib;
        libMultiLineOptimizedOptions += "\n";
        }      
      }
    }
#endif

  // Get extra linker options for this target type.
  std::string extraLinkOptions;
  std::string extraLinkOptionsDebug;
  std::string extraLinkOptionsRelease;
  std::string extraLinkOptionsMinSizeRel;
  std::string extraLinkOptionsRelWithDebInfo;
  if(target.GetType() == cmTarget::EXECUTABLE)
    {
    extraLinkOptions = 
      this->Makefile->GetRequiredDefinition("CMAKE_EXE_LINKER_FLAGS");
    }
  if(target.GetType() == cmTarget::SHARED_LIBRARY)
    {
    extraLinkOptions = 
      this->Makefile->GetRequiredDefinition("CMAKE_SHARED_LINKER_FLAGS");
    }
  if(target.GetType() == cmTarget::MODULE_LIBRARY)
    {
    extraLinkOptions = 
      this->Makefile->GetRequiredDefinition("CMAKE_MODULE_LINKER_FLAGS");
    }

  // Get extra linker options for this target.
  if(const char* targetLinkFlags = target.GetProperty("LINK_FLAGS"))
    {
    extraLinkOptions += " ";
    extraLinkOptions += targetLinkFlags;
    }

  if(const char* targetLinkFlags = target.GetProperty("LINK_FLAGS_DEBUG"))
    {
    extraLinkOptionsDebug += " ";
    extraLinkOptionsDebug += targetLinkFlags;
    }

  if(const char* targetLinkFlags = target.GetProperty("LINK_FLAGS_RELEASE"))
    {
    extraLinkOptionsRelease += " ";
    extraLinkOptionsRelease += targetLinkFlags;
    }

  if(const char* targetLinkFlags = target.GetProperty("LINK_FLAGS_MINSIZEREL"))
    {
    extraLinkOptionsMinSizeRel += " ";
    extraLinkOptionsMinSizeRel += targetLinkFlags;
    }

  if(const char* targetLinkFlags =
     target.GetProperty("LINK_FLAGS_RELWITHDEBINFO"))
    {
    extraLinkOptionsRelWithDebInfo += " ";
    extraLinkOptionsRelWithDebInfo += targetLinkFlags;
    }




  // Get standard libraries for this language.
  if(targetBuilds)
    {
    // Get the language to use for linking.
    const char* linkLanguage = target.GetLinkerLanguage();
    if(!linkLanguage)
      {
      cmSystemTools::Error
        ("CMake can not determine linker language for target:",
         target.GetName());
      return;
      }

    // Compute the variable name to lookup standard libraries for this
    // language.
    std::string standardLibsVar = "CMAKE_";
    standardLibsVar += linkLanguage;
    standardLibsVar += "_STANDARD_LIBRARIES";

    // Add standard libraries.
    if(const char* stdLibs =
       this->Makefile->GetDefinition(standardLibsVar.c_str()))
      {
      extraLinkOptions += " ";
      extraLinkOptions += stdLibs;
      }
    }

  // Compute version number information.
  std::string targetVersionFlag;
  if(target.GetType() == cmTarget::EXECUTABLE ||
     target.GetType() == cmTarget::SHARED_LIBRARY ||
     target.GetType() == cmTarget::MODULE_LIBRARY)
    {
    int major;
    int minor;
    target.GetTargetVersion(major, minor);
    cmOStringStream targetVersionStream;
    targetVersionStream << "/version:" << major << "." << minor;
    targetVersionFlag = targetVersionStream.str();
    }

  // Compute the real name of the target.
  std::string outputName = 
    "(OUTPUT_NAME is for libraries and executables only)";
  std::string outputNameDebug = outputName;
  std::string outputNameRelease = outputName;
  std::string outputNameMinSizeRel = outputName;
  std::string outputNameRelWithDebInfo = outputName;
  if(target.GetType() == cmTarget::EXECUTABLE ||
     target.GetType() == cmTarget::STATIC_LIBRARY ||
     target.GetType() == cmTarget::SHARED_LIBRARY ||
     target.GetType() == cmTarget::MODULE_LIBRARY)
    {
    outputName = target.GetFullName();
    outputNameDebug = target.GetFullName("Debug");
    outputNameRelease = target.GetFullName("Release");
    outputNameMinSizeRel = target.GetFullName("MinSizeRel");
    outputNameRelWithDebInfo = target.GetFullName("RelWithDebInfo");
    }

  // Compute the output directory for the target.
  std::string outputDirDebug;
  std::string outputDirRelease;
  std::string outputDirMinSizeRel;
  std::string outputDirRelWithDebInfo;
  if(target.GetType() == cmTarget::EXECUTABLE ||
     target.GetType() == cmTarget::STATIC_LIBRARY ||
     target.GetType() == cmTarget::SHARED_LIBRARY ||
     target.GetType() == cmTarget::MODULE_LIBRARY)
    {
    outputDirDebug =
        removeQuotes(this->ConvertToOptionallyRelativeOutputPath(
                       target.GetDirectory("Debug").c_str()));
    outputDirRelease =
        removeQuotes(this->ConvertToOptionallyRelativeOutputPath(
                 target.GetDirectory("Release").c_str()));
    outputDirMinSizeRel =
        removeQuotes(this->ConvertToOptionallyRelativeOutputPath(
                 target.GetDirectory("MinSizeRel").c_str()));
    outputDirRelWithDebInfo =
        removeQuotes(this->ConvertToOptionallyRelativeOutputPath(
                 target.GetDirectory("RelWithDebInfo").c_str()));
    }

  // Compute the proper link information for the target.
  std::string optionsDebug;
  std::string optionsRelease;
  std::string optionsMinSizeRel;
  std::string optionsRelWithDebInfo;
  if(target.GetType() == cmTarget::EXECUTABLE ||
     target.GetType() == cmTarget::SHARED_LIBRARY ||
     target.GetType() == cmTarget::MODULE_LIBRARY)
    {
    extraLinkOptionsDebug =
      extraLinkOptions + " " + extraLinkOptionsDebug;
    extraLinkOptionsRelease =
      extraLinkOptions + " " + extraLinkOptionsRelease;
    extraLinkOptionsMinSizeRel =
      extraLinkOptions + " " + extraLinkOptionsMinSizeRel;
    extraLinkOptionsRelWithDebInfo =
      extraLinkOptions + " " + extraLinkOptionsRelWithDebInfo;
    this->ComputeLinkOptions(target, "Debug", extraLinkOptionsDebug,
                             optionsDebug);
    this->ComputeLinkOptions(target, "Release", extraLinkOptionsRelease,
                             optionsRelease);
    this->ComputeLinkOptions(target, "MinSizeRel", extraLinkOptionsMinSizeRel,
                             optionsMinSizeRel);
    this->ComputeLinkOptions(target, "RelWithDebInfo",
                             extraLinkOptionsRelWithDebInfo,
                             optionsRelWithDebInfo);
    }

  // Compute the path of the import library.
  std::string targetImplibFlagDebug;
  std::string targetImplibFlagRelease;
  std::string targetImplibFlagMinSizeRel;
  std::string targetImplibFlagRelWithDebInfo;
  if(target.GetType() == cmTarget::SHARED_LIBRARY ||
     target.GetType() == cmTarget::MODULE_LIBRARY ||
     target.GetType() == cmTarget::EXECUTABLE)
    {
    std::string fullPathImpDebug = target.GetDirectory("Debug", true);
    std::string fullPathImpRelease = target.GetDirectory("Release", true);
    std::string fullPathImpMinSizeRel =
      target.GetDirectory("MinSizeRel", true);
    std::string fullPathImpRelWithDebInfo =
      target.GetDirectory("RelWithDebInfo", true);
    fullPathImpDebug += "/";
    fullPathImpRelease += "/";
    fullPathImpMinSizeRel += "/";
    fullPathImpRelWithDebInfo += "/";
    fullPathImpDebug += target.GetFullName("Debug", true);
    fullPathImpRelease += target.GetFullName("Release", true);
    fullPathImpMinSizeRel += target.GetFullName("MinSizeRel", true);
    fullPathImpRelWithDebInfo += target.GetFullName("RelWithDebInfo", true);

    targetImplibFlagDebug = "/implib:";
    targetImplibFlagRelease = "/implib:";
    targetImplibFlagMinSizeRel = "/implib:";
    targetImplibFlagRelWithDebInfo = "/implib:";
    targetImplibFlagDebug +=
      this->ConvertToOptionallyRelativeOutputPath(fullPathImpDebug.c_str());
    targetImplibFlagRelease +=
      this->ConvertToOptionallyRelativeOutputPath(fullPathImpRelease.c_str());
    targetImplibFlagMinSizeRel +=
      this->ConvertToOptionallyRelativeOutputPath(
        fullPathImpMinSizeRel.c_str());
    targetImplibFlagRelWithDebInfo +=
      this->ConvertToOptionallyRelativeOutputPath(
        fullPathImpRelWithDebInfo.c_str());
    }

#ifdef CM_USE_OLD_VS6
  // Compute link information for the target.
  if(extraLinkOptions.size())
    {
    libOptions += " ";
    libOptions += extraLinkOptions;
    libOptions += " ";
    libMultiLineOptions += "# ADD LINK32 ";
    libMultiLineOptions +=  extraLinkOptions;
    libMultiLineOptions += " \n";
    libMultiLineOptionsForDebug += "# ADD LINK32 ";
    libMultiLineOptionsForDebug +=  extraLinkOptions;
    libMultiLineOptionsForDebug += " \n";
    }
#endif

  // are there any custom rules on the target itself
  // only if the target is a lib or exe
  std::string customRuleCodeRelease        
      = this->CreateTargetRules(target, "RELEASE",        libName);
  std::string customRuleCodeDebug          
      = this->CreateTargetRules(target, "DEBUG",          libName);
  std::string customRuleCodeMinSizeRel     
      = this->CreateTargetRules(target, "MINSIZEREL",     libName);
  std::string customRuleCodeRelWithDebInfo 
      = this->CreateTargetRules(target, "RELWITHDEBINFO", libName);

  std::ifstream fin(this->DSPHeaderTemplate.c_str());
  if(!fin)
    {
    cmSystemTools::Error("Error Reading ", this->DSPHeaderTemplate.c_str());
    }
  std::string staticLibOptions;
  std::string staticLibOptionsDebug;
  std::string staticLibOptionsRelease;
  std::string staticLibOptionsMinSizeRel;
  std::string staticLibOptionsRelWithDebInfo;
  if(target.GetType() == cmTarget::STATIC_LIBRARY )
    { 
    if(const char* libflags = target.GetProperty("STATIC_LIBRARY_FLAGS"))
      {
      staticLibOptions = libflags;
      staticLibOptionsDebug = libflags;
      staticLibOptionsRelease = libflags;
      staticLibOptionsMinSizeRel = libflags;
      staticLibOptionsRelWithDebInfo = libflags;
      }
    if(const char* libflagsDebug =
       target.GetProperty("STATIC_LIBRARY_FLAGS_DEBUG"))
      {
      staticLibOptionsDebug += " ";
      staticLibOptionsDebug = libflagsDebug;
      }
    if(const char* libflagsRelease =
       target.GetProperty("STATIC_LIBRARY_FLAGS_RELEASE"))
      {
      staticLibOptionsRelease += " ";
      staticLibOptionsRelease = libflagsRelease;
      }
    if(const char* libflagsMinSizeRel =
       target.GetProperty("STATIC_LIBRARY_FLAGS_MINSIZEREL"))
      {
      staticLibOptionsMinSizeRel += " ";
      staticLibOptionsMinSizeRel = libflagsMinSizeRel;
      }
    if(const char* libflagsRelWithDebInfo =
       target.GetProperty("STATIC_LIBRARY_FLAGS_RELWITHDEBINFO"))
      {
      staticLibOptionsRelWithDebInfo += " ";
      staticLibOptionsRelWithDebInfo = libflagsRelWithDebInfo;
      }
    }

  // Add the export symbol definition for shared library objects.
  std::string exportSymbol;
  if(const char* exportMacro = target.GetExportMacro())
    {
    exportSymbol = exportMacro;
    }

  std::string line;
  std::string libnameExports;
  if(exportSymbol.size())
    {
    libnameExports = "/D \"";
    libnameExports += exportSymbol;
    libnameExports += "\"";
    }
  while(cmSystemTools::GetLineFromStream(fin, line))
    {
    const char* mfcFlag = this->Makefile->GetDefinition("CMAKE_MFC_FLAG");
    if(!mfcFlag)
      {
      mfcFlag = "0";
      }
    cmSystemTools::ReplaceString(line, "OUTPUT_LIBNAME_EXPORTS",
                                 libnameExports.c_str());
    cmSystemTools::ReplaceString(line, "CMAKE_MFC_FLAG",
                                 mfcFlag);
    if(target.GetType() == cmTarget::STATIC_LIBRARY )
      {
      cmSystemTools::ReplaceString(line, "CM_STATIC_LIB_ARGS_DEBUG",
                                   staticLibOptionsDebug.c_str());
      cmSystemTools::ReplaceString(line, "CM_STATIC_LIB_ARGS_RELEASE",
                                   staticLibOptionsRelease.c_str());
      cmSystemTools::ReplaceString(line, "CM_STATIC_LIB_ARGS_MINSIZEREL",
                                   staticLibOptionsMinSizeRel.c_str());
      cmSystemTools::ReplaceString(line, "CM_STATIC_LIB_ARGS_RELWITHDEBINFO",
                                   staticLibOptionsRelWithDebInfo.c_str());
      cmSystemTools::ReplaceString(line, "CM_STATIC_LIB_ARGS",
                                   staticLibOptions.c_str());
      } 
    if(this->Makefile->IsOn("CMAKE_VERBOSE_MAKEFILE"))
      {
      cmSystemTools::ReplaceString(line, "/nologo", "");
      }

#ifdef CM_USE_OLD_VS6
    cmSystemTools::ReplaceString(line, "CM_LIBRARIES",
                                 libOptions.c_str());
    cmSystemTools::ReplaceString(line, "CM_DEBUG_LIBRARIES",
                                 libDebugOptions.c_str());
    cmSystemTools::ReplaceString(line, "CM_OPTIMIZED_LIBRARIES",
                                 libOptimizedOptions.c_str());
    cmSystemTools::ReplaceString(line, "CM_MULTILINE_LIBRARIES_FOR_DEBUG",
                                 libMultiLineOptionsForDebug.c_str());
    cmSystemTools::ReplaceString(line, "CM_MULTILINE_LIBRARIES",
                                 libMultiLineOptions.c_str());
    cmSystemTools::ReplaceString(line, "CM_MULTILINE_DEBUG_LIBRARIES",
                                 libMultiLineDebugOptions.c_str());
    cmSystemTools::ReplaceString(line, "CM_MULTILINE_OPTIMIZED_LIBRARIES",
                                 libMultiLineOptimizedOptions.c_str());
#endif
    
    // Substitute the rules for custom command. When specifying just the 
    // target name for the command the command can be different for
    // different configs
    cmSystemTools::ReplaceString(line, "CMAKE_CUSTOM_RULE_CODE_RELEASE",
                                 customRuleCodeRelease.c_str());
    cmSystemTools::ReplaceString(line, "CMAKE_CUSTOM_RULE_CODE_DEBUG",
                                 customRuleCodeDebug.c_str());
    cmSystemTools::ReplaceString(line, "CMAKE_CUSTOM_RULE_CODE_MINSIZEREL",
                                 customRuleCodeMinSizeRel.c_str());
    cmSystemTools::ReplaceString(line, "CMAKE_CUSTOM_RULE_CODE_RELWITHDEBINFO",
                                 customRuleCodeRelWithDebInfo.c_str());

    // Substitute the real output name into the template.
    cmSystemTools::ReplaceString(line, "OUTPUT_NAME_DEBUG", 
                                 outputNameDebug.c_str());
    cmSystemTools::ReplaceString(line, "OUTPUT_NAME_RELEASE", 
                                 outputNameRelease.c_str());
    cmSystemTools::ReplaceString(line, "OUTPUT_NAME_MINSIZEREL",
                                 outputNameMinSizeRel.c_str());
    cmSystemTools::ReplaceString(line, "OUTPUT_NAME_RELWITHDEBINFO", 
                                 outputNameRelWithDebInfo.c_str());
    cmSystemTools::ReplaceString(line, "OUTPUT_NAME", outputName.c_str());

    // Substitute the proper link information into the template.
    cmSystemTools::ReplaceString(line, "CM_MULTILINE_OPTIONS_DEBUG", 
                                 optionsDebug.c_str());
    cmSystemTools::ReplaceString(line, "CM_MULTILINE_OPTIONS_RELEASE", 
                                 optionsRelease.c_str());
    cmSystemTools::ReplaceString(line, "CM_MULTILINE_OPTIONS_MINSIZEREL",
                                 optionsMinSizeRel.c_str());
    cmSystemTools::ReplaceString(line, "CM_MULTILINE_OPTIONS_RELWITHDEBINFO", 
                                 optionsRelWithDebInfo.c_str());

    cmSystemTools::ReplaceString(line, "BUILD_INCLUDES",
                                 this->IncludeOptions.c_str());
    cmSystemTools::ReplaceString(line, "TARGET_VERSION_FLAG",
                                 targetVersionFlag.c_str());
    cmSystemTools::ReplaceString(line, "TARGET_IMPLIB_FLAG_DEBUG",
                                 targetImplibFlagDebug.c_str());
    cmSystemTools::ReplaceString(line, "TARGET_IMPLIB_FLAG_RELEASE",
                                 targetImplibFlagRelease.c_str());
    cmSystemTools::ReplaceString(line, "TARGET_IMPLIB_FLAG_MINSIZEREL",
                                 targetImplibFlagMinSizeRel.c_str());
    cmSystemTools::ReplaceString(line, "TARGET_IMPLIB_FLAG_RELWITHDEBINFO",
                                 targetImplibFlagRelWithDebInfo.c_str());

    std::string vs6name = GetVS6TargetName(libName);
    cmSystemTools::ReplaceString(line, "OUTPUT_LIBNAME", vs6name.c_str());

#ifdef CM_USE_OLD_VS6
    // because LIBRARY_OUTPUT_PATH and EXECUTABLE_OUTPUT_PATH
    // are already quoted in the template file,
    // we need to remove the quotes here, we still need
    // to convert to output path for unix to win32 conversion
    cmSystemTools::ReplaceString
      (line, "LIBRARY_OUTPUT_PATH",
       removeQuotes(this->ConvertToOptionallyRelativeOutputPath
                    (libPath.c_str())).c_str());
    cmSystemTools::ReplaceString
      (line, "EXECUTABLE_OUTPUT_PATH",
       removeQuotes(this->ConvertToOptionallyRelativeOutputPath
                    (exePath.c_str())).c_str());
#endif

    if(targetBuilds)
      {
      cmSystemTools::ReplaceString(line, "OUTPUT_DIRECTORY_DEBUG",
                                   outputDirDebug.c_str());
      cmSystemTools::ReplaceString(line, "OUTPUT_DIRECTORY_RELEASE",
                                   outputDirRelease.c_str());
      cmSystemTools::ReplaceString(line, "OUTPUT_DIRECTORY_MINSIZEREL",
                                   outputDirMinSizeRel.c_str());
      cmSystemTools::ReplaceString(line, "OUTPUT_DIRECTORY_RELWITHDEBINFO",
                                   outputDirRelWithDebInfo.c_str());
#ifdef CM_USE_OLD_VS6
      std::string outPath = target.GetDirectory();
      cmSystemTools::ReplaceString
        (line, "OUTPUT_DIRECTORY",
         removeQuotes(this->ConvertToOptionallyRelativeOutputPath
                      (outPath.c_str())).c_str());
#endif
      }

    cmSystemTools::ReplaceString(line, 
                                 "EXTRA_DEFINES", 
                                 this->Makefile->GetDefineFlags());
    const char* debugPostfix
      = this->Makefile->GetDefinition("CMAKE_DEBUG_POSTFIX");
    cmSystemTools::ReplaceString(line, "DEBUG_POSTFIX", 
                                 debugPostfix?debugPostfix:"");
    // store flags for each configuration
    std::string flags = " ";
    std::string flagsRelease = " ";
    std::string flagsMinSize = " ";
    std::string flagsDebug = " ";
    std::string flagsDebugRel = " ";
    if(target.GetType() >= cmTarget::EXECUTABLE && 
       target.GetType() <= cmTarget::MODULE_LIBRARY)
      {
      const char* linkLanguage = target.GetLinkerLanguage();
      if(!linkLanguage)
        {
        cmSystemTools::Error
          ("CMake can not determine linker language for target:",
           target.GetName());
        return;
        }
      // if CXX is on and the target contains cxx code then add the cxx flags
      std::string baseFlagVar = "CMAKE_";
      baseFlagVar += linkLanguage;
      baseFlagVar += "_FLAGS";
      flags = this->Makefile->GetSafeDefinition(baseFlagVar.c_str());
      
      std::string flagVar = baseFlagVar + "_RELEASE";
      flagsRelease = this->Makefile->GetSafeDefinition(flagVar.c_str());
      flagsRelease += " -DCMAKE_INTDIR=\\\"Release\\\" ";

      flagVar = baseFlagVar + "_MINSIZEREL";
      flagsMinSize = this->Makefile->GetSafeDefinition(flagVar.c_str());
      flagsMinSize += " -DCMAKE_INTDIR=\\\"MinSizeRel\\\" ";

      flagVar = baseFlagVar + "_DEBUG";
      flagsDebug = this->Makefile->GetSafeDefinition(flagVar.c_str());
      flagsDebug += " -DCMAKE_INTDIR=\\\"Debug\\\" ";

      flagVar = baseFlagVar + "_RELWITHDEBINFO";
      flagsDebugRel = this->Makefile->GetSafeDefinition(flagVar.c_str());
      flagsDebugRel += " -DCMAKE_INTDIR=\\\"RelWithDebInfo\\\" ";
      }
    
    // if unicode is not found, then add -D_MBCS
    std::string defs = this->Makefile->GetDefineFlags();
    if(flags.find("D_UNICODE") == flags.npos &&
       defs.find("D_UNICODE") == flags.npos) 
      {
      flags += " /D \"_MBCS\"";
      }

    // Add per-target flags.
    if(const char* targetFlags = target.GetProperty("COMPILE_FLAGS"))
      {
      flags += " ";
      flags += targetFlags;
      }

    // Add per-target and per-configuration preprocessor definitions.
    std::string defines = " ";
    std::string debugDefines = " ";
    std::string releaseDefines = " ";
    std::string minsizeDefines = " ";
    std::string debugrelDefines = " ";

    this->AppendDefines(
      defines,
      this->Makefile->GetProperty("COMPILE_DEFINITIONS"), 0);
    this->AppendDefines(
      debugDefines,
      this->Makefile->GetProperty("COMPILE_DEFINITIONS_DEBUG"),0);
    this->AppendDefines(
      releaseDefines,
      this->Makefile->GetProperty("COMPILE_DEFINITIONS_RELEASE"), 0);
    this->AppendDefines(
      minsizeDefines,
      this->Makefile->GetProperty("COMPILE_DEFINITIONS_MINSIZEREL"), 0);
    this->AppendDefines(
      debugrelDefines,
      this->Makefile->GetProperty("COMPILE_DEFINITIONS_RELWITHDEBINFO"), 0);

    this->AppendDefines(
      defines,
      target.GetProperty("COMPILE_DEFINITIONS"), 0);
    this->AppendDefines(
      debugDefines,
      target.GetProperty("COMPILE_DEFINITIONS_DEBUG"), 0);
    this->AppendDefines(
      releaseDefines,
      target.GetProperty("COMPILE_DEFINITIONS_RELEASE"), 0);
    this->AppendDefines(
      minsizeDefines,
      target.GetProperty("COMPILE_DEFINITIONS_MINSIZEREL"), 0);
    this->AppendDefines(
      debugrelDefines,
      target.GetProperty("COMPILE_DEFINITIONS_RELWITHDEBINFO"), 0);
    flags += defines;
    flagsDebug += debugDefines;
    flagsRelease += releaseDefines;
    flagsMinSize += minsizeDefines;
    flagsDebugRel += debugrelDefines;
 
    // The template files have CXX FLAGS in them, that need to be replaced.
    // There are not separate CXX and C template files, so we use the same
    // variable names.   The previous code sets up flags* variables to contain
    // the correct C or CXX flags
    cmSystemTools::ReplaceString(line, "CMAKE_CXX_FLAGS_MINSIZEREL",
                                 flagsMinSize.c_str());
    cmSystemTools::ReplaceString(line, "CMAKE_CXX_FLAGS_DEBUG", 
                                 flagsDebug.c_str());
    cmSystemTools::ReplaceString(line, "CMAKE_CXX_FLAGS_RELWITHDEBINFO", 
                                 flagsDebugRel.c_str());
    cmSystemTools::ReplaceString(line, "CMAKE_CXX_FLAGS_RELEASE", 
                                 flagsRelease.c_str());
    cmSystemTools::ReplaceString(line, "CMAKE_CXX_FLAGS", flags.c_str());

    cmSystemTools::ReplaceString(line, "COMPILE_DEFINITIONS_MINSIZEREL",
                                 minsizeDefines.c_str());
    cmSystemTools::ReplaceString(line, "COMPILE_DEFINITIONS_DEBUG", 
                                 debugDefines.c_str());
    cmSystemTools::ReplaceString(line, "COMPILE_DEFINITIONS_RELWITHDEBINFO", 
                                 debugrelDefines.c_str());
    cmSystemTools::ReplaceString(line, "COMPILE_DEFINITIONS_RELEASE", 
                                 releaseDefines.c_str());
    cmSystemTools::ReplaceString(line, "COMPILE_DEFINITIONS", defines.c_str());

    fout << line.c_str() << std::endl;
    }
}

void cmLocalVisualStudio6Generator::WriteDSPFooter(std::ostream& fout)
{  
  std::ifstream fin(this->DSPFooterTemplate.c_str());
  if(!fin)
    {
    cmSystemTools::Error("Error Reading ",
                         this->DSPFooterTemplate.c_str());
    }
  std::string line;
  while(cmSystemTools::GetLineFromStream(fin, line))
    {
    fout << line << std::endl;
    }
}

//----------------------------------------------------------------------------
void cmLocalVisualStudio6Generator
::ComputeLinkOptions(cmTarget& target,
                     const char* configName,
                     const std::string extraOptions,
                     std::string& options)
{
  // Compute the link information for this configuration.
  cmComputeLinkInformation* pcli = target.GetLinkInformation(configName);
  if(!pcli)
    {
    return;
    }
  cmComputeLinkInformation& cli = *pcli;
  typedef cmComputeLinkInformation::ItemVector ItemVector;
  ItemVector const& linkLibs = cli.GetItems();
  std::vector<std::string> const& linkDirs = cli.GetDirectories();

  // Build the link options code.
  for(std::vector<std::string>::const_iterator d = linkDirs.begin();
      d != linkDirs.end(); ++d)
    {
    std::string dir = *d;
    if(!dir.empty())
      {
      if(dir[dir.size()-1] != '/')
        {
        dir += "/";
        }
      dir += "$(IntDir)";
      options += "# ADD LINK32 /LIBPATH:";
      options += this->ConvertToOptionallyRelativeOutputPath(dir.c_str());
      options += " /LIBPATH:";
      options += this->ConvertToOptionallyRelativeOutputPath(d->c_str());
      options += "\n";
      }
    }
  for(ItemVector::const_iterator l = linkLibs.begin();
      l != linkLibs.end(); ++l)
    {
    options += "# ADD LINK32 ";
    if(l->IsPath)
      {
      options +=
        this->ConvertToOptionallyRelativeOutputPath(l->Value.c_str());
      }
    else
      {
      options += l->Value;
      }
    options += "\n";
    }

  // Add extra options if any.
  if(!extraOptions.empty())
    {
    options += "# ADD LINK32 ";
    options += extraOptions;
    options += "\n";
    }
}

std::string
cmLocalVisualStudio6Generator
::GetTargetDirectory(cmTarget const&) const
{
  // No per-target directory for this generator (yet).
  return "";
}

void cmLocalVisualStudio6Generator
::GetTargetObjectFileDirectories(cmTarget* ,
                                 std::vector<std::string>& 
                                 dirs)
{
  std::string dir = this->Makefile->GetCurrentOutputDirectory();
  dir += "/";
  dir += this->GetGlobalGenerator()->GetCMakeCFGInitDirectory();
  dirs.push_back(dir);
}

std::string
cmLocalVisualStudio6Generator
::GetConfigName(std::string const& configuration) const
{
  // Strip the subdirectory name out of the configuration name.
  std::string config = configuration;
  std::string::size_type pos = config.find_last_of(" ");
  config = config.substr(pos+1, std::string::npos);
  config = config.substr(0, config.size()-1);
  return config;
}

//----------------------------------------------------------------------------
bool
cmLocalVisualStudio6Generator
::CheckDefinition(std::string const& define) const
{
  // Perform the standard check first.
  if(!this->cmLocalGenerator::CheckDefinition(define))
    {
    return false;
    }

  // Now do the VS6-specific check.
  if(define.find_first_of(" ") != define.npos &&
     define.find_first_of("\"$;") != define.npos)
    {
    cmOStringStream e;
    e << "WARNING: The VS6 IDE does not support preprocessor definition "
      << "values with spaces and '\"', '$', or ';'.\n"
      << "CMake is dropping a preprocessor definition: " << define << "\n"
      << "Consider defining the macro in a (configured) header file.\n";
    cmSystemTools::Message(e.str().c_str());
    return false;
    }

  // Assume it is supported.
  return true;
}
