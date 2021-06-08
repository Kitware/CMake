/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

#include <cm/memory>
#include <cm/string_view>
#include <cmext/string_view>

class cmGeneratorTarget;
class cmInstallImportedRuntimeArtifactsGenerator;
class cmInstallTargetGenerator;

class cmInstallRuntimeDependencySet
{
public:
  cmInstallRuntimeDependencySet(std::string name = "");

  cmInstallRuntimeDependencySet(const cmInstallRuntimeDependencySet&) = delete;
  cmInstallRuntimeDependencySet& operator=(
    const cmInstallRuntimeDependencySet&) = delete;

  cm::string_view GetName() const { return this->Name; }

  cm::string_view GetDisplayName() const
  {
    if (this->Name.empty()) {
      return "<anonymous>"_s;
    }
    return this->Name;
  }

  class Item
  {
  public:
    virtual ~Item() = default;

    virtual std::string GetItemPath(const std::string& config) const = 0;

    virtual void AddPostExcludeFiles(
      const std::string& /*config*/, std::set<std::string>& /*files*/,
      cmInstallRuntimeDependencySet* /*set*/) const
    {
    }
  };

  class TargetItem : public Item
  {
  public:
    TargetItem(cmInstallTargetGenerator* target)
      : Target(target)
    {
    }

    std::string GetItemPath(const std::string& config) const override;

    void AddPostExcludeFiles(
      const std::string& config, std::set<std::string>& files,
      cmInstallRuntimeDependencySet* set) const override;

  private:
    cmInstallTargetGenerator* Target;
  };

  class ImportedTargetItem : public Item
  {
  public:
    ImportedTargetItem(cmInstallImportedRuntimeArtifactsGenerator* target)
      : Target(target)
    {
    }

    std::string GetItemPath(const std::string& config) const override;

  private:
    cmInstallImportedRuntimeArtifactsGenerator* Target;
  };

  void AddExecutable(std::unique_ptr<Item> executable);
  void AddLibrary(std::unique_ptr<Item> library);
  void AddModule(std::unique_ptr<Item> module);
  bool AddBundleExecutable(std::unique_ptr<Item> bundleExecutable);

  void AddExecutable(cmInstallTargetGenerator* executable)
  {
    this->AddExecutable(cm::make_unique<TargetItem>(executable));
  }

  void AddLibrary(cmInstallTargetGenerator* library)
  {
    this->AddLibrary(cm::make_unique<TargetItem>(library));
  }

  void AddModule(cmInstallTargetGenerator* module)
  {
    this->AddModule(cm::make_unique<TargetItem>(module));
  }

  bool AddBundleExecutable(cmInstallTargetGenerator* bundleExecutable)
  {
    return this->AddBundleExecutable(
      cm::make_unique<TargetItem>(bundleExecutable));
  }

  void AddExecutable(cmInstallImportedRuntimeArtifactsGenerator* executable)
  {
    this->AddExecutable(cm::make_unique<ImportedTargetItem>(executable));
  }

  void AddLibrary(cmInstallImportedRuntimeArtifactsGenerator* library)
  {
    this->AddLibrary(cm::make_unique<ImportedTargetItem>(library));
  }

  void AddModule(cmInstallImportedRuntimeArtifactsGenerator* module)
  {
    this->AddModule(cm::make_unique<ImportedTargetItem>(module));
  }

  bool AddBundleExecutable(
    cmInstallImportedRuntimeArtifactsGenerator* bundleExecutable)
  {
    return this->AddBundleExecutable(
      cm::make_unique<ImportedTargetItem>(bundleExecutable));
  }

  const std::vector<std::unique_ptr<Item>>& GetExecutables() const
  {
    return this->Executables;
  }

  const std::vector<std::unique_ptr<Item>>& GetLibraries() const
  {
    return this->Libraries;
  }

  const std::vector<std::unique_ptr<Item>>& GetModules() const
  {
    return this->Modules;
  }

  Item* GetBundleExecutable() const { return this->BundleExecutable; }

  bool Empty() const
  {
    return this->Executables.empty() && this->Libraries.empty() &&
      this->Modules.empty();
  }

private:
  std::string Name;
  std::vector<std::unique_ptr<Item>> Executables;
  std::vector<std::unique_ptr<Item>> Libraries;
  std::vector<std::unique_ptr<Item>> Modules;
  Item* BundleExecutable = nullptr;

  std::map<const cmGeneratorTarget*, std::set<const cmGeneratorTarget*>>
    TargetDepends;
};
