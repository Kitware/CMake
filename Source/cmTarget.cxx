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
#include "cmTarget.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmLocalGenerator.h"
#include "cmGlobalGenerator.h"
#include <map>
#include <set>
#include <queue>
#include <stdlib.h> // required for atof

//----------------------------------------------------------------------------
cmTarget::cmTarget()
{
  m_Makefile = 0;
  m_LinkLibrariesAnalyzed = false;
  m_LinkDirectoriesComputed = false;
}

void cmTarget::SetType(TargetType type, const char* name)
{
  m_Name = name;
  // only add dependency information for library targets
  m_TargetType = type;
  if(m_TargetType >= STATIC_LIBRARY && m_TargetType <= MODULE_LIBRARY) {
    m_RecordDependencies = true;
  } else {
    m_RecordDependencies = false;
  }
}


void cmTarget::TraceVSDependencies(std::string projFile, 
                                   cmMakefile *makefile)
{ 
  // get the classes from the source lists then add them to the groups
  std::vector<cmSourceFile*> & classes = this->GetSourceFiles();
  // use a deck to keep track of processed source files
  std::queue<std::string> srcFilesToProcess;
  std::set<cmStdString> srcFilesQueued;
  std::string name;
  std::vector<cmSourceFile*> newClasses;
  for(std::vector<cmSourceFile*>::const_iterator i = classes.begin(); 
      i != classes.end(); ++i)
    {
    name = (*i)->GetSourceName();
    if ((*i)->GetSourceExtension() != "rule")
      {
      name += ".";
      name += (*i)->GetSourceExtension();
      }
    srcFilesToProcess.push(name);
    srcFilesQueued.insert(name);
    // does this sourcefile have object depends on it?
    // If so then add them as well
    const char* additionalDeps = (*i)->GetProperty("OBJECT_DEPENDS");
    std::vector<std::string> depends = (*i)->GetDepends();
    if (additionalDeps || depends.size())
      {
      if(additionalDeps)
        {
        cmSystemTools::ExpandListArgument(additionalDeps, depends);
        }
      for(std::vector<std::string>::iterator id = depends.begin();
          id != depends.end(); ++id)
        {
        // if there is a custom rule to generate that dependency
        // then add it to the list
        cmSourceFile* outsf = 
          makefile->GetSourceFileWithOutput(id->c_str());
        // if a source file was found then add it
        if (outsf && 
            (std::find(classes.begin(),classes.end(),outsf) == classes.end()) &&
            (std::find(newClasses.begin(),newClasses.end(),outsf) == newClasses.end()))
          {
          // then add the source to this target and add it to the queue
          newClasses.push_back(outsf);
          name = outsf->GetSourceName();
          if (outsf->GetSourceExtension() != "rule")
            {
            name += ".";
            name += outsf->GetSourceExtension();
            }
          std::string temp = 
            cmSystemTools::GetFilenamePath(outsf->GetFullPath());
          temp += "/";
          temp += name;
          // if it hasn't been processed
          if (srcFilesQueued.find(temp) == srcFilesQueued.end())
            {
            srcFilesToProcess.push(temp);
            srcFilesQueued.insert(temp);
            }
          }
        }
      }
    }
  for(std::vector<cmSourceFile*>::const_iterator i = newClasses.begin(); 
      i != newClasses.end(); ++i)
    {
    classes.push_back(*i);
    }
  
  // add in the project file itself
  if (projFile.size())
    {
    srcFilesToProcess.push(projFile);
    srcFilesQueued.insert(projFile);
    }
  // add in the library depends for custom targets
  if (this->GetType() == cmTarget::UTILITY)
    {
    for (std::vector<cmCustomCommand>::iterator ic = 
           this->GetPostBuildCommands().begin();
         ic != this->GetPostBuildCommands().end(); ++ic)
      {
      cmCustomCommand &c = *ic;
      for (std::vector<std::string>::const_iterator i = c.GetDepends().begin();
           i != c.GetDepends().end(); ++i)
        {
        srcFilesToProcess.push(*i);
        srcFilesQueued.insert(*i);
        }
      }
    }
  while (!srcFilesToProcess.empty())
    {
    // is this source the output of a custom command
    cmSourceFile* outsf = 
      makefile->GetSourceFileWithOutput(srcFilesToProcess.front().c_str());
    if (outsf)
      {
      // is it not already in the target?
      if (std::find(classes.begin(),classes.end(),outsf) == classes.end())
        {
        // then add the source to this target and add it to the queue
        classes.push_back(outsf);
        name = outsf->GetSourceName();
        if (outsf->GetSourceExtension() != "rule")
          {
          name += ".";
          name += outsf->GetSourceExtension();
          }
        std::string temp = 
          cmSystemTools::GetFilenamePath(outsf->GetFullPath());
        temp += "/";
        temp += name;
        // if it hasn't been processed
        if (srcFilesQueued.find(temp) == srcFilesQueued.end())
          {
          srcFilesToProcess.push(temp);
          srcFilesQueued.insert(temp);
          }
        }
      // add its dependencies to the list to check
      unsigned int i;
      for (i = 0; i < outsf->GetCustomCommand()->GetDepends().size(); ++i)
        {
        std::string dep = cmSystemTools::GetFilenameName(
          outsf->GetCustomCommand()->GetDepends()[i]);
        if (cmSystemTools::GetFilenameLastExtension(dep) == ".exe")
          {
          dep = cmSystemTools::GetFilenameWithoutLastExtension(dep);
          }
        // watch for target dependencies,
        if(m_Makefile->GetLocalGenerator()->GetGlobalGenerator()->FindTarget(0, dep.c_str()))
          {
          // add the depend as a utility on the target
          this->AddUtility(dep.c_str());
          }
        else
          {
          if (srcFilesQueued.find(outsf->GetCustomCommand()->GetDepends()[i]) 
          == srcFilesQueued.end())
            {
            srcFilesToProcess.push(outsf->GetCustomCommand()->GetDepends()[i]);
            srcFilesQueued.insert(outsf->GetCustomCommand()->GetDepends()[i]);
            }
          }
        }
      }
    // finished with this SF move to the next
    srcFilesToProcess.pop();
    }
  // mark all custom commands in the targets list of source files as used.
  for(std::vector<cmSourceFile*>::iterator i =  m_SourceFiles.begin();
      i != m_SourceFiles.end(); ++i)
    {
    cmCustomCommand* cc = (*i)->GetCustomCommand();
    if(cc)
      {
      cc->Used();
      }
    }
}

