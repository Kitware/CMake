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
#include "cmVCProjWriter.h"
#include "cmStandardIncludes.h"
#include "cmSystemTools.h"
#include "cmRegularExpression.h"


// TODO
// for CommandLine= need to repleace quotes with &quot
// write out configurations

cmVCProjWriter::~cmVCProjWriter()
{
}


cmVCProjWriter::cmVCProjWriter(cmMakefile*mf)
{
  m_Makefile = mf;
  m_Configurations.push_back("Debug");
  m_Configurations.push_back("Release");
  m_Configurations.push_back("MinSizeRel");
  m_Configurations.push_back("RelWithDebInfo");
}

void cmVCProjWriter::OutputVCProjFile()
{ 
  // If not an in source build, then create the output directory
  if(strcmp(m_Makefile->GetStartOutputDirectory(),
            m_Makefile->GetHomeDirectory()) != 0)
    {
    if(!cmSystemTools::MakeDirectory(m_Makefile->GetStartOutputDirectory()))
      {
      cmSystemTools::Error("Error creating directory ",
                           m_Makefile->GetStartOutputDirectory());
      }
    }

  // Setup /I and /LIBPATH options for the resulting VCProj file
  std::vector<std::string>& includes = m_Makefile->GetIncludeDirectories();
  std::vector<std::string>::iterator i;
  for(i = includes.begin(); i != includes.end(); ++i)
    {
    std::string tmp = cmSystemTools::EscapeSpaces(i->c_str());
    cmSystemTools::ConvertToWindowsSlashesAndCleanUp(tmp);
    m_IncludeOptions += ",";
    // quote if not already quoted
    if (tmp[0] != '"')
      {
      m_IncludeOptions += tmp;
      }
    else
      {
      m_IncludeOptions += tmp;
      }
    }
  
  // Create the VCProj or set of VCProj's for libraries and executables

  // clear project names
  m_CreatedProjectNames.clear();

  // build any targets
  cmTargets &tgts = m_Makefile->GetTargets();
  for(cmTargets::iterator l = tgts.begin(); 
      l != tgts.end(); l++)
    {
    switch(l->second.GetType())
      {
      case cmTarget::STATIC_LIBRARY:
        this->SetBuildType(STATIC_LIBRARY, l->first.c_str());
        break;
      case cmTarget::SHARED_LIBRARY:
        this->SetBuildType(DLL, l->first.c_str());
        break;
      case cmTarget::EXECUTABLE:
        this->SetBuildType(EXECUTABLE,l->first.c_str());
        break;
      case cmTarget::WIN32_EXECUTABLE:
        this->SetBuildType(WIN32_EXECUTABLE,l->first.c_str());
        break;
      case cmTarget::UTILITY:
        this->SetBuildType(UTILITY, l->first.c_str());
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
      this->CreateSingleVCProj(l->first.c_str(),l->second);
      }
    }
}

void cmVCProjWriter::CreateSingleVCProj(const char *lname, cmTarget &target)
{
  // add to the list of projects
  std::string pname = lname;
  m_CreatedProjectNames.push_back(pname);
  // create the dsp.cmake file
  std::string fname;
  fname = m_Makefile->GetStartOutputDirectory();
  fname += "/";
  fname += lname;
  fname += ".vcproj";
  // save the name of the real dsp file
  std::string realVCProj = fname;
  fname += ".cmake";
  std::ofstream fout(fname.c_str());
  if(!fout)
    {
    cmSystemTools::Error("Error Writing ", fname.c_str());
    }
  this->WriteVCProjFile(fout,lname,target);
  fout.close();
  // if the dsp file has changed, then write it.
  cmSystemTools::CopyFileIfDifferent(fname.c_str(), realVCProj.c_str());
}


