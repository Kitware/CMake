#include "cmBorlandMakefileGenerator.h"
#include "cmMakefile.h"
#include "cmStandardIncludes.h"
#include "cmSystemTools.h"
#include "cmSourceFile.h"
#include "cmMakeDepend.h"
#include "cmCacheManager.h"
#include <Sysutils.hpp>
using namespace std;
//---------------------------------------------------------------------------
cmBorlandMakefileGenerator::cmBorlandMakefileGenerator() {
  m_CacheOnly = false;
  m_Recurse   = false;
}
//---------------------------------------------------------------------------
void cmBorlandMakefileGenerator::GenerateMakefile() {
  if (m_CacheOnly) {
    // Generate the cache only stuff
    this->GenerateCacheOnly();
    // if recurse then generate for all sub- makefiles
    if (m_Recurse) {
      this->RecursiveGenerateCacheOnly();
    }
  }
}
//---------------------------------------------------------------------------
void cmBorlandMakefileGenerator::GenerateCacheOnly() {
  cmSystemTools::MakeDirectory(m_Makefile->GetStartOutputDirectory());
  string dest = m_Makefile->GetStartOutputDirectory();
  dest += "/makefile.mak";
  this->OutputMakefile(dest.c_str());
}
//---------------------------------------------------------------------------
void cmBorlandMakefileGenerator::RecursiveGenerateCacheOnly() {
  vector<cmMakefile*> makefiles;
  m_Makefile->FindSubDirectoryCMakeListsFiles(makefiles);
  for (vector<cmMakefile*>::iterator i=makefiles.begin(); i!=makefiles.end(); ++i) {
    cmMakefile* mf = *i;
    cmBorlandMakefileGenerator* gen = new cmBorlandMakefileGenerator;
    gen->SetCacheOnlyOn();
    gen->SetRecurseOff();
    mf->SetMakefileGenerator(gen);
    mf->GenerateMakefile();
  }
  // CLEAN up the makefiles created
  for (unsigned int i=0; i<makefiles.size(); ++i) {
    delete makefiles[i];
  }
}
//---------------------------------------------------------------------------
void cmBorlandMakefileGenerator::OutputMakefile(const char* file) {
  //
  // Create sub directories for aux source directories
  //
  vector<string>& auxSourceDirs = m_Makefile->GetAuxSourceDirectories();
  if ( auxSourceDirs.size() ) {
    // For the case when this is running as a remote build
    // on unix, make the directory
    for (vector<string>::iterator i=auxSourceDirs.begin(); i!=auxSourceDirs.end(); ++i) {
      cmSystemTools::MakeDirectory(i->c_str());
    }
  }
  ostrstream fout;
  //
  // Begin writing to makefile.mak
  //
  fout << "# CMAKE Borland (win32) makefile : Edit with Caution \n\n";
  //
  // Turn on Autodependency chaecking
  //
  fout << ".autodepend \n\n";
  //
  // Define all our compile and make flags/variables
  //
  string replace;
  // Careful with these directory paths....\ vs /
  replace = "BCBBINPATH       = @BCB_BIN_PATH@ \n";
  fout << m_Makefile->ExpandVariablesInString(replace);
  replace = "BCB              = $(BCBBINPATH)/.. \n";
  fout << m_Makefile->ExpandVariablesInString(replace);
  replace = "OUTDIRLIB        = @LIBRARY_OUTPUT_PATH@ \n";
  fout << cmSystemTools::ConvertToWindowsSlashes(m_Makefile->ExpandVariablesInString(replace));
  replace = "OUTDIREXE        = @EXECUTABLE_OUTPUT_PATH@ \n";
  fout << m_Makefile->ExpandVariablesInString(replace);
  replace = "USERDEFINES      = @DEFS_USER@ \n";
  fout << m_Makefile->ExpandVariablesInString(replace);
  replace = "SYSDEFINES       = @DEFS_SYS@ \n";
  fout << m_Makefile->ExpandVariablesInString(replace);
  replace = "CMAKE_COMMAND    = ${CMAKE_COMMAND} \n";
  fout << m_Makefile->ExpandVariablesInString(replace);
  replace = "CPP              = \"$(BCBBINPATH)/BCC32.exe\" +CPP_PROJ.CFG \n";
  fout << m_Makefile->ExpandVariablesInString(replace);
  replace =
    "CPPFLAGS_DEBUG   = @FLAGS_CPP_DEBUG@ \n"
    "CPPFLAGS_RELEASE = @FLAGS_CPP_RELEASE@ \n"
    "CPPFLAGS_WARNING = @FLAGS_CPP_WARNING@ \n"
    "LINKFLAGS_DLL    = @FLAGS_LINK_DLL@ \n"
    "LINKFLAGS_BPL    = @FLAGS_LINK_BPL@ \n"
    "LINKFLAGS_EXE    = @FLAGS_LINK_EXE@ \n"
    "LINKFLAGS_DEBUG  = @FLAGS_LINK_DEBUG@ \n";
  fout << m_Makefile->ExpandVariablesInString(replace);
  replace = "LINKFLAGS_STATIC = @FLAGS_LINK_STATIC@ \n";
  fout << m_Makefile->ExpandVariablesInString(replace);
  fout << "CMAKE_CURRENT_SOURCE = " << m_Makefile->GetStartDirectory() << "\n";
  fout << "CMAKE_CURRENT_BINARY = " << m_Makefile->GetStartOutputDirectory() << "\n";
  fout << "OBJDIR               = " << m_Makefile->GetStartOutputDirectory() << "\n";
  fout << "CMAKEDEFINES         = " << m_Makefile->GetDefineFlags() << "\n";
  fout << "LINK_LIB = \\ \n";
  fout << "  import32.lib \\ \n";
  fout << "  cw32mti.lib \n\n";
  //
  // create a make variable with all of the sources for this makefile for depend purposes.
  //
  vector<string> lfiles = m_Makefile->GetListFiles();
  // sort the array
  sort(lfiles.begin(), lfiles.end(), less<string>());
  // remove duplicates
  vector<string>::iterator new_end = unique(lfiles.begin(), lfiles.end());
  lfiles.erase(new_end, lfiles.end());
  fout << "CMAKE_MAKEFILE_SOURCES = \\ \n";
  string dir;
  for (vector<string>::const_iterator i=lfiles.begin(); i!=lfiles.end(); ++i) {
    dir = *i;
    cmSystemTools::ConvertToWindowsSlashes(dir);
    fout << "  " << dir << " \\\n";
  }
  dir = m_Makefile->GetHomeOutputDirectory();
  dir += "/CMakeCache.txt";
  cmSystemTools::ConvertToWindowsSlashes(dir);
  fout << "  " << dir << "\n\n";
  //
  // Output Include paths
  //
  vector<string>& includes = m_Makefile->GetIncludeDirectories();
  fout << "INCLUDEPATH =";
  for (vector<string>::iterator i=includes.begin(); i!=includes.end(); ++i) {
    string include = *i;
    fout << "-I" << cmSystemTools::EscapeSpaces(i->c_str()) << "; \\\n  ";
  }
  fout << "-I" << cmSystemTools::EscapeSpaces(m_Makefile->GetStartDirectory()) << "\n\n";
  //
  // for each target add to the list of targets
  //
  fout << "TARGETS = ";
  const cmTargets &tgts = m_Makefile->GetTargets();
  // list libraries first
  for (cmTargets::const_iterator l=tgts.begin(); l!=tgts.end(); l++) {
    if ((l->second.GetType() == cmTarget::STATIC_LIBRARY) && l->second.IsInAll()) {
      fout << " \\\n  $(OUTDIRLIB)\\" << l->first.c_str() << ".lib";
    }
    if ((l->second.GetType() == cmTarget::SHARED_LIBRARY) && l->second.IsInAll()) {
      fout << " \\\n  $(OUTDIRLIB)\\" << l->first.c_str() << ".dll";
    }
    if ((l->second.GetType() == cmTarget::MODULE_LIBRARY) && l->second.IsInAll()) {
      fout << " \\\n  $(OUTDIRLIB)\\" << l->first.c_str() << ".bpl";
    }
  }
  // executables
  for (cmTargets::const_iterator l=tgts.begin(); l!=tgts.end(); l++) {
    if ((l->second.GetType() == cmTarget::EXECUTABLE || l->second.GetType() == cmTarget::WIN32_EXECUTABLE) && l->second.IsInAll()) {
      fout << " \\\n  " << l->first.c_str() << ".exe";
    }
  }
  // list utilities last
  for (cmTargets::const_iterator l=tgts.begin(); l!=tgts.end(); l++) {
    if (l->second.GetType() == cmTarget::UTILITY && l->second.IsInAll()) {
      fout << " \\\n  " << l->first.c_str();
    }
  }
  fout << "\n\n";
  //
  // Now create the source file groups for each target
  //
  for (cmTargets::const_iterator l=tgts.begin(); l!=tgts.end(); l++) {
    vector<cmSourceFile> classes = l->second.GetSourceFiles();
    if (classes.begin() != classes.end()) {
        fout << l->first << "_SRC_OBJS = ";
        for (vector<cmSourceFile>::iterator i=classes.begin(); i!=classes.end(); i++) {
        string ext = i->GetSourceExtension();
          if (!i->IsAHeaderFileOnly() && (ext!="def" && ext!="rc")) {
          fout << " \\\n  " << cmSystemTools::ConvertToWindowsSlashes(i->GetSourceName()) << ".obj ";
        }
      }
        fout << "\n\n";
    }
  }
  //
  // Create the link lib list for each target
  //
  // do .lib files
  string libname;
  for (cmTargets::const_iterator t=tgts.begin(); t!=tgts.end(); t++) {
    cmTarget::LinkLibraries& libs = t->second.GetLinkLibraries();

    if ((t->second.GetType() == cmTarget::STATIC_LIBRARY)   ||
        (t->second.GetType() == cmTarget::SHARED_LIBRARY)   ||
        (t->second.GetType() == cmTarget::MODULE_LIBRARY)   ||
        (t->second.GetType() == cmTarget::EXECUTABLE)       ||
        (t->second.GetType() == cmTarget::WIN32_EXECUTABLE))
    {
        fout << t->first << "_LINK_LIB = ";
        for (cmTarget::LinkLibraries::const_iterator l=libs.begin(); l!=libs.end(); l++) {
          if ((t->first!=l->first) &&
              (t->second.GetType()!=cmTarget::INSTALL_FILES || t->second.GetType()!=cmTarget::INSTALL_PROGRAMS)) {
            // if this lib is not a target then don't add OUTDIRLIB to it
            if (tgts.find(l->first)==tgts.end())
              libname = l->first;
            else
              libname = "$(OUTDIRLIB)\\" + l->first;
            if (libname.find(".bpi")!=string::npos) continue;
            cmSystemTools::ReplaceString(libname, ".lib", "");
            libname += ".lib";
            fout << " \\\n  " << cmSystemTools::EscapeSpaces(libname.c_str());
          }
        }
        fout << "\n\n";
    }
  }
  // do .bpi package files
  for (cmTargets::const_iterator t=tgts.begin(); t!=tgts.end(); t++) {
    cmTarget::LinkLibraries& libs = t->second.GetLinkLibraries();
    if ((t->second.GetType() == cmTarget::STATIC_LIBRARY)   ||
        (t->second.GetType() == cmTarget::SHARED_LIBRARY)   ||
        (t->second.GetType() == cmTarget::MODULE_LIBRARY)   ||
        (t->second.GetType() == cmTarget::EXECUTABLE)       ||
        (t->second.GetType() == cmTarget::WIN32_EXECUTABLE))
    {
        fout << t->first << "_LINK_BPI = ";
        for (cmTarget::LinkLibraries::const_iterator l=libs.begin(); l!=libs.end(); l++) {
          if ((t->first!=l->first) &&
              (t->second.GetType()!=cmTarget::INSTALL_FILES || t->second.GetType()!=cmTarget::INSTALL_PROGRAMS)) {
            // if this lib is not a target then don't add OUTDIRLIB to it
            if (tgts.find(l->first)==tgts.end())
              libname = l->first;
            else
              libname = "$(OUTDIRLIB)\\" + l->first;
            if (libname.find(".bpi")==string::npos) continue;
            fout << " \\\n  " << cmSystemTools::EscapeSpaces(libname.c_str());
          }
        }
        fout << "\n\n";
    }
  }
  //
  // Create the link dir list - use same for all targets
  //
  vector<string> dirs = m_Makefile->GetLinkDirectories();
  fout << "LINK_DIR =";
  for (vector<string>::const_iterator d=dirs.begin(); d!=dirs.end(); d++) {
    string temp = cmSystemTools::EscapeSpaces(d->c_str());
    fout << temp << ";";
  }
  fout << "$(OUTDIRLIB)\n\n";

  //
  // The project rule - Build All targets
  //
  fout << "DEFAULT : \n";
  fout << "  @$(MAKE) makefile.mak \n";
  fout << "  @$(MAKE) ALL \n\n";
  //
  // Create a rule to allow us to setup the compiler and output dir
  //
  fout << "PREPARE : \n";
  fout << "  @if not exist \"$(OBJDIR)/.\" md \"$(OBJDIR)\" \n";
  fout << "  @copy &&| \n";
  fout << "    $(SYSDEFINES) $(CMAKEDEFINES) $(USERDEFINES)\n";
  fout << "    $(CPPFLAGS_DEBUG) $(CPPFLAGS_WARNING) \n";
  fout << "    $(INCLUDEPATH) \n";
  fout << "    -I\"$(BCB)/include\";\"$(BCB)/include/rw\";\"$(BCB)/include/vcl\"; \n";
  fout << "| CPP_PROJ.CFG \n\n";
  //
  this->OutputDependencies(fout);
  this->OutputTargets(fout);
  this->OutputSubDirectoryRules(fout);
  //
  this->OutputCustomRules(fout);
  this->OutputMakeRules(fout);
  //
  // We'll omit current dir in path where possible
  string fullname, outpath = m_Makefile->GetStartOutputDirectory();
  outpath += "/";
  //
  for (cmTargets::const_iterator l=tgts.begin(); l!=tgts.end(); l++) {
    vector<cmSourceFile> classes = l->second.GetSourceFiles();
    if (classes.begin() != classes.end()) {
        for (vector<cmSourceFile>::iterator i=classes.begin(); i!=classes.end(); i++) {
          if (!i->IsAHeaderFileOnly()) {
          fullname = i->GetFullPath();
          cmSystemTools::ReplaceString(fullname, outpath.c_str(), "");
          fout << "" << cmSystemTools::ConvertToWindowsSlashes(i->GetSourceName()) << ".obj : " << fullname << "\n";
        }
      }
    }
  }
  //
  //
  //
  ofstream ffout(file);
  if (!ffout) {
    cmSystemTools::Error("Error can not open for write: ", file);
    return;
  }
  string makefileastext = fout.str();
//  cmSystemTools::CleanUpWindowsSlashes(makefileastext);
//  makefileastext = StringReplace(makefileastext.c_str(), "¬", "/", TReplaceFlags()<<rfReplaceAll).c_str();
  ffout << makefileastext << "\n# End of File\n";
}
//---------------------------------------------------------------------------
// output the list of libraries that the executables in this makefile will depend on.
void cmBorlandMakefileGenerator::OutputDependencies(ostream& fout) {
    // Each dependency should only be emitted once.
    set<string> emitted;
    //
    // Output/Search the list of libraries that will be linked into the executable
    //
    fout << "DEPEND_LIBS = ";
    cmTarget::LinkLibraries& libs = m_Makefile->GetLinkLibraries();
    emitted.clear();
    for (cmTarget::LinkLibraries::const_iterator lib2=libs.begin(); lib2!=libs.end(); ++lib2) {

      // loop over the list of directories that the libraries might
      // be in, looking for an ADD_LIBRARY(lib...) line. This would
      // be stored in the cache
      if( ! emitted.insert(lib2->first).second ) continue;

      const char* cacheValue = m_Makefile->GetDefinition(lib2->first.c_str());
      if (cacheValue) {
        fout << "\\\n  $(OUTDIRLIB)\\" << lib2->first << ".lib ";
      }
    }
    fout << "\n\n";
    //
    // Same list, but this time output a rule to rebuild if they are out of date
    //
    emitted.clear();
    for (cmTarget::LinkLibraries::const_iterator lib2=libs.begin(); lib2!=libs.end(); ++lib2) {
      // loop over the list of directories that the libraries might
      // be in, looking for an ADD_LIBRARY(lib...) line. This would
      // be stored in the cache
      if ( ! emitted.insert(lib2->first).second ) continue;

//      const char* cacheValue = cmCacheManager::GetInstance()->GetCacheValue(lib2->first.c_str());
//      if (cacheValue) {
//        // put out a rule to build the library if it does not exist
//        fout << "$(OUTDIRLIB)/" << lib2->first << ".lib : " << "$(OUTDIRLIB)/" << lib2->first << ".dll \n";
//        fout << "  @implib -w " << "$(OUTDIRLIB)/" << lib2->first << ".lib " << "$(OUTDIRLIB)/" << lib2->first << ".dll \n\n";
//      }
    }
//    fout << "\n";
}
void cmBorlandMakefileGenerator::OutputTargets(ostream& fout) {
    // Do Libraries first as executables may depend on them
    const cmTargets &tgts = m_Makefile->GetTargets();
    for (cmTargets::const_iterator l=tgts.begin(); l!=tgts.end(); l++) {
      if (l->second.GetType() == cmTarget::STATIC_LIBRARY) {
        //
        // at the moment, static and shared are treated the same
        // WARNING. TLIB fails with Unix style Forward slashes - use $(OUTDIRLIB)\\
        // WARNING. IMPLIB works better with Forward slashes - use $(OUTDIRLIB)\\
        //
        fout << "# this should be a static library \n";
        fout << "$(OUTDIRLIB)\\" << l->first << ".lib : ${" << l->first << "_SRC_OBJS} \n";
        string Libname = "$(OUTDIRLIB)\\" + l->first + ".lib";
        fout << "  TLib.exe $(LINKFLAGS_STATIC) /u " << Libname.c_str() << " @&&| \n";
        fout << "    $? \n";
        fout << "| \n\n";
      }
      if (l->second.GetType() == cmTarget::SHARED_LIBRARY) {
        fout << "# this should be a shared (DLL) library \n";
        fout << "$(OUTDIRLIB)\\" << l->first << ".dll : ${" << l->first << "_SRC_OBJS} \n";
        fout << "  @ilink32.exe @&&| \n";
        fout << "    -L\"$(BCB)/lib\" -L$(LINK_DIR) $(LINKFLAGS_DLL) $(LINKFLAGS_DEBUG) \"$(BCB)/lib/c0d32.obj\" ";
        fout << "$(" << l->first << "_SRC_OBJS) ";
        fout << "$(" << l->first << "_LINK_BPI) , $<, $*, ";
        fout << "$(" << l->first << "_LINK_LIB) $(LINK_LIB) \n";
        fout << "| \n";
        fout << "  @implib -w " << "$(OUTDIRLIB)\\" << l->first << ".lib " << "$(OUTDIRLIB)\\" << l->first << ".dll \n\n";
      }
      if (l->second.GetType() == cmTarget::MODULE_LIBRARY) {
        fout << "# this should be a Borland Package library \n";
        fout << "$(OUTDIRLIB)\\" << l->first << ".bpl : ${" << l->first << "_SRC_OBJS} \n";
        fout << "  @ilink32.exe @&&| \n";
        fout << "    -L\"$(BCB)/lib\" -L$(LINK_DIR) $(LINKFLAGS_BPL) $(LINKFLAGS_DEBUG) \"$(BCB)/lib/c0pkg32.obj\" ";
        fout << "$(" << l->first << "_SRC_OBJS) ";
        fout << "$(" << l->first << "_LINK_BPI) , $<, $*, ";
        fout << "$(" << l->first << "_LINK_LIB) $(LINK_LIB) \n";
        fout << "| \n";
      }
    }
    // Do Executables
    for (cmTargets::const_iterator l=tgts.begin(); l!=tgts.end(); l++) {
      if (l->second.GetType()==cmTarget::WIN32_EXECUTABLE) {
        fout << l->first << ".exe : ${" << l->first << "_SRC_OBJS} \n";
        fout << "  @ilink32.exe @&&| \n";
        fout << "    -L\"$(BCB)/lib\" -L$(LINK_DIR) $(LINKFLAGS_EXE) $(LINKFLAGS_DEBUG) \"$(BCB)/lib/c0w32.obj\" ";
        fout << "$(" << l->first << "_SRC_OBJS) ";
        fout << "$(" << l->first << "_LINK_BPI) , $<, $*, ";
        fout << "$(" << l->first << "_LINK_LIB) $(LINK_LIB) \n";
        fout << "| \n\n";
      }
      else if (l->second.GetType()==cmTarget::EXECUTABLE) {
        fout << l->first << ".exe : ${" << l->first << "_SRC_OBJS} \n";
        fout << "  @ilink32.exe @&&| \n";
        fout << "    -L\"$(BCB)/lib\" -L$(LINK_DIR) $(LINKFLAGS_EXE) $(LINKFLAGS_DEBUG) \"$(BCB)/lib/c0x32.obj\" ";
        fout << "$(" << l->first << "_SRC_OBJS) , $<, $*, ";
        fout << "$(" << l->first << "_LINK_LIB) $(LINK_LIB) \n";
        fout << "| \n\n";
      }
    }
}
//---------------------------------------------------------------------------
void cmBorlandMakefileGenerator::OutputSubDirectoryRules(ostream& fout) {
  // output rules for decending into sub directories
  const vector<string>& SubDirectories = m_Makefile->GetSubDirectories();
  //
  if ( SubDirectories.size() == 0) {
    return;
  }
  //
  this->OutputSubDirectoryVars(fout, "SUBDIR_BUILD", "build",
                               0,
                               0,
                               SubDirectories);
}
//---------------------------------------------------------------------------
// fix up names of directories so they can be used
// as targets in makefiles.
inline string FixDirectoryName(const char* dir)
{
  string s = dir;
  // replace ../ with 3 under bars
  size_t pos = s.find("../");
  if (pos != string::npos)
    {
    s.replace(pos, 3, "___");
    }
  // replace / directory separators with a single under bar
  pos = s.find("/");
  while(pos != string::npos)
    {
    s.replace(pos, 1, "_");
    pos = s.find("/");
    }
  return s;
}

