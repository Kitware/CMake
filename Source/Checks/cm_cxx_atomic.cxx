#include <atomic>
int main()
{
  std::atomic<long long>(0).load();
  return 0;
}