void cmTarget::GenerateSourceFilesFromSourceLists( cmMakefile &mf)
{
  // this is only done for non install targets
  if ((this->m_TargetType == cmTarget::INSTALL_FILES)
      || (this->m_TargetType == cmTarget::INSTALL_PROGRAMS))
    {
    return;
    }

  // for each src lists add the classes
  for (std::vector<std::string>::const_iterator s = m_SourceLists.begin();
       s != m_SourceLists.end(); ++s)
    {
    int done = 0;
    // replace any variables
    std::string temps = *s;
    mf.ExpandVariablesInString(temps);

    // Next if one wasn't found then assume it is a single class
    // check to see if it is an existing source file
    if (!done)
      {
      cmSourceFile* sourceFile = mf.GetSource(temps.c_str());
      if ( sourceFile )
        {
        m_SourceFiles.push_back(sourceFile);
        done = 1;
        }
      }
      
    // if we still are not done, try to create the SourceFile structure
    if (!done)
      {
      cmSourceFile file;
      file.SetProperty("ABSTRACT","0");
      file.SetName(temps.c_str(), mf.GetCurrentDirectory(),
                   mf.GetSourceExtensions(),
                   mf.GetHeaderExtensions());
      m_SourceFiles.push_back(mf.AddSource(file));
      }
    }
  
  // expand any link library variables whle we are at it
  LinkLibraries::iterator p = m_LinkLibraries.begin();
  for (;p != m_LinkLibraries.end(); ++p)
    {
    mf.ExpandVariablesInString(p->first, true, true);
    }
}


void cmTarget::MergeLinkLibraries( cmMakefile& mf,
                                   const char *selfname,
                                   const LinkLibraries& libs )
{
  // Only add on libraries we haven't added on before.
  // Assumption: the global link libraries could only grow, never shrink
  LinkLibraries::const_iterator i = libs.begin();
  i += m_PrevLinkedLibraries.size();
  for( ; i != libs.end(); ++i )
    {
    // We call this so that the dependencies get written to the cache
    this->AddLinkLibrary( mf, selfname, i->first.c_str(), i->second );
    }
  m_PrevLinkedLibraries = libs;
}

//----------------------------------------------------------------------------
void cmTarget::AddLinkDirectory(const char* d)
{
  // Make sure we don't add unnecessary search directories.
  if(std::find(m_ExplicitLinkDirectories.begin(),
               m_ExplicitLinkDirectories.end(), d)
     == m_ExplicitLinkDirectories.end() )
    {
    m_ExplicitLinkDirectories.push_back( d );
    m_LinkDirectoriesComputed = false;
    }
}

//----------------------------------------------------------------------------
const std::vector<std::string>& cmTarget::GetLinkDirectories()
{
  // Make sure all library dependencies have been analyzed.
  if(!m_LinkLibrariesAnalyzed && !m_LinkLibraries.empty())
    {
    cmSystemTools::Error(
      "cmTarget::GetLinkDirectories called before cmTarget::AnalyzeLibDependencies on target ",
      m_Name.c_str());
    }

  // Make sure the complete set of link directories has been computed.
  if(!m_LinkDirectoriesComputed)
    {
    // Compute the full set of link directories including the
    // locations of targets that have been linked in.  Start with the
    // link directories given explicitly.
    m_LinkDirectories = m_ExplicitLinkDirectories;
    for(LinkLibraries::iterator ll = m_LinkLibraries.begin();
        ll != m_LinkLibraries.end(); ++ll)
      {
      // If this library is a CMake target then add its location as a
      // link directory.
      std::string lib = ll->first;
      cmTarget* tgt = 0;
      if(m_Makefile && m_Makefile->GetLocalGenerator() &&
         m_Makefile->GetLocalGenerator()->GetGlobalGenerator())
        {
        tgt = (m_Makefile->GetLocalGenerator()->GetGlobalGenerator()
               ->FindTarget(0, lib.c_str()));
        }
      if(tgt)
        {
        // Add the directory only if it is not already present.  This
        // is an N^2 algorithm for adding the directories, but N
        // should not get very big.
        const char* libpath = tgt->GetDirectory();
        if(std::find(m_LinkDirectories.begin(), m_LinkDirectories.end(),
                     libpath) == m_LinkDirectories.end())
          {
          m_LinkDirectories.push_back(libpath);
          }
        }
      }

    // The complete set of link directories has now been computed.
    m_LinkDirectoriesComputed = true;
    }

  // Return the complete set of link directories.
  return m_LinkDirectories;
}

