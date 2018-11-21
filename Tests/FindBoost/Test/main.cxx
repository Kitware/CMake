#include <boost/filesystem.hpp>
#include <boost/thread.hpp>

namespace {

boost::mutex m1;
boost::recursive_mutex m2;

void threadmain()
{
  boost::lock_guard<boost::mutex> lock1(m1);
  boost::lock_guard<boost::recursive_mutex> lock2(m2);

  boost::filesystem::path p(boost::filesystem::current_path());
}
}

int main()
{
  boost::thread foo(threadmain);
  foo.join();

  int version = BOOST_VERSION;
  int major = version / 100000;
  int minor = version / 100 % 1000;
  int patch = version % 100;
  char version_string[100];
  snprintf(version_string, sizeof(version_string), "%d.%d.%d", major, minor,
           patch);
  printf("Found Boost version %s, expected version %s\n", version_string,
         CMAKE_EXPECTED_BOOST_VERSION_COMPONENTS);
  int ret = strcmp(version_string, CMAKE_EXPECTED_BOOST_VERSION_COMPONENTS);
  char raw_version_string[100];
  snprintf(raw_version_string, sizeof(raw_version_string), "%d",
           BOOST_VERSION);
  printf("Found Boost version %s, expected version %s\n", raw_version_string,
         CMAKE_EXPECTED_BOOST_VERSION);
  return ret | strcmp(raw_version_string, CMAKE_EXPECTED_BOOST_VERSION);
}
