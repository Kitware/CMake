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
const char* cmTarget::TargetTypeNames[] = {
  "EXECUTABLE", "STATIC_LIBRARY",
  "SHARED_LIBRARY", "MODULE_LIBRARY", "UTILITY", "GLOBAL_TARGET",
  "INSTALL_FILES", "INSTALL_PROGRAMS", "INSTALL_DIRECTORY"
};

//----------------------------------------------------------------------------
cmTarget::cmTarget()
{
  this->Makefile = 0;
  this->LinkLibrariesAnalyzed = false;
  this->LinkDirectoriesComputed = false;
  this->HaveInstallRule = false;
}

void cmTarget::SetType(TargetType type, const char* name)
{
  this->Name = name;
  // only add dependency information for library targets
  this->TargetTypeValue = type;
  if(this->TargetTypeValue >= STATIC_LIBRARY 
     && this->TargetTypeValue <= MODULE_LIBRARY) 
    {
    this->RecordDependencies = true;
    } 
  else 
    {
    this->RecordDependencies = false;
    }
}

//----------------------------------------------------------------------------
void cmTarget::SetMakefile(cmMakefile* mf)
{
  // Set our makefile.
  this->Makefile = mf;

  // Setup default property values.
  this->SetPropertyDefault("INSTALL_NAME_DIR", "");
  this->SetPropertyDefault("INSTALL_RPATH", "");
  this->SetPropertyDefault("INSTALL_RPATH_USE_LINK_PATH", "OFF");
  this->SetPropertyDefault("SKIP_BUILD_RPATH", "OFF");
  this->SetPropertyDefault("BUILD_WITH_INSTALL_RPATH", "OFF");

  // Collect the set of configuration types.
  std::vector<std::string> configNames;
  if(const char* configurationTypes =
     mf->GetDefinition("CMAKE_CONFIGURATION_TYPES"))
    {
    cmSystemTools::ExpandListArgument(configurationTypes, configNames);
    }
  else if(const char* buildType = mf->GetDefinition("CMAKE_BUILD_TYPE"))
    {
    if(*buildType)
      {
      configNames.push_back(buildType);
      }
    }

  // Setup per-configuration property default values.
  for(std::vector<std::string>::iterator ci = configNames.begin();
      ci != configNames.end(); ++ci)
    {
    // Initialize per-configuration name postfix property from the
    // variable only for non-executable targets.  This preserves
    // compatibility with previous CMake versions in which executables
    // did not support this variable.  Projects may still specify the
    // property directly.  TODO: Make this depend on backwards
    // compatibility setting.
    if(this->TargetTypeValue != cmTarget::EXECUTABLE)
      {
      std::string property = cmSystemTools::UpperCase(*ci);
      property += "_POSTFIX";
      this->SetPropertyDefault(property.c_str(), 0);
      }
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
            (std::find(classes.begin(),classes.end(),outsf) == classes.end())
            &&
            (std::find(newClasses.begin(),newClasses.end(),outsf)
             == newClasses.end()))
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
  if (this->GetType() == cmTarget::UTILITY || 
      this->GetType() == cmTarget::GLOBAL_TARGET)
    {
    for (std::vector<cmCustomCommand>::iterator ic = 
           this->GetPostBuildCommands().begin();
         ic != this->GetPostBuildCommands().end(); ++ic)
      {
      cmCustomCommand &c = *ic;
      for (std::vector<std::string>::const_iterator i 
             = c.GetDepends().begin(); i != c.GetDepends().end(); ++i)
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
        const std::string& fullName 
          = outsf->GetCustomCommand()->GetDepends()[i];
        std::string dep = cmSystemTools::GetFilenameName(fullName);
        if (cmSystemTools::GetFilenameLastExtension(dep) == ".exe")
          {
          dep = cmSystemTools::GetFilenameWithoutLastExtension(dep);
          }
        bool isUtility = false;
        // see if we can find a target with this name
        cmTarget* t =  this->Makefile->GetLocalGenerator()->
          GetGlobalGenerator()->FindTarget(0, dep.c_str());
        if(t)
          {
          // if we find the target and the dep was given as a full
          // path, then make sure it was not a full path to something
          // else, and the fact that the name matched a target was 
          // just a coincident 
          if(cmSystemTools::FileIsFullPath(fullName.c_str()))
            {
            std::string tLocation = t->GetLocation(0);
            tLocation = cmSystemTools::GetFilenamePath(tLocation);
            std::string depLocation = cmSystemTools::GetFilenamePath(
              std::string(fullName));
            depLocation = cmSystemTools::CollapseFullPath(depLocation.c_str());
            tLocation = cmSystemTools::CollapseFullPath(tLocation.c_str());
            if(depLocation == tLocation)
              {
              isUtility = true;
              }
            }
          // if it was not a full path then it must be a target
          else
            {
            isUtility = true;
            }
          }
        if(isUtility)
          {
          // add the depend as a utility on the target
          this->AddUtility(dep.c_str());
          }
        else
          {
          if (srcFilesQueued.find(outsf->GetCustomCommand()->GetDepends()[i]) 
          == srcFilesQueued.end())
            {
            srcFilesToProcess.push
              (outsf->GetCustomCommand()->GetDepends()[i]);
            srcFilesQueued.insert(outsf->GetCustomCommand()->GetDepends()[i]);
            }
          }
        }
      }
    // finished with this SF move to the next
    srcFilesToProcess.pop();
    }
  // mark all custom commands in the targets list of source files as used.
  for(std::vector<cmSourceFile*>::iterator i =  this->SourceFiles.begin();
      i != this->SourceFiles.end(); ++i)
    {
    cmCustomCommand* cc = (*i)->GetCustomCommand();
    if(cc)
      {
      cc->SetUsed();
      }
    }
}

