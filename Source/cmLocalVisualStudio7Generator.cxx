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
#include "cmGlobalVisualStudio7Generator.h"
#include "cmLocalVisualStudio7Generator.h"
#include "cmMakefile.h"
#include "cmSystemTools.h"
#include "cmSourceFile.h"
#include "cmCacheManager.h"
#include "cmake.h"

cmLocalVisualStudio7Generator::cmLocalVisualStudio7Generator()
{
  m_Version71 = false;
}

cmLocalVisualStudio7Generator::~cmLocalVisualStudio7Generator()
{
}


void cmLocalVisualStudio7Generator::Generate(bool /* fromTheTop */)
{
  this->OutputVCProjFile();
}

// TODO
// for CommandLine= need to repleace quotes with &quot
// write out configurations
void cmLocalVisualStudio7Generator::OutputVCProjFile()
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

  // expand vars for custom commands
  m_Makefile->ExpandVariablesInCustomCommands();  

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

void cmLocalVisualStudio7Generator::CreateSingleVCProj(const char *lname, cmTarget &target)
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


void cmLocalVisualStudio7Generator::AddVCProjBuildRule()
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
  std::vector<std::string> argv;
  argv.push_back(makefileIn);
  makefileIn = m_Makefile->GetStartDirectory();
  makefileIn += "/";
  makefileIn += "CMakeLists.txt";
  std::string args;
  args = "-H";
  args +=
    cmSystemTools::ConvertToOutputPath(m_Makefile->GetHomeDirectory());
  argv.push_back(args);
  args = "-S";
  args +=
    cmSystemTools::ConvertToOutputPath(m_Makefile->GetStartDirectory());
  argv.push_back(args);
  args = "-O";
  args += 
    cmSystemTools::ConvertToOutputPath(m_Makefile->GetStartOutputDirectory());
  argv.push_back(args);
  args = "-B";
  args += 
    cmSystemTools::ConvertToOutputPath(m_Makefile->GetHomeOutputDirectory());
  argv.push_back(args);
  
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
  m_Makefile->AddCustomCommandToOutput(dspname.c_str(), dsprule.c_str(), 
                                       argv, makefileIn.c_str(), listFiles,
                                       NULL, true);
}


void cmLocalVisualStudio7Generator::WriteConfigurations(std::ostream& fout, 
                                                        const char *libName,
                                                        const cmTarget &target)
{
  std::vector<std::string> *configs = 
    static_cast<cmGlobalVisualStudio7Generator *>(m_GlobalGenerator)->GetConfigurations();
  fout << "\t<Configurations>\n";
  for( std::vector<std::string>::iterator i = configs->begin();
       i != configs->end(); ++i)
    {
    this->WriteConfiguration(fout, i->c_str(), libName, target);
    }
  fout << "\t</Configurations>\n";
}

