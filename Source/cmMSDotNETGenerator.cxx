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
#include "cmMSDotNETGenerator.h"
#include "cmStandardIncludes.h"
#include "cmMakefile.h"
#include "cmCacheManager.h"
#include "windows.h"
#include "cmSystemTools.h"
#include "cmRegularExpression.h"
#include "cmSourceGroup.h"




cmMSDotNETGenerator::cmMSDotNETGenerator()
{
  // default to building a sln project file
  BuildProjOn();
}

void cmMSDotNETGenerator::GenerateMakefile()
{
  std::string configTypes = m_Makefile->GetDefinition("CMAKE_CONFIGURATION_TYPES");
  std::string::size_type start = 0;
  std::string::size_type endpos = 0;
  while(endpos != std::string::npos)
    {
    endpos = configTypes.find(' ', start);
    std::string config;
    std::string::size_type len;
    if(endpos != std::string::npos)
      {
      len = endpos - start;
      }
    else
      {
      len = configTypes.size() - start;
      }
    config = configTypes.substr(start, len);
    if(config == "Debug" || config == "Release" ||
       config == "MinSizeRel" || config == "RelWithDebInfo")
      {
      m_Configurations.push_back(config);
      }
    else
      {
      cmSystemTools::Error("Invalid configuration type in CMAKE_CONFIGURATION_TYPES: ",
                           config.c_str(),
                           " (Valid types are Debug,Release,MinSizeRel,RelWithDebInfo)");
      }
    start = endpos+1;
    }
  if(m_Configurations.size() == 0)
    {
    m_Configurations.push_back("Debug");
    m_Configurations.push_back("Release");
    }
  if(m_BuildSLN)
    {
    this->OutputSLNFile();
    }
  else
    {
    this->OutputVCProjFile();
    }
}

cmMSDotNETGenerator::~cmMSDotNETGenerator()
{
}

void cmMSDotNETGenerator::SetLocal(bool local)
{
  m_BuildSLN = !local;
}

void cmMSDotNETGenerator::ComputeSystemInfo()
{
  // now load the settings
  if(!m_Makefile->GetDefinition("CMAKE_ROOT"))
    {
    cmSystemTools::Error(
      "CMAKE_ROOT has not been defined, bad GUI or driver program");
    return;
    }
  std::string fpath = 
    m_Makefile->GetDefinition("CMAKE_ROOT");
  fpath += "/Templates/CMakeDotNetSystemConfig.cmake";
  m_Makefile->ReadListFile(NULL,fpath.c_str());
}



// output the SLN file
void cmMSDotNETGenerator::OutputSLNFile()
{ 
  // if this is an out of source build, create the output directory
  if(strcmp(m_Makefile->GetStartOutputDirectory(),
            m_Makefile->GetHomeDirectory()) != 0)
    {
    if(!cmSystemTools::MakeDirectory(m_Makefile->GetStartOutputDirectory()))
      {
      cmSystemTools::Error("Error creating output directory for SLN file",
                           m_Makefile->GetStartOutputDirectory());
      }
    }
  // create the dsw file name
  std::string fname;
  fname = m_Makefile->GetStartOutputDirectory();
  fname += "/";
  if(strlen(m_Makefile->GetProjectName()) == 0)
    {
    m_Makefile->SetProjectName("Project");
    }
  fname += m_Makefile->GetProjectName();
  fname += ".sln";
  std::ofstream fout(fname.c_str());
  if(!fout)
    {
    cmSystemTools::Error("Error can not open SLN file for write: "
                         ,fname.c_str());
    return;
    }
  this->WriteSLNFile(fout);
}


