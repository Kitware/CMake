/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmBuildDatabase.h"

#include <cstdlib>
#include <set>
#include <utility>

#include <cm/memory>
#include <cm/string_view>
#include <cmext/string_view>

#include <cm3p/json/reader.h>
#include <cm3p/json/value.h>
#include <cm3p/json/writer.h>

#include "cmsys/FStream.hxx"

#include "cmComputeLinkInformation.h"
#include "cmFileSet.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmSourceFile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

namespace {

std::string PlaceholderName = "<__CMAKE_UNKNOWN>";

}

cmBuildDatabase::cmBuildDatabase() = default;
cmBuildDatabase::cmBuildDatabase(cmBuildDatabase const&) = default;
cmBuildDatabase::~cmBuildDatabase() = default;

cmBuildDatabase::LookupTable cmBuildDatabase::GenerateLookupTable()
{
  LookupTable lut;

  for (auto& Set_ : this->Sets) {
    for (auto& TranslationUnit_ : Set_.TranslationUnits) {
      // This table is from source path to TU instance. This is fine because a
      // single target (where this is used) cannot contain the same source file
      // multiple times.
      lut[TranslationUnit_.Source] = &TranslationUnit_;
    }
  }

  return lut;
}

bool cmBuildDatabase::HasPlaceholderNames() const
{
  for (auto const& Set_ : this->Sets) {
    for (auto const& TranslationUnit_ : Set_.TranslationUnits) {
      for (auto const& provide : TranslationUnit_.Provides) {
        if (provide.first == PlaceholderName) {
          return true;
        }
        if (provide.second == PlaceholderName) {
          return true;
        }
      }
    }
  }

  return false;
}

void cmBuildDatabase::Write(std::string const& path) const
{
  Json::Value mcdb = Json::objectValue;

  mcdb["version"] = 1;
  mcdb["revision"] = 0;

  Json::Value& sets = mcdb["sets"] = Json::arrayValue;

  for (auto const& Set_ : this->Sets) {
    Json::Value set = Json::objectValue;

    set["name"] = Set_.Name;
    set["family-name"] = Set_.FamilyName;

    Json::Value& visible_sets = set["visible-sets"] = Json::arrayValue;
    for (auto const& VisibleSet : Set_.VisibleSets) {
      visible_sets.append(VisibleSet);
    }

    Json::Value& tus = set["translation-units"] = Json::arrayValue;
    for (auto const& TranslationUnit_ : Set_.TranslationUnits) {
      Json::Value tu = Json::objectValue;

      if (!TranslationUnit_.WorkDirectory.empty()) {
        tu["work-directory"] = TranslationUnit_.WorkDirectory;
      }
      tu["source"] = TranslationUnit_.Source;
      if (TranslationUnit_.Object) {
        tu["object"] = *TranslationUnit_.Object;
      }
      tu["private"] = TranslationUnit_.Private;

      Json::Value& reqs = tu["requires"] = Json::arrayValue;
      for (auto const& Require : TranslationUnit_.Requires) {
        reqs.append(Require);
      }

      Json::Value& provides = tu["provides"] = Json::objectValue;
      for (auto const& Provide : TranslationUnit_.Provides) {
        provides[Provide.first] = Provide.second;
      }

      Json::Value& baseline_arguments = tu["baseline-arguments"] =
        Json::arrayValue;
      for (auto const& BaselineArgument : TranslationUnit_.BaselineArguments) {
        baseline_arguments.append(BaselineArgument);
      }

      Json::Value& local_arguments = tu["local-arguments"] = Json::arrayValue;
      for (auto const& LocalArgument : TranslationUnit_.LocalArguments) {
        local_arguments.append(LocalArgument);
      }

      Json::Value& arguments = tu["arguments"] = Json::arrayValue;
      for (auto const& Argument : TranslationUnit_.Arguments) {
        arguments.append(Argument);
      }

      tus.append(tu);
    }

    sets.append(set);
  }

  cmGeneratedFileStream mcdbf(path);
  mcdbf << mcdb;
}

static bool ParseFilename(Json::Value const& val, std::string& result)
{
  if (val.isString()) {
    result = val.asString();
  } else {
    return false;
  }

  return true;
}

#define PARSE_BLOB(val, res)                                                  \
  do {                                                                        \
    if (!ParseFilename(val, res)) {                                           \
      cmSystemTools::Error(cmStrCat("-E cmake_module_compile_db failed to ",  \
                                    "parse ", path, ": invalid blob"));       \
      return {};                                                              \
    }                                                                         \
  } while (0)

