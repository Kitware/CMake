include(FeatureSummary)

cmake_policy(SET CMP0183 NEW)

set(FOO "lower")
add_feature_info(Foo "FOO MATCHES \"(UPPER|lower)\"" "Foo.")

feature_summary(WHAT ALL)
