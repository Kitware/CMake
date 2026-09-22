{
  "name": "pc-link-test",
  "requires": {
    "cps-pkg-test": {
      "components": ["cps-pkg-test"],
      "extensions": {
        "cmake": {
          "domains@v1": ["unrecognized-domain"]
        }
      }
    }
  },
  "components": {
    "UnknownDomain": {
      "type": "interface",
      "requires": ["cps-pkg-test:cps-pkg-test"]
    }
  }
}
