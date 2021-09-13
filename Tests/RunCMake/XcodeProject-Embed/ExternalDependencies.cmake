add_library(sharedFrameworkExt SHARED func.m)
set_target_properties(sharedFrameworkExt PROPERTIES FRAMEWORK TRUE)

add_library(sharedDylibExt SHARED func.m)
