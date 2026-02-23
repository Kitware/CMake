{
  "cps_version": "0.13",
  "name": "BadPrefix2",
  "cps_path": "@prefix@/if the specified prefix is longer than the actual prefix, it obviously doesn't match/however, the code to compare path tails just subtracted without checking/that could cause CMake to try to pass a negative number as a string offset/that would result in an indexing error that could lead to a seg-fault/since we don't know where the test is running, it's hard to ensure that this string is longer than the actual path/so let's just make it absurdly long and hope for the best/this should be okay on at least some instances, which will be enough to let us know if there's still a problem/cps",
  "components": {}
}
