/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmVisualStudio10TargetGenerator.h"
#include "cmGlobalVisualStudio10Generator.h"
#include "cmTarget.h"
#include "cmComputeLinkInformation.h"
#include "cmGeneratedFileStream.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmVisualStudioGeneratorOptions.h"
#include "cmLocalVisualStudio7Generator.h"
#include "cmVS10CLFlagTable.h"
#include "cmVS10LinkFlagTable.h"
#include "cmVS10LibFlagTable.h"

#include <cmsys/auto_ptr.hxx>

static std::string cmVS10EscapeXML(std::string arg)
{
  cmSystemTools::ReplaceString(arg, "&", "&amp;");
  cmSystemTools::ReplaceString(arg, "<", "&lt;");
  cmSystemTools::ReplaceString(arg, ">", "&gt;");
  return arg;
}

static std::string cmVS10EscapeComment(std::string comment)
{
  // MSBuild takes the CDATA of a <Message></Message> element and just
  // does "echo $CDATA" with no escapes.  We must encode the string.
  // http://technet.microsoft.com/en-us/library/cc772462%28WS.10%29.aspx
  std::string echoable;
  for(std::string::iterator c = comment.begin(); c != comment.end(); ++c)
    {
    switch (*c)
      {
      case '\r': break;
      case '\n': echoable += '\t'; break;
      case '"': /* no break */
      case '|': /* no break */
      case '&': /* no break */
      case '<': /* no break */
      case '>': /* no break */
      case '^': echoable += '^'; /* no break */
      default:  echoable += *c; break;
      }
    }
  return echoable;
}

cmVisualStudio10TargetGenerator::
cmVisualStudio10TargetGenerator(cmTarget* target,
                                cmGlobalVisualStudio10Generator* gg)
{
  this->GlobalGenerator = gg;
  this->Target = target;
  this->Makefile = target->GetMakefile();
  this->LocalGenerator =  
    (cmLocalVisualStudio7Generator*)
    this->Makefile->GetLocalGenerator();
  this->Name = this->Target->GetName();
  this->GlobalGenerator->CreateGUID(this->Name.c_str());
  this->GUID = this->GlobalGenerator->GetGUID(this->Name.c_str());
  this->Platform = gg->GetPlatformName();
  this->ComputeObjectNames();
  this->BuildFileStream = 0;
}

cmVisualStudio10TargetGenerator::~cmVisualStudio10TargetGenerator()
{
  for(OptionsMap::iterator i = this->ClOptions.begin();
      i != this->ClOptions.end(); ++i)
    {
    delete i->second;
    }
  if(!this->BuildFileStream)
    {
    return;
    }
  if (this->BuildFileStream->Close())
    {
    this->GlobalGenerator
      ->FileReplacedDuringGenerate(this->PathToVcxproj);
    }
  delete this->BuildFileStream;
}

void cmVisualStudio10TargetGenerator::WritePlatformConfigTag(
  const char* tag,
  const char* config,
  int indentLevel,
  const char* attribute,
  const char* end,
  std::ostream* stream)

{
  if(!stream)
    {
    stream = this->BuildFileStream;
    }
  stream->fill(' ');
  stream->width(indentLevel*2 ); 
  (*stream ) << "";
  (*stream ) << "<" << tag 
             << " Condition=\"'$(Configuration)|$(Platform)'=='";
  (*stream ) << config << "|" << this->Platform << "'\"";
  if(attribute)
    {
    (*stream ) << attribute;
    }
  // close the tag
  (*stream ) << ">";
  if(end)
    {
    (*stream ) << end;
    }
}

void cmVisualStudio10TargetGenerator::WriteString(const char* line,
                                                  int indentLevel)
{
  this->BuildFileStream->fill(' ');
  this->BuildFileStream->width(indentLevel*2 );
  // write an empty string to get the fill level indent to print
  (*this->BuildFileStream ) << "";
  (*this->BuildFileStream ) << line;
}

#define VS10_USER_PROPS "$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props"

void cmVisualStudio10TargetGenerator::Generate()
{
  // do not generate external ms projects
  if(this->Target->GetProperty("EXTERNAL_MSPROJECT"))
    {
    return;
    }
  // Tell the global generator the name of the project file
  this->Target->SetProperty("GENERATOR_FILE_NAME",this->Name.c_str());
  this->Target->SetProperty("GENERATOR_FILE_NAME_EXT",
                            ".vcxproj");
  if(this->Target->GetType() <= cmTarget::MODULE_LIBRARY)
    {
    if(!this->ComputeClOptions())
      {
      return;
      }
    }
  cmMakefile* mf = this->Target->GetMakefile();
  std::string path =  mf->GetStartOutputDirectory();
  path += "/";
  path += this->Name;
  path += ".vcxproj";
  this->BuildFileStream =
    new cmGeneratedFileStream(path.c_str());
  this->PathToVcxproj = path;
  this->BuildFileStream->SetCopyIfDifferent(true);
  
  // Write the encoding header into the file
  char magic[] = {0xEF,0xBB, 0xBF};
  this->BuildFileStream->write(magic, 3);
  this->WriteString("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n",0);
  this->WriteString("<Project DefaultTargets=\"Build\" "
                    "ToolsVersion=\"4.0\" "
                    "xmlns=\"http://schemas.microsoft.com/"
                    "developer/msbuild/2003\">\n",
                    0);
  this->WriteProjectConfigurations();
  this->WriteString("<PropertyGroup Label=\"Globals\">\n", 1);
  this->WriteString("<ProjectGUID>", 2);
  (*this->BuildFileStream) <<  "{" << this->GUID << "}</ProjectGUID>\n";

  const char* vsProjectTypes =
    this->Target->GetProperty("VS_GLOBAL_PROJECT_TYPES");
  if(vsProjectTypes)
    {
    this->WriteString("<ProjectTypes>", 2);
    (*this->BuildFileStream) << cmVS10EscapeXML(vsProjectTypes) <<
      "</ProjectTypes>\n";
    }

  const char* vsProjectName = this->Target->GetProperty("VS_SCC_PROJECTNAME");
  const char* vsLocalPath = this->Target->GetProperty("VS_SCC_LOCALPATH");
  const char* vsProvider = this->Target->GetProperty("VS_SCC_PROVIDER");

  if( vsProjectName && vsLocalPath && vsProvider )
    {
    this->WriteString("<SccProjectName>", 2);
    (*this->BuildFileStream) << cmVS10EscapeXML(vsProjectName) <<
      "</SccProjectName>\n";
    this->WriteString("<SccLocalPath>", 2);
    (*this->BuildFileStream) << cmVS10EscapeXML(vsLocalPath) <<
      "</SccLocalPath>\n";
    this->WriteString("<SccProvider>", 2);
    (*this->BuildFileStream) << cmVS10EscapeXML(vsProvider) <<
      "</SccProvider>\n";

    const char* vsAuxPath = this->Target->GetProperty("VS_SCC_AUXPATH");
    if( vsAuxPath )
      {
      this->WriteString("<SccAuxPath>", 2);
       (*this->BuildFileStream) << cmVS10EscapeXML(vsAuxPath) <<
         "</SccAuxPath>\n";
      }
    }

  const char* vsGlobalKeyword =
    this->Target->GetProperty("VS_GLOBAL_KEYWORD");
  if(!vsGlobalKeyword)
    {
    this->WriteString("<Keyword>Win32Proj</Keyword>\n", 2);
    }
  else
    {
    this->WriteString("<Keyword>", 2);
    (*this->BuildFileStream) << cmVS10EscapeXML(vsGlobalKeyword) <<
      "</Keyword>\n";
    }

  this->WriteString("<Platform>", 2);
  (*this->BuildFileStream) << this->Platform << "</Platform>\n";
  const char* projLabel = this->Target->GetProperty("PROJECT_LABEL");
  if(!projLabel)
    {
    projLabel = this->Name.c_str();
    }
  this->WriteString("<ProjectName>", 2);
  (*this->BuildFileStream) << projLabel << "</ProjectName>\n";
  this->WriteString("</PropertyGroup>\n", 1);
  this->WriteString("<Import Project="
                    "\"$(VCTargetsPath)\\Microsoft.Cpp.Default.props\" />\n",
                    1);
  this->WriteProjectConfigurationValues();
  this->WriteString(
    "<Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.props\" />\n", 1);
  this->WriteString("<ImportGroup Label=\"ExtensionSettings\">\n", 1);
  this->WriteString("</ImportGroup>\n", 1);
  this->WriteString("<ImportGroup Label=\"PropertySheets\">\n", 1);
  this->WriteString("<Import Project=\"" VS10_USER_PROPS "\""
                    " Condition=\"exists('" VS10_USER_PROPS "')\""
                    " Label=\"LocalAppDataPlatform\" />", 2);
  this->WriteString("</ImportGroup>\n", 1);
  this->WriteString("<PropertyGroup Label=\"UserMacros\" />\n", 1);
  this->WritePathAndIncrementalLinkOptions();
  this->WriteItemDefinitionGroups();
  this->WriteCustomCommands();
  this->WriteObjSources();
  this->WriteCLSources();
  this->WriteDotNetReferences();
  this->WriteProjectReferences();
  this->WriteString(
    "<Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\""
    " />\n", 1);
  this->WriteString("<ImportGroup Label=\"ExtensionTargets\">\n", 1);
  this->WriteString("</ImportGroup>\n", 1);
  this->WriteString("</Project>", 0);
  // The groups are stored in a separate file for VS 10
  this->WriteGroups();
}