void cmTarget::GenerateSourceFilesFromSourceLists( cmMakefile &mf)
{
  // this is only done for non install targets
  if ((this->TargetTypeValue == cmTarget::INSTALL_FILES)
      || (this->TargetTypeValue == cmTarget::INSTALL_PROGRAMS))
    {
    return;
    }

  // for each src lists add the classes
  for (std::vector<std::string>::const_iterator s = this->SourceLists.begin();
       s != this->SourceLists.end(); ++s)
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
        this->SourceFiles.push_back(sourceFile);
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
                   mf.GetHeaderExtensions(), this->Name.c_str());
      this->SourceFiles.push_back(mf.AddSource(file));
      }
    }
  
  // expand any link library variables whle we are at it
  LinkLibraryVectorType::iterator p = this->LinkLibraries.begin();
  for (;p != this->LinkLibraries.end(); ++p)
    {
    mf.ExpandVariablesInString(p->first, true, true);
    }
}


void cmTarget::MergeLinkLibraries( cmMakefile& mf,
                                   const char *selfname,
                                   const LinkLibraryVectorType& libs )
{
  // Only add on libraries we haven't added on before.
  // Assumption: the global link libraries could only grow, never shrink
  LinkLibraryVectorType::const_iterator i = libs.begin();
  i += this->PrevLinkedLibraries.size();
  for( ; i != libs.end(); ++i )
    {
    // We call this so that the dependencies get written to the cache
    this->AddLinkLibrary( mf, selfname, i->first.c_str(), i->second );
    }
  this->PrevLinkedLibraries = libs;
}

//----------------------------------------------------------------------------
void cmTarget::AddLinkDirectory(const char* d)
{
  // Make sure we don't add unnecessary search directories.
  if(std::find(this->ExplicitLinkDirectories.begin(),
               this->ExplicitLinkDirectories.end(), d)
     == this->ExplicitLinkDirectories.end() )
    {
    this->ExplicitLinkDirectories.push_back( d );
    this->LinkDirectoriesComputed = false;
    }
}

