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
// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
# include "Registry.hxx.in"
# include "Configure.hxx.in"
# include "kwsys_stl.hxx.in"
# include "kwsys_stl_string.hxx.in"
# include "kwsys_stl_map.hxx.in"
# include "kwsys_ios_iostream.h.in"
# include "kwsys_ios_fstream.h.in"
# include "kwsys_ios_sstream.h.in"
#endif

#include <ctype.h> // for isspace
#include <stdio.h>
#include <string.h> /* strlen, strncpy */
#include <stdlib.h> /* getenv */

#ifdef _WIN32
# include <windows.h>
#endif


namespace KWSYS_NAMESPACE
{
class RegistryHelper {
public:
  RegistryHelper(Registry::RegistryType registryType);
  virtual ~RegistryHelper();

  // Read a value from the registry.
  virtual bool ReadValue(const char *key, const char **value);

  // Delete a key from the registry.
  virtual bool DeleteKey(const char *key);

  // Delete a value from a given key.
  virtual bool DeleteValue(const char *key);

  // Set value in a given key.
  virtual bool SetValue(const char *key, const char *value);

  // Open the registry at toplevel/subkey.
  virtual bool Open(const char *toplevel, const char *subkey,
    int readonly);

  // Close the registry.
  virtual bool Close();

  // Set the value of changed
  void SetChanged(bool b) { m_Changed = b; }
  void SetTopLevel(const char* tl);
  const char* GetTopLevel() { return m_TopLevel.c_str(); }

  //! Read from local or global scope. On Windows this mean from local machine
  // or local user. On unix this will read from $HOME/.Projectrc or
  // /etc/Project
  void SetGlobalScope(bool b);
  bool GetGlobalScope();

  kwsys_stl::string EncodeKey(const char* str);
  kwsys_stl::string EncodeValue(const char* str);
  kwsys_stl::string DecodeValue(const char* str);

protected:
  bool m_Changed;
  kwsys_stl::string m_TopLevel;
  bool m_GlobalScope;

#ifdef _WIN32
  HKEY HKey;
#endif
  // Strip trailing and ending spaces.
  char *Strip(char *str);
  void SetSubKey(const char* sk);
  kwsys_stl::string CreateKey(const char *key);

  typedef kwsys_stl::map<kwsys_stl::string, kwsys_stl::string> StringToStringMap;
  StringToStringMap EntriesMap;
  kwsys_stl::string m_SubKey;
  bool m_Empty;
  bool m_SubKeySpecified;
  kwsys_stl::string m_HomeDirectory;