void cmVisualStudio10TargetGenerator::WriteDotNetReferences()
{
  const char* vsDotNetReferences
    = this->Target->GetProperty("VS_DOTNET_REFERENCES");
  if(vsDotNetReferences)
    {
    std::string references(vsDotNetReferences);
    std::string::size_type position = 0;

    this->WriteString("<ItemGroup>\n", 1);
    while(references.length() > 0)
      {
      if((position = references.find(";")) == std::string::npos)
        {
        position = references.length() + 1;
        }

      this->WriteString("<Reference Include=\"", 2);
      (*this->BuildFileStream) <<
        cmVS10EscapeXML(references.substr(0, position)) << "\">\n";
      this->WriteString("<CopyLocalSatelliteAssemblies>true"
                        "</CopyLocalSatelliteAssemblies>\n", 3);
      this->WriteString("<ReferenceOutputAssembly>true"
                        "</ReferenceOutputAssembly>\n", 3);
      this->WriteString("</Reference>\n", 2);

      references.erase(0, position + 1);
      }

    this->WriteString("</ItemGroup>\n", 1);
    }
}

// ConfigurationType Application, Utility StaticLibrary DynamicLibrary

void cmVisualStudio10TargetGenerator::WriteProjectConfigurations()
{
  this->WriteString("<ItemGroup Label=\"ProjectConfigurations\">\n", 1);
  std::vector<std::string> *configs =
    static_cast<cmGlobalVisualStudio7Generator *>
    (this->GlobalGenerator)->GetConfigurations();
  for(std::vector<std::string>::iterator i = configs->begin();
      i != configs->end(); ++i)
    {
    this->WriteString("<ProjectConfiguration Include=\"", 2);
    (*this->BuildFileStream ) <<  *i << "|" << this->Platform << "\">\n";
    this->WriteString("<Configuration>", 3);
    (*this->BuildFileStream ) <<  *i << "</Configuration>\n";
    this->WriteString("<Platform>", 3);
    (*this->BuildFileStream) << this->Platform << "</Platform>\n";
    this->WriteString("</ProjectConfiguration>\n", 2);
    }
  this->WriteString("</ItemGroup>\n", 1);
}

void cmVisualStudio10TargetGenerator::WriteProjectConfigurationValues()
{
  cmGlobalVisualStudio10Generator* gg =
    static_cast<cmGlobalVisualStudio10Generator*>(this->GlobalGenerator);
  std::vector<std::string> *configs =
    static_cast<cmGlobalVisualStudio7Generator *>
    (this->GlobalGenerator)->GetConfigurations();
  for(std::vector<std::string>::iterator i = configs->begin();
      i != configs->end(); ++i)
    {
    this->WritePlatformConfigTag("PropertyGroup",
                                 i->c_str(),
                                 1, " Label=\"Configuration\"", "\n");
    std::string configType = "<ConfigurationType>";
    switch(this->Target->GetType())
      {
      case cmTarget::SHARED_LIBRARY:
      case cmTarget::MODULE_LIBRARY:
        configType += "DynamicLibrary";
        break;
      case cmTarget::STATIC_LIBRARY:
        configType += "StaticLibrary";
        break;
      case cmTarget::EXECUTABLE:
        configType += "Application";
        break;
      case cmTarget::UTILITY:
        configType += "Utility";
        break;
      }
    configType += "</ConfigurationType>\n";
    this->WriteString(configType.c_str(), 2); 

    const char* mfcFlag = 
      this->Target->GetMakefile()->GetDefinition("CMAKE_MFC_FLAG");
    std::string mfcFlagValue = mfcFlag ? mfcFlag : "0";

    std::string useOfMfcValue = "false";
    if(mfcFlagValue == "1")
      {
      useOfMfcValue = "Static";
      }
    else if(mfcFlagValue == "2")
      {
      useOfMfcValue = "Dynamic";
      }
    std::string mfcLine = "<UseOfMfc>";
    mfcLine += useOfMfcValue + "</UseOfMfc>\n";
    this->WriteString(mfcLine.c_str(), 2);

    if(this->Target->GetType() <= cmTarget::MODULE_LIBRARY &&
       this->ClOptions[*i]->UsingUnicode())
      {
      this->WriteString("<CharacterSet>Unicode</CharacterSet>\n", 2);
      }
    else
      {
      this->WriteString("<CharacterSet>MultiByte</CharacterSet>\n", 2);
      }
    if(const char* toolset = gg->GetPlatformToolset())
      {
      std::string pts = "<PlatformToolset>";
      pts += toolset;
      pts += "</PlatformToolset>\n";
      this->WriteString(pts.c_str(), 2);
      }
    this->WriteString("</PropertyGroup>\n", 1);
    }
}

void cmVisualStudio10TargetGenerator::WriteCustomCommands()
{
  this->SourcesVisited.clear();
  std::vector<cmSourceFile*> const& sources = this->Target->GetSourceFiles();
  for(std::vector<cmSourceFile*>::const_iterator source = sources.begin();
      source != sources.end(); ++source)
    {
    cmSourceFile* sf = *source;
    this->WriteCustomCommand(sf);
    }
}

//----------------------------------------------------------------------------
void cmVisualStudio10TargetGenerator::WriteCustomCommand(cmSourceFile* sf)
{
  if(this->SourcesVisited.insert(sf).second)
    {
    if(std::vector<cmSourceFile*> const* depends =
       this->Target->GetSourceDepends(sf))
      {
      for(std::vector<cmSourceFile*>::const_iterator di = depends->begin();
          di != depends->end(); ++di)
        {
        this->WriteCustomCommand(*di);
        }
      }
    if(cmCustomCommand const* command = sf->GetCustomCommand())
      {
      this->WriteString("<ItemGroup>\n", 1);
      this->WriteCustomRule(sf, *command);
      this->WriteString("</ItemGroup>\n", 1);
      }
    }
}

