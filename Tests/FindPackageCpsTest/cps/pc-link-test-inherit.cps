{
  "name": "pc-link-test",
  "requires": {
    "cps-pkg-appendix": {
      "components": ["cps-pkg-appendix"],
      "extensions": {
        "cmake": {
          "domains@v1": ["pkg-config"]
        }
      }
    }
  },
  "components": {
    "InheritsRequirement": {
      "type": "interface",
      "requires": ["cps-pkg-test:cps-pkg-test", "cps-pkg-appendix:cps-pkg-appendix"]
    }
  }
}
