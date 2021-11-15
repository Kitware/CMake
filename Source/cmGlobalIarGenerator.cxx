/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2004-2009 Kitware, Inc.
  Copyright 2015 Rockwell Automation Technologies, Inc.
  Copyright 2015 Jakub Korbel (jkorbel@ra.rockwell.com).

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include <iostream>
#include <cstdlib>
#include <cassert>
#include <algorithm>

#include "cmake.h"

#include "cmGlobalIarGenerator.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmLocalIarGenerator.h"

#include "cmTimestamp.h"
#include "cmMakefile.h"
#include "cmGeneratedFileStream.h"
#include "cmTarget.h"
#include "cmGeneratorTarget.h"
#include "cmSourceFile.h"
#include "cmSystemTools.h"
#include "cmCustomCommand.h"
#include "cmLinkLineComputer.h"

#include "cmsys/Glob.hxx"


/// @brief XML Declaration.
const char* cmGlobalIarGenerator::XML_DECL =
    "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n";

const char* cmGlobalIarGenerator::PROJ_FILE_EXT = ".ewp";
const char* cmGlobalIarGenerator::WS_FILE_EXT = ".eww";
const char* cmGlobalIarGenerator::DEFAULT_MAKE_PROGRAM = "IarBuild.exe";

const char* cmGlobalIarGenerator::MULTIOPTS_COMPILER[13] = {
  "--dependencies",
                                     "--diagnostics_tables",
                                     "--dlib_config",
                                     "-f",
                                     "-l",
                                     "--output",
                                     "-o",
                                     "--predef_macros",
                                     "--preinclude",
                                     "--preprocess",
                                     "--public_equ",
                                     "--section",
                                     "--system_include_dir" };
const char* cmGlobalIarGenerator::MULTIOPTS_LINKER[24] = {
  "--call_graph",
                                "--config",
                                "--config_def",
                                "--config_search",
                                "--cpp_init_routine",
                                "--define_symbol",
                                "--dependencies",
                                "--diagnostics_tables",
                                "--entry",
                                "--export_builtin_config",
                                "--extra_init",
                                "-f",
                                "--image_input",
                                "--keep",
                                "--log",
                                "--log_file",
                                "--map",
                                "--output",
                                "-o",
                                "--place_holder",
                                "--redirect",
                                "--search",
                                "--stack_usage_control",
                                "--whole_archive" };

/// @brief Global configuration of the project (it should be visible
/// from everywhere).
cmGlobalIarGenerator::GlobalCmakeCfg cmGlobalIarGenerator::GLOBALCFG =
    cmGlobalIarGenerator::GlobalCmakeCfg();

//------------------------------------------------------------------------------
///
/// @brief Converts integer to decimal string notation.
///
/// @param[in] val Value to convert.
///
/// @return Converted string.
///
//------------------------------------------------------------------------------
static inline std::string int2str(int val)
{
  std::stringstream tmp;
  tmp << val;
  return tmp.str();
}


//------------------------------------------------------------------------------
///
/// @brief Generic XML node.
///
/// This node can have multiple children and attributes, or a plain text
/// value.
///
//------------------------------------------------------------------------------
class XmlNode
{
  /// @brief Node name (represented later as tag name).
  std::string nodeName;

  /// @brief Plain text value.
  std::string plainValue;

  /// @brief Attribute pairs (attr="val").
  std::vector< std::pair< std::string, std::string > > attrs;

  /// @brief Children vector.
  std::vector<XmlNode*> children;

  /// @brief Simple printing helper table for tabs up to 11 levels.
  static const char* LEVELS[];

public:
  XmlNode(std::string name, std::string value) :
  nodeName(name), plainValue(value)
{
  // Just copy values.
}

  XmlNode(std::string name) : nodeName(name), plainValue("")
  {
    // Just copy values.
  }

  /// This function creates a new child node (dynamic memory allocation).
  XmlNode* NewChild(std::string name, std::string value)
  {
    XmlNode* child = new XmlNode(name, value);
    children.push_back(child);
    return child;
  }

  /// This function creates a new child node (dynamic memory allocation).
  XmlNode* NewChild(std::string name)
  {
    XmlNode* child = new XmlNode(name);
    children.push_back(child);
    return child;
  }

  /// This function adds a child node (no memory allocation).
  ///
  /// @warning delete will be issued for this node!
  XmlNode* AddChild(XmlNode* child)
  {
    children.push_back(child);
    return this;
  }

  void AddAttr(std::string name, std::string value)
  {
    attrs.push_back(std::make_pair(name, value));
  }

  std::string& AppendOpenTag(std::string& tag, unsigned int level) const
  {
    tag += std::string(LEVELS[level]) + "<" + nodeName;

    for(std::vector< std::pair<std::string, std::string> >::const_iterator it
        = attrs.begin();
        it != attrs.end();
        ++it)
      {
      tag += std::string(" ") + it->first + "=\"" + it->second + "\"";
      }
    tag += ">";
    return tag;
  }

  std::string& AppendCloseTag(std::string& tag, unsigned int level) const
  {
    (void) level; // May come handy, when open and close are on a separate
    // line.

    tag += std::string("</") + nodeName + ">\n";
    return tag;
  }

  void ToString(unsigned int level, std::string& outStr) const
  {
    this->AppendOpenTag(outStr, level);

    bool moreThanOnce = false;

    for(std::vector<XmlNode*>::const_iterator it = this->children.begin();
        it != this->children.end();
        ++it)
      {
      if (!moreThanOnce)
        {
        moreThanOnce = true;
        outStr += "\n";
        }

      (*it)->ToString(level+1, outStr);
      }

    if (!moreThanOnce)
      {
      if (this->plainValue.empty())
        {
        outStr[outStr.length()-1] = '/';
        outStr += ">\n";
        // Do not append close tag.
        }
      else
        {
        outStr += this->plainValue;
        this->AppendCloseTag(outStr, level);
        }
      }
    else
      {
      outStr += LEVELS[level];
      this->AppendCloseTag(outStr, level);
      }
  }

  virtual ~XmlNode()
  {
    for(std::vector<XmlNode*>::iterator it = this->children.begin();
        it != this->children.end();
        ++it)
      {
      delete *it;
      }
  }
};

const char* XmlNode::LEVELS[] =
    {
        "",
        "\t",
        "\t\t",
        "\t\t\t",
        "\t\t\t\t",
        "\t\t\t\t\t",
        "\t\t\t\t\t\t",
        "\t\t\t\t\t\t\t",
        "\t\t\t\t\t\t\t\t",
        "\t\t\t\t\t\t\t\t\t",
        "\t\t\t\t\t\t\t\t\t\t"
    };

const char* RUNTIME_LIBRARY_CONFIG[] =
    {
        "None"  , // 0
        "Normal", // 1
        "Full"  , // 2
        "Custom"  // 3
    };


const char* SCANF_PRINTF_FORMATTING[] =
    {
        "Auto"  , // 0
        "Full", // 1
        "Full without multibytes"  , // 2
        "Large", // 3
        "Large without multibytes"  , // 4
        "Small", // 5
        "Small without multibytes"  , // 6
        "Tiny"  // 7
    };

const int SCANF_FORMATTING_CNT = 7;
const int PRINTF_FORMATTING_CNT = 8;


class FileTreeNode
{
public:
  std::string ftNodeName;

  std::vector<FileTreeNode*> children;

  FileTreeNode(std::string name) : ftNodeName(name)
  {

  }

  void TransformToIarTree(XmlNode* root)
  {
    if (!this->children.empty())
      {
      XmlNode* group = root->NewChild("group");
      group->NewChild("name", this->ftNodeName);

      for (std::vector<FileTreeNode*>::const_iterator it =
          this->children.begin();
          it != this->children.end();
          ++it)
        {
        (*it)->TransformToIarTree(group);
        }

      }
    else
      {
      XmlNode* file = root->NewChild("file");
      file->NewChild("name", this->ftNodeName);
      }
  }

  FileTreeNode* NewNode(std::string name)
  {
    FileTreeNode* node = new FileTreeNode(name);
    children.push_back(node);
    return node;
  }

  static void AddToTree(FileTreeNode* root, std::string path,
      const std::string& fullpath)
  {
    size_t slashPos = path.find_first_of("/\\");
    std::string currentChunk;
    std::string restOfPath;
    if (slashPos != std::string::npos)
      {
      currentChunk = path.substr(0, slashPos);
      restOfPath = path.substr(slashPos+1);
      }
    else
      {
      currentChunk = path;
      restOfPath = "";
      }

    if (!currentChunk.empty())
      {
      bool found = false;
      for (std::vector<FileTreeNode*>::const_iterator it =
          root->children.begin();
          it != root->children.end();
          ++it)
        {
        if ((*it)->ftNodeName == currentChunk)
          {
          // Found, move in tree.
          AddToTree(*it, restOfPath, fullpath);
          found = true;
          break;
          }
        }

      if (!found)
        {
        if (restOfPath.empty())
          {
          // Not found, create new and finish.
          root->NewNode(fullpath);
          return;
          }
        else
          {
          // Not found, create new and move inside.
          FileTreeNode* newNode = root->NewNode(currentChunk);
          AddToTree(newNode, restOfPath, fullpath);
          }
        }
      }
  }

  virtual ~FileTreeNode()
  {
    for(std::vector<FileTreeNode*>::iterator it = children.begin();
        it != children.end();
        ++it)
      {
      delete *it;
      }
  }
};

class IarOption : public XmlNode
{
private:
  int optVersion;
  std::string optName;
public:
  IarOption(std::string name, int version) : XmlNode("option"),
  optVersion(version), optName(name)
{
  if (!optName.empty())
    {
    this->NewChild("name", optName);
    }

  if (optVersion >= 0)
    {
    this->NewChild("version", int2str(optVersion));
    }
}

  IarOption(std::string name) : XmlNode("option"),
      optVersion(-1), optName(name)
  {
    if (!optName.empty())
      {
      this->NewChild("name", optName);
      }
  }

  void NewState(std::string state)
  {
    this->NewChild("state", state);
  }

  void NewStates(std::vector<std::string> states)
  {
    for (std::vector<std::string>::const_iterator it = states.begin();
        it != states.end();
        ++it)
      {
      this->NewChild("state", *it);
      }
  }
};

class IarDebuggerPlugin : public XmlNode
{
public:
  IarDebuggerPlugin(std::string file, bool load) : XmlNode("plugin")
{
  if (!file.empty())
    {
    this->NewChild("file", file);
    this->NewChild("loadFlag", load ? "1" : "0");
    }
}
};


class IarData : public XmlNode
{
private:
  int dataVersion;
  bool dataWantNonLocal;
  bool dataDebug;
public:
  IarData(int version, bool wantNonLocal, bool debug) :
  XmlNode("data"),
  dataVersion(version),
  dataWantNonLocal(wantNonLocal),
  dataDebug(debug)
{
  this->NewChild("version", int2str(dataVersion));
  this->NewChild("wantNonLocal", dataWantNonLocal ? "1" : "0");
  this->NewChild("debug", dataDebug ? "1" : "0");
}

  IarOption* NewOption(std::string name, int version)
  {
    IarOption* option = new IarOption(name, version);
    this->AddChild(option);
    return option;
  }

  IarOption* NewOption(std::string name)
  {
    IarOption* option = new IarOption(name);
    this->AddChild(option);
    return option;
  }
};


class IarSettings : public XmlNode
{
private:
  std::string settingsName;
  int archiveVersion;

public:
  IarSettings(std::string name,
      int version) :
      XmlNode("settings"),
      settingsName(name),
      archiveVersion(version)
{
  if (!settingsName.empty())
    {
    this->NewChild("name", settingsName);
    }

  this->NewChild("archiveVersion", int2str(archiveVersion));
}

  IarData* NewData(int version, bool wantNonLocal, bool debug)
  {
    IarData* data = new IarData(version, wantNonLocal, debug);
    this->AddChild(data);
    return data;
  }
};


class IarFsNode : public XmlNode
{
private:
  std::string fsPath;
  bool fsIsDir;

  std::string GetLastDir(std::string path)
  {
    size_t position = path.find_last_of("/\\");
    if (position != std::string::npos)
      {
      return path.substr(position+1);
      }
    return std::string("");
  }

public:
  IarFsNode(std::string path,
      bool isDir) :
      XmlNode(isDir ? "group" : "file"),
      fsPath(path),
      fsIsDir(isDir)
{
  this->NewChild("name", isDir ? GetLastDir(fsPath) : fsPath);

}