void cmVCProjWriter::AddVCProjBuildRule(cmSourceGroup& sourceGroup)
{
  std::string dspname = *(m_CreatedProjectNames.end()-1);
  if(dspname == "ALL_BUILD")
  {
    return;
  }
  dspname += ".vcproj.cmake";
  std::string makefileIn = m_Makefile->GetStartDirectory();
  makefileIn += "/";
  makefileIn += "CMakeLists.txt";
  makefileIn = cmSystemTools::HandleNetworkPaths(makefileIn.c_str());
  makefileIn = cmSystemTools::EscapeSpaces(makefileIn.c_str());
  std::string dsprule = "${CMAKE_COMMAND}";
  m_Makefile->ExpandVariablesInString(dsprule);
  dsprule = cmSystemTools::HandleNetworkPaths(dsprule.c_str());
  std::string args = makefileIn;
  args += " -H\"";
  args += cmSystemTools::HandleNetworkPaths(m_Makefile->GetHomeDirectory());
  args += "\" -S\"";
  args += cmSystemTools::HandleNetworkPaths(m_Makefile->GetStartDirectory());
  args += "\" -O\"";
  args += cmSystemTools::HandleNetworkPaths(m_Makefile->GetStartOutputDirectory());
  args += "\" -B\"";
  args += cmSystemTools::HandleNetworkPaths(m_Makefile->GetHomeOutputDirectory());
  args += "\"";
  m_Makefile->ExpandVariablesInString(args);

  std::string configFile = 
    m_Makefile->GetDefinition("CMAKE_ROOT");
  configFile += "/Templates/CMakeWindowsSystemConfig.cmake";
  std::vector<std::string> listFiles = m_Makefile->GetListFiles();
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
  
  std::vector<std::string> outputs;
  outputs.push_back(dspname);
  cmCustomCommand cc(makefileIn.c_str(), dsprule.c_str(),
                     args.c_str(),
		     listFiles, 
		     outputs);
  sourceGroup.AddCustomCommand(cc);
}


void cmVCProjWriter::WriteConfigurations(std::ostream& fout, 
                                         const char *libName,
                                         const cmTarget &target)
{
  fout << "\t<Configurations>\n";
  
  fout << "\t</Configurations>\n";
}


