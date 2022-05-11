#!/usr/bin/env bash
set -e
readonly usage='usage: sign-notarize.bash -i <id> -d <dev-acct> -k <key-item> [-p <provider>] [--] <package>.dmg

Sign and notarize the "CMake.app" bundle inside the given "<package>.dmg" disk image.
Also produce a "<package>.tar.gz" tarball containing the same "CMake.app".

Options:

    -i <id>                Signing Identity
    -d <dev-acct>          Developer account name
    -k <key-item>          Keychain item containing account credentials
    -p <provider>          Provider short name
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
dev_acct=''
key_item=''
provider=''
while test "$#" != 0; do
    case "$1" in
    -i) shift; id="$1" ;;
    -d) shift; dev_acct="$1" ;;
    -k) shift; key_item="$1" ;;
    -p) shift; provider="$1" ;;
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
if test -z "$id" -o -z "$dev_acct" -o -z "$key_item"; then
    die "$usage"
fi
if test -n "$provider"; then
    provider="--provider $provider"
fi

# Verify environment.
if ! xcnotary="$(type -p xcnotary)"; then
    die "'xcnotary' not found in PATH"
fi
readonly xcnotary

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

# Extract SLA
readonly sla_xml="$tmpdir/sla.xml"
hdiutil udifderez -xml "$dmg" > "$sla_xml"
plutil -remove 'blkx' "$sla_xml"
plutil -remove 'plst' "$sla_xml"

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

xcnotary notarize "$vol_path/CMake.app" -d "$dev_acct" -k "$key_item" $provider

# Create a tarball of the volume next to the original disk image.
readonly tar_gz="${dmg/%.dmg/.tar.gz}"
tar cvzf "$tar_gz" -C /Volumes "$vol_name/CMake.app"

# Unmount the modified udrw image.
hdiutil detach "$vol_path"

# Convert back to read-only, compressed image.
hdiutil convert "${udrw_dmg}" -format UDZO -imagekey zlib-level=9 -ov -o "$dmg"

# Re-insert SLA.
hdiutil udifrez -xml "${sla_xml}" 'FIXME_WHY_IS_THIS_ARGUMENT_NEEDED' "$dmg"
