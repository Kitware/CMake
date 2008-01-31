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
#include "cmComputeLinkInformation.h"

#include "cmComputeLinkDepends.h"

#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmTarget.h"

#include <algorithm>

#include <ctype.h>

/*
Notes about linking on various platforms:

------------------------------------------------------------------------------

Linux, FreeBSD, Mac OS X, IRIX, Sun, Windows:

Linking to libraries using the full path works fine.

------------------------------------------------------------------------------

On AIX, more work is needed.

  The "-bnoipath" option is needed.  From "man ld":

    Note: If you specify a shared object, or an archive file
    containing a shared object, with an absolute or relative path
    name, instead of with the -lName flag, the path name is
    included in the import file ID string in the loader section of
    the output file. You can override this behavior with the
    -bnoipath option.

      noipath

        For shared objects listed on the command-line, rather than
        specified with the -l flag, use a null path component when
        listing the shared object in the loader section of the
        output file. A null path component is always used for
        shared objects specified with the -l flag. This option
        does not affect the specification of a path component by
        using a line beginning with #! in an import file. The
        default is the ipath option.

  This prevents the full path specified on the compile line from being
  compiled directly into the binary.

  By default the linker places -L paths in the embedded runtime path.
  In order to implement CMake's RPATH interface correctly, we need the
  -blibpath:Path option.  From "man ld":

      libpath:Path

        Uses Path as the library path when writing the loader section
        of the output file. Path is neither checked for validity nor
        used when searching for libraries specified by the -l flag.
        Path overrides any library paths generated when the -L flag is
        used.

        If you do not specify any -L flags, or if you specify the
        nolibpath option, the default library path information is
        written in the loader section of the output file. The default
        library path information is the value of the LIBPATH
        environment variable if it is defined, and /usr/lib:/lib,
        otherwise.

  We can pass -Wl,-blibpath:/usr/lib:/lib always to avoid the -L stuff
  and not break when the user sets LIBPATH.  Then if we want to add an
  rpath we insert it into the option before /usr/lib.

------------------------------------------------------------------------------

On HP-UX, more work is needed.  There are differences between
versions.

ld: 92453-07 linker linker ld B.10.33 990520

  Linking with a full path works okay for static and shared libraries.
  The linker seems to always put the full path to where the library
  was found in the binary whether using a full path or -lfoo syntax.
  Transitive link dependencies work just fine due to the full paths.

  It has the "-l:libfoo.sl" option.  The +nodefaultrpath is accepted
  but not documented and does not seem to do anything.  There is no
  +forceload option.

ld: 92453-07 linker ld HP Itanium(R) B.12.41  IPF/IPF

  Linking with a full path works okay for static libraries.

  Linking with a full path works okay for shared libraries.  However
  dependent (transitive) libraries of those linked directly must be
  either found with an rpath stored in the direct dependencies or
  found in -L paths as if they were specified with "-l:libfoo.sl"
  (really "-l:<soname>").  The search matches that of the dynamic
  loader but only with -L paths.  In other words, if we have an
  executable that links to shared library bar which links to shared
  library foo, the link line for the exe must contain

    /dir/with/bar/libbar.sl -L/dir/with/foo

  It does not matter whether the exe wants to link to foo directly or
  whether /dir/with/foo/libfoo.sl is listed.  The -L path must still
  be present.  It should match the runtime path computed for the
  executable taking all directly and transitively linked libraries
  into account.

  The "+nodefaultrpath" option should be used to avoid getting -L
  paths in the rpath unless we add our own rpath with +b.  This means
  that skip-build-rpath should use this option.

  See documentation in "man ld", "man dld.so", and
  http://docs.hp.com/en/B2355-90968/creatingandusinglibraries.htm

    +[no]defaultrpath
      +defaultrpath is the default.  Include any paths that are
      specified with -L in the embedded path, unless you specify the
      +b option.  If you use +b, only the path list specified by +b is
      in the embedded path.

      The +nodefaultrpath option removes all library paths that were
      specified with the -L option from the embedded path.  The linker
      searches the library paths specified by the -L option at link
      time.  At run time, the only library paths searched are those
      specified by the environment variables LD_LIBRARY_PATH and
      SHLIB_PATH, library paths specified by the +b linker option, and
      finally the default library paths.

    +rpathfirst
      This option will cause the paths specified in RPATH (embedded
      path) to be used before the paths specified in LD_LIBRARY_PATH
      or SHLIB_PATH, in searching for shared libraries.  This changes
      the default search order of LD_LIBRARY_PATH, SHLIB_PATH, and
      RPATH (embedded path).
*/

