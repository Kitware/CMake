curl -L -O "https://download.swift.org/swift-5.7.1-release/ubuntu1804/swift-5.7.1-RELEASE/swift-5.7.1-RELEASE-ubuntu18.04.tar.gz"
echo '2b30f9efc969d9e96f0836d0871130dffb369822a3823ee6f3db44c29c1698e3  swift-5.7.1-RELEASE-ubuntu18.04.tar.gz' > swift.sha256sum
sha256sum --check swift.sha256sum
mkdir /opt/swift
tar xzf swift-5.7.1-RELEASE-ubuntu18.04.tar.gz -C /opt/swift --strip-components=2
rm swift-5.7.1-RELEASE-ubuntu18.04.tar.gz swift.sha256sum
export SWIFTC="/opt/swift/bin/swiftc"