//----------------------------------------------------------------------------
const std::vector<std::string>& cmTarget::GetLinkDirectories()
{
  // Make sure all library dependencies have been analyzed.
  if(!this->LinkLibrariesAnalyzed && !this->LinkLibraries.empty())
    {
    cmSystemTools::Error(
      "cmTarget::GetLinkDirectories called before "
      "cmTarget::AnalyzeLibDependencies on target ",
      this->Name.c_str());
    }

  // Make sure the complete set of link directories has been computed.
  if(!this->LinkDirectoriesComputed)
    {
    // Compute the full set of link directories including the
    // locations of targets that have been linked in.  Start with the
    // link directories given explicitly.
    this->LinkDirectories = this->ExplicitLinkDirectories;
    for(LinkLibraryVectorType::iterator ll = this->LinkLibraries.begin();
        ll != this->LinkLibraries.end(); ++ll)
      {
      // If this library is a CMake target then add its location as a
      // link directory.
      std::string lib = ll->first;
      cmTarget* tgt = 0;
      if(this->Makefile && this->Makefile->GetLocalGenerator() &&
         this->Makefile->GetLocalGenerator()->GetGlobalGenerator())
        {
        tgt = (this->Makefile->GetLocalGenerator()->GetGlobalGenerator()
               ->FindTarget(0, lib.c_str()));
        }
      if(tgt)
        {
        // Add the directory only if it is not already present.  This
        // is an N^2 algorithm for adding the directories, but N
        // should not get very big.
        const char* libpath = tgt->GetDirectory();
        if(std::find(this->LinkDirectories.begin(), 
                     this->LinkDirectories.end(),
                     libpath) == this->LinkDirectories.end())
          {
          this->LinkDirectories.push_back(libpath);
          }
        }
      }

    // The complete set of link directories has now been computed.
    this->LinkDirectoriesComputed = true;
    }

  // Return the complete set of link directories.
  return this->LinkDirectories;
}