  Registry::RegistryType m_RegistryType;
};

//----------------------------------------------------------------------------
#define Registry_BUFFER_SIZE 8192

//----------------------------------------------------------------------------
Registry::Registry(Registry::RegistryType registryType)
{
  m_Opened      = false;
  m_Locked      = false;
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
void Registry::SetGlobalScope(bool b)
{
  this->Helper->SetGlobalScope(b);
}

//----------------------------------------------------------------------------
bool Registry::GetGlobalScope()
{
  return this->Helper->GetGlobalScope();
}

//----------------------------------------------------------------------------
bool Registry::Open(const char *toplevel,
  const char *subkey, int readonly)
{
  bool res = false;
  if ( m_Locked )
    {
    return res;
    }
  if ( m_Opened )
    {
    if ( !this->Close() )
      {
      return res;
      }
    }
  if ( !toplevel || !*toplevel )
    {
    kwsys_ios::cerr << "Registry::Opened() Toplevel not defined"
      << kwsys_ios::endl;
    return res;
    }

  if ( isspace(toplevel[0]) ||
       isspace(toplevel[strlen(toplevel)-1]) )
    {
    kwsys_ios::cerr << "Toplevel has to start with letter or number and end"
      " with one" << kwsys_ios::endl;
    return res;
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
  const char **value)
{
  *value = 0;
  bool res = false;
  bool open = false;
  if ( ! value )
    {
    return res;
    }
  if ( !m_Opened )
    {
    if ( !this->Open(this->GetTopLevel(), subkey,
        Registry::READONLY) )
      {
      return res;
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
  bool res = false;
  bool open = false;
  if ( !m_Opened )
    {
    if ( !this->Open(this->GetTopLevel(), subkey,
        Registry::READWRITE) )
      {
      return res;
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
  bool res = false;
  bool open = false;
  if ( !m_Opened )
    {
    if ( !this->Open(this->GetTopLevel(), subkey,
        Registry::READWRITE) )
      {
      return res;
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
  bool res = false;
  bool open = false;
  if ( !m_Opened )
    {
    if ( !this->Open(this->GetTopLevel(), subkey,
        Registry::READWRITE) )
      {
      return res;
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
  m_GlobalScope = false;
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
  this->EntriesMap.clear();
  m_Empty = 1;

#ifdef _WIN32
  if ( m_RegistryType == Registry::WIN32_REGISTRY)
    {
    HKEY scope = HKEY_CURRENT_USER;
    if ( this->GetGlobalScope() )
      {
      scope = HKEY_LOCAL_MACHINE;
      }
    int res = 0;
    kwsys_ios::ostringstream str;
    DWORD dwDummy;
    str << "Software\\Kitware\\" << toplevel << "\\" << subkey;
    if ( readonly == Registry::READONLY )
      {
      res = ( RegOpenKeyEx(scope, str.str().c_str(),
          0, KEY_READ, &this->HKey) == ERROR_SUCCESS );
      }
    else
      {
      res = ( RegCreateKeyEx(scope, str.str().c_str(),
          0, "", REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE,
          NULL, &this->HKey, &dwDummy) == ERROR_SUCCESS );
      }
    if ( res != 0 )
      {
      this->SetSubKey( subkey );
      }
    return (res != 0);
    }
#endif
  if ( m_RegistryType == Registry::FILE_REGISTRY )
    {
    bool res = false;
    int cc;
    kwsys_ios::ostringstream str;
    const char* homeDirectory;
    if ( (homeDirectory = getenv("HOME")) == 0 )
      {
      if ( (homeDirectory = getenv("USERPROFILE")) == 0 )
        {
        return false;
        }
      }
    m_HomeDirectory = homeDirectory;
    str << m_HomeDirectory.c_str() << "/." << toplevel << "rc";
    if ( readonly == Registry::READWRITE )
      {
      kwsys_ios::ofstream ofs( str.str().c_str(), kwsys_ios::ios::out|kwsys_ios::ios::app );
      if ( ofs.fail() )
        {
        return false;
        }
      ofs.close();
      }

    kwsys_ios::ifstream *ifs = new kwsys_ios::ifstream(str.str().c_str(), kwsys_ios::ios::in
#ifndef KWSYS_IOS_USE_ANSI
      | kwsys_ios::ios::nocreate
#endif
      );
    if ( !ifs )
      {
      return false;
      }
    if ( ifs->fail())
      {
      delete ifs;
      return false;
      }

    res = true;
    char buffer[Registry_BUFFER_SIZE];
    while( !ifs->fail() )
      {
      ifs->getline(buffer, Registry_BUFFER_SIZE);
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
          this->EntriesMap[nkey] = this->DecodeValue(nvalue);
          m_Empty = 0;
          delete [] key;
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
#ifdef _WIN32
  if ( m_RegistryType == Registry::WIN32_REGISTRY)
    {
    int res;
    res = ( RegCloseKey(this->HKey) == ERROR_SUCCESS );
    return (res != 0);
    }
#endif
  if ( m_RegistryType == Registry::FILE_REGISTRY )
    {
    if ( !m_Changed )
      {
      this->SetSubKey(0);
      return true;
      }

    kwsys_ios::ostringstream str;
    str << m_HomeDirectory.c_str() << "/." << this->GetTopLevel() << "rc";
    kwsys_ios::ofstream *ofs = new kwsys_ios::ofstream(str.str().c_str(), kwsys_ios::ios::out);
    if ( !ofs )
      {
      return false;
      }
    if ( ofs->fail())
      {
      delete ofs;
      return false;
      }
    *ofs << "# This file is automatically generated by the application" << kwsys_ios::endl
      << "# If you change any lines or add new lines, note that all" << kwsys_ios::endl
      << "# comments and empty lines will be deleted. Every line has" << kwsys_ios::endl
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
        *ofs << it->first.c_str() << " = " << this->EncodeValue(it->second.c_str()).c_str() << kwsys_ios::endl;
        }
      }
    this->EntriesMap.clear();
    ofs->close();
    delete ofs;
    this->SetSubKey(0);
    m_Empty = 1;
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
bool RegistryHelper::ReadValue(const char *skey, const char **value)

{
#ifdef _WIN32
  if ( m_RegistryType == Registry::WIN32_REGISTRY)
    {
    kwsys_stl::string key = this->CreateKey( skey );
    if ( key.empty() )
      {
      return false;
      }
    DWORD dwType, dwSize;
    dwType = REG_SZ;
    char buffer[1024]; // Replace with RegQueryInfoKey
    dwSize = sizeof(buffer);
    int res = ( RegQueryValueEx(this->HKey, skey, NULL, &dwType,
        (BYTE *)buffer, &dwSize) == ERROR_SUCCESS );
    if ( !res )
      {
      return false;
      }
    this->EntriesMap[key] = buffer;
    RegistryHelper::StringToStringMap::iterator it
      = this->EntriesMap.find(key);
    *value = it->second.c_str();
    return true;
    }
#endif
  if ( m_RegistryType == Registry::FILE_REGISTRY )
    {
    bool res = false;
    kwsys_stl::string key = this->CreateKey( skey );
    if ( key.empty() )
      {
      return false;
      }

    RegistryHelper::StringToStringMap::iterator it
      = this->EntriesMap.find(key);
    if ( it != this->EntriesMap.end() )
      {
      *value = it->second.c_str();
      res = true;
      }
    return res;
    }
  return false;
}

//----------------------------------------------------------------------------
bool RegistryHelper::DeleteKey(const char* skey)
{
#ifdef _WIN32
  if ( m_RegistryType == Registry::WIN32_REGISTRY)
    {
    int res = ( RegDeleteKey( this->HKey, skey ) == ERROR_SUCCESS );
    return (res != 0);
    }
#endif
  if ( m_RegistryType == Registry::FILE_REGISTRY )
    {
    kwsys_stl::string key = this->CreateKey( skey );
    if ( key.empty() )
      {
      return false;
      }
    this->EntriesMap.erase(key);
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
bool RegistryHelper::DeleteValue(const char *skey)
{
#ifdef _WIN32
  if ( m_RegistryType == Registry::WIN32_REGISTRY)
    {
    int res = ( RegDeleteValue( this->HKey, skey ) == ERROR_SUCCESS );
    return (res != 0);
    }
#endif
  if ( m_RegistryType == Registry::FILE_REGISTRY )
    {
    kwsys_stl::string key = this->CreateKey( skey );
    if ( key.empty() )
      {
      return false;
      }
    this->EntriesMap.erase(key);
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
bool RegistryHelper::SetValue(const char *skey, const char *value)
{
#ifdef _WIN32
  if ( m_RegistryType == Registry::WIN32_REGISTRY)
    {
    DWORD len = (DWORD)(value ? strlen(value) : 0);
    int res = ( RegSetValueEx(this->HKey, skey, 0, REG_SZ,
        (CONST BYTE *)(const char *)value,
        len+1) == ERROR_SUCCESS );
    return (res != 0);
    }
#endif
  if ( m_RegistryType == Registry::FILE_REGISTRY )
    {
    kwsys_stl::string key = this->CreateKey( skey );
    if ( key.empty() )
      {
      return 0;
      }
    this->EntriesMap[key] = value;
    return 1;
    }
  return false;
}

//----------------------------------------------------------------------------
kwsys_stl::string RegistryHelper::CreateKey( const char *key )
{
  if ( !m_SubKeySpecified || m_SubKey.empty() || !key )
    {
    return "";
    }
  kwsys_ios::ostringstream ostr;
  ostr << this->EncodeKey(this->m_SubKey.c_str()).c_str()
       << "\\" << this->EncodeKey(key).c_str();
  return ostr.str();
}

//----------------------------------------------------------------------------
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
  size_t len;
  char *nstr;
  if ( !str )
    {
    return NULL;
    }
  len = strlen(str);
  nstr = str;
  for( cc=0; cc < static_cast<int>(len); cc++ )
    {
    if ( !isspace( *nstr ) )
      {
      break;
      }
    nstr ++;
    }
  for( cc= static_cast<int>(strlen(nstr))-1; cc>=0; cc-- )
    {
    if ( !isspace( nstr[cc] ) )
      {
      nstr[cc+1] = 0;
      break;
      }
    }
  return nstr;
}

//----------------------------------------------------------------------------
void RegistryHelper::SetGlobalScope(bool b)
{
  m_GlobalScope = b;
}

//----------------------------------------------------------------------------
bool RegistryHelper::GetGlobalScope()
{
  return m_GlobalScope;
}

//----------------------------------------------------------------------------
kwsys_stl::string RegistryHelper::EncodeKey(const char* str)
{
  kwsys_ios::ostringstream ostr;
  while ( *str )
    {
    switch ( *str )
      {
    case '%': case '=': case '\n': case '\r': case '\t':
      char buffer[4];
      sprintf(buffer, "%%%02X", *str);
      ostr << buffer;
      break;
    default:
      ostr << *str;
      }
    str ++;
    }
  return ostr.str();
}

//----------------------------------------------------------------------------
kwsys_stl::string RegistryHelper::EncodeValue(const char* str)
{
  kwsys_ios::ostringstream ostr;
  while ( *str )
    {
    switch ( *str )
      {
    case '%': case '=': case '\n': case '\r': case '\t':
      char buffer[4];
      sprintf(buffer, "%%%02X", *str);
      ostr << buffer;
      break;
    default:
      ostr << *str;
      }
    str ++;
    }
  return ostr.str();
}

//----------------------------------------------------------------------------
kwsys_stl::string RegistryHelper::DecodeValue(const char* str)
{
  kwsys_ios::ostringstream ostr;
  while ( *str )
    {
    unsigned int val;
    switch ( *str )
      {
    case '%':
      if ( *(str+1) && *(str+2) && sscanf(str+1, "%x", &val) == 1 )
        {
        ostr << static_cast<char>(val);
        str += 2;
        }
      else
        {
        ostr << *str;
        }
      break;
    default:
      ostr << *str;
      }
    str ++;
    }
  return ostr.str();
}

} // namespace KWSYS_NAMESPACE