void 
cmVisualStudio10TargetGenerator::WriteCustomRule(cmSourceFile* source,
                                                 cmCustomCommand const & 
                                                 command)
{
  std::string sourcePath = source->GetFullPath();
  // the rule file seems to need to exist for vs10
  if (source->GetExtension() == "rule")
    {
    if(!cmSystemTools::FileExists(sourcePath.c_str()))
      {
      // Make sure the path exists for the file
      std::string path = cmSystemTools::GetFilenamePath(sourcePath);
      cmSystemTools::MakeDirectory(path.c_str());
      std::ofstream fout(sourcePath.c_str());
      if(fout)
        {
        fout << "# generated from CMake\n";
        fout.flush();
        fout.close();
        }
      else
        {
        std::string error = "Could not create file: [";
        error +=  sourcePath;
        error += "]  ";
        cmSystemTools::Error
          (error.c_str(), cmSystemTools::GetLastSystemError().c_str());
        }
      }
    }
  cmLocalVisualStudio7Generator* lg = this->LocalGenerator;
  std::string comment = lg->ConstructComment(command);
  comment = cmVS10EscapeComment(comment);
  std::vector<std::string> *configs =
    static_cast<cmGlobalVisualStudio7Generator *>
    (this->GlobalGenerator)->GetConfigurations(); 
  this->WriteString("<CustomBuild Include=\"", 2);
  // custom command have to use relative paths or they do not
  // show up in the GUI
  std::string path = cmSystemTools::RelativePath(
    this->Makefile->GetCurrentOutputDirectory(),
    sourcePath.c_str());
  this->ConvertToWindowsSlash(path);
  (*this->BuildFileStream ) << path << "\">\n";
  for(std::vector<std::string>::iterator i = configs->begin();
      i != configs->end(); ++i)
    {
    std::string script =
      cmVS10EscapeXML(lg->ConstructScript(command, i->c_str()));
    this->WritePlatformConfigTag("Message",i->c_str(), 3);
    (*this->BuildFileStream ) << cmVS10EscapeXML(comment) << "</Message>\n";
    this->WritePlatformConfigTag("Command", i->c_str(), 3);
    (*this->BuildFileStream ) << script << "</Command>\n";
    this->WritePlatformConfigTag("AdditionalInputs", i->c_str(), 3);
    
    (*this->BuildFileStream ) << source->GetFullPath();
    for(std::vector<std::string>::const_iterator d = 
          command.GetDepends().begin();
        d != command.GetDepends().end(); 
        ++d)
      {
      std::string dep;
      if(this->LocalGenerator->GetRealDependency(d->c_str(), i->c_str(), dep))
        {
        this->ConvertToWindowsSlash(dep);
        (*this->BuildFileStream ) << ";" << dep;
        }
      }
    (*this->BuildFileStream ) << ";%(AdditionalInputs)</AdditionalInputs>\n";
    this->WritePlatformConfigTag("Outputs", i->c_str(), 3);
    const char* sep = "";
    for(std::vector<std::string>::const_iterator o = 
          command.GetOutputs().begin();
        o != command.GetOutputs().end(); 
        ++o)
      {
      std::string out = *o;
      this->ConvertToWindowsSlash(out);
      (*this->BuildFileStream ) << sep << out;
      sep = ";";
      }
    (*this->BuildFileStream ) << "</Outputs>\n";
    }
  this->WriteString("</CustomBuild>\n", 2);
}

void cmVisualStudio10TargetGenerator::ConvertToWindowsSlash(std::string& s)
{
  // first convert all of the slashes
  std::string::size_type pos = 0;
  while((pos = s.find('/', pos)) != std::string::npos)
    {
    s[pos] = '\\';
    pos++;
    }
}
void cmVisualStudio10TargetGenerator::WriteGroups()
{ 
  // collect up group information
  std::vector<cmSourceGroup> sourceGroups = 
    this->Makefile->GetSourceGroups();
  std::vector<cmSourceFile*>  classes = this->Target->GetSourceFiles();
  
  std::set<cmSourceGroup*> groupsUsed;
  std::vector<cmSourceFile*> clCompile;
  std::vector<cmSourceFile*> customBuild;
  std::vector<cmSourceFile*> none;
  std::vector<cmSourceFile*> headers;
  std::vector<cmSourceFile*> idls;
  std::vector<cmSourceFile*> resource;
  
  for(std::vector<cmSourceFile*>::const_iterator s = classes.begin(); 
      s != classes.end(); s++)
    {
    cmSourceFile* sf = *s; 
    std::string const& source = sf->GetFullPath();
    cmSourceGroup& sourceGroup = 
      this->Makefile->FindSourceGroup(source.c_str(), sourceGroups);
    groupsUsed.insert(&sourceGroup);
    const char* lang = sf->GetLanguage(); 
    bool header = (*s)->GetPropertyAsBool("HEADER_FILE_ONLY")
      || this->GlobalGenerator->IgnoreFile
      ((*s)->GetExtension().c_str());
    std::string ext =
      cmSystemTools::LowerCase((*s)->GetExtension());
    if(!lang)
      {
      lang = "None";
      }
    if(header)
      {
      headers.push_back(sf);
      }
    else if(lang[0] == 'C')
      {
      clCompile.push_back(sf);
      }
    else if(strcmp(lang, "RC") == 0)
      {
      resource.push_back(sf);
      }
    else if(sf->GetCustomCommand())
      {
      customBuild.push_back(sf);
      }
    else if(ext == "idl")
      {
      idls.push_back(sf);
      }
    else
      {
      none.push_back(sf);
      }
    }

  this->AddMissingSourceGroups(groupsUsed, sourceGroups);

  // Write out group file
  std::string path =  this->Makefile->GetStartOutputDirectory();
  path += "/";
  path += this->Name;
  path += ".vcxproj.filters";
  cmGeneratedFileStream fout(path.c_str());
  fout.SetCopyIfDifferent(true);
  char magic[] = {0xEF,0xBB, 0xBF};
  fout.write(magic, 3);
  cmGeneratedFileStream* save = this->BuildFileStream;
  this->BuildFileStream = & fout;
  this->WriteString("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                    "<Project "
                    "ToolsVersion=\"4.0\" "
                    "xmlns=\"http://schemas.microsoft.com/"
                    "developer/msbuild/2003\">\n",
                    0);
  this->WriteGroupSources("ClCompile", clCompile, sourceGroups);
  this->WriteGroupSources("ClInclude", headers, sourceGroups);
  this->WriteGroupSources("ResourceCompile", resource, sourceGroups);
  this->WriteGroupSources("Midl", idls, sourceGroups);
  this->WriteGroupSources("CustomBuild", customBuild, sourceGroups);

  this->WriteString("<ItemGroup>\n", 1);
  for(std::set<cmSourceGroup*>::iterator g = groupsUsed.begin();
      g != groupsUsed.end(); ++g)
    {
    cmSourceGroup* sg = *g;
    const char* name = sg->GetFullName();
    if(strlen(name) != 0)
      {
      this->WriteString("<Filter Include=\"", 2);
      (*this->BuildFileStream) << name << "\">\n";
      std::string guidName = "SG_Filter_";
      guidName += name;
      this->GlobalGenerator->CreateGUID(guidName.c_str());
      this->WriteString("<UniqueIdentifier>", 3);
      std::string guid 
        = this->GlobalGenerator->GetGUID(guidName.c_str());
      (*this->BuildFileStream) 
        << "{"
        << guid << "}"
        << "</UniqueIdentifier>\n";
      this->WriteString("</Filter>\n", 2);
      }
    }
  this->WriteString("</ItemGroup>\n", 1);
  this->WriteGroupSources("None", none, sourceGroups);
  this->WriteString("</Project>\n", 0);
  // restore stream pointer
  this->BuildFileStream = save;

  if (fout.Close())
    {
    this->GlobalGenerator->FileReplacedDuringGenerate(path);
    }
}