  IarData* NewData(int version, bool wantNonLocal, bool debug)
  {
    IarData* data = new IarData(version, wantNonLocal, debug);
    this->AddChild(data);
    return data;
  }
};


//----------------------------------------------------------------------------
cmGlobalIarGenerator::cmGlobalIarGenerator(cmake* cm)
: cmGlobalGenerator(cm)
{
    cm->GetState()->SetIarIDE(true);
}

cmGlobalIarGenerator::~cmGlobalIarGenerator()
{
}

std::unique_ptr<cmLocalGenerator> cmGlobalIarGenerator::CreateLocalGenerator(
  cmMakefile* mf)
{
  return std::unique_ptr<cmLocalGenerator>(
    cm::make_unique<cmLocalIarGenerator>(this, mf));
}


void cmGlobalIarGenerator::GetDocumentation(cmDocumentationEntry& entry)
{
  entry.Name = GetActualName();
  entry.Brief =
    "Generates IAR Embedded Workbench files (experimental, work-in-progress).";
}


void cmGlobalIarGenerator::EnableLanguage(
  std::vector<std::string> const& l, cmMakefile* mf, bool optional)
{
    // Get global config from IAR_* variables from toolchain and CMakeLists.txt.
    if (GLOBALCFG.iarArmPath.empty())
    {
        // Load the settings only once.
        GLOBALCFG.iarArmPath = mf->GetSafeDefinition("IAR_ARM_PATH");
    }

    this->cmGlobalGenerator::EnableLanguage(l, mf, optional);
}

std::string cmGlobalIarGenerator::FindIarBuildCommand()
{
    std::string commonBin = GLOBALCFG.iarArmPath + "/../common/bin";
    std::vector<std::string> userPaths;
    userPaths.push_back(commonBin);

    // TODO COMMENT
    // cmSystemTools::Message(std::string("USER PATH: ") + commonBin);
    // END TODO

    std::string makeProgram =
      cmSystemTools::FindProgram(DEFAULT_MAKE_PROGRAM, userPaths);
    if (makeProgram.empty()) {
      makeProgram = commonBin + "/" + DEFAULT_MAKE_PROGRAM;
    }

    return makeProgram;
}


bool cmGlobalIarGenerator::FindMakeProgram(cmMakefile* mf)
{
  // The GHS generator knows how to lookup its build tool
  // directly instead of needing a helper module to do it, so we
  // do not actually need to put CMAKE_MAKE_PROGRAM into the cache.
  if (cmIsOff(mf->GetDefinition("CMAKE_MAKE_PROGRAM"))) {
    mf->AddDefinition("CMAKE_MAKE_PROGRAM",
                      this->FindIarBuildCommand().c_str());
  }

  return true;
}


std::vector<cmGlobalGenerator::GeneratedMakeCommand>
cmGlobalIarGenerator::GenerateBuildCommand(
  const std::string& makeProgram, const std::string& projectName,
  const std::string& projectDir, std::vector<std::string> const& targetNames,
  const std::string& config, bool /*fast*/, int jobs, bool /*verbose*/,
  std::vector<std::string> const& makeOptions)
{
  cmGlobalGenerator::GeneratedMakeCommand makeCommand = {};

  makeCommand.Add(
    this->SelectMakeProgram(makeProgram, this->FindIarBuildCommand()));

  makeCommand.Add(makeOptions.begin(), makeOptions.end());

  if (!targetNames.empty()) {
    if (std::find(targetNames.begin(), targetNames.end(), "clean") !=
        targetNames.end()) {
      makeCommand.Add("-clean");
    } else {
      for (const auto& tname : targetNames) {
        if (!tname.empty()) {
          makeCommand.Add(tname + ".ewp");
        }
      }
    }
  }

  makeCommand.Add("-build");
  /*auto majorVer = GLOBALCFG.wbVersion.substr(
    0, GLOBALCFG.wbVersion.find('.'));
  auto majorVerInt = atoi(majorVer.c_str());
  if (majorVerInt < 8)
  {*/
  std::string buildType = GLOBALCFG.buildType;
  if (GLOBALCFG.buildType.empty()) {
    buildType = "Release";
  }

  makeCommand.Add(buildType);
  /*}*/

  // TODO COMMENT
  // cmSystemTools::Message(makeCommand.Printable());
  // END TODO

  return { makeCommand };
}


//----------------------------------------------------------------------------
void cmGlobalIarGenerator::Generate()
{
    // TODO COMMENT
    // cmSystemTools::Message(std::string("Generation has started..."));
    // END TODO

  const cmLocalGenerator* const lgs0 =
      this->GetLocalGenerators()[0].get();
  const cmMakefile* globalMakefile = lgs0->GetMakefile();

  GLOBALCFG.buildType = globalMakefile->GetSafeDefinition("CMAKE_BUILD_TYPE");
  std::string flagsWithType = std::string("CMAKE_C_FLAGS_") + cmSystemTools::UpperCase(GLOBALCFG.buildType);

  GLOBALCFG.iarCCompilerFlags = globalMakefile->GetSafeDefinition("CMAKE_C_FLAGS");
  GLOBALCFG.iarCCompilerFlags += std::string(" ") + globalMakefile->GetSafeDefinition(flagsWithType);

  flagsWithType = std::string("CMAKE_ASM_IAR_FLAGS_" +
                              cmSystemTools::UpperCase(GLOBALCFG.buildType));
  GLOBALCFG.iarAsmFlags =
    globalMakefile->GetSafeDefinition("CMAKE_ASM_IAR_FLAGS");
  GLOBALCFG.iarAsmFlags +=
    std::string(" ") + globalMakefile->GetSafeDefinition(flagsWithType);

  flagsWithType = std::string("CMAKE_CXX_FLAGS_"+cmSystemTools::UpperCase(GLOBALCFG.buildType));
  GLOBALCFG.iarCxxCompilerFlags = globalMakefile->GetSafeDefinition("CMAKE_CXX_FLAGS");
  GLOBALCFG.iarCxxCompilerFlags += std::string(" ") + globalMakefile->GetSafeDefinition(flagsWithType);

  flagsWithType = std::string("CMAKE_EXE_LINKER_FLAGS_") + cmSystemTools::UpperCase(GLOBALCFG.buildType);
  GLOBALCFG.iarLinkerFlags = globalMakefile->GetSafeDefinition("CMAKE_EXE_LINKER_FLAGS");
  GLOBALCFG.iarLinkerFlags += std::string(" ") + globalMakefile->GetSafeDefinition(flagsWithType);

  GLOBALCFG.compilerDlibConfig =
      globalMakefile->GetSafeDefinition("IAR_COMPILER_DLIB_CONFIG");

  for (int i = 0; i < 4; i++)
  {
      if (std::string(RUNTIME_LIBRARY_CONFIG[i]) == GLOBALCFG.compilerDlibConfig)
      {
          GLOBALCFG.compilerDlibConfigId = i;
      }
  }

  GLOBALCFG.bufferedTermOut = globalMakefile->GetSafeDefinition("IAR_GENERAL_BUFFERED_TERMINAL_OUTPUT");
  GLOBALCFG.scanfFmt = globalMakefile->GetSafeDefinition("IAR_GENERAL_SCANF_FORMATTER");
  GLOBALCFG.printfFmt = globalMakefile->GetSafeDefinition("IAR_GENERAL_PRINTF_FORMATTER");
  GLOBALCFG.semihostingEnabled = globalMakefile->GetSafeDefinition("IAR_SEMIHOSTING_ENABLE");

  for (int i = 0; i < SCANF_FORMATTING_CNT; i++)
  {
      if (std::string(SCANF_PRINTF_FORMATTING[i]) == GLOBALCFG.scanfFmt)
      {
          GLOBALCFG.scanfFmtId = i;
      }
  }

  for (int i = 0; i < PRINTF_FORMATTING_CNT; i++)
  {
      if (std::string(SCANF_PRINTF_FORMATTING[i]) == GLOBALCFG.printfFmt)
      {
          GLOBALCFG.printfFmtId = i;
      }
  }

  GLOBALCFG.compilerPathExe =
      globalMakefile->GetSafeDefinition("IAR_COMPILER_PATH_EXE");
  GLOBALCFG.cpuName = globalMakefile->GetSafeDefinition("IAR_CPU_NAME");
  GLOBALCFG.systemName =
      globalMakefile->GetSafeDefinition("CMAKE_SYSTEM_NAME");
  GLOBALCFG.dbgExtraOptions =
      globalMakefile->GetSafeDefinition("IAR_DEBUGGER_CSPY_EXTRAOPTIONS");
  GLOBALCFG.dbgCspyFlashLoaderv3 =
      globalMakefile->GetSafeDefinition("IAR_DEBUGGER_CSPY_FLASHLOADER_V3");
  GLOBALCFG.dbgCspyMacfile =
      globalMakefile->GetSafeDefinition("IAR_DEBUGGER_CSPY_MACFILE");
  GLOBALCFG.dbgCspyMemfile =
      globalMakefile->GetSafeDefinition("IAR_DEBUGGER_CSPY_MEMFILE");
  GLOBALCFG.dbgIjetProbeconfig =
      globalMakefile->GetSafeDefinition("IAR_DEBUGGER_IJET_PROBECONFIG");
  GLOBALCFG.dbgProbeSelection =
      globalMakefile->GetSafeDefinition("IAR_DEBUGGER_PROBE");
  GLOBALCFG.dbgLogFile =
      globalMakefile->GetSafeDefinition("IAR_DEBUGGER_LOGFILE");
  GLOBALCFG.linkerEntryRoutine =
      globalMakefile->GetSafeDefinition("IAR_LINKER_ENTRY_ROUTINE");
  GLOBALCFG.linkerIcfFile =
      globalMakefile->GetSafeDefinition("IAR_LINKER_ICF_FILE");
  GLOBALCFG.tgtArch =
      globalMakefile->GetSafeDefinition("IAR_TARGET_ARCHITECTURE");

  GLOBALCFG.rtos = globalMakefile->GetSafeDefinition("IAR_TARGET_RTOS");

  // Pre-include support is not included in regular cmake (--include header.h)
  // todo after / if it is supported, remove this code for the sake of
  // standard variables.
  std::string preInclude =
      globalMakefile->GetSafeDefinition("IAR_COMPILER_PREINCLUDE");
  GLOBALCFG.compilerPreInclude = preInclude;

  // todo We are supporting only arm (different linker/compiler options must
  // be used for different IAR version).
  GLOBALCFG.wbVersion =
      globalMakefile->GetSafeDefinition("IAR_WORKBENCH_VERSION");
  //if (GLOBALCFG.wbVersion.empty())
  //{
  GLOBALCFG.tgtArch = "ARM";
  //}

  // todo Add list of IAR variables, which the user is able to set.
  GLOBALCFG.chipSelection =
      globalMakefile->GetSafeDefinition("IAR_CHIP_SELECTION");
  if (GLOBALCFG.chipSelection.empty())
    {
    GLOBALCFG.chipSelection = "None";
    }


  // IAR needs a workspace name. This would be the root CMake project.
  for (std::map<std::string, std::vector<cmLocalGenerator*> >::const_iterator
      it = this->GetProjectMap().begin();
      it!= this->GetProjectMap().end();
      ++it)
    {
    const cmMakefile* makeFile = it->second[0]->GetMakefile();

    // create a project file
    if (strcmp(makeFile->GetCurrentBinaryDirectory().c_str(),
        makeFile->GetHomeOutputDirectory().c_str()) == 0)
      {
      this->workspace.workspaceDir = makeFile->GetCurrentBinaryDirectory();
      this->workspace.name = lgs0->GetProjectName();
      }
    }

  this->cmGlobalGenerator::Generate();

  // Finally, create IAR workspace file containing a list of all IAR projects.
  this->workspace.CreateWorkspaceFile();
}

//----------------------------------------------------------------------------
std::string cmGlobalIarGenerator::ToToolkitPath(std::string absolutePath)
{
  if(!GLOBALCFG.iarArmPath.empty())
    {
    if (absolutePath.find(GLOBALCFG.iarArmPath) != std::string::npos)
      {
      std::string outStr = "$TOOLKIT_DIR$/";
      outStr += absolutePath.substr(GLOBALCFG.iarArmPath.length()+1);
      std::replace( outStr.begin(), outStr.end(), '/', '\\');
      return outStr;
      }
    }

  return absolutePath;
}

