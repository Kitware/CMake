/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmGraphVizWriter.h"
#include "cmMakefile.h"
#include "cmLocalGenerator.h"
#include "cmGlobalGenerator.h"
#include "cmGeneratedFileStream.h"

#include <memory>



static const char* getShapeForTarget(const cmTarget* target)
{
  if (!target)
    {
    return "ellipse";
    }

  switch ( target->GetType() )
    {
    case cmTarget::EXECUTABLE:
      return "house";
    case cmTarget::STATIC_LIBRARY:
      return "diamond";
    case cmTarget::SHARED_LIBRARY:
      return "polygon";
    case cmTarget::MODULE_LIBRARY:
      return "octagon";
    default:
      break;
    }

  return "box";
}


cmGraphVizWriter::cmGraphVizWriter(const std::vector<cmLocalGenerator*>&
                                                               localGenerators)
:GraphType("digraph")
,GraphName("GG")
,GraphHeader("node [\n  fontsize = \"12\"\n];")
,GraphNodePrefix("node")
,LocalGenerators(localGenerators)
{
  int cnt = collectAllTargets();
  collectAllExternalLibs(cnt);
}


void cmGraphVizWriter::ReadSettings(const char* settingsFileName,
                                    const char* fallbackSettingsFileName)
{
  cmake cm;
  cmGlobalGenerator ggi;
  ggi.SetCMakeInstance(&cm);
  std::auto_ptr<cmLocalGenerator> lg(ggi.CreateLocalGenerator());
  cmMakefile *mf = lg->GetMakefile();

  const char* inFileName = settingsFileName;

  if ( !cmSystemTools::FileExists(inFileName) )
    {
    inFileName = fallbackSettingsFileName;
    if ( !cmSystemTools::FileExists(inFileName) )
      {
      return;
      }
    }

  if ( !mf->ReadListFile(0, inFileName) )
    {
    cmSystemTools::Error("Problem opening GraphViz options file: ",
                         inFileName);
    return;
    }

  std::cout << "Reading GraphViz options file: " << inFileName << std::endl;

#define __set_if_set(var, cmakeDefinition) \
  { \
  const char* value = mf->GetDefinition(cmakeDefinition); \
  if ( value ) \
    { \
    var = value; \
    } \
  }

  __set_if_set(this->GraphType, "GRAPHVIZ_GRAPH_TYPE");
  __set_if_set(this->GraphName, "GRAPHVIZ_GRAPH_NAME");
  __set_if_set(this->GraphHeader, "GRAPHVIZ_GRAPH_HEADER");
  __set_if_set(this->GraphNodePrefix, "GRAPHVIZ_NODE_PREFIX");

  this->TargetsToIgnore.clear();
  const char* ignoreTargets = mf->GetDefinition("GRAPHVIZ_IGNORE_TARGETS");
  if ( ignoreTargets )
    {
    std::vector<std::string> ignoreTargetsVector;
    cmSystemTools::ExpandListArgument(ignoreTargets,ignoreTargetsVector);
    for(std::vector<std::string>::iterator itvIt = ignoreTargetsVector.begin();
        itvIt != ignoreTargetsVector.end();
        ++ itvIt )
      {
      this->TargetsToIgnore.insert(itvIt->c_str());
      }
    }

}


void cmGraphVizWriter::WritePerTargetFiles(const char* fileName)
{
  for(std::map<cmStdString, const cmTarget*>::const_iterator ptrIt =
                                                      this->TargetPtrs.begin();
      ptrIt != this->TargetPtrs.end();
      ++ptrIt)
    {
    if (ptrIt->second == NULL)
      {
      continue;
      }

    if ((ptrIt->second->GetType() != cmTarget::EXECUTABLE)
      && (ptrIt->second->GetType() != cmTarget::STATIC_LIBRARY)
      && (ptrIt->second->GetType() != cmTarget::SHARED_LIBRARY)
      && (ptrIt->second->GetType() != cmTarget::MODULE_LIBRARY))
      {
      continue;
      }

    std::set<std::string> insertedConnections;
    std::set<std::string> insertedNodes;

    std::string currentFilename = fileName;
    currentFilename += ".";
    currentFilename += ptrIt->first;
    cmGeneratedFileStream str(currentFilename.c_str());
    if ( !str )
      {
      return;
      }

    fprintf(stderr, "Writing %s...\n", currentFilename.c_str());
    this->WriteHeader(str);

    this->WriteConnections(ptrIt->first.c_str(),
                              insertedNodes, insertedConnections, str);
    this->WriteFooter(str);
    }

}


void cmGraphVizWriter::WriteGlobalFile(const char* fileName)
{
  cmGeneratedFileStream str(fileName);
  if ( !str )
    {
    return;
    }
  this->WriteHeader(str);

  fprintf(stderr, "Writing %s...\n", fileName);
  std::set<std::string> insertedConnections;
  std::set<std::string> insertedNodes;

  for(std::map<cmStdString, const cmTarget*>::const_iterator ptrIt =
                                                      this->TargetPtrs.begin();
      ptrIt != this->TargetPtrs.end();
      ++ptrIt)
    {
    if (ptrIt->second == NULL)
      {
      continue;
      }

    if ((ptrIt->second->GetType() != cmTarget::EXECUTABLE)
      && (ptrIt->second->GetType() != cmTarget::STATIC_LIBRARY)
      && (ptrIt->second->GetType() != cmTarget::SHARED_LIBRARY)
      && (ptrIt->second->GetType() != cmTarget::MODULE_LIBRARY))
      {
      continue;
      }

    this->WriteConnections(ptrIt->first.c_str(),
                              insertedNodes, insertedConnections, str);
    }
  this->WriteFooter(str);
}