// Add to groupsUsed empty source groups that have non-empty children.
void
cmVisualStudio10TargetGenerator::AddMissingSourceGroups(
  std::set<cmSourceGroup*>& groupsUsed,
  const std::vector<cmSourceGroup>& allGroups
  )
{
  for(std::vector<cmSourceGroup>::const_iterator current = allGroups.begin();
      current != allGroups.end(); ++current)
    {
    std::vector<cmSourceGroup> const& children = current->GetGroupChildren();
    if(children.empty())
      {
      continue; // the group is really empty
      }

    this->AddMissingSourceGroups(groupsUsed, children);

    cmSourceGroup* current_ptr = const_cast<cmSourceGroup*>(&(*current));
    if(groupsUsed.find(current_ptr) != groupsUsed.end())
      {
      continue; // group has already been added to set
      }

    // check if it least one of the group's descendants is not empty
    // (at least one child must already have been added)
    std::vector<cmSourceGroup>::const_iterator child_it = children.begin();
    while(child_it != children.end())
      {
      cmSourceGroup* child_ptr = const_cast<cmSourceGroup*>(&(*child_it));
      if(groupsUsed.find(child_ptr) != groupsUsed.end())
        {
        break; // found a child that was already added => add current group too
        }
      child_it++;
      }

    if(child_it == children.end())
      {
      continue; // no descendants have source files => ignore this group
      }

    groupsUsed.insert(current_ptr);
    }
}

void
cmVisualStudio10TargetGenerator::
WriteGroupSources(const char* name,
                  std::vector<cmSourceFile*> const& sources,
                  std::vector<cmSourceGroup>& sourceGroups)
{
  this->WriteString("<ItemGroup>\n", 1);
  for(std::vector<cmSourceFile*>::const_iterator s = sources.begin();
      s != sources.end(); ++s)
    {
    cmSourceFile* sf = *s;
    if(sf->GetExtension() == "obj")
      {
      continue;
      }
    std::string const& source = sf->GetFullPath();
    cmSourceGroup& sourceGroup = 
      this->Makefile->FindSourceGroup(source.c_str(), sourceGroups);
    const char* filter = sourceGroup.GetFullName();
    this->WriteString("<", 2); 
    std::string path = source;
    // custom command sources must use relative paths or they will
    // not show up in the GUI.
    if(sf->GetCustomCommand())
      {
      path = cmSystemTools::RelativePath(
        this->Makefile->GetCurrentOutputDirectory(),
        source.c_str());
      }
    this->ConvertToWindowsSlash(path);
    (*this->BuildFileStream) << name << " Include=\""
                             << path;
    if(strlen(filter))
      {
      (*this->BuildFileStream) << "\">\n";
      this->WriteString("<Filter>", 3);
      (*this->BuildFileStream) << filter << "</Filter>\n";
      this->WriteString("</", 2);
      (*this->BuildFileStream) << name << ">\n";
      }
    else
      {
      (*this->BuildFileStream) << "\" />\n";
      }
    }
  this->WriteString("</ItemGroup>\n", 1);
}

void cmVisualStudio10TargetGenerator::WriteObjSources()
{ 
  if(this->Target->GetType() > cmTarget::MODULE_LIBRARY)
    {
    return;
    }
  bool first = true;
  std::vector<cmSourceFile*>const & sources = this->Target->GetSourceFiles();
  for(std::vector<cmSourceFile*>::const_iterator source = sources.begin();
      source != sources.end(); ++source)
    {
    std::string ext =
      cmSystemTools::LowerCase((*source)->GetExtension());
    if(ext == "obj" || ext == "o")
      {
      if(first)
        {
        this->WriteString("<ItemGroup>\n", 1);
        first = false;
        }
      // If an object file is generated, then vs10
      // will use it in the build, and we have to list
      // it as None instead of Object
      if((*source)->GetPropertyAsBool("GENERATED"))
        {
        this->WriteString("<None Include=\"", 2);
        }
      // If it is not a generated object then we have
      // to use the Object type
      else
        {
        this->WriteString("<Object Include=\"", 2);
        }
      (*this->BuildFileStream ) << (*source)->GetFullPath() << "\" />\n";
      }
    }
  if(!first)
    {
    this->WriteString("</ItemGroup>\n", 1); 
    }
}


void cmVisualStudio10TargetGenerator::WriteCLSources()
{
  if(this->Target->GetType() > cmTarget::UTILITY)
    {
    return;
    }
  this->WriteString("<ItemGroup>\n", 1);
  std::vector<cmSourceFile*>const& sources = this->Target->GetSourceFiles();
  for(std::vector<cmSourceFile*>::const_iterator source = sources.begin();
      source != sources.end(); ++source)
    {
    std::string ext = cmSystemTools::LowerCase((*source)->GetExtension());
    if((*source)->GetCustomCommand() || ext == "o" || ext == "obj")
      {
      continue;
      }
    // If it is not a custom command and it is not a pre-built obj file,
    // then add it as a source (c/c++/header/rc/idl) file
    bool header = (*source)->GetPropertyAsBool("HEADER_FILE_ONLY")
      || this->GlobalGenerator->IgnoreFile(ext.c_str());
    const char* lang = (*source)->GetLanguage();
    bool cl = lang && (strcmp(lang, "C") == 0 || strcmp(lang, "CXX") ==0);
    bool rc = lang && (strcmp(lang, "RC") == 0);
    bool idl = ext == "idl";
    std::string sourceFile = (*source)->GetFullPath();
    // do not use a relative path here because it means that you
    // can not use as long a path to the file.
    this->ConvertToWindowsSlash(sourceFile);
    // output the source file
    if(header)
      {
      this->WriteString("<ClInclude Include=\"", 2);
      }
    else if(cl)
      {
      this->WriteString("<ClCompile Include=\"", 2);
      }
    else if(rc)
      {
      this->WriteString("<ResourceCompile Include=\"", 2);
      }
    else if(idl)
      {
      this->WriteString("<Midl Include=\"", 2);
      }
    else
      {
      this->WriteString("<None Include=\"", 2);
      }
    (*this->BuildFileStream ) << sourceFile << "\"";
    // ouput any flags specific to this source file
    if(!header && cl && this->OutputSourceSpecificFlags(*source))
      {
      // if the source file has specific flags the tag
      // is ended on a new line
      this->WriteString("</ClCompile>\n", 2);
      }
    else if(!header && rc && this->OutputSourceSpecificFlags(*source))
      {
      this->WriteString("</ResourceCompile>\n", 2);
      }
    else
      {
      (*this->BuildFileStream ) << " />\n";
      }
    }
  this->WriteString("</ItemGroup>\n", 1);
}

