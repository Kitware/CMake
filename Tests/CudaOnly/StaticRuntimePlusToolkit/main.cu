

int shared_version();
int static_version();
int mixed_version();

int main()
{
  return mixed_version() == 0 && shared_version() == 0 &&
    static_version() == 0;
}