// Write a SLN file to the stream
void cmMSDotNETGenerator::WriteSLNFile(std::ostream& fout)
{
  // Write out the header for a SLN file
  this->WriteSLNHeader(fout);
  
  // Create a list of cmMakefile created from all the
  // CMakeLists.txt files that are in sub directories of
  // this one.
  std::vector<cmMakefile*> allListFiles;
  // add this makefile to the list
  allListFiles.push_back(m_Makefile);
  // add a special target that depends on ALL projects for easy build
  // of Debug only
  m_Makefile->AddUtilityCommand("ALL_BUILD", "echo", "\"Build all projects\"",
                                false); 
  
  std::string ctest = m_Makefile->GetDefinition("CMAKE_COMMAND");
  ctest = cmSystemTools::GetFilenamePath(ctest.c_str());
  ctest += "/";
  ctest += "ctest";
  ctest += cmSystemTools::GetExecutableExtension();
  if(!cmSystemTools::FileExists(ctest.c_str()))
    {
    ctest = m_Makefile->GetDefinition("CMAKE_COMMAND");
    ctest = cmSystemTools::GetFilenamePath(ctest.c_str());
    ctest += "/Debug/";
    ctest += "ctest";
    ctest += cmSystemTools::GetExecutableExtension();
    }
  if(!cmSystemTools::FileExists(ctest.c_str()))
    {
    ctest = m_Makefile->GetDefinition("CMAKE_COMMAND");
    ctest = cmSystemTools::GetFilenamePath(ctest.c_str());
    ctest += "/Release/";
    ctest += "ctest";
    ctest += cmSystemTools::GetExecutableExtension();
    }
  m_Makefile->AddUtilityCommand("RUN_TESTS", ctest.c_str(), "-D $(IntDir)",
                                false);
  m_Makefile->FindSubDirectoryCMakeListsFiles(allListFiles);
  // For each cmMakefile, create a VCProj for it, and
  // add it to this SLN file
  std::vector<cmMakefile*>::iterator k;
  for(k = allListFiles.begin();
      k != allListFiles.end(); ++k)
    {
    cmMakefile* mf = *k;
    cmMSDotNETGenerator* pg = 0;
    // if not this makefile, then create a new generator
    if(m_Makefile != mf)
      {
      // Create an MS generator with SLN off, so it only creates dsp files
      pg = new cmMSDotNETGenerator;
      }
    else
      {
      pg = static_cast<cmMSDotNETGenerator*>(m_Makefile->GetMakefileGenerator());
      }
    // make sure the generator is building dsp files
    pg->BuildSLNOff();
    mf->SetMakefileGenerator(pg);
    mf->GenerateMakefile();
    // Get the source directory from the makefile
    std::string dir = mf->GetStartDirectory();
    // Get the home directory with the trailing slash
    std::string homedir = m_Makefile->GetHomeDirectory();
    homedir += "/";
    // remove the home directory and / from the source directory
    // this gives a relative path 
    cmSystemTools::ReplaceString(dir, homedir.c_str(), "");

    // Get the list of create dsp files names from the cmVCProjWriter, more
    // than one dsp could have been created per input CMakeLists.txt file
    // for each target
    std::vector<std::string> dspnames = 
      pg->GetCreatedProjectNames();
    cmTargets &tgts = pg->GetMakefile()->GetTargets();
    cmTargets::iterator l = tgts.begin();
    for(std::vector<std::string>::iterator si = dspnames.begin(); 
        l != tgts.end(); ++l)
      {
      // special handling for the current makefile
      if(mf == m_Makefile)
        {
        dir = "."; // no subdirectory for project generated
        // if this is the special ALL_BUILD utility, then
        // make it depend on every other non UTILITY project.
        // This is done by adding the names to the GetUtilities
        // vector on the makefile
        if(l->first == "ALL_BUILD")
          {
          for(std::vector<cmMakefile*>::iterator a = allListFiles.begin();
              a != allListFiles.end(); ++a)
            {
            const cmTargets &atgts = (*a)->GetTargets();
            for(cmTargets::const_iterator al = atgts.begin();
                al != atgts.end(); ++al)
              {
              if (al->second.IsInAll())
                {
                if (al->second.GetType() == cmTarget::UTILITY)
                  {
                  l->second.AddUtility(al->first.c_str());
                  }
                else
                  {
                  l->second.GetLinkLibraries().push_back(
                    cmTarget::LinkLibraries::value_type(al->first,
                                                        cmTarget::GENERAL));
                  }
                }
              }
            }
          }
        }
      // Write the project into the SLN file
      if (strncmp(l->first.c_str(), "INCLUDE_EXTERNAL_MSPROJECT", 26) == 0)
      {
        cmCustomCommand cc = l->second.GetCustomCommands()[0];
        
        // dodgy use of the cmCustomCommand's members to store the 
        // arguments from the INCLUDE_EXTERNAL_MSPROJECT command
        std::vector<std::string> stuff = cc.GetDepends();
        std::vector<std::string> depends = cc.GetOutputs();
        this->WriteExternalProject(fout, stuff[0].c_str(), stuff[1].c_str(), depends);
        ++si;
      }
      else if ((l->second.GetType() != cmTarget::INSTALL_FILES)
          && (l->second.GetType() != cmTarget::INSTALL_PROGRAMS))
        {
        this->WriteProject(fout, si->c_str(), dir.c_str(), 
                           pg,l->second);
        ++si;
        }
      }
    }
  fout << "Global\n"
       << "\tGlobalSection(SolutionConfiguration) = preSolution\n";
  
  int c = 0;
  for(std::vector<std::string>::iterator i = m_Configurations.begin();
      i != m_Configurations.end(); ++i)
    {
    fout << "\t\tConfigName." << c << " = " << *i << "\n";
    c++;
    }
  fout << "\tEndGlobalSection\n"
       << "\tGlobalSection(ProjectDependencies) = postSolution\n";
  // loop over again and compute the depends
  for(k = allListFiles.begin(); k != allListFiles.end(); ++k)
    {
    cmMakefile* mf = *k;
    cmMSDotNETGenerator* pg =  
      static_cast<cmMSDotNETGenerator*>(mf->GetMakefileGenerator());
        // Get the list of create dsp files names from the cmVCProjWriter, more
    // than one dsp could have been created per input CMakeLists.txt file
    // for each target
    std::vector<std::string> dspnames = 
      pg->GetCreatedProjectNames();
    cmTargets &tgts = pg->GetMakefile()->GetTargets();
    cmTargets::iterator l = tgts.begin();
    std::string dir = mf->GetStartDirectory();
    for(std::vector<std::string>::iterator si = dspnames.begin(); 
        l != tgts.end(); ++l)
      {
      if ((l->second.GetType() != cmTarget::INSTALL_FILES)
          && (l->second.GetType() != cmTarget::INSTALL_PROGRAMS))
        {
        this->WriteProjectDepends(fout, si->c_str(), dir.c_str(), 
                                  pg,l->second);
        ++si;
        }
      }
    }
  fout << "\tEndGlobalSection\n";
  fout << "\tGlobalSection(ProjectConfiguration) = postSolution\n";
    // loop over again and compute the depends
  for(k = allListFiles.begin(); k != allListFiles.end(); ++k)
    {
    cmMakefile* mf = *k;
    cmMSDotNETGenerator* pg =  
      static_cast<cmMSDotNETGenerator*>(mf->GetMakefileGenerator());
        // Get the list of create dsp files names from the cmVCProjWriter, more
    // than one dsp could have been created per input CMakeLists.txt file
    // for each target
    std::vector<std::string> dspnames = 
      pg->GetCreatedProjectNames();
    cmTargets &tgts = pg->GetMakefile()->GetTargets();
    cmTargets::iterator l = tgts.begin();
    std::string dir = mf->GetStartDirectory();
    for(std::vector<std::string>::iterator si = dspnames.begin(); 
        l != tgts.end(); ++l)
      {
      if ((l->second.GetType() != cmTarget::INSTALL_FILES)
          && (l->second.GetType() != cmTarget::INSTALL_PROGRAMS))
        {
        this->WriteProjectConfigurations(fout, si->c_str());
        ++si;
        }
      }
    // delete the cmMakefile which also deletes the cmMSProjectGenerator
    if(mf != m_Makefile)
      {
      delete mf;
      }
    }
  fout << "\tEndGlobalSection\n";

  // Write the footer for the SLN file
  this->WriteSLNFooter(fout);
}