void cmGraphVizWriter::WriteHeader(cmGeneratedFileStream& str) const
{
  str << this->GraphType << " " << this->GraphName << " {" << std::endl;
  str << this->GraphHeader << std::endl;
}


void cmGraphVizWriter::WriteFooter(cmGeneratedFileStream& str) const
{
  str << "}" << std::endl;
}


void cmGraphVizWriter::WriteConnections(const char* targetName,
                                    std::set<std::string>& insertedNodes,
                                    std::set<std::string>& insertedConnections,
                                    cmGeneratedFileStream& str) const
{
  std::map<cmStdString, const cmTarget* >::const_iterator targetPtrIt =
                                             this->TargetPtrs.find(targetName);

  if (targetPtrIt == this->TargetPtrs.end())  // not found at all
    {
    return;
    }

  this->WriteNode(targetName, targetPtrIt->second, insertedNodes, str);

  if (targetPtrIt->second == NULL) // it's an external library
    {
    return;
    }


  std::string myNodeName = this->TargetNamesNodes.find(targetName)->second;

  const cmTarget::LinkLibraryVectorType* ll =
                            &(targetPtrIt->second->GetOriginalLinkLibraries());

  for (cmTarget::LinkLibraryVectorType::const_iterator llit = ll->begin();
       llit != ll->end();
       ++ llit )
    {
    const char* libName = llit->first.c_str();
    std::map<cmStdString, cmStdString>::const_iterator libNameIt =
                                          this->TargetNamesNodes.find(libName);

    std::string connectionName = myNodeName;
    connectionName += "-";
    connectionName += libNameIt->second;
    if (insertedConnections.find(connectionName) == insertedConnections.end())
      {
      insertedConnections.insert(connectionName);
      this->WriteNode(libName, this->TargetPtrs.find(libName)->second,
                      insertedNodes, str);

      str << "    \"" << myNodeName.c_str() << "\" -> \""
          << libNameIt->second.c_str() << "\"";
      str << " // " << targetName << " -> " << libName << std::endl;
      this->WriteConnections(libName, insertedNodes, insertedConnections, str);
      }
    }

}


void cmGraphVizWriter::WriteNode(const char* targetName,
                                 const cmTarget* target,
                                 std::set<std::string>& insertedNodes,
                                 cmGeneratedFileStream& str) const
{
  if (insertedNodes.find(targetName) == insertedNodes.end())
  {
    insertedNodes.insert(targetName);
    std::map<cmStdString, cmStdString>::const_iterator nameIt =
                                       this->TargetNamesNodes.find(targetName);

    str << "    \"" << nameIt->second.c_str() << "\" [ label=\""
        << targetName <<  "\" shape=\"" << getShapeForTarget(target)
        << "\"];" << std::endl;
  }
}


int cmGraphVizWriter::collectAllTargets()
{
  int cnt = 0;
  // First pass get the list of all cmake targets
  for (std::vector<cmLocalGenerator*>::const_iterator lit =
                                                 this->LocalGenerators.begin();
       lit != this->LocalGenerators.end();
       ++ lit )
    {
    const cmTargets* targets = &((*lit)->GetMakefile()->GetTargets());
    for ( cmTargets::const_iterator tit = targets->begin();
          tit != targets->end();
          ++ tit )
      {
      const char* realTargetName = tit->first.c_str();
      if(this->IgnoreThisTarget(realTargetName))
        {
        // Skip ignored targets
        continue;
        }
      //std::cout << "Found target: " << tit->first.c_str() << std::endl;
      cmOStringStream ostr;
      ostr << this->GraphNodePrefix << cnt++;
      this->TargetNamesNodes[realTargetName] = ostr.str();
      this->TargetPtrs[realTargetName] = &tit->second;
      }
    }

  return cnt;
}


int cmGraphVizWriter::collectAllExternalLibs(int cnt)
{
  // Ok, now find all the stuff we link to that is not in cmake
  for (std::vector<cmLocalGenerator*>::const_iterator lit =
                                                 this->LocalGenerators.begin();
       lit != this->LocalGenerators.end();
       ++ lit )
    {
    const cmTargets* targets = &((*lit)->GetMakefile()->GetTargets());
    for ( cmTargets::const_iterator tit = targets->begin();
          tit != targets->end();
          ++ tit )
      {
      const char* realTargetName = tit->first.c_str();
      if (this->IgnoreThisTarget(realTargetName))
        {
        // Skip ignored targets
        continue;
        }
      const cmTarget::LinkLibraryVectorType* ll =
                                     &(tit->second.GetOriginalLinkLibraries());
      for (cmTarget::LinkLibraryVectorType::const_iterator llit = ll->begin();
           llit != ll->end();
           ++ llit )
        {
        const char* libName = llit->first.c_str();
        if (this->IgnoreThisTarget(libName))
          {
          // Skip ignored targets
          continue;
          }

        std::map<cmStdString, const cmTarget*>::const_iterator tarIt =
                                                this->TargetPtrs.find(libName);
        if ( tarIt == this->TargetPtrs.end() )
          {
          cmOStringStream ostr;
          ostr << this->GraphNodePrefix << cnt++;
          this->TargetNamesNodes[libName] = ostr.str();
          this->TargetPtrs[libName] = NULL;
          //str << "    \"" << ostr.c_str() << "\" [ label=\"" << libName
          //<<  "\" shape=\"ellipse\"];" << std::endl;
          }
        }
      }
    }
   return cnt;
}


bool cmGraphVizWriter::IgnoreThisTarget(const char* name) const
{
  return (this->TargetsToIgnore.find(name) != this->TargetsToIgnore.end());
}
