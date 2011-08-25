#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmSystemTools.h"

# include <cmsys/Terminal.h>

#include "cmQtAutomoc.h"


cmQtAutomoc::cmQtAutomoc()
:Verbose(cmsys::SystemTools::GetEnv("VERBOSE") != 0)
,ColorOutput(true)
,RunMocFailed(false)
,GenerateAll(false)
{

  std::string colorEnv = "";
  cmsys::SystemTools::GetEnv("COLOR", colorEnv);
  if(!colorEnv.empty())
    {
    if(cmSystemTools::IsOn(colorEnv.c_str()))
      {
      this->ColorOutput = true;
      }
    else
      {
      this->ColorOutput = false;
      }
    }
}


void cmQtAutomoc::SetupAutomocTarget(cmTarget* target)
{
  cmMakefile* makefile = target->GetMakefile();
  const char* targetName = target->GetName();
  // don't do anything if there is no Qt4:
  std::string qtMajorVersion = makefile->GetSafeDefinition("QT_VERSION_MAJOR");
  if (qtMajorVersion != "4")
    {
    return;
    }

  // create a custom target for running automoc at buildtime:
  std::string automocTargetName = targetName;
  automocTargetName += "_automoc";

  std::string targetDir = makefile->GetCurrentOutputDirectory();
  targetDir += makefile->GetCMakeInstance()->GetCMakeFilesDirectory();
  targetDir += "/";
  targetDir += automocTargetName;
  targetDir += ".dir/";

  cmCustomCommandLine currentLine;
  currentLine.push_back(makefile->GetCMakeInstance()->GetCMakeCommand());
  currentLine.push_back("-E");
  currentLine.push_back("cmake_automoc");
  currentLine.push_back(targetDir);

  cmCustomCommandLines commandLines;
  commandLines.push_back(currentLine);

  std::string workingDirectory = cmSystemTools::CollapseFullPath(
                                    "", makefile->GetCurrentOutputDirectory());

  std::vector<std::string> depends;
  std::string automocComment = "Automoc for target ";
  automocComment += targetName;

  makefile->AddUtilityCommand(automocTargetName.c_str(), true,
                              workingDirectory.c_str(), depends,
                              commandLines, false, automocComment.c_str());
  target->AddUtility(automocTargetName.c_str());

  // configure a file to get all information to automoc at buildtime:
  std::string _moc_files;
  std::string _moc_headers;
  const char* sepFiles = "";
  const char* sepHeaders = "";

  const std::vector<cmSourceFile*>& srcFiles = target->GetSourceFiles();

  for(std::vector<cmSourceFile*>::const_iterator fileIt = srcFiles.begin();
      fileIt != srcFiles.end();
      ++fileIt)
    {
    cmSourceFile* sf = *fileIt;
    std::string absFile = sf->GetFullPath();
    bool skip = cmSystemTools::IsOn(sf->GetPropertyForUser("SKIP_AUTOMOC"));
    bool generated = cmSystemTools::IsOn(sf->GetPropertyForUser("GENERATED"));

    if ((skip==false) && (generated == false))
      {
      std::string ext = sf->GetExtension();
      cmSystemTools::FileFormat fileType = cmSystemTools::GetFileFormat(
                                                                  ext.c_str());
      if (fileType == cmSystemTools::CXX_FILE_FORMAT)
        {
        _moc_files += sepFiles;
        _moc_files += absFile;
        sepFiles = ";";
        }
      else if (fileType == cmSystemTools::HEADER_FILE_FORMAT)
        {
        _moc_headers += sepHeaders;
        _moc_headers += absFile;
        sepHeaders = ";";
        }
      }
    }

  std::string _moc_incs = makefile->GetProperty("INCLUDE_DIRECTORIES");
  std::string _moc_defs = makefile->GetProperty("DEFINITIONS");
  std::string _moc_compile_defs = makefile->GetProperty("COMPILE_DEFINITIONS");
  // forget the variables added here afterwards again:
  cmMakefile::ScopePushPop varScope(makefile);
  static_cast<void>(varScope);

  makefile->AddDefinition("_moc_target_name", automocTargetName.c_str());
  makefile->AddDefinition("_moc_incs", _moc_incs.c_str());
  makefile->AddDefinition("_moc_defs", _moc_defs.c_str());
  makefile->AddDefinition("_moc_compile_defs", _moc_compile_defs.c_str());
  makefile->AddDefinition("_moc_files", _moc_files.c_str());
  makefile->AddDefinition("_moc_headers", _moc_headers.c_str());

  const char* cmakeRoot = makefile->GetDefinition("CMAKE_ROOT");
  std::string inputFile = cmakeRoot;
  inputFile += "/Modules/AutomocInfo.cmake.in";
  std::string outputFile = targetDir;
  outputFile += "/AutomocInfo.cmake";
  makefile->ConfigureFile(inputFile.c_str(), outputFile.c_str(),
                          false, true, false);

  std::string mocCppFile =  makefile->GetCurrentOutputDirectory();
  mocCppFile += "/";
  mocCppFile += automocTargetName;
  mocCppFile += ".cpp";
  cmSourceFile* mocCppSource = makefile->GetOrCreateSource(mocCppFile.c_str(),
                                                         true);
  target->AddSourceFile(mocCppSource);

  makefile->AppendProperty("ADDITIONAL_MAKE_CLEAN_FILES",
                           mocCppFile.c_str(), false);
}


