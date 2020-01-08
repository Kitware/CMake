/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmGlobalGeneratorFactory_h
#define cmGlobalGeneratorFactory_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include <cm/memory>

class cmGlobalGenerator;
class cmake;
struct cmDocumentationEntry;

/** \class cmGlobalGeneratorFactory
 * \brief Responable for creating cmGlobalGenerator instances
 *
 * Subclasses of this class generate instances of cmGlobalGenerator.
 */
class cmGlobalGeneratorFactory
{
public:
  virtual ~cmGlobalGeneratorFactory() = default;

  /** Create a GlobalGenerator */
  virtual std::unique_ptr<cmGlobalGenerator> CreateGlobalGenerator(
    const std::string& n, cmake* cm) const = 0;

  /** Get the documentation entry for this factory */
  virtual void GetDocumentation(cmDocumentationEntry& entry) const = 0;

  /** Get the names of the current registered generators */
  virtual std::vector<std::string> GetGeneratorNames() const = 0;
  virtual std::vector<std::string> GetGeneratorNamesWithPlatform() const = 0;

  /** Determine whether or not this generator supports toolsets */
  virtual bool SupportsToolset() const = 0;

  /** Determine whether or not this generator supports platforms */
  virtual bool SupportsPlatform() const = 0;

  /** Get the list of supported platforms name for this generator */
  virtual std::vector<std::string> GetKnownPlatforms() const = 0;

  /** If the generator suports platforms, get its default.  */
  virtual std::string GetDefaultPlatformName() const = 0;
};

template <class T>
class cmGlobalGeneratorSimpleFactory : public cmGlobalGeneratorFactory
{
public:
  /** Create a GlobalGenerator */
  std::unique_ptr<cmGlobalGenerator> CreateGlobalGenerator(
    const std::string& name, cmake* cm) const override
  {
    if (name != T::GetActualName()) {
      return std::unique_ptr<cmGlobalGenerator>();
    }
    return std::unique_ptr<cmGlobalGenerator>(cm::make_unique<T>(cm));
  }

  /** Get the documentation entry for this factory */
  void GetDocumentation(cmDocumentationEntry& entry) const override
  {
    T::GetDocumentation(entry);
  }

  /** Get the names of the current registered generators */
  std::vector<std::string> GetGeneratorNames() const override
  {
    std::vector<std::string> names;
    names.push_back(T::GetActualName());
    return names;
  }
  std::vector<std::string> GetGeneratorNamesWithPlatform() const override
  {
    return std::vector<std::string>();
  }

  /** Determine whether or not this generator supports toolsets */
  bool SupportsToolset() const override { return T::SupportsToolset(); }

  /** Determine whether or not this generator supports platforms */
  bool SupportsPlatform() const override { return T::SupportsPlatform(); }

  /** Get the list of supported platforms name for this generator */
  std::vector<std::string> GetKnownPlatforms() const override
  {
    // default is no platform supported
    return std::vector<std::string>();
  }

  std::string GetDefaultPlatformName() const override { return std::string(); }
};

#endif
