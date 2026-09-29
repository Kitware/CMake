{
  "cps_version": "0.13",
  "name": "Foo",
  "configuration": "default",
  "components": {
    "PrefixTest": {
      "includes": ["@prefix@/include"]
    },
    "ExecutableTest": {
      "location": "@prefix@/foo"
    }
  }
}