void cmVCProjWriter::WriteVCProjFile(std::ostream& fout, 
                                 const char *libName,
                                 cmTarget &target)
{
  // We may be modifying the source groups temporarily, so make a copy.
  std::vector<cmSourceGroup> sourceGroups = m_Makefile->GetSourceGroups();
  
  // get the classes from the source lists then add them to the groups
  std::vector<cmSourceFile> classes = target.GetSourceFiles();
  for(std::vector<cmSourceFile>::iterator i = classes.begin(); 
      i != classes.end(); i++)
    {
    // Add the file to the list of sources.
    std::string source = i->GetFullPath();
    cmSourceGroup& sourceGroup = m_Makefile->FindSourceGroup(source.c_str(),
                                                             sourceGroups);
    sourceGroup.AddSource(source.c_str());
    }
  
  // add any custom rules to the source groups
  for (std::vector<cmCustomCommand>::const_iterator cr = 
         target.GetCustomCommands().begin(); 
       cr != target.GetCustomCommands().end(); ++cr)
    {
    cmSourceGroup& sourceGroup = 
      m_Makefile->FindSourceGroup(cr->GetSourceName().c_str(),
                                  sourceGroups);
    cmCustomCommand cc(*cr);
    cc.ExpandVariables(*m_Makefile);
    sourceGroup.AddCustomCommand(cc);
    }
  
  // open the project
  this->WriteProjectStart(fout, libName, target, sourceGroups);
  // write the configuration information
  this->WriteConfigurations(fout, libName, target);

  fout << "\t<Files>\n";
  // Find the group in which the CMakeLists.txt source belongs, and add
  // the rule to generate this VCProj file.
  for(std::vector<cmSourceGroup>::reverse_iterator sg = sourceGroups.rbegin();
      sg != sourceGroups.rend(); ++sg)
    {
    if(sg->Matches("CMakeLists.txt"))
      {
      this->AddVCProjBuildRule(*sg);
      break;
      }    
    }
  

  // Loop through every source group.
  for(std::vector<cmSourceGroup>::const_iterator sg = sourceGroups.begin();
      sg != sourceGroups.end(); ++sg)
    {
    const cmSourceGroup::BuildRules& buildRules = sg->GetBuildRules();
    // If the group is empty, don't write it at all.
    if(buildRules.empty())
      { continue; }
    
    // If the group has a name, write the header.
    std::string name = sg->GetName();
    if(name != "")
      {
      this->WriteVCProjBeginGroup(fout, name.c_str(), "");
      }
    
    // Loop through each build rule in the source group.
    for(cmSourceGroup::BuildRules::const_iterator cc =
          buildRules.begin(); cc != buildRules.end(); ++ cc)
      {
      std::string source = cc->first;
      const cmSourceGroup::Commands& commands = cc->second;

      if (source != libName || target.GetType() == cmTarget::UTILITY)
        {
        fout << "\t\t\t<File\n";
        
        // Tell MS-Dev what the source is.  If the compiler knows how to
        // build it, then it will.
        fout << "\t\t\t\tRelativePath=\"" << cmSystemTools::EscapeSpaces(source.c_str()) << "\">\n";
        if (!commands.empty())
          {
          cmSourceGroup::CommandFiles totalCommand;
          std::string totalCommandStr;
          totalCommandStr = this->CombineCommands(commands, totalCommand,
                                                  source.c_str());
          this->WriteCustomRule(fout, source.c_str(), totalCommandStr.c_str(), 
                                totalCommand.m_Depends, 
                                totalCommand.m_Outputs);
          }
        fout << "\t\t\t</File>\n";
        }
      }
    
    // If the group has a name, write the footer.
    if(name != "")
      {
      this->WriteVCProjEndGroup(fout);
      }
    }  
  fout << "\t</Files>\n";

  // Write the VCProj file's footer.
  this->WriteVCProjFooter(fout);
}


void cmVCProjWriter::WriteCustomRule(std::ostream& fout,
                                    const char* source,
                                    const char* command,
                                    const std::set<std::string>& depends,
                                    const std::set<std::string>& outputs)
{
  std::string cmd = command;
  cmSystemTools::ReplaceString(cmd, "\"", "&quot;");
  std::vector<std::string>::iterator i;
  for(i = m_Configurations.begin(); i != m_Configurations.end(); ++i)
    {
    fout << "\t\t\t\t<FileConfiguration\n";
    fout << "\t\t\t\t\tName=\"" << *i << "|Win32\">\n";
    fout << "\t\t\t\t\t<Tool\n"
         << "\t\t\t\t\tName=\"VCCustomBuildTool\"\n"
         << "\t\t\t\t\tCommandLine=\"" << cmd << "\n\"\n"
         << "\t\t\t\t\tAdditionalDependencies=\"";
    // Write out the dependencies for the rule.
    std::string temp;
    for(std::set<std::string>::const_iterator d = depends.begin();
	d != depends.end(); ++d)
      {
      temp = *d;
      fout << cmSystemTools::EscapeSpaces(cmSystemTools::ConvertToWindowsSlashes(temp))
           << ";";
      }
    fout << "\"\n";
    fout << "\t\t\t\t\tOutputs=\"";
    bool first = true;
    // Write a rule for every output generated by this command.
    for(std::set<std::string>::const_iterator output = outputs.begin();
        output != outputs.end(); ++output)
      {
      if(!first)
        {
        fout << ";";
        }
      else
        {
        first = true;
        }
      fout << output->c_str();
      }
    fout << "\"/>\n";
    fout << "\t\t\t\t</FileConfiguration>\n";
    }
}