void cmLocalVisualStudio7Generator::WriteConfiguration(std::ostream& fout, 
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
  
  std::string flags;
  std::string flagsRelease = " ";
  std::string flagsMinSize = " ";
  std::string flagsDebug = " ";
  std::string flagsDebugRel = " ";
  if(target.HasCxx())
    {
    flags = m_Makefile->GetDefinition("CMAKE_CXX_FLAGS");
    flagsRelease += m_Makefile->GetDefinition("CMAKE_CXX_FLAGS_RELEASE");
    flagsMinSize += m_Makefile->GetDefinition("CMAKE_CXX_FLAGS_MINSIZEREL");
    flagsDebug += m_Makefile->GetDefinition("CMAKE_CXX_FLAGS_DEBUG");
    flagsDebugRel += m_Makefile->GetDefinition("CMAKE_CXX_FLAGS_RELWITHDEBINFO");
    }
  else
    {
    flags = m_Makefile->GetDefinition("CMAKE_C_FLAGS");
    flagsRelease += m_Makefile->GetDefinition("CMAKE_C_FLAGS_RELEASE");
    flagsMinSize += m_Makefile->GetDefinition("CMAKE_C_FLAGS_MINSIZEREL");
    flagsDebug += m_Makefile->GetDefinition("CMAKE_C_FLAGS_DEBUG");
    flagsDebugRel += m_Makefile->GetDefinition("CMAKE_C_FLAGS_RELWITHDEBINFO");
    }
  
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


  int runtime = 0;
  int optimized = 0;
  int inlineFunctions = 0;
  const char* pre = "WIN32,_DEBUG,_WINDOWS";
  // set the flags and defaults for
  // runtime, optimized, and inlineFunctions , and
  // default pre processor flags
  if(strcmp(configName, "Debug") == 0)
    {
    inlineFunctions = 0;
    flags += flagsDebug;
    optimized = 0;
    runtime = 3;
    pre = "WIN32,_DEBUG,_WINDOWS";
    }
  else if (strcmp(configName, "Release") == 0)
    {
    inlineFunctions = 1;
    optimized =2;
    pre = "WIN32,NDEBUG,_WINDOWS";
    flags += flagsRelease;
    runtime = 2;
    }
  else if(strcmp(configName, "MinSizeRel") == 0)
    {
    inlineFunctions = 1;
    runtime = 2;
    optimized = 1;
    pre = "WIN32,NDEBUG,_WINDOWS";
    flags += flagsMinSize;
    }
  else if(strcmp(configName, "RelWithDebInfo") == 0)
    {
    inlineFunctions = 1;
    optimized = 2;
    runtime = 2;
    pre = "WIN32,NDEBUG,_WINDOWS";
    flags += flagsDebugRel;
    }
  
  fout << "\t\t\tIntermediateDirectory=\".\\" << configName << "\"\n"
       << "\t\t\tConfigurationType=\"" << configType << "\"\n"
       << "\t\t\tUseOfMFC=\"" << mfcFlag << "\"\n"
       << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"FALSE\"\n";
  // if -D_UNICODE or /D_UNICODE is found in the flags
  // change the character set to unicode, if not then
  // default to MBCS 
  std::string defs = m_Makefile->GetDefineFlags();
  if(flags.find("D_UNICODE") != flags.npos ||
     defs.find("D_UNICODE") != flags.npos)
    {
    fout << "\t\t\tCharacterSet=\"1\">\n";
    }
  else
    {
    fout << "\t\t\tCharacterSet=\"2\">\n";
    }
  
  fout << "\t\t\t<Tool\n"
       << "\t\t\t\tName=\"VCCLCompilerTool\"\n"
       << "\t\t\t\tAdditionalOptions=\"";

  cmSystemTools::ReplaceString(flags, "\"", "&quot;");
  // check the flags for the run time library flag options
  // if there is a match set the run time flag
  if(flags.find("MTd") != flags.npos)
    {
    cmSystemTools::ReplaceString(flags, "-MTd", "");
    cmSystemTools::ReplaceString(flags, "/MTd", "");
    runtime = 1;
    }
  else if (flags.find("MDd") != flags.npos)
    {
    cmSystemTools::ReplaceString(flags, "-MDd", "");
    cmSystemTools::ReplaceString(flags, "/MDd", "");
    runtime = 3;
    }
  else if (flags.find("MLd") != flags.npos)
    {
    cmSystemTools::ReplaceString(flags, "-MLd", "");
    cmSystemTools::ReplaceString(flags, "/MLd", "");
    runtime = 5;
    }
  else if (flags.find("MT") != flags.npos)
    {
    cmSystemTools::ReplaceString(flags, "-MT", "");
    cmSystemTools::ReplaceString(flags, "/MT", "");
    runtime = 0;
    }
  else if (flags.find("MD") != flags.npos)
    {
    cmSystemTools::ReplaceString(flags, "-MD", "");
    cmSystemTools::ReplaceString(flags, "/MD", "");
    runtime = 2;
    }
  else if (flags.find("ML") != flags.npos)
    {
    cmSystemTools::ReplaceString(flags, "-ML", "");
    cmSystemTools::ReplaceString(flags, "/ML", "");
    runtime = 4;
    }
  fout << flags;

  fout << " -DCMAKE_INTDIR=\\&quot;" << configName << "\\&quot;" 
       << "\"\n";

  fout << "\t\t\t\tAdditionalIncludeDirectories=\"";
  std::vector<std::string>& includes = m_Makefile->GetIncludeDirectories();
  std::vector<std::string>::iterator i = includes.begin();
  for(;i != includes.end(); ++i)
    {
    std::string ipath = this->ConvertToXMLOutputPath(i->c_str());
    fout << ipath << ";";
    }
  fout << "\"\n";

  fout << "\t\t\t\tOptimization=\"" << optimized << "\"\n"
       << "\t\t\t\tRuntimeLibrary=\"" << runtime << "\"\n"
       << "\t\t\t\tInlineFunctionExpansion=\"" << inlineFunctions << "\"\n"
       << "\t\t\t\tPreprocessorDefinitions=\"" << pre;
  if(target.GetType() == cmTarget::SHARED_LIBRARY
     || target.GetType() == cmTarget::MODULE_LIBRARY)
    {
    std::string exportSymbol;
    if (const char* custom_export_name = target.GetProperty("DEFINE_SYMBOL"))
      {
      exportSymbol = custom_export_name;
      }
    else
      {
      std::string id = libName;
      id += "_EXPORTS";
      exportSymbol = cmSystemTools::MakeCindentifier(id.c_str());
      }
    fout << "," << exportSymbol;
    }
  this->OutputDefineFlags(fout);
  fout << "\"\n";
  if(m_Makefile->IsOn("CMAKE_CXX_USE_RTTI"))
    {
    fout << "\t\t\t\tRuntimeTypeInfo=\"TRUE\"\n";
    }
  fout << "\t\t\t\tAssemblerListingLocation=\"" << configName << "\"\n";
  fout << "\t\t\t\tObjectFile=\"" << configName << "\\\"\n";
  if ( m_Makefile->GetDefinition("CMAKE_CXX_WARNING_LEVEL") )
    {
    fout << "\t\t\t\tWarningLevel=\"" << m_Makefile->GetDefinition("CMAKE_CXX_WARNING_LEVEL") << "\"\n";
    }
  fout << "\t\t\t\tDebugInformationFormat=\"3\"";
  fout << "/>\n";  // end of <Tool Name=VCCLCompilerTool

  fout << "\t\t\t<Tool\n\t\t\t\tName=\"VCCustomBuildTool\"/>\n";
  fout << "\t\t\t<Tool\n\t\t\t\tName=\"VCResourceCompilerTool\"\n"
       << "AdditionalIncludeDirectories=\"";
  for(i = includes.begin();i != includes.end(); ++i)
    {
    std::string ipath = this->ConvertToXMLOutputPath(i->c_str());
    fout << ipath << ";";
    }
  fout << "\"\n/>\n";
  fout << "\t\t\t<Tool\n\t\t\t\tName=\"VCMIDLTool\"/>\n";
  this->OutputTargetRules(fout, target, libName);
  this->OutputBuildTool(fout, configName, libName, target);
  fout << "\t\t</Configuration>\n";
}