#define PARSE_FILENAME(val, res, make_full)                                   \
  do {                                                                        \
    if (!ParseFilename(val, res)) {                                           \
      cmSystemTools::Error(cmStrCat("-E cmake_module_compile_db failed to ",  \
                                    "parse ", path, ": invalid filename"));   \
      return {};                                                              \
    }                                                                         \
                                                                              \
    if (make_full && work_directory && !work_directory->empty() &&            \
        !cmSystemTools::FileIsFullPath(res)) {                                \
      res = cmStrCat(*work_directory, '/', res);                              \
    }                                                                         \
  } while (0)

std::unique_ptr<cmBuildDatabase> cmBuildDatabase::Load(std::string const& path)
{
  Json::Value mcdb;
  {
    cmsys::ifstream mcdbf(path.c_str(), std::ios::in | std::ios::binary);
    Json::Reader reader;
    if (!reader.parse(mcdbf, mcdb, false)) {
      cmSystemTools::Error(
        cmStrCat("-E cmake_module_compile_db failed to parse ", path,
                 reader.getFormattedErrorMessages()));
      return {};
    }
  }

  Json::Value const& version = mcdb["version"];
  if (version.asUInt() > 1) {
    cmSystemTools::Error(
      cmStrCat("-E cmake_module_compile_db failed to parse ", path,
               ": version ", version.asString()));
    return {};
  }

  auto db = cm::make_unique<cmBuildDatabase>();

  Json::Value const& sets = mcdb["sets"];
  if (sets.isArray()) {
    for (auto const& set : sets) {
      Set Set_;

      Json::Value const& name = set["name"];
      if (!name.isString()) {
        cmSystemTools::Error(
          cmStrCat("-E cmake_module_compile_db failed to parse ", path,
                   ": name is not a string"));
        return {};
      }
      Set_.Name = name.asString();

      Json::Value const& family_name = set["family-name"];
      if (!family_name.isString()) {
        cmSystemTools::Error(
          cmStrCat("-E cmake_module_compile_db failed to parse ", path,
                   ": family-name is not a string"));
        return {};
      }
      Set_.FamilyName = family_name.asString();

      Json::Value const& visible_sets = set["visible-sets"];
      if (!visible_sets.isArray()) {
        cmSystemTools::Error(
          cmStrCat("-E cmake_module_compile_db failed to parse ", path,
                   ": visible-sets is not an array"));
        return {};
      }
      for (auto const& visible_set : visible_sets) {
        if (!visible_set.isString()) {
          cmSystemTools::Error(
            cmStrCat("-E cmake_module_compile_db failed to parse ", path,
                     ": a visible-sets item is not a string"));
          return {};
        }

        Set_.VisibleSets.emplace_back(visible_set.asString());
      }

      Json::Value const& translation_units = set["translation-units"];
      if (!translation_units.isArray()) {
        cmSystemTools::Error(
          cmStrCat("-E cmake_module_compile_db failed to parse ", path,
                   ": translation-units is not an array"));
        return {};
      }
      for (auto const& translation_unit : translation_units) {
        if (!translation_unit.isObject()) {
          cmSystemTools::Error(
            cmStrCat("-E cmake_module_compile_db failed to parse ", path,
                     ": a translation-units item is not an object"));
          return {};
        }

        TranslationUnit TranslationUnit_;

        cm::optional<std::string> work_directory;
        Json::Value const& workdir = translation_unit["work-directory"];
        if (workdir.isString()) {
          PARSE_BLOB(workdir, TranslationUnit_.WorkDirectory);
          work_directory = TranslationUnit_.WorkDirectory;
        } else if (!workdir.isNull()) {
          cmSystemTools::Error(
            cmStrCat("-E cmake_module_compile_db failed to parse ", path,
                     ": work-directory is not a string"));
          return {};
        }

        Json::Value const& source = translation_unit["source"];
        PARSE_FILENAME(source, TranslationUnit_.Source, true);

        if (translation_unit.isMember("object")) {
          Json::Value const& object = translation_unit["object"];
          if (!object.isNull()) {
            TranslationUnit_.Object = "";
            PARSE_FILENAME(object, *TranslationUnit_.Object, false);
          }
        }

        if (translation_unit.isMember("private")) {
          Json::Value const& priv = translation_unit["private"];
          if (!priv.isBool()) {
            cmSystemTools::Error(
              cmStrCat("-E cmake_module_compile_db failed to parse ", path,
                       ": private is not a boolean"));
            return {};
          }
          TranslationUnit_.Private = priv.asBool();
        }

        if (translation_unit.isMember("requires")) {
          Json::Value const& reqs = translation_unit["requires"];
          if (!reqs.isArray()) {
            cmSystemTools::Error(
              cmStrCat("-E cmake_module_compile_db failed to parse ", path,
                       ": requires is not an array"));
            return {};
          }

          for (auto const& require : reqs) {
            if (!require.isString()) {
              cmSystemTools::Error(
                cmStrCat("-E cmake_module_compile_db failed to parse ", path,
                         ": a requires item is not a string"));
              return {};
            }

            TranslationUnit_.Requires.emplace_back(require.asString());
          }
        }

        if (translation_unit.isMember("provides")) {
          Json::Value const& provides = translation_unit["provides"];
          if (!provides.isObject()) {
            cmSystemTools::Error(
              cmStrCat("-E cmake_module_compile_db failed to parse ", path,
                       ": provides is not an object"));
            return {};
          }

          for (auto i = provides.begin(); i != provides.end(); ++i) {
            if (!i->isString()) {
              cmSystemTools::Error(
                cmStrCat("-E cmake_module_compile_db failed to parse ", path,
                         ": a provides value is not a string"));
              return {};
            }

            TranslationUnit_.Provides[i.key().asString()] = i->asString();
          }
        }

        if (translation_unit.isMember("baseline-arguments")) {
          Json::Value const& baseline_arguments =
            translation_unit["baseline-arguments"];
          if (!baseline_arguments.isArray()) {
            cmSystemTools::Error(
              cmStrCat("-E cmake_module_compile_db failed to parse ", path,
                       ": baseline_arguments is not an array"));
            return {};
          }

          for (auto const& baseline_argument : baseline_arguments) {
            if (baseline_argument.isString()) {
              TranslationUnit_.BaselineArguments.emplace_back(
                baseline_argument.asString());
            } else {
              cmSystemTools::Error(
                cmStrCat("-E cmake_module_compile_db failed to parse ", path,
                         ": a baseline argument is not a string"));
              return {};
            }
          }
        }

        if (translation_unit.isMember("local-arguments")) {
          Json::Value const& local_arguments =
            translation_unit["local-arguments"];
          if (!local_arguments.isArray()) {
            cmSystemTools::Error(
              cmStrCat("-E cmake_module_compile_db failed to parse ", path,
                       ": local_arguments is not an array"));
            return {};
          }

          for (auto const& local_argument : local_arguments) {
            if (local_argument.isString()) {
              TranslationUnit_.LocalArguments.emplace_back(
                local_argument.asString());
            } else {
              cmSystemTools::Error(
                cmStrCat("-E cmake_module_compile_db failed to parse ", path,
                         ": a local argument is not a string"));
              return {};
            }
          }
        }

        if (translation_unit.isMember("arguments")) {
          Json::Value const& arguments = translation_unit["arguments"];
          if (!arguments.isArray()) {
            cmSystemTools::Error(
              cmStrCat("-E cmake_module_compile_db failed to parse ", path,
                       ": arguments is not an array"));
            return {};
          }

          for (auto const& argument : arguments) {
            if (argument.isString()) {
              TranslationUnit_.Arguments.emplace_back(argument.asString());
            } else {
              cmSystemTools::Error(
                cmStrCat("-E cmake_module_compile_db failed to parse ", path,
                         ": an argument is not a string"));
              return {};
            }
          }
        }

        Set_.TranslationUnits.emplace_back(std::move(TranslationUnit_));
      }

      db->Sets.emplace_back(std::move(Set_));
    }
  }

  return db;
}

