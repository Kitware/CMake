/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <algorithm>
#include <cstddef>
#include <iosfwd>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <cmext/algorithm>

class cmGeneratorTarget;

class cmXCodeObject
{
public:
  enum Type
  {
    OBJECT_LIST,
    STRING,
    ATTRIBUTE_GROUP,
    OBJECT_REF,
    OBJECT
  };
  enum PBXType
  {
    PBXGroup,
    PBXBuildStyle,
    PBXProject,
    PBXHeadersBuildPhase,
    PBXSourcesBuildPhase,
    PBXFrameworksBuildPhase,
    PBXNativeTarget,
    PBXFileReference,
    PBXBuildFile,
    PBXContainerItemProxy,
    PBXTargetDependency,
    PBXShellScriptBuildPhase,
    PBXResourcesBuildPhase,
    PBXApplicationReference,
    PBXExecutableFileReference,
    PBXLibraryReference,
    PBXToolTarget,
    PBXLibraryTarget,
    PBXAggregateTarget,
    XCBuildConfiguration,
    XCConfigurationList,
    PBXCopyFilesBuildPhase,
    None
  };
  class StringVec : public std::vector<std::string>
  {
  };
  static char const* PBXTypeNames[];
  virtual ~cmXCodeObject();
  cmXCodeObject(PBXType ptype, Type type, std::string id);
  Type GetType() const { return this->TypeValue; }
  PBXType GetIsA() const { return this->IsA; }

  bool IsEmpty() const;

  void SetString(std::string const& s);
  std::string const& GetString() const { return this->String; }

  void AddAttribute(std::string const& name, cmXCodeObject* value)
  {
    this->ObjectAttributes[name] = value;
  }

  void AddAttributeIfNotEmpty(std::string const& name, cmXCodeObject* value)
  {
    if (value && !value->IsEmpty()) {
      AddAttribute(name, value);
    }
  }

  void SetObject(cmXCodeObject* value) { this->Object = value; }
  cmXCodeObject* GetObject() { return this->Object; }
  void AddObject(cmXCodeObject* value) { this->List.push_back(value); }
  size_t GetObjectCount() { return this->List.size(); }
  void InsertObject(size_t position, cmXCodeObject* value)
  {
    if (position < GetObjectCount()) {
      this->List.insert(this->List.begin() + position, value);
    }
  }
  void PrependObject(cmXCodeObject* value)
  {
    this->List.insert(this->List.begin(), value);
  }
  bool HasObject(cmXCodeObject* o) const
  {
    return cm::contains(this->List, o);
  }
  void AddUniqueObject(cmXCodeObject* value)
  {
    if (!cm::contains(this->List, value)) {
      this->List.push_back(value);
    }
  }
  static void Indent(int level, std::ostream& out);
  void Print(std::ostream& out);
  void PrintAttribute(std::ostream& out, int level,
                      std::string const& separator, int factor,
                      std::string const& name, cmXCodeObject const* object,
                      cmXCodeObject const* parent);
  virtual void PrintComment(std::ostream&) {}

  static void PrintList(std::vector<cmXCodeObject*> const&, std::ostream& out);
  std::string const& GetId() const { return this->Id; }
  void SetId(std::string const& id) { this->Id = id; }
  cmGeneratorTarget* GetTarget() const { return this->Target; }
  void SetTarget(cmGeneratorTarget* t) { this->Target = t; }
  std::string const& GetComment() const { return this->Comment; }
  bool HasComment() const { return (!this->Comment.empty()); }
  cmXCodeObject* GetAttribute(char const* name) const
  {
    auto const i = this->ObjectAttributes.find(name);
    if (i != this->ObjectAttributes.end()) {
      return i->second;
    }
    return nullptr;
  }
  // search the attribute list for an object of the specified type
  cmXCodeObject* GetObject(cmXCodeObject::PBXType t) const
  {
    for (auto* o : this->List) {
      if (o->IsA == t) {
        return o;
      }
    }
    return nullptr;
  }

  void CopyAttributes(cmXCodeObject*);

  void AddDependLibrary(std::string const& configName, std::string const& l)
  {
    this->DependLibraries[configName].push_back(l);
  }
  std::map<std::string, StringVec> const& GetDependLibraries() const
  {
    return this->DependLibraries;
  }
  void AddDependTarget(std::string const& configName, std::string const& tName)
  {
    this->DependTargets[configName].push_back(tName);
  }
  std::map<std::string, StringVec> const& GetDependTargets() const
  {
    return this->DependTargets;
  }
  std::vector<cmXCodeObject*> const& GetObjectList() const
  {
    return this->List;
  }
  void SetComment(std::string const& c) { this->Comment = c; }
  static void PrintString(std::ostream& os, std::string const& String);

protected:
  void PrintString(std::ostream& os) const;

  cmGeneratorTarget* Target;
  Type TypeValue;
  std::string Id;
  PBXType IsA;
  int Version;
  std::string Comment;
  std::string String;
  cmXCodeObject* Object;
  std::vector<cmXCodeObject*> List;
  std::map<std::string, StringVec> DependLibraries;
  std::map<std::string, StringVec> DependTargets;
  std::map<std::string, cmXCodeObject*> ObjectAttributes;
};
