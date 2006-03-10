#include "cmXCodeObject.h"
#include "cmSystemTools.h"

const char* cmXCodeObject::PBXTypeNames[] = {
    "PBXGroup", "PBXBuildStyle", "PBXProject", "PBXHeadersBuildPhase", 
    "PBXSourcesBuildPhase", "PBXFrameworksBuildPhase", "PBXNativeTarget",
    "PBXFileReference", "PBXBuildFile", "PBXContainerItemProxy", 
    "PBXTargetDependency", "PBXShellScriptBuildPhase", 
    "PBXResourcesBuildPhase", "PBXApplicationReference",
    "PBXExecutableFileReference", "PBXLibraryReference", "PBXToolTarget",
    "PBXLibraryTarget", "PBXAggregateTarget", "XCBuildConfiguration", 
    "XCConfigurationList",
    "None"
  };

cmXCodeObject::~cmXCodeObject()
{
  m_Version = 15;
}


//----------------------------------------------------------------------------
cmXCodeObject::cmXCodeObject(PBXType ptype, Type type)
{
  m_Version = 15;
  m_PBXTargetDependency = 0;
  m_cmTarget = 0;
  m_Object =0;
  
  m_IsA = ptype;
  if(type == OBJECT)
    {
    cmOStringStream str;
    str << (void*)this;
    str << (void*)this;
    str << (void*)this;
    m_Id = str.str();
    }
  else
    {
    m_Id = "Temporary cmake object, should not be refered to in xcode file";
    }
  cmSystemTools::ReplaceString(m_Id, "0x", "");
  m_Id = cmSystemTools::UpperCase(m_Id);
  if(m_Id.size() < 24)
    {
    int diff = 24 - m_Id.size();
    for(int i =0; i < diff; ++i)
      {
      m_Id += "0";
      }
    }
  m_Type = type;
  if(m_Type == OBJECT)
    {
    this->AddAttribute("isa", 0);
    }
}

//----------------------------------------------------------------------------
void cmXCodeObject::Indent(int level, std::ostream& out)
{
  while(level)
    {
    out << "       ";
    level--;
    }
}

//----------------------------------------------------------------------------
void cmXCodeObject::Print(std::ostream& out)
{
  std::string separator = "\n";
  int indentFactor = 1;
  if(m_Version > 15 && (m_IsA == PBXFileReference || m_IsA == PBXBuildFile))
    {
    separator = " ";
    indentFactor = 0;
    }
  cmXCodeObject::Indent(2*indentFactor, out);
  out << m_Id << " ";
  if(!(this->m_IsA == PBXGroup && this->m_Comment.size() == 0))
    {
    this->PrintComment(out);
    }
  out << " = {";
  if(separator == "\n")
    {
    out << separator;
    }
  std::map<cmStdString, cmXCodeObject*>::iterator i;
  cmXCodeObject::Indent(3*indentFactor, out);
  out << "isa = " << PBXTypeNames[m_IsA]  << ";" << separator;
  for(i = m_ObjectAttributes.begin(); i != m_ObjectAttributes.end(); ++i)
    { 
    cmXCodeObject* object = i->second;
    if(i->first != "isa")
      { 
      cmXCodeObject::Indent(3*indentFactor, out);
      }
    else
      {
      continue;
      }
    if(object->m_Type == OBJECT_LIST)
      {
      out << i->first << " = (" << separator;
      for(unsigned int k = 0; k < i->second->m_List.size(); k++)
        {
        cmXCodeObject::Indent(4*indentFactor, out);
        out << i->second->m_List[k]->m_Id << " ";
        i->second->m_List[k]->PrintComment(out);
        out << "," << separator;
        } 
      cmXCodeObject::Indent(3*indentFactor, out);
      out << ");" << separator;
      }
    else if(object->m_Type == ATTRIBUTE_GROUP)
      {
      std::map<cmStdString, cmXCodeObject*>::iterator j;
      out << i->first << " = {" << separator;
      for(j = object->m_ObjectAttributes.begin(); j != 
            object->m_ObjectAttributes.end(); ++j)
        {
        cmXCodeObject::Indent(4 *indentFactor, out);
        out << j->first << " = " << j->second->m_String << ";";
        out << separator;
        }
      cmXCodeObject::Indent(3 *indentFactor, out);
      out << "};" << separator;
      }
    else if(object->m_Type == OBJECT_REF)
      {
      out << i->first << " = " << object->m_Object->m_Id;
      if(object->m_Object->HasComment() && i->first != "remoteGlobalIDString")
        {
        out << " ";
        object->m_Object->PrintComment(out);
        }
      out << ";" << separator;
      }
    else if(object->m_Type == STRING)
      {
      out << i->first << " = " << object->m_String << ";" << separator;
      }
    else
      {
      out << "what is this?? " << i->first << "\n";
      }
    }
  cmXCodeObject::Indent(2*indentFactor, out);
  out << "};\n";
}
  
//----------------------------------------------------------------------------
void cmXCodeObject::PrintList(std::vector<cmXCodeObject*> const& objs,
                              std::ostream& out)
{ 
  cmXCodeObject::Indent(1, out);
  out << "objects = {\n";
  for(unsigned int i = 0; i < objs.size(); ++i)
    {
    if(objs[i]->m_Type == OBJECT)
      {
      objs[i]->Print(out);
      }
    }
  cmXCodeObject::Indent(1, out);
  out << "};\n";
}


void cmXCodeObject::CopyAttributes(cmXCodeObject* copy)
{
  this->m_ObjectAttributes = copy->m_ObjectAttributes;
  this->m_List = copy->m_List;
  this->m_String = copy->m_String;
  this->m_Object = copy->m_Object;
}

void cmXCodeObject::SetString(const char* s)
{
  std::string ss = s;
  if(ss.size() == 0)
    {
    m_String = "\"\"";
    return;
    }
  bool needQuote = false;
  m_String = "";
  if(ss.find_first_of(" <>.+-=") != ss.npos)
    {
    needQuote = true;
    }
  if(needQuote)
    {
    m_String = "\"";
    }
  m_String += s;
  if(needQuote)
    {
    m_String += "\"";
    }
}
