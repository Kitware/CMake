{
  "cps_version": "0.13",
  "name": "includes",
  "cps_path": "@prefix@/cps",
  "components": {
    "default": {
      "type": "interface",
      "includes": {
        "bogus": [],
        "cxx": ["@prefix@/include/cxx"],
        "*": ["@prefix@/include"]
      }
    }
  }
}