void cmVisualStudio10TargetGenerator::ComputeObjectNames()
{
  // We may be modifying the source groups temporarily, so make a copy.
  std::vector<cmSourceGroup> sourceGroups = this->Makefile->GetSourceGroups();

  // get the classes from the source lists then add them to the groups
  std::vector<cmSourceFile*>const & classes = this->Target->GetSourceFiles();
  for(std::vector<cmSourceFile*>::const_iterator i = classes.begin();
      i != classes.end(); i++)
    {
    // Add the file to the list of sources.
    std::string source = (*i)->GetFullPath();
    if(cmSystemTools::UpperCase((*i)->GetExtension()) == "DEF")
      {
      this->ModuleDefinitionFile = (*i)->GetFullPath();
      }
    cmSourceGroup& sourceGroup =
      this->Makefile->FindSourceGroup(source.c_str(), sourceGroups);
    sourceGroup.AssignSource(*i);
    }

  // Compute which sources need unique object computation.
  this->LocalGenerator->ComputeObjectNameRequirements(sourceGroups);
}

bool cmVisualStudio10TargetGenerator::OutputSourceSpecificFlags(
  cmSourceFile* source)
{ 
  cmSourceFile& sf = *source;
  cmLocalVisualStudio7Generator* lg = this->LocalGenerator;

  // Compute the maximum length full path to the intermediate
  // files directory for any configuration.  This is used to construct
  // object file names that do not produce paths that are too long.
  std::string dir_max;
  lg->ComputeMaxDirectoryLength(dir_max, *this->Target);

  std::string objectName;
  if(lg->NeedObjectName.find(&sf) != lg->NeedObjectName.end())
    {
    objectName = lg->GetObjectFileNameWithoutTarget(sf, dir_max);
    }
  std::string flags;
  std::string defines;
  if(const char* cflags = sf.GetProperty("COMPILE_FLAGS"))
    {
    flags += cflags;
    } 
  if(const char* cdefs = sf.GetProperty("COMPILE_DEFINITIONS"))
    {
    defines += cdefs;
    }
  const char* lang =
    this->GlobalGenerator->GetLanguageFromExtension
    (sf.GetExtension().c_str());
  const char* sourceLang = this->LocalGenerator->GetSourceFileLanguage(sf);
  const char* linkLanguage = this->Target->GetLinkerLanguage();
  bool needForceLang = false;
  // source file does not match its extension language
  if(lang && sourceLang && strcmp(lang, sourceLang) != 0)
    {
    needForceLang = true;
    lang = sourceLang;
    }  
  // if the source file does not match the linker language
  // then force c or c++
  if(needForceLang || (linkLanguage && lang
                       && strcmp(lang, linkLanguage) != 0))
    {
    if(strcmp(lang, "CXX") == 0)
      {
      // force a C++ file type
      flags += " /TP ";
      }
    else if(strcmp(lang, "C") == 0)
      {
      // force to c
      flags += " /TC ";
      }
    }
  bool hasFlags = false;
  // for the first time we need a new line if there is something
  // produced here.
  const char* firstString = ">\n";
  if(objectName.size())
    {
    (*this->BuildFileStream ) << firstString;
    firstString = ""; 
    hasFlags = true;
    this->WriteString("<ObjectFileName>", 3);
    (*this->BuildFileStream )
      << "$(IntDir)/" << objectName << "</ObjectFileName>\n";
    }
  std::vector<std::string> *configs =
    static_cast<cmGlobalVisualStudio7Generator *>
    (this->GlobalGenerator)->GetConfigurations();
  for( std::vector<std::string>::iterator config = configs->begin();
       config != configs->end(); ++config)
    { 
    std::string configUpper = cmSystemTools::UpperCase(*config);
    std::string configDefines = defines;
    std::string defPropName = "COMPILE_DEFINITIONS_";
    defPropName += configUpper;
    if(const char* ccdefs = sf.GetProperty(defPropName.c_str()))
      {
      if(configDefines.size())
        {
        configDefines += ";";
        }
      configDefines += ccdefs;
      }
    // if we have flags or defines for this config then 
    // use them
    if(flags.size() || configDefines.size())
      {
      (*this->BuildFileStream ) << firstString;
      firstString = ""; // only do firstString once
      hasFlags = true;
      cmVisualStudioGeneratorOptions 
        clOptions(this->LocalGenerator,
                  cmVisualStudioGeneratorOptions::Compiler,
                  cmVS10CLFlagTable, 0, this);
      clOptions.Parse(flags.c_str());
      clOptions.AddDefines(configDefines.c_str());
      clOptions.SetConfiguration((*config).c_str());
      clOptions.OutputAdditionalOptions(*this->BuildFileStream, "      ", "");
      clOptions.OutputFlagMap(*this->BuildFileStream, "      "); 
      clOptions.OutputPreprocessorDefinitions(*this->BuildFileStream,
                                              "      ", "\n", lang);
      }
    }
  return hasFlags;
}


void cmVisualStudio10TargetGenerator::WritePathAndIncrementalLinkOptions()
{
  cmTarget::TargetType ttype = this->Target->GetType();
  if(ttype > cmTarget::GLOBAL_TARGET)
    {
    return;
    }

  this->WriteString("<PropertyGroup>\n", 2);
  this->WriteString("<_ProjectFileVersion>10.0.20506.1"
                    "</_ProjectFileVersion>\n", 3);
  std::vector<std::string> *configs =
    static_cast<cmGlobalVisualStudio7Generator *>
    (this->GlobalGenerator)->GetConfigurations();
  for(std::vector<std::string>::iterator config = configs->begin();
      config != configs->end(); ++config)
    {
    if(ttype >= cmTarget::UTILITY)
      {
      this->WritePlatformConfigTag("IntDir", config->c_str(), 3);
      *this->BuildFileStream
        << "$(Platform)\\$(Configuration)\\$(ProjectName)\\"
        << "</IntDir>\n";
      }
    else
      {
      std::string targetNameFull =
        this->Target->GetFullName(config->c_str());
      std::string intermediateDir = this->LocalGenerator->
        GetTargetDirectory(*this->Target);
      intermediateDir += "/";
      intermediateDir += *config;
      intermediateDir += "/";
      this->ConvertToWindowsSlash(intermediateDir);
      std::string outDir = this->Target->GetDirectory(config->c_str());
      this->ConvertToWindowsSlash(outDir);

      this->WritePlatformConfigTag("OutDir", config->c_str(), 3);
      *this->BuildFileStream << outDir
                             << "\\"
                             << "</OutDir>\n";

      this->WritePlatformConfigTag("IntDir", config->c_str(), 3);
      *this->BuildFileStream << intermediateDir
                             << "</IntDir>\n";

      this->WritePlatformConfigTag("TargetName", config->c_str(), 3);
      *this->BuildFileStream
        << cmSystemTools::GetFilenameWithoutLastExtension(
             targetNameFull.c_str())
        << "</TargetName>\n";

      this->WritePlatformConfigTag("TargetExt", config->c_str(), 3);
      *this->BuildFileStream
        << cmSystemTools::GetFilenameLastExtension(targetNameFull.c_str())
        << "</TargetExt>\n";

      this->OutputLinkIncremental(*config);
      }
    }
  this->WriteString("</PropertyGroup>\n", 2);
}



