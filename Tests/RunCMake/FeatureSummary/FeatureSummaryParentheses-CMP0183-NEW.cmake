include(FeatureSummary)

cmake_policy(SET CMP0183 NEW)

set(WITH_FOO 1)
set(WITH_BAR 1)
set(WITH_BAZ 0)

add_feature_info(Foo "WITH_FOO AND (WITH_BAR OR WITH_BAZ)" "Foo.")

feature_summary(WHAT ALL)