//----------------------------------------------------------------------------
std::string cmGlobalIarGenerator::ToWorkbenchPath(std::string absolutePath)
{
  if(!GLOBALCFG.iarArmPath.empty())
    {
    std::string ewPath = GLOBALCFG.iarArmPath.substr(0,
        GLOBALCFG.iarArmPath.length()-(sizeof("/arm")-1));
    if (absolutePath.find(ewPath) != std::string::npos)
      {
      std::string outStr = "$EW_DIR$/";
      outStr += absolutePath.substr(ewPath.length()+1);
      std::replace( outStr.begin(), outStr.end(), '/', '\\');
      return outStr;
      }
    }

  return absolutePath;
}

namespace IarArg
{
  const unsigned int KEY_LEN = 256;
  const unsigned int VAL_LEN = 256;

  enum ArgState
    {
      ARG_STATE_EXP_ARG,
      ARG_STATE_EXP_ARG_NAME,
      ARG_STATE_EXP_ARG_VALUE
    };

  const char* ParseValue(const char* pChar, char* pVal)
  {
    const char* pBegin = pChar;
    // Value.
    bool breakMe = false;
    bool parseError = false;
    char escape = 0;
    while (!breakMe)
      {
      switch(pChar[0])
      {
      case '"':
        if (pChar == pBegin && escape == '\0')
          {
          escape = '"';
          pBegin++;
          }
        else
          {
          parseError = breakMe = !(escape == '"' && (pChar[1] == ' ' || pChar[1] == '\0'));
          }

        break;

      case '\'':
        if (escape == '\0')
          {
          escape = '\'';
          }
        else
          {
          parseError = breakMe = !(escape == '\'' && (pChar[1] == ' ' || pChar[1] == '\0'));
          }

        break;

      case ' ':
        if (escape == '\0')
          {
          // Break if escape not in effect.
          breakMe = true;
          }
        break;
      case '\0':
        breakMe = true;
        break;
      }

      if (breakMe)
        {
        break;
        }

      // Move the char pointer.
      pChar++;
      }

    size_t valBytes = (pChar - pBegin - (escape != '\0'));
    if (!parseError && (valBytes + 1 < 256)) // 1 per \0.
      {
      memcpy((void*)pVal, (void*)pBegin, valBytes);
      pVal[valBytes] = '\0';
      }

    return (!parseError) ? pChar : NULL;
  }

  const char* ParseKey(const char* pChar, char* pKey)
  {
    const char* pBegin = pChar;
    // Key.
    bool breakMe = false;
    bool parseError = false;
    char escape = 0;
    while (!breakMe)
      {
      switch(pChar[0])
      {
      case '"':
        if (pChar == pBegin && escape == '\0')
          {
          escape = '"';
          pBegin++;
          }
        else
          {
          parseError = breakMe = !(escape == '"' && (pChar[1] == ' ' || pChar[1] == '\0'));
          }

        break;

      case '\'':
        if (escape == '\0')
          {
          escape = '\'';
          }
        else
          {
          parseError = breakMe = !(escape == '\'' && (pChar[1] == ' ' || pChar[1] == '\0'));
          }

        break;

      case '\\':
        parseError = breakMe = true;
        break;

      case ' ':
        if (escape == '\0')
          {
          // Break if escape not in effect.
          breakMe = true;
          }
        break;
      case '\0':
        breakMe = true;
        break;
      default:
        // Check the character:
        parseError = breakMe =  !((pChar[0] >= 'A' && pChar[0] <= 'Z') ||
            (pChar[0] >= 'a' && pChar[0] <= 'z') ||
            (pChar[0] >= '0' && pChar[0] <= '9') ||
            pChar[0] == '+' || pChar[0] == '-');
        break;
      }

      if (breakMe)
        {
        break;
        }

      // Move the char pointer.
      pChar++;
      }

    size_t valBytes = (pChar - pBegin - (escape != '\0'));
    if (!parseError && (valBytes + 1 < 256)) // 1 per \0.
      {
        memcpy((void*)pKey, (void*)pBegin, valBytes);
        pKey[valBytes] = '\0';
      }

    return (!parseError) ? pChar : NULL;
  }

  const char* ParseNext(const char* cmdChar, char* pKey, char* pValue)
  {
    bool breakMe = false;
    bool parseError = false;
    bool isGnuStyle = false; // gnu style: --attr{= }val | posix style: -attr val
    ArgState state = ARG_STATE_EXP_ARG;

    while (!breakMe)
      {
      switch(cmdChar[0])
      {
      case '\0':
        parseError = breakMe = true;
        break;
      case '-':
        if (state == ARG_STATE_EXP_ARG)
          {
          // Look ahead.
          if (cmdChar[1] == '-')
            {
            isGnuStyle = true;
            cmdChar++;
            }
          state = ARG_STATE_EXP_ARG_NAME;
          }
        else if (state == ARG_STATE_EXP_ARG_VALUE)
          {
          // If expecting value, but param happens, we have found a flag
          // without value. End now.
          breakMe = true;
          *pValue = '\0';
          }
        else
          {
          // Disallow ---.
          parseError = breakMe = true;
          }
        break;
      case ' ':
        if (state == ARG_STATE_EXP_ARG_NAME)
          {
          // This is an error.
          }
        break;
      case '=':
        break;
      default:
        // Any char.
        if (state == ARG_STATE_EXP_ARG_NAME)
          {
          // Parse name:
          cmdChar = ParseKey(cmdChar, pKey);
          if (cmdChar != NULL)
            {
            state = ARG_STATE_EXP_ARG_VALUE;
            }
          else
            {
            parseError = breakMe = true;
            }

          }
        else if (state == ARG_STATE_EXP_ARG_VALUE)
          {
          breakMe = true;
          cmdChar = ParseValue(cmdChar, pValue);
          parseError = (cmdChar == NULL);
          }
        break;
      }

      if (breakMe)
        {
        break;
        }

      cmdChar++;
      }

    return (!parseError) ? cmdChar : NULL;
  }
};

//----------------------------------------------------------------------------
void cmGlobalIarGenerator::ParseCmdLineOpts(
  std::string cmdLine, const char* multiOpts[], size_t multiOptsLen,
  std::vector<std::string>& opts)
{
#if 0
  const char* pChar = cmdLine.c_str();
  while(pChar != NULL || *pChar != '\0')
    {
    char key[IarArg::KEY_LEN];
    char value[IarArg::VAL_LEN];
    // We need to accept whole string.
    pChar = IarArg::ParseNext(pChar, key, value);
    if (pChar != NULL)
      {
      // Find the key in a map.
      }
    }
#endif

  bool isMulti = false;
  std::string multicmd = "";
  std::vector<std::string> cmds = cmSystemTools::SplitString(cmdLine, ' ');
  for (std::vector<std::string>::const_iterator it = cmds.begin();
       it != cmds.end(); ++it) {

    if (isMulti) {
      multicmd += " " + *it;
      opts.push_back(multicmd);
      isMulti = false;
      continue;
    }

    for (int i = 0; i < multiOptsLen; i++) {
      if ((*it).find(multiOpts[i]) == 0) {
        // Multi option.
        isMulti = true;
        multicmd = *it;
        break;
      }
    }

    if (isMulti) {
      continue;
    }

    if ((*it).find("-O") == 0) {
        // Skip big O it is unusable in IDE!
        continue;
    }

    if ((*it) == std::string(""))
    {
      continue;
    }

    opts.push_back(*it);
  }
}

void cmGlobalIarGenerator::GetCmdLines(std::vector<cmCustomCommand> const& rTmpCmdVec,
                                      std::string& rBuildCmd,
                                      int& rStart)
{
    for(std::vector<cmCustomCommand>::const_iterator it = rTmpCmdVec.begin();
            it != rTmpCmdVec.end(); ++it)
    {
        rStart += 1;
        rBuildCmd += "REM Begin Command "+int2str(rStart)+"\n";
        rBuildCmd += std::string("REM Description: ")+(*it).GetComment()+"\n";
        rBuildCmd += "\n";
        rBuildCmd += "REM Change working directory:\n";
        std::string cwd = std::string((*it).GetWorkingDirectory());
        //std::replace( cwd.begin(), cwd.end(), '/', '\\');
        rBuildCmd += "cd " + cwd + "\n";
        rBuildCmd += "REM Executing command lines:\n";

        const cmCustomCommandLines& cmdLines = (*it).GetCommandLines();

        for(cmCustomCommandLines::const_iterator it2 = cmdLines.begin();
                it2 != cmdLines.end(); ++it2)
        {
            std::string line = "";
            bool firstTok = true;
            for(cmCustomCommandLine::const_iterator it3 = (*it2).begin();
                    it3 != (*it2).end(); ++it3)
            {
                //if (firstTok)
                //{
                //    // Most likely a path.
                //    std::string token = (*it3);
                //    std::replace( token.begin(), token.end(), '/', '\\');
                //    line += token + " ";
                //    firstTok = false;
                //    continue;
                //}

                line += (*it3) + " ";
            }

            line += "\n";
            rBuildCmd += line;
        }

        rBuildCmd += "REM End Command "+int2str(rStart)+"\n\n";
    }
}

