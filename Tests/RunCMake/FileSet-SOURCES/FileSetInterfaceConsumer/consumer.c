#include "consumer_id.h"
#include "owner_local.h"

#if DEF_ID != EXPECTED_ID
#  error DEF_ID does not match EXPECTED_ID of the consumer
#endif

#if OPT_ID != EXPECTED_ID
#  error OPT_ID does not match EXPECTED_ID of the consumer
#endif

#if INC_ID != EXPECTED_ID
#  error INC_ID does not match EXPECTED_ID of the consumer
#endif

#ifndef OWNER_LOCAL_DEF
#  error OWNER_LOCAL_DEF not defined
#endif

#ifndef OWNER_LOCAL_OPT
#  error OWNER_LOCAL_OPT not defined
#endif

#ifndef OWNER_LOCAL_INC
#  error OWNER_LOCAL_INC not defined
#endif

int main(void)
{
  return 0;
}
