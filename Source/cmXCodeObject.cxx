#include "cmXCodeObject.h"
const char* cmXCodeObject::PBXTypeNames[] = {
    "PBXGroup", "PBXBuildStyle", "PBXProject", "PBXHeadersBuildPhase", 
    "PBXSourcesBuildPhase", "PBXFrameworksBuildPhase", "PBXNativeTarget",
    "PBXFileReference", "PBXBuildFile", "PBXContainerItemProxy", "PBXTargetDependency",
    "PBXShellScriptBuildPhase", "PBXResourcesBuildPhase", "PBXApplicationReference",
    "PBXExecutableFileReference", "PBXLibraryReference", "PBXToolTarget", "PBXLibraryTarget",
    "None"
  };

std::vector<cmXCodeObject*> cmXCodeObject::s_AllObjects;

cmXCodeObject::cmXCodeObject(PBXType ptype, Type type)
{
  m_IsA = ptype;
  cmOStringStream str;
  str << (void*)this;
  m_Id = str.str();
  m_Type = type;
  cmXCodeObject::s_AllObjects.push_back(this);
}


void cmXCodeObject::Indent(int level, std::ostream& out)
{
  while(level)
    {
    out << "       ";
    level--;
    }
}

void cmXCodeObject::Print(std::ostream& out)
{
  this->Indent(1, out);
  out << m_Id << " = {\n";
  std::map<cmStdString, cmXCodeObject*>::iterator i;
  for(i = m_ObjectAttributes.begin(); i != m_ObjectAttributes.end(); ++i)
    { 
    cmXCodeObject* object = i->second;
    if(object->m_Type == OBJECT_LIST)
      {
      this->Indent(2, out);
      out << i->first << " = {\n";
      for(unsigned int k = 0; k < i->second->m_List.size(); k++)
        {
        this->Indent(3, out);
        out << i->second->m_List[k]->m_Id << ",\n";
        } 
      this->Indent(2, out);
      out << "};\n";
      }
    else if(object->m_Type == ATTRIBUTE_GROUP)
      {
      std::map<cmStdString, cmStdString>::iterator j;
      this->Indent(2, out);
      out << i->first << " = {\n";
      for(j = object->m_StringAttributes.begin(); j != object->m_StringAttributes.end(); ++j)
        {
        this->Indent(3, out);
        out << j->first << " = " << j->second << ";\n";
        }
      this->Indent(2, out);
      out << " }\n";
      }
    else if(object->m_Type == OBJECT_REF)
      {
      this->Indent(2, out);
      out << i->first << " = " << object->m_Object->m_Id << ";\n";
      }
        
    }
      
  this->Indent(2, out);
  out << "isa = " << PBXTypeNames[m_IsA] << ";\n";
  std::map<cmStdString, cmStdString>::iterator j;
  for(j = m_StringAttributes.begin(); j != m_StringAttributes.end(); ++j)
    {
    this->Indent(2, out);
    out << j->first << " = " << j->second << ";\n";
    }
  this->Indent(1, out);
  out << "};\n";
}
  
void cmXCodeObject::PrintAll(std::ostream& out)
{
  out << "objects = {\n";
  for(unsigned int i = 0; i < s_AllObjects.size(); ++i)
    {
    if(s_AllObjects[i]->m_Type == OBJECT)
      {
      s_AllObjects[i]->Print(out);
      }
    }
  out << "};\n";
}