cmBuildDatabase cmBuildDatabase::Merge(
  std::vector<cmBuildDatabase> const& components)
{
  cmBuildDatabase db;

  for (auto const& component : components) {
    db.Sets.insert(db.Sets.end(), component.Sets.begin(),
                   component.Sets.end());
  }

  return db;
}

cmBuildDatabase cmBuildDatabase::ForTarget(cmGeneratorTarget* gt,
                                           std::string const& config)
{
  cmBuildDatabase db;

  Set set;
  set.Name = cmStrCat(gt->GetName(), '@', config);
  set.FamilyName = gt->GetFamilyName();
  if (auto* cli = gt->GetLinkInformation(config)) {
    std::set<cmGeneratorTarget const*> emitted;
    std::vector<cmGeneratorTarget const*> targets;
    for (auto const& item : cli->GetItems()) {
      auto const* linkee = item.Target;
      if (linkee && linkee->HaveCxx20ModuleSources() &&
          !linkee->IsImported() && emitted.insert(linkee).second) {
        set.VisibleSets.push_back(cmStrCat(linkee->GetName(), '@', config));
      }
    }
  }

  for (auto const& sfbt : gt->GetSourceFiles(config)) {
    auto const* sf = sfbt.Value;

    bool isCXXModule = false;
    bool isPrivate = true;
    if (sf->GetLanguage() != "CXX"_s) {
      auto const* fs = gt->GetFileSetForSource(config, sf);
      if (fs && fs->GetType() == "CXX_MODULES"_s) {
        isCXXModule = true;
        isPrivate = !cmFileSetVisibilityIsForInterface(fs->GetVisibility());
      }
    }

    TranslationUnit tu;

    // FIXME: Makefiles will want this to be the current working directory.
    tu.WorkDirectory = gt->GetLocalGenerator()->GetBinaryDirectory();
    tu.Source = sf->GetFullPath();
    if (!gt->IsSynthetic()) {
      auto* gg = gt->GetGlobalGenerator();
      std::string const objectDir = gg->ConvertToOutputPath(
        cmStrCat(gt->GetSupportDirectory(), gg->GetConfigDirectory(config)));
      std::string const objectFileName = gt->GetObjectName(sf);
      tu.Object = cmStrCat(objectDir, '/', objectFileName);
    }
    if (isCXXModule) {
      tu.Provides[PlaceholderName] = PlaceholderName;
    }

    cmGeneratorTarget::ClassifiedFlags classifiedFlags =
      gt->GetClassifiedFlagsForSource(sf, config);
    for (auto const& classifiedFlag : classifiedFlags) {
      if (classifiedFlag.Classification ==
          cmGeneratorTarget::FlagClassification::BaselineFlag) {
        tu.BaselineArguments.push_back(classifiedFlag.Flag);
        tu.LocalArguments.push_back(classifiedFlag.Flag);
      } else if (classifiedFlag.Classification ==
                 cmGeneratorTarget::FlagClassification::PrivateFlag) {
        tu.LocalArguments.push_back(classifiedFlag.Flag);
      }
      tu.Arguments.push_back(classifiedFlag.Flag);
    }
    tu.Private = isPrivate;

    set.TranslationUnits.emplace_back(std::move(tu));
  }

  db.Sets.emplace_back(std::move(set));

  return db;
}