//----------------------------------------------------------------------------
void cmGlobalIarGenerator::ConvertTargetToProject(const cmTarget& tgt,
    cmGeneratorTarget* genTgt)
{
  // For IAR, each code related target is considered a separate IAR project.
  cmGlobalIarGenerator::Project* project = new cmGlobalIarGenerator::Project();
  project->name = genTgt->GetName();

  // Is this a lib or a linkable type?
  cmStateEnums::TargetType type = genTgt->GetType();
  project->isLib =
      (type == cmStateEnums::STATIC_LIBRARY ||
          type == cmStateEnums::SHARED_LIBRARY ||
          type == cmStateEnums::MODULE_LIBRARY ||
          type == cmStateEnums::OBJECT_LIBRARY ||
          type == cmStateEnums::INTERFACE_LIBRARY ||
          type == cmStateEnums::UNKNOWN_LIBRARY );

  cmMakefile* makeFile = tgt.GetMakefile();

  project->projectDir = makeFile->GetCurrentSourceDirectory();
  project->binaryDir = this->workspace.workspaceDir;

  // INCLUDE DIRECTORIES: Gather all includes.
  const std::vector <BT<std::string>> includeDirsVector =
      genTgt->GetIncludeDirectories("", "C");
  for (std::vector<BT<std::string>>::const_iterator it =
      includeDirsVector.begin();
      it != includeDirsVector.end(); ++it)
    {
    project->includes.push_back((*it).Value);
    }

  // SOURCE FILES: Gather all sources.
  std::vector<cmSourceFile*> sourceFilesVector;
  genTgt->GetSourceFiles(sourceFilesVector, "");

  for(std::vector<cmSourceFile*>::const_iterator it =
      sourceFilesVector.begin();
      it != sourceFilesVector.end();
      ++it)
    {
    project->sources.push_back((*it)->GetFullPath());
    }

  // Compose build configuration

  const std::vector<std::unique_ptr<cmTarget>>& owned =
      makeFile->GetOwnedImportedTargets();

  cmGlobalIarGenerator::BuildConfig buildCfg;
  buildCfg.name = GLOBALCFG.buildType;
  buildCfg.isDebug = (GLOBALCFG.buildType == "Debug");
  buildCfg.exeDir = buildCfg.name;
  buildCfg.objectDir = buildCfg.exeDir;
  buildCfg.listDir = buildCfg.exeDir;
  if (!buildCfg.exeDir.empty()) {
    buildCfg.objectDir += "/";
    buildCfg.listDir += "/";
  }
  buildCfg.objectDir += "Object";
  buildCfg.listDir += "List";
  buildCfg.toolchain = GLOBALCFG.tgtArch;
  buildCfg.outputFile = genTgt->GetExportName();

  buildCfg.preBuildCmd = "";
  buildCfg.postBuildCmd = "";

  std::string prebuild = project->binaryDir + "/" + buildCfg.exeDir+"/"+project->name+"_prebuild.bat";
  std::string postbuild = project->binaryDir + "/" + buildCfg.exeDir+"/"+project->name+"_postbuild.bat";

  buildCfg.icfPath = GLOBALCFG.linkerIcfFile;

  // Prebuild & postbuild.
  std::string buildCmd = "";
  buildCmd.reserve(2048);

  std::string cmdHdr = "";
  cmdHdr.reserve(1024);

  cmdHdr += "REM ==========================================================\n";
  cmdHdr += "REM This file has been generated from CMake cmGlobalIarGenerator.\n";
  cmdHdr += "REM DO NOT EDIT.\n";
  cmdHdr += "REM ==========================================================\n\n";

  int cmdIx = 0;
  // Pre-link and pre-build are prebuild, IAR does not have anything like pre-link...
  GetCmdLines(genTgt->GetPreBuildCommands(),
              buildCmd,
              cmdIx);

  GetCmdLines(genTgt->GetPreLinkCommands(),
              buildCmd,
              cmdIx);

  if (cmdIx > 0)
  {
      buildCfg.preBuildCmd = prebuild;
      FILE* pBuild = fopen(prebuild.c_str(), "w");
      if (pBuild != NULL)
      {
          fwrite(cmdHdr.c_str(), cmdHdr.length(), 1, pBuild);
          fwrite(buildCmd.c_str(), buildCmd.length(), 1, pBuild);
          fclose(pBuild);
      }
  }

  // Post-build is postbuild...
  cmdIx = 0;
  buildCmd = "";
  GetCmdLines(genTgt->GetPostBuildCommands(),
              buildCmd,
              cmdIx);
  if (cmdIx > 0)
  {
      buildCfg.postBuildCmd = postbuild;
      FILE* pBuild = fopen(postbuild.c_str(), "w");
      if (pBuild != NULL)
      {
          fwrite(cmdHdr.c_str(), cmdHdr.length(), 1, pBuild);
          fwrite(buildCmd.c_str(), buildCmd.length(), 1, pBuild);
          fclose(pBuild);
      }
  }


  // Compile definitions:
  std::vector<std::string> compileDefs;
  genTgt->GetCompileDefinitions(compileDefs, buildCfg.name, "C");
  for(std::vector<std::string>::const_iterator it = compileDefs.begin();
      it != compileDefs.end(); ++it)
    {
    buildCfg.compileDefs.push_back(*it);
    }

  // Compiler options:
  std::vector<std::string> compilerOpts;
  genTgt->GetCompileOptions(compilerOpts, buildCfg.name, "C");
  for (std::vector<std::string>::const_iterator it = compilerOpts.begin();
       it != compilerOpts.end(); ++it) {
    buildCfg.compilerOpts.push_back(*it);
  }

  // Linker options:

   std::string linkLibs;
   std::string frameworkPath;
   std::string linkPath;
   std::string flags;
   std::string linkFlags;
   cmLocalGenerator* lg = genTgt->GetLocalGenerator();
   cmLinkLineComputer linkLineComputer(lg,
                                       lg->GetStateSnapshot().GetDirectory());
   lg->GetTargetFlags(&linkLineComputer, GLOBALCFG.buildType, linkLibs, flags,
                      linkFlags, frameworkPath, linkPath,
                      (cmGeneratorTarget*)genTgt);

   cmGlobalIarGenerator::ParseCmdLineOpts(
     linkFlags, cmGlobalIarGenerator::MULTIOPTS_LINKER,
     sizeof(cmGlobalIarGenerator::MULTIOPTS_LINKER) / sizeof(const char*),
     buildCfg.linkerOpts);

  /*std::vector<std::string> linkerOpts;
  genTgt->GetLinkOptions(linkerOpts, buildCfg.name, "C");
  for (std::vector<std::string>::const_iterator it = linkerOpts.begin();
       it != linkerOpts.end(); ++it) {
    buildCfg.linkerOpts.push_back(*it);
  }*/

  std::string importedLocationStr = std::string("IMPORTED_LOCATION_")
                  + cmSystemTools::UpperCase(GLOBALCFG.buildType);

  // Libraries:
  const cmTarget::LinkLibraryVectorType& libs =
      tgt.GetOriginalLinkLibraries();
  for(cmTarget::LinkLibraryVectorType::const_iterator it = libs.begin();
      it != libs.end(); ++it)
    {
    bool found = false;
    /*for(std::vector<cmTarget*>::const_iterator it2 = owned.begin();
        it2 != owned.end(); ++it2)*/
    for (const auto& l : owned)
      {
      if (it->first == l.get()->GetName())
        {
          cmProp propStrPtr = l.get()->GetProperty(importedLocationStr);
          const char* pPropertyStr = NULL;
          if (propStrPtr != NULL)
          {
              pPropertyStr = propStrPtr->c_str();
          }
          propStrPtr = l.get()->GetProperty(std::string("IMPORTED_LOCATION"));
          const char* pNoBtPropertyStr = NULL;
          if (propStrPtr != NULL)
          {
              pNoBtPropertyStr = propStrPtr->c_str();
          }

          if (pPropertyStr != NULL)
          {
              buildCfg.libraries.push_back(pPropertyStr);
          }
          else if (pNoBtPropertyStr != NULL)
          {
              buildCfg.libraries.push_back(pNoBtPropertyStr);
          }

          found = true;
          break;
        }
      }

    if (!found)
      {
      // If there is no imported target attaches, look for it in regular
      // folders.
      std::string libpath = this->workspace.workspaceDir;
      libpath += std::string("/") + buildCfg.exeDir + "/";
      libpath += std::string(it->first) + ".a";

      buildCfg.libraries.push_back(libpath);
      }
    }

  // Add configurations to the list.
  project->buildCfg = buildCfg;

  project->CreateProjectFile();

  // Register the project into a workspace.
  this->workspace.RegisterProject(project->name, project);
}

