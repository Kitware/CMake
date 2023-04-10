PROGRAM MAINF90
  USE libraryModuleA
  USE libraryModuleB
  USE subdirModuleA
  USE externalMod
  USE libraryCycleA
  USE libraryCycleB
  CALL printExtModGreeting
  CALL libraryCycleA2
  CALL libraryCycleB2
END PROGRAM MAINF90
