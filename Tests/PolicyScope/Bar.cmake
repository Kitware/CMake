cmake_minimum_required(VERSION 3.31)

# Make sure a policy set differently by our includer is now correct.
cmake_policy(GET CMP0180 cmp)
check(CMP0180 "NEW" "${cmp}")

# Test allowing the top-level file to not have cmake_minimum_required.
cmake_policy(SET CMP0000 OLD)
