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
#include "cmGlobalGenerator.h"
#include "cmLocalVisualStudio6Generator.h"
#include "cmMakefile.h"
#include "cmSystemTools.h"
#include "cmSourceFile.h"
#include "cmCacheManager.h"
#include "cmake.h"

#include <cmsys/RegularExpression.hxx>

cmLocalVisualStudio6Generator::cmLocalVisualStudio6Generator()
{
}

cmLocalVisualStudio6Generator::~cmLocalVisualStudio6Generator()
{
}


void cmLocalVisualStudio6Generator::Generate()
{ 
  std::set<cmStdString> lang;
  lang.insert("C");
  lang.insert("CXX");
  this->CreateCustomTargetsAndCommands(lang);
  this->OutputDSPFile();
}

void cmLocalVisualStudio6Generator::OutputDSPFile()
{ 
  // If not an in source build, then create the output directory
  if(strcmp(this->Makefile->GetStartOutputDirectory(),
            this->Makefile->GetHomeDirectory()) != 0)
    {
    if(!cmSystemTools::MakeDirectory(this->Makefile->GetStartOutputDirectory()))
      {
      cmSystemTools::Error("Error creating directory ",
                           this->Makefile->GetStartOutputDirectory());
      }
    }

  // Setup /I and /LIBPATH options for the resulting DSP file.  VS 6
  // truncates long include paths so make it as short as possible if
  // the length threatents this problem.
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

  // clear project names
  this->CreatedProjectNames.clear();
  // Call TraceVSDependencies on all targets
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

    // INCLUDE_EXTERNAL_MSPROJECT command only affects the workspace
    // so don't build a projectfile for it
    if ((l->second.GetType() != cmTarget::INSTALL_FILES)
        && (l->second.GetType() != cmTarget::INSTALL_PROGRAMS)
        && (strncmp(l->first.c_str(), "INCLUDE_EXTERNAL_MSPROJECT", 26) != 0))
      {
      cmTarget& target = l->second;
      target.TraceVSDependencies(target.GetName(), this->Makefile);
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
      case cmTarget::INSTALL_FILES:
        break;
      case cmTarget::INSTALL_PROGRAMS:
        break;
      default:
        cmSystemTools::Error("Bad target type", l->first.c_str());
        break;
      }
    // INCLUDE_EXTERNAL_MSPROJECT command only affects the workspace
    // so don't build a projectfile for it
    if ((l->second.GetType() != cmTarget::INSTALL_FILES)
        && (l->second.GetType() != cmTarget::INSTALL_PROGRAMS)
        && (strncmp(l->first.c_str(), "INCLUDE_EXTERNAL_MSPROJECT", 26) != 0))
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

void cmLocalVisualStudio6Generator::CreateSingleDSP(const char *lname, cmTarget &target)
{
  // add to the list of projects
  std::string pname = lname;
  this->CreatedProjectNames.push_back(pname);
  // create the dsp.cmake file
  std::string fname;
  fname = this->Makefile->GetStartOutputDirectory();
  fname += "/";
  fname += lname;
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
  this->WriteDSPFile(fout,lname,target);
  fout.close();
  // if the dsp file has changed, then write it.
  cmSystemTools::CopyFileIfDifferent(fname.c_str(), realDSP.c_str());
}


void cmLocalVisualStudio6Generator::AddDSPBuildRule(cmTarget& tgt)
{
  std::string dspname = tgt.GetName();
  dspname += ".dsp.cmake";
  const char* dsprule = this->Makefile->GetRequiredDefinition("CMAKE_COMMAND");
  cmCustomCommandLine commandLine;
  commandLine.push_back(dsprule);
  std::string makefileIn = this->Makefile->GetStartDirectory();
  makefileIn += "/";
  makefileIn += "CMakeLists.txt";
  std::string comment = "Building Custom Rule ";
  comment += makefileIn;
  std::string args;
  args = "-H";
  args +=
    this->Convert(this->Makefile->GetHomeDirectory(),START_OUTPUT, SHELL, true);
  commandLine.push_back(args);
  args = "-B";
  args += 
    this->Convert(this->Makefile->GetHomeOutputDirectory(), 
                  START_OUTPUT, SHELL, true);
  commandLine.push_back(args);

  std::string configFile = 
    this->Makefile->GetRequiredDefinition("CMAKE_ROOT");
  configFile += "/Templates/CMakeWindowsSystemConfig.cmake";
  std::vector<std::string> listFiles = this->Makefile->GetListFiles();
  bool found = false;
  for(std::vector<std::string>::iterator i = listFiles.begin();
      i != listFiles.end(); ++i)
    {
    if(*i == configFile)
      {
      found  = true;
      }
    }
  if(!found)
    {
    listFiles.push_back(configFile);
    }

  cmCustomCommandLines commandLines;
  commandLines.push_back(commandLine);
  const char* no_working_directory = 0;
  this->Makefile->AddCustomCommandToOutput(dspname.c_str(), listFiles,
                                           makefileIn.c_str(), commandLines,
                                           comment.c_str(),
                                           no_working_directory, true);
  if(cmSourceFile* file = this->Makefile->GetSource(makefileIn.c_str()))
    {
    tgt.GetSourceFiles().push_back(file);
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
  
  // trace the visual studio dependencies
  std::string name = libName;
  name += ".dsp.cmake";

  // We may be modifying the source groups temporarily, so make a copy.
  std::vector<cmSourceGroup> sourceGroups = this->Makefile->GetSourceGroups();
  
  // get the classes from the source lists then add them to the groups
  std::vector<cmSourceFile*> & classes = target.GetSourceFiles();

  // now all of the source files have been properly assigned to the target
  // now stick them into source groups using the reg expressions
  for(std::vector<cmSourceFile*>::iterator i = classes.begin(); 
      i != classes.end(); i++)
    {
    // Add the file to the list of sources.
    std::string source = (*i)->GetFullPath();
    cmSourceGroup& sourceGroup = this->Makefile->FindSourceGroup(source.c_str(),
                                                             sourceGroups);
    sourceGroup.AssignSource(*i);
    // while we are at it, if it is a .rule file then for visual studio 6 we
    // must generate it
    if ((*i)->GetSourceExtension() == "rule")
      {
      if(!cmSystemTools::FileExists(source.c_str()))
        {
        cmSystemTools::ReplaceString(source, "$(IntDir)/", "");
#if defined(_WIN32) || defined(__CYGWIN__)
        std::ofstream fout(source.c_str(), 
                           std::ios::binary | std::ios::out | std::ios::trunc);
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

void cmLocalVisualStudio6Generator::WriteGroup(const cmSourceGroup *sg, cmTarget target, std::ostream &fout, const char *libName)
{
  const std::vector<const cmSourceFile *> &sourceFiles = 
    sg->GetSourceFiles();
  // If the group is empty, don't write it at all.
        
  if(sourceFiles.empty())
    { 
    return; 
    }
    
  // If the group has a name, write the header.
  std::string name = sg->GetName();
  if(name != "")
    {
    this->WriteDSPBeginGroup(fout, name.c_str(), "");
    }
    
  // Loop through each source in the source group.
  for(std::vector<const cmSourceFile *>::const_iterator sf =
        sourceFiles.begin(); sf != sourceFiles.end(); ++sf)
    {
    std::string source = (*sf)->GetFullPath();
    const cmCustomCommand *command = 
      (*sf)->GetCustomCommand();
    std::string compileFlags;
    std::vector<std::string> depends;

    // Add per-source file flags.
    if(const char* cflags = (*sf)->GetProperty("COMPILE_FLAGS"))
      {
      compileFlags += cflags;
      }

    const char* lang = 
      this->GlobalGenerator->GetLanguageFromExtension((*sf)->GetSourceExtension().c_str());
    if(lang && strcmp(lang, "CXX") == 0)
      {
      // force a C++ file type
      compileFlags += " /TP ";
      }
      
    // Check for extra object-file dependencies.
    const char* dependsValue = (*sf)->GetProperty("OBJECT_DEPENDS");
    if(dependsValue)
      {
      cmSystemTools::ExpandListArgument(dependsValue, depends);
      }
    if (source != libName || target.GetType() == cmTarget::UTILITY ||
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
        std::string script =
          this->ConstructScript(command->GetCommandLines(), 
                                command->GetWorkingDirectory(),
                                "\\\n\t");
        std::string comment =
          this->ConstructComment(*command,
                                 "Building Custom Rule $(InputPath)");
        if(comment == "<hack>")
          {
          comment = "";
          }
        const char* flags = compileFlags.size() ? compileFlags.c_str(): 0;
        this->WriteCustomRule(fout, source.c_str(), script.c_str(),
                              comment.c_str(), command->GetDepends(),
                              command->GetOutputs(), flags);
        }
      else if(compileFlags.size())
        {
        for(std::vector<std::string>::iterator i
              = this->Configurations.begin(); i != this->Configurations.end(); ++i)
          { 
          if (i == this->Configurations.begin())
            {
            fout << "!IF  \"$(CFG)\" == " << i->c_str() << std::endl;
            }
          else 
            {
            fout << "!ELSEIF  \"$(CFG)\" == " << i->c_str() << std::endl;
            }
          fout << "\n# ADD CPP " << compileFlags << "\n\n";
          } 
        fout << "!ENDIF\n\n";
        }
      fout << "# End Source File\n";
      }
    }

  std::vector<cmSourceGroup> children  = sg->GetGroupChildren();

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
  target.GetSourceFiles().push_back(outsf);

  // Free the fake output name.
  delete [] output;
}

void
cmLocalVisualStudio6Generator
::WriteCustomRule(std::ostream& fout,
                  const char* source,
                  const char* command,
                  const char* comment,
                  const std::vector<std::string>& depends,
                  const std::vector<std::string>& outputs,
                  const char* flags)
{
  // Write the rule for each configuration.
  std::vector<std::string>::iterator i;
  for(i = this->Configurations.begin(); i != this->Configurations.end(); ++i)
    {
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
    for(std::vector<std::string>::const_iterator d = depends.begin();
        d != depends.end(); ++d)
      {
      // Lookup the real name of the dependency in case it is a CMake target.
      std::string dep = this->GetRealDependency(d->c_str(), i->c_str());
      fout << "\\\n\t" <<
        this->ConvertToOptionallyRelativeOutputPath(dep.c_str());
      }
    fout << "\n";

    fout << "# PROP Ignore_Default_Tool 1\n";
    fout << "# Begin Custom Build -";
    if(comment && *comment)
      {
      fout << " " << comment;
      }
    fout << "\n\n";
    if(outputs.empty())
      {
      fout << source << "_force :  \"$(SOURCE)\" \"$(INTDIR)\" \"$(OUTDIR)\"\n\t";
      fout << command << "\n\n";
      }
    else
      {
      for(std::vector<std::string>::const_iterator o = outputs.begin();
          o != outputs.end(); ++o)
        {
        // Write a rule for every output generated by this command.
        fout << this->ConvertToOptionallyRelativeOutputPath(o->c_str())
             << " :  \"$(SOURCE)\" \"$(INTDIR)\" \"$(OUTDIR)\"\n\t";
        fout << command << "\n\n";
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
  const char *def= this->Makefile->GetDefinition( "MSPROJECT_TEMPLATE_DIRECTORY");

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
  this->Configurations.erase(this->Configurations.begin(), this->Configurations.end());
  // now add all the configurations possible
  std::string line;
  while(cmSystemTools::GetLineFromStream(fin, line))
    {
    cmSystemTools::ReplaceString(line, "OUTPUT_LIBNAME",libName);
    if (reg.find(line))
      {
      this->Configurations.push_back(line.substr(reg.end()));
      }
    }
}

// look for custom rules on a target and collect them together
std::string 
cmLocalVisualStudio6Generator::CreateTargetRules(cmTarget &target, 
                                                 const char * /* libName */)
{
  std::string customRuleCode = "";

  if (target.GetType() >= cmTarget::UTILITY )
    {
    return customRuleCode;
    }

  // are there any rules?
  if (target.GetPreBuildCommands().size() + 
      target.GetPreLinkCommands().size() + 
      target.GetPostBuildCommands().size() == 0)
    {
    return customRuleCode;
    }
    
  customRuleCode = "# Begin Special Build Tool\n";

  // Write the pre-build and pre-link together (VS6 does not support
  // both).  Make sure no continuation character is put on the last
  // line.
  int prelink_total = (static_cast<int>(target.GetPreBuildCommands().size())+
                       static_cast<int>(target.GetPreLinkCommands().size()));
  int prelink_count = 0;
  if(prelink_total > 0)
    {
    // header stuff
    customRuleCode += "PreLink_Cmds=";
    }
  const char* prelink_newline = "\\\n\t";
  for (std::vector<cmCustomCommand>::const_iterator cr =
         target.GetPreBuildCommands().begin();
       cr != target.GetPreBuildCommands().end(); ++cr)
    {
    if(++prelink_count == prelink_total)
      {
      prelink_newline = "";
      }
    customRuleCode += this->ConstructScript(cr->GetCommandLines(),
                                            cr->GetWorkingDirectory(),
                                            prelink_newline);
    }
  for (std::vector<cmCustomCommand>::const_iterator cr =
         target.GetPreLinkCommands().begin();
       cr != target.GetPreLinkCommands().end(); ++cr)
    {
    if(++prelink_count == prelink_total)
      {
      prelink_newline = "";
      }
    customRuleCode += this->ConstructScript(cr->GetCommandLines(),
                                            cr->GetWorkingDirectory(),
                                            prelink_newline);
    }
  if(prelink_total > 0)
    {
    customRuleCode += "\n";
    }

  // Write the post-build rules.  Make sure no continuation character
  // is put on the last line.
  int postbuild_total = static_cast<int>(target.GetPostBuildCommands().size());
  int postbuild_count = 0;
  const char* postbuild_newline = "\\\n\t";
  if(postbuild_total > 0)
    {
    customRuleCode += "PostBuild_Cmds=";
    }
  for (std::vector<cmCustomCommand>::const_iterator cr =
         target.GetPostBuildCommands().begin();
       cr != target.GetPostBuildCommands().end(); ++cr)
    {
    if(++postbuild_count == postbuild_total)
      {
      postbuild_newline = "";
      }
    customRuleCode += this->ConstructScript(cr->GetCommandLines(),
                                            cr->GetWorkingDirectory(),
                                            postbuild_newline);
    }
  if(postbuild_total > 0)
    {
    customRuleCode += "\n";
    }

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

  
void cmLocalVisualStudio6Generator
::WriteDSPHeader(std::ostream& fout, 
                 const char *libName, cmTarget &target, 
                 std::vector<cmSourceGroup> &)
{
  std::set<std::string> pathEmitted;
  
  // determine the link directories
  std::string libOptions;
  std::string libDebugOptions;
  std::string libOptimizedOptions;

  std::string libMultiLineOptions;
  std::string libMultiLineOptionsForDebug;
  std::string libMultiLineDebugOptions;
  std::string libMultiLineOptimizedOptions;

  // suppoirt override in output directory
  std::string libPath = "";
  if (this->Makefile->GetDefinition("LIBRARY_OUTPUT_PATH"))
    {
    libPath = this->Makefile->GetDefinition("LIBRARY_OUTPUT_PATH");
    }
  std::string exePath = "";
  if (this->Makefile->GetDefinition("EXECUTABLE_OUTPUT_PATH"))
    {
    exePath = this->Makefile->GetDefinition("EXECUTABLE_OUTPUT_PATH");
    
    }
  if(libPath.size())
    {
    // make sure there is a trailing slash
    if(libPath[libPath.size()-1] != '/')
      {
      libPath += "/";
      }
    std::string lpath = 
      this->ConvertToOptionallyRelativeOutputPath(libPath.c_str());
    if(lpath.size() == 0)
      {
      lpath = ".";
      }
    std::string lpathIntDir = libPath + "$(INTDIR)";
    lpathIntDir =  this->ConvertToOptionallyRelativeOutputPath(lpathIntDir.c_str());
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
    // make sure there is a trailing slash
    if(exePath[exePath.size()-1] != '/')
      {
      exePath += "/";
      }
    std::string lpath = 
      this->ConvertToOptionallyRelativeOutputPath(exePath.c_str());
    if(lpath.size() == 0)
      {
      lpath = ".";
      }
    std::string lpathIntDir = exePath + "$(INTDIR)";
    lpathIntDir =  this->ConvertToOptionallyRelativeOutputPath(lpathIntDir.c_str());
    
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
    lpathIntDir =  this->ConvertToOptionallyRelativeOutputPath(lpathIntDir.c_str());
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
        (target.GetType()==cmTarget::SHARED_LIBRARY && libName != j->first) ||
        (target.GetType()==cmTarget::MODULE_LIBRARY && libName != j->first))
      {
      // Compute the proper name to use to link this library.
      std::string lib;
      std::string libDebug;
      cmTarget* tgt = this->GlobalGenerator->FindTarget(0, j->first.c_str());
      if(tgt)
        {
        lib = cmSystemTools::GetFilenameWithoutExtension(tgt->GetFullName().c_str());
        libDebug = cmSystemTools::GetFilenameWithoutExtension(tgt->GetFullName("Debug").c_str());
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
      libDebug = this->ConvertToOptionallyRelativeOutputPath(libDebug.c_str());

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
  std::string outputName = "(OUTPUT_NAME is for executables only)";
  std::string extraLinkOptions;
  // TODO: Fix construction of library/executable name through
  // cmTarget.  OUTPUT_LIBNAMEDEBUG_POSTFIX should be replaced by the
  // library's debug configuration name.  OUTPUT_LIBNAME should be
  // replaced by the non-debug configuration name.  This generator
  // should just be re-written to not use template files and just
  // generate the code.  Setting up these substitutions is a pain.
  if(target.GetType() == cmTarget::EXECUTABLE)
    {
    extraLinkOptions = 
      this->Makefile->GetRequiredDefinition("CMAKE_EXE_LINKER_FLAGS");

    // Use the OUTPUT_NAME property if it was set.  This is supported
    // only for executables.
    if(const char* outName = target.GetProperty("OUTPUT_NAME"))
      {
      outputName = outName;
      }
    else
      {
      outputName = target.GetName();
      }
    outputName += ".exe";
    }
  if(target.GetType() == cmTarget::SHARED_LIBRARY)
    {
    extraLinkOptions = this->Makefile->GetRequiredDefinition("CMAKE_SHARED_LINKER_FLAGS");
    }
  if(target.GetType() == cmTarget::MODULE_LIBRARY)
    {
    extraLinkOptions = this->Makefile->GetRequiredDefinition("CMAKE_MODULE_LINKER_FLAGS");
    }

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
  if(const char* stdLibs =  this->Makefile->GetDefinition("CMAKE_STANDARD_LIBRARIES"))
    {
    libOptions += " ";
    libOptions += stdLibs;
    libOptions += " ";
    libMultiLineOptions += "# ADD LINK32 ";
    libMultiLineOptions +=  stdLibs;
    libMultiLineOptions += " \n";
    libMultiLineOptionsForDebug += "# ADD LINK32 ";
    libMultiLineOptionsForDebug +=  stdLibs;
    libMultiLineOptionsForDebug += " \n";
    }
  if(const char* targetLinkFlags = target.GetProperty("LINK_FLAGS"))
    {
    libOptions += " ";
    libOptions += targetLinkFlags;
    libOptions += " ";
    libMultiLineOptions += "# ADD LINK32 ";
    libMultiLineOptions +=  targetLinkFlags;
    libMultiLineOptions += " \n";
    libMultiLineOptionsForDebug += "# ADD LINK32 ";
    libMultiLineOptionsForDebug +=  targetLinkFlags;
    libMultiLineOptionsForDebug += " \n";
    }
  
  // are there any custom rules on the target itself
  // only if the target is a lib or exe
  std::string customRuleCode = this->CreateTargetRules(target, libName);

  std::ifstream fin(this->DSPHeaderTemplate.c_str());
  if(!fin)
    {
    cmSystemTools::Error("Error Reading ", this->DSPHeaderTemplate.c_str());
    }
  std::string staticLibOptions;
  if(target.GetType() == cmTarget::STATIC_LIBRARY )
    { 
    if(const char* libflags = target.GetProperty("STATIC_LIBRARY_FLAGS"))
      {
      staticLibOptions = libflags;
      }
    }
  std::string exportSymbol;
  if (const char* custom_export_name = target.GetProperty("DEFINE_SYMBOL"))
    {
    exportSymbol = custom_export_name;
    }
  else
    {
    std::string in = libName;
    in += "_EXPORTS";
    exportSymbol = cmSystemTools::MakeCindentifier(in.c_str());
    }


  std::string line;
  while(cmSystemTools::GetLineFromStream(fin, line))
    {
    const char* mfcFlag = this->Makefile->GetDefinition("CMAKE_MFC_FLAG");
    if(!mfcFlag)
      {
      mfcFlag = "0";
      }
    cmSystemTools::ReplaceString(line, "OUTPUT_LIBNAME_EXPORTS",
                                 exportSymbol.c_str());
    cmSystemTools::ReplaceString(line, "CMAKE_CUSTOM_RULE_CODE",
                                 customRuleCode.c_str());
    cmSystemTools::ReplaceString(line, "CMAKE_MFC_FLAG",
                                 mfcFlag);
    if(target.GetType() == cmTarget::STATIC_LIBRARY )
      {
      cmSystemTools::ReplaceString(line, "CM_STATIC_LIB_ARGS",
                                   staticLibOptions.c_str());
      } 
    if(this->Makefile->IsOn("CMAKE_VERBOSE_MAKEFILE"))
      {
      cmSystemTools::ReplaceString(line, "/nologo", "");
      }
    
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

    // Replace the template file text OUTPUT_NAME with the real output
    // name that will be used.  Only the executable template should
    // have this text.
    cmSystemTools::ReplaceString(line, "OUTPUT_NAME", outputName.c_str());

    cmSystemTools::ReplaceString(line, "BUILD_INCLUDES",
                                 this->IncludeOptions.c_str());
    cmSystemTools::ReplaceString(line, "OUTPUT_LIBNAME",libName);
    // because LIBRARY_OUTPUT_PATH and EXECUTABLE_OUTPUT_PATH 
    // are already quoted in the template file,
    // we need to remove the quotes here, we still need
    // to convert to output path for unix to win32 conversion 
    cmSystemTools::ReplaceString(line, "LIBRARY_OUTPUT_PATH",
                                 removeQuotes(
                                   this->ConvertToOptionallyRelativeOutputPath(libPath.c_str())).c_str());
    cmSystemTools::ReplaceString(line, "EXECUTABLE_OUTPUT_PATH",
                                 removeQuotes(
                                   this->ConvertToOptionallyRelativeOutputPath(exePath.c_str())).c_str());


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
      const char* linkLanguage = target.GetLinkerLanguage(this->GetGlobalGenerator());
      if(!linkLanguage)
        {
        cmSystemTools::Error("CMake can not determine linker language for target:",
                             target.GetName());
        return;
        }
      // if CXX is on and the target contains cxx code then add the cxx flags
      std::string baseFlagVar = "CMAKE_";
      baseFlagVar += linkLanguage;
      baseFlagVar += "_FLAGS";
      flags = this->Makefile->GetRequiredDefinition(baseFlagVar.c_str());
      
      std::string flagVar = baseFlagVar + "_RELEASE";
      flagsRelease = this->Makefile->GetRequiredDefinition(flagVar.c_str());
      flagsRelease += " -DCMAKE_INTDIR=\\\"Release\\\" ";
      if(const char* targetLinkFlags = target.GetProperty("LINK_FLAGS_RELEASE"))
        {
        flagsRelease += targetLinkFlags;
        flagsRelease += " ";
        }
      flagVar = baseFlagVar + "_MINSIZEREL";
      flagsMinSize = this->Makefile->GetRequiredDefinition(flagVar.c_str());
      flagsMinSize += " -DCMAKE_INTDIR=\\\"MinSizeRel\\\" ";
      if(const char* targetLinkFlags = target.GetProperty("LINK_FLAGS_MINSIZEREL"))
        {
        flagsMinSize += targetLinkFlags;
        flagsMinSize += " ";
        }
      
      flagVar = baseFlagVar + "_DEBUG";
      flagsDebug = this->Makefile->GetRequiredDefinition(flagVar.c_str());
      flagsDebug += " -DCMAKE_INTDIR=\\\"Debug\\\" ";
      if(const char* targetLinkFlags = target.GetProperty("LINK_FLAGS_DEBUG"))
        {
        flagsDebug += targetLinkFlags;
        flagsDebug += " ";
        }

      flagVar = baseFlagVar + "_RELWITHDEBINFO";
      flagsDebugRel = this->Makefile->GetRequiredDefinition(flagVar.c_str());
      flagsDebugRel += " -DCMAKE_INTDIR=\\\"RelWithDebInfo\\\" ";
      if(const char* targetLinkFlags = target.GetProperty("LINK_FLAGS_RELWITHDEBINFO"))
        {
        flagsDebugRel += targetLinkFlags;
        flagsDebugRel += " ";
        }

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

    // The template files have CXX FLAGS in them, that need to be replaced.
    // There are not separate CXX and C template files, so we use the same
    // variable names.   The previous code sets up flags* variables to contain
    // the correct C or CXX flags
    cmSystemTools::ReplaceString(line, "CMAKE_CXX_FLAGS_MINSIZEREL", flagsMinSize.c_str());
    cmSystemTools::ReplaceString(line, "CMAKE_CXX_FLAGS_DEBUG", flagsDebug.c_str());
    cmSystemTools::ReplaceString(line, "CMAKE_CXX_FLAGS_RELWITHDEBINFO", flagsDebugRel.c_str());
    cmSystemTools::ReplaceString(line, "CMAKE_CXX_FLAGS_RELEASE", flagsRelease.c_str());
    cmSystemTools::ReplaceString(line, "CMAKE_CXX_FLAGS", flags.c_str());
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