//----------------------------------------------------------------------------
void cmGlobalIarGenerator::Project::CreateProjectFile()
{
  std::string fileName = this->binaryDir;
  fileName += std::string("/") + this->name + ".ewp";

  FILE* pFile = fopen(fileName.c_str(), "w");

  XmlNode root("project", "");
  root.NewChild("fileVersion", "2");

  XmlNode* config = root.NewChild("configuration");
  if (this->buildCfg.name.empty()) {
    config->NewChild("name", this->buildCfg.isDebug ? "Debug" : "Release");
  }
  else {
    config->NewChild("name", this->buildCfg.name);
  }

  XmlNode* toolchain = config->NewChild("toolchain");
  toolchain->NewChild("name", this->buildCfg.toolchain);

  config->NewChild("debug", this->buildCfg.isDebug ? "1" : "0");


  // GENERAL SETTINGS:
  IarSettings* generalSettings = new IarSettings("General", 3);
  config->AddChild(generalSettings);

  IarData* generalData = generalSettings->NewData(21, true, this->buildCfg.isDebug);
  generalData->NewOption("ExePath")->NewState(this->buildCfg.exeDir);
  generalData->NewOption("ObjPath")->NewState(this->buildCfg.objectDir + "/" + this->name);
  generalData->NewOption("ListPath")->NewState(this->buildCfg.listDir + "/" + this->name);
  generalData->NewOption("Variant", 20)->NewState("42");
  generalData->NewOption("GEndianMode")->NewState("0");


  std::string pPrintfIdStr = int2str(GLOBALCFG.printfFmtId);
  std::string pScanfIdStr = int2str(GLOBALCFG.scanfFmtId);

  generalData->NewOption("Input variant", 3)->NewState(pPrintfIdStr);
  generalData->NewOption("Input description")
              ->NewState("No specifier n, no float nor "
                  "long long, no scan set,"
                  " no assignment suppressing, without multibyte support.");
  generalData->NewOption("Output variant", 2)->NewState(pScanfIdStr);
  generalData->NewOption("Output description")
                ->NewState("No specifier a, A, without multibyte support.");
  generalData->NewOption("GOutputBinary")
                ->NewState(this->isLib ? "1" : "0");
  generalData->NewOption("FPU", 2)->NewState("3");
  generalData->NewOption("OGCoreOrChip")->NewState("1");


  std::string pDlibIdStr = int2str(GLOBALCFG.compilerDlibConfigId);

  generalData->NewOption("GRuntimeLibSelect", 0)->NewState(std::string(pDlibIdStr));
  generalData->NewOption("GRuntimeLibSelectSlave", 0)->NewState(std::string(pDlibIdStr));
  generalData->NewOption("RTDescription")
              ->NewState("Use the normal configuration of the C/C++ runtime"
                  " library. No locale interface, C locale, no file descriptor"
                  " support, no multibytes in printf and scanf, and no hex floats"
                  " in strtod.");
  generalData->NewOption("OGProductVersion")
                ->NewState(cmGlobalIarGenerator::GLOBALCFG.wbVersion);
  generalData->NewOption("OGLastSavedByProductVersion")
                ->NewState(cmGlobalIarGenerator::GLOBALCFG.wbVersion);
  generalData->NewOption("GeneralEnableMisra")->NewState("0");
  generalData->NewOption("GeneralMisraVerbose")->NewState("0");
  std::string chipSelection = cmGlobalIarGenerator::GLOBALCFG.chipSelection;
  chipSelection += "\t" + cmGlobalIarGenerator::GLOBALCFG.chipSelection;

  generalData->NewOption("OGChipSelectEditMenu")->NewState(chipSelection);


  const char* pGenLowLevelIfaceStr = GLOBALCFG.semihostingEnabled == "ON" ? "1" : "0";

  generalData->NewOption("GenLowLevelInterface")
                ->NewState(pGenLowLevelIfaceStr);
  generalData->NewOption("GEndianModeBE")->NewState("1");

  const char* pBufferedStr = GLOBALCFG.bufferedTermOut == "ON" ? "1" : "0";

  generalData->NewOption("OGBufferedTerminalOutput")->NewState(pBufferedStr);
  generalData->NewOption("GenStdoutInterface")->NewState("0");
  generalData->NewOption("GeneralMisraRules98", 0)
                ->NewState("100011111011010110111001110011111110111001101100010111011"
                    "110110110011111111111110011001111100111011100111111111111"
                    "1111111111111");
  generalData->NewOption("GeneralMisraVer")->NewState("0");
  generalData->NewOption("GeneralMisraRules04", 0)
                ->NewState("11110111001011111111100011011111111111111111111111111"
                    "00101111011110101011111111111111111111111111011111110"
                    "11111001111011111011111111111111111");
  generalData->NewOption("RTConfigPath2")
              ->NewState(std::string("$TOOLKIT_DIR$\\INC\\c\\DLib_Config_") + cmGlobalIarGenerator::GLOBALCFG.compilerDlibConfig + ".h");
  generalData->NewOption("GFPUCoreSlave", 20)->NewState("42");
  generalData->NewOption("GBECoreSlave", 20)->NewState("42");
  generalData->NewOption("OGUseCmsis")->NewState("0");
  generalData->NewOption("OGUseCmsisDspLib")->NewState("0");

  // ARM Compiler (ICCARM):
  IarSettings* iccArmSettings = new IarSettings("ICCARM", 2);
  config->AddChild(iccArmSettings);


  IarData* iccArmData = iccArmSettings->NewData(28, true, this->buildCfg.isDebug);

  iccArmData->NewOption("CCOptimizationNoSizeConstraints")->NewState("0");
  iccArmData->NewOption("CCDefines")->NewStates(this->buildCfg.compileDefs);
  iccArmData->NewOption("CCPreprocFile")->NewState("0");
  iccArmData->NewOption("CCPreprocComments")->NewState("0");
  iccArmData->NewOption("CCPreprocLine")->NewState("0");
  iccArmData->NewOption("CCListCFile")->NewState(this->buildCfg.isDebug ? "1" : "0");
  iccArmData->NewOption("CCListCMnemonics")->NewState("0");
  iccArmData->NewOption("CCListCMessages")->NewState("0");
  iccArmData->NewOption("CCListAssFile")->NewState("0");
  iccArmData->NewOption("CCListAssSource")->NewState("0");
  iccArmData->NewOption("CCEnableRemarks")->NewState("0");
  iccArmData->NewOption("CCDiagSuppress")
                ->NewState("");
  iccArmData->NewOption("CCDiagRemark")->NewState("");
  iccArmData->NewOption("CCDiagWarning")->NewState("");
  iccArmData->NewOption("CCDiagError")->NewState("");
  iccArmData->NewOption("CCObjPrefix")->NewState("1");
  iccArmData->NewOption("CCAllowList", 1)
                ->NewState(this->buildCfg.isDebug ? "0000000" : "1111111");
  iccArmData->NewOption("CCDebugInfo")->NewState(this->buildCfg.isDebug ? "1" : "0");
  iccArmData->NewOption("IEndianMode")->NewState("1");
  iccArmData->NewOption("IProcessor")->NewState("1");
  iccArmData->NewOption("IExtraOptionsCheck")->NewState("1");

  cmGlobalIarGenerator::ParseCmdLineOpts(
    GLOBALCFG.iarCCompilerFlags, MULTIOPTS_COMPILER,
    sizeof(MULTIOPTS_COMPILER) / sizeof(const char*),
    this->buildCfg.compilerOpts);

  iccArmData->NewOption("IExtraOptions")->NewStates(this->buildCfg.compilerOpts);
  iccArmData->NewOption("CCLangConformance")->NewState("0");
  iccArmData->NewOption("CCSignedPlainChar")->NewState("1");
  iccArmData->NewOption("CCRequirePrototypes")->NewState("0");
  iccArmData->NewOption("CCMultibyteSupport")->NewState("0");
  iccArmData->NewOption("CCDiagWarnAreErr")->NewState("0");
  iccArmData->NewOption("CCCompilerRuntimeInfo")->NewState("0");
  iccArmData->NewOption("IFpuProcessor")->NewState("1");
  iccArmData->NewOption("OutputFile")->NewState("$FILE_BNAME$.o");
  iccArmData->NewOption("CCLibConfigHeader")->NewState("1");
  iccArmData->NewOption("PreInclude")
                ->NewState(cmGlobalIarGenerator::GLOBALCFG.compilerPreInclude);
  iccArmData->NewOption("CompilerMisraOverride")->NewState("0");
  iccArmData->NewOption("CCIncludePath2")->NewStates(this->includes);
  iccArmData->NewOption("CCStdIncCheck")->NewState("0");
  iccArmData->NewOption("CCCodeSection")->NewState(".text");
  iccArmData->NewOption("IInterwork2")->NewState("0");
  iccArmData->NewOption("IProcessorMode2")->NewState("1");
  iccArmData->NewOption("CCOptLevel")->NewState(this->buildCfg.isDebug ? "0" : "3");
  iccArmData->NewOption("CCOptStrategy", 0)->NewState("1");
  iccArmData->NewOption("CCOptLevelSlave")
                ->NewState(this->buildCfg.isDebug ? "0" : "3");
  iccArmData->NewOption("CompilerMisraRules98", 0)
                ->NewState("100011111011010110111001110011111110111001101100010111"
                    "011110110110011111111111110011001111100111011100111111"
                    "1111111111111111111");
  iccArmData->NewOption("CompilerMisraRules04", 0)
                ->NewState("111101110010111111111000110111111111111111111111111110"
                    "010111101111010101111111111111111111111111101111111011"
                    "111001111011111011111111111111111");
  iccArmData->NewOption("CCPosIndRopi")->NewState("0");
  iccArmData->NewOption("CCPosIndRwpi")->NewState("0");
  iccArmData->NewOption("CCPosIndNoDynInit")->NewState("0");
  iccArmData->NewOption("IccLang")->NewState("2");
  iccArmData->NewOption("IccCDialect")->NewState("1");
  iccArmData->NewOption("IccAllowVLA")->NewState("0");
  iccArmData->NewOption("IccCppDialect")->NewState("2");
  iccArmData->NewOption("IccExceptions")->NewState("0");
  iccArmData->NewOption("IccRTTI")->NewState("0");
  iccArmData->NewOption("IccStaticDestr")->NewState("0");
  iccArmData->NewOption("IccCppInlineSemantics")->NewState("1");
  iccArmData->NewOption("IccCmsis")->NewState("1");
  iccArmData->NewOption("IccFloatSemantics")->NewState("0");


  // AARM:
  IarSettings* aArmSettings = new IarSettings("AARM", 2);
  config->AddChild(aArmSettings);
  IarData* aArmData = aArmSettings->NewData(8, true, this->buildCfg.isDebug);

  aArmData->NewOption("AObjPrefix")->NewState("1");
  aArmData->NewOption("AEndian")->NewState("1");
  aArmData->NewOption("ACaseSensitivity")->NewState("1");
  aArmData->NewOption("MacroChars", 0)->NewState("0");
  aArmData->NewOption("AWarnEnable")->NewState("0");
  aArmData->NewOption("AWarnWhat")->NewState("0");
  aArmData->NewOption("AWarnOne")->NewState("");
  aArmData->NewOption("AWarnRange1")->NewState("");
  aArmData->NewOption("AWarnRange2")->NewState("");
  aArmData->NewOption("ADebug")->NewState(this->buildCfg.isDebug ? "1" : "0");
  aArmData->NewOption("AltRegisterNames")->NewState("0");
  aArmData->NewOption("ADefines")->NewState("");
  aArmData->NewOption("AList")->NewState("0");
  aArmData->NewOption("AListHeader")->NewState("1");
  aArmData->NewOption("AListing")->NewState("1");
  aArmData->NewOption("Includes")->NewState("0");
  aArmData->NewOption("MacDefs")->NewState("0");
  aArmData->NewOption("MacExps")->NewState("1");
  aArmData->NewOption("MacExec")->NewState("0");
  aArmData->NewOption("OnlyAssed")->NewState("0");
  aArmData->NewOption("MultiLine")->NewState("0");
  aArmData->NewOption("PageLengthCheck")->NewState("0");
  aArmData->NewOption("PageLength")->NewState("80");
  aArmData->NewOption("TabSpacing")->NewState("8");
  aArmData->NewOption("AXRef")->NewState("0");
  aArmData->NewOption("AXRefDefines")->NewState("0");
  aArmData->NewOption("AXRefInternal")->NewState("0");
  aArmData->NewOption("AXRefDual")->NewState("0");
  aArmData->NewOption("AProcessor")->NewState("1");
  aArmData->NewOption("AFpuProcessor")->NewState("1");
  aArmData->NewOption("AOutputFile")->NewState("$FILE_BNAME$.o");
  aArmData->NewOption("AMultibyteSupport")->NewState("0");
  aArmData->NewOption("ALimitErrorsCheck")->NewState("0");
  aArmData->NewOption("ALimitErrorsEdit")->NewState("100");
  aArmData->NewOption("AIgnoreStdInclude")->NewState("0");
  aArmData->NewOption("AUserIncludes")->NewState("");
  aArmData->NewOption("AExtraOptionsCheckV2")->NewState("0");
  aArmData->NewOption("AExtraOptionsV2")->NewState("");


  // OBJCOPY:
  IarSettings* objCopySettings = new IarSettings("OBJCOPY", 0);
  config->AddChild(objCopySettings);
  IarData* objCopyData = objCopySettings->NewData(1, true, this->buildCfg.isDebug);

  objCopyData->NewOption("OOCOutputFormat", 2)->NewState("2");
  objCopyData->NewOption("OCOutputOverride")->NewState("0");

  std::string outFile = this->buildCfg.outputFile;
  outFile += ".bin";
  objCopyData->NewOption("OOCOutputFile")->NewState(outFile);
  objCopyData->NewOption("OOCCommandLineProducer")->NewState("1");
  objCopyData->NewOption("OOCObjCopyEnable")->NewState("0");


  // CUSTOM:
  IarSettings* customSettings = new IarSettings("CUSTOM", 3);
  XmlNode* customData = customSettings->NewChild("data");
  customData->NewChild("extensions");
  customData->NewChild("cmdline");
  config->AddChild(customSettings);


  // BICOMP:
  IarSettings* bicompSettings = new IarSettings("BICOMP", 0);
  bicompSettings->NewChild("data");
  config->AddChild(bicompSettings);


  // BUILDACTION:
  IarSettings* bactionSettings = new IarSettings("BUILDACTION", 1);
  XmlNode* bactionData = bactionSettings->NewChild("data");
  bactionData->NewChild("prebuild", this->buildCfg.preBuildCmd);
  bactionData->NewChild("postbuild", this->buildCfg.postBuildCmd);
  config->AddChild(bactionSettings);


  // IAR Linker (ILINK):
  IarSettings* ilinkSettings = new IarSettings("ILINK", 0);
  config->AddChild(ilinkSettings);
  IarData* ilinkData = ilinkSettings->NewData(15, true, this->buildCfg.isDebug);

  outFile = this->buildCfg.outputFile;
  outFile += ".elf";
  ilinkData->NewOption("IlinkOutputFile")->NewState(outFile);
  ilinkData->NewOption("IlinkLibIOConfig")->NewState("1");
  ilinkData->NewOption("XLinkMisraHandler")->NewState("0");
  ilinkData->NewOption("IlinkInputFileSlave")->NewState("0");
  ilinkData->NewOption("IlinkDebugInfoEnable")->NewState("1");
  ilinkData->NewOption("IlinkKeepSymbols")->NewState("");
  ilinkData->NewOption("IlinkRawBinaryFile")->NewState("");
  ilinkData->NewOption("IlinkRawBinarySymbol")->NewState("");
  ilinkData->NewOption("IlinkRawBinarySegment")->NewState("");
  ilinkData->NewOption("IlinkRawBinaryAlign")->NewState("");
  ilinkData->NewOption("IlinkDefines")->NewState("");
  ilinkData->NewOption("IlinkConfigDefines")->NewState("");
  ilinkData->NewOption("IlinkMapFile")->NewState("1");
  ilinkData->NewOption("IlinkLogFile")->NewState(this->buildCfg.isDebug ? "1" : "0");
  ilinkData->NewOption("IlinkLogInitialization")
                ->NewState(this->buildCfg.isDebug ? "1" : "0");
  ilinkData->NewOption("IlinkLogModule")->NewState(this->buildCfg.isDebug ? "1" : "0");
  ilinkData->NewOption("IlinkLogSection")->NewState(this->buildCfg.isDebug ? "1" : "0");
  ilinkData->NewOption("IlinkLogVeneer")->NewState(this->buildCfg.isDebug ? "1" : "0");
  ilinkData->NewOption("IlinkIcfOverride")->NewState("1");
  ilinkData->NewOption("IlinkIcfFile")->NewState(this->buildCfg.icfPath);
  ilinkData->NewOption("IlinkIcfFileSlave")->NewState("");
  ilinkData->NewOption("IlinkEnableRemarks")->NewState("0");
  ilinkData->NewOption("IlinkSuppressDiags")->NewState("");
  ilinkData->NewOption("IlinkTreatAsRem")->NewState("");
  ilinkData->NewOption("IlinkTreatAsWarn")->NewState("");
  ilinkData->NewOption("IlinkTreatAsErr")->NewState("");
  ilinkData->NewOption("IlinkWarningsAreErrors")->NewState("0");
  ilinkData->NewOption("IlinkUseExtraOptions")->NewState("1");
  ilinkData->NewOption("IlinkExtraOptions")->NewStates(this->buildCfg.linkerOpts);
  ilinkData->NewOption("IlinkLowLevelInterfaceSlave")->NewState("1");
  ilinkData->NewOption("IlinkAutoLibEnable")->NewState("1");
  ilinkData->NewOption("IlinkAdditionalLibs")->NewStates(this->buildCfg.libraries);
  ilinkData->NewOption("IlinkOverrideProgramEntryLabel")->NewState("0");
  ilinkData->NewOption("IlinkProgramEntryLabelSelect")->NewState("0");
  ilinkData->NewOption("IlinkProgramEntryLabel")
                ->NewState(cmGlobalIarGenerator::GLOBALCFG.linkerEntryRoutine);
  ilinkData->NewOption("DoFill")->NewState("0");
  ilinkData->NewOption("FillerByte")->NewState("0xFF");
  ilinkData->NewOption("FillerStart")->NewState("0x0");
  ilinkData->NewOption("FillerEnd")->NewState("0x0");
  ilinkData->NewOption("CrcSize", 0)->NewState("1");
  ilinkData->NewOption("CrcAlign")->NewState("1");
  ilinkData->NewOption("CrcPoly")->NewState("0x11021");
  ilinkData->NewOption("CrcCompl",0)->NewState("0");
  ilinkData->NewOption("CrcBitOrder",0)->NewState("0");
  ilinkData->NewOption("CrcInitialValue")->NewState("0x0");
  ilinkData->NewOption("DoCrc")->NewState("0");
  ilinkData->NewOption("IlinkBE8Slave")->NewState("1");
  ilinkData->NewOption("IlinkBufferedTerminalOutput")->NewState("1");
  ilinkData->NewOption("IlinkStdoutInterfaceSlave")->NewState("1");
  ilinkData->NewOption("CrcFullSize")->NewState("0");
  ilinkData->NewOption("IlinkIElfToolPostProcess")->NewState("0");
  ilinkData->NewOption("IlinkLogAutoLibSelect")
                ->NewState(this->buildCfg.isDebug ? "1" : "0");
  ilinkData->NewOption("IlinkLogRedirSymbols")->NewState("0");
  ilinkData->NewOption("IlinkLogUnusedFragments")->NewState("0");
  ilinkData->NewOption("IlinkCrcReverseByteOrder")->NewState("0");
  ilinkData->NewOption("IlinkCrcUseAsInput")->NewState("1");
  ilinkData->NewOption("IlinkOptInline")->NewState("1");
  ilinkData->NewOption("IlinkOptExceptionsAllow")->NewState("0");
  ilinkData->NewOption("IlinkOptExceptionsForce")->NewState("0");
  ilinkData->NewOption("IlinkCmsis")->NewState("1");
  ilinkData->NewOption("IlinkOptMergeDuplSections")->NewState("1");
  ilinkData->NewOption("IlinkOptUseVfe")->NewState("1");
  ilinkData->NewOption("IlinkOptForceVfe")->NewState("0");
  ilinkData->NewOption("IlinkStackAnalysisEnable")->NewState("0");
  ilinkData->NewOption("IlinkStackControlFile")->NewState("");
  ilinkData->NewOption("IlinkStackCallGraphFile")->NewState("");
  ilinkData->NewOption("CrcAlgorithm", 0)->NewState("1");
  ilinkData->NewOption("CrcUnitSize", 0)->NewState("0");

  // IARCHIVE:
  IarSettings* iArchiveSettings = new IarSettings("IARCHIVE", 0);
  config->AddChild(iArchiveSettings);
  IarData* iArchiveData = iArchiveSettings->NewData(0, true, this->buildCfg.isDebug);

  // 00
  iArchiveData->NewOption("IarchiveInputs")->NewState("");
  iArchiveData->NewOption("IarchiveOverride")->NewState("0");
  iArchiveData->NewOption("IarchiveOutput")->NewState("###Unitialized###");


  // BILINK:
  IarSettings* bilinkSettings = new IarSettings("BILINK", 0);
  bilinkSettings->NewChild("data");
  config->AddChild(bilinkSettings);


  // This file is outside our source directory.
  XmlNode* groupExternal = new XmlNode("group");
  groupExternal->NewChild("name", "external");


  FileTreeNode ftRoot = FileTreeNode(this->name);

  // ADD FILES:
  for (std::vector<std::string>::const_iterator it = this->sources.begin();
      it != this->sources.end();
      ++it)
    {
    if (it->substr(0, this->projectDir.length()) == this->projectDir)
      {
      // This file is inside our directory.
      FileTreeNode::AddToTree(&ftRoot,
          it->substr(this->projectDir.length()+1),
          *it);
      }
    else
      {
      // This file is outside our source directory.
      XmlNode* extFile = groupExternal->NewChild("file");
      extFile->NewChild("name", *it);
      }

    }

  ftRoot.TransformToIarTree(&root);
  root.AddChild(groupExternal);


  std::string output;
  output.reserve(1 << 20); // 16K.
  output += XML_DECL;
  root.ToString(0, output);

  fwrite(output.c_str(), output.length(), 1, pFile);

  fclose(pFile);

  if (!this->isLib)
  {
      this->CreateDebuggerFile();
  }
}