//----------------------------------------------------------------------------
cmComputeLinkInformation
::cmComputeLinkInformation(cmTarget* target, const char* config)
{
  // Store context information.
  this->Target = target;
  this->Makefile = this->Target->GetMakefile();
  this->LocalGenerator = this->Makefile->GetLocalGenerator();
  this->GlobalGenerator = this->LocalGenerator->GetGlobalGenerator();

  // The configuration being linked.
  this->Config = config;

  // Get the language used for linking this target.
  this->LinkLanguage =
    this->Target->GetLinkerLanguage(this->GlobalGenerator);
  if(!this->LinkLanguage)
    {
    // The Compute method will do nothing, so skip the rest of the
    // initialization.
    return;
    }

  // Check whether we should use an import library for linking a target.
  this->UseImportLibrary =
    this->Makefile->GetDefinition("CMAKE_IMPORT_LIBRARY_SUFFIX")?true:false;

  // On platforms without import libraries there may be a special flag
  // to use when creating a plugin (module) that obtains symbols from
  // the program that will load it.
  this->LoaderFlag = 0;
  if(!this->UseImportLibrary &&
     this->Target->GetType() == cmTarget::MODULE_LIBRARY)
    {
    std::string loader_flag_var = "CMAKE_SHARED_MODULE_LOADER_";
    loader_flag_var += this->LinkLanguage;
    loader_flag_var += "_FLAG";
    this->LoaderFlag = this->Makefile->GetDefinition(loader_flag_var.c_str());
    }

  // Get options needed to link libraries.
  this->LibLinkFlag =
    this->Makefile->GetSafeDefinition("CMAKE_LINK_LIBRARY_FLAG");
  this->LibLinkFileFlag =
    this->Makefile->GetSafeDefinition("CMAKE_LINK_LIBRARY_FILE_FLAG");
  this->LibLinkSuffix =
    this->Makefile->GetSafeDefinition("CMAKE_LINK_LIBRARY_SUFFIX");

  // Get options needed to specify RPATHs.
  this->RuntimeUseChrpath = false;
  if(this->Target->GetType() != cmTarget::STATIC_LIBRARY)
    {
    std::string rtVar = "CMAKE_";
    if(this->Target->GetType() == cmTarget::EXECUTABLE)
      {
      rtVar += "EXECUTABLE";
      }
    else
      {
      rtVar += "SHARED_LIBRARY";
      }
    rtVar += "_RUNTIME_";
    rtVar += this->LinkLanguage;
    rtVar += "_FLAG";
    std::string rtSepVar = rtVar + "_SEP";
    this->RuntimeFlag = this->Makefile->GetSafeDefinition(rtVar.c_str());
    this->RuntimeSep = this->Makefile->GetSafeDefinition(rtSepVar.c_str());
    this->RuntimeAlways =
      (this->Makefile->
       GetSafeDefinition("CMAKE_PLATFORM_REQUIRED_RUNTIME_PATH"));
    this->RuntimeUseChrpath = this->Target->IsChrpathUsed();
    }

  // Get link type information.
  this->ComputeLinkTypeInfo();

  // Setup the link item parser.
  this->ComputeItemParserInfo();

  // Setup framework support.
  this->ComputeFrameworkInfo();

  // Choose a mode for dealing with shared library dependencies.
  this->SharedDependencyMode = SharedDepModeNone;
  if(const char* mode =
     this->Makefile->GetDefinition("CMAKE_DEPENDENT_SHARED_LIBRARY_MODE"))
    {
    if(strcmp(mode, "LINK") == 0)
      {
      this->SharedDependencyMode = SharedDepModeLink;
      }
    else if(strcmp(mode, "DIR") == 0)
      {
      this->SharedDependencyMode = SharedDepModeDir;
      }
    }

  // Get the implicit link directories for this platform.
  if(const char* implicitLinks =
     (this->Makefile->GetDefinition
      ("CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES")))
    {
    std::vector<std::string> implicitLinkVec;
    cmSystemTools::ExpandListArgument(implicitLinks, implicitLinkVec);
    for(std::vector<std::string>::const_iterator
          i = implicitLinkVec.begin();
        i != implicitLinkVec.end(); ++i)
      {
      this->ImplicitLinkDirs.insert(*i);
      }
    }

  // Initial state.
  this->RuntimeSearchPathComputed = false;
  this->HaveUserFlagItem = false;

  // Decide whether to enable compatible library search path mode.
  // There exists code that effectively does
  //
  //    /path/to/libA.so -lB
  //
  // where -lB is meant to link to /path/to/libB.so.  This is broken
  // because it specified -lB without specifying a link directory (-L)
  // in which to search for B.  This worked in CMake 2.4 and below
  // because -L/path/to would be added by the -L/-l split for A.  In
  // order to support such projects we need to add the directories
  // containing libraries linked with a full path to the -L path.
  this->OldLinkDirMode = false;
  if(this->Makefile->IsOn("CMAKE_LINK_OLD_PATHS") ||
     this->Makefile->GetLocalGenerator()
     ->NeedBackwardsCompatibility(2, 4))
    {
    this->OldLinkDirMode = true;
    }
}

//----------------------------------------------------------------------------
cmComputeLinkInformation::ItemVector const&
cmComputeLinkInformation::GetItems()
{
  return this->Items;
}

//----------------------------------------------------------------------------
std::vector<std::string> const& cmComputeLinkInformation::GetDirectories()
{
  return this->Directories;
}

//----------------------------------------------------------------------------
std::vector<std::string> const& cmComputeLinkInformation::GetDepends()
{
  return this->Depends;
}

//----------------------------------------------------------------------------
std::vector<std::string> const& cmComputeLinkInformation::GetFrameworkPaths()
{
  return this->FrameworkPaths;
}

//----------------------------------------------------------------------------
std::set<cmTarget*> const&
cmComputeLinkInformation::GetSharedLibrariesLinked()
{
  return this->SharedLibrariesLinked;
}

//----------------------------------------------------------------------------
bool cmComputeLinkInformation::Compute()
{
  // Skip targets that do not link.
  if(!(this->Target->GetType() == cmTarget::EXECUTABLE ||
       this->Target->GetType() == cmTarget::SHARED_LIBRARY ||
       this->Target->GetType() == cmTarget::MODULE_LIBRARY ||
       this->Target->GetType() == cmTarget::STATIC_LIBRARY))
    {
    return false;
    }

  // We require a link language for the target.
  if(!this->LinkLanguage)
    {
    cmSystemTools::
      Error("CMake can not determine linker language for target:",
            this->Target->GetName());
    return false;
    }

  // Compute the ordered link line items.
  cmComputeLinkDepends cld(this->Target, this->Config);
  cmComputeLinkDepends::EntryVector const& linkEntries = cld.Compute();

  // Add the link line items.
  for(cmComputeLinkDepends::EntryVector::const_iterator
        lei = linkEntries.begin();
      lei != linkEntries.end(); ++lei)
    {
    this->AddItem(lei->Item, lei->Target, lei->IsSharedDep);
    }

  // Restore the target link type so the correct system runtime
  // libraries are found.
  const char* lss = this->Target->GetProperty("LINK_SEARCH_END_STATIC");
  if(cmSystemTools::IsOn(lss))
    {
    this->SetCurrentLinkType(LinkStatic);
    }
  else
    {
    this->SetCurrentLinkType(this->StartLinkType);
    }

  // Compute the linker search path.
  this->ComputeLinkerSearchDirectories();

  return true;
}

