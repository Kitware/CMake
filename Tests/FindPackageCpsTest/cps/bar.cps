{
  "cps_version": "0.13",
  "name": "Bar",
  "cps_path": "@prefix@/cps",
  "requires": {
    "Dep1": null,
    "Dep2": null
  },
  "components": {
    "Target1": {
      "type": "interface",
      "requires": [ "Dep1:Target", "Dep2:Target" ]
    }
  }
}
