{
  "cps_version": "0.13",
  "name": "ConfigMapped",
  "cps_path": "@prefix@/cps",
  "configurations": [ "default" ],
  "components": {
    "target": {
      "type": "interface",
      "configurations": {
        "default": {
          "definitions": {
            "*": {
              "NAME": "default",
              "MARKER": "1"
            }
          }
        },
        "test": {
          "definitions": {
            "*": {
              "NAME": "test",
              "MARKER": "4"
            }
          }
        }
      }
    }
  }
}