void cmTarget::ClearDependencyInformation( cmMakefile& mf, 
                                           const char* target )
{
  // Clear the dependencies. The cache variable must exist iff we are
  // recording dependency information for this target.
  std::string depname = target;
  depname += "_LIB_DEPENDS";
  if (this->RecordDependencies)
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
  this->LinkLibraries.push_back( std::pair<std::string, 
                                 cmTarget::LinkLibraryType>(lib,llt) );
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
      std::find(this->Frameworks.begin(),
                this->Frameworks.end(), frameworkDir);
    if(i == this->Frameworks.end())
      {
      this->Frameworks.push_back(frameworkDir);
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
  this->LinkLibraries.push_back( std::pair<std::string, 
                                 cmTarget::LinkLibraryType>(lib,llt) );

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
                                "debug", 
                                "Library is used for debug links only",
                                cmCacheManager::STATIC);
          }
        else
          {
          mf.AddCacheDefinition
            (linkTypeName.c_str(), "general", 
             "Library is used for both debug and optimized links",
             cmCacheManager::STATIC);
          }
        }
        break;
      case cmTarget::OPTIMIZED:
        {
        const char* def = mf.GetDefinition(linkTypeName.c_str());
        if(!def || strcmp(def, "optimized") == 0)
          {
          mf.AddCacheDefinition
            (linkTypeName.c_str(), "optimized", 
             "Library is used for debug links only",
             cmCacheManager::STATIC);
          }
        else
          {
          mf.AddCacheDefinition
            (linkTypeName.c_str(), "general", 
             "Library is used for both debug and optimized links",
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
  if(this->RecordDependencies)
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

  if ( this->OriginalLinkLibraries.size() == 0 )
    {
    this->OriginalLinkLibraries = this->LinkLibraries;
    }

  // 1. Build the dependency graph
  //
  for(LinkLibraryVectorType::reverse_iterator lib 
        = this->LinkLibraries.rbegin();
      lib != this->LinkLibraries.rend(); ++lib)
    {
    this->GatherDependencies( mf, lib->first, dep_map );
    }

  // 2. Remove any dependencies that are already satisfied in the original
  // link line.
  //
  for(LinkLibraryVectorType::iterator lib = this->LinkLibraries.begin();
      lib != this->LinkLibraries.end(); ++lib)
    {
    for( LinkLibraryVectorType::iterator lib2 = lib;
      lib2 != this->LinkLibraries.end(); ++lib2)
      {
      DeleteDependency( dep_map, lib->first, lib2->first );
      }
    }

  
  // 3. Create the new link line by simply emitting any dependencies that are
  // missing.  Start from the back and keep adding.
  //
  std::set<cmStdString> done, visited;
  std::vector<std::string> newLinkLibraries;
  for(LinkLibraryVectorType::reverse_iterator lib = 
        this->LinkLibraries.rbegin();
      lib != this->LinkLibraries.rend(); ++lib)
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
  for( std::vector<std::string>::reverse_iterator k = 
         newLinkLibraries.rbegin();
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
    this->LinkLibraries.push_back( std::make_pair(*k,llt) );
    }
  this->LinkLibrariesAnalyzed = true;
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
    while( (itr = std::find(depList.begin(), depList.end(), dep)) != 
           depList.end() )
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
    {
    return;
    }

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
    {
    return;
    }

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
  this->Properties[prop] = value;
}

const char* cmTarget::GetDirectory(const char* config)
{
  switch( this->GetType() )
    {
    case cmTarget::STATIC_LIBRARY:
    case cmTarget::MODULE_LIBRARY:
    case cmTarget::SHARED_LIBRARY:
      this->Directory = 
        this->Makefile->GetSafeDefinition("LIBRARY_OUTPUT_PATH");
      break;
    case cmTarget::EXECUTABLE:
      this->Directory = 
        this->Makefile->GetSafeDefinition("EXECUTABLE_OUTPUT_PATH");
      break;
    default:
      this->Directory = this->Makefile->GetStartOutputDirectory();
      break;
    }
  if(this->Directory.empty())
    {
    this->Directory = this->Makefile->GetStartOutputDirectory();
    }
  // if LIBRARY_OUTPUT_PATH or EXECUTABLE_OUTPUT_PATH was relative
  // then make them full paths because this directory MUST 
  // be a full path or things will not work!!!
  if(!cmSystemTools::FileIsFullPath(this->Directory.c_str()))
    {
    this->Directory = this->Makefile->GetCurrentOutputDirectory() + 
      std::string("/") + this->Directory;
    }
  if(config)
    {
    // Add the configuration's subdirectory.
    this->Makefile->GetLocalGenerator()->GetGlobalGenerator()->
      AppendDirectoryForConfig("/", config, "", this->Directory);
    }
  return this->Directory.c_str();
}

const char* cmTarget::GetLocation(const char* config)
{
  this->Location = this->GetDirectory(config);
  if(!this->Location.empty())
    {
    this->Location += "/";
    }
  if(!config)
    {
     // No specific configuration was given so it will not appear on
     // the result of GetDirectory.  Add a name here to be replaced at
     // build time.
    const char* cfgid = this->Makefile->GetDefinition("CMAKE_CFG_INTDIR");
    if(cfgid && strcmp(cfgid, ".") != 0)
      {
      this->Location += cfgid;
      this->Location += "/";
       }
    }
  this->Location += this->GetFullName(config, false);
  return this->Location.c_str();
}

//----------------------------------------------------------------------------
void cmTarget::GetTargetVersion(int& major, int& minor)
{
  // Set the default values.
  major = 0;
  minor = 0;

  // Look for a VERSION property.
  if(const char* version = this->GetProperty("VERSION"))
    {
    // Try to parse the version number and store the results that were
    // successfully parsed.
    int parsed_major;
    int parsed_minor;
    switch(sscanf(version, "%d.%d", &parsed_major, &parsed_minor))
      {
      case 2: minor = parsed_minor; // no break!
      case 1: major = parsed_major; // no break!
      default: break;
      }
    }
}

const char *cmTarget::GetProperty(const char* prop)
{
  // watch for special "computed" properties that are dependent on other
  // properties or variables, always recompute them
  if (!strcmp(prop,"LOCATION"))
    {
    // Set the LOCATION property of the target.  Note that this cannot take
    // into account the per-configuration name of the target because the
    // configuration type may not be known at CMake time.  We should
    // deprecate this feature and instead support transforming an executable
    // target name given as the command part of custom commands into the
    // proper path at build time.  Alternatively we could put environment
    // variable settings in all custom commands that hold the name of the
    // target for each configuration and then give a reference to the
    // variable in the location.
    this->SetProperty("LOCATION", this->GetLocation(0));
    }
  
  // Per-configuration location can be computed.
  int len = static_cast<int>(strlen(prop));
  if(len > 9 && strcmp(prop+len-9, "_LOCATION") == 0)
    {
    std::string configName(prop, len-9);
    this->SetProperty(prop, this->GetLocation(configName.c_str()));
    }

  // the type property returns what type the target is
  if (!strcmp(prop,"TYPE"))
    {
    switch( this->GetType() )
      {
      case cmTarget::STATIC_LIBRARY:
        return "STATIC_LIBRARY";
        // break; /* unreachable */
      case cmTarget::MODULE_LIBRARY:
        return "MODULE_LIBRARY";
        // break; /* unreachable */
      case cmTarget::SHARED_LIBRARY:
        return "SHARED_LIBRARY";
        // break; /* unreachable */
      case cmTarget::EXECUTABLE:
        return "EXECUTABLE";
        // break; /* unreachable */
      case cmTarget::UTILITY:
        return "UTILITY";
        // break; /* unreachable */
      case cmTarget::GLOBAL_TARGET:
        return "GLOBAL_TARGET";
        // break; /* unreachable */
      case cmTarget::INSTALL_FILES:
        return "INSTALL_FILES";
        // break; /* unreachable */
      case cmTarget::INSTALL_PROGRAMS:
        return "INSTALL_PROGRAMS";
        // break; /* unreachable */
      case cmTarget::INSTALL_DIRECTORY:
        return "INSTALL_DIRECTORY";
        // break; /* unreachable */
      }
    return 0;
    }
      
  std::map<cmStdString,cmStdString>::const_iterator i = 
    this->Properties.find(prop);
  if (i != this->Properties.end())
    {
    return i->second.c_str();
    }
  return 0;
}

bool cmTarget::GetPropertyAsBool(const char* prop)
{
  std::map<cmStdString,cmStdString>::const_iterator i = 
    this->Properties.find(prop);
  if (i != this->Properties.end())
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
  for(std::vector<cmSourceFile*>::const_iterator i 
        = this->SourceFiles.begin();
      i != this->SourceFiles.end(); ++i)
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
    const_cast<cmTarget*>(this)->SetProperty("LINKER_LANGUAGE", 
                                             languages.begin()->c_str());
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
        m += this->Name + " Contains more than one Prefered language: ";
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
    case cmTarget::GLOBAL_TARGET:
    case cmTarget::INSTALL_FILES:
    case cmTarget::INSTALL_PROGRAMS:
    case cmTarget::INSTALL_DIRECTORY:
      break;
    }
  return "";
}

const char* cmTarget::GetSuffixVariableInternal(TargetType type,
                                                bool implib)
{
  switch(type)
    {
    case cmTarget::STATIC_LIBRARY:
      return "CMAKE_STATIC_LIBRARY_SUFFIX";
    case cmTarget::SHARED_LIBRARY:
      return (implib
              ? "CMAKE_IMPORT_LIBRARY_SUFFIX"
              : "CMAKE_SHARED_LIBRARY_SUFFIX");
    case cmTarget::MODULE_LIBRARY:
      return "CMAKE_SHARED_MODULE_SUFFIX";
    case cmTarget::EXECUTABLE:
      return "CMAKE_EXECUTABLE_SUFFIX";
    case cmTarget::UTILITY:
    case cmTarget::GLOBAL_TARGET:
    case cmTarget::INSTALL_FILES:
    case cmTarget::INSTALL_PROGRAMS:
    case cmTarget::INSTALL_DIRECTORY:
      break;
    }
  return "";
}


const char* cmTarget::GetPrefixVariableInternal(TargetType type,
                                                bool implib)
{
  switch(type)
    {
    case cmTarget::STATIC_LIBRARY:
      return "CMAKE_STATIC_LIBRARY_PREFIX";
    case cmTarget::SHARED_LIBRARY:
      return (implib
              ? "CMAKE_IMPORT_LIBRARY_PREFIX"
              : "CMAKE_SHARED_LIBRARY_PREFIX");
    case cmTarget::MODULE_LIBRARY:
      return "CMAKE_SHARED_MODULE_PREFIX";
    case cmTarget::EXECUTABLE:
    case cmTarget::UTILITY:
    case cmTarget::GLOBAL_TARGET:
    case cmTarget::INSTALL_FILES:
    case cmTarget::INSTALL_PROGRAMS:
    case cmTarget::INSTALL_DIRECTORY:
      break;
    }
  return "";
}

//----------------------------------------------------------------------------
std::string cmTarget::GetPDBName(const char* config)
{
  std::string prefix;
  std::string base;
  std::string suffix;
  this->GetFullNameInternal(this->GetType(), config, false,
                            prefix, base, suffix);
  return prefix+base+".pdb";
}

//----------------------------------------------------------------------------
std::string cmTarget::GetFullName(const char* config, bool implib)
{
  return this->GetFullNameInternal(this->GetType(), config, implib);
}

//----------------------------------------------------------------------------
void cmTarget::GetFullName(std::string& prefix, std::string& base,
                           std::string& suffix, const char* config,
                           bool implib)
{
  this->GetFullNameInternal(this->GetType(), config, implib,
                            prefix, base, suffix);
}

//----------------------------------------------------------------------------
std::string cmTarget::GetFullPath(const char* config, bool implib)
{
  // Start with the output directory for the target.
  std::string fpath = this->GetDirectory(config);
  fpath += "/";

  // Add the full name of the target.
  fpath += this->GetFullName(config, implib);
  return fpath;
}

//----------------------------------------------------------------------------
std::string cmTarget::GetFullNameInternal(TargetType type, const char* config,
                                          bool implib)
{
  std::string prefix;
  std::string base;
  std::string suffix;
  this->GetFullNameInternal(type, config, implib, prefix, base, suffix);
  return prefix+base+suffix;
}

//----------------------------------------------------------------------------
void cmTarget::GetFullNameInternal(TargetType type,
                                   const char* config,
                                   bool implib,
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

  // Return an empty name for the import library if this platform
  // does not support import libraries.
  if(implib &&
     !this->Makefile->GetDefinition("CMAKE_IMPORT_LIBRARY_SUFFIX"))
    {
    outPrefix = "";
    outBase = "";
    outSuffix = "";
    return;
    }

  // The implib option is only allowed for shared libraries.
  if(type != cmTarget::SHARED_LIBRARY)
    {
    implib = false;
    }

  // Compute the full name for main target types.
  const char* targetPrefix = (implib
                              ? this->GetProperty("IMPORT_PREFIX")
                              : this->GetProperty("PREFIX"));
  const char* targetSuffix = (implib
                              ? this->GetProperty("IMPORT_SUFFIX")
                              : this->GetProperty("SUFFIX"));
  const char* configPostfix = 0;
  if(config && *config)
    {
    std::string configProp = cmSystemTools::UpperCase(config);
    configProp += "_POSTFIX";
    configPostfix = this->GetProperty(configProp.c_str());
    }
  const char* prefixVar = this->GetPrefixVariableInternal(type, implib);
  const char* suffixVar = this->GetSuffixVariableInternal(type, implib);
  const char* ll =
    this->GetLinkerLanguage(
      this->Makefile->GetLocalGenerator()->GetGlobalGenerator());
  // first try language specific suffix
  if(ll)
    {
    if(!targetSuffix && suffixVar && *suffixVar)
      {
      std::string langSuff = suffixVar + std::string("_") + ll;
      targetSuffix = this->Makefile->GetDefinition(langSuff.c_str());
      }
    if(!targetPrefix && prefixVar && *prefixVar)
      {
      std::string langPrefix = prefixVar + std::string("_") + ll;
      targetPrefix = this->Makefile->GetDefinition(langPrefix.c_str());
      }
    }

  // if there is no prefix on the target use the cmake definition
  if(!targetPrefix && prefixVar)
    {
    targetPrefix = this->Makefile->GetSafeDefinition(prefixVar);
    }
  // if there is no suffix on the target use the cmake definition
  if(!targetSuffix && suffixVar)
    {
    targetSuffix = this->Makefile->GetSafeDefinition(suffixVar);
    }

  // Begin the final name with the prefix.
  outPrefix = targetPrefix?targetPrefix:"";

  // Append the target name or property-specified name.
  const char* outName = 0;
  if(config && *config)
    {
    std::string configProp = cmSystemTools::UpperCase(config);
    configProp += "_OUTPUT_NAME";
    outName = this->GetProperty(configProp.c_str());
    }
  if(!outName)
    {
    outName = this->GetProperty("OUTPUT_NAME");
    }
  if(outName)
    {
    outBase = outName;
    }
  else
    {
    outBase = this->GetName();
    }

  // Append the per-configuration postfix.
  outBase += configPostfix?configPostfix:"";

  // Name shared libraries with their version number on some platforms.
  if(const char* version = this->GetProperty("VERSION"))
    {
    if(type == cmTarget::SHARED_LIBRARY && !implib &&
       this->Makefile->IsOn("CMAKE_SHARED_LIBRARY_NAME_WITH_VERSION"))
      {
      outBase += "-";
      outBase += version;
      }
    }

  // Append the suffix.
  outSuffix = targetSuffix?targetSuffix:"";
}

void cmTarget::GetLibraryNames(std::string& name,
                               std::string& soName,
                               std::string& realName,
                               std::string& impName,
                               std::string& pdbName,
                               const char* config)
{
  // Get the names based on the real type of the library.
  this->GetLibraryNamesInternal(name, soName, realName, impName, pdbName,
                                this->GetType(), config);
}

void cmTarget::GetLibraryCleanNames(std::string& staticName,
                                    std::string& sharedName,
                                    std::string& sharedSOName,
                                    std::string& sharedRealName,
                                    std::string& importName,
                                    std::string& pdbName,
                                    const char* config)
{
  // Get the name as if this were a static library.
  std::string soName;
  std::string realName;
  std::string impName;
  this->GetLibraryNamesInternal(staticName, soName, realName, impName,
                                pdbName, cmTarget::STATIC_LIBRARY, config);

  // Get the names as if this were a shared library.
  if(this->GetType() == cmTarget::STATIC_LIBRARY)
    {
    // Since the real type is static then the user either specified
    // STATIC or did not specify a type.  In the former case the
    // shared library will never be present.  In the latter case the
    // type will never be MODULE.  Either way the only names that
    // might have to be cleaned are the shared library names.
    this->GetLibraryNamesInternal(sharedName, sharedSOName, sharedRealName,
                                  importName, pdbName,
                                  cmTarget::SHARED_LIBRARY, config);
    }
  else
    {
    // Use the name of the real type of the library (shared or module).
    this->GetLibraryNamesInternal(sharedName, sharedSOName, sharedRealName,
                                  importName, pdbName, this->GetType(),
                                  config);
    }
}

void cmTarget::GetLibraryNamesInternal(std::string& name,
                                       std::string& soName,
                                       std::string& realName,
                                       std::string& impName,
                                       std::string& pdbName,
                                       TargetType type,
                                       const char* config)
{
  // Construct the name of the soname flag variable for this language.
  const char* ll =
    this->GetLinkerLanguage(
      this->Makefile->GetLocalGenerator()->GetGlobalGenerator());
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
     !this->Makefile->GetDefinition(sonameFlag.c_str()))
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

  // Get the components of the library name.
  std::string prefix;
  std::string base;
  std::string suffix;
  this->GetFullNameInternal(type, config, false, prefix, base, suffix);

  // The library name.
  name = prefix+base+suffix;

  // The library's soname.
#if defined(__APPLE__)
  soName = prefix+base;
#else
  soName = name;
#endif
  if(soversion)
    {
    soName += ".";
    soName += soversion;
    }
#if defined(__APPLE__)
  soName += suffix;
#endif

  // The library's real name on disk.
#if defined(__APPLE__)
  realName = prefix+base;
#else
  realName = name;
#endif
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
#if defined(__APPLE__)
  realName += suffix;
#endif

  // The import library name.
  if(type == cmTarget::SHARED_LIBRARY)
    {
    impName = this->GetFullNameInternal(type, config, true);
    }
  else
    {
    impName = "";
    }

  // The program database file name.
  pdbName = prefix+base+".pdb";
}