void cmTarget::ClearDependencyInformation( cmMakefile& mf, const char* target )
{
  // Clear the dependencies. The cache variable must exist iff we are
  // recording dependency information for this target.
  std::string depname = target;
  depname += "_LIB_DEPENDS";
  if (m_RecordDependencies)
    {
    mf.AddCacheDefinition(depname.c_str(), "",
                          "Dependencies for target", cmCacheManager::STATIC);
    }
  else
    {
    if (mf.GetDefinition( depname.c_str() ))
      {
      std::string message = "Target ";
      message += target;
      message += " has dependency information when it shouldn't.\n";
      message += "Your cache is probably stale. Please remove the entry\n  ";
      message += depname;
      message += "\nfrom the cache.";
      cmSystemTools::Error( message.c_str() );  
      }
    }
}



void cmTarget::AddLinkLibrary(const std::string& lib, 
                              LinkLibraryType llt)
{
  this->AddFramework(lib.c_str(), llt);
  m_LinkLibraries.push_back( std::pair<std::string, cmTarget::LinkLibraryType>(lib,llt) );
}

bool cmTarget::AddFramework(const std::string& libname, LinkLibraryType llt)
{
  (void)llt; // TODO: What is this?
  if(cmSystemTools::IsPathToFramework(libname.c_str()))
    {
    std::string frameworkDir = libname;
    frameworkDir += "/../";
    frameworkDir = cmSystemTools::CollapseFullPath(frameworkDir.c_str());
    std::vector<std::string>::iterator i = 
      std::find(m_Frameworks.begin(),
                m_Frameworks.end(), frameworkDir);
    if(i == m_Frameworks.end())
      {
      m_Frameworks.push_back(frameworkDir);
      }
    return true;
    }
  return false;
}
void cmTarget::AddLinkLibrary(cmMakefile& mf,
                              const char *target, const char* lib, 
                              LinkLibraryType llt)
{
  // Never add a self dependency, even if the user asks for it.
  if(strcmp( target, lib ) == 0)
    {
    return;
    }
  this->AddFramework(lib, llt);
  m_LinkLibraries.push_back( std::pair<std::string, cmTarget::LinkLibraryType>(lib,llt) );

  if(llt != cmTarget::GENERAL)
    {
    // Store the library's link type in the cache.  If it is a
    // conflicting type then assume it is always used.  This is the
    // case when the user sets the cache entries for debug and
    // optimized versions of the library to the same value.
    std::string linkTypeName = lib;
    linkTypeName += "_LINK_TYPE";
    switch(llt)
      {
      case cmTarget::DEBUG:
        {
        const char* def = mf.GetDefinition(linkTypeName.c_str());
        if(!def || strcmp(def, "debug") == 0)
          {
          mf.AddCacheDefinition(linkTypeName.c_str(),
                                "debug", "Library is used for debug links only",
                                cmCacheManager::STATIC);
          }
        else
          {
          mf.AddCacheDefinition(linkTypeName.c_str(),
                                "general", "Library is used for both debug and optimized links",
                                cmCacheManager::STATIC);
          }
        }
        break;
      case cmTarget::OPTIMIZED:
        {
        const char* def = mf.GetDefinition(linkTypeName.c_str());
        if(!def || strcmp(def, "optimized") == 0)
          {
          mf.AddCacheDefinition(linkTypeName.c_str(),
                                "optimized", "Library is used for debug links only",
                                cmCacheManager::STATIC);
          }
        else
          {
          mf.AddCacheDefinition(linkTypeName.c_str(),
                                "general", "Library is used for both debug and optimized links",
                                cmCacheManager::STATIC);
          }
        }
        break;
      case cmTarget::GENERAL:
        break;
      }
    }
  // Add the explicit dependency information for this target. This is
  // simply a set of libraries separated by ";". There should always
  // be a trailing ";". These library names are not canonical, in that
  // they may be "-framework x", "-ly", "/path/libz.a", etc.
  // We shouldn't remove duplicates here because external libraries
  // may be purposefully duplicated to handle recursive dependencies,
  // and we removing one instance will break the link line. Duplicates
  // will be appropriately eliminated at emit time.
  if(m_RecordDependencies)
    {
    std::string targetEntry = target;
    targetEntry += "_LIB_DEPENDS";
    std::string dependencies;
    const char* old_val = mf.GetDefinition( targetEntry.c_str() );
    if( old_val )
      {
      dependencies += old_val;
      }
    dependencies += lib;
    dependencies += ";";
    mf.AddCacheDefinition( targetEntry.c_str(), dependencies.c_str(),
                           "Dependencies for the target", 
                           cmCacheManager::STATIC );
    }
  
}