//----------------------------------------------------------------------------
void cmGlobalIarGenerator::RegisterProject(const std::string& projectName)
{
  (void) projectName;
}

//----------------------------------------------------------------------------
void cmGlobalIarGenerator::Project::CreateDebuggerFile()
{
  std::string debuggerFileName = this->binaryDir;
  debuggerFileName += std::string("/") + this->name + ".ewd";

  XmlNode root = XmlNode("project");
  root.NewChild("fileVersion", "2");

  for (unsigned int i = 0; i < 2; i++)
    {
    bool isDebug = (i == 0);

    XmlNode* config = root.NewChild("configuration");
    config->NewChild("name", isDebug ? "Debug" : "Release");

    XmlNode* toolchain = config->NewChild("toolchain");
    toolchain->NewChild("name", "ARM");

    config->NewChild("debug", isDebug ? "1" : "0");


    // GENERAL SETTINGS:
    IarSettings* cspySettings = new IarSettings("C-SPY", 2);
    config->AddChild(cspySettings);

    IarData* cspyData = cspySettings->NewData(25, true, isDebug);

    cspyData->NewOption("CInput")->NewState("1");
    cspyData->NewOption("CEndian")->NewState("1");
    cspyData->NewOption("CProcessor")->NewState("1");
    cspyData->NewOption("OCVariant")->NewState("0");
    cspyData->NewOption("MacOverride")->NewState("1");
    cspyData->NewOption("MacFile")
            ->NewState(cmGlobalIarGenerator::GLOBALCFG.dbgCspyMacfile);
    cspyData->NewOption("MemOverride")->NewState("1");
    cspyData->NewOption("MemFile")
            ->NewState(cmGlobalIarGenerator::GLOBALCFG.dbgCspyMemfile);
    cspyData->NewOption("RunToEnable")->NewState(isDebug ? "1" : "0");
    cspyData->NewOption("RunToName")->NewState("main");

    cspyData->NewOption("CExtraOptionsCheck")->NewState(isDebug ? "1" : "0");
    cspyData->NewOption("CExtraOptions")
            ->NewState(isDebug ? "--jet_use_hw_breakpoint_for_semihosting" : "");
    cspyData->NewOption("CFpuProcessor")->NewState("1");
    cspyData->NewOption("OCDDFArgumentProducer")->NewState("");
    cspyData->NewOption("OCDownloadSuppressDownload")->NewState("0");
    cspyData->NewOption("OCDownloadVerifyAll")->NewState("1");
    cspyData->NewOption("OCProductVersion")->NewState(GLOBALCFG.wbVersion);

    if (GLOBALCFG.dbgProbeSelection == "J-Link")
    {
        cspyData->NewOption("OCDynDriverList")->NewState("JLINK_ID");
    }
    else if (GLOBALCFG.dbgProbeSelection == "I-Jet")
    {
        cspyData->NewOption("OCDynDriverList")->NewState("IJET_ID");
    }
    else
    {
        // I-Jet is the default probe.
        cspyData->NewOption("OCDynDriverList")->NewState("IJET_ID");
    }

    cspyData->NewOption("OCLastSavedByProductVersion")
            ->NewState(GLOBALCFG.wbVersion);
    cspyData->NewOption("OCDownloadAttachToProgram")->NewState("0");

    cspyData->NewOption("UseFlashLoader")->NewState("0");
    cspyData->NewOption("CLowLevel")->NewState("1");
    cspyData->NewOption("OCBE8Slave")->NewState("1");
    cspyData->NewOption("MacFile2")->NewState("");
    cspyData->NewOption("CDevice")->NewState("1");
    cspyData->NewOption("FlashLoadersV3")
            ->NewState(isDebug ? "" :
                cmGlobalIarGenerator::GLOBALCFG.dbgCspyFlashLoaderv3);
    cspyData->NewOption("OCImagesSuppressCheck1")->NewState("0");
    cspyData->NewOption("OCImagesPath1")->NewState("");
    cspyData->NewOption("OCImagesSuppressCheck2")->NewState("0");
    cspyData->NewOption("OCImagesPath2")->NewState("");

    cspyData->NewOption("OCImagesSuppressCheck3")->NewState("0");
    cspyData->NewOption("OCImagesPath3")->NewState("");
    cspyData->NewOption("OverrideDefFlashBoard")
            ->NewState(isDebug ? "0" : "1");
    cspyData->NewOption("OCImagesOffset1")->NewState("");
    cspyData->NewOption("OCImagesOffset2")->NewState("");
    cspyData->NewOption("OCImagesOffset3")->NewState("");
    cspyData->NewOption("OCImagesUse1")->NewState("0");
    cspyData->NewOption("OCImagesUse2")->NewState("0");
    cspyData->NewOption("OCImagesUse3")->NewState("0");
    cspyData->NewOption("OCDeviceConfigMacroFile")->NewState("1");

    cspyData->NewOption("OCDebuggerExtraOption")->NewState("1");
    cspyData->NewOption("OCAllMTBOptions")->NewState("1");

    // ARMSIM_ID
    IarSettings* armsimId = new IarSettings("ARMSIM_ID", 2);
    config->AddChild(armsimId);

    IarData* armsimData = armsimId->NewData(1, true, isDebug);

    armsimData->NewOption("OCSimDriverInfo")->NewState("1");
    armsimData->NewOption("OCSimEnablePSP")->NewState("0");
    armsimData->NewOption("OCSimPspOverrideConfig")->NewState("0");
    armsimData->NewOption("OCSimPspConfigFile")->NewState("");

    // ANGEL_ID

    IarSettings* angelId = new IarSettings("ANGEL_ID", 2);
    config->AddChild(angelId);

    IarData* angelData = angelId->NewData(0, true, isDebug);

    angelData->NewOption("CCAngelHeartbeat")->NewState("1");
    angelData->NewOption("CAngelCommunication")->NewState("1");
    angelData->NewOption("CAngelCommBaud",0)->NewState("3");
    angelData->NewOption("CAngelCommPort",0)->NewState("0");
    angelData->NewOption("ANGELTCPIP")->NewState("aaa.bbb.ccc.ddd");
    angelData->NewOption("DoAngelLogfile")->NewState("0");
    angelData->NewOption("AngelLogFile")
            ->NewState(cmGlobalIarGenerator::GLOBALCFG.dbgLogFile);
    angelData->NewOption("OCDriverInfo")->NewState("1");

    // CMSISDAP_ID

    IarSettings* cmsisdapId = new IarSettings("CMSISDAP_ID", 2);
    config->AddChild(cmsisdapId);

    IarData* cmsisdapData = cmsisdapId->NewData(0, true, isDebug);

    cmsisdapData->NewOption("OCDriverInfo")->NewState("1");
    cmsisdapData->NewOption("CMSISDAPAttachSlave")->NewState("1");
    cmsisdapData->NewOption("OCIarProbeScriptFile")->NewState("1");
    cmsisdapData->NewOption("CMSISDAPResetList",1)->NewState("10");
    cmsisdapData->NewOption("CMSISDAPHWResetDuration")->NewState("300");
    cmsisdapData->NewOption("CMSISDAPHWResetDelay")->NewState("200");
    cmsisdapData->NewOption("CMSISDAPDoLogfile")->NewState("0");
    cmsisdapData->NewOption("CMSISDAPLogFile")
            ->NewState(cmGlobalIarGenerator::GLOBALCFG.dbgLogFile);
    cmsisdapData->NewOption("CMSISDAPInterfaceRadio")->NewState("0");
    cmsisdapData->NewOption("CMSISDAPInterfaceCmdLine")->NewState("0");
    cmsisdapData->NewOption("CMSISDAPMultiTargetEnable")->NewState("0");
    cmsisdapData->NewOption("CMSISDAPMultiTarget")->NewState("0");
    cmsisdapData->NewOption("CMSISDAPJtagSpeedList",0)->NewState("0");
    cmsisdapData->NewOption("CMSISDAPBreakpointRadio")->NewState("0");
    cmsisdapData->NewOption("CMSISDAPRestoreBreakpointsCheck")->NewState("0");
    cmsisdapData->NewOption("CMSISDAPUpdateBreakpointsEdit")
            ->NewState("_call_main");
    cmsisdapData->NewOption("RDICatchReset")->NewState("0");
    cmsisdapData->NewOption("RDICatchUndef")->NewState("0");
    cmsisdapData->NewOption("RDICatchSWI")->NewState("0");
    cmsisdapData->NewOption("RDICatchData")->NewState("0");
    cmsisdapData->NewOption("RDICatchPrefetch")->NewState("0");
    cmsisdapData->NewOption("RDICatchIRQ")->NewState("0");
    cmsisdapData->NewOption("RDICatchFIQ")->NewState("0");
    cmsisdapData->NewOption("CatchCORERESET")->NewState("0");
    cmsisdapData->NewOption("CatchMMERR")->NewState("0");
    cmsisdapData->NewOption("CatchNOCPERR")->NewState("0");
    cmsisdapData->NewOption("CatchCHKERR")->NewState("0");
    cmsisdapData->NewOption("CatchSTATERR")->NewState("0");
    cmsisdapData->NewOption("CatchBUSERR")->NewState("0");
    cmsisdapData->NewOption("CatchINTERR")->NewState("0");
    cmsisdapData->NewOption("CatchHARDERR")->NewState("0");
    cmsisdapData->NewOption("CatchDummy")->NewState("0");
    cmsisdapData->NewOption("CMSISDAPMultiCPUEnable")->NewState("0");
    cmsisdapData->NewOption("CMSISDAPMultiCPUNumber")->NewState("0");

    IarSettings* gdbId = new IarSettings("GDBSERVER_ID", 2);
    config->AddChild(gdbId);

    IarData* gdbData = gdbId->NewData(0, true, isDebug);

    gdbData->NewOption("OCDriverInfo")->NewState("1");
    gdbData->NewOption("TCPIP")->NewState("aaa.bbb.ccc.ddd");
    gdbData->NewOption("DoLogfile")->NewState("0");
    gdbData->NewOption("LogFile")
            ->NewState(cmGlobalIarGenerator::GLOBALCFG.dbgLogFile);
    gdbData->NewOption("CCJTagBreakpointRadio")->NewState("0");
    gdbData->NewOption("CCJTagDoUpdateBreakpoints")->NewState("0");
    gdbData->NewOption("CCJTagUpdateBreakpoints")->NewState("main");

    IarSettings* iarromId = new IarSettings("IARROM_ID", 2);
    config->AddChild(iarromId);

    IarData* iarromData = iarromId->NewData(1, true, isDebug);

    iarromData->NewOption("CRomLogFileCheck")
            ->NewState(cmGlobalIarGenerator::GLOBALCFG.dbgLogFile);
    iarromData->NewOption("CRomCommPort",0)->NewState("0");
    iarromData->NewOption("CRomCommBaud",0)->NewState("7");
    iarromData->NewOption("OCDriverInfo")->NewState("1");


    IarSettings* ijetId = new IarSettings("IJET_ID", 2);
    config->AddChild(ijetId);

    IarData* ijetData = ijetId->NewData(2, true, isDebug);

    ijetData->NewOption("OCDriverInfo")->NewState("1");
    ijetData->NewOption("IjetAttachSlave")->NewState("1");
    ijetData->NewOption("OCIarProbeScriptFile")->NewState("1");
    ijetData->NewOption("IjetResetList",1)->NewState("2");
    ijetData->NewOption("IjetHWResetDuration")->NewState("300");
    ijetData->NewOption("IjetHWResetDelay")->NewState("200");
    ijetData->NewOption("IjetPowerFromProbe")->NewState(isDebug ? "0" : "1");
    ijetData->NewOption("IjetPowerRadio")->NewState(isDebug ? "1" : "0");
    ijetData->NewOption("IjetDoLogfile")->NewState("0");
    ijetData->NewOption("IjetLogFile")
            ->NewState(cmGlobalIarGenerator::GLOBALCFG.dbgLogFile);
    ijetData->NewOption("IjetInterfaceRadio")->NewState("0");
    ijetData->NewOption("IjetInterfaceCmdLine")->NewState("0");
    ijetData->NewOption("IjetMultiTargetEnable")->NewState("0");
    ijetData->NewOption("IjetMultiTarget")->NewState("0");
    ijetData->NewOption("IjetScanChainNonARMDevices")->NewState("0");
    ijetData->NewOption("IjetIRLength")->NewState("0");
    ijetData->NewOption("IjetJtagSpeedList",0)->NewState(isDebug ? "5" : "0");
    ijetData->NewOption("IjetProtocolRadio")->NewState("0");
    ijetData->NewOption("IjetSwoPin")->NewState("0");
    ijetData->NewOption("IjetCpuClockEdit")->NewState("72.0");
    ijetData->NewOption("IjetSwoPrescalerList",1)->NewState("0");
    ijetData->NewOption("IjetBreakpointRadio")->NewState("0");
    ijetData->NewOption("IjetRestoreBreakpointsCheck")->NewState("0");
    ijetData->NewOption("IjetUpdateBreakpointsEdit")->NewState("_call_main");
    ijetData->NewOption("RDICatchReset")->NewState("0");
    ijetData->NewOption("RDICatchUndef")->NewState("0");
    ijetData->NewOption("RDICatchSWI")->NewState("0");
    ijetData->NewOption("RDICatchData")->NewState("0");
    ijetData->NewOption("RDICatchPrefetch")->NewState("0");
    ijetData->NewOption("RDICatchIRQ")->NewState("0");
    ijetData->NewOption("RDICatchFIQ")->NewState("0");
    ijetData->NewOption("CatchCORERESET")->NewState("0");
    ijetData->NewOption("CatchMMERR")->NewState("0");
    ijetData->NewOption("CatchNOCPERR")->NewState("0");
    ijetData->NewOption("CatchCHKERR")->NewState("0");
    ijetData->NewOption("CatchSTATERR")->NewState("0");
    ijetData->NewOption("CatchBUSERR")->NewState("0");
    ijetData->NewOption("CatchINTERR")->NewState("0");
    ijetData->NewOption("CatchHARDERR")->NewState("0");
    ijetData->NewOption("CatchDummy")->NewState("0");
    ijetData->NewOption("OCProbeCfgOverride")->NewState(isDebug ? "1" : "0");
    ijetData->NewOption("OCProbeConfig")
            ->NewState(isDebug ?
                cmGlobalIarGenerator::GLOBALCFG.dbgIjetProbeconfig : "");
    ijetData->NewOption("IjetProbeConfigRadio")
            ->NewState(isDebug ? "1" : "0");
    ijetData->NewOption("IjetMultiCPUEnable")->NewState("0");
    ijetData->NewOption("IjetMultiCPUNumber")->NewState("0");
    ijetData->NewOption("IjetSelectedCPUBehaviour")
            ->NewState(isDebug ? "R4" : "0");
    ijetData->NewOption("ICpuName")->NewState(isDebug ? "R4" : "");

    IarSettings* jlinkId = new IarSettings("JLINK_ID", 2);
    config->AddChild(jlinkId);

    IarData* jlinkData = jlinkId->NewData(15, true, isDebug);

    jlinkData->NewOption("JLinkSpeed")->NewState("10000");
    jlinkData->NewOption("CCJLinkDoLogfile")->NewState("0");
    jlinkData->NewOption("CCJLinkLogFile")
            ->NewState(cmGlobalIarGenerator::GLOBALCFG.dbgLogFile);
    jlinkData->NewOption("CCJLinkHWResetDelay")->NewState("0");
    jlinkData->NewOption("OCDriverInfo")->NewState("1");
    jlinkData->NewOption("JLinkInitialSpeed")->NewState("32");
    jlinkData->NewOption("CCDoJlinkMultiTarget")->NewState("0");
    jlinkData->NewOption("CCScanChainNonARMDevices")->NewState("0");
    jlinkData->NewOption("CCJLinkMultiTarget")->NewState("0");
    jlinkData->NewOption("CCJLinkIRLength")->NewState("0");
    jlinkData->NewOption("CCJLinkCommRadio")->NewState("0");
    jlinkData->NewOption("CCJLinkTCPIP")->NewState("aaa.bbb.ccc.ddd");
    jlinkData->NewOption("CCJLinkSpeedRadioV2")->NewState("1");
    jlinkData->NewOption("CCUSBDevice",1)->NewState("1");
    jlinkData->NewOption("CCRDICatchReset")->NewState("0");
    jlinkData->NewOption("CCRDICatchUndef")->NewState("0");
    jlinkData->NewOption("CCRDICatchSWI")->NewState("0");
    jlinkData->NewOption("CCRDICatchData")->NewState("0");
    jlinkData->NewOption("CCRDICatchPrefetch")->NewState("0");
    jlinkData->NewOption("CCRDICatchIRQ")->NewState("0");
    jlinkData->NewOption("CCRDICatchFIQ")->NewState("0");
    jlinkData->NewOption("CCJLinkBreakpointRadio")->NewState("0");
    jlinkData->NewOption("CCJLinkDoUpdateBreakpoints")->NewState("0");
    jlinkData->NewOption("CCJLinkUpdateBreakpoints")->NewState("main");
    jlinkData->NewOption("CCJLinkInterfaceRadio")->NewState("0");
    jlinkData->NewOption("OCJLinkAttachSlave")->NewState("1");
    jlinkData->NewOption("CCJLinkResetList",6)
            ->NewState(isDebug ? "1" : "5");
    jlinkData->NewOption("CCJLinkInterfaceCmdLine")->NewState("0");
    jlinkData->NewOption("CCCatchCORERESET")->NewState("0");
    jlinkData->NewOption("CCCatchMMERR")->NewState("0");
    jlinkData->NewOption("CCCatchNOCPERR")->NewState("0");
    jlinkData->NewOption("CCCatchCHRERR")->NewState("0");
    jlinkData->NewOption("CCCatchSTATERR")->NewState("0");
    jlinkData->NewOption("CCCatchBUSERR")->NewState("0");
    jlinkData->NewOption("CCCatchINTERR")->NewState("0");
    jlinkData->NewOption("CCCatchHARDERR")->NewState("0");
    jlinkData->NewOption("CCCatchDummy")->NewState("0");
    jlinkData->NewOption("OCJLinkScriptFile")->NewState("1");
    jlinkData->NewOption("CCJLinkUsbSerialNo")->NewState("");
    jlinkData->NewOption("CCTcpIpAlt",0)->NewState("0");
    jlinkData->NewOption("CCJLinkTcpIpSerialNo")->NewState("");
    jlinkData->NewOption("CCCpuClockEdit")->NewState("72.0");
    jlinkData->NewOption("CCSwoClockAuto")->NewState("0");
    jlinkData->NewOption("CCSwoClockEdit")->NewState("2000");
    jlinkData->NewOption("OCJLinkTraceSource")->NewState("0");
    jlinkData->NewOption("OCJLinkTraceSourceDummy")->NewState("0");
    jlinkData->NewOption("OCJLinkDeviceName")->NewState("1");

    IarSettings* lmiftdiId = new IarSettings("LMIFTDI_ID", 2);
    config->AddChild(lmiftdiId);

    IarData* lmiftdiData = lmiftdiId->NewData(2, true, isDebug);

    lmiftdiData->NewOption("OCDriverInfo")->NewState("1");
    lmiftdiData->NewOption("LmiftdiSpeed")->NewState("500");
    lmiftdiData->NewOption("CCLmiftdiDoLogfile")->NewState("0");
    lmiftdiData->NewOption("CCLmiftdiLogFile")
            ->NewState(cmGlobalIarGenerator::GLOBALCFG.dbgLogFile);
    lmiftdiData->NewOption("CCLmiFtdiInterfaceRadio")->NewState("0");
    lmiftdiData->NewOption("CCLmiFtdiInterfaceCmdLine")->NewState("0");

    IarSettings* macraigorId = new IarSettings("MACRAIGOR_ID", 2);
    config->AddChild(macraigorId);

    IarData* macraigorData = macraigorId->NewData(3, true, isDebug);

    macraigorData->NewOption("jtag",0)->NewState("0");
    macraigorData->NewOption("EmuSpeed")->NewState("1");
    macraigorData->NewOption("TCPIP")->NewState("aaa.bbb.ccc.ddd");
    macraigorData->NewOption("DoLogfile")->NewState("0");
    macraigorData->NewOption("LogFile")
            ->NewState(cmGlobalIarGenerator::GLOBALCFG.dbgLogFile);
    macraigorData->NewOption("DoEmuMultiTarget")->NewState("0");
    macraigorData->NewOption("EmuMultiTarget")->NewState("0@ARM7TDMI");
    macraigorData->NewOption("EmuHWReset")->NewState("0");
    macraigorData->NewOption("CEmuCommBaud",0)->NewState("4");
    macraigorData->NewOption("CEmuCommPort",0)->NewState("0");
    macraigorData->NewOption("jtago",0)->NewState("0");
    macraigorData->NewOption("OCDriverInfo")->NewState("1");
    macraigorData->NewOption("UnusedAddr")->NewState("0x00800000");
    macraigorData->NewOption("CCMacraigorHWResetDelay")->NewState("");
    macraigorData->NewOption("CCJTagBreakpointRadio")->NewState("0");
    macraigorData->NewOption("CCJTagDoUpdateBreakpoints")->NewState("0");
    macraigorData->NewOption("CCJTagUpdateBreakpoints")->NewState("main");
    macraigorData->NewOption("CCMacraigorInterfaceRadio")->NewState("0");
    macraigorData->NewOption("CCMacraigorInterfaceCmdLine")->NewState("0");

    IarSettings* pemicroId = new IarSettings("PEMICRO_ID", 2);
    config->AddChild(pemicroId);

    IarData* pemicroData = pemicroId->NewData(1, true, isDebug);

    pemicroData->NewOption("OCDriverInfo")->NewState("1");
    pemicroData->NewOption("OCPEMicroAttachSlave")->NewState("1");
    pemicroData->NewOption("CCPEMicroInterfaceList",0)->NewState("0");
    pemicroData->NewOption("CCPEMicroResetDelay")->NewState("");
    pemicroData->NewOption("CCPEMicroJtagSpeed")->NewState("#UNINITIALIZED#");
    pemicroData->NewOption("CCJPEMicroShowSettings")->NewState("0");
    pemicroData->NewOption("DoLogfile")->NewState("0");
    pemicroData->NewOption("LogFile")
            ->NewState(cmGlobalIarGenerator::GLOBALCFG.dbgLogFile);
    pemicroData->NewOption("CCPEMicroUSBDevice",0)->NewState("0");
    pemicroData->NewOption("CCPEMicroSerialPort",0)->NewState("0");
    pemicroData->NewOption("CCJPEMicroTCPIPAutoScanNetwork")->NewState("1");
    pemicroData->NewOption("CCPEMicroTCPIP")->NewState("10.0.0.1");
    pemicroData->NewOption("CCPEMicroCommCmdLineProducer")->NewState("0");
    pemicroData->NewOption("CCSTLinkInterfaceRadio")->NewState("0");
    pemicroData->NewOption("CCSTLinkInterfaceCmdLine")->NewState("0");


    IarSettings* rdiId = new IarSettings("RDI_ID", 2);
    config->AddChild(rdiId);

    IarData* rdiData = rdiId->NewData(2, true, isDebug);

    rdiData->NewOption("CRDIDriverDll")->NewState("###Uninitialized###");
    rdiData->NewOption("CRDILogFileCheck")->NewState("0");
    rdiData->NewOption("CRDILogFileEdit")
            ->NewState(cmGlobalIarGenerator::GLOBALCFG.dbgLogFile);
    rdiData->NewOption("CCRDIHWReset")->NewState("0");
    rdiData->NewOption("CCRDICatchReset")->NewState("0");
    rdiData->NewOption("CCRDICatchUndef")->NewState("0");
    rdiData->NewOption("CCRDICatchSWI")->NewState("0");
    rdiData->NewOption("CCRDICatchData")->NewState("0");
    rdiData->NewOption("CCRDICatchPrefetch")->NewState("0");
    rdiData->NewOption("CCRDICatchIRQ")->NewState("0");
    rdiData->NewOption("CCRDICatchFIQ")->NewState("0");
    rdiData->NewOption("OCDriverInfo")->NewState("1");


    IarSettings* stlinkId = new IarSettings("STLINK_ID", 2);
    config->AddChild(stlinkId);

    IarData* stlinkData = stlinkId->NewData(2, true, isDebug);


    stlinkData->NewOption("OCDriverInfo")->NewState("1");
    stlinkData->NewOption("CCSTLinkInterfaceRadio")->NewState("0");
    stlinkData->NewOption("CCSTLinkInterfaceCmdLine")->NewState("0");
    stlinkData->NewOption("CCSTLinkResetList",1)->NewState("0");
    stlinkData->NewOption("CCCpuClockEdit")->NewState("72.0");
    stlinkData->NewOption("CCSwoClockAuto")->NewState("0");
    stlinkData->NewOption("CCSwoClockEdit")->NewState("2000");


    IarSettings* thirdPartyId = new IarSettings("THIRDPARTY_ID", 2);
    config->AddChild(thirdPartyId);

    IarData* thirdPartyData = thirdPartyId->NewData(0, true, isDebug);

    thirdPartyData->NewOption("CThirdPartyDriverDll")
            ->NewState("###Uninitialized###");
    thirdPartyData->NewOption("CThirdPartyLogFileCheck")->NewState("0");
    thirdPartyData->NewOption("CThirdPartyLogFileEditB")
            ->NewState(cmGlobalIarGenerator::GLOBALCFG.dbgLogFile);
    thirdPartyData->NewOption("OCDriverInfo")->NewState("1");

    IarSettings* xds100Id = new IarSettings("XDS100_ID", 2);
    config->AddChild(xds100Id);

    IarData* xds100Data = xds100Id->NewData(2, true, isDebug);

    xds100Data->NewOption("OCDriverInfo")->NewState("1");
    xds100Data->NewOption("OCXDS100AttachSlave")->NewState("1");
    xds100Data->NewOption("TIPackageOverride")->NewState("0");
    xds100Data->NewOption("TIPackage")->NewState("");
    xds100Data->NewOption("CCXds100InterfaceList",0)->NewState("0");
    xds100Data->NewOption("BoardFile")->NewState("");
    xds100Data->NewOption("DoLogfile")->NewState("0");
    xds100Data->NewOption("LogFile")
            ->NewState(cmGlobalIarGenerator::GLOBALCFG.dbgLogFile);


    XmlNode* debuggerPlugins = new XmlNode("debuggerPlugins");

    cmsys::Glob g;
    g.SetRecurse(true);
    g.RecurseThroughSymlinksOff();
    std::string expr = cmGlobalIarGenerator::GLOBALCFG.iarArmPath;
    expr += std::string("/plugins/") + "*.ewplugin";

    g.FindFiles(expr);
    std::vector<std::string>& files = g.GetFiles();

    // Allow language checking.
    for(std::vector<std::string>::const_iterator it = files.begin();
        it != files.end(); ++it)
      {

      if (it->find("JPN") == std::string::npos)
        {
        debuggerPlugins->AddChild(
            new IarDebuggerPlugin(cmGlobalIarGenerator::ToToolkitPath(*it),
                ((it->find(GLOBALCFG.rtos) != std::string::npos))));
        }
      }

    expr = cmGlobalIarGenerator::GLOBALCFG.iarArmPath
        .substr(0, cmGlobalIarGenerator::GLOBALCFG.iarArmPath.length() -
            (sizeof("/arm")-1));
    expr += std::string("/common/plugins/") + "*.ewplugin";

    g.FindFiles(expr);
    files = g.GetFiles();

    for(std::vector<std::string>::const_iterator it = files.begin();
        it != files.end(); ++it)
      {
      if (it->find("JPN") == std::string::npos)
        {
        bool load = (it->find("SymList") != std::string::npos) ||
            (it->find("CodeCoverage") != std::string::npos);

        debuggerPlugins->AddChild(
            new IarDebuggerPlugin(cmGlobalIarGenerator::ToWorkbenchPath(*it),
                load));
        }
      }

    config->AddChild(debuggerPlugins);
    }

  FILE* pFile = fopen(debuggerFileName.c_str(), "w");

  std::string output;
  output.reserve(1 << 20); // 1K.
  output += XML_DECL;

  root.ToString(0, output);
  fwrite(output.c_str(), output.length(), 1, pFile);


  fclose(pFile);

}

