cmake_minimum_required(VERSION 3.2)

# Make sure a policy set differently by our includer is now correct.
cmake_policy(GET CMP0056 cmp)
check(CMP0056 "NEW" "${cmp}")