//----------------------------------------------------------------------------
void cmComputeLinkInformation::AddItem(std::string const& item,
                                       cmTarget* tgt, bool isSharedDep)
{
  // If dropping shared library dependencies, ignore them.
  if(isSharedDep && this->SharedDependencyMode == SharedDepModeNone)
    {
    return;
    }

  // Compute the proper name to use to link this library.
  const char* config = this->Config;
  bool impexe = (tgt && tgt->IsExecutableWithExports());
  if(impexe && !this->UseImportLibrary && !this->LoaderFlag)
    {
    // Skip linking to executables on platforms with no import
    // libraries or loader flags.
    return;
    }

  if(tgt && (tgt->GetType() == cmTarget::STATIC_LIBRARY ||
             tgt->GetType() == cmTarget::SHARED_LIBRARY ||
             tgt->GetType() == cmTarget::MODULE_LIBRARY ||
             impexe))
    {
    // This is a CMake target.  Ask the target for its real name.
    if(impexe && this->LoaderFlag)
      {
      // This link item is an executable that may provide symbols
      // used by this target.  A special flag is needed on this
      // platform.  Add it now.
      std::string linkItem;
      linkItem = this->LoaderFlag;
      std::string exe = tgt->GetFullPath(config, this->UseImportLibrary);
      linkItem += exe;
      this->Items.push_back(Item(linkItem, true));
      this->Depends.push_back(exe);
      }
    else
      {
      // Decide whether to use an import library.
      bool implib =
        (this->UseImportLibrary &&
         (impexe || tgt->GetType() == cmTarget::SHARED_LIBRARY));

      // Handle shared dependencies in directory mode.
      if(isSharedDep && this->SharedDependencyMode == SharedDepModeDir)
        {
        std::string dir = tgt->GetDirectory(config, implib);
        this->SharedDependencyDirectories.push_back(dir);
        return;
        }

      // Pass the full path to the target file.
      std::string lib = tgt->GetFullPath(config, implib);
      this->Depends.push_back(lib);

      if(tgt->IsFrameworkOnApple())
        {
        // Frameworks on OS X need only the framework directory to
        // link.
        std::string fw = tgt->GetDirectory(config, implib);
        this->AddFrameworkItem(fw);
        this->SharedLibrariesLinked.insert(tgt);
        }
      else
        {
        this->AddTargetItem(lib, tgt);
        this->AddLibraryRuntimeInfo(lib, tgt);
        }
      }
    }
  else
    {
    // This is not a CMake target.  Use the name given.
    if(cmSystemTools::FileIsFullPath(item.c_str()))
      {
      if(cmSystemTools::FileIsDirectory(item.c_str()))
        {
        // This is a directory.
        this->AddDirectoryItem(item);
        }
      else
        {
        // Use the full path given to the library file.
        this->Depends.push_back(item);
        this->AddFullItem(item);
        this->AddLibraryRuntimeInfo(item);
        }
      }
    else
      {
      // This is a library or option specified by the user.
      this->AddUserItem(item);
      }
    }
}

//----------------------------------------------------------------------------
void cmComputeLinkInformation::ComputeLinkTypeInfo()
{
  // First assume we cannot do link type stuff.
  this->LinkTypeEnabled = false;

  // Lookup link type selection flags.
  const char* static_link_type_flag = 0;
  const char* shared_link_type_flag = 0;
  const char* target_type_str = 0;
  switch(this->Target->GetType())
    {
    case cmTarget::EXECUTABLE:     target_type_str = "EXE"; break;
    case cmTarget::SHARED_LIBRARY: target_type_str = "SHARED_LIBRARY"; break;
    case cmTarget::MODULE_LIBRARY: target_type_str = "SHARED_MODULE"; break;
    default: break;
    }
  if(target_type_str)
    {
    std::string static_link_type_flag_var = "CMAKE_";
    static_link_type_flag_var += target_type_str;
    static_link_type_flag_var += "_LINK_STATIC_";
    static_link_type_flag_var += this->LinkLanguage;
    static_link_type_flag_var += "_FLAGS";
    static_link_type_flag =
      this->Makefile->GetDefinition(static_link_type_flag_var.c_str());

    std::string shared_link_type_flag_var = "CMAKE_";
    shared_link_type_flag_var += target_type_str;
    shared_link_type_flag_var += "_LINK_DYNAMIC_";
    shared_link_type_flag_var += this->LinkLanguage;
    shared_link_type_flag_var += "_FLAGS";
    shared_link_type_flag =
      this->Makefile->GetDefinition(shared_link_type_flag_var.c_str());
    }

  // We can support link type switching only if all needed flags are
  // known.
  if(static_link_type_flag && *static_link_type_flag &&
     shared_link_type_flag && *shared_link_type_flag)
    {
    this->LinkTypeEnabled = true;
    this->StaticLinkTypeFlag = static_link_type_flag;
    this->SharedLinkTypeFlag = shared_link_type_flag;
    }

  // TODO: Lookup the starting link type from the target (is it being
  // linked statically?).
  this->StartLinkType = LinkShared;
  this->CurrentLinkType = this->StartLinkType;
}