void cmTarget::GetExecutableNames(std::string& name,
                                  std::string& realName,
                                  std::string& pdbName,
                                  const char* config)
{
  // Get the names based on the real type of the executable.
  this->GetExecutableNamesInternal(name, realName, pdbName,
                                   this->GetType(), config);
}

void cmTarget::GetExecutableCleanNames(std::string& name,
                                       std::string& realName,
                                       std::string& pdbName,
                                       const char* config)
{
  // Get the name and versioned name of this executable.
  this->GetExecutableNamesInternal(name, realName, pdbName,
                                   cmTarget::EXECUTABLE, config);
}

void cmTarget::GetExecutableNamesInternal(std::string& name,
                                          std::string& realName,
                                          std::string& pdbName,
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

  // Get the components of the executable name.
  std::string prefix;
  std::string base;
  std::string suffix;
  this->GetFullNameInternal(type, config, false, prefix, base, suffix);

  // The executable name.
  name = prefix+base+suffix;

  // The executable's real name on disk.
#if defined(__CYGWIN__)
  realName = prefix+base;
#else
  realName = name;
#endif
  if(version)
    {
    realName += "-";
    realName += version;
    }
#if defined(__CYGWIN__)
  realName += suffix;
#endif

  // The program database file name.
  pdbName = prefix+base+".pdb";
}

