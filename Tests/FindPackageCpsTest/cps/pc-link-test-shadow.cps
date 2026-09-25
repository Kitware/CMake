{
  "name": "pc-link-test",
  "requires": {
    "cps-pkg-test": {
      "components": ["cps-pkg-test"]
    }
  },
  "components": {
    "ShadowsRequirement": {
      "type": "interface",
      "requires": ["cps-pkg-test:cps-pkg-test"],
      "compile_requires": ["cps-pkg-test:cps-pkg-test"],
      "link_requires": ["cps-pkg-test:cps-pkg-test"],
      "dyld_requires": ["cps-pkg-test:cps-pkg-test"]
    }
  }
}
