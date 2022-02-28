export MY_RUBY_HOME="/usr/local/rvm/rubies/ruby-2.7.0"

if test -z "$CI_MERGE_REQUEST_ID"; then
  curl -L -O "https://download.swift.org/swift-5.5.3-release/ubuntu1804/swift-5.5.3-RELEASE/swift-5.5.3-RELEASE-ubuntu18.04.tar.gz"
  echo '910634e2d97e14c43ed1f29caeb57fd01d10c2ff88cebb79baee1016b52c7492  swift-5.5.3-RELEASE-ubuntu18.04.tar.gz' > swift.sha256sum
  sha256sum --check swift.sha256sum
  mkdir /opt/swift
  tar xzf swift-5.5.3-RELEASE-ubuntu18.04.tar.gz -C /opt/swift --strip-components=2
  rm swift-5.5.3-RELEASE-ubuntu18.04.tar.gz swift.sha256sum
  export SWIFTC="/opt/swift/bin/swiftc"
fi