// Write a dsp file into the SLN file,
// Note, that dependencies from executables to 
// the libraries it uses are also done here
void cmMSDotNETGenerator::WriteProject(std::ostream& fout, 
                               const char* dspname,
                               const char* dir,
                               cmMSDotNETGenerator*,
                               const cmTarget&
  )
{
  std::string d = cmSystemTools::ConvertToOutputPath(dir);
  fout << "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\" = \"" 
       << dspname << "\", \""
       << d << "\\" << dspname << ".vcproj\", \"{"
       << this->CreateGUID(dspname) << "}\"\nEndProject\n";
}



// Write a dsp file into the SLN file,
// Note, that dependencies from executables to 
// the libraries it uses are also done here
void cmMSDotNETGenerator::WriteProjectDepends(std::ostream& fout, 
                                      const char* dspname,
                                      const char* ,
                                      cmMSDotNETGenerator*,
                                      const cmTarget& target
  )
{
  int depcount = 0;
  // insert Begin Project Dependency  Project_Dep_Name project stuff here 
  if (target.GetType() != cmTarget::STATIC_LIBRARY)
    {
    cmTarget::LinkLibraries::const_iterator j, jend;
    j = target.GetLinkLibraries().begin();
    jend = target.GetLinkLibraries().end();
    for(;j!= jend; ++j)
      {
      if(j->first != dspname)
	{
        // is the library part of this SLN ? If so add dependency
        const char* cacheValue
          = m_Makefile->GetDefinition(j->first.c_str());
        if(cacheValue)
          {
          fout << "\t\t{" << this->CreateGUID(dspname) << "}." << depcount << " = {"
               << this->CreateGUID(j->first.c_str()) << "}\n";
          depcount++;
          }
        }
      }
    }

  std::set<std::string>::const_iterator i, end;
  // write utility dependencies.
  i = target.GetUtilities().begin();
  end = target.GetUtilities().end();
  for(;i!= end; ++i)
    {
    if(*i != dspname)
      {
      fout << "\t\t{" << this->CreateGUID(dspname) << "}." << depcount << " = {"
           << this->CreateGUID(i->c_str()) << "}\n";
      depcount++;
      }
    }
}


// Write a dsp file into the SLN file,
// Note, that dependencies from executables to 
// the libraries it uses are also done here
void cmMSDotNETGenerator::WriteProjectConfigurations(std::ostream& fout, const char* name)
{
  std::string guid = this->CreateGUID(name);
  for(std::vector<std::string>::iterator i = m_Configurations.begin();
      i != m_Configurations.end(); ++i)
    {
    fout << "\t\t{" << guid << "}." << *i << ".ActiveCfg = " << *i << "|Win32\n"
         << "\t\t{" << guid << "}." << *i << ".Build.0 = " << *i << "|Win32\n";
    }
}