//----------------------------------------------------------------------------
void cmComputeLinkInformation::ComputeItemParserInfo()
{
  // Get possible library name prefixes.
  cmMakefile* mf = this->Makefile;
  this->AddLinkPrefix(mf->GetDefinition("CMAKE_STATIC_LIBRARY_PREFIX"));
  this->AddLinkPrefix(mf->GetDefinition("CMAKE_SHARED_LIBRARY_PREFIX"));

  // Import library names should be matched and treated as shared
  // libraries for the purposes of linking.
  this->AddLinkExtension(mf->GetDefinition("CMAKE_IMPORT_LIBRARY_SUFFIX"),
                         LinkShared);
  this->AddLinkExtension(mf->GetDefinition("CMAKE_STATIC_LIBRARY_SUFFIX"),
                         LinkStatic);
  this->AddLinkExtension(mf->GetDefinition("CMAKE_SHARED_LIBRARY_SUFFIX"),
                         LinkShared);
  this->AddLinkExtension(mf->GetDefinition("CMAKE_LINK_LIBRARY_SUFFIX"),
                         LinkUnknown);
  if(const char* linkSuffixes =
     mf->GetDefinition("CMAKE_EXTRA_LINK_EXTENSIONS"))
    {
    std::vector<std::string> linkSuffixVec;
    cmSystemTools::ExpandListArgument(linkSuffixes, linkSuffixVec);
    for(std::vector<std::string>::iterator i = linkSuffixVec.begin();
        i != linkSuffixVec.end(); ++i)
      {
      this->AddLinkExtension(i->c_str(), LinkUnknown);
      }
    }

  // Compute a regex to match link extensions.
  std::string libext = this->CreateExtensionRegex(this->LinkExtensions);

  // Create regex to remove any library extension.
  std::string reg("(.*)");
  reg += libext;
  this->RemoveLibraryExtension.compile(reg.c_str());

  // Create a regex to match a library name.  Match index 1 will be
  // the prefix if it exists and empty otherwise.  Match index 2 will
  // be the library name.  Match index 3 will be the library
  // extension.
  reg = "^(";
  for(std::set<cmStdString>::iterator p = this->LinkPrefixes.begin();
      p != this->LinkPrefixes.end(); ++p)
    {
    reg += *p;
    reg += "|";
    }
  reg += ")";
  reg += "([^/]*)";

  // Create a regex to match any library name.
  std::string reg_any = reg;
  reg_any += libext;
#ifdef CM_COMPUTE_LINK_INFO_DEBUG
  fprintf(stderr, "any regex [%s]\n", reg_any.c_str());
#endif
  this->ExtractAnyLibraryName.compile(reg_any.c_str());

  // Create a regex to match static library names.
  if(!this->StaticLinkExtensions.empty())
    {
    std::string reg_static = reg;
    reg_static += this->CreateExtensionRegex(this->StaticLinkExtensions);
#ifdef CM_COMPUTE_LINK_INFO_DEBUG
  fprintf(stderr, "static regex [%s]\n", reg_static.c_str());
#endif
    this->ExtractStaticLibraryName.compile(reg_static.c_str());
    }

  // Create a regex to match shared library names.
  if(!this->SharedLinkExtensions.empty())
    {
    std::string reg_shared = reg;
    reg_shared += this->CreateExtensionRegex(this->SharedLinkExtensions);
#ifdef CM_COMPUTE_LINK_INFO_DEBUG
  fprintf(stderr, "shared regex [%s]\n", reg_shared.c_str());
#endif
    this->ExtractSharedLibraryName.compile(reg_shared.c_str());
    }
}

//----------------------------------------------------------------------------
void cmComputeLinkInformation::AddLinkPrefix(const char* p)
{
  if(p)
    {
    this->LinkPrefixes.insert(p);
    }
}

//----------------------------------------------------------------------------
void cmComputeLinkInformation::AddLinkExtension(const char* e, LinkType type)
{
  if(e && *e)
    {
    if(type == LinkStatic)
      {
      this->StaticLinkExtensions.push_back(e);
      }
    if(type == LinkShared)
      {
      this->SharedLinkExtensions.push_back(e);
      }
    this->LinkExtensions.push_back(e);
    }
}

//----------------------------------------------------------------------------
std::string
cmComputeLinkInformation
::CreateExtensionRegex(std::vector<std::string> const& exts)
{
  // Build a list of extension choices.
  std::string libext = "(";
  const char* sep = "";
  for(std::vector<std::string>::const_iterator i = exts.begin();
      i != exts.end(); ++i)
    {
    // Separate this choice from the previous one.
    libext += sep;
    sep = "|";

    // Store this extension choice with the "." escaped.
    libext += "\\";
#if defined(_WIN32) && !defined(__CYGWIN__)
    libext += this->NoCaseExpression(i->c_str());
#else
    libext += *i;
#endif
    }

  // Finish the list.
  libext += ").*";
  return libext;
}

//----------------------------------------------------------------------------
std::string cmComputeLinkInformation::NoCaseExpression(const char* str)
{
  std::string ret;
  const char* s = str;
  while(*s)
    {
    if(*s == '.')
      {
      ret += *s;
      }
    else
      {
      ret += "[";
      ret += tolower(*s);
      ret += toupper(*s);
      ret += "]";
      }
    s++;
    }
  return ret;
}