void 
cmVisualStudio10TargetGenerator::
OutputLinkIncremental(std::string const& configName)
{ 
  std::string CONFIG = cmSystemTools::UpperCase(configName);
  // static libraries and things greater than modules do not need
  // to set this option
  if(this->Target->GetType() == cmTarget::STATIC_LIBRARY
     || this->Target->GetType() > cmTarget::MODULE_LIBRARY)
    {
    return;
    }
  const char* linkType = "SHARED";
  if(this->Target->GetType() == cmTarget::EXECUTABLE)
    {
    linkType = "EXE";
    }
  
  // assume incremental linking
  const char* incremental = "true";
  const char* linkLanguage =
    this->Target->GetLinkerLanguage(configName.c_str());
  if(!linkLanguage)
    {
    cmSystemTools::Error
      ("CMake can not determine linker language for target:",
       this->Name.c_str());
    return;
    }
  std::string linkFlagVarBase = "CMAKE_";
  linkFlagVarBase += linkType;
  linkFlagVarBase += "_LINKER_FLAGS";
  std::string flags = this->
    Target->GetMakefile()->GetRequiredDefinition(linkFlagVarBase.c_str());
  std::string linkFlagVar = linkFlagVarBase + "_" + CONFIG;
  flags += this->
    Target->GetMakefile()->GetRequiredDefinition(linkFlagVar.c_str());
  if(strcmp(linkLanguage, "C") == 0 || strcmp(linkLanguage, "CXX") == 0
     || strcmp(linkLanguage, "Fortran") == 0)
    {
    std::string baseFlagVar = "CMAKE_";
    baseFlagVar += linkLanguage;
    baseFlagVar += "_FLAGS";
    flags += this->
      Target->GetMakefile()->GetRequiredDefinition(baseFlagVar.c_str());
    std::string flagVar = baseFlagVar + std::string("_") + CONFIG;
    flags += 
      Target->GetMakefile()->GetRequiredDefinition(flagVar.c_str());
    }  
  const char* targetLinkFlags = this->Target->GetProperty("LINK_FLAGS");
  if(targetLinkFlags)
    {
    flags += " ";
    flags += targetLinkFlags;
    }
  std::string flagsProp = "LINK_FLAGS_";
  flagsProp += CONFIG;
  if(const char* flagsConfig = this->Target->GetProperty(flagsProp.c_str()))
    {
    flags += " ";
    flags += flagsConfig;
    }
  if(flags.find("INCREMENTAL:NO") != flags.npos)
    {
    incremental = "false";
    }
  this->WritePlatformConfigTag("LinkIncremental", configName.c_str(), 3);
  *this->BuildFileStream << incremental
                         << "</LinkIncremental>\n"; 

  const char* manifest = "true";
  if(flags.find("MANIFEST:NO") != flags.npos)
    {
    manifest = "false";
    }
  this->WritePlatformConfigTag("GenerateManifest", configName.c_str(), 3);
  *this->BuildFileStream << manifest
                         << "</GenerateManifest>\n";
}