void cmBorlandMakefileGenerator::OutputSubDirectoryVars(ostream& fout,
                       const char* var,
                       const char* target,
                       const char* target1,
                       const char* target2,
                       const vector<string>& SubDirectories)
{
  if (!SubDirectories.size()) return;
  //
  fout << "# Variable for making " << target << " in subdirectories.\n";
  fout << var << " = \\\n";
  unsigned int i;
  for (i =0; i < SubDirectories.size(); i++) {
    string subdir = FixDirectoryName(SubDirectories[i].c_str());
    fout << "  " << target << "_" << subdir.c_str();
    if (i == SubDirectories.size()-1) {
      fout << " \n\n";
    }
    else {
      fout << " \\\n";
    }
  }
  //
  fout << "# Targets for making " << target << " in subdirectories.\n";
  for (unsigned int i=0; i<SubDirectories.size(); i++) {
    string subdir = FixDirectoryName(SubDirectories[i].c_str());
    fout << target << "_" << subdir.c_str() << ":\n";
    fout << "  cd " << m_Makefile->GetStartOutputDirectory() << "/" << SubDirectories[i] << " \n";
    fout << "  make -fmakefile.mak \n\n";
  }
}



// Output each custom rule in the following format:
// output: source depends...
//   (tab)   command...

// This routine is copied direct from unix makefile generator
void cmBorlandMakefileGenerator::OutputCustomRules(ostream& fout) {
  // We may be modifying the source groups temporarily, so make a copy.
  std::vector<cmSourceGroup> sourceGroups = m_Makefile->GetSourceGroups();

  const cmTargets &tgts = m_Makefile->GetTargets();
  for(cmTargets::const_iterator tgt = tgts.begin();
      tgt != tgts.end(); ++tgt)
    {
    // add any custom rules to the source groups
    for (std::vector<cmCustomCommand>::const_iterator cr = 
           tgt->second.GetCustomCommands().begin(); 
         cr != tgt->second.GetCustomCommands().end(); ++cr)
      {
      cmSourceGroup& sourceGroup = 
        m_Makefile->FindSourceGroup(cr->GetSourceName().c_str(),
                                    sourceGroups);
      cmCustomCommand cc(*cr);
      cc.ExpandVariables(*m_Makefile);
      sourceGroup.AddCustomCommand(cc);
      }
    }

  // Loop through every source group.
  for(std::vector<cmSourceGroup>::const_iterator sg =
        sourceGroups.begin(); sg != sourceGroups.end(); ++sg)
    {
    const cmSourceGroup::BuildRules& buildRules = sg->GetBuildRules();
    if(buildRules.empty())
      { continue; }
    
    std::string name = sg->GetName();
    if(name != "")
      {
      fout << "# Start of source group \"" << name.c_str() << "\"\n";
      }
    
    // Loop through each source in the source group.
    for(cmSourceGroup::BuildRules::const_iterator cc =
          buildRules.begin(); cc != buildRules.end(); ++ cc)
      {
      std::string source = cc->first;
      const cmSourceGroup::Commands& commands = cc->second;
      // Loop through every command generating code from the current source.
      for(cmSourceGroup::Commands::const_iterator c = commands.begin();
          c != commands.end(); ++c)
        {
        std::string command = c->first;
        const cmSourceGroup::CommandFiles& commandFiles = c->second;
        // if the command has no outputs, then it is a utility command
        // with no outputs
        if(commandFiles.m_Outputs.size() == 0)
          {
        fout << source.c_str() << ": ";
        // Write out all the dependencies for this rule.
        for(std::set<std::string>::const_iterator d =
          commandFiles.m_Depends.begin();
        d != commandFiles.m_Depends.end(); ++d)
          {
        std::string dep = cmSystemTools::EscapeSpaces(d->c_str());
        fout << " " << dep.c_str();
          }
        fout << "\n\t" << command.c_str() << "\n\n";
          }
        // Write a rule for every output generated by this command.
        for(std::set<std::string>::const_iterator output =
              commandFiles.m_Outputs.begin();
            output != commandFiles.m_Outputs.end(); ++output)
          {
          std::string src = cmSystemTools::EscapeSpaces(source.c_str());
          fout << output->c_str() << ": " << src.c_str();
          // Write out all the dependencies for this rule.
          for(std::set<std::string>::const_iterator d =
                commandFiles.m_Depends.begin();
              d != commandFiles.m_Depends.end(); ++d)
            {
            std::string dep = cmSystemTools::EscapeSpaces(d->c_str());
            fout << " " << dep.c_str();
            }
          fout << "\n\t" << command.c_str() << "\n\n";
          }
        }
      }
    if(name != "")
      {
      fout << "# End of source group \"" << name.c_str() << "\"\n\n";
      }
    }

  fout << "# End Custom Rules \n";
}


