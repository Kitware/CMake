{
  "cps_version": "0.15.0",
  "name": "pc-link-test",
  "cps_path": "@prefix@/cps",
  "components": {
    "Parent": {
      "type": "interface",
      "requires": ["cps-pkg-test:cps-pkg-test"]
    }
  },
  "requires": {
    "cps-pkg-test": {
      "components": ["cps-pkg-test"],
      "extensions": {
        "cmake": {
          "domains@v1": ["unrecognized-domain", "pkg-config"]
        }
      }
    }
  }
}