//----------------------------------------------------------------------------
void cmGlobalIarGenerator::Workspace::CreateWorkspaceFile()
{
  const static std::string errorCheck =
      "if %ERRORLEVEL% NEQ 0 (SET RETURN_VALUE=1)\n";
  std::string wsFileName = this->workspaceDir + "/" + this->name + ".eww";
  this->workspacePath = wsFileName;
  std::string batFileName = this->workspaceDir + "/BUILD_" + this->name + ".bat";

  std::string iarBuildCmd = cmGlobalIarGenerator::GLOBALCFG.iarArmPath;
  std::size_t lastSlash = iarBuildCmd.find_last_of("/\\");
  if (lastSlash != std::string::npos)
  {
      iarBuildCmd = iarBuildCmd.substr(0, lastSlash) + "/common/bin/IarBuild.exe";
  }
  else
  {
      iarBuildCmd = "IarBuild.exe";
  }

  std::replace( iarBuildCmd.begin(), iarBuildCmd.end(), '/', '\\');

  /*printf("Build cmd: %s.\n", iarBuildCmd.c_str());*/

  FILE* pFile = fopen(wsFileName.c_str(), "w");
  FILE* pBatFile = fopen(batFileName.c_str(), "w");

  std::string output;
  output.reserve(1 << 20); // 1K.
  output += XML_DECL;

  std::string batchOutput = "";
  batchOutput.reserve(1 << 20); // 1K.
  batchOutput += "REM ===================================================\n";
  batchOutput += "REM IAR BUILD (generated from CMake extraIarGenerator).\n";
  batchOutput += "REM ===================================================\n\n";
  batchOutput += "SET RETURN_VALUE=0\n\n";

  XmlNode root = XmlNode("workspace");

  XmlNode* batch = new XmlNode("batchDefinition");
  batch->NewChild("name", cmGlobalIarGenerator::GLOBALCFG.buildType + "_BuildAll");

  std::vector<Project*> vProjects;

  // Go through all registered projects.
  for (std::map<std::string, Project*>::const_iterator it =
          this->projects.begin();
          it != this->projects.end();
          ++it)
  {
      // Libraries first.

      if (it->second->isLib)
      {
          // Register project file to various structures.
          XmlNode* projEntry = root.NewChild("project");

          std::string projPath = it->second->binaryDir;
          projPath += std::string("/") + it->second->name + ".ewp";

          projEntry->NewChild("path", projPath);

          XmlNode* member = batch->NewChild("member");
          member->NewChild("project", it->first);
          member->NewChild("configuration", cmGlobalIarGenerator::GLOBALCFG.buildType);

          // Add batch command.
          std::string projPathWin = projPath;
          std::replace( projPathWin.begin(), projPathWin.end(), '/', '\\');
          batchOutput += "\"" + iarBuildCmd + "\" \""
                  + projPathWin + "\" -build "
                  + cmGlobalIarGenerator::GLOBALCFG.buildType +" -log all\n";
          batchOutput += errorCheck;
      }
      else
      {
          vProjects.push_back(it->second);
      }
  }

  for (std::vector<Project*>::const_iterator it = vProjects.begin();
          it != vProjects.end();
       ++it)
    {
      // Executables next.

      // Register project file to various structures.
      XmlNode* projEntry = root.NewChild("project");

      std::string projPath = (*it)->binaryDir;
      projPath += std::string("/") + (*it)->name + ".ewp";

      projEntry->NewChild("path", projPath);

      XmlNode* member = batch->NewChild("member");
      member->NewChild("project", (*it)->name);
      member->NewChild("configuration", cmGlobalIarGenerator::GLOBALCFG.buildType);

      // Add batch command.
      std::string projPathWin = projPath;
      std::replace( projPathWin.begin(), projPathWin.end(), '/', '\\');
      batchOutput += "\"" + iarBuildCmd + "\" \""
              + projPathWin + "\" -build "
              + cmGlobalIarGenerator::GLOBALCFG.buildType +" -log all\n";
      batchOutput += errorCheck;
    }

  batchOutput += "\nexit %RETURN_VALUE%\n\n";

  batchOutput += "\n\nREM ===================================================\n";
  batchOutput += "REM END IAR BUILD.\n";
  batchOutput += "REM ===================================================\n\n";


  XmlNode* batchBuild = root.NewChild("batchBuild");
  batchBuild->AddChild(batch);

  root.ToString(0, output);

  fwrite(output.c_str(), output.length(), 1, pFile);
  fwrite(batchOutput.c_str(), batchOutput.length(), 1, pBatFile);

  fclose(pFile);
  fclose(pBatFile);

  //this->CreateDebuggerFile();
}

//----------------------------------------------------------------------------
void cmGlobalIarGenerator::Workspace::RegisterProject(std::string wsName,
    Project* project)
{
  this->projects.insert(std::make_pair(wsName, project));
}


#include <future>

#include <windows.h>

#include <objbase.h>
#include <shellapi.h>

static bool OpenWorkspace(std::string workspace)
{
    HRESULT comInitialized =
        CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(comInitialized)) {
        return false;
    }

    HINSTANCE hi =
        ShellExecuteA(NULL, "open", workspace.c_str(), NULL, NULL, SW_SHOWNORMAL);

    CoUninitialize();

    return reinterpret_cast<intptr_t>(hi) > 32;
}


bool cmGlobalIarGenerator::Open(const std::string& bindir,
    const std::string& projectName,
    bool dryRun)
{
    std::string projFile = bindir + "/" + projectName + ".eww";
    // TODO: COMMENT
    // cmSystemTools::Message(std::string("Trying to OPEN: ") + projFile);
    // END TODO

    if (dryRun) {
        return cmSystemTools::FileExists(projFile, true);
    }

    return std::async(std::launch::async, OpenWorkspace, projFile).get();
}