//----------------------------------------------------------------------------
void cmTarget::SetPropertyDefault(const char* property,
                                  const char* default_value)
{
  // Compute the name of the variable holding the default value.
  std::string var = "CMAKE_";
  var += property;

  if(const char* value = this->Makefile->GetDefinition(var.c_str()))
    {
    this->SetProperty(property, value);
    }
  else if(default_value)
    {
    this->SetProperty(property, default_value);
    }
}

//----------------------------------------------------------------------------
bool cmTarget::HaveBuildTreeRPATH()
{
  return (!this->GetPropertyAsBool("SKIP_BUILD_RPATH") &&
          !this->LinkLibraries.empty());
}

//----------------------------------------------------------------------------
bool cmTarget::HaveInstallTreeRPATH()
{
  const char* install_rpath = this->GetProperty("INSTALL_RPATH");
  return install_rpath && *install_rpath;
}

//----------------------------------------------------------------------------
bool cmTarget::NeedRelinkBeforeInstall()
{
  // Only executables and shared libraries can have an rpath and may
  // need relinking.
  if(this->TargetTypeValue != cmTarget::EXECUTABLE &&
     this->TargetTypeValue != cmTarget::SHARED_LIBRARY &&
     this->TargetTypeValue != cmTarget::MODULE_LIBRARY)
    {
    return false;
    }

  // If there is no install location this target will not be installed
  // and therefore does not need relinking.
  if(!this->GetHaveInstallRule())
    {
    return false;
    }

  // If skipping all rpaths completely then no relinking is needed.
  if(this->Makefile->IsOn("CMAKE_SKIP_RPATH"))
    {
    return false;
    }

  // If building with the install-tree rpath no relinking is needed.
  if(this->GetPropertyAsBool("BUILD_WITH_INSTALL_RPATH"))
    {
    return false;
    }

  // Check for rpath support on this platform.
  if(const char* ll = this->GetLinkerLanguage(
       this->Makefile->GetLocalGenerator()->GetGlobalGenerator()))
    {
    std::string flagVar = "CMAKE_SHARED_LIBRARY_RUNTIME_";
    flagVar += ll;
    flagVar += "_FLAG";
    if(!this->Makefile->IsSet(flagVar.c_str()))
      {
      // There is no rpath support on this platform so nothing needs
      // relinking.
      return false;
      }
    }
  else
    {
    // No linker language is known.  This error will be reported by
    // other code.
    return false;
    }

  // If either a build or install tree rpath is set then the rpath
  // will likely change between the build tree and install tree and
  // this target must be relinked.
  return this->HaveBuildTreeRPATH() || this->HaveInstallTreeRPATH();
}