void cmBorlandMakefileGenerator::OutputMakeRules(ostream& fout) {
  this->OutputMakeRule(fout,
                       "Rule to build c file(s)",
                       ".c.obj",
                       0,
                       "$(CPP) -n$(OBJDIR) {$< }");
  this->OutputMakeRule(fout,
                       "Rule to build cpp file(s)",
                       ".cpp.obj",
                       0,
                       "$(CPP) -n$(OBJDIR) {$< }");
  this->OutputMakeRule(fout,
                       "Rule to build cxx file(s)",
                       ".cxx.obj",
                       0,
                       "$(CPP) -Pcxx -n$(OBJDIR) {$< }");
  this->OutputMakeRule(fout,
                       "The project ALL rule",
                       "ALL",
                       "PREPARE ${TARGETS} ${SUBDIR_BUILD} ${CMAKE_COMMAND}",
                       0);
  this->OutputMakeRule(fout,
                       "Rule to build the makefile",
                       "makefile.mak",
                       "${CMAKE_COMMAND} ${CMAKE_MAKEFILE_SOURCES} ",
                       "${CMAKE_COMMAND} "
                       "-H${CMAKE_SOURCE_DIR} -B${CMAKE_BINARY_DIR}");
  this->OutputMakeRule(fout,
                       "Rebuild the cache",
                       "${CMAKE_BINARY_DIR}/CMakeCache.txt",
                       0,
                       "${CMAKE_COMMAND} "
                       "-H${CMAKE_SOURCE_DIR} -B${CMAKE_BINARY_DIR}");
  this->OutputMakeRule(fout,
                       "Rebuild cmake dummy rule",
                       "${CMAKE_COMMAND}",
                       0,
                       "echo \"cmake might be out of date\"");
  
}