void
cmTarget::AnalyzeLibDependencies( const cmMakefile& mf )
{
  // There are two key parts of the dependency analysis: (1)
  // determining the libraries in the link line, and (2) constructing
  // the dependency graph for those libraries.
  //
  // The latter is done using the cache entries that record the
  // dependencies of each library.
  //
  // The former is a more thorny issue, since it is not clear how to
  // determine if two libraries listed on the link line refer to the a
  // single library or not. For example, consider the link "libraries"
  //    /usr/lib/libtiff.so -ltiff 
  // Is this one library or two? The solution implemented here is the
  // simplest (and probably the only practical) one: two libraries are
  // the same if their "link strings" are identical. Thus, the two
  // libraries above are considered distinct. This also means that for
  // dependency analysis to be effective, the CMake user must specify
  // libraries build by his project without using any linker flags or
  // file extensions. That is,
  //    LINK_LIBRARIES( One Two )
  // instead of
  //    LINK_LIBRARIES( -lOne ${binarypath}/libTwo.a )
  // The former is probably what most users would do, but it never
  // hurts to document the assumptions. :-) Therefore, in the analysis
  // code, the "canonical name" of a library is simply its name as
  // given to a LINK_LIBRARIES command.
  //
  // Also, we will leave the original link line intact; we will just add any
  // dependencies that were missing.
  //
  // There is a problem with recursive external libraries
  // (i.e. libraries with no dependency information that are
  // recursively dependent). We must make sure that the we emit one of
  // the libraries twice to satisfy the recursion, but we shouldn't
  // emit it more times than necessary. In particular, we must make
  // sure that handling this improbable case doesn't cost us when
  // dealing with the common case of non-recursive libraries. The
  // solution is to assume that the recursion is satisfied at one node
  // of the dependency tree. To illustrate, assume libA and libB are
  // extrenal and mutually dependent. Suppose libX depends on
  // libA, and libY on libA and libX. Then
  //   TARGET_LINK_LIBRARIES( Y X A B A )
  //   TARGET_LINK_LIBRARIES( X A B A )
  //   TARGET_LINK_LIBRARIES( Exec Y )
  // would result in "-lY -lX -lA -lB -lA". This is the correct way to
  // specify the dependencies, since the mutual dependency of A and B
  // is resolved *every time libA is specified*.
  //
  // Something like
  //   TARGET_LINK_LIBRARIES( Y X A B A )
  //   TARGET_LINK_LIBRARIES( X A B )
  //   TARGET_LINK_LIBRARIES( Exec Y )
  // would result in "-lY -lX -lA -lB", and the mutual dependency
  // information is lost. This is because in some case (Y), the mutual
  // dependency of A and B is listed, while in another other case (X),
  // it is not. Depending on which line actually emits A, the mutual
  // dependency may or may not be on the final link line.  We can't
  // handle this pathalogical case cleanly without emitting extra
  // libraries for the normal cases. Besides, the dependency
  // information for X is wrong anyway: if we build an executable
  // depending on X alone, we would not have the mutual dependency on
  // A and B resolved.
  //
  // IMPROVEMENTS:
  // -- The current algorithm will not always pick the "optimal" link line
  //    when recursive dependencies are present. It will instead break the
  //    cycles at an aribtrary point. The majority of projects won't have
  //    cyclic dependencies, so this is probably not a big deal. Note that
  //    the link line is always correct, just not necessary optimal.

  typedef std::vector< std::string > LinkLine;

  // The dependency map.
  DependencyMap dep_map;

  // 1. Build the dependency graph
  //
  for(LinkLibraries::reverse_iterator lib = m_LinkLibraries.rbegin();
      lib != m_LinkLibraries.rend(); ++lib)
    {
    this->GatherDependencies( mf, lib->first, dep_map );
    }

  // 2. Remove any dependencies that are already satisfied in the original
  // link line.
  //
  for(LinkLibraries::iterator lib = m_LinkLibraries.begin();
      lib != m_LinkLibraries.end(); ++lib)
    {
    for( LinkLibraries::iterator lib2 = lib;
      lib2 != m_LinkLibraries.end(); ++lib2)
      {
      DeleteDependency( dep_map, lib->first, lib2->first );
      }
    }

  
  // 3. Create the new link line by simply emitting any dependencies that are
  // missing.  Start from the back and keep adding.
  //
  std::set<cmStdString> done, visited;
  std::vector<std::string> newLinkLibraries;
  for(LinkLibraries::reverse_iterator lib = m_LinkLibraries.rbegin();
      lib != m_LinkLibraries.rend(); ++lib)
    {
    // skip zero size library entries, this may happen
    // if a variable expands to nothing.
    if (lib->first.size() != 0)
      {
      Emit( lib->first, dep_map, done, visited, newLinkLibraries );
      }
    }

  // 4. Add the new libraries to the link line.
  //
  for( std::vector<std::string>::reverse_iterator k = newLinkLibraries.rbegin();
       k != newLinkLibraries.rend(); ++k )
    {
    std::string linkType = *k;
    linkType += "_LINK_TYPE";
    cmTarget::LinkLibraryType llt = cmTarget::GENERAL;
    const char* linkTypeString = mf.GetDefinition( linkType.c_str() );
    if(linkTypeString)
      {
      if(strcmp(linkTypeString, "debug") == 0)
        {
        llt = cmTarget::DEBUG;
        }
      if(strcmp(linkTypeString, "optimized") == 0)
        {
        llt = cmTarget::OPTIMIZED;
        }
      }
    m_LinkLibraries.push_back( std::make_pair(*k,llt) );
    }
  m_LinkLibrariesAnalyzed = true;
}