bool cmQtAutomoc::Run(const char* targetDirectory)
{
  cmake cm;
  cmGlobalGenerator* gg = this->CreateGlobalGenerator(&cm, targetDirectory);
  cmMakefile* makefile = gg->GetCurrentLocalGenerator()->GetMakefile();

  this->ReadAutomocInfoFile(makefile, targetDirectory);
  this->ReadOldMocDefinitionsFile(makefile, targetDirectory);

  this->Init();

  if (this->QtMajorVersion == "4")
    {
    this->RunAutomocQt4();
    }

  this->WriteOldMocDefinitionsFile(targetDirectory);

  delete gg;
  gg = NULL;
  makefile = NULL;
  return true;
}


cmGlobalGenerator* cmQtAutomoc::CreateGlobalGenerator(cmake* cm,
                                                  const char* targetDirectory)
{
  cmGlobalGenerator* gg = new cmGlobalGenerator();
  gg->SetCMakeInstance(cm);

  cmLocalGenerator* lg = gg->CreateLocalGenerator();
  lg->GetMakefile()->SetHomeOutputDirectory(targetDirectory);
  lg->GetMakefile()->SetStartOutputDirectory(targetDirectory);
  lg->GetMakefile()->SetHomeDirectory(targetDirectory);
  lg->GetMakefile()->SetStartDirectory(targetDirectory);
  gg->SetCurrentLocalGenerator(lg);

  return gg;
}


bool cmQtAutomoc::ReadAutomocInfoFile(cmMakefile* makefile,
                                      const char* targetDirectory)
{
  std::string filename(cmSystemTools::CollapseFullPath(targetDirectory));
  cmSystemTools::ConvertToUnixSlashes(filename);
  filename += "/AutomocInfo.cmake";

  if (!makefile->ReadListFile(0, filename.c_str()))
    {
    cmSystemTools::Error("Error processing file:", filename.c_str());
    return false;
    }

  this->QtMajorVersion = makefile->GetSafeDefinition("AM_QT_VERSION_MAJOR");
  this->Sources = makefile->GetSafeDefinition("AM_SOURCES");
  this->Headers = makefile->GetSafeDefinition("AM_HEADERS");
  this->IncludeProjectDirsBefore = makefile->IsOn(
                                "AM_CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE");
  this->Srcdir = makefile->GetSafeDefinition("AM_CMAKE_CURRENT_SOURCE_DIR");
  this->Builddir = makefile->GetSafeDefinition("AM_CMAKE_CURRENT_BINARY_DIR");
  this->MocExecutable = makefile->GetSafeDefinition("AM_QT_MOC_EXECUTABLE");
  this->MocCompileDefinitionsStr = makefile->GetSafeDefinition(
                                                 "AM_MOC_COMPILE_DEFINITIONS");
  this->MocDefinitionsStr = makefile->GetSafeDefinition("AM_MOC_DEFINITIONS");
  this->MocIncludesStr = makefile->GetSafeDefinition("AM_MOC_INCLUDES");
  this->ProjectBinaryDir = makefile->GetSafeDefinition("AM_CMAKE_BINARY_DIR");
  this->ProjectSourceDir = makefile->GetSafeDefinition("AM_CMAKE_SOURCE_DIR");
  this->TargetName = makefile->GetSafeDefinition("AM_TARGET_NAME");

  return true;
}


bool cmQtAutomoc::ReadOldMocDefinitionsFile(cmMakefile* makefile,
                                            const char* targetDirectory)
{
  std::string filename(cmSystemTools::CollapseFullPath(targetDirectory));
  cmSystemTools::ConvertToUnixSlashes(filename);
  filename += "/AutomocOldMocDefinitions.cmake";

  if (makefile->ReadListFile(0, filename.c_str()))
    {
    this->OldMocDefinitionsStr =
                         makefile->GetSafeDefinition("AM_OLD_MOC_DEFINITIONS");
    }
  return true;
}


void cmQtAutomoc::WriteOldMocDefinitionsFile(const char* targetDirectory)
{
  std::string filename(cmSystemTools::CollapseFullPath(targetDirectory));
  cmSystemTools::ConvertToUnixSlashes(filename);
  filename += "/AutomocOldMocDefinitions.cmake";

  std::fstream outfile;
  outfile.open(filename.c_str(),
               std::ios::out | std::ios::trunc);
  outfile << "set(AM_OLD_MOC_DEFINITIONS \""
              << this->Join(this->MocDefinitions, ' ') << "\")\n";

  outfile.close();
}


void cmQtAutomoc::Init()
{
  this->OutMocCppFilename = this->Builddir;
  this->OutMocCppFilename += this->TargetName;
  this->OutMocCppFilename += ".cpp";

  std::vector<std::string> cdefList;
  cmSystemTools::ExpandListArgument(this->MocCompileDefinitionsStr, cdefList);
  if (!cdefList.empty())
    {
    for(std::vector<std::string>::const_iterator it = cdefList.begin();
        it != cdefList.end();
        ++it)
      {
      this->MocDefinitions.push_back("-D" + (*it));
      }
    }
  else
    {
    std::string tmpMocDefs = this->MocDefinitionsStr;
    cmSystemTools::ReplaceString(tmpMocDefs, " ", ";");

    std::vector<std::string> defList;
    cmSystemTools::ExpandListArgument(tmpMocDefs, defList);

    for(std::vector<std::string>::const_iterator it = defList.begin();
        it != defList.end();
        ++it)
      {
      if (this->StartsWith(*it, "-D"))
        {
        this->MocDefinitions.push_back(*it);
        }
      }
    }

  std::vector<std::string> incPaths;
  cmSystemTools::ExpandListArgument(this->MocIncludesStr, incPaths);

  std::set<std::string> frameworkPaths;
  for(std::vector<std::string>::const_iterator it = incPaths.begin();
      it != incPaths.end();
      ++it)
    {
    const std::string &path = *it;
    this->MocIncludes.push_back("-I" + path);
    if (this->EndsWith(path, ".framework/Headers"))
      {
      // Go up twice to get to the framework root
      std::vector<std::string> pathComponents;
      cmsys::SystemTools::SplitPath(path.c_str(), pathComponents);
      std::string frameworkPath =cmsys::SystemTools::JoinPath(
                             pathComponents.begin(), pathComponents.end() - 2);
      frameworkPaths.insert(frameworkPath);
      }
    }

  for (std::set<std::string>::const_iterator it = frameworkPaths.begin();
         it != frameworkPaths.end(); ++it)
    {
    this->MocIncludes.push_back("-F");
    this->MocIncludes.push_back(*it);
    }


    if (this->IncludeProjectDirsBefore)
      {
      const std::string &binDir = "-I" + this->ProjectBinaryDir;

      const std::string srcDir = "-I" + this->ProjectSourceDir;

      std::list<std::string> sortedMocIncludes;
      std::list<std::string>::iterator it = this->MocIncludes.begin();
      while (it != this->MocIncludes.end())
        {
        if (this->StartsWith(*it, binDir))
          {
          sortedMocIncludes.push_back(*it);
          it = this->MocIncludes.erase(it);
          }
        else
          {
          ++it;
          }
        }
      it = this->MocIncludes.begin();
      while (it != this->MocIncludes.end())
        {
        if (this->StartsWith(*it, srcDir))
          {
          sortedMocIncludes.push_back(*it);
          it = this->MocIncludes.erase(it);
          }
        else
          {
          ++it;
          }
        }
      sortedMocIncludes.insert(sortedMocIncludes.end(),
                           this->MocIncludes.begin(), this->MocIncludes.end());
      this->MocIncludes = sortedMocIncludes;
    }

}


bool cmQtAutomoc::RunAutomocQt4()
{
  if (!cmsys::SystemTools::FileExists(this->OutMocCppFilename.c_str())
    || (this->OldMocDefinitionsStr != this->Join(this->MocDefinitions, ' ')))
    {
    this->GenerateAll = true;
    }

  // the program goes through all .cpp files to see which moc files are
  // included. It is not really interesting how the moc file is named, but
  // what file the moc is created from. Once a moc is included the same moc
  // may not be included in the _automoc.cpp file anymore. OTOH if there's a
  // header containing Q_OBJECT where no corresponding moc file is included
  // anywhere a moc_<filename>.cpp file is created and included in
  // the _automoc.cpp file.

  // key = moc source filepath, value = moc output filepath
  std::map<std::string, std::string> includedMocs;
  // key = moc source filepath, value = moc output filename
  std::map<std::string, std::string> notIncludedMocs;


  std::vector<std::string> sourceFiles;
  cmSystemTools::ExpandListArgument(this->Sources, sourceFiles);

  for (std::vector<std::string>::const_iterator it = sourceFiles.begin();
       it != sourceFiles.end();
       ++it)
    {
    const std::string &absFilename = *it;
    if (this->Verbose)
      {
      std::cout << "AUTOMOC: Checking " << absFilename << std::endl;
      }
    this->ParseCppFile(absFilename, includedMocs, notIncludedMocs);
    }

  std::vector<std::string> headerFiles;
  cmSystemTools::ExpandListArgument(this->Headers, headerFiles);
  for (std::vector<std::string>::const_iterator it = headerFiles.begin();
       it != headerFiles.end();
       ++it)
    {
    const std::string &absFilename = *it;
    if (this->Verbose)
      {
      std::cout << "AUTOMOC: Checking " << absFilename << std::endl;
      }
    if (includedMocs.find(absFilename) == includedMocs.end()
              && notIncludedMocs.find(absFilename) == notIncludedMocs.end())
      {
      // if this header is not getting processed yet and is explicitly
      // mentioned for the automoc the moc is run unconditionally on the
      // header and the resulting file is included in the _automoc.cpp file
      // (unless there's a .cpp file later on that includes the moc from
      // this header)
      const std::string currentMoc = "moc_" + cmsys::SystemTools::
                         GetFilenameWithoutLastExtension(absFilename) + ".cpp";
      notIncludedMocs[absFilename] = currentMoc;
      }
    }

  // run moc on all the moc's that are #included in source files
  for(std::map<std::string, std::string>::const_iterator
                                                     it = includedMocs.begin();
      it != includedMocs.end();
      ++it)
    {
    this->GenerateMoc(it->first, it->second);
    }

  std::stringstream outStream(std::stringstream::out);
  outStream << "/* This file is autogenerated, do not edit*/\n";

  bool automocCppChanged = false;
  if (notIncludedMocs.empty())
    {
    outStream << "enum some_compilers { need_more_than_nothing };\n";
    }
  else
    {
    // run moc on the remaining headers and include them in
    // the _automoc.cpp file
    for(std::map<std::string, std::string>::const_iterator
                                                  it = notIncludedMocs.begin();
        it != notIncludedMocs.end();
        ++it)
      {
      bool mocSuccess = this->GenerateMoc(it->first, it->second);
      if (mocSuccess)
        {
        automocCppChanged = true;
        }
      outStream << "#include \"" << it->second << "\"\n";
      }
    }

  if (this->RunMocFailed)
    {
    std::cerr << "returning failed.."<< std::endl;
    return false;
    }
  outStream.flush();
  std::string automocSource = outStream.str();
  if (!automocCppChanged)
    {
    // compare contents of the _automoc.cpp file
    const std::string oldContents = this->ReadAll(this->OutMocCppFilename);
    if (oldContents == automocSource)
      {
      // nothing changed: don't touch the _automoc.cpp file
      return true;
      }
    }

  // source file that includes all remaining moc files (_automoc.cpp file)
  std::fstream outfile;
  outfile.open(this->OutMocCppFilename.c_str(),
               std::ios::out | std::ios::trunc);
  outfile << automocSource;
  outfile.close();

  return true;
}


void cmQtAutomoc::ParseCppFile(const std::string& absFilename,
                           std::map<std::string, std::string>& includedMocs,
                           std::map<std::string, std::string>& notIncludedMocs)
{
  cmsys::RegularExpression mocIncludeRegExp(
              "[\n][ \t]*#[ \t]*include[ \t]+"
              "[\"<](([^ \">]+/)?moc_[^ \">/]+\\.cpp|[^ \">]+\\.moc)[\">]");
  cmsys::RegularExpression qObjectRegExp("[\n][ \t]*Q_OBJECT[^a-zA-Z0-9_]");
  std::list<std::string> headerExtensions;
  headerExtensions.push_back(".h");
  headerExtensions.push_back(".hpp");
  headerExtensions.push_back(".hxx");
#if defined(_WIN32)
  // not case sensitive, don't add ".H"
#elif defined(__APPLE__)
  // detect case-sensitive filesystem
  long caseSensitive = pathconf(this->Srcdir.c_str(), _PC_CASE_SENSITIVE);
  if (caseSensitive == 1)
  {
    headerExtensions.push_back(".H");
  }
#else
  headerExtensions.push_back(".H");
#endif

  const std::string contentsString = this->ReadAll(absFilename);
  if (contentsString.empty())
    {
    std::cerr << "AUTOMOC: empty source file: " << absFilename << std::endl;
    return;
    }
  const std::string absPath = cmsys::SystemTools::GetFilenamePath(
                   cmsys::SystemTools::GetRealPath(absFilename.c_str())) + '/';

  std::string::size_type matchOffset = 0;
  if (!mocIncludeRegExp.find(contentsString.c_str()))
    {
    // no moc #include, look whether we need to create a moc from
    // the .h nevertheless
    const std::string basename =
              cmsys::SystemTools::GetFilenameWithoutLastExtension(absFilename);
    for(std::list<std::string>::const_iterator ext = headerExtensions.begin();
        ext != headerExtensions.end();
        ++ext)
      {
      const std::string headername = absPath + basename + (*ext);
      if (cmsys::SystemTools::FileExists(headername.c_str())
              && includedMocs.find(headername) == includedMocs.end()
              && notIncludedMocs.find(headername) == notIncludedMocs.end())
        {
        const std::string currentMoc = "moc_" + basename + ".cpp";
        const std::string contents = this->ReadAll(headername);
        if (qObjectRegExp.find(contents))
          {
          //std::cout << "header contains Q_OBJECT macro";
          notIncludedMocs[headername] = currentMoc;
          }
        break;
        }
      }
    for(std::list<std::string>::const_iterator ext = headerExtensions.begin();
        ext != headerExtensions.end();
        ++ext)
      {
      const std::string privateHeaderName = absPath+basename+"_p"+(*ext);
      if (cmsys::SystemTools::FileExists(privateHeaderName.c_str())
          && includedMocs.find(privateHeaderName) == includedMocs.end()
          && notIncludedMocs.find(privateHeaderName) == notIncludedMocs.end())
        {
        const std::string currentMoc = "moc_" + basename + "_p.cpp";
        const std::string contents = this->ReadAll(privateHeaderName);
        if (qObjectRegExp.find(contents))
          {
          //std::cout << "header contains Q_OBJECT macro";
          notIncludedMocs[privateHeaderName] = currentMoc;
          }
        break;
        }
      }
    }
  else
    {
    // for every moc include in the file
    do
      {
      const std::string currentMoc = mocIncludeRegExp.match(1);
      //std::cout << "found moc include: " << currentMoc << std::endl;

      std::string basename = cmsys::SystemTools::
                                   GetFilenameWithoutLastExtension(currentMoc);
      const bool moc_style = this->StartsWith(basename, "moc_");

      // If the moc include is of the moc_foo.cpp style we expect
      // the Q_OBJECT class declaration in a header file.
      // If the moc include is of the foo.moc style we need to look for
      // a Q_OBJECT macro in the current source file, if it contains the
      // macro we generate the moc file from the source file, else from the
      // header.
      // Q_OBJECT
      if (moc_style || !qObjectRegExp.find(contentsString))
        {
        if (moc_style)
          {
          // basename should be the part of the moc filename used for
          // finding the correct header, so we need to remove the moc_ part
          basename = basename.substr(4);
          }

        bool headerFound = false;
        for(std::list<std::string>::const_iterator ext =
                                                      headerExtensions.begin();
            ext != headerExtensions.end();
            ++ext)
          {
          const std::string &sourceFilePath = absPath + basename + (*ext);
          if (cmsys::SystemTools::FileExists(sourceFilePath.c_str()))
            {
            headerFound = true;
            includedMocs[sourceFilePath] = currentMoc;
            notIncludedMocs.erase(sourceFilePath);
            break;
            }
          }
        if (!headerFound)
          {
          // the moc file is in a subdir => look for the header in the
          // same subdir
          if (currentMoc.find_first_of('/') != std::string::npos)
            {
            const std::string &filepath = absPath
                    + cmsys::SystemTools::GetFilenamePath(currentMoc)
                    + '/' + basename;

            for(std::list<std::string>::const_iterator ext =
                                                      headerExtensions.begin();
                ext != headerExtensions.end();
                ++ext)
              {
              const std::string &sourceFilePath = filepath + (*ext);
              if (cmsys::SystemTools::FileExists(sourceFilePath.c_str()))
                {
                headerFound = true;
                includedMocs[sourceFilePath] = currentMoc;
                notIncludedMocs.erase(sourceFilePath);
                break;
                }
              }
            if (!headerFound)
              {
              std::cerr << "AUTOMOC: The file \"" << absFilename
                        << "\" includes the moc file \"" << currentMoc
                        << "\", but neither \"" << absPath << basename
                        << '{' << this->Join(headerExtensions, ',')
                        << "}\" nor \"" << filepath << '{'
                        << this->Join(headerExtensions, ',') << '}'
                        << "\" exist." << std::endl;
              ::exit(EXIT_FAILURE);
              }
            }
          else
            {
            std::cerr << "AUTOMOC: The file \"" << absFilename
                      << "\" includes the moc file \"" << currentMoc
                      << "\", but \"" << absPath << basename << '{'
                      << this->Join(headerExtensions, ',') << '}'
                      << "\" does not exist." << std::endl;
            ::exit(EXIT_FAILURE);
            }
          }
        }
      else
        {
        includedMocs[absFilename] = currentMoc;
        notIncludedMocs.erase(absFilename);
        }
      matchOffset += mocIncludeRegExp.end();
      } while(mocIncludeRegExp.find(contentsString.c_str() + matchOffset));
    }
}


bool cmQtAutomoc::GenerateMoc(const std::string& sourceFile,
                              const std::string& mocFileName)
{
  const std::string mocFilePath = this->Builddir + mocFileName;
  int sourceNewerThanMoc = 0;
  bool success = cmsys::SystemTools::FileTimeCompare(sourceFile.c_str(),
                                                     mocFilePath.c_str(),
                                                     &sourceNewerThanMoc);
  if (this->GenerateAll || !success || sourceNewerThanMoc >= 0)
    {
    // make sure the directory for the resulting moc file exists
    std::string mocDir = mocFilePath.substr(0, mocFilePath.rfind('/'));
    if (!cmsys::SystemTools::FileExists(mocDir.c_str(), false))
      {
      cmsys::SystemTools::MakeDirectory(mocDir.c_str());
      }

    std::string msg = "Generating ";
    msg += mocFileName;
    cmSystemTools::MakefileColorEcho(cmsysTerminal_Color_ForegroundBlue
                                           |cmsysTerminal_Color_ForegroundBold,
                                     msg.c_str(), true, this->ColorOutput);

    std::vector<cmStdString> command;
    command.push_back(this->MocExecutable);
    for (std::list<std::string>::const_iterator it = this->MocIncludes.begin();
         it != this->MocIncludes.end();
         ++it)
      {
      command.push_back(*it);
      }
    for(std::list<std::string>::const_iterator it=this->MocDefinitions.begin();
        it != this->MocDefinitions.end();
        ++it)
      {
      command.push_back(*it);
      }
#ifdef _WIN32
    command.push_back("-DWIN32");
#endif
    command.push_back("-o");
    command.push_back(mocFilePath);
    command.push_back(sourceFile);

    if (this->Verbose)
      {
      for(std::vector<cmStdString>::const_iterator cmdIt = command.begin();
          cmdIt != command.end();
          ++cmdIt)
        {
        std::cout << *cmdIt << " ";
        }
      std::cout << std::endl;
      }

    std::string output;
    int retVal = 0;
    bool result = cmSystemTools::RunSingleCommand(command, &output, &retVal);
    if (!result || retVal)
      {
      std::cerr << "AUTOMOC: process for " << mocFilePath << " failed:\n"
                << output << std::endl;
      this->RunMocFailed = true;
      cmSystemTools::RemoveFile(mocFilePath.c_str());
      }
    return true;
    }
  return false;
}


std::string cmQtAutomoc::Join(const std::list<std::string>& lst,char separator)
{
    if (lst.empty())
      {
      return "";
      }

    std::string result;
    for (std::list<std::string>::const_iterator it = lst.begin();
         it != lst.end();
         ++it)
      {
      result += (*it) + separator;
      }
    result.erase(result.end() - 1);
    return result;
}


bool cmQtAutomoc::StartsWith(const std::string& str, const std::string& with)
{
  return (str.substr(0, with.length()) == with);
}


bool cmQtAutomoc::EndsWith(const std::string& str, const std::string& with)
{
  if (with.length() > (str.length()))
    {
    return false;
    }
  return (str.substr(str.length() - with.length(), with.length()) == with);
}


std::string cmQtAutomoc::ReadAll(const std::string& filename)
{
  std::ifstream file(filename.c_str());
  std::stringstream stream;
  stream << file.rdbuf();
  file.close();
  return stream.str();
}
