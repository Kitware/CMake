/*=========================================================================

  Program:   KWSys - Kitware System Library
  Module:    $RCSfile$

  Copyright (c) Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "kwsysPrivate.h"
#include KWSYS_HEADER(Registry.hxx)

#include KWSYS_HEADER(Configure.hxx)
#include KWSYS_HEADER(ios/iostream)
#include KWSYS_HEADER(stl/string)
#include KWSYS_HEADER(stl/map)
#include KWSYS_HEADER(ios/iostream)
#include KWSYS_HEADER(ios/fstream)
#include KWSYS_HEADER(ios/sstream)

#include <ctype.h> // for isspace
#include <stdio.h>

#ifdef KWSYS_IOS_USE_ANSI
# define VTK_IOS_NOCREATE 
#else
# define VTK_IOS_NOCREATE | kwsys_ios::ios::nocreate
#endif
#define BUFFER_SIZE 8192

namespace KWSYS_NAMESPACE
{
class RegistryHelper {
public:
  RegistryHelper(Registry::RegistryType registryType);
  virtual ~RegistryHelper();

  // Read a value from the registry.
  virtual bool ReadValue(const char *key, char *value);

  // Delete a key from the registry.
  virtual bool DeleteKey(const char *key);

  // Delete a value from a given key.
  virtual bool DeleteValue(const char *key);

  // Set value in a given key.
  virtual bool SetValue(const char *key, 
    const char *value);

  // Open the registry at toplevel/subkey.
  virtual bool Open(const char *toplevel, const char *subkey, 
    int readonly);

  // Close the registry. 
  virtual bool Close();

  // Set the value of changed
  void SetChanged(bool b) { m_Changed = b; }
  void SetTopLevel(const char* tl);
  const char* GetTopLevel() { return m_TopLevel.c_str(); }

protected:
  bool m_Changed;
  kwsys_stl::string m_TopLevel;  

#ifdef WIN32
  HKEY HKey;
#endif
  // Strip trailing and ending spaces.
  char *Strip(char *str);
  void SetSubKey(const char* sk);
  char *CreateKey(const char *key);

  typedef kwsys_stl::map<kwsys_stl::string, kwsys_stl::string> StringToStringMap;
  StringToStringMap EntriesMap;
  kwsys_stl::string m_SubKey;
  bool m_Empty;
  bool m_SubKeySpecified;

  Registry::RegistryType m_RegistryType;
};


//----------------------------------------------------------------------------
Registry::Registry(Registry::RegistryType registryType)
{
  m_Opened      = false;
  m_Locked      = false;
  m_GlobalScope = false;
  this->Helper = 0;
  this->Helper = new RegistryHelper(registryType);
}

//----------------------------------------------------------------------------
Registry::~Registry()
{
  if ( m_Opened )
    {
    kwsys_ios::cerr << "Registry::Close should be "
                  "called here. The registry is not closed."
                  << kwsys_ios::endl;
    }
  delete this->Helper;
}

//----------------------------------------------------------------------------
bool Registry::Open(const char *toplevel,
  const char *subkey, int readonly)
{
  bool res = false;
  if ( m_Locked )
    {
    return 0;
    }
  if ( m_Opened )
    {
    if ( !this->Close() )
      {
      return false;
      }
    }
  if ( !toplevel || !*toplevel )
    {
    kwsys_ios::cerr << "Registry::Opened() Toplevel not defined" << kwsys_ios::endl;
    return false;
    }

  if ( isspace(toplevel[0]) || 
       isspace(toplevel[strlen(toplevel)-1]) )
    {
    kwsys_ios::cerr << "Toplevel has to start with letter or number and end"
      " with one" << kwsys_ios::endl;
    return 0;
    }

  res = this->Helper->Open(toplevel, subkey, readonly);
  if ( readonly != Registry::READONLY )
    {
    m_Locked = true;
    }
  
  if ( res )
    {
    m_Opened = true;
    this->Helper->SetTopLevel(toplevel);
    }
  return res;
}

//----------------------------------------------------------------------------
bool Registry::Close()
{
  bool res = false;
  if ( m_Opened )
    {
    res = this->Helper->Close();
    }

  if ( res )
    {
    m_Opened = false;
    m_Locked = false;
    this->Helper->SetChanged(false);
    }
  return res;
}

//----------------------------------------------------------------------------
bool Registry::ReadValue(const char *subkey, 
  const char *key, 
  char *value)
{  
  *value = 0;
  bool res = true;
  bool open = false;  
  if ( ! value )
    {
    return false;
    }
  if ( !m_Opened )
    {
    if ( !this->Open(this->GetTopLevel(), subkey, 
        Registry::READONLY) )
      {
      return false;
      }
    open = true;
    }
  res = this->Helper->ReadValue(key, value);

  if ( open )
    {
    if ( !this->Close() )
      {
      res = false;
      }
    }
  return res;
}

//----------------------------------------------------------------------------
bool Registry::DeleteKey(const char *subkey, const char *key)
{
  bool res = true;
  bool open = false;
  if ( !m_Opened )
    {
    if ( !this->Open(this->GetTopLevel(), subkey, 
        Registry::READWRITE) )
      {
      return false;
      }
    open = true;
    }

  res = this->Helper->DeleteKey(key);
  if ( res )
    {
    this->Helper->SetChanged(true);
    }

  if ( open )
    {
    if ( !this->Close() )
      {
      res = false;
      }
    }
  return res;
}

//----------------------------------------------------------------------------
bool Registry::DeleteValue(const char *subkey, const char *key)
{
  bool res = true;
  bool open = false;
  if ( !m_Opened )
    {
    if ( !this->Open(this->GetTopLevel(), subkey, 
        Registry::READWRITE) )
      {
      return false;
      }
    open = true;
    }

  res = this->Helper->DeleteValue(key);
  if ( res )
    {
    this->Helper->SetChanged(true);
    }

  if ( open )
    {
    if ( !this->Close() )
      {
      res = false;
      }
    }
  return res;
}

//----------------------------------------------------------------------------
bool Registry::SetValue(const char *subkey, const char *key, 
  const char *value)
{
  bool res = true;
  bool open = false;
  if ( !m_Opened )
    {
    if ( !this->Open(this->GetTopLevel(), subkey, 
        Registry::READWRITE) )
      {
      return false;
      }
    open = true;
    }

  res = this->Helper->SetValue( key, value );
  if ( res )
    {
    this->Helper->SetChanged(true);
    }

  if ( open )
    {
    if ( !this->Close() )
      {
      res = false;
      }
    }
  return res;
}

//----------------------------------------------------------------------------
const char* Registry::GetTopLevel()
{
  return this->Helper->GetTopLevel();
}

//----------------------------------------------------------------------------
void Registry::SetTopLevel(const char* tl)
{
  this->Helper->SetTopLevel(tl);
}

//----------------------------------------------------------------------------
void RegistryHelper::SetTopLevel(const char* tl)
{
  if ( tl )
    {
    m_TopLevel = tl;
    }
  else
    {
    m_TopLevel = "";
    }
}

//----------------------------------------------------------------------------
RegistryHelper::RegistryHelper(Registry::RegistryType registryType)
{
  m_Changed = false;
  m_TopLevel    = "";
  m_SubKey  = "";
  m_SubKeySpecified = false;
  m_Empty       = true;
  m_RegistryType = registryType;
}

//----------------------------------------------------------------------------
RegistryHelper::~RegistryHelper()
{
}


//----------------------------------------------------------------------------
bool RegistryHelper::Open(const char *toplevel, const char *subkey,
  int readonly)
{  
#ifdef WIN32
  if ( m_RegistryType == Registry::RegistryType::WIN32)
    {
    HKEY scope = HKEY_CURRENT_USER;
    if ( this->GetGlobalScope() )
      {
      scope = HKEY_LOCAL_MACHINE;
      }
    int res = 0;
    ostrstream str;
    DWORD dwDummy;
    str << "Software\\Kitware\\" << toplevel << "\\" << subkey << ends;
    if ( readonly == vtkKWRegistryUtilities::READONLY )
      {
      res = ( RegOpenKeyEx(scope, str.str(), 
          0, KEY_READ, &this->HKey) == ERROR_SUCCESS );
      }
    else
      {
      res = ( RegCreateKeyEx(scope, str.str(),
          0, "", REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE, 
          NULL, &this->HKey, &dwDummy) == ERROR_SUCCESS );    
      }
    str.rdbuf()->freeze(0);
    return res;
    }
#endif
  if ( m_RegistryType == Registry::UNIX_REGISTRY )
    {
    int res = 0;
    int cc;
    kwsys_ios::ostringstream str;
    if ( !getenv("HOME") )
      {
      return 0;
      }
    str << getenv("HOME") << "/." << toplevel << "rc";
    if ( readonly == Registry::READWRITE )
      {
      kwsys_ios::ofstream ofs( str.str().c_str(), kwsys_ios::ios::out|kwsys_ios::ios::app );
      if ( ofs.fail() )
        {
        return 0;
        }
      ofs.close();
      }

    kwsys_ios::ifstream *ifs = new kwsys_ios::ifstream(str.str().c_str(), kwsys_ios::ios::in VTK_IOS_NOCREATE);
    if ( !ifs )
      {
      return 0;
      }
    if ( ifs->fail())
      {
      delete ifs;
      return 0;
      }

    res = 1;
    char buffer[BUFFER_SIZE];
    while( !ifs->fail() )
      {
      int found = 0;
      ifs->getline(buffer, BUFFER_SIZE);
      if ( ifs->fail() || ifs->eof() )
        {
        break;
        }
      char *line = this->Strip(buffer);
      if ( *line == '#'  || *line == 0 )
        {
        // Comment
        continue;
        }   
      int linelen = static_cast<int>(strlen(line));
      for ( cc = 0; cc < linelen; cc++ )
        {
        if ( line[cc] == '=' )
          {
          char *key = new char[ cc+1 ];
          strncpy( key, line, cc );
          key[cc] = 0;
          char *value = line + cc + 1;
          char *nkey = this->Strip(key);
          char *nvalue = this->Strip(value);
          this->EntriesMap[nkey] = nvalue;
          m_Empty = 0;
          delete [] key;
          found = 1;      
          break;
          }
        }
      }
    ifs->close();
    this->SetSubKey( subkey );
    delete ifs;
    return res;
    }
  return false;
}

//----------------------------------------------------------------------------
bool RegistryHelper::Close()
{
#ifdef WIN32
  if ( m_RegistryType == Registry::RegistryType::WIN32)
    {
    int res;
    res = ( RegCloseKey(this->HKey) == ERROR_SUCCESS );    
    return res;
    }
#else
  if ( m_RegistryType == Registry::UNIX_REGISTRY )
    {
    int res = 0;
    if ( !m_Changed )
      {
      this->EntriesMap.erase(
        this->EntriesMap.begin(),
        this->EntriesMap.end());
      m_Empty = 1;
      this->SetSubKey(0);
      return 1;
      }

    kwsys_ios::ostringstream str;
    if ( !getenv("HOME") )
      {
      return 0;
      }
    str << getenv("HOME") << "/." << this->GetTopLevel() << "rc";
    kwsys_ios::ofstream *ofs = new kwsys_ios::ofstream(str.str().c_str(), kwsys_ios::ios::out);
    if ( !ofs )
      {
      return 0;
      }
    if ( ofs->fail())
      {
      delete ofs;
      return 0;
      }
    *ofs << "# This file is automatically generated by the application" << kwsys_ios::endl
      << "# If you change any lines or add new lines, note that all" << kwsys_ios::endl
      << "# coments and empty lines will be deleted. Every line has" << kwsys_ios::endl
      << "# to be in format: " << kwsys_ios::endl
      << "# key = value" << kwsys_ios::endl
      << "#" << kwsys_ios::endl;

    if ( !this->EntriesMap.empty() )
      {
      RegistryHelper::StringToStringMap::iterator it;
      for ( it = this->EntriesMap.begin();
        it != this->EntriesMap.end();
        ++ it )
        {
        *ofs << it->first.c_str() << " = " << it->second.c_str()<< kwsys_ios::endl;
        }
      }
    this->EntriesMap.erase(
      this->EntriesMap.begin(),
      this->EntriesMap.end());
    ofs->close();
    delete ofs;
    res = 1;
    this->SetSubKey(0);
    m_Empty = 1;
    return res;
    }
#endif
  return false;
}

//----------------------------------------------------------------------------
bool RegistryHelper::ReadValue(const char *skey, char *value)

{
#ifdef WIN32
  if ( m_RegistryType == Registry::RegistryType::WIN32)
    {
    int res = 1;
    DWORD dwType, dwSize;  
    dwType = REG_SZ;
    dwSize = BUFFER_SIZE;
    res = ( RegQueryValueEx(this->HKey, key, NULL, &dwType, 
        (BYTE *)value, &dwSize) == ERROR_SUCCESS );
    return res;
    }
#else
  if ( m_RegistryType == Registry::UNIX_REGISTRY )
    {
    int res = 0;
    char *key = this->CreateKey( skey );
    if ( !key )
      {
      return 0;
      }
    RegistryHelper::StringToStringMap::iterator it
      = this->EntriesMap.find(key);
    if ( it != this->EntriesMap.end() )
      {
      strcpy(value, it->second.c_str());
      res = 1;
      }
    delete [] key;
    return res;
    }
#endif
  return false;
}

//----------------------------------------------------------------------------
bool RegistryHelper::DeleteKey(const char* key)
{
#ifdef WIN32
  if ( m_RegistryType == Registry::RegistryType::WIN32)
    {
    int res = 1;
    res = ( RegDeleteKey( this->HKey, key ) == ERROR_SUCCESS );
    return res;
    }
#else
  if ( m_RegistryType == Registry::UNIX_REGISTRY )
    {
    (void)key;
    int res = 0;
    return res;
    }
#endif
  return false;
}

//----------------------------------------------------------------------------
bool RegistryHelper::DeleteValue(const char *skey)
{
#ifdef WIN32
  if ( m_RegistryType == Registry::RegistryType::WIN32)
    {
    int res = 1;
    res = ( RegDeleteValue( this->HKey, key ) == ERROR_SUCCESS );
    return res;
    }
#else
  if ( m_RegistryType == Registry::UNIX_REGISTRY )
    {
    char *key = this->CreateKey( skey );
    if ( !key )
      {
      return 0;
      }
    this->EntriesMap.erase(key);
    delete [] key;
    return 1;
    }
#endif
  return false;
}

//----------------------------------------------------------------------------
bool RegistryHelper::SetValue(const char *skey, const char *value)
{
#ifdef WIN32
  if ( m_RegistryType == Registry::RegistryType::WIN32)
    {
    int res = 1;
    DWORD len = (DWORD)(value ? strlen(value) : 0);
    res = ( RegSetValueEx(this->HKey, key, 0, REG_SZ, 
        (CONST BYTE *)(const char *)value, 
        len+1) == ERROR_SUCCESS );
    return res;
    }
#else
  if ( m_RegistryType == Registry::UNIX_REGISTRY )
    {
    char *key = this->CreateKey( skey );
    if ( !key )
      {
      return 0;
      }
    this->EntriesMap[key] = value;
    delete [] key;
    return 1;
    }
#endif
  return false;
}

//----------------------------------------------------------------------------
char *RegistryHelper::CreateKey( const char *key )
{
  char *newkey;
  if ( !m_SubKeySpecified || m_SubKey.empty() || !key )
    {
    return 0;
    }
  int len = strlen(this->m_SubKey.c_str()) + strlen(key) + 1;
  newkey = new char[ len+1 ] ;
  ::sprintf(newkey, "%s\\%s", this->m_SubKey.c_str(), key);
  return newkey;
}

void RegistryHelper::SetSubKey(const char* sk)
{
  if ( !sk )
    {
    m_SubKey = "";
    m_SubKeySpecified = false;
    }
  else
    {
    m_SubKey = sk;
    m_SubKeySpecified = true;
    }
}

//----------------------------------------------------------------------------
char *RegistryHelper::Strip(char *str)
{
  int cc;
  int len;
  char *nstr;
  if ( !str )
    {
    return NULL;
    }  
  len = strlen(str);
  nstr = str;
  for( cc=0; cc<len; cc++ )
    {
    if ( !isspace( *nstr ) )
      {
      break;
      }
    nstr ++;
    }
  for( cc=(strlen(nstr)-1); cc>=0; cc-- )
    {
    if ( !isspace( nstr[cc] ) )
      {
      nstr[cc+1] = 0;
      break;
      }
    }
  return nstr;
}

} // namespace KWSYS_NAMESPACE
