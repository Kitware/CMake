{
  "cps_version": "0.13",
  "name": "animal",
  "cps_path": "@prefix@",
  "configurations": [ "default" ],
  "components": {
    "animal": {
      "type": "interface",
      "configurations": {
        "cat": {
          "requires": [ ":cat" ]
        },
        "dog": {
          "requires": [ ":dog" ]
        }
      }
    },
    "cat": {
      "type": "interface",
      "configurations": {
        "tame": {
          "definitions": {
            "*": {
              "NOISE": "meow"
            }
          }
        },
        "wild": {
          "definitions": {
            "*": {
              "NOISE": "roar"
            }
          }
        }
      }
    },
    "dog": {
      "type": "interface",
      "configurations": {
        "tame": {
          "definitions": {
            "*": {
              "NOISE": "whine"
            }
          }
        },
        "wild": {
          "definitions": {
            "*": {
              "NOISE": "growl"
            }
          }
        }
      }
    }
  }
}
