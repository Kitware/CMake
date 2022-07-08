#!/usr/bin/env bash
set -e
readonly usage='usage: sign-notarize.bash -i <id> -k <keychain-profile> [--] <package>.dmg

Sign and notarize the "CMake.app" bundle inside the given "<package>.dmg" disk image.
Also produce a "<package>.tar.gz" tarball containing the same "CMake.app".

Options:

    -i <id>                Signing Identity
    -k <keychain-profile>  Keychain profile containing stored credentials

Create the keychain profile ahead of time using

    xcrun notarytool store-credentials <keychain-profile> \
      --apple-id <dev-acct> --team-id <team-id> [--password <app-specific-password>]

where:

    <dev-acct>              is an Apple ID of a developer account
    <team-id>               is from https://developer.apple.com/account/#!/membership
    <app-specific-password> is generated via https://support.apple.com/en-us/HT204397
                            If --password is omitted, notarytool will prompt for it.

This creates a keychain item called "com.apple.gke.notary.tool" with an
account name "com.apple.gke.notary.tool.saved-creds.<keychain-profile>".
'

cleanup() {
    if test -d "$tmpdir"; then
        rm -rf "$tmpdir"
    fi
    if test -d "$vol_path"; then
        hdiutil detach "$vol_path"
    fi
}

trap "cleanup" EXIT

die() {
    echo "$@" 1>&2; exit 1
}

id=''
keychain_profile=''
while test "$#" != 0; do
    case "$1" in
    -i) shift; id="$1" ;;
    -k) shift; keychain_profile="$1" ;;
    --) shift ; break ;;
    -*) die "$usage" ;;
    *) break ;;
    esac
    shift
done
case "$1" in
*.dmg) readonly dmg="$1"; shift ;;
*) die "$usage" ;;
esac
test "$#" = 0 || die "$usage"

# Verify arguments.
if test -z "$id" -o -z "$keychain_profile"; then
    die "$usage"
fi

# Verify environment.
if ! xcrun --find notarytool 2>/dev/null; then
    die "'xcrun notarytool' not found"
fi

readonly tmpdir="$(mktemp -d)"

# Prepare entitlements.
readonly entitlements_xml="$tmpdir/entitlements.xml"
echo '<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>com.apple.security.cs.allow-dyld-environment-variables</key>
  <true/>
</dict>
</plist>' > "$entitlements_xml"

# Convert from read-only original image to read-write.
readonly udrw_dmg="$tmpdir/udrw.dmg"
hdiutil convert "$dmg" -format UDRW -o "${udrw_dmg}"

# Mount the temporary udrw image.
readonly vol_name="$(basename "${dmg%.dmg}")"
readonly vol_path="/Volumes/$vol_name"
hdiutil attach "${udrw_dmg}"

codesign --verify --timestamp --options=runtime --verbose --deep \
  -s "$id" \
  --entitlements "$entitlements_xml" \
  "$vol_path/CMake.app/Contents/bin/cmake" \
  "$vol_path/CMake.app/Contents/bin/ccmake" \
  "$vol_path/CMake.app/Contents/bin/ctest" \
  "$vol_path/CMake.app/Contents/bin/cpack" \
  "$vol_path/CMake.app"

ditto -c -k --keepParent "$vol_path/CMake.app" "$tmpdir/CMake.app.zip"
xcrun notarytool submit "$tmpdir/CMake.app.zip" --keychain-profile "$keychain_profile" --wait
xcrun stapler staple "$vol_path/CMake.app"

# Create a tarball of the volume next to the original disk image.
readonly tar_gz="${dmg/%.dmg/.tar.gz}"
tar cvzf "$tar_gz" -C /Volumes "$vol_name/CMake.app"

# Unmount the modified udrw image.
hdiutil detach "$vol_path"

# Convert back to read-only, compressed image.
hdiutil convert "${udrw_dmg}" -format UDZO -imagekey zlib-level=9 -ov -o "$dmg"
