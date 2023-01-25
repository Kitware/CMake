export module importable;

int forwarding();

export int from_import()
{
  return forwarding();
}