// Write a dsp file into the SLN file,
// Note, that dependencies from executables to 
// the libraries it uses are also done here
void cmMSDotNETGenerator::WriteExternalProject(std::ostream& , 
			       const char* ,
			       const char* ,
                               const std::vector<std::string>& )
{
  cmSystemTools::Error("WriteExternalProject not implemented");
//  fout << "#########################################################"
//     "######################\n\n";
//   fout << "Project: \"" << name << "\"=" 
//        << location << " - Package Owner=<4>\n\n";
//   fout << "Package=<5>\n{{{\n}}}\n\n";
//   fout << "Package=<4>\n";
//   fout << "{{{\n";

  
//   std::vector<std::string>::const_iterator i, end;
//   // write dependencies.
//   i = dependencies.begin();
//   end = dependencies.end();
//   for(;i!= end; ++i)
//   {
//     fout << "Begin Project Dependency\n";
//     fout << "Project_Dep_Name " << *i << "\n";
//     fout << "End Project Dependency\n";
//   }
//   fout << "}}}\n\n";
}



// Standard end of dsw file
void cmMSDotNETGenerator::WriteSLNFooter(std::ostream& fout)
{
  fout << "\tGlobalSection(ExtensibilityGlobals) = postSolution\n"
       << "\tEndGlobalSection\n"
       << "\tGlobalSection(ExtensibilityAddIns) = postSolution\n"
       << "\tEndGlobalSection\n"
       << "EndGlobal\n";
}

  
// ouput standard header for dsw file
void cmMSDotNETGenerator::WriteSLNHeader(std::ostream& fout)
{
  fout << "Microsoft Visual Studio Solution File, Format Version 7.00\n";
}


std::string cmMSDotNETGenerator::CreateGUID(const char* name)
{
  std::map<cmStdString, cmStdString>::iterator i = m_GUIDMap.find(name);
  if(i != m_GUIDMap.end())
    {
    return i->second;
    }
  std::string ret;
  UUID uid;
  unsigned char *uidstr;
  UuidCreate(&uid);
  UuidToString(&uid,&uidstr);
  ret = reinterpret_cast<char*>(uidstr);
  RpcStringFree(&uidstr);
  ret = cmSystemTools::UpperCase(ret);
  m_GUIDMap[name] = ret;
  return ret;
}




// TODO
// for CommandLine= need to repleace quotes with &quot
// write out configurations


