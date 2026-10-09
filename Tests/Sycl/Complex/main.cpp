bool device_available();
int mixed_value();
int mixed_kernel();
int main()
{
  if (!device_available()) {
    return 77;
  }
  return mixed_value() == 46 && mixed_kernel() == 42 ? 0 : 1;
}
