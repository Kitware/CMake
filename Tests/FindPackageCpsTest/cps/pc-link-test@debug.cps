{
  "name": "pc-link-test",
  "configuration": "debug",
  "requires": {
    "cps-pkg-test": {},
    "cps-pkg-unavailable": {}
  },
  "components": {
    "Parent": {
      "compile_requires": ["cps-pkg-test:cps-pkg-test"],
      "link_requires": ["cps-pkg-test:cps-pkg-test"],
      "dyld_requires": ["cps-pkg-test:cps-pkg-test"]
    }
  }
}