void cmTarget::InsertDependency( DependencyMap& depMap,
                                 const cmStdString& lib,
                                 const cmStdString& dep ) 
{
  depMap[lib].push_back(dep);
}

void cmTarget::DeleteDependency( DependencyMap& depMap,
                                 const cmStdString& lib,
                                 const cmStdString& dep ) 
{
  // Make sure there is an entry in the map for lib. If so, delete all
  // dependencies to dep. There may be repeated entries because of
  // external libraries that are specified multiple times.
  DependencyMap::iterator map_itr = depMap.find( lib );
  if( map_itr != depMap.end() )
    {
    DependencyList& depList = map_itr->second;
    DependencyList::iterator itr;
    while( (itr = std::find(depList.begin(), depList.end(), dep)) != depList.end() )
      {
      depList.erase( itr );
      }
    }
}

void cmTarget::Emit( const std::string& lib,
                     const DependencyMap& dep_map,
                     std::set<cmStdString>& emitted,
                     std::set<cmStdString>& visited,
                     std::vector<std::string>& link_line )
{
  // It's already been emitted
  if( emitted.find(lib) != emitted.end() )
    return;

  // Emit the dependencies only if this library node hasn't been
  // visited before. If it has, then we have a cycle. The recursion
  // that got us here should take care of everything.

  if( visited.insert(lib).second )
    {
    if( dep_map.find(lib) != dep_map.end() ) // does it have dependencies?
      {
      const DependencyList& dep_on = dep_map.find( lib )->second;
      DependencyList::const_reverse_iterator i;

      // To cater for recursive external libraries, we must emit
      // duplicates on this link line *unless* they were emitted by
      // some other node, in which case we assume that the recursion
      // was resolved then. We making the simplifying assumption that
      // any duplicates on a single link line are on purpose, and must
      // be preserved.

      // This variable will keep track of the libraries that were
      // emitted directory from the current node, and not from a
      // recursive call. This way, if we come across a library that
      // has already been emitted, we repeat it iff it has been
      // emitted here.
      std::set<cmStdString> emitted_here;
      for( i = dep_on.rbegin(); i != dep_on.rend(); ++i )
        {
        if( emitted_here.find(*i) != emitted_here.end() )
          {
          // a repeat. Must emit.
          emitted.insert(*i);
          link_line.push_back( *i );
          }
        else
          {
          // Emit only if no-one else has
          if( emitted.find(*i) == emitted.end() )
            {
            // emit dependencies
            Emit( *i, dep_map, emitted, visited, link_line );
            // emit self
            emitted.insert(*i);
            emitted_here.insert(*i);
            link_line.push_back( *i );
            }
          }
        }
      }
    }
}


void cmTarget::GatherDependencies( const cmMakefile& mf,
                                   const std::string& lib,
                                   DependencyMap& dep_map )
{
  // If the library is already in the dependency map, then it has
  // already been fully processed.
  if( dep_map.find(lib) != dep_map.end() )
    return;

  const char* deps = mf.GetDefinition( (lib+"_LIB_DEPENDS").c_str() );
  if( deps && strcmp(deps,"") != 0 )
    {
    // Make sure this library is in the map, even if it has an empty
    // set of dependencies. This distinguishes the case of explicitly
    // no dependencies with that of unspecified dependencies.
    dep_map[lib];

    // Parse the dependency information, which is simply a set of
    // libraries separated by ";". There is always a trailing ";".
    std::string depline = deps;
    std::string::size_type start = 0;
    std::string::size_type end;
    end = depline.find( ";", start );
    while( end != std::string::npos )
      {
      std::string l = depline.substr( start, end-start );
      if( l.size() != 0 )
        {
        InsertDependency( dep_map, lib, l );
        GatherDependencies( mf, l, dep_map );
        }
      start = end+1; // skip the ;
      end = depline.find( ";", start );
      }
    DeleteDependency( dep_map, lib, lib); // cannot depend on itself
    }
}


void cmTarget::SetProperty(const char* prop, const char* value)
{
  if (!prop)
    {
    return;
    }
  if (!value)
    {
    value = "NOTFOUND";
    }
  m_Properties[prop] = value;
}

const char* cmTarget::GetDirectory()
{
  switch( this->GetType() )
    {
    case cmTarget::STATIC_LIBRARY:
    case cmTarget::MODULE_LIBRARY:
    case cmTarget::SHARED_LIBRARY:
      m_Directory = m_Makefile->GetSafeDefinition("LIBRARY_OUTPUT_PATH");
      break;
    case cmTarget::EXECUTABLE:
      m_Directory = m_Makefile->GetSafeDefinition("EXECUTABLE_OUTPUT_PATH");
      break;
    default:
      return 0;
    }
  if(m_Directory.empty())
    {
    m_Directory = m_Makefile->GetStartOutputDirectory();
    }
  return m_Directory.c_str();
}