void cmBorlandMakefileGenerator::OutputMakeRule(ostream& fout,
                                             const char* comment,
                                             const char* target,
                                             const char* depends,
                                             const char* command)
{
  string replace;
  if (comment) {
    replace = comment;
    m_Makefile->ExpandVariablesInString(replace);
    fout << "# " << comment << " \n";
  }
  //
  replace = target;
  m_Makefile->ExpandVariablesInString(replace);
  fout << replace.c_str() << ": ";
  if (depends) {
    replace = depends;
    m_Makefile->ExpandVariablesInString(replace);
    fout << replace.c_str();
  }
  fout << "\n";
  //
  if (command) {
    replace = command;
    m_Makefile->ExpandVariablesInString(replace);
    fout << "  " << replace.c_str() << " \n";
  }
  fout << "\n";
}


void cmBorlandMakefileGenerator::SetLocal (bool local) {
  if (local) {
    m_CacheOnly = false;
    m_Recurse = false;
  }
  else {
    m_CacheOnly = true;
    m_Recurse = true;
  }
}

void cmBorlandMakefileGenerator::ComputeSystemInfo() {
  // now load the settings
  if (!m_Makefile->GetDefinition("CMAKE_ROOT")) {
    cmSystemTools::Error("CMAKE_ROOT has not been defined, bad GUI or driver program");
    return;
  }
  string fpath = m_Makefile->GetDefinition("CMAKE_ROOT");
  fpath += "/Templates/CMakeWindowsBorlandConfig.cmake";
  m_Makefile->ReadListFile(NULL,fpath.c_str());
}
