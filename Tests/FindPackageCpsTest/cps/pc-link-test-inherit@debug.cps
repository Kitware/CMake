{
  "name": "pc-link-test",
  "configuration": "debug",
  "requires": {
    "cps-pkg-test": {},
    "cps-pkg-appendix": {},
    "cps-pkg-unavailable": {}
  },
  "components": {
    "InheritsRequirement": {
      "requires": ["cps-pkg-test:cps-pkg-test", "cps-pkg-appendix:cps-pkg-appendix"]
    }
  }
}
