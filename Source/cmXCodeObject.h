#ifndef cmXCodeObject_h
#define cmXCodeObject_h

#include "cmStandardIncludes.h"
class cmTarget;

class cmXCodeObject
{
public:
  enum Type { OBJECT_LIST, STRING, ATTRIBUTE_GROUP, OBJECT_REF, OBJECT };
  enum PBXType { PBXGroup, PBXBuildStyle, PBXProject, PBXHeadersBuildPhase, 
                 PBXSourcesBuildPhase, PBXFrameworksBuildPhase, 
                 PBXNativeTarget, PBXFileReference, PBXBuildFile, 
                 PBXContainerItemProxy, PBXTargetDependency,
                 PBXShellScriptBuildPhase, PBXResourcesBuildPhase,
                 PBXApplicationReference, PBXExecutableFileReference, 
                 PBXLibraryReference, PBXToolTarget, PBXLibraryTarget, 
                 PBXAggregateTarget,
                 None
  };
  static const char* PBXTypeNames[];
  ~cmXCodeObject();
  cmXCodeObject(PBXType ptype, Type type);
  void SetString(const char* s)
    {
      m_String = "\"";
      m_String += s;
      m_String += "\"";
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
  static void Indent(int level, std::ostream& out);
  void Print(std::ostream& out);
  static void PrintList(std::vector<cmXCodeObject*> const&, std::ostream& out);
  const char* GetId()
    {
      return m_Id.c_str();
    }
  cmTarget* GetcmTarget()
    {
      return m_cmTarget;
    }
  void SetcmTarget(cmTarget* t)
    {
      m_cmTarget = t;
    }
  cmXCodeObject* GetObject(const char* name)
    {
      if(m_ObjectAttributes.count(name))
        {
        return m_ObjectAttributes[name];
        }
      return 0;
    }
  
private:
  cmTarget* m_cmTarget;
  Type m_Type;
  cmStdString m_Id;
  PBXType m_IsA;
  cmStdString m_String;
  cmXCodeObject* m_Object;
  std::vector<cmXCodeObject*> m_List;
  std::map<cmStdString, cmXCodeObject*> m_ObjectAttributes;
};
#endif