void cmMSDotNETGenerator::OutputVCProjFile()
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
  
  m_LibraryOutputPath = "";
  if (m_Makefile->GetDefinition("LIBRARY_OUTPUT_PATH"))
    {
    m_LibraryOutputPath = m_Makefile->GetDefinition("LIBRARY_OUTPUT_PATH");
    }
  if(m_LibraryOutputPath.size())
    {
    // make sure there is a trailing slash
    if(m_LibraryOutputPath[m_LibraryOutputPath.size()-1] != '/')
      {
      m_LibraryOutputPath += "/";
      }
    }
  m_ExecutableOutputPath = "";
  if (m_Makefile->GetDefinition("EXECUTABLE_OUTPUT_PATH"))
    {
    m_ExecutableOutputPath = m_Makefile->GetDefinition("EXECUTABLE_OUTPUT_PATH");
    }
  if(m_ExecutableOutputPath.size())
    {
    // make sure there is a trailing slash
    if(m_ExecutableOutputPath[m_ExecutableOutputPath.size()-1] != '/')
      {
      m_ExecutableOutputPath += "/";
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

void cmMSDotNETGenerator::CreateSingleVCProj(const char *lname, cmTarget &target)
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


void cmMSDotNETGenerator::AddVCProjBuildRule(cmSourceGroup& sourceGroup)
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
  makefileIn = cmSystemTools::ConvertToOutputPath(makefileIn.c_str());
  std::string dsprule = "${CMAKE_COMMAND}";
  m_Makefile->ExpandVariablesInString(dsprule);
  dsprule = cmSystemTools::ConvertToOutputPath(dsprule.c_str());
  std::string args = makefileIn;
  args += " -H\"";
  args +=
    cmSystemTools::ConvertToOutputPath(m_Makefile->GetHomeDirectory());
  args += "\" -S\"";
  args += 
    cmSystemTools::ConvertToOutputPath(m_Makefile->GetStartDirectory());
  args += "\" -O\"";
  args += 
    cmSystemTools::ConvertToOutputPath(m_Makefile->GetStartOutputDirectory());
  args += "\" -B\"";
  args += 
    cmSystemTools::ConvertToOutputPath(m_Makefile->GetHomeOutputDirectory());
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


void cmMSDotNETGenerator::WriteConfigurations(std::ostream& fout, 
                                         const char *libName,
                                         const cmTarget &target)
{
  fout << "\t<Configurations>\n";
  for( std::vector<std::string>::iterator i = m_Configurations.begin();
       i != m_Configurations.end(); ++i)
    {
    this->WriteConfiguration(fout, i->c_str(), libName, target);
    }
  fout << "\t</Configurations>\n";
}

void cmMSDotNETGenerator::WriteConfiguration(std::ostream& fout, 
                                             const char* configName,
                                             const char *libName,
                                             const cmTarget &target)
{ 
  const char* mfcFlag = m_Makefile->GetDefinition("CMAKE_MFC_FLAG");
  if(!mfcFlag)
    {
    mfcFlag = "0";
    }
  fout << "\t\t<Configuration\n"
       << "\t\t\tName=\"" << configName << "|Win32\"\n"
       << "\t\t\tOutputDirectory=\"" << configName << "\"\n";
  // This is an internal type to Visual Studio, it seems that:
  // 4 == static library
  // 2 == dll
  // 1 == executable
  // 10 == utility 
  const char* configType = "10";
  switch(target.GetType())
    { 
    case cmTarget::STATIC_LIBRARY:
      configType = "4";
      break;
    case cmTarget::SHARED_LIBRARY:
    case cmTarget::MODULE_LIBRARY:
      configType = "2";
      break;
    case cmTarget::EXECUTABLE: 
    case cmTarget::WIN32_EXECUTABLE:  
      configType = "1";
      break; 
    case cmTarget::UTILITY:
      configType = "10";
    default:
      break;
    }
  
  fout << "\t\t\tIntermediateDirectory=\".\\" << configName << "\"\n"
       << "\t\t\tConfigurationType=\"" << configType << "\"\n"
       << "\t\t\tUseOfMFC=\"" << mfcFlag << "\"\n"
       << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"FALSE\"\n"
       << "\t\t\tCharacterSet=\"2\">\n";
  fout << "\t\t\t<Tool\n"
       << "\t\t\t\tName=\"VCCLCompilerTool\"\n"
       << "\t\t\t\tAdditionalOptions=\"" << 
    m_Makefile->GetDefinition("CMAKE_CXX_FLAGS") << "\"\n";
  
  fout << "\t\t\t\tAdditionalIncludeDirectories=\"";
  std::vector<std::string>& includes = m_Makefile->GetIncludeDirectories();
  std::vector<std::string>::iterator i = includes.begin();
  for(;i != includes.end(); ++i)
    {
    std::string ipath = this->ConvertToXMLOutputPath(i->c_str());
    fout << ipath << ";";
    }
  fout << "\"\n";
  
// Optimization = 0  None Debug  /O0
// Optimization = 1  MinSize     /O1
// Optimization = 2  MaxSpeed    /O2
// Optimization = 3  Max Optimization   /O3
// RuntimeLibrary = 0 /MT   multithread
// RuntimeLibrary = 1 /MTd  multithread debug
// RuntimeLibrary = 2 /MD   multithread dll
// RuntimeLibrary = 3 /MDd  multithread dll debug
// RuntimeLibrary = 4 /ML   single thread
// RuntimeLibrary = 5 /MLd  single thread debug
// InlineFunctionExpansion = 0 none
// InlineFunctionExpansion = 1 when inline keyword
// InlineFunctionExpansion = 2 any time you can


  if(strcmp(configName, "Debug") == 0)
    {
    fout << "\t\t\t\tOptimization=\"0\"\n"
         << "\t\t\t\tRuntimeLibrary=\"3\"\n"
         << "\t\t\t\tInlineFunctionExpansion=\"0\"\n"
         << "\t\t\t\tPreprocessorDefinitions=\"WIN32,_DEBUG,_WINDOWS";
    }
  else if(strcmp(configName, "Release") == 0)
    {
    fout << "\t\t\t\tOptimization=\"2\"\n"
         << "\t\t\t\tRuntimeLibrary=\"2\"\n"
         << "\t\t\t\tInlineFunctionExpansion=\"1\"\n"
         << "\t\t\t\tPreprocessorDefinitions=\"WIN32,NDEBUG,_WINDOWS";
    }
  else if(strcmp(configName, "MinSizeRel") == 0)
    {
    fout << "\t\t\t\tOptimization=\"1\"\n"
         << "\t\t\t\tRuntimeLibrary=\"2\"\n"
         << "\t\t\t\tInlineFunctionExpansion=\"1\"\n"
         << "\t\t\t\tPreprocessorDefinitions=\"WIN32,NDEBUG,_WINDOWS";
    }
  else if(strcmp(configName, "RelWithDebInfo") == 0)
    {
    fout << "\t\t\t\tOptimization=\"2\"\n"
         << "\t\t\t\tRuntimeLibrary=\"2\"\n"
         << "\t\t\t\tInlineFunctionExpansion=\"1\"\n"
         << "\t\t\t\tPreprocessorDefinitions=\"WIN32,NDEBUG,_WINDOWS";
    }
  if(target.GetType() == cmTarget::SHARED_LIBRARY
     || target.GetType() == cmTarget::MODULE_LIBRARY)
    {
    fout << "," << libName << "_EXPORTS";
    }
  this->OutputDefineFlags(fout);
  fout << "\"\n";
  if(m_Makefile->IsOn("CMAKE_CXX_USE_RTTI"))
    {
    fout << "\t\t\t\tRuntimeTypeInfo=\"TRUE\"\n";
    }
  fout << "\t\t\t\tAssemblerListingLocation=\"" << configName << "\"\n";
  fout << "\t\t\t\tObjectFile=\"" << configName << "\\\"\n";
  fout << "\t\t\t\tWarningLevel=\"" << m_Makefile->GetDefinition("CMAKE_CXX_WARNING_LEVEL") << "\"\n";
  fout << "\t\t\t\tDetect64BitPortabilityProblems=\"TRUE\"\n"
       << "\t\t\t\tDebugInformationFormat=\"3\"";
  fout << "/>\n";  // end of <Tool Name=VCCLCompilerTool

  fout << "\t\t\t<Tool\n\t\t\t\tName=\"VCCustomBuildTool\"/>\n";
  fout << "\t\t\t<Tool\n\t\t\t\tName=\"VCMIDLTool\"/>\n";
  fout << "\t\t\t<Tool\n\t\t\t\tName=\"VCPostBuildEventTool\"";
  this->OutputTargetRules(fout, target, libName);
  fout << "/>\n";
  fout << "\t\t\t<Tool\n\t\t\t\tName=\"VCPreBuildEventTool\"/>\n";
  this->OutputBuildTool(fout, configName, libName, target);
  fout << "\t\t</Configuration>\n";
}

void cmMSDotNETGenerator::OutputBuildTool(std::ostream& fout,
                                          const char* configName,
                                          const char *libName,
                                          const cmTarget &target)
{ 
  switch(target.GetType())
    {
    case cmTarget::STATIC_LIBRARY:
    {
      std::string libpath = m_LibraryOutputPath + 
        "$(OutDir)/" + libName + ".lib";
      fout << "\t\t\t<Tool\n"
           << "\t\t\t\tName=\"VCLibrarianTool\"\n"
           << "\t\t\t\t\tOutputFile=\"" 
           << this->ConvertToXMLOutputPath(libpath.c_str()) << ".\"/>\n";
      break;
    }
    case cmTarget::SHARED_LIBRARY:
    case cmTarget::MODULE_LIBRARY:
      fout << "\t\t\t<Tool\n"
           << "\t\t\t\tName=\"VCLinkerTool\"\n"
           << "\t\t\t\tAdditionalOptions=\"/MACHINE:I386\"\n"
           << "\t\t\t\tAdditionalDependencies=\" odbc32.lib odbccp32.lib ";
      this->OutputLibraries(fout, configName, libName, target);
      fout << "\"\n";
      fout << "\t\t\t\tOutputFile=\"" 
           << m_ExecutableOutputPath << configName << "/" 
           << libName << ".dll\"\n";
      fout << "\t\t\t\tLinkIncremental=\"1\"\n";
      fout << "\t\t\t\tSuppressStartupBanner=\"TRUE\"\n";
      fout << "\t\t\t\tAdditionalLibraryDirectories=\"";
      this->OutputLibraryDirectories(fout, configName, libName, target);
      fout << "\"\n";
      this->OutputModuleDefinitionFile(fout, target);
      fout << "\t\t\t\tProgramDatabaseFile=\"" << m_LibraryOutputPath 
           << "$(OutDir)\\" << libName << ".pdb\"\n";
      if(strcmp(configName, "Debug") == 0)
        {
        fout << "\t\t\t\tGenerateDebugInformation=\"TRUE\"\n";
        }
      fout << "\t\t\t\tStackReserveSize=\"" 
           << m_Makefile->GetDefinition("CMAKE_CXX_STACK_SIZE") << "\"\n";
      fout << "\t\t\t\tImportLibrary=\"" 
           << m_ExecutableOutputPath << configName << "/" 
           << libName << ".lib\"/>\n";
      break;
    case cmTarget::EXECUTABLE:
    case cmTarget::WIN32_EXECUTABLE:

      fout << "\t\t\t<Tool\n"
           << "\t\t\t\tName=\"VCLinkerTool\"\n"
           << "\t\t\t\tAdditionalOptions=\"/MACHINE:I386\"\n"
           << "\t\t\t\tAdditionalDependencies=\" odbc32.lib odbccp32.lib ";
      this->OutputLibraries(fout, configName, libName, target);
      fout << "\"\n";
      fout << "\t\t\t\tOutputFile=\"" 
           << m_ExecutableOutputPath << configName << "/" << libName << ".exe\"\n";
      fout << "\t\t\t\tLinkIncremental=\"1\"\n";
      fout << "\t\t\t\tSuppressStartupBanner=\"TRUE\"\n";
      fout << "\t\t\t\tAdditionalLibraryDirectories=\"";
      this->OutputLibraryDirectories(fout, configName, libName, target);
      fout << "\"\n";
      fout << "\t\t\t\tProgramDatabaseFile=\"" << m_LibraryOutputPath 
           << "$(OutDir)\\" << libName << ".pdb\"\n";
      if(strcmp(configName, "Debug") == 0)
        {
        fout << "\t\t\t\tGenerateDebugInformation=\"TRUE\"\n";
        }
      if( target.GetType() == cmTarget::EXECUTABLE)
        {
        fout << "\t\t\t\tSubSystem=\"1\"\n";
        }
      else
        {      
        fout << "\t\t\t\tSubSystem=\"2\"\n";
        }
      fout << "\t\t\t\tStackReserveSize=\"" 
           << m_Makefile->GetDefinition("CMAKE_CXX_STACK_SIZE") << "\"/>\n";
      break;
    case cmTarget::UTILITY:
      break;
    }
}

void cmMSDotNETGenerator::OutputModuleDefinitionFile(std::ostream& fout,
                                                     const cmTarget &target)
{
  std::vector<cmSourceFile> const& classes = target.GetSourceFiles();
  for(std::vector<cmSourceFile>::const_iterator i = classes.begin(); 
      i != classes.end(); i++)
    {  
    if(cmSystemTools::UpperCase(i->GetSourceExtension()) == "DEF")
      {
      fout << "\t\t\t\tModuleDefinitionFile=\""
           << this->ConvertToXMLOutputPath(i->GetFullPath().c_str())
           << "\"\n";
      return;
      }
    }
  
}

void cmMSDotNETGenerator::OutputLibraryDirectories(std::ostream& fout,
                                                   const char*,
                                                   const char*,
                                                   const cmTarget &)
{
  bool hasone = false;
  if(m_LibraryOutputPath.size())
    {
    hasone = true;
    fout << m_LibraryOutputPath << "$(INTDIR)," << m_LibraryOutputPath;
    }
  if(m_ExecutableOutputPath.size())
    {
    hasone = true;
    fout << m_ExecutableOutputPath << "$(INTDIR)," << m_ExecutableOutputPath;
    }
    
  std::set<std::string> pathEmitted;
  std::vector<std::string>::iterator i;
  std::vector<std::string>& libdirs = m_Makefile->GetLinkDirectories();
  for(i = libdirs.begin(); i != libdirs.end(); ++i)
    {
    std::string lpath = *i;
    if(lpath[lpath.size()-1] != '/')
      {
      lpath += "/";
      }
    if(pathEmitted.insert(lpath).second)
      {
      if(hasone)
        {
        fout << ",";
        }
      std::string lpathi = lpath + "$(INTDIR)";
	  fout << this->ConvertToXMLOutputPath(lpathi.c_str()) << "," << lpath;
      hasone = true;
      }
    }
}

void cmMSDotNETGenerator::OutputLibraries(std::ostream& fout,
                                          const char* configName,
                                          const char* libName,
                                          const cmTarget &target)
{
  const cmTarget::LinkLibraries& libs = target.GetLinkLibraries();
  cmTarget::LinkLibraries::const_iterator j;
  for(j = libs.begin(); j != libs.end(); ++j)
    { 
    if(j->first != libName)
      {
      std::string lib = j->first;
      if(j->first.find(".lib") == std::string::npos)
        {
        lib += ".lib";
        }
      lib = this->ConvertToXMLOutputPath(lib.c_str());
      if (j->second == cmTarget::GENERAL
          || (j->second == cmTarget::DEBUG && strcmp(configName, "DEBUG") == 0)
          || (j->second == cmTarget::OPTIMIZED && strcmp(configName, "DEBUG") != 0))
        {
        fout << lib << " ";
        }
      }
    }
}

void cmMSDotNETGenerator::OutputDefineFlags(std::ostream& fout)
{
  std::string defs = m_Makefile->GetDefineFlags();
  std::string::size_type pos = defs.find("-D");
  bool done = pos == std::string::npos;
  if(!done)
    {
    fout << ",";
    }
  while(!done)
    {
    std::string::size_type nextpos = defs.find("-D", pos+2);
    std::string define;
    if(nextpos != std::string::npos)
      {
      define = defs.substr(pos+2, nextpos - pos -3);
      }
    else
      {
      define = defs.substr(pos+2);
      done = true;
      }
    fout << define << ",";
    if(!done)
      {
      pos = defs.find("-D", nextpos);
      }
    } 
}

void cmMSDotNETGenerator::WriteVCProjFile(std::ostream& fout, 
                                 const char *libName,
                                 cmTarget &target)
{
  // We may be modifying the source groups temporarily, so make a copy.
  std::vector<cmSourceGroup> sourceGroups = m_Makefile->GetSourceGroups();
  
  // get the classes from the source lists then add them to the groups
  std::vector<cmSourceFile> const& classes = target.GetSourceFiles();
  for(std::vector<cmSourceFile>::const_iterator i = classes.begin(); 
      i != classes.end(); i++)
    {
    // Add the file to the list of sources.
    std::string source = i->GetFullPath();
    if(cmSystemTools::UpperCase(i->GetSourceExtension()) == "DEF")
      {
      m_ModuleDefinitionFile = i->GetFullPath();
      }
    
    cmSourceGroup& sourceGroup = m_Makefile->FindSourceGroup(source.c_str(),
                                                             sourceGroups);
    sourceGroup.AddSource(source.c_str(), &(*i));
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
      const cmSourceGroup::Commands& commands = cc->second.m_Commands;
      const char* compileFlags = 0;
      if(cc->second.m_SourceFile)
        {
        compileFlags = cc->second.m_SourceFile->GetCompileFlags();
        }
      if (source != libName || target.GetType() == cmTarget::UTILITY)
        {
        fout << "\t\t\t<File\n";
        std::string d = this->ConvertToXMLOutputPath(source.c_str());
        // Tell MS-Dev what the source is.  If the compiler knows how to
        // build it, then it will.
        fout << "\t\t\t\tRelativePath=\"" << d << "\">\n";
        if (!commands.empty())
          {
          cmSourceGroup::CommandFiles totalCommand;
          std::string totalCommandStr;
          totalCommandStr = this->CombineCommands(commands, totalCommand,
                                                  source.c_str());
          this->WriteCustomRule(fout, source.c_str(), totalCommandStr.c_str(), 
                                totalCommand.m_Depends, 
                                totalCommand.m_Outputs, compileFlags);
          }
        else if(compileFlags)
          {
          for(std::vector<std::string>::iterator i
                = m_Configurations.begin(); i != m_Configurations.end(); ++i)
            {
            fout << "\t\t\t\t<FileConfiguration\n"
                 << "\t\t\t\t\tName=\""  << *i << "|Win32\">\n"
                 << "\t\t\t\t\t<Tool\n"
                 << "\t\t\t\t\tName=\"VCCLCompilerTool\"\n"
                 << "\t\t\t\t\tAdditionalOptions=\""
                 << compileFlags << "\"/>\n"
                 << "\t\t\t\t</FileConfiguration>\n";
            }
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


void cmMSDotNETGenerator::WriteCustomRule(std::ostream& fout,
                                          const char* source,
                                          const char* command,
                                          const std::set<std::string>& depends,
                                          const std::set<std::string>& outputs,
                                          const char* compileFlags)
{
  std::string cmd = command;
  cmSystemTools::ReplaceString(cmd, "\"", "&quot;");
  std::vector<std::string>::iterator i;
  for(i = m_Configurations.begin(); i != m_Configurations.end(); ++i)
    {
    fout << "\t\t\t\t<FileConfiguration\n";
    fout << "\t\t\t\t\tName=\"" << *i << "|Win32\">\n";
    if(compileFlags)
      {
      fout << "\t\t\t\t\t<Tool\n"
           << "\t\t\t\t\tName=\"VCCLCompilerTool\"\n"
           << "\t\t\t\t\tAdditionalOptions=\""
           << compileFlags << "\"/>\n";
      }
    fout << "\t\t\t\t\t<Tool\n"
         << "\t\t\t\t\tName=\"VCCustomBuildTool\"\n"
         << "\t\t\t\t\tCommandLine=\"" << cmd << "\n\"\n"
         << "\t\t\t\t\tAdditionalDependencies=\"";
    // Write out the dependencies for the rule.
    std::string temp;
    for(std::set<std::string>::const_iterator d = depends.begin();
	d != depends.end(); ++d)
      {
      fout << this->ConvertToXMLOutputPath(d->c_str())
           << ";";
      }
    fout << "\"\n";
    fout << "\t\t\t\t\tOutputs=\"";
    if(outputs.size() == 0)
      {
      fout << source << "_force";
      }
    
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


void cmMSDotNETGenerator::WriteVCProjBeginGroup(std::ostream& fout, 
					const char* group,
					const char* )
{
  fout << "\t\t<Filter\n"
       << "\t\t\tName=\"" << group << "\"\n"
       << "\t\t\tFilter=\"\">\n";
}


void cmMSDotNETGenerator::WriteVCProjEndGroup(std::ostream& fout)
{
  fout << "\t\t</Filter>\n";
}





std::string
cmMSDotNETGenerator::CombineCommands(const cmSourceGroup::Commands &commands,
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
    temp= 
      cmSystemTools::ConvertToOutputPath(c->second.m_Command.c_str()); 
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

void cmMSDotNETGenerator::OutputTargetRules(std::ostream& fout,
                                            const cmTarget &target, 
                                            const char *libName)
{
  if (target.GetType() >= cmTarget::UTILITY)
    {
    return;
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
      if(!init)
        {
        fout << "\nCommandLine=\"";
        init = true;
        }
      fout << cc.GetCommand() << " " << cc.GetArguments() << "\n";
      }
    }
  if (init)
    {
    fout << "\"";
    }
}

void cmMSDotNETGenerator::WriteProjectStart(std::ostream& fout, const char *libName,
                                       const cmTarget &, 
                                       std::vector<cmSourceGroup> &)
{
  fout << "<?xml version=\"1.0\" encoding = \"Windows-1252\"?>\n"
       << "<VisualStudioProject\n"
       << "\tProjectType=\"Visual C++\"\n"
       << "\tVersion=\"7.00\"\n"
       << "\tName=\"" << libName << "\"\n"
       << "\tSccProjectName=\"\"\n"
       << "\tSccLocalPath=\"\"\n"
       << "\tKeyword=\"Win32Proj\">\n"
       << "\t<Platforms>\n"
       << "\t\t<Platform\n\t\t\tName=\"Win32\"/>\n"
       << "\t</Platforms>\n";
}


void cmMSDotNETGenerator::WriteVCProjFooter(std::ostream& fout)
{
  fout << "\t<Globals>\n"
       << "\t</Globals>\n"
       << "</VisualStudioProject>\n";
}


std::string cmMSDotNETGenerator::ConvertToXMLOutputPath(const char* path)
{
  std::string ret = cmSystemTools::ConvertToOutputPath(path);
  cmSystemTools::ReplaceString(ret, "\"", "&quot;");
  return ret;
}
