#include "cmStandardIncludes.h"

class cmXCodeObject
{
public:
  enum Type { OBJECT_LIST, ATTRIBUTE_GROUP, OBJECT_REF, OBJECT };
  enum PBXType { PBXGroup, PBXBuildStyle, PBXProject, PBXHeadersBuildPhase, 
                 PBXSourcesBuildPhase, PBXFrameworksBuildPhase, PBXNativeTarget,
                 PBXFileReference, PBXBuildFile, PBXContainerItemProxy, PBXTargetDependency,
                 PBXShellScriptBuildPhase, PBXResourcesBuildPhase, PBXApplicationReference,
                 PBXExecutableFileReference, PBXLibraryReference, PBXToolTarget, PBXLibraryTarget,
                 None
  };
  static const char* PBXTypeNames[];
  
  cmXCodeObject(PBXType ptype, Type type);
  void AddAttribute(const char* name, const char* value)
    {
      m_StringAttributes[name] = value;
    }
  void AddAttribute(const char* name, cmXCodeObject* value)
    {
      m_ObjectAttributes[name] = value;
    }
  
  void SetObject(cmXCodeObject* value)
    {
      m_Object = value;
    }
  void AddObject(cmXCodeObject* value)
    {
      m_List.push_back(value);
    }
  void Indent(int level, std::ostream& out);
  void Print(std::ostream& out);
  static void PrintAll(std::ostream& out);
  const char* GetId()
    {
      return m_Id.c_str();
    }
  Type m_Type;
  cmStdString m_Id;
  PBXType m_IsA;
  cmXCodeObject* m_Object;
  std::vector<cmXCodeObject*> m_List;
  std::map<cmStdString, cmXCodeObject*> m_ObjectAttributes;
  std::map<cmStdString, cmStdString> m_StringAttributes;
  static std::vector<cmXCodeObject*> s_AllObjects;
};