int cmcmd_cmake_module_compile_db(
  std::vector<std::string>::const_iterator argBeg,
  std::vector<std::string>::const_iterator argEnd)
{
  const std::string* command = nullptr;
  const std::string* output = nullptr;
  std::vector<const std::string*> inputs;

  bool next_is_output = false;
  for (auto i = argBeg; i != argEnd; ++i) {
    // The first argument is always the command.
    if (!command) {
      command = &(*i);
      continue;
    }

    if (*i == "-o"_s) {
      next_is_output = true;
      continue;
    }
    if (next_is_output) {
      if (output) {
        cmSystemTools::Error(
          "-E cmake_module_compile_db only supports one output file");
        return EXIT_FAILURE;
      }

      output = &(*i);
      next_is_output = false;
      continue;
    }

    inputs.emplace_back(&(*i));
  }

  if (!command) {
    cmSystemTools::Error("-E cmake_module_compile_db requires a subcommand");
    return EXIT_FAILURE;
  }

  int ret = EXIT_SUCCESS;

  if (*command == "verify"_s) {
    if (output) {
      cmSystemTools::Error(
        "-E cmake_module_compile_db verify does not support an output");
      return EXIT_FAILURE;
    }

    for (auto const* i : inputs) {
      auto db = cmBuildDatabase::Load(*i);
      if (!db) {
        cmSystemTools::Error(cmStrCat("failed to verify ", *i));
        ret = EXIT_FAILURE;
      }
    }
  } else if (*command == "merge"_s) {
    if (!output) {
      cmSystemTools::Error(
        "-E cmake_module_compile_db verify requires an output");
      return EXIT_FAILURE;
    }

    std::vector<cmBuildDatabase> dbs;

    for (auto const* i : inputs) {
      auto db = cmBuildDatabase::Load(*i);
      if (!db) {
        cmSystemTools::Error(cmStrCat("failed to read ", *i));
        return EXIT_FAILURE;
      }

      dbs.emplace_back(std::move(*db));
    }

    auto db = cmBuildDatabase::Merge(dbs);
    db.Write(*output);
  } else {
    cmSystemTools::Error(
      cmStrCat("-E cmake_module_compile_db unknown subcommand ", *command));
    return EXIT_FAILURE;
  }

  return ret;
}