//----------------------------------------------------------------------------
bool cmVisualStudio10TargetGenerator::ComputeClOptions()
{
  std::vector<std::string> const* configs =
    this->GlobalGenerator->GetConfigurations();
  for(std::vector<std::string>::const_iterator i = configs->begin();
      i != configs->end(); ++i)
    {
    if(!this->ComputeClOptions(*i))
      {
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------------
bool cmVisualStudio10TargetGenerator::ComputeClOptions(
  std::string const& configName)
{
  // much of this was copied from here:
  // copied from cmLocalVisualStudio7Generator.cxx 805
  // TODO: Integrate code below with cmLocalVisualStudio7Generator.

  cmsys::auto_ptr<Options> pOptions(
    new Options(this->LocalGenerator, Options::Compiler,
                cmVS10CLFlagTable));
  Options& clOptions = *pOptions;

  std::string flags;
  // collect up flags for 
  if(this->Target->GetType() < cmTarget::UTILITY)
    {
    const char* linkLanguage =
      this->Target->GetLinkerLanguage(configName.c_str());
    if(!linkLanguage)
      {
      cmSystemTools::Error
        ("CMake can not determine linker language for target:",
         this->Name.c_str());
      return false;
      }
    if(strcmp(linkLanguage, "C") == 0 || strcmp(linkLanguage, "CXX") == 0
       || strcmp(linkLanguage, "Fortran") == 0)
      {
      std::string baseFlagVar = "CMAKE_";
      baseFlagVar += linkLanguage;
      baseFlagVar += "_FLAGS";
      flags = this->
        Target->GetMakefile()->GetRequiredDefinition(baseFlagVar.c_str());
      std::string flagVar = baseFlagVar + std::string("_") +
        cmSystemTools::UpperCase(configName);
      flags += " ";
      flags += this->
        Target->GetMakefile()->GetRequiredDefinition(flagVar.c_str());
      }
    // set the correct language
    if(strcmp(linkLanguage, "C") == 0)
      {
      flags += " /TC ";
      }
    if(strcmp(linkLanguage, "CXX") == 0)
      {
      flags += " /TP ";
      }
    }
  // Add the target-specific flags.
  if(const char* targetFlags = this->Target->GetProperty("COMPILE_FLAGS"))
    {
    flags += " ";
    flags += targetFlags;
    }
  std::string configUpper = cmSystemTools::UpperCase(configName);
  std::string defPropName = "COMPILE_DEFINITIONS_";
  defPropName += configUpper;

  // Get preprocessor definitions for this directory.
  std::string defineFlags = this->Target->GetMakefile()->GetDefineFlags();
  clOptions.FixExceptionHandlingDefault();
  clOptions.Parse(flags.c_str());
  clOptions.Parse(defineFlags.c_str());
  clOptions.AddDefines
    (this->Makefile->GetProperty("COMPILE_DEFINITIONS"));
  clOptions.AddDefines(this->Target->GetProperty("COMPILE_DEFINITIONS"));
  clOptions.AddDefines(this->Makefile->GetProperty(defPropName.c_str()));
  clOptions.AddDefines(this->Target->GetProperty(defPropName.c_str()));
  clOptions.SetVerboseMakefile(
    this->Makefile->IsOn("CMAKE_VERBOSE_MAKEFILE"));

  // Add a definition for the configuration name.
  std::string configDefine = "CMAKE_INTDIR=\"";
  configDefine += configName;
  configDefine += "\"";
  clOptions.AddDefine(configDefine);
  if(const char* exportMacro = this->Target->GetExportMacro())
    {
    clOptions.AddDefine(exportMacro);
    }

  this->ClOptions[configName] = pOptions.release();
  return true;
}

//----------------------------------------------------------------------------
void cmVisualStudio10TargetGenerator::WriteClOptions(
  std::string const& configName,
  std::vector<std::string> const& includes)
{
  Options& clOptions = *(this->ClOptions[configName]);
  this->WriteString("<ClCompile>\n", 2);
  clOptions.OutputAdditionalOptions(*this->BuildFileStream, "      ", "");
  this->OutputIncludes(includes);
  clOptions.OutputFlagMap(*this->BuildFileStream, "      ");

  // If not in debug mode, write the DebugInformationFormat field
  // without value so PDBs don't get generated uselessly.
  if(!clOptions.IsDebug())
    {
    this->WriteString("<DebugInformationFormat>"
                      "</DebugInformationFormat>\n", 3);
    }

  clOptions.OutputPreprocessorDefinitions(*this->BuildFileStream, "      ", 
                                          "\n", "CXX");
  this->WriteString("<AssemblerListingLocation>", 3);
  *this->BuildFileStream << configName 
                         << "</AssemblerListingLocation>\n";
  this->WriteString("<ObjectFileName>$(IntDir)</ObjectFileName>\n", 3);
  this->WriteString("<ProgramDataBaseFileName>", 3);
  *this->BuildFileStream << this->Target->GetDirectory(configName.c_str())
                         << "/" 
                         << this->Target->GetPDBName(configName.c_str())
                         << "</ProgramDataBaseFileName>\n";
  this->WriteString("</ClCompile>\n", 2);
}

void cmVisualStudio10TargetGenerator::
OutputIncludes(std::vector<std::string> const & includes)
{
  this->WriteString("<AdditionalIncludeDirectories>", 3);
  for(std::vector<std::string>::const_iterator i =  includes.begin();
      i != includes.end(); ++i)
    {
    *this->BuildFileStream << *i << ";";
    }
  this->WriteString("%(AdditionalIncludeDirectories)"
                    "</AdditionalIncludeDirectories>\n", 0);
}
  


void cmVisualStudio10TargetGenerator::
WriteRCOptions(std::string const& configName,
               std::vector<std::string> const & includes)
{
  this->WriteString("<ResourceCompile>\n", 2);
  Options& clOptions = *(this->ClOptions[configName]);
  clOptions.OutputPreprocessorDefinitions(*this->BuildFileStream, "      ",
                                          "\n", "RC");
  this->OutputIncludes(includes);
  this->WriteString("</ResourceCompile>\n", 2);
}


void
cmVisualStudio10TargetGenerator::WriteLibOptions(std::string const& config)
{
  if(this->Target->GetType() != cmTarget::STATIC_LIBRARY)
    {
    return;
    }
  const char* libflags = this->Target->GetProperty("STATIC_LIBRARY_FLAGS");
  std::string flagsConfigVar = "STATIC_LIBRARY_FLAGS_";
  flagsConfigVar += cmSystemTools::UpperCase(config);
  const char* libflagsConfig =
    this->Target->GetProperty(flagsConfigVar.c_str());
  if(libflags || libflagsConfig)
    {
    this->WriteString("<Lib>\n", 2);
    cmVisualStudioGeneratorOptions
      libOptions(this->LocalGenerator,
                 cmVisualStudioGeneratorOptions::Linker,
                 cmVS10LibFlagTable, 0, this);
    libOptions.Parse(libflags?libflags:"");
    libOptions.Parse(libflagsConfig?libflagsConfig:"");
    libOptions.OutputAdditionalOptions(*this->BuildFileStream, "      ", "");
    libOptions.OutputFlagMap(*this->BuildFileStream, "      "); 
    this->WriteString("</Lib>\n", 2);
    }
}


void cmVisualStudio10TargetGenerator::WriteLinkOptions(std::string const&
                                                       config)
{
  
  // static libraries and things greater than modules do not need
  // to set this option
  if(this->Target->GetType() == cmTarget::STATIC_LIBRARY
     || this->Target->GetType() > cmTarget::MODULE_LIBRARY)
    {
    return;
    }
  const char* linkLanguage =
    this->Target->GetLinkerLanguage(config.c_str());
  if(!linkLanguage)
    {
    cmSystemTools::Error
      ("CMake can not determine linker language for target:",
       this->Name.c_str());
    return;
    }

  this->WriteString("<Link>\n", 2);
  std::string CONFIG = cmSystemTools::UpperCase(config);
  
  const char* linkType = "SHARED";
  if(this->Target->GetType() == cmTarget::MODULE_LIBRARY)
    {
    linkType = "MODULE";
    }
  if(this->Target->GetType() == cmTarget::EXECUTABLE)
    {
    linkType = "EXE";
    }
  std::string stackVar = "CMAKE_";
  stackVar += linkLanguage;
  stackVar += "_STACK_SIZE";
  const char* stackVal = this->Makefile->GetDefinition(stackVar.c_str());
  std::string flags;
  if(stackVal)
    {
    flags += " ";
    flags += stackVal;
    }
  // assume incremental linking
  std::string linkFlagVarBase = "CMAKE_";
  linkFlagVarBase += linkType;
  linkFlagVarBase += "_LINKER_FLAGS";
  flags += " ";
  flags += this->
    Target->GetMakefile()->GetRequiredDefinition(linkFlagVarBase.c_str());
  std::string linkFlagVar = linkFlagVarBase + "_" + CONFIG;
  flags += " ";
  flags += this->
    Target->GetMakefile()->GetRequiredDefinition(linkFlagVar.c_str());
  const char* targetLinkFlags = this->Target->GetProperty("LINK_FLAGS");
  if(targetLinkFlags)
    {
    flags += " ";
    flags += targetLinkFlags;
    }
  std::string flagsProp = "LINK_FLAGS_";
  flagsProp += CONFIG;
  if(const char* flagsConfig = this->Target->GetProperty(flagsProp.c_str()))
    {
    flags += " ";
    flags += flagsConfig;
    }
  cmVisualStudioGeneratorOptions
    linkOptions(this->LocalGenerator,
                cmVisualStudioGeneratorOptions::Linker,
                cmVS10LinkFlagTable, 0, this);
  if ( this->Target->GetPropertyAsBool("WIN32_EXECUTABLE") )
    {
    flags += " /SUBSYSTEM:WINDOWS";
    }
  else
    {
    flags += " /SUBSYSTEM:CONSOLE";
    }
  cmSystemTools::ReplaceString(flags, "/INCREMENTAL:YES", "");
  cmSystemTools::ReplaceString(flags, "/INCREMENTAL:NO", "");
  std::string standardLibsVar = "CMAKE_";
  standardLibsVar += linkLanguage;
  standardLibsVar += "_STANDARD_LIBRARIES";
  std::string 
    libs = this->Makefile->GetSafeDefinition(standardLibsVar.c_str());
  // Remove trailing spaces from libs
  std::string::size_type pos = libs.size()-1;
  if(libs.size() != 0)
    {
    while(libs[pos] == ' ')
      {
      pos--;
      }
    }
  if(pos != libs.size()-1)
    {
    libs = libs.substr(0, pos+1);
    }
  // Replace spaces in libs with ;
  cmSystemTools::ReplaceString(libs, " ", ";");
  cmComputeLinkInformation* pcli =
    this->Target->GetLinkInformation(config.c_str());
  if(!pcli)
    {
    cmSystemTools::Error
      ("CMake can not compute cmComputeLinkInformation for target:",
       this->Name.c_str());
    return;
    }
  // add the libraries for the target to libs string
  cmComputeLinkInformation& cli = *pcli;
  this->AddLibraries(cli, libs);
  linkOptions.AddFlag("AdditionalDependencies", libs.c_str());

  std::vector<std::string> const& ldirs = cli.GetDirectories();
  const char* sep = "";
  std::string linkDirs;
  for(std::vector<std::string>::const_iterator d = ldirs.begin();
      d != ldirs.end(); ++d)
    {
    // first just full path
    linkDirs += sep;
    linkDirs += *d;
    sep = ";";
    linkDirs += sep;
    // next path with configuration type Debug, Release, etc
    linkDirs += *d;
    linkDirs += "/$(Configuration)";
    linkDirs += sep;
    }
  linkDirs += "%(AdditionalLibraryDirectories)";
  linkOptions.AddFlag("AdditionalLibraryDirectories", linkDirs.c_str());
  linkOptions.AddFlag("AdditionalDependencies", libs.c_str());
  linkOptions.AddFlag("Version", "");
  if(linkOptions.IsDebug() || flags.find("/debug") != flags.npos)
    {
    linkOptions.AddFlag("GenerateDebugInformation", "true");
    }
  else
    {
    linkOptions.AddFlag("GenerateDebugInformation", "false");
    } 
  std::string targetName;
  std::string targetNameSO;
  std::string targetNameFull;
  std::string targetNameImport;
  std::string targetNamePDB;
  if(this->Target->GetType() == cmTarget::EXECUTABLE)
    {
    this->Target->GetExecutableNames(targetName, targetNameFull,
                                     targetNameImport, targetNamePDB, 
                                     config.c_str());
    }
  else
    {
    this->Target->GetLibraryNames(targetName, targetNameSO, targetNameFull,
                                  targetNameImport, targetNamePDB, 
                                  config.c_str());
    }

  std::string dir = this->Target->GetDirectory(config.c_str());
  dir += "/";
  std::string pdb = dir;
  pdb += targetNamePDB;
  std::string imLib = this->Target->GetDirectory(config.c_str(), true);
  imLib += "/";
  imLib += targetNameImport;

  linkOptions.AddFlag("ImportLibrary", imLib.c_str());
  linkOptions.AddFlag("ProgramDataBaseFileName", pdb.c_str());
  linkOptions.Parse(flags.c_str());
  if(!this->ModuleDefinitionFile.empty())
    {
    linkOptions.AddFlag("ModuleDefinitionFile",
                        this->ModuleDefinitionFile.c_str());
    }

  linkOptions.RemoveFlag("GenerateManifest");
  linkOptions.OutputAdditionalOptions(*this->BuildFileStream, "      ", "");
  linkOptions.OutputFlagMap(*this->BuildFileStream, "      ");
  
  this->WriteString("</Link>\n", 2);
  if(!this->GlobalGenerator->NeedLinkLibraryDependencies(*this->Target))
    {
    this->WriteString("<ProjectReference>\n", 2);
    this->WriteString(
      "  <LinkLibraryDependencies>false</LinkLibraryDependencies>\n", 2);
    this->WriteString("</ProjectReference>\n", 2);
    }
}

void cmVisualStudio10TargetGenerator::AddLibraries(
  cmComputeLinkInformation& cli,
  std::string& libstring)
{ 
  typedef cmComputeLinkInformation::ItemVector ItemVector;
  ItemVector libs = cli.GetItems();
  const char* sep = ";";
  for(ItemVector::const_iterator l = libs.begin(); l != libs.end(); ++l)
    {
    if(l->IsPath)
      {
      std::string path = this->LocalGenerator->
        Convert(l->Value.c_str(),
                cmLocalGenerator::START_OUTPUT,
                cmLocalGenerator::UNCHANGED);
      this->ConvertToWindowsSlash(path);
      libstring += sep;
      libstring += path;
      }
    else
      {
      libstring += sep;
      libstring += l->Value;
      }
    }
}


void cmVisualStudio10TargetGenerator::
WriteMidlOptions(std::string const& /*config*/,
                 std::vector<std::string> const & includes)
{
  this->WriteString("<Midl>\n", 2);
  this->OutputIncludes(includes);
  this->WriteString("<OutputDirectory>$(IntDir)</OutputDirectory>\n", 3);
  this->WriteString("<HeaderFileName>%(Filename).h</HeaderFileName>\n", 3);
  this->WriteString(
    "<TypeLibraryName>%(Filename).tlb</TypeLibraryName>\n", 3);
  this->WriteString(
    "<InterfaceIdentifierFileName>"
    "%(Filename)_i.c</InterfaceIdentifierFileName>\n", 3);
  this->WriteString("<ProxyFileName>%(Filename)_p.c</ProxyFileName>\n",3);
  this->WriteString("</Midl>\n", 2);
}


void cmVisualStudio10TargetGenerator::WriteItemDefinitionGroups()
{
  std::vector<std::string> *configs =
    static_cast<cmGlobalVisualStudio7Generator *>
    (this->GlobalGenerator)->GetConfigurations();
  std::vector<std::string> includes;
  this->LocalGenerator->GetIncludeDirectories(includes);
  for(std::vector<std::string>::iterator i = configs->begin();
      i != configs->end(); ++i)
    {
    this->WritePlatformConfigTag("ItemDefinitionGroup", i->c_str(), 1);
    *this->BuildFileStream << "\n";
    //    output cl compile flags <ClCompile></ClCompile>
    if(this->Target->GetType() <= cmTarget::MODULE_LIBRARY)
      {
      this->WriteClOptions(*i, includes);
      //    output rc compile flags <ResourceCompile></ResourceCompile>
      this->WriteRCOptions(*i, includes);
      }
    //    output midl flags       <Midl></Midl>
    this->WriteMidlOptions(*i, includes);
    // write events
    this->WriteEvents(*i);
    //    output link flags       <Link></Link> 
    this->WriteLinkOptions(*i);
    //    output lib flags       <Lib></Lib> 
    this->WriteLibOptions(*i);
    this->WriteString("</ItemDefinitionGroup>\n", 1);
    }
}

void 
cmVisualStudio10TargetGenerator::WriteEvents(std::string const& configName)
{
  this->WriteEvent("PreLinkEvent",
                   this->Target->GetPreLinkCommands(), configName);
  this->WriteEvent("PreBuildEvent",
                   this->Target->GetPreBuildCommands(), configName);
  this->WriteEvent("PostBuildEvent",
                   this->Target->GetPostBuildCommands(), configName);
}

void cmVisualStudio10TargetGenerator::WriteEvent(
  const char* name,
  std::vector<cmCustomCommand> & commands,
  std::string const& configName)
{
  if(commands.size() == 0)
    {
    return;
    }
  this->WriteString("<", 2);
  (*this->BuildFileStream ) << name << ">\n";
  cmLocalVisualStudio7Generator* lg = this->LocalGenerator;
  std::string script;
  const char* pre = "";
  std::string comment;
  for(std::vector<cmCustomCommand>::iterator i = commands.begin();
      i != commands.end(); ++i)
    {  
    cmCustomCommand& command = *i;
    comment += pre;
    comment += lg->ConstructComment(command);
    script += pre;
    pre = "\n";
    script +=
      cmVS10EscapeXML(lg->ConstructScript(command, configName.c_str()));
    }
  comment = cmVS10EscapeComment(comment);
  this->WriteString("<Message>",3);
  (*this->BuildFileStream ) << cmVS10EscapeXML(comment) << "</Message>\n";
  this->WriteString("<Command>", 3);
  (*this->BuildFileStream ) << script;
  (*this->BuildFileStream ) << "</Command>" << "\n";
  this->WriteString("</", 2);
  (*this->BuildFileStream ) << name << ">\n";
}


void cmVisualStudio10TargetGenerator::WriteProjectReferences()
{
  cmGlobalGenerator::TargetDependSet const& unordered
    = this->GlobalGenerator->GetTargetDirectDepends(*this->Target);
  typedef cmGlobalVisualStudioGenerator::OrderedTargetDependSet
    OrderedTargetDependSet;
  OrderedTargetDependSet depends(unordered);
  this->WriteString("<ItemGroup>\n", 1);
  for( OrderedTargetDependSet::const_iterator i = depends.begin();
       i != depends.end(); ++i)
    {
    cmTarget* dt = *i;
    // skip fortran targets as they can not be processed by MSBuild
    // the only reference will be in the .sln file
    if(static_cast<cmGlobalVisualStudioGenerator*>(this->GlobalGenerator)
       ->TargetIsFortranOnly(*dt))
      {
      continue;
      }
    this->WriteString("<ProjectReference Include=\"", 2);
    cmMakefile* mf = dt->GetMakefile();
    std::string name = dt->GetName();
    std::string path;
    const char* p = dt->GetProperty("EXTERNAL_MSPROJECT");
    if(p)
      {
      path = p;
      }
    else
      {
      path =  mf->GetStartOutputDirectory();
      path += "/";
      path += dt->GetName();
      path += ".vcxproj";
      }
    (*this->BuildFileStream) << path << "\">\n";
    this->WriteString("<Project>", 3);
    (*this->BuildFileStream) 
      << this->GlobalGenerator->GetGUID(name.c_str())
      << "</Project>\n";
    this->WriteString("</ProjectReference>\n", 2);
    }
  this->WriteString("</ItemGroup>\n", 1);
}
