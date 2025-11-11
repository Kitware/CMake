curl -L -O "https://download.swift.org/swift-6.2.1-release/debian12/swift-6.2.1-RELEASE/swift-6.2.1-RELEASE-debian12.tar.gz"
echo 'd6405e4fb7f092cbb9973a892ce8410837b4335f67d95bf8607baef1f69939e4  swift-6.2.1-RELEASE-debian12.tar.gz' > swift.sha256sum
sha256sum --check swift.sha256sum
mkdir /opt/swift
tar xzf swift-6.2.1-RELEASE-debian12.tar.gz -C /opt/swift --strip-components=2
rm swift-6.2.1-RELEASE-debian12.tar.gz swift.sha256sum
export SWIFTC="/opt/swift/bin/swiftc"