void cmVCProjWriter::WriteVCProjBeginGroup(std::ostream& fout, 
					const char* group,
					const char* filter)
{
  fout << "\t\t<Filter\n"
       << "\t\t\tName=\"" << group << "\"\n"
       << "\t\t\tFilter=\"\">\n";
}


void cmVCProjWriter::WriteVCProjEndGroup(std::ostream& fout)
{
  fout << "\t\t</Filter>\n";
}




void cmVCProjWriter::SetBuildType(BuildType b, const char *libName)
{

}

std::string
cmVCProjWriter::CombineCommands(const cmSourceGroup::Commands &commands,
                                cmSourceGroup::CommandFiles &totalCommand,
                                const char *source)
  
{
  // Loop through every custom command generating code from the
  // current source.
  // build up the depends and outputs and commands 
  std::string totalCommandStr = "";
  std::string temp;
  for(cmSourceGroup::Commands::const_iterator c = commands.begin();
      c != commands.end(); ++c)
    {
    temp= c->second.m_Command; 
    cmSystemTools::ConvertToWindowsSlashes(temp);
    temp = cmSystemTools::EscapeSpaces(temp.c_str());
    totalCommandStr += temp;
    totalCommandStr += " ";
    totalCommandStr += c->second.m_Arguments;
    totalCommand.Merge(c->second);
    }      
  // Create a dummy file with the name of the source if it does
  // not exist
  if(totalCommand.m_Outputs.empty())
    { 
    std::string dummyFile = m_Makefile->GetStartOutputDirectory();
    dummyFile += "/";
    dummyFile += source;
    if(!cmSystemTools::FileExists(dummyFile.c_str()))
      {
      std::ofstream fout(dummyFile.c_str());
      fout << "Dummy file created by cmake as unused source for utility command.\n";
      }
    }
  return totalCommandStr;
}


// look for custom rules on a target and collect them together
std::string 
cmVCProjWriter::CreateTargetRules(const cmTarget &target, 
                               const char *libName)
{
  std::string customRuleCode = "";

  if (target.GetType() >= cmTarget::UTILITY)
    {
    return customRuleCode;
    }
  
  // Find the group in which the lix exe custom rules belong
  bool init = false;
  for (std::vector<cmCustomCommand>::const_iterator cr = 
         target.GetCustomCommands().begin(); 
       cr != target.GetCustomCommands().end(); ++cr)
    {
    cmCustomCommand cc(*cr);
    cc.ExpandVariables(*m_Makefile);
    if (cc.GetSourceName() == libName)
      {
      if (!init)
        {
        // header stuff
        customRuleCode = "# Begin Special Build Tool\nPostBuild_Cmds=";
        init = true;
        }
      else
        {
        customRuleCode += "\t";
        }
      customRuleCode += cc.GetCommand() + " " + cc.GetArguments();
      }
    }

  if (init)
    {
    customRuleCode += "\n# End Special Build Tool\n";
    }
  return customRuleCode;
}

void cmVCProjWriter::WriteProjectStart(std::ostream& fout, const char *libName,
                                       const cmTarget &target, 
                                       std::vector<cmSourceGroup> &)
{
  fout << "<?xml version=\"1.0\" encoding = \"Windows-1252\"?>\n"
       << "<VisualStudioProject\n"
       << "\tProjectType=\"Visual C++\"\n"
       << "\tVersion=\"7.00\"\n"
       << "\tName=\"" << libName << "\"\n"
       << "\tSccProjectName=\"\"\n"
       << "\tSccLocalPath=\"\"\n"
       << "\tKeyword=\"AtlProj\">\n"
       << "\t<Platforms>\n"
       << "\t\t<Platform\n\t\t\tName=\"Win32\"/>\n"
       << "\t</Platforms>\n";
}


void cmVCProjWriter::WriteVCProjFooter(std::ostream& fout)
{
  fout << "\t<Globals>\n"
       << "\t</Globals>\n"
       << "</VisualStudioProject>\n";
}
