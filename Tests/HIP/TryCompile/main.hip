int __host__ try_compile_hip_func(int x);

int main(int argc, char** argv)
{
  try_compile_hip_func(int(42));

  return 0;
}
