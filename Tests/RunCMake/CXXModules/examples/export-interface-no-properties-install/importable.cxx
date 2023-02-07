export module importable;

extern "C++" {
int forwarding();
}

export int from_import()
{
  return forwarding();
}