void cmLocalVisualStudio7Generator::OutputBuildTool(std::ostream& fout,
                                          const char* configName,
                                          const char *libName,
                                          const cmTarget &target)
{ 
  std::string temp;
  std::string debugPostfix = "";
  bool debug = !strcmp(configName,"Debug");
  if (debug && m_Makefile->GetDefinition("CMAKE_DEBUG_POSTFIX"))
    {
    debugPostfix = m_Makefile->GetDefinition("CMAKE_DEBUG_POSTFIX");
    }  
  
  std::string extraLinkOptions;
  if((target.GetType() == cmTarget::EXECUTABLE) ||
     (target.GetType() == cmTarget::WIN32_EXECUTABLE))
    {
    extraLinkOptions = m_Makefile->GetDefinition("CMAKE_EXE_LINKER_FLAGS");
    }
  if(target.GetType() == cmTarget::SHARED_LIBRARY)
    {
    extraLinkOptions = m_Makefile->GetDefinition("CMAKE_SHARED_LINKER_FLAGS");
    }
  if(target.GetType() == cmTarget::MODULE_LIBRARY)
    {
    extraLinkOptions = m_Makefile->GetDefinition("CMAKE_MODULE_LINKER_FLAGS");
    }
  
  const char* targetLinkFlags = target.GetProperty("LINK_FLAGS");
  if(targetLinkFlags)
    {
    extraLinkOptions += " ";
    extraLinkOptions += targetLinkFlags;
    }
  
  switch(target.GetType())
    {
    case cmTarget::STATIC_LIBRARY:
    {
      std::string libpath = m_LibraryOutputPath + 
        "$(OutDir)/" + libName + debugPostfix + ".lib";
      fout << "\t\t\t<Tool\n"
           << "\t\t\t\tName=\"VCLibrarianTool\"\n"
           << "\t\t\t\t\tOutputFile=\"" 
           << this->ConvertToXMLOutputPathSingle(libpath.c_str()) << ".\"/>\n";
      break;
    }
    case cmTarget::SHARED_LIBRARY:
    case cmTarget::MODULE_LIBRARY:
      fout << "\t\t\t<Tool\n"
           << "\t\t\t\tName=\"VCLinkerTool\"\n"
           << "\t\t\t\tAdditionalOptions=\"/MACHINE:I386";
      if(extraLinkOptions.size())
        {
        fout << " " << cmLocalVisualStudio7Generator::EscapeForXML(
          extraLinkOptions.c_str()).c_str();
        }
      fout << "\"\n"
           << "\t\t\t\tAdditionalDependencies=\" odbc32.lib odbccp32.lib ";
      this->OutputLibraries(fout, configName, libName, target);
      fout << "\"\n";
      temp = m_LibraryOutputPath;
      temp += configName;
      temp += "/";
      temp += libName;
      temp += debugPostfix;
      temp += ".dll";
      fout << "\t\t\t\tOutputFile=\"" 
           << this->ConvertToXMLOutputPathSingle(temp.c_str()) << "\"\n";
      fout << "\t\t\t\tLinkIncremental=\"1\"\n";
      fout << "\t\t\t\tSuppressStartupBanner=\"TRUE\"\n";
      fout << "\t\t\t\tAdditionalLibraryDirectories=\"";
      this->OutputLibraryDirectories(fout, configName, libName, target);
      fout << "\"\n";
      this->OutputModuleDefinitionFile(fout, target);
      temp = m_LibraryOutputPath;
      temp += "$(OutDir)/";
      temp += libName;
      temp += debugPostfix;
      temp += ".pdb";
      fout << "\t\t\t\tProgramDatabaseFile=\"" << 
        this->ConvertToXMLOutputPathSingle(temp.c_str()) << "\"\n";
      if(strcmp(configName, "Debug") == 0
         || strcmp(configName, "RelWithDebInfo") == 0)
        {
        fout << "\t\t\t\tGenerateDebugInformation=\"TRUE\"\n";
        }
      fout << "\t\t\t\tStackReserveSize=\"" 
           << m_Makefile->GetDefinition("CMAKE_CXX_STACK_SIZE") << "\"\n";
      temp = m_ExecutableOutputPath;
      temp += configName;
      temp += "/";
      temp += libName;
      temp += debugPostfix;
      temp += ".lib";
      fout << "\t\t\t\tImportLibrary=\"" << this->ConvertToXMLOutputPathSingle(temp.c_str()) << "\"/>\n";
      break;
    case cmTarget::EXECUTABLE:
    case cmTarget::WIN32_EXECUTABLE:

      fout << "\t\t\t<Tool\n"
           << "\t\t\t\tName=\"VCLinkerTool\"\n"
           << "\t\t\t\tAdditionalOptions=\"/MACHINE:I386"; 
      if(extraLinkOptions.size())
        {
        fout << " " << cmLocalVisualStudio7Generator::EscapeForXML(
          extraLinkOptions.c_str()).c_str();
        }
      fout << "\"\n"
           << "\t\t\t\tAdditionalDependencies=\" odbc32.lib odbccp32.lib ";
      this->OutputLibraries(fout, configName, libName, target);
      fout << "\"\n";
      temp = m_ExecutableOutputPath;
      temp += configName;
      temp += "/";
      temp += libName;
      temp += ".exe";
      fout << "\t\t\t\tOutputFile=\"" << this->ConvertToXMLOutputPathSingle(temp.c_str()) << "\"\n";
      fout << "\t\t\t\tLinkIncremental=\"1\"\n";
      fout << "\t\t\t\tSuppressStartupBanner=\"TRUE\"\n";
      fout << "\t\t\t\tAdditionalLibraryDirectories=\"";
      this->OutputLibraryDirectories(fout, configName, libName, target);
      fout << "\"\n";
      fout << "\t\t\t\tProgramDatabaseFile=\"" << m_LibraryOutputPath 
           << "$(OutDir)\\" << libName << ".pdb\"\n";
      if(strcmp(configName, "Debug") == 0 
         || strcmp(configName, "RelWithDebInfo") == 0)
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

void cmLocalVisualStudio7Generator::OutputModuleDefinitionFile(std::ostream& fout,
                                                     const cmTarget &target)
{
  std::vector<cmSourceFile*> const& classes = target.GetSourceFiles();
  for(std::vector<cmSourceFile*>::const_iterator i = classes.begin(); 
      i != classes.end(); i++)
    {  
    if(cmSystemTools::UpperCase((*i)->GetSourceExtension()) == "DEF")
      {
      fout << "\t\t\t\tModuleDefinitionFile=\""
           << this->ConvertToXMLOutputPath((*i)->GetFullPath().c_str())
           << "\"\n";
      return;
      }
    }
  
}

void cmLocalVisualStudio7Generator::OutputLibraryDirectories(std::ostream& fout,
                                                   const char*,
                                                   const char*,
                                                   const cmTarget &tgt)
{ 
  bool hasone = false;
  if(m_LibraryOutputPath.size())
    {
    hasone = true;
    std::string temp = m_LibraryOutputPath;
    temp += "$(INTDIR)";
    
    fout << this->ConvertToXMLOutputPath(temp.c_str()) << "," << 
      this->ConvertToXMLOutputPath(m_LibraryOutputPath.c_str());
    }
  if(m_ExecutableOutputPath.size() && 
     (m_LibraryOutputPath != m_ExecutableOutputPath))
    {
    if (hasone)
      {
      fout << ",";
      }
    hasone = true;
    std::string temp = m_ExecutableOutputPath;
    temp += "$(INTDIR)"; 
    fout << this->ConvertToXMLOutputPath(temp.c_str()) << "," << 
      this->ConvertToXMLOutputPath(m_ExecutableOutputPath.c_str());
    }
    
  std::set<std::string> pathEmitted;
  std::vector<std::string>::const_iterator i;
  const std::vector<std::string>& libdirs = tgt.GetLinkDirectories();
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
      fout << this->ConvertToXMLOutputPath(lpathi.c_str()) << "," << 
        this->ConvertToXMLOutputPath(lpath.c_str());
      hasone = true;
      }
    }
}