//-------------------------------------------------------------------
void cmComputeLinkInformation::SetCurrentLinkType(LinkType lt)
{
  // If we are changing the current link type add the flag to tell the
  // linker about it.
  if(this->CurrentLinkType != lt)
    {
    this->CurrentLinkType = lt;

    if(this->LinkTypeEnabled)
      {
      switch(this->CurrentLinkType)
        {
        case LinkStatic:
          this->Items.push_back(Item(this->StaticLinkTypeFlag, false));
          break;
        case LinkShared:
          this->Items.push_back(Item(this->SharedLinkTypeFlag, false));
          break;
        default:
          break;
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmComputeLinkInformation::AddTargetItem(std::string const& item,
                                             cmTarget* target)
{
  // This is called to handle a link item that is a full path to a target.
  // If the target is not a static library make sure the link type is
  // shared.  This is because dynamic-mode linking can handle both
  // shared and static libraries but static-mode can handle only
  // static libraries.  If a previous user item changed the link type
  // to static we need to make sure it is back to shared.
  if(target->GetType() != cmTarget::STATIC_LIBRARY)
    {
    this->SetCurrentLinkType(LinkShared);
    }

  // If this platform wants a flag before the full path, add it.
  if(!this->LibLinkFileFlag.empty())
    {
    this->Items.push_back(Item(this->LibLinkFileFlag, false));
    }

  // Keep track of shared library targets linked.
  if(target->GetType() == cmTarget::SHARED_LIBRARY)
    {
    this->SharedLibrariesLinked.insert(target);
    }

  // Now add the full path to the library.
  this->Items.push_back(Item(item, true));
}

//----------------------------------------------------------------------------
void cmComputeLinkInformation::AddFullItem(std::string const& item)
{
  // Check for the implicit link directory special case.
  if(this->CheckImplicitDirItem(item))
    {
    return;
    }

  // This is called to handle a link item that is a full path.
  // If the target is not a static library make sure the link type is
  // shared.  This is because dynamic-mode linking can handle both
  // shared and static libraries but static-mode can handle only
  // static libraries.  If a previous user item changed the link type
  // to static we need to make sure it is back to shared.
  if(this->LinkTypeEnabled)
    {
    std::string name = cmSystemTools::GetFilenameName(item);
    if(this->ExtractSharedLibraryName.find(name))
      {
      this->SetCurrentLinkType(LinkShared);
      }
    else if(!this->ExtractStaticLibraryName.find(item))
      {
      // We cannot determine the type.  Assume it is the target's
      // default type.
      this->SetCurrentLinkType(this->StartLinkType);
      }
    }

  // Record the directory in which the library appears because CMake
  // 2.4 in below added these as -L paths.
  if(this->OldLinkDirMode)
    {
    this->OldLinkDirs.push_back(cmSystemTools::GetFilenamePath(item));
    }

  // If this platform wants a flag before the full path, add it.
  if(!this->LibLinkFileFlag.empty())
    {
    this->Items.push_back(Item(this->LibLinkFileFlag, false));
    }

  // Now add the full path to the library.
  this->Items.push_back(Item(item, true));
}

//----------------------------------------------------------------------------
bool cmComputeLinkInformation::CheckImplicitDirItem(std::string const& item)
{
  // We only switch to a pathless item if the link type may be
  // enforced.  Fortunately only platforms that support link types
  // seem to have magic per-architecture implicit link directories.
  if(!this->LinkTypeEnabled)
    {
    return false;
    }

  // Check if this item is in an implicit link directory.
  std::string dir = cmSystemTools::GetFilenamePath(item);
  if(this->ImplicitLinkDirs.find(dir) == this->ImplicitLinkDirs.end())
    {
    // Only libraries in implicit link directories are converted to
    // pathless items.
    return false;
    }

  // Many system linkers support multiple architectures by
  // automatically selecting the implicit linker search path for the
  // current architecture.  If the library appears in an implicit link
  // directory then just report the file name without the directory
  // portion.  This will allow the system linker to locate the proper
  // library for the architecture at link time.
  std::string file = cmSystemTools::GetFilenameName(item);
  this->AddUserItem(file);
  return true;
}

//----------------------------------------------------------------------------
void cmComputeLinkInformation::AddUserItem(std::string const& item)
{
  // This is called to handle a link item that does not match a CMake
  // target and is not a full path.  We check here if it looks like a
  // library file name to automatically request the proper link type
  // from the linker.  For example:
  //
  //   foo       ==>  -lfoo
  //   libfoo.a  ==>  -Wl,-Bstatic -lfoo
  std::string lib;

  // Parse out the prefix, base, and suffix components of the
  // library name.  If the name matches that of a shared or static
  // library then set the link type accordingly.
  //
  // Search for shared library names first because some platforms
  // have shared libraries with names that match the static library
  // pattern.  For example cygwin and msys use the convention
  // libfoo.dll.a for import libraries and libfoo.a for static
  // libraries.  On AIX a library with the name libfoo.a can be
  // shared!
  if(this->ExtractSharedLibraryName.find(item))
    {
    // This matches a shared library file name.
#ifdef CM_COMPUTE_LINK_INFO_DEBUG
    fprintf(stderr, "shared regex matched [%s] [%s] [%s]\n",
            this->ExtractSharedLibraryName.match(1).c_str(),
            this->ExtractSharedLibraryName.match(2).c_str(),
            this->ExtractSharedLibraryName.match(3).c_str());
#endif
    // Set the link type to shared.
    this->SetCurrentLinkType(LinkShared);

    // Use just the library name so the linker will search.
    lib = this->ExtractSharedLibraryName.match(2);
    }
  else if(this->ExtractStaticLibraryName.find(item))
    {
    // This matches a static library file name.
#ifdef CM_COMPUTE_LINK_INFO_DEBUG
    fprintf(stderr, "static regex matched [%s] [%s] [%s]\n",
            this->ExtractStaticLibraryName.match(1).c_str(),
            this->ExtractStaticLibraryName.match(2).c_str(),
            this->ExtractStaticLibraryName.match(3).c_str());
#endif
    // Set the link type to static.
    this->SetCurrentLinkType(LinkStatic);

    // Use just the library name so the linker will search.
    lib = this->ExtractStaticLibraryName.match(2);
    }
  else if(this->ExtractAnyLibraryName.find(item))
    {
    // This matches a library file name.
#ifdef CM_COMPUTE_LINK_INFO_DEBUG
    fprintf(stderr, "any regex matched [%s] [%s] [%s]\n",
            this->ExtractAnyLibraryName.match(1).c_str(),
            this->ExtractAnyLibraryName.match(2).c_str(),
            this->ExtractAnyLibraryName.match(3).c_str());
#endif
    // Restore the target link type since this item does not specify
    // one.
    this->SetCurrentLinkType(this->StartLinkType);

    // Use just the library name so the linker will search.
    lib = this->ExtractAnyLibraryName.match(2);
    }
  else if(item[0] == '-' || item[0] == '$' || item[0] == '`')
    {
    // This is a linker option provided by the user.
    this->HaveUserFlagItem = true;

    // Restore the target link type since this item does not specify
    // one.
    this->SetCurrentLinkType(this->StartLinkType);

    // Use the item verbatim.
    this->Items.push_back(Item(item, false));
    return;
    }
  else
    {
    // This is a name specified by the user.
    this->HaveUserFlagItem = true;

    // We must ask the linker to search for a library with this name.
    // Restore the target link type since this item does not specify
    // one.
    this->SetCurrentLinkType(this->StartLinkType);
    lib = item;
    }

  // Create an option to ask the linker to search for the library.
  std::string out = this->LibLinkFlag;
  out += lib;
  out += this->LibLinkSuffix;
  this->Items.push_back(Item(out, false));

  // Here we could try to find the library the linker will find and
  // add a runtime information entry for it.  It would probably not be
  // reliable and we want to encourage use of full paths for library
  // specification.
}

//----------------------------------------------------------------------------
void cmComputeLinkInformation::AddFrameworkItem(std::string const& item)
{
  // Try to separate the framework name and path.
  if(!this->SplitFramework.find(item.c_str()))
    {
    cmOStringStream e;
    e << "Could not parse framework path \"" << item << "\" "
      << "linked by target " << this->Target->GetName() << ".";
    cmSystemTools::Error(e.str().c_str());
    return;
    }

  // Add the directory portion to the framework search path.
  this->AddFrameworkPath(this->SplitFramework.match(1));

  // Add the item using the -framework option.
  std::string fw = "-framework ";
  fw += this->SplitFramework.match(2);
  this->Items.push_back(Item(fw, false));
}

//----------------------------------------------------------------------------
void cmComputeLinkInformation::AddDirectoryItem(std::string const& item)
{
#ifdef __APPLE__
  if(cmSystemTools::IsPathToFramework(item.c_str()))
    {
    this->AddFrameworkItem(item);
    }
  else
#endif
    {
    this->DropDirectoryItem(item);
    }
}

//----------------------------------------------------------------------------
void cmComputeLinkInformation::DropDirectoryItem(std::string const& item)
{
  // A full path to a directory was found as a link item.  Warn the
  // user.
  cmOStringStream e;
  e << "WARNING: Target \"" << this->Target->GetName()
    << "\" requests linking to directory \"" << item << "\".  "
    << "Targets may link only to libraries.  "
    << "CMake is dropping the item.";
  cmSystemTools::Message(e.str().c_str());
}

//----------------------------------------------------------------------------
void cmComputeLinkInformation::ComputeFrameworkInfo()
{
  // Avoid adding system framework paths.  See "man ld" on OS X.
  this->FrameworkPathsEmmitted.insert("/Library/Frameworks");
  this->FrameworkPathsEmmitted.insert("/Network/Library/Frameworks");
  this->FrameworkPathsEmmitted.insert("/System/Library/Frameworks");

  // Regular expression to extract a framework path and name.
  this->SplitFramework.compile("(.*)/(.*)\\.framework$");
}

//----------------------------------------------------------------------------
void cmComputeLinkInformation::AddFrameworkPath(std::string const& p)
{
  if(this->FrameworkPathsEmmitted.insert(p).second)
    {
    this->FrameworkPaths.push_back(p);
    }
}

//----------------------------------------------------------------------------
void cmComputeLinkInformation::ComputeLinkerSearchDirectories()
{
  // Some search paths should never be emitted.
  this->DirectoriesEmmitted = this->ImplicitLinkDirs;
  this->DirectoriesEmmitted.insert("");

  // Check if we need to include the runtime search path at link time.
  std::string var = "CMAKE_SHARED_LIBRARY_LINK_";
  var += this->LinkLanguage;
  var += "_WITH_RUNTIME_PATH";
  if(this->Makefile->IsOn(var.c_str()))
    {
    // This platform requires the runtime library path for shared
    // libraries to be specified at link time as -L paths.  It needs
    // them so that transitive dependencies of the libraries linked
    // may be found by the linker.
    this->AddLinkerSearchDirectories(this->GetRuntimeSearchPath());
    }

  // Get the search path entries requested by the user.
  this->AddLinkerSearchDirectories(this->Target->GetLinkDirectories());

  // Support broken projects if necessary.
  if(this->HaveUserFlagItem && this->OldLinkDirMode)
    {
    this->AddLinkerSearchDirectories(this->OldLinkDirs);
    }

  // Help the linker find dependent shared libraries.
  if(this->SharedDependencyMode == SharedDepModeDir)
    {
    // TODO: These directories should probably be added to the runtime
    // path ordering analysis.  However they are a bit different.
    // They should be placed both on the -L path and in the rpath.
    // The link-with-runtime-path feature above should be replaced by
    // this.
    this->AddLinkerSearchDirectories(this->SharedDependencyDirectories);
    }

}

//----------------------------------------------------------------------------
void
cmComputeLinkInformation
::AddLinkerSearchDirectories(std::vector<std::string> const& dirs)
{
  for(std::vector<std::string>::const_iterator i = dirs.begin();
      i != dirs.end(); ++i)
    {
    if(this->DirectoriesEmmitted.insert(*i).second)
      {
      this->Directories.push_back(*i);
      }
    }
}

//----------------------------------------------------------------------------
std::vector<std::string> const&
cmComputeLinkInformation::GetRuntimeSearchPath()
{
  if(!this->RuntimeSearchPathComputed)
    {
    this->RuntimeSearchPathComputed = true;
    this->CollectRuntimeDirectories();
    this->FindConflictingLibraries();
    this->OrderRuntimeSearchPath();
    }
  return this->RuntimeSearchPath;
}

//============================================================================
// Directory ordering computation.
//   - Useful to compute a safe runtime library path order
//   - Need runtime path for supporting INSTALL_RPATH_USE_LINK_PATH
//   - Need runtime path at link time to pickup transitive link dependencies
//     for shared libraries (in future when we do not always add them).

//----------------------------------------------------------------------------
void
cmComputeLinkInformation::AddLibraryRuntimeInfo(std::string const& fullPath,
                                                cmTarget* target)
{
  // Skip targets that are not shared libraries (modules cannot be linked).
  if(target->GetType() != cmTarget::SHARED_LIBRARY)
    {
    return;
    }

  // Try to get the soname of the library.  Only files with this name
  // could possibly conflict.
  std::string soName = target->GetSOName(this->Config);
  const char* soname = soName.empty()? 0 : soName.c_str();

  // Add the library runtime entry.
  this->AddLibraryRuntimeInfo(fullPath, soname);
}

//----------------------------------------------------------------------------
void
cmComputeLinkInformation::AddLibraryRuntimeInfo(std::string const& fullPath,
                                                const char* soname)
{
  // Get the name of the library from the file name.
  std::string file = cmSystemTools::GetFilenameName(fullPath);
  if(!this->ExtractSharedLibraryName.find(file.c_str()))
    {
    // This is not the name of a shared library.
    return;
    }

  // Add the runtime information at most once.
  if(this->LibraryRuntimeInfoEmmitted.insert(fullPath).second)
    {
    // Construct the runtime information entry for this library.
    LibraryRuntimeEntry entry;
    entry.FileName =  cmSystemTools::GetFilenameName(fullPath);
    entry.SOName = soname? soname : "";
    entry.Directory = cmSystemTools::GetFilenamePath(fullPath);
    this->LibraryRuntimeInfo.push_back(entry);
    }
  else
    {
    // This can happen if the same library is linked multiple times.
    // In that case the runtime information check need be done only
    // once anyway.  For shared libs we could add a check in AddItem
    // to not repeat them.
    }
}

//----------------------------------------------------------------------------
void cmComputeLinkInformation::CollectRuntimeDirectories()
{
  // Get all directories that should be in the runtime search path.

  // Add directories containing libraries linked with full path.
  for(std::vector<LibraryRuntimeEntry>::iterator
        ei = this->LibraryRuntimeInfo.begin();
      ei != this->LibraryRuntimeInfo.end(); ++ei)
    {
    ei->DirectoryIndex = this->AddRuntimeDirectory(ei->Directory);
    }

  // Add link directories specified for the target.
  std::vector<std::string> const& dirs = this->Target->GetLinkDirectories();
  for(std::vector<std::string>::const_iterator di = dirs.begin();
      di != dirs.end(); ++di)
    {
    this->AddRuntimeDirectory(*di);
    }
}

//----------------------------------------------------------------------------
int cmComputeLinkInformation::AddRuntimeDirectory(std::string const& dir)
{
  // Add the runtime directory with a unique index.
  std::map<cmStdString, int>::iterator i =
    this->RuntimeDirectoryIndex.find(dir);
  if(i == this->RuntimeDirectoryIndex.end())
    {
    std::map<cmStdString, int>::value_type
      entry(dir, static_cast<int>(this->RuntimeDirectories.size()));
    i = this->RuntimeDirectoryIndex.insert(entry).first;
    this->RuntimeDirectories.push_back(dir);
    }

  return i->second;
}

//----------------------------------------------------------------------------
struct cmCLIRuntimeConflictCompare
{
  typedef std::pair<int, int> RuntimeConflictPair;

  // The conflict pair is unique based on just the directory
  // (first).  The second element is only used for displaying
  // information about why the entry is present.
  bool operator()(RuntimeConflictPair const& l,
                  RuntimeConflictPair const& r)
    {
    return l.first == r.first;
    }
};

//----------------------------------------------------------------------------
void cmComputeLinkInformation::FindConflictingLibraries()
{
  // Allocate the conflict graph.
  this->RuntimeConflictGraph.resize(this->RuntimeDirectories.size());
  this->RuntimeDirectoryVisited.resize(this->RuntimeDirectories.size(), 0);

  // Find all runtime directories providing each library.
  for(unsigned int lri = 0; lri < this->LibraryRuntimeInfo.size(); ++lri)
    {
    this->FindDirectoriesForLib(lri);
    }

  // Clean up the conflict graph representation.
  for(std::vector<RuntimeConflictList>::iterator
        i = this->RuntimeConflictGraph.begin();
      i != this->RuntimeConflictGraph.end(); ++i)
    {
    // Sort the outgoing edges for each graph node so that the
    // original order will be preserved as much as possible.
    std::sort(i->begin(), i->end());

    // Make the edge list unique so cycle detection will be reliable.
    RuntimeConflictList::iterator last =
      std::unique(i->begin(), i->end(), cmCLIRuntimeConflictCompare());
    i->erase(last, i->end());
    }
}

//----------------------------------------------------------------------------
void cmComputeLinkInformation::FindDirectoriesForLib(unsigned int lri)
{
  // Search through the runtime directories to find those providing
  // this library.
  LibraryRuntimeEntry& re = this->LibraryRuntimeInfo[lri];
  for(unsigned int i = 0; i < this->RuntimeDirectories.size(); ++i)
    {
    // Skip the directory that is supposed to provide the library.
    if(this->RuntimeDirectories[i] == re.Directory)
      {
      continue;
      }

    // Determine which type of check to do.
    if(!re.SOName.empty())
      {
      // We have the library soname.  Check if it will be found.
      std::string file = this->RuntimeDirectories[i];
      file += "/";
      file += re.SOName;
      std::set<cmStdString> const& files =
        (this->GlobalGenerator
         ->GetDirectoryContent(this->RuntimeDirectories[i], false));
      if((std::set<cmStdString>::const_iterator(files.find(re.SOName)) !=
          files.end()) ||
         cmSystemTools::FileExists(file.c_str(), true))
        {
        // The library will be found in this directory but this is not
        // the directory named for it.  Add an entry to make sure the
        // desired directory comes before this one.
        RuntimeConflictPair p(re.DirectoryIndex, lri);
        this->RuntimeConflictGraph[i].push_back(p);
        }
      }
    else
      {
      // We do not have the soname.  Look for files in the directory
      // that may conflict.
      std::set<cmStdString> const& files =
        (this->GlobalGenerator
         ->GetDirectoryContent(this->RuntimeDirectories[i], true));

      // Get the set of files that might conflict.  Since we do not
      // know the soname just look at all files that start with the
      // file name.  Usually the soname starts with the library name.
      std::string base = re.FileName;
      std::set<cmStdString>::const_iterator first = files.lower_bound(base);
      ++base[base.size()-1];
      std::set<cmStdString>::const_iterator last = files.upper_bound(base);
      bool found = false;
      for(std::set<cmStdString>::const_iterator fi = first;
          !found && fi != last; ++fi)
        {
        found = true;
        }

      if(found)
        {
        // The library may be found in this directory but this is not
        // the directory named for it.  Add an entry to make sure the
        // desired directory comes before this one.
        RuntimeConflictPair p(re.DirectoryIndex, lri);
        this->RuntimeConflictGraph[i].push_back(p);
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmComputeLinkInformation::OrderRuntimeSearchPath()
{
  // Allow a cycle to be diagnosed once.
  this->CycleDiagnosed = false;
  this->WalkId = 0;

  // Iterate through the directories in the original order.
  for(unsigned int i=0; i < this->RuntimeDirectories.size(); ++i)
    {
    // Start a new DFS from this node.
    ++this->WalkId;
    this->VisitRuntimeDirectory(i);
    }
}

//----------------------------------------------------------------------------
void cmComputeLinkInformation::VisitRuntimeDirectory(unsigned int i)
{
  // Skip nodes already visited.
  if(this->RuntimeDirectoryVisited[i])
    {
    if(this->RuntimeDirectoryVisited[i] == this->WalkId)
      {
      // We have reached a node previously visited on this DFS.
      // There is a cycle.
      this->DiagnoseCycle();
      }
    return;
    }

  // We are now visiting this node so mark it.
  this->RuntimeDirectoryVisited[i] = this->WalkId;

  // Visit the neighbors of the node first.
  RuntimeConflictList const& clist = this->RuntimeConflictGraph[i];
  for(RuntimeConflictList::const_iterator j = clist.begin();
      j != clist.end(); ++j)
    {
    this->VisitRuntimeDirectory(j->first);
    }

  // Now that all directories required to come before this one have
  // been emmitted, emit this directory.
  this->RuntimeSearchPath.push_back(this->RuntimeDirectories[i]);
}

//----------------------------------------------------------------------------
void cmComputeLinkInformation::DiagnoseCycle()
{
  // Report the cycle at most once.
  if(this->CycleDiagnosed)
    {
    return;
    }
  this->CycleDiagnosed = true;

  // Construct the message.
  cmOStringStream e;
  e << "WARNING: Cannot generate a safe runtime path for target "
    << this->Target->GetName()
    << " because there is a cycle in the constraint graph:\n";

  // Display the conflict graph.
  for(unsigned int i=0; i < this->RuntimeConflictGraph.size(); ++i)
    {
    RuntimeConflictList const& clist = this->RuntimeConflictGraph[i];
    e << "dir " << i << " is [" << this->RuntimeDirectories[i] << "]\n";
    for(RuntimeConflictList::const_iterator j = clist.begin();
        j != clist.end(); ++j)
      {
      e << "  dir " << j->first << " must precede it due to [";
      LibraryRuntimeEntry const& re = this->LibraryRuntimeInfo[j->second];
      if(re.SOName.empty())
        {
        e << re.FileName;
        }
      else
        {
        e << re.SOName;
        }
      e << "]\n";
      }
    }
  cmSystemTools::Message(e.str().c_str());
}

//----------------------------------------------------------------------------
void cmComputeLinkInformation::GetRPath(std::vector<std::string>& runtimeDirs,
                                        bool for_install)
{
  // Select whether to generate runtime search directories.
  bool outputRuntime =
    !this->Makefile->IsOn("CMAKE_SKIP_RPATH") && !this->RuntimeFlag.empty();

  // Select whether to generate an rpath for the install tree or the
  // build tree.
  bool linking_for_install =
    (for_install ||
     this->Target->GetPropertyAsBool("BUILD_WITH_INSTALL_RPATH"));
  bool use_install_rpath =
    (outputRuntime && this->Target->HaveInstallTreeRPATH() &&
     linking_for_install);
  bool use_build_rpath =
    (outputRuntime && this->Target->HaveBuildTreeRPATH() &&
     !linking_for_install);
  bool use_link_rpath =
    outputRuntime && linking_for_install &&
    this->Target->GetPropertyAsBool("INSTALL_RPATH_USE_LINK_PATH");

  // Construct the RPATH.
  if(use_install_rpath)
    {
    const char* install_rpath = this->Target->GetProperty("INSTALL_RPATH");
    cmSystemTools::ExpandListArgument(install_rpath, runtimeDirs);
    }
  if(use_build_rpath || use_link_rpath)
    {
    std::vector<std::string> const& rdirs = this->GetRuntimeSearchPath();
    for(std::vector<std::string>::const_iterator ri = rdirs.begin();
        ri != rdirs.end(); ++ri)
      {
      // Put this directory in the rpath if using build-tree rpath
      // support or if using the link path as an rpath.
      if(use_build_rpath)
        {
        runtimeDirs.push_back(*ri);
        }
      else if(use_link_rpath)
        {
        // Do not add any path inside the source or build tree.
        const char* topSourceDir = this->Makefile->GetHomeDirectory();
        const char* topBinaryDir = this->Makefile->GetHomeOutputDirectory();
        if(!cmSystemTools::ComparePath(ri->c_str(), topSourceDir) &&
           !cmSystemTools::ComparePath(ri->c_str(), topBinaryDir) &&
           !cmSystemTools::IsSubDirectory(ri->c_str(), topSourceDir) &&
           !cmSystemTools::IsSubDirectory(ri->c_str(), topBinaryDir))
          {
          runtimeDirs.push_back(*ri);
          }
        }
      }
    }

  // Add runtime paths required by the platform to always be
  // present.  This is done even when skipping rpath support.
  cmSystemTools::ExpandListArgument(this->RuntimeAlways.c_str(), runtimeDirs);
}

//----------------------------------------------------------------------------
std::string cmComputeLinkInformation::GetRPathString(bool for_install)
{
  // Get the directories to use.
  std::vector<std::string> runtimeDirs;
  this->GetRPath(runtimeDirs, for_install);

  // Concatenate the paths.
  std::string rpath;
  const char* sep = "";
  for(std::vector<std::string>::const_iterator ri = runtimeDirs.begin();
      ri != runtimeDirs.end(); ++ri)
    {
    // Separate from previous path.
    rpath += sep;
    sep = this->GetRuntimeSep().c_str();

    // Add this path.
    rpath += *ri;
    }
  return rpath;
}

//----------------------------------------------------------------------------
std::string cmComputeLinkInformation::GetChrpathString()
{
  if(!this->RuntimeUseChrpath)
    {
    return "";
    }

  return this->GetRPathString(true);
}

//----------------------------------------------------------------------------
std::string cmComputeLinkInformation::GetChrpathTool()
{
  return this->Makefile->GetSafeDefinition("CMAKE_CHRPATH");
}
