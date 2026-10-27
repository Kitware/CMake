#ifndef SHARED_ID
#  error SHARED_ID not defined
#endif

#if SHARED_ID != EXPECTED_ID
#  error SHARED_ID does not match EXPECTED_ID of the compiled target
#endif

int main(void)
{
  return 0;
}
