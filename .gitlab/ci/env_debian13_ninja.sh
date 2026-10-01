export MY_RUBY_HOME="/usr/local/rvm/rubies/ruby-3.2.2"

if test "$CMAKE_CI_NIGHTLY" = "true"; then
  source .gitlab/ci/android-ndk-env.sh
  source .gitlab/ci/android-sdk-env.sh
fi

if test -z "$CI_MERGE_REQUEST_ID"; then
  source .gitlab/ci/swift-env.sh
fi