const char* cmTarget::GetLocation(const char* config)
{
  m_Location = this->GetDirectory();
  if(!m_Location.empty())
    {
    m_Location += "/";
    }
  const char* cfgid = m_Makefile->GetDefinition("CMAKE_CFG_INTDIR");
  if(cfgid && strcmp(cfgid, ".") != 0)
    {
    m_Location += cfgid;
    m_Location += "/";
    }
  m_Location += this->GetFullName(config);
  return m_Location.c_str();
}

void cmTarget::UpdateLocation()
{
  // make sure we have a makefile
  if (!m_Makefile)
    {
    return;
    }
  
  // set the LOCATION property of the target
  std::string target_location;
  switch( this->GetType() )
    {
    case cmTarget::STATIC_LIBRARY:
    case cmTarget::MODULE_LIBRARY:
    case cmTarget::SHARED_LIBRARY:
      target_location = 
        m_Makefile->GetSafeDefinition("LIBRARY_OUTPUT_PATH");
      break;
    case cmTarget::EXECUTABLE:
      target_location = 
        m_Makefile->GetSafeDefinition("EXECUTABLE_OUTPUT_PATH");
      break;
    default:
      return;
    }
  if ( target_location.size() == 0 )
    {
    target_location += m_Makefile->GetStartOutputDirectory();
    }
  if ( target_location.size() > 0 )
    {
    target_location += "/";
    }
  const char* cfgid = m_Makefile->GetDefinition("CMAKE_CFG_INTDIR");
  if ( cfgid && strcmp(cfgid, ".") != 0 )
    {
    target_location += cfgid;
    target_location += "/";
    }  
  target_location += this->GetFullName();
  this->SetProperty("LOCATION",target_location.c_str());
}

const char *cmTarget::GetProperty(const char* prop)
{
  // watch for special "computed" properties that are dependent on other
  // properties or variables, always recompute them
  if (!strcmp(prop,"LOCATION"))
    {
    this->UpdateLocation();
    }
  
  // the type property returns what type the target is
  if (!strcmp(prop,"TYPE"))
    {
    switch( this->GetType() )
      {
      case cmTarget::STATIC_LIBRARY:
        return "STATIC_LIBRARY";
        break;
      case cmTarget::MODULE_LIBRARY:
        return "MODULE_LIBRARY";
        break;
      case cmTarget::SHARED_LIBRARY:
        return "SHARED_LIBRARY";
        break;
      case cmTarget::EXECUTABLE:
        return "EXECUTABLE";
        break;
      case cmTarget::UTILITY:
        return "UTILITY";
        break;
      case cmTarget::INSTALL_FILES:
        return "INSTALL_FILES";
        break;
      case cmTarget::INSTALL_PROGRAMS:
        return "INSTALL_PROGRAMS";
        break;
      }
    return 0;
    }
      
  std::map<cmStdString,cmStdString>::const_iterator i = 
    m_Properties.find(prop);
  if (i != m_Properties.end())
    {
    return i->second.c_str();
    }
  return 0;
}

bool cmTarget::GetPropertyAsBool(const char* prop)
{
  std::map<cmStdString,cmStdString>::const_iterator i = 
    m_Properties.find(prop);
  if (i != m_Properties.end())
    {
    return cmSystemTools::IsOn(i->second.c_str());
    }
  return false;
}

const char* cmTarget::GetLinkerLanguage(cmGlobalGenerator* gg)
{
  if(this->GetProperty("HAS_CXX"))
    {
    const_cast<cmTarget*>(this)->SetProperty("LINKER_LANGUAGE", "CXX");
    }
  const char* linkerLang = this->GetProperty("LINKER_LANGUAGE");
  if(linkerLang)
    {
    return linkerLang;
    }
  std::set<cmStdString> languages;
  for(std::vector<cmSourceFile*>::const_iterator i = m_SourceFiles.begin();
      i != m_SourceFiles.end(); ++i)
    {
    const char* lang = 
      gg->GetLanguageFromExtension((*i)->GetSourceExtension().c_str());
    if(lang)
      {
      languages.insert(lang);
      }
    }
  if(languages.size() == 0)
    {
    return 0;
    }
  if(languages.size() == 1)
    {
    const_cast<cmTarget*>(this)->SetProperty("LINKER_LANGUAGE", languages.begin()->c_str());
    return this->GetProperty("LINKER_LANGUAGE");
    }
  const char* prefLang = 0;
  for(std::set<cmStdString>::const_iterator s = languages.begin(); 
      s != languages.end(); ++s)
    {
    const char* lpref = gg->GetLinkerPreference(s->c_str());
    if(lpref[0] == 'P')
      {
      if(prefLang && !(*s == prefLang))
        {
        std::string m = "Error Target: ";
        m += m_Name + " Contains more than one Prefered language: ";
        m += *s;
        m += " and ";
        m += prefLang;
        m += "\nYou must set the LINKER_LANGUAGE property for this target.";
        cmSystemTools::Error(m.c_str());
        }
      else
        {
        prefLang = s->c_str();
        }
      }
    }
  if(!prefLang)
    {
    prefLang = languages.begin()->c_str();
    }
  const_cast<cmTarget*>(this)->SetProperty("LINKER_LANGUAGE", prefLang);
  return this->GetProperty("LINKER_LANGUAGE");
}

