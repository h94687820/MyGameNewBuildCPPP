#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ANDROID_DIR="$ROOT_DIR/android"

if [[ -z "${ANDROID_SDK_ROOT:-}" && -n "${ANDROID_HOME:-}" ]]; then
    export ANDROID_SDK_ROOT="$ANDROID_HOME"
fi

if [[ -z "${ANDROID_SDK_ROOT:-}" || ! -d "$ANDROID_SDK_ROOT" ]]; then
    cat >&2 <<'EOF'
Android SDK was not found.

Set ANDROID_SDK_ROOT to an Android SDK directory that contains:
  platform-tools/
  platforms/android-35/
  build-tools/35.0.0/

Then run this script again.
EOF
    exit 1
fi

if [[ -z "${ANDROID_NDK_ROOT:-}" && -n "${ANDROID_NDK_HOME:-}" ]]; then
    export ANDROID_NDK_ROOT="$ANDROID_NDK_HOME"
fi

if [[ -z "${ANDROID_NDK_ROOT:-}" || ! -d "$ANDROID_NDK_ROOT" ]]; then
    cat >&2 <<'EOF'
Android NDK was not found.

Set ANDROID_NDK_ROOT to an installed NDK directory (r25c or newer),
then run this script again.
EOF
    exit 1
fi

cat > "$ANDROID_DIR/local.properties" <<EOF
sdk.dir=$ANDROID_SDK_ROOT
ndk.dir=$ANDROID_NDK_ROOT
EOF

cd "$ANDROID_DIR"
gradle --no-daemon --stacktrace assembleDebug

APK="$ANDROID_DIR/app/build/outputs/apk/debug/app-debug.apk"
if [[ -f "$APK" ]]; then
    printf 'APK created: %s\n' "$APK"
else
    echo "Gradle finished but the expected APK was not found." >&2
    exit 1
fi