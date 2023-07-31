/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmDependsFortran.h"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <map>
#include <type_traits>
#include <utility>

#include "cmsys/FStream.hxx"

#include "cmFortranParser.h" /* Interface to parser object.  */
#include "cmGeneratedFileStream.h"
#include "cmGlobalUnixMakefileGenerator3.h"
#include "cmList.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmOutputConverter.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"

// TODO: Test compiler for the case of the mod file.  Some always
// use lower case and some always use upper case.  I do not know if any
// use the case from the source code.

static void cmFortranModuleAppendUpperLower(std::string const& mod,
                                            std::string& mod_upper,
                                            std::string& mod_lower)
{
  std::string::size_type ext_len = 0;
  if (cmHasLiteralSuffix(mod, ".mod") || cmHasLiteralSuffix(mod, ".sub")) {
    ext_len = 4;
  } else if (cmHasLiteralSuffix(mod, ".smod")) {
    ext_len = 5;
  }
  std::string const& name = mod.substr(0, mod.size() - ext_len);
  std::string const& ext = mod.substr(mod.size() - ext_len);
  mod_upper += cmSystemTools::UpperCase(name) + ext;
  mod_lower += mod;
}

class cmDependsFortranInternals
{
public:
  // The set of modules provided by this target.
  std::set<std::string> TargetProvides;

  // Map modules required by this target to locations.
  using TargetRequiresMap = std::map<std::string, std::string>;
  TargetRequiresMap TargetRequires;

  // Information about each object file.
  using ObjectInfoMap = std::map<std::string, cmFortranSourceInfo>;
  ObjectInfoMap ObjectInfo;

  cmFortranSourceInfo& CreateObjectInfo(const std::string& obj,
                                        const std::string& src)
  {
    auto i = this->ObjectInfo.find(obj);
    if (i == this->ObjectInfo.end()) {
      std::map<std::string, cmFortranSourceInfo>::value_type entry(
        obj, cmFortranSourceInfo());
      i = this->ObjectInfo.insert(entry).first;
      i->second.Source = src;
    }
    return i->second;
  }
};

cmDependsFortran::cmDependsFortran() = default;

cmDependsFortran::cmDependsFortran(cmLocalUnixMakefileGenerator3* lg)
  : cmDepends(lg)
  , Internal(new cmDependsFortranInternals)
{
  // Configure the include file search path.
  this->SetIncludePathFromLanguage("Fortran");

  // Get the list of definitions.
  cmMakefile* mf = this->LocalGenerator->GetMakefile();
  cmList definitions{ mf->GetDefinition("CMAKE_TARGET_DEFINITIONS_Fortran") };

  // translate i.e. FOO=BAR to FOO and add it to the list of defined
  // preprocessor symbols
  for (std::string def : definitions) {
    std::string::size_type assignment = def.find('=');
    if (assignment != std::string::npos) {
      def = def.substr(0, assignment);
    }
    this->PPDefinitions.insert(def);
  }

  this->CompilerId = mf->GetSafeDefinition("CMAKE_Fortran_COMPILER_ID");
  this->SModSep = mf->GetSafeDefinition("CMAKE_Fortran_SUBMODULE_SEP");
  this->SModExt = mf->GetSafeDefinition("CMAKE_Fortran_SUBMODULE_EXT");
}

cmDependsFortran::~cmDependsFortran() = default;

bool cmDependsFortran::WriteDependencies(const std::set<std::string>& sources,
                                         const std::string& obj,
                                         std::ostream& /*makeDepends*/,
                                         std::ostream& /*internalDepends*/)
{
  // Make sure this is a scanning instance.
  if (sources.empty() || sources.begin()->empty()) {
    cmSystemTools::Error("Cannot scan dependencies without a source file.");
    return false;
  }
  if (obj.empty()) {
    cmSystemTools::Error("Cannot scan dependencies without an object file.");
    return false;
  }

  cmFortranCompiler fc;
  fc.Id = this->CompilerId;
  fc.SModSep = this->SModSep;
  fc.SModExt = this->SModExt;

  bool okay = true;
  for (std::string const& src : sources) {
    // Get the information object for this source.
    cmFortranSourceInfo& info = this->Internal->CreateObjectInfo(obj, src);

    // Create the parser object. The constructor takes info by reference,
    // so we may look into the resulting objects later.
    cmFortranParser parser(fc, this->IncludePath, this->PPDefinitions, info);

    // Push on the starting file.
    cmFortranParser_FilePush(&parser, src.c_str());

    // Parse the translation unit.
    if (cmFortran_yyparse(parser.Scanner) != 0) {
      // Failed to parse the file.  Report failure to write dependencies.
      okay = false;
      /* clang-format off */
      std::cerr <<
        "warning: failed to parse dependencies from Fortran source "
        "'" << src << "': " << parser.Error << std::endl
        ;
      /* clang-format on */
    }
  }
  return okay;
}

bool cmDependsFortran::Finalize(std::ostream& makeDepends,
                                std::ostream& internalDepends)
{
  // Prepare the module search process.
  if (!this->LocateModules()) {
    return false;
  }

  // Get the directory in which stamp files will be stored.
  const std::string& stamp_dir = this->TargetDirectory;

  // Get the directory in which module files will be created.
  cmMakefile* mf = this->LocalGenerator->GetMakefile();
  std::string mod_dir =
    mf->GetSafeDefinition("CMAKE_Fortran_TARGET_MODULE_DIR");
  if (mod_dir.empty()) {
    mod_dir = this->LocalGenerator->GetCurrentBinaryDirectory();
  }

  bool building_intrinsics =
    !mf->GetSafeDefinition("CMAKE_Fortran_TARGET_BUILDING_INSTRINSIC_MODULES")
       .empty();

  // Actually write dependencies to the streams.
  using ObjectInfoMap = cmDependsFortranInternals::ObjectInfoMap;
  ObjectInfoMap const& objInfo = this->Internal->ObjectInfo;
  for (auto const& i : objInfo) {
    if (!this->WriteDependenciesReal(i.first, i.second, mod_dir, stamp_dir,
                                     makeDepends, internalDepends,
                                     building_intrinsics)) {
      return false;
    }
  }

  // Store the list of modules provided by this target.
  std::string fiName = cmStrCat(this->TargetDirectory, "/fortran.internal");
  cmGeneratedFileStream fiStream(fiName);
  fiStream << "# The fortran modules provided by this target.\n";
  fiStream << "provides\n";
  std::set<std::string> const& provides = this->Internal->TargetProvides;
  for (std::string const& i : provides) {
    fiStream << ' ' << i << '\n';
  }

  // Create a script to clean the modules.
  if (!provides.empty()) {
    std::string fcName =
      cmStrCat(this->TargetDirectory, "/cmake_clean_Fortran.cmake");
    cmGeneratedFileStream fcStream(fcName);
    fcStream << "# Remove fortran modules provided by this target.\n";
    fcStream << "FILE(REMOVE";
    for (std::string const& i : provides) {
      std::string mod_upper = cmStrCat(mod_dir, '/');
      std::string mod_lower = cmStrCat(mod_dir, '/');
      cmFortranModuleAppendUpperLower(i, mod_upper, mod_lower);
      std::string stamp = cmStrCat(stamp_dir, '/', i, ".stamp");
      fcStream << "\n"
                  "  \""
               << this->LocalGenerator->MaybeRelativeToCurBinDir(mod_lower)
               << "\"\n"
                  "  \""
               << this->LocalGenerator->MaybeRelativeToCurBinDir(mod_upper)
               << "\"\n"
                  "  \""
               << this->LocalGenerator->MaybeRelativeToCurBinDir(stamp)
               << "\"\n";
    }
    fcStream << "  )\n";
  }
  return true;
}

bool cmDependsFortran::LocateModules()
{
  // Collect the set of modules provided and required by all sources.
  using ObjectInfoMap = cmDependsFortranInternals::ObjectInfoMap;
  ObjectInfoMap const& objInfo = this->Internal->ObjectInfo;
  for (auto const& infoI : objInfo) {
    cmFortranSourceInfo const& info = infoI.second;
    // Include this module in the set provided by this target.
    this->Internal->TargetProvides.insert(info.Provides.begin(),
                                          info.Provides.end());

    for (std::string const& r : info.Requires) {
      this->Internal->TargetRequires[r].clear();
    }
  }

  // Short-circuit for simple targets.
  if (this->Internal->TargetRequires.empty()) {
    return true;
  }

  // Match modules provided by this target to those it requires.
  this->MatchLocalModules();

  // Load information about other targets.
  cmMakefile* mf = this->LocalGenerator->GetMakefile();
  cmList infoFiles{ mf->GetDefinition(
    "CMAKE_Fortran_TARGET_LINKED_INFO_FILES") };
  for (auto const& i : infoFiles) {
    std::string targetDir = cmSystemTools::GetFilenamePath(i);
    std::string fname = targetDir + "/fortran.internal";
    cmsys::ifstream fin(fname.c_str());
    if (!fin) {
      cmSystemTools::Error(cmStrCat("-E cmake_depends failed to open ", fname,
                                    " for module information"));
      return false;
    }
    this->MatchRemoteModules(fin, targetDir);
  }
  return true;
}

void cmDependsFortran::MatchLocalModules()
{
  std::string const& stampDir = this->TargetDirectory;
  std::set<std::string> const& provides = this->Internal->TargetProvides;
  for (std::string const& i : provides) {
    this->ConsiderModule(i, stampDir);
  }
}

void cmDependsFortran::MatchRemoteModules(std::istream& fin,
                                          const std::string& stampDir)
{
  std::string line;
  bool doing_provides = false;
  while (cmSystemTools::GetLineFromStream(fin, line)) {
    // Ignore comments and empty lines.
    if (line.empty() || line[0] == '#' || line[0] == '\r') {
      continue;
    }

    if (line[0] == ' ') {
      if (doing_provides) {
        std::string mod = line;
        if (!cmHasLiteralSuffix(mod, ".mod") &&
            !cmHasLiteralSuffix(mod, ".smod") &&
            !cmHasLiteralSuffix(mod, ".sub")) {
          // Support fortran.internal files left by older versions of CMake.
          // They do not include the ".mod" extension.
          mod += ".mod";
        }
        this->ConsiderModule(mod.substr(1), stampDir);
      }
    } else if (line == "provides") {
      doing_provides = true;
    } else {
      doing_provides = false;
    }
  }
}

void cmDependsFortran::ConsiderModule(const std::string& name,
                                      const std::string& stampDir)
{
  // Locate each required module.
  auto required = this->Internal->TargetRequires.find(name);
  if (required != this->Internal->TargetRequires.end() &&
      required->second.empty()) {
    // The module is provided by a CMake target.  It will have a stamp file.
    std::string stampFile = cmStrCat(stampDir, '/', name, ".stamp");
    required->second = stampFile;
  }
}

bool cmDependsFortran::WriteDependenciesReal(std::string const& obj,
                                             cmFortranSourceInfo const& info,
                                             std::string const& mod_dir,
                                             std::string const& stamp_dir,
                                             std::ostream& makeDepends,
                                             std::ostream& internalDepends,
                                             bool buildingIntrinsics)
{
  // Get the source file for this object.
  std::string const& src = info.Source;

  // Write the include dependencies to the output stream.
  std::string obj_i = this->LocalGenerator->MaybeRelativeToTopBinDir(obj);
  std::string obj_m = cmSystemTools::ConvertToOutputPath(obj_i);
  internalDepends << obj_i << "\n " << src << '\n';
  if (!info.Includes.empty()) {
    const auto& lineContinue = static_cast<cmGlobalUnixMakefileGenerator3*>(
                                 this->LocalGenerator->GetGlobalGenerator())
                                 ->LineContinueDirective;
    bool supportLongLineDepend = static_cast<cmGlobalUnixMakefileGenerator3*>(
                                   this->LocalGenerator->GetGlobalGenerator())
                                   ->SupportsLongLineDependencies();
    if (supportLongLineDepend) {
      makeDepends << obj_m << ':';
    }
    for (std::string const& i : info.Includes) {
      std::string dependee = cmSystemTools::ConvertToOutputPath(
        this->LocalGenerator->MaybeRelativeToTopBinDir(i));
      if (supportLongLineDepend) {
        makeDepends << ' ' << lineContinue << ' ' << dependee;
      } else {
        makeDepends << obj_m << ": " << dependee << '\n';
      }
      internalDepends << ' ' << i << '\n';
    }
    makeDepends << '\n';
  }

  std::set<std::string> req = info.Requires;
  if (buildingIntrinsics) {
    req.insert(info.Intrinsics.begin(), info.Intrinsics.end());
  }

  // Write module requirements to the output stream.
  for (std::string const& i : req) {
    // Require only modules not provided in the same source.
    if (info.Provides.find(i) != info.Provides.cend()) {
      continue;
    }

    // The object file should depend on timestamped files for the
    // modules it uses.
    auto required = this->Internal->TargetRequires.find(i);
    if (required == this->Internal->TargetRequires.end()) {
      abort();
    }
    if (!required->second.empty()) {
      // This module is known.  Depend on its timestamp file.
      std::string stampFile = cmSystemTools::ConvertToOutputPath(
        this->LocalGenerator->MaybeRelativeToTopBinDir(required->second));
      makeDepends << obj_m << ": " << stampFile << '\n';
    } else {
      // This module is not known to CMake.  Try to locate it where
      // the compiler will and depend on that.
      std::string module;
      if (this->FindModule(i, module)) {
        module = cmSystemTools::ConvertToOutputPath(
          this->LocalGenerator->MaybeRelativeToTopBinDir(module));
        makeDepends << obj_m << ": " << module << '\n';
      }
    }
  }

  // If any modules are provided then they must be converted to stamp files.
  if (!info.Provides.empty()) {
    // Create a target to copy the module after the object file
    // changes.
    for (std::string const& i : info.Provides) {
      // Include this module in the set provided by this target.
      this->Internal->TargetProvides.insert(i);

      // Always use lower case for the mod stamp file name.  The
      // cmake_copy_f90_mod will call back to this class, which will
      // try various cases for the real mod file name.
      std::string modFile = cmStrCat(mod_dir, '/', i);
      modFile = this->LocalGenerator->ConvertToOutputFormat(
        this->LocalGenerator->MaybeRelativeToTopBinDir(modFile),
        cmOutputConverter::SHELL);
      std::string stampFile = cmStrCat(stamp_dir, '/', i, ".stamp");
      stampFile = this->LocalGenerator->MaybeRelativeToTopBinDir(stampFile);
      std::string const stampFileForShell =
        this->LocalGenerator->ConvertToOutputFormat(stampFile,
                                                    cmOutputConverter::SHELL);
      std::string const stampFileForMake =
        cmSystemTools::ConvertToOutputPath(stampFile);

      makeDepends << obj_m << ".provides.build"
                  << ": " << stampFileForMake << '\n';
      // Note that when cmake_copy_f90_mod finds that a module file
      // and the corresponding stamp file have no differences, the stamp
      // file is not updated. In such case the stamp file will be always
      // older than its prerequisite and trigger cmake_copy_f90_mod
      // on each new build. This is expected behavior for incremental
      // builds and can not be changed without performing recursive make
      // calls that would considerably slow down the building process.
      makeDepends << stampFileForMake << ": " << obj_m << '\n';
      makeDepends << "\t$(CMAKE_COMMAND) -E cmake_copy_f90_mod " << modFile
                  << ' ' << stampFileForShell;
      cmMakefile* mf = this->LocalGenerator->GetMakefile();
      cmValue cid = mf->GetDefinition("CMAKE_Fortran_COMPILER_ID");
      if (cmNonempty(cid)) {
        makeDepends << ' ' << *cid;
      }
      makeDepends << '\n';
    }
    makeDepends << obj_m << ".provides.build:\n";
    // After copying the modules update the timestamp file.
    makeDepends << "\t$(CMAKE_COMMAND) -E touch " << obj_m
                << ".provides.build\n";

    // Make sure the module timestamp rule is evaluated by the time
    // the target finishes building.
    std::string driver = cmStrCat(this->TargetDirectory, "/build");
    driver = cmSystemTools::ConvertToOutputPath(
      this->LocalGenerator->MaybeRelativeToTopBinDir(driver));
    makeDepends << driver << ": " << obj_m << ".provides.build\n";
  }

  return true;
}

bool cmDependsFortran::FindModule(std::string const& name, std::string& module)
{
  // Construct possible names for the module file.
  std::string mod_upper;
  std::string mod_lower;
  cmFortranModuleAppendUpperLower(name, mod_upper, mod_lower);

  // Search the include path for the module.
  std::string fullName;
  for (std::string const& ip : this->IncludePath) {
    // Try the lower-case name.
    fullName = cmStrCat(ip, '/', mod_lower);
    if (cmSystemTools::FileExists(fullName, true)) {
      module = fullName;
      return true;
    }

    // Try the upper-case name.
    fullName = cmStrCat(ip, '/', mod_upper);
    if (cmSystemTools::FileExists(fullName, true)) {
      module = fullName;
      return true;
    }
  }
  return false;
}

bool cmDependsFortran::CopyModule(const std::vector<std::string>& args)
{
  // Implements
  //
  //   $(CMAKE_COMMAND) -E cmake_copy_f90_mod input.mod output.mod.stamp
  //                                          [compiler-id]
  //
  // Note that the case of the .mod file depends on the compiler.  In
  // the future this copy could also account for the fact that some
  // compilers include a timestamp in the .mod file so it changes even
  // when the interface described in the module does not.

  std::string mod = args[2];
  std::string const& stamp = args[3];
  std::string compilerId;
  if (args.size() >= 5) {
    compilerId = args[4];
  }
  if (!cmHasLiteralSuffix(mod, ".mod") && !cmHasLiteralSuffix(mod, ".smod") &&
      !cmHasLiteralSuffix(mod, ".sub")) {
    // Support depend.make files left by older versions of CMake.
    // They do not include the ".mod" extension.
    mod += ".mod";
  }
  std::string mod_dir = cmSystemTools::GetFilenamePath(mod);
  if (!mod_dir.empty()) {
    mod_dir += "/";
  }
  std::string mod_upper = mod_dir;
  std::string mod_lower = mod_dir;
  cmFortranModuleAppendUpperLower(cmSystemTools::GetFilenameName(mod),
                                  mod_upper, mod_lower);
  if (cmSystemTools::FileExists(mod_upper, true)) {
    if (cmDependsFortran::ModulesDiffer(mod_upper, stamp, compilerId)) {
      if (!cmSystemTools::CopyFileAlways(mod_upper, stamp)) {
        std::cerr << "Error copying Fortran module from \"" << mod_upper
                  << "\" to \"" << stamp << "\".\n";
        return false;
      }
    }
    return true;
  }
  if (cmSystemTools::FileExists(mod_lower, true)) {
    if (cmDependsFortran::ModulesDiffer(mod_lower, stamp, compilerId)) {
      if (!cmSystemTools::CopyFileAlways(mod_lower, stamp)) {
        std::cerr << "Error copying Fortran module from \"" << mod_lower
                  << "\" to \"" << stamp << "\".\n";
        return false;
      }
    }
    return true;
  }

  std::cerr << "Error copying Fortran module \"" << args[2] << "\".  Tried \""
            << mod_upper << "\" and \"" << mod_lower << "\".\n";
  return false;
}

// Helper function to look for a short sequence in a stream.  If this
// is later used for longer sequences it should be re-written using an
// efficient string search algorithm such as Boyer-Moore.
static bool cmFortranStreamContainsSequence(std::istream& ifs, const char* seq,
                                            int len)
{
  assert(len > 0);

  int cur = 0;
  while (cur < len) {
    // Get the next character.
    int token = ifs.get();
    if (!ifs) {
      return false;
    }

    // Check the character.
    if (token == static_cast<int>(seq[cur])) {
      ++cur;
    } else {
      // Assume the sequence has no repeating subsequence.
      cur = 0;
    }
  }

  // The entire sequence was matched.
  return true;
}

// Helper function to compare the remaining content in two streams.
static bool cmFortranStreamsDiffer(std::istream& ifs1, std::istream& ifs2)
{
  // Compare the remaining content.
  for (;;) {
    int ifs1_c = ifs1.get();
    int ifs2_c = ifs2.get();
    if (!ifs1 && !ifs2) {
      // We have reached the end of both streams simultaneously.
      // The streams are identical.
      return false;
    }

    if (!ifs1 || !ifs2 || ifs1_c != ifs2_c) {
      // We have reached the end of one stream before the other or
      // found differing content.  The streams are different.
      break;
    }
  }

  return true;
}

bool cmDependsFortran::ModulesDiffer(const std::string& modFile,
                                     const std::string& stampFile,
                                     const std::string& compilerId)
{
  /*
  gnu >= 4.9:
    A mod file is an ascii file compressed with gzip.
    Compiling twice produces identical modules.

  gnu < 4.9:
    A mod file is an ascii file.
    <bar.mod>
    FORTRAN module created from /path/to/foo.f90 on Sun Dec 30 22:47:58 2007
    If you edit this, you'll get what you deserve.
    ...
    </bar.mod>
    As you can see the first line contains the date.

  intel:
    A mod file is a binary file.
    However, looking into both generated bar.mod files with a hex editor
    shows that they differ only before a sequence linefeed-zero (0x0A 0x00)
    which is located some bytes in front of the absolute path to the source
    file.

  sun:
    A mod file is a binary file.  Compiling twice produces identical modules.

  others:
    TODO ...
  */

  /* Compilers which do _not_ produce different mod content when the same
   * source is compiled twice
   *   -SunPro
   */
  if (compilerId == "SunPro") {
    return cmSystemTools::FilesDiffer(modFile, stampFile);
  }

#if defined(_WIN32) || defined(__CYGWIN__)
  cmsys::ifstream finModFile(modFile.c_str(), std::ios::in | std::ios::binary);
  cmsys::ifstream finStampFile(stampFile.c_str(),
                               std::ios::in | std::ios::binary);
#else
  cmsys::ifstream finModFile(modFile.c_str());
  cmsys::ifstream finStampFile(stampFile.c_str());
#endif
  if (!finModFile || !finStampFile) {
    // At least one of the files does not exist.  The modules differ.
    return true;
  }

  /* Compilers which _do_ produce different mod content when the same
   * source is compiled twice
   *   -GNU
   *   -Intel
   *
   * Eat the stream content until all recompile only related changes
   * are left behind.
   */
  if (compilerId == "GNU") {
    // GNU Fortran 4.9 and later compress .mod files with gzip
    // but also do not include a date so we can fall through to
    // compare them without skipping any prefix.
    unsigned char hdr[2];
    bool okay = !finModFile.read(reinterpret_cast<char*>(hdr), 2).fail();
    finModFile.seekg(0);
    if (!okay || hdr[0] != 0x1f || hdr[1] != 0x8b) {
      const char seq[1] = { '\n' };
      const int seqlen = 1;

      if (!cmFortranStreamContainsSequence(finModFile, seq, seqlen)) {
        // The module is of unexpected format.  Assume it is different.
        std::cerr << compilerId << " fortran module " << modFile
                  << " has unexpected format." << std::endl;
        return true;
      }

      if (!cmFortranStreamContainsSequence(finStampFile, seq, seqlen)) {
        // The stamp must differ if the sequence is not contained.
        return true;
      }
    }
  } else if (compilerId == "Intel" || compilerId == "IntelLLVM") {
    const char seq[2] = { '\n', '\0' };
    const int seqlen = 2;

    // Skip the leading byte which appears to be a version number.
    // We do not need to check for an error because the sequence search
    // below will fail in that case.
    finModFile.get();
    finStampFile.get();

    if (!cmFortranStreamContainsSequence(finModFile, seq, seqlen)) {
      // The module is of unexpected format.  Assume it is different.
      std::cerr << compilerId << " fortran module " << modFile
                << " has unexpected format." << std::endl;
      return true;
    }

    if (!cmFortranStreamContainsSequence(finStampFile, seq, seqlen)) {
      // The stamp must differ if the sequence is not contained.
      return true;
    }
  }

  // Compare the remaining content.  If no compiler id matched above,
  // including the case none was given, this will compare the whole
  // content.
  return cmFortranStreamsDiffer(finModFile, finStampFile);
}
