/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) 2001 Insight Consortium
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * The name of the Insight Consortium, nor the names of any consortium members,
   nor of any contributors, may be used to endorse or promote products derived
   from this software without specific prior written permission.

  * Modified source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "cmCableWrapTclCommand.h"
#include "cmCacheManager.h"
#include "cmTarget.h"
#include "cmGeneratedFileStream.h"
#include "cmMakeDepend.h"

cmCableWrapTclCommand::cmCableWrapTclCommand():
  m_CableClassSet(NULL), m_MakeDepend(new cmMakeDepend)
{
}

cmCableWrapTclCommand::~cmCableWrapTclCommand()
{
  if(m_CableClassSet)
    {
    delete m_CableClassSet;
    }
  delete m_MakeDepend;
}



// cmCableWrapTclCommand
bool cmCableWrapTclCommand::InitialPass(std::vector<std::string>& args)
{
  if(args.size() < 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // First, we want to expand all CMAKE variables in all arguments.
  for(std::vector<std::string>::iterator a = args.begin();
      a != args.end(); ++a)
    {
    m_Makefile->ExpandVariablesInString(*a);
    }
  
  // Prepare to iterate through the arguments.
  std::vector<std::string>::const_iterator arg = args.begin();
  
  // The first argument is the name of the target.
  m_TargetName = *arg++;
  
  // Create the new class set.
  m_CableClassSet = new cmCableClassSet(m_TargetName.c_str());
  
  // Add all the regular entries.
  for(; (arg != args.end()) && (*arg != "SOURCES_BEGIN"); ++arg)
    {
    m_CableClassSet->ParseAndAddElement(arg->c_str(), m_Makefile);
    }
  
  // Add any sources that are associated with all the members.
  if(arg != args.end())
    {
    for(++arg; arg != args.end(); ++arg)
      {
      m_CableClassSet->AddSource(arg->c_str());
      }
    }  

  this->GenerateCableFiles();
  
  // Add the source list to the target.
  m_Makefile->GetTargets()[m_TargetName.c_str()].GetSourceLists().push_back(m_TargetName);
  
  return true;
}


/**
 * Generate the files that CABLE will use to generate the wrappers.
 */
void cmCableWrapTclCommand::GenerateCableFiles() const
{
  // Make sure the dependency generator is ready to go.
  m_MakeDepend->SetMakefile(m_Makefile);

  // Each wrapped class may have an associated "tag" that represents
  // an alternative name without funky C++ syntax in it.  This makes
  // it easier to refer to the class in a Tcl script.  We will also
  // use the tags to make easy-to-read, unique file names for each
  // class's wrapper.  Count the number of times each tag is used.
  // Warn if a tag is used more than once.
  std::map<cmStdString, unsigned int> tagCounts;
  for(cmCableClassSet::CableClassMap::const_iterator
        c = m_CableClassSet->Begin(); c != m_CableClassSet->End(); ++c)
    {
    std::string tag = c->second->GetTag();
    if((++tagCounts[tag] > 1) && (tag != ""))
      {
      std::string message =
        "CABLE_WRAP_TCL has found two classes with the tag "+tag
        +" for target "+m_TargetName;
      cmSystemTools::Message(message.c_str(), "Warning");
      }
    }

  // Each class wrapper will be written into its own CABLE "group"
  // file.  This should hold the names of the groups generated so that
  // the package configuration file can tell cable how to generate the
  // package initialization code.
  std::vector<std::string> groupTags;

  // Setup the output directory name and make sure it exists.
  std::string outDir = m_Makefile->GetCurrentOutputDirectory();
  cmSystemTools::MakeDirectory((outDir+"/Tcl").c_str());

  // Write out the cable configuration files with one class per group.
  // Try to name the groups based on their class's tag, but use an
  // index to disambiguate tag repeats (mostly used for empty tags).
  std::map<cmStdString, unsigned int> tagIndexes;
  for(cmCableClassSet::CableClassMap::const_iterator
        c = m_CableClassSet->Begin(); c != m_CableClassSet->End(); ++c)
    {
    // Construct the group's tag-based name, with index if necessary.
    std::string tag = c->second->GetTag();
    std::string groupTag;
    if(tagCounts[tag] > 1)
      {
      unsigned int tagIndex = tagIndexes[tag]++;
      std::strstream indexStrStream;
      indexStrStream << tagIndex << std::ends;
      std::string indexStr = indexStrStream.str();
      groupTag = "_"+indexStr;
      }
    if(tag != "")
      {
      groupTag += "_"+tag;
      }
    
    // Save this group tag in the list of tags for the main package
    // configuration file below.
    groupTags.push_back(groupTag);
    
    // Actually generate the class's configuration file.
    this->GenerateCableClassFiles(c->first.c_str(), *(c->second),
                                  groupTag.c_str());
    }  

  // Construct the output file names.
  std::string packageConfigName = outDir+"/Tcl/"+m_TargetName+"_config.xml";
  std::string packageTclFileName = "Tcl/"+m_TargetName+"_tcl";
  std::string packageTclFullName = outDir+"/"+packageTclFileName;  
  
  // Generate the main package configuration file for CABLE.  This
  // just lists the "group" files generated above.
  cmGeneratedFileStream packageConfig(packageConfigName.c_str());
  if(packageConfig)
    {
    packageConfig <<
      "<CableConfiguration package=\"" << m_TargetName.c_str() << "\">\n";
    for(std::vector<std::string>::const_iterator g = groupTags.begin();
        g != groupTags.end(); ++g)
      {
      packageConfig <<
        "  <Group name=\"" << m_TargetName.c_str() << g->c_str() << "\"/>\n";
      }
    packageConfig <<
      "</CableConfiguration>\n";
  
    packageConfig.close();
    }
  else
    {
    cmSystemTools::Error("Error opening CABLE configuration file for writing: ",
                         packageConfigName.c_str());
    }
  
  // Generate the rule to run CABLE for the package configuration file.
  std::string command = "${CABLE}";
  m_Makefile->ExpandVariablesInString(command);
  std::vector<std::string> depends;
  depends.push_back(command);
  std::string commandArgs = " "+packageConfigName+
    " -tcl "+packageTclFullName+".cxx";
  
  depends.push_back(packageConfigName);
  
  std::vector<std::string> outputs;
  outputs.push_back(packageTclFileName+".cxx");
  
  m_Makefile->AddCustomCommand(packageConfigName.c_str(),
                               command.c_str(),
                               commandArgs.c_str(),
                               depends,
                               outputs, m_TargetName.c_str());

  // Add the generated source to the package target's source list.
  cmSourceFile file;
  file.SetName(packageTclFileName.c_str(), outDir.c_str(), "cxx", false);
  // Set dependency hints.
  file.GetDepends().push_back("WrapTclFacility/wrapCalls.h");
  m_Makefile->AddSource(file, m_TargetName.c_str());
}


void cmCableWrapTclCommand::GenerateCableClassFiles(const char* name,
                                                    const cmCableClass& c,
                                                    const char* groupTag) const
{  
  std::string outDir = m_Makefile->GetCurrentOutputDirectory();  
  
  std::string className = name;
  std::string groupName = m_TargetName+groupTag;
  std::string classConfigName = outDir+"/Tcl/"+groupName+"_config_tcl.xml";
  std::string classCxxName = outDir+"/Tcl/"+groupName+"_cxx.cc";
  std::string classXmlName = outDir+"/Tcl/"+groupName+"_cxx.xml";
  std::string classTclFileName = "Tcl/"+groupName+"_tcl";
  std::string classTclFullName = outDir+"/"+classTclFileName;
  
  cmGeneratedFileStream classConfig(classConfigName.c_str());
  if(classConfig)
    {
    classConfig <<
      "<CableConfiguration source=\"" << classXmlName.c_str() << "\" "
      "group=\"" << groupName.c_str() << "\">\n";
    for(cmCableClass::Sources::const_iterator source = c.SourcesBegin();
        source != c.SourcesEnd(); ++source)
      {
      classConfig <<
        "  <Header name=\"" << source->c_str() << "\"/>\n";
      }
    classConfig <<
      "  <Class name=\"_wrap_::wrapper::Wrapper\">\n";
    if(c.GetTag() != "")
      {
      classConfig <<
        "    <AlternateName name=\"" << c.GetTag().c_str() << "\"/>\n";
      }
    classConfig <<
      "  </Class>\n"
      "</CableConfiguration>\n";
  
    classConfig.close();
    }
  else
    {
    cmSystemTools::Error("Error opening CABLE configuration file for writing: ",
                         classConfigName.c_str());
    }

  cmGeneratedFileStream classCxx(classCxxName.c_str());
  if(classCxx)
    {
    for(cmCableClass::Sources::const_iterator source = c.SourcesBegin();
        source != c.SourcesEnd(); ++source)
      {
      classCxx <<
        "#include \"" << source->c_str() << "\"\n";
      }
    classCxx <<
      "\n"
      "namespace _wrap_\n"
      "{\n"
      "\n"
      "struct wrapper\n"
      "{\n"
      "  typedef ::" << className.c_str() << " Wrapper;\n"
      "};\n"
      "\n"
      "template <typename T> void Eat(T) {}\n"
      "\n"
      "void InstantiateMemberDeclarations()\n"
      "{\n"
      "  Eat(sizeof(wrapper::Wrapper));\n"
      "}\n"
      "\n"
      "}\n";
    
    classCxx.close();
    }
  else
    {
    cmSystemTools::Error("Error opening file for writing: ",
                         classCxxName.c_str());
    }

  // Generate the rule to have GCC-XML parse the classes to be wrapped.
  {
  std::string command = this->GetGccXmlFromCache();
  std::vector<std::string> depends;
  depends.push_back(command);

  // Get the dependencies of the file.
  const cmDependInformation* dependInfo =
    m_MakeDepend->FindDependencies(classCxxName.c_str());
  if(dependInfo)
    {
    for(cmDependInformation::DependencySet::const_iterator d = 
          dependInfo->m_DependencySet.begin();
        d != dependInfo->m_DependencySet.end(); ++d)
      {
      // Make sure the full path is given.  If not, the dependency was
      // not found.
      if((*d)->m_FullPath != "")
        {
        depends.push_back((*d)->m_FullPath);
        }
      }
    }
  
  std::string commandArgs = this->GetGccXmlFlagsFromCache();
  commandArgs += " ";
  commandArgs += m_Makefile->GetDefineFlags();
  commandArgs += " -I\"";
  commandArgs += m_Makefile->GetStartDirectory();
  commandArgs += "\"";
    
  const std::vector<std::string>& includes = 
    m_Makefile->GetIncludeDirectories();
  for(std::vector<std::string>::const_iterator i = includes.begin();
      i != includes.end(); ++i)
    {
      commandArgs += " -I";
      commandArgs += cmSystemTools::EscapeSpaces(i->c_str());
    }
  
  commandArgs += " -fxml="+classXmlName+" "+classCxxName;
  
  std::vector<std::string> outputs;
  outputs.push_back(classXmlName);
  
  m_Makefile->AddCustomCommand(classCxxName.c_str(),
			       command.c_str(),
			       commandArgs.c_str(),
			       depends,
			       outputs, m_TargetName.c_str());
  }

  // Generate the rule to run cable on the GCC-XML output to generate wrappers.
  {
  std::string command = this->GetCableFromCache();
  std::vector<std::string> depends;
  depends.push_back(command);
  std::string commandArgs = " "+classConfigName+" -tcl "+classTclFullName+".cxx";
  
  depends.push_back(classConfigName);
  depends.push_back(classXmlName);
  
  std::vector<std::string> outputs;
  outputs.push_back(classTclFileName+".cxx");
  
  m_Makefile->AddCustomCommand(classConfigName.c_str(),
                               command.c_str(),
                               commandArgs.c_str(),
                               depends,
                               outputs, m_TargetName.c_str());
  }

  // Add the generated source to the package's source list.
  cmSourceFile file;
  file.SetName(classTclFileName.c_str(), outDir.c_str(), "cxx", false);
  // Set dependency hints.
  for(cmCableClass::Sources::const_iterator source = c.SourcesBegin();
      source != c.SourcesEnd(); ++source)
    {
    file.GetDepends().push_back(*source);
    }
  file.GetDepends().push_back("WrapTclFacility/wrapCalls.h");
  m_Makefile->AddSource(file, m_TargetName.c_str());
}


/**
 * Get the "GCCXML" cache entry value.  If there is no cache entry for GCCXML,
 * one will be created and initialized to NOTFOUND.
 */
std::string cmCableWrapTclCommand::GetGccXmlFromCache() const
{
  const char* gccxml =
    m_Makefile->GetDefinition("GCCXML");
  if(gccxml)
    { return gccxml; }

  m_Makefile->AddCacheDefinition("GCCXML",
                                 "NOTFOUND",
                                 "Path to GCC-XML executable.",
                                 cmCacheManager::FILEPATH);
  return "NOTFOUND";
}


/**
 * Get the "GCCXML_FLAGS" cache entry value.  If there is no cache
 * entry for GCCXML_FLAGS, one will be created and initialized "".
 */
std::string cmCableWrapTclCommand::GetGccXmlFlagsFromCache() const
{
  const char* gccxmlFlags =
    m_Makefile->GetDefinition("GCCXML_FLAGS");
  if(gccxmlFlags)
    { return gccxmlFlags; }

  m_Makefile->AddCacheDefinition(
    "GCCXML_FLAGS",
    "",
    "Flags to GCC-XML to get it to parse the native compiler's headers.",
    cmCacheManager::STRING);
  return "";
}


/**
 * Get the "CABLE" cache entry value.  If there is no cache entry for CABLE,
 * one will be created and initialized to NOTFOUND.
 */
std::string cmCableWrapTclCommand::GetCableFromCache() const
{
  const char* cable =
    m_Makefile->GetDefinition("CABLE");
  if(cable)
    { return cable; }

  m_Makefile->AddCacheDefinition("CABLE",
                                 "NOTFOUND",
                                 "Path to CABLE executable.",
                                 cmCacheManager::FILEPATH);
  return "NOTFOUND";
}