const char* cmTarget::GetCreateRuleVariable()
{
  switch(this->GetType())
    { 
    case cmTarget::STATIC_LIBRARY:
      return "_CREATE_STATIC_LIBRARY";
    case cmTarget::SHARED_LIBRARY:
      return "_CREATE_SHARED_LIBRARY";
    case cmTarget::MODULE_LIBRARY:
      return "_CREATE_SHARED_MODULE";
    case cmTarget::EXECUTABLE:
      return "_LINK_EXECUTABLE";
    case cmTarget::UTILITY:
    case cmTarget::INSTALL_FILES:
    case cmTarget::INSTALL_PROGRAMS:
      break;
    }
  return "";
}

const char* cmTarget::GetSuffixVariable()
{
  return this->GetSuffixVariableInternal(this->GetType());
}

const char* cmTarget::GetSuffixVariableInternal(TargetType type)
{
  switch(type)
    {
    case cmTarget::STATIC_LIBRARY:
      return "CMAKE_STATIC_LIBRARY_SUFFIX";
    case cmTarget::SHARED_LIBRARY:
      return "CMAKE_SHARED_LIBRARY_SUFFIX";
    case cmTarget::MODULE_LIBRARY:
      return "CMAKE_SHARED_MODULE_SUFFIX";
    case cmTarget::EXECUTABLE:
      return "CMAKE_EXECUTABLE_SUFFIX";
    case cmTarget::UTILITY:
    case cmTarget::INSTALL_FILES:
    case cmTarget::INSTALL_PROGRAMS:
      break;
    }
  return "";
}


const char* cmTarget::GetPrefixVariable() 
{
  return this->GetPrefixVariableInternal(this->GetType());
}

const char* cmTarget::GetPrefixVariableInternal(TargetType type) 
{
  switch(type)
    {
    case cmTarget::STATIC_LIBRARY:
      return "CMAKE_STATIC_LIBRARY_PREFIX";
    case cmTarget::SHARED_LIBRARY:
      return "CMAKE_SHARED_LIBRARY_PREFIX";
    case cmTarget::MODULE_LIBRARY:
      return "CMAKE_SHARED_MODULE_PREFIX";
    case cmTarget::EXECUTABLE:
    case cmTarget::UTILITY:
    case cmTarget::INSTALL_FILES:
    case cmTarget::INSTALL_PROGRAMS:
      break;
    }
  return "";
}

//----------------------------------------------------------------------------
std::string cmTarget::GetFullName(const char* config)
{
  return this->GetFullNameInternal(this->GetType(), config);
}

//----------------------------------------------------------------------------
void cmTarget::GetFullName(std::string& prefix, std::string& base,
                           std::string& suffix, const char* config)
{
  this->GetFullNameInternal(this->GetType(), config, prefix, base, suffix);
}

//----------------------------------------------------------------------------
std::string cmTarget::GetFullPath(const char* config)
{
  // Start with the output directory for the target.
  std::string fpath = this->GetDirectory();

  // Add the configuration's subdirectory.
  m_Makefile->GetLocalGenerator()->GetGlobalGenerator()->
    AppendDirectoryForConfig(config, fpath);

  // Add the full name of the target.
  fpath += "/";
  fpath += this->GetFullName(config);
  return fpath;
}

//----------------------------------------------------------------------------
std::string cmTarget::GetFullNameInternal(TargetType type, const char* config)
{
  std::string prefix;
  std::string base;
  std::string suffix;
  this->GetFullNameInternal(type, config, prefix, base, suffix);
  return prefix+base+suffix;
}

//----------------------------------------------------------------------------
void cmTarget::GetFullNameInternal(TargetType type,
                                   const char* config,
                                   std::string& outPrefix,
                                   std::string& outBase,
                                   std::string& outSuffix)
{
  // Use just the target name for non-main target types.
  if(type != cmTarget::STATIC_LIBRARY &&
     type != cmTarget::SHARED_LIBRARY &&
     type != cmTarget::MODULE_LIBRARY &&
     type != cmTarget::EXECUTABLE)
    {
    outPrefix = "";
    outBase = this->GetName();
    outSuffix = "";
    return;
    }

  // Compute the full name for main target types.
  const char* targetPrefix = this->GetProperty("PREFIX");
  const char* targetSuffix = this->GetProperty("SUFFIX");
  const char* configPostfix = 0;
  if(config && *config && type != cmTarget::EXECUTABLE)
    {
    std::string configVar = "CMAKE_";
    configVar += config;
    configVar += "_POSTFIX";
    configVar = cmSystemTools::UpperCase(configVar);
    configPostfix = m_Makefile->GetDefinition(configVar.c_str());
    }
  const char* prefixVar = this->GetPrefixVariableInternal(type);
  const char* suffixVar = this->GetSuffixVariableInternal(type);
  const char* ll =
    this->GetLinkerLanguage(
      m_Makefile->GetLocalGenerator()->GetGlobalGenerator());
  // first try language specific suffix
  if(ll)
    {
    if(!targetSuffix && suffixVar && *suffixVar)
      {
      std::string langSuff = suffixVar + std::string("_") + ll;
      targetSuffix = m_Makefile->GetDefinition(langSuff.c_str());
      }
    if(!targetPrefix && prefixVar && *prefixVar)
      {
      std::string langPrefix = prefixVar + std::string("_") + ll;
      targetPrefix = m_Makefile->GetDefinition(langPrefix.c_str());
      }
    }

  // if there is no prefix on the target use the cmake definition
  if(!targetPrefix && prefixVar)
    {
    targetPrefix = m_Makefile->GetSafeDefinition(prefixVar);
    }
  // if there is no suffix on the target use the cmake definition
  if(!targetSuffix && suffixVar)
    {
    targetSuffix = m_Makefile->GetSafeDefinition(suffixVar);
    }

  // Begin the final name with the prefix.
  outPrefix = targetPrefix?targetPrefix:"";

  // Append the target name or property-specified name.  Support this
  // only for executable targets.
  const char* outname = this->GetProperty("OUTPUT_NAME");
  if(outname && type == cmTarget::EXECUTABLE)
    {
    outBase = outname;
    }
  else
    {
    outBase = this->GetName();
    }

  // Append the per-configuration postfix.
  outBase += configPostfix?configPostfix:"";

  // Append the suffix.
  outSuffix = targetSuffix?targetSuffix:"";
}