void cmLocalVisualStudio7Generator::OutputLibraries(std::ostream& fout,
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
      std::string debugPostfix = "";
      // if this is a library we are building then watch for a debugPostfix
      if (!strcmp(configName,"Debug"))
        {
        std::string libPath = j->first + "_CMAKE_PATH";
        const char* cacheValue
          = m_GlobalGenerator->GetCMakeInstance()->GetCacheDefinition(libPath.c_str());
        if(cacheValue && m_Makefile->GetDefinition("CMAKE_DEBUG_POSTFIX"))
          {
          debugPostfix = m_Makefile->GetDefinition("CMAKE_DEBUG_POSTFIX");
          }
        }
      if(j->first.find(".lib") == std::string::npos)
        {
        lib += debugPostfix + ".lib";
        }
      lib = this->ConvertToXMLOutputPath(lib.c_str());
      if (j->second == cmTarget::GENERAL
          || (j->second == cmTarget::DEBUG && strcmp(configName, "Debug") == 0)
          || (j->second == cmTarget::OPTIMIZED && strcmp(configName, "Debug") != 0))
        {
        fout << lib << " ";
        }
      }
    }
}

void cmLocalVisualStudio7Generator::OutputDefineFlags(std::ostream& fout)
{
  std::string defs = m_Makefile->GetDefineFlags();
  cmSystemTools::ReplaceString(defs, "/D","-D");
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

    cmSystemTools::ReplaceString(define, "\"", "&quot;");
    fout << define << ",";
    if(!done)
      {
      pos = defs.find("-D", nextpos);
      }
    } 
}


