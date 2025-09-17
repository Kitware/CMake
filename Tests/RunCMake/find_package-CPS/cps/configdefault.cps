{
  "cps_version": "0.13",
  "name": "ConfigDefault",
  "cps_path": "@prefix@/cps",
  "configurations": [ "second", "first" ],
  "components": {
    "target": {
      "type": "interface",
      "configurations": {
        "first": {
          "definitions": {
            "*": {
              "NAME": "first",
              "MARKER": "1"
            }
          }
        }
      }
    }
  }
}