//----------------------------------------------------------------------------
std::string cmTarget::GetInstallNameDirForBuildTree(const char* config)
{
  // If building directly for installation then the build tree install_name
  // is the same as the install tree.
  if(this->GetPropertyAsBool("BUILD_WITH_INSTALL_RPATH"))
    {
    return GetInstallNameDirForInstallTree(config);
    }

  // Use the build tree directory for the target.
  if(this->Makefile->IsOn("CMAKE_PLATFORM_HAS_INSTALLNAME") &&
     !this->Makefile->IsOn("CMAKE_SKIP_RPATH") &&
     !this->GetPropertyAsBool("SKIP_BUILD_RPATH"))
    {
    std::string dir = this->GetDirectory(config);
    dir += "/";
    return dir;
    }
  else
    {
    return "";
    }
}

//----------------------------------------------------------------------------
std::string cmTarget::GetInstallNameDirForInstallTree(const char*)
{
  // Lookup the target property.
  const char* install_name_dir = this->GetProperty("INSTALL_NAME_DIR");
  if(this->Makefile->IsOn("CMAKE_PLATFORM_HAS_INSTALLNAME") &&
     !this->Makefile->IsOn("CMAKE_SKIP_RPATH") &&
     install_name_dir && *install_name_dir)
    {
    std::string dir = install_name_dir;
    dir += "/";
    return dir;
    }
  else
    {
    return "";
    }
}