void cmLocalVisualStudio7Generator::WriteVCProjFile(std::ostream& fout, 
                                                    const char *libName,
                                                    cmTarget &target)
{
  // get the configurations
  std::vector<std::string> *configs = 
    static_cast<cmGlobalVisualStudio7Generator *>
    (m_GlobalGenerator)->GetConfigurations();
  
  // if we should add regen rule then...
  const char *suppRegenRule = 
    m_Makefile->GetDefinition("CMAKE_SUPPRESS_REGENERATION");
  if (!cmSystemTools::IsOn(suppRegenRule))
    {
    this->AddVCProjBuildRule();
    }

  // trace the visual studio dependencies
  std::string name = libName;
  name += ".vcproj.cmake";
  target.TraceVSDependencies(name, m_Makefile);

  // We may be modifying the source groups temporarily, so make a copy.
  std::vector<cmSourceGroup> sourceGroups = m_Makefile->GetSourceGroups();
  
  // get the classes from the source lists then add them to the groups
  std::vector<cmSourceFile*> & classes = target.GetSourceFiles();
  for(std::vector<cmSourceFile*>::const_iterator i = classes.begin(); 
      i != classes.end(); i++)
    {
    // Add the file to the list of sources.
    std::string source = (*i)->GetFullPath();
    if(cmSystemTools::UpperCase((*i)->GetSourceExtension()) == "DEF")
      {
      m_ModuleDefinitionFile = (*i)->GetFullPath();
      }
    cmSourceGroup& sourceGroup = 
      m_Makefile->FindSourceGroup(source.c_str(), sourceGroups);
    sourceGroup.AssignSource(*i);
    }
  
  // open the project
  this->WriteProjectStart(fout, libName, target, sourceGroups);
  // write the configuration information
  this->WriteConfigurations(fout, libName, target);

  fout << "\t<Files>\n";


  // Loop through every source group.
  for(std::vector<cmSourceGroup>::const_iterator sg = sourceGroups.begin();
      sg != sourceGroups.end(); ++sg)
    {
    const std::vector<const cmSourceFile *> &sourceFiles = 
      sg->GetSourceFiles();
    // If the group is empty, don't write it at all.
    if(sourceFiles.empty())
      { 
      continue; 
      }
    
    // If the group has a name, write the header.
    std::string name = sg->GetName();
    if(name != "")
      {
      this->WriteVCProjBeginGroup(fout, name.c_str(), "");
      }
    
    // Loop through each source in the source group.
    for(std::vector<const cmSourceFile *>::const_iterator sf =
          sourceFiles.begin(); sf != sourceFiles.end(); ++sf)
      {
      std::string source = (*sf)->GetFullPath();
      const cmCustomCommand *command = (*sf)->GetCustomCommand();
      std::string compileFlags;
      std::string additionalDeps;

      // Check for extra compiler flags.
      const char* cflags = (*sf)->GetProperty("COMPILE_FLAGS");
      if(cflags)
        {
        compileFlags = cflags;
        }
      if(cmSystemTools::GetFileFormat((*sf)->GetSourceExtension().c_str())
         == cmSystemTools::CXX_FILE_FORMAT)
        {
        // force a C++ file type
        compileFlags += " /TP ";
        }
      // Check for extra object-file dependencies.
      const char* deps = (*sf)->GetProperty("OBJECT_DEPENDS");
      if(deps)
        {
        std::vector<std::string> depends;
        cmSystemTools::ExpandListArgument(deps, depends);
        if(!depends.empty())
          {
          std::vector<std::string>::iterator i = depends.begin();
          additionalDeps = this->ConvertToXMLOutputPath(i->c_str());
          for(++i;i != depends.end(); ++i)
            {
            additionalDeps += ";";
            additionalDeps += this->ConvertToXMLOutputPath(i->c_str());
            }
          }
        }
      if (source != libName || target.GetType() == cmTarget::UTILITY)
        {
        fout << "\t\t\t<File\n";
        std::string d = this->ConvertToXMLOutputPathSingle(source.c_str());
        // Tell MS-Dev what the source is.  If the compiler knows how to
        // build it, then it will.
        fout << "\t\t\t\tRelativePath=\"" << d << "\">\n";
        if (command)
          {
          std::string totalCommandStr;
          totalCommandStr = 
            cmSystemTools::ConvertToOutputPath(command->GetCommand().c_str()); 
          totalCommandStr += " ";
          totalCommandStr += command->GetArguments();
          totalCommandStr += "\n";
          const char* comment = command->GetComment().c_str();
          const char* flags = compileFlags.size() ? compileFlags.c_str(): 0;
          this->WriteCustomRule(fout, source.c_str(), totalCommandStr.c_str(), 
                                (*comment?comment:"Custom Rule"),
                                command->GetDepends(),
                                command->GetOutput().c_str(), flags);
          }
        else if(compileFlags.size() || additionalDeps.length())
          {
          for(std::vector<std::string>::iterator i = configs->begin(); 
              i != configs->end(); ++i)
            {
            fout << "\t\t\t\t<FileConfiguration\n"
                 << "\t\t\t\t\tName=\""  << *i << "|Win32\">\n"
                 << "\t\t\t\t\t<Tool\n"
                 << "\t\t\t\t\tName=\"VCCLCompilerTool\"\n";
            if(compileFlags.size())
              {
              fout << "\t\t\t\t\tAdditionalOptions=\""
                   << compileFlags << "\"\n";
              }
            if(additionalDeps.length())
              {
              fout << "\t\t\t\t\tAdditionalDependencies=\""
                   << additionalDeps.c_str() << "\"\n";
              }
            fout << "\t\t\t\t\t/>\n"
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


void cmLocalVisualStudio7Generator::
WriteCustomRule(std::ostream& fout,
                const char* source,
                const char* command,
                const char* comment,
                const std::vector<std::string>& depends,
                const char *output,
                const char* compileFlags)
{
  std::string cmd = command;
  cmSystemTools::ReplaceString(cmd, "\"", "&quot;");
  std::vector<std::string>::iterator i;
  std::vector<std::string> *configs = 
    static_cast<cmGlobalVisualStudio7Generator *>(m_GlobalGenerator)->GetConfigurations();
  for(i = configs->begin(); i != configs->end(); ++i)
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
         << "\t\t\t\t\tDescription=\"Building " << comment;
    fout << " " << output;
    fout << "\"\n"
         << "\t\t\t\t\tCommandLine=\"" << cmd << "\n\"\n"
         << "\t\t\t\t\tAdditionalDependencies=\"";
    // Write out the dependencies for the rule.
    std::string temp;
    for(std::vector<std::string>::const_iterator d = depends.begin();
        d != depends.end(); ++d)
      {  
      std::string dep = cmSystemTools::GetFilenameName(*d);
      if (cmSystemTools::GetFilenameLastExtension(dep) == ".exe")
        {
        dep = cmSystemTools::GetFilenameWithoutLastExtension(dep);
        }
      std::string libPath = dep + "_CMAKE_PATH";
      const char* cacheValue = m_Makefile->GetDefinition(libPath.c_str());
      if (cacheValue)
        { 
        std::string exePath = "";
        if (m_Makefile->GetDefinition("EXECUTABLE_OUTPUT_PATH"))
          {
          exePath = m_Makefile->GetDefinition("EXECUTABLE_OUTPUT_PATH");
          }
        if(exePath.size())
          {
           libPath = exePath;
          }
        else
          {
          libPath = cacheValue;
          }
        libPath += "/";
        libPath += "$(INTDIR)";
        libPath += dep;
        libPath += ".exe";
        fout << this->ConvertToXMLOutputPath(libPath.c_str())
             << ";";
        }
      else
        {
        fout << this->ConvertToXMLOutputPath(d->c_str())
             << ";";
        }
      }
    fout << "\"\n";
    fout << "\t\t\t\t\tOutputs=\"";
    if(output == 0)
      {
      fout << source << "_force";
      }
    
    // Write a rule for the output generated by this command.
    fout << this->ConvertToXMLOutputPathSingle(output);
    fout << "\"/>\n";
    fout << "\t\t\t\t</FileConfiguration>\n";
    }
}


void cmLocalVisualStudio7Generator::WriteVCProjBeginGroup(std::ostream& fout, 
                                        const char* group,
                                        const char* )
{
  fout << "\t\t<Filter\n"
       << "\t\t\tName=\"" << group << "\"\n"
       << "\t\t\tFilter=\"\">\n";
}


void cmLocalVisualStudio7Generator::WriteVCProjEndGroup(std::ostream& fout)
{
  fout << "\t\t</Filter>\n";
}


// look for custom rules on a target and collect them together
void cmLocalVisualStudio7Generator::OutputTargetRules(std::ostream& fout,
                                                      const cmTarget &target, 
                                                      const char * /* libName */)
{
  if (target.GetType() > cmTarget::UTILITY)
    {
    return;
    }
  
  // add the pre build rules
  fout << "\t\t\t<Tool\n\t\t\t\tName=\"VCPreBuildEventTool\"";
  bool init = false;
  for (std::vector<cmCustomCommand>::const_iterator cr = 
         target.GetPreBuildCommands().begin(); 
       cr != target.GetPreBuildCommands().end(); ++cr)
    {
    cmCustomCommand cc(*cr);
    cc.ExpandVariables(*m_Makefile);
    if(!init)
      {
      fout << "\nCommandLine=\"";
      init = true;
      }
    std::string args = cc.GetArguments();
    cmSystemTools::ReplaceString(args, "\"", "&quot;");
    fout << this->ConvertToXMLOutputPath(cc.GetCommand().c_str()) << " " << 
      args << "\n";
    }
  if (init)
    {
    fout << "\"";
    }
  fout << "/>\n";

  // add the pre Link rules
  fout << "\t\t\t<Tool\n\t\t\t\tName=\"VCPreLinkEventTool\"";
  init = false;
  for (std::vector<cmCustomCommand>::const_iterator cr = 
         target.GetPreLinkCommands().begin(); 
       cr != target.GetPreLinkCommands().end(); ++cr)
    {
    cmCustomCommand cc(*cr);
    cc.ExpandVariables(*m_Makefile);
    if(!init)
      {
      fout << "\nCommandLine=\"";
      init = true;
      }
    std::string args = cc.GetArguments();
    cmSystemTools::ReplaceString(args, "\"", "&quot;");
    fout << this->ConvertToXMLOutputPath(cc.GetCommand().c_str()) << " " << 
      args << "\n";
    }
  if (init)
    {
    fout << "\"";
    }
  fout << "/>\n";
  
  // add the PostBuild rules
  fout << "\t\t\t<Tool\n\t\t\t\tName=\"VCPostBuildEventTool\"";
  init = false;
  for (std::vector<cmCustomCommand>::const_iterator cr = 
         target.GetPostBuildCommands().begin(); 
       cr != target.GetPostBuildCommands().end(); ++cr)
    {
    cmCustomCommand cc(*cr);
    cc.ExpandVariables(*m_Makefile);
    if(!init)
      {
      fout << "\nCommandLine=\"";
      init = true;
      }
    std::string args = cc.GetArguments();
    cmSystemTools::ReplaceString(args, "\"", "&quot;");
    fout << this->ConvertToXMLOutputPath(cc.GetCommand().c_str()) << " " << 
      args << "\n";
    }
  if (init)
    {
    fout << "\"";
    }
  fout << "/>\n";
}

void 
cmLocalVisualStudio7Generator::WriteProjectStart(std::ostream& fout, 
                                                 const char *libName,
                                                 const cmTarget &, 
                                                 std::vector<cmSourceGroup> &)
{
  fout << "<?xml version=\"1.0\" encoding = \"Windows-1252\"?>\n"
       << "<VisualStudioProject\n"
       << "\tProjectType=\"Visual C++\"\n";
  if(m_Version71)
    {
    fout << "\tVersion=\"7.10\"\n";
    }
  else
    {
    fout << "\tVersion=\"7.00\"\n";
    }
  
  fout << "\tName=\"" << libName << "\"\n"
       << "\tSccProjectName=\"\"\n"
       << "\tSccLocalPath=\"\"\n"
       << "\tKeyword=\"Win32Proj\">\n"
       << "\t<Platforms>\n"
       << "\t\t<Platform\n\t\t\tName=\"Win32\"/>\n"
       << "\t</Platforms>\n";
}


void cmLocalVisualStudio7Generator::WriteVCProjFooter(std::ostream& fout)
{
  fout << "\t<Globals>\n"
       << "\t</Globals>\n"
       << "</VisualStudioProject>\n";
}


std::string cmLocalVisualStudio7Generator::EscapeForXML(const char* s)
{
  std::string ret = s;
  cmSystemTools::ReplaceString(ret, "&", "&amp;");
  cmSystemTools::ReplaceString(ret, "\"", "&quot;");
  cmSystemTools::ReplaceString(ret, "<", "&lt;");
  cmSystemTools::ReplaceString(ret, ">", "&gt;");
  return ret;
}

std::string cmLocalVisualStudio7Generator::ConvertToXMLOutputPath(const char* path)
{
  std::string ret = cmSystemTools::ConvertToOutputPath(path);
  cmSystemTools::ReplaceString(ret, "&", "&amp;");
  cmSystemTools::ReplaceString(ret, "\"", "&quot;");
  cmSystemTools::ReplaceString(ret, "<", "&lt;");
  cmSystemTools::ReplaceString(ret, ">", "&gt;");
  return ret;
}

std::string cmLocalVisualStudio7Generator::ConvertToXMLOutputPathSingle(const char* path)
{
  std::string ret = cmSystemTools::ConvertToOutputPath(path);
  cmSystemTools::ReplaceString(ret, "\"", "");
  cmSystemTools::ReplaceString(ret, "&", "&amp;");
  cmSystemTools::ReplaceString(ret, "<", "&lt;");
  cmSystemTools::ReplaceString(ret, ">", "&gt;");
  return ret;
}