void cmTarget::GetLibraryNames(std::string& name,
                               std::string& soName,
                               std::string& realName,
                               const char* config)
{
  // Get the names based on the real type of the library.
  this->GetLibraryNamesInternal(name, soName, realName, this->GetType(),
                                config);
}

void cmTarget::GetLibraryCleanNames(std::string& staticName,
                                    std::string& sharedName,
                                    std::string& sharedSOName,
                                    std::string& sharedRealName,
                                    const char* config)
{
  // Get the name as if this were a static library.
  std::string soName;
  std::string realName;
  this->GetLibraryNamesInternal(staticName, soName, realName,
                                cmTarget::STATIC_LIBRARY, config);

  // Get the names as if this were a shared library.
  if(this->GetType() == cmTarget::STATIC_LIBRARY)
    {
    // Since the real type is static then the user either specified
    // STATIC or did not specify a type.  In the former case the
    // shared library will never be present.  In the latter case the
    // type will never be MODULE.  Either way the only names that
    // might have to be cleaned are the shared library names.
    this->GetLibraryNamesInternal(sharedName, sharedSOName,
                                  sharedRealName, cmTarget::SHARED_LIBRARY,
                                  config);
    }
  else
    {
    // Use the name of the real type of the library (shared or module).
    this->GetLibraryNamesInternal(sharedName, sharedSOName,
                                  sharedRealName, this->GetType(),
                                  config);
    }
}

void cmTarget::GetLibraryNamesInternal(std::string& name,
                                       std::string& soName,
                                       std::string& realName,
                                       TargetType type,
                                       const char* config)
{
  // Construct the name of the soname flag variable for this language.
  const char* ll =
    this->GetLinkerLanguage(
      m_Makefile->GetLocalGenerator()->GetGlobalGenerator());
  std::string sonameFlag = "CMAKE_SHARED_LIBRARY_SONAME";
  if(ll)
    {
    sonameFlag += "_";
    sonameFlag += ll;
    }
  sonameFlag += "_FLAG";

  // Check for library version properties.
  const char* version = this->GetProperty("VERSION");
  const char* soversion = this->GetProperty("SOVERSION");
  if((type != cmTarget::SHARED_LIBRARY && type != cmTarget::MODULE_LIBRARY) ||
     !m_Makefile->GetDefinition(sonameFlag.c_str()))
    {
    // Versioning is supported only for shared libraries and modules,
    // and then only when the platform supports an soname flag.
    version = 0;
    soversion = 0;
    }
  if(version && !soversion)
    {
    // The soversion must be set if the library version is set.  Use
    // the library version as the soversion.
    soversion = version;
    }

  // The library name.
  name = this->GetFullNameInternal(type, config);

  // The library's soname.
  soName = name;
  if(soversion)
    {
    soName += ".";
    soName += soversion;
    }

  // The library's real name on disk.
  realName = name;
  if(version)
    {
    realName += ".";
    realName += version;
    }
  else if(soversion)
    {
    realName += ".";
    realName += soversion;
    }
}

void cmTarget::GetExecutableNames(std::string& name,
                                  std::string& realName,
                                  const char* config)
{
  // Get the names based on the real type of the executable.
  this->GetExecutableNamesInternal(name, realName, this->GetType(), config);
}

void cmTarget::GetExecutableCleanNames(std::string& name,
                                       std::string& realName,
                                       const char* config)
{
  // Get the name and versioned name of this executable.
  this->GetExecutableNamesInternal(name, realName, cmTarget::EXECUTABLE,
                                   config);
}

void cmTarget::GetExecutableNamesInternal(std::string& name,
                                          std::string& realName,
                                          TargetType type,
                                          const char* config)
{
  // This versioning is supported only for executables and then only
  // when the platform supports symbolic links.
#if defined(_WIN32) && !defined(__CYGWIN__)
  const char* version = 0;
#else
  // Check for executable version properties.
  const char* version = this->GetProperty("VERSION");
  if(type != cmTarget::EXECUTABLE)
    {
    version = 0;
    }
#endif

  // The executable name.
  name = this->GetFullNameInternal(type, config);

  // The executable's real name on disk.
  realName = name;
  if(version)
    {
    realName += "-";
    realName += version;
    }
}
