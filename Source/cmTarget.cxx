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
#include "cmTarget.h"
#include "cmMakefile.h"

#include <map>
#include <set>


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
    // replace any variables
    std::string temps = *s;
    mf.ExpandVariablesInString(temps);
    // look for a srclist
    if (mf.GetSources().find(temps) != mf.GetSources().end())
      {
      const std::vector<cmSourceFile*> &clsList = 
        mf.GetSources().find(temps)->second;
      // if we ahave a limited build list, use it
      m_SourceFiles.insert(m_SourceFiles.end(), 
                           clsList.begin(), 
                           clsList.end());
      }
    // if one wasn't found then assume it is a single class
    else
      {
      // if the source file is already in the makefile, use it
      if (mf.GetSource(temps.c_str()))
        {
        m_SourceFiles.push_back(mf.GetSource(temps.c_str()));
        }
      // otherwise try to create it
      else
        {
        cmSourceFile file;
        file.SetIsAnAbstractClass(false);
        file.SetName(temps.c_str(), mf.GetCurrentDirectory(),
                     mf.GetSourceExtensions(),
                     mf.GetHeaderExtensions());
        m_SourceFiles.push_back(mf.AddSource(file));
        }
      }
    }

  // expand any link library variables whle we are at it
  LinkLibraries::iterator p = m_LinkLibraries.begin();
  for (;p != m_LinkLibraries.end(); ++p)
    {
    mf.ExpandVariablesInString(p->first);
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

void cmTarget::AddLinkDirectory(const char* d)
{
  // Make sure we don't add unnecessary search directories.
  if( std::find( m_LinkDirectories.begin(), m_LinkDirectories.end(), d )
      == m_LinkDirectories.end() )
    m_LinkDirectories.push_back( d );
}



void cmTarget::AddLinkLibrary(const std::string& lib, 
                              LinkLibraryType llt)
{
  m_LinkLibraries.push_back( std::pair<std::string, cmTarget::LinkLibraryType>(lib,llt) );
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

  m_LinkLibraries.push_back( std::pair<std::string, cmTarget::LinkLibraryType>(lib,llt) );

  if(llt != cmTarget::GENERAL)
    {
    std::string linkTypeName = lib;
    linkTypeName += "_LINK_TYPE";
    switch(llt)
      {
      case cmTarget::DEBUG:
        mf.AddCacheDefinition(linkTypeName.c_str(),
                              "debug", "Library is used for debug links only", 
                              cmCacheManager::STATIC);
        break;
      case cmTarget::OPTIMIZED:
        mf.AddCacheDefinition(linkTypeName.c_str(),
                              "optimized", "Library is used for debug links only", 
                              cmCacheManager::STATIC);
        break;
      case cmTarget::GENERAL: break;
      }
    }
  // Add the explicit dependency information for this target. This is
  // simply a set of libraries separated by ";". There should always
  // be a trailing ";". These library names are not canonical, in that
  // they may be "-framework x", "-ly", "/path/libz.a", etc.
  // only add depend information for library targets
  if(m_TargetType >= STATIC_LIBRARY && m_TargetType <= MODULE_LIBRARY)
    {
    std::string targetEntry = target;
    targetEntry += "_LIB_DEPENDS";
    std::string dependencies;
    const char* old_val = mf.GetDefinition( targetEntry.c_str() );
    if( old_val )
      {
      dependencies += old_val;
      }
    if( dependencies.find( lib ) == std::string::npos )
      {
      dependencies += lib;
      dependencies += ";";
      }
    mf.AddCacheDefinition( targetEntry.c_str(), dependencies.c_str(),
                           "Dependencies for the target", 
                           cmCacheManager::STATIC );
    }
  
}

bool cmTarget::HasCxx() const
{
  for(std::vector<cmSourceFile*>::const_iterator i =  m_SourceFiles.begin();
      i != m_SourceFiles.end(); ++i)
    {
    if((*i)->GetSourceExtension() != "c")
      {
      return true;
      }
    }
  return false;
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

  typedef std::vector< std::string > LinkLine;

  // The dependency map.
  DependencyMap dep_map;
  
  // Keeps track of which dependencies have already been emitted for a given
  // target. This could be via this function, or because they were already
  // satisfied on the original link line.
  DependencyMap satisfied;

  // If LIBRARY_OUTPUT_PATH is not set, then we must add search paths
  // for all the new libraries added by the dependency analysis.
  const char* libOutPath = mf.GetDefinition("LIBRARY_OUTPUT_PATH");
  bool addLibDirs = (libOutPath==0 || strcmp(libOutPath,"")==0);

  // 1. Determine the dependencies already satisfied by the original link
  // line.
  for(LinkLibraries::iterator lib = m_LinkLibraries.begin();
      lib != m_LinkLibraries.end(); ++lib)
    {
    for( LinkLibraries::iterator lib2 = lib;
      lib2 != m_LinkLibraries.end(); ++lib2)
      {
      satisfied[ lib->first ].insert( lib2->first );
      }
    }
  
  // 2. Build the explicit dependency map
  for(LinkLibraries::reverse_iterator lib = m_LinkLibraries.rbegin();
      lib != m_LinkLibraries.rend(); ++lib)
    {
    this->GatherDependencies( mf, lib->first, dep_map );
    }
  
  // 3. Create the new link line by simply emitting any dependencies that are
  // missing.  Start from the back and keep adding.
  
  std::set<cmStdString> done, visited;
  std::vector<std::string> newLinkLibraries;
  for(LinkLibraries::reverse_iterator lib = m_LinkLibraries.rbegin();
      lib != m_LinkLibraries.rend(); ++lib)
    {
    // skip zero size library entries, this may happen
    // if a variable expands to nothing.
    if (lib->first.size() == 0) continue;

    // Emit all the dependencies that are not already satisfied on the
    // original link line.
    if( dep_map.find(lib->first) != dep_map.end() ) // does it have dependencies?
      {
      const std::set<cmStdString>& dep_on = dep_map.find( lib->first )->second;
      std::set<cmStdString>::const_iterator i;
      for( i = dep_on.begin(); i != dep_on.end(); ++i )
        {
        if( satisfied[lib->first].end() == satisfied[lib->first].find( *i ) )
          {
          Emit( *i, dep_map, done, visited, newLinkLibraries );
          }
        }
      }
    }

  // 4. Add the new libraries to the link line.

  for( std::vector<std::string>::reverse_iterator k = newLinkLibraries.rbegin();
       k != newLinkLibraries.rend(); ++k )
    {
    if( addLibDirs )
      {
      const char* libpath = mf.GetDefinition( k->c_str() );
      if( libpath )
        {
        // Don't add a link directory that is already present.
        if(std::find(m_LinkDirectories.begin(),
                     m_LinkDirectories.end(), libpath) == m_LinkDirectories.end())
          {
          m_LinkDirectories.push_back(libpath);
          }
        }
      }
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
}




void cmTarget::Emit( const std::string& lib,
                     const DependencyMap& dep_map,
                     std::set<cmStdString>& emitted,
                     std::set<cmStdString>& visited,
                     std::vector<std::string>& link_line ) const
{
  // It's already been emitted
  if( emitted.find(lib) != emitted.end() )
    {
    return;
    }

  // If this library hasn't been visited before, then emit all its
  // dependencies before emitting the library itself. If it has been
  // visited before, then there is a dependency cycle. Just emit the
  // library itself, and let the recursion that got us here deal with
  // emitting the dependencies for the library.

  if( visited.insert(lib).second )
    {
    if( dep_map.find(lib) != dep_map.end() ) // does it have dependencies?
      {
      const std::set<cmStdString>& dep_on = dep_map.find( lib )->second;
      std::set<cmStdString>::const_iterator i;
      for( i = dep_on.begin(); i != dep_on.end(); ++i )
        {
        Emit( *i, dep_map, emitted, visited, link_line );
        }
      }
    }
  link_line.push_back( lib );
  emitted.insert(lib);
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
        dep_map[ lib ].insert( l );
        GatherDependencies( mf, l, dep_map );
        }
      start = end+1; // skip the ;
      end = depline.find( ";", start );
      }
    dep_map[lib].erase(lib); // cannot depend on itself
    }
}


// return true if lib1 depends on lib2

bool cmTarget::DependsOn( const std::string& lib1, const std::string& lib2,
                          const DependencyMap& dep_map,
                          std::set<cmStdString>& visited ) const
{
  if( !visited.insert( lib1 ).second )
    {
    return false; // already visited here
    }
  

  if( lib1 == lib2 )
    {
    return false;
    }

  if( dep_map.find(lib1) == dep_map.end() )
    {
    return false; // lib1 doesn't have any dependencies
    }

  const std::set<cmStdString>& dep_set = dep_map.find(lib1)->second;

  if( dep_set.end() != dep_set.find( lib2 )  )
    {
    return true; // lib1 doesn't directly depend on lib2.
    }

  // Do a recursive check: does lib1 depend on x which depends on lib2?
  for( std::set<cmStdString>::const_iterator itr = dep_set.begin();
       itr != dep_set.end(); ++itr )
    {
      if( this->DependsOn( *itr, lib2, dep_map, visited ) )
        {
        return true;
        }
    }

  return false;
}
