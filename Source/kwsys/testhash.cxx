#include "kwsysPrivate.h"
#include KWSYS_HEADER(hash_map.hxx)
#include KWSYS_HEADER(hash_set.hxx)
#include KWSYS_HEADER(ios/iostream)

#if defined(_MSC_VER)
# pragma warning (disable:4786)
#endif

template class kwsys::hash_map<const char*, int>;
template class kwsys::hash_set<int>;

bool test_hash_map()
{
  typedef kwsys::hash_map<const char*, int> mtype;
  mtype m;
  const char* keys[] = {"hello", "world"};
  m[keys[0]] = 1;
  m.insert(mtype::value_type(keys[1], 2));
  int sum = 0;
  for(mtype::iterator mi = m.begin(); mi != m.end(); ++mi)
    {
    kwsys_ios::cout << "Found entry [" << mi->first << "," << mi->second << "]"
                    << kwsys_ios::endl;
    sum += mi->second;
    }
  return sum == 3;
}

bool test_hash_set()
{
  typedef kwsys::hash_set<int> stype;
  stype s;
  s.insert(1);
  s.insert(2);
  int sum = 0;
  for(stype::iterator si = s.begin(); si != s.end(); ++si)
    {
    kwsys_ios::cout << "Found entry [" << *si << "]" << kwsys_ios::endl;
    sum += *si;
    }
  return sum == 3;
}

int main()
{
  bool result = true;
  result = test_hash_map() && result;
  result = test_hash_set() && result;
  return result? 0:1;
}
