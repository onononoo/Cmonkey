#!/bin/sh
# Builds core.wasm and assembles dist/chrome and dist/firefox.
set -e
cd "$(dirname "$0")"
PATH="$PATH:/c/Program Files/LLVM/bin"  # winget's LLVM doesn't add itself to PATH
clang --target=wasm32 -O2 -nostdlib -ffreestanding -Wl,--no-entry -o core.wasm core.c
for b in chrome firefox; do
  rm -rf "dist/$b" && mkdir -p "dist/$b"
  cp core.wasm core.js background.js popup.html popup.js "dist/$b/"
  cp "manifest.$b.json" "dist/$b/manifest.json"
done

# Single-file packages: .xpi for Firefox, .zip for store uploads (both are plain zips).
PY=$(command -v python || command -v python3)
rm -f dist/Cmonkey.xpi dist/Cmonkey-chrome.zip
(cd dist/firefox && "$PY" -m zipfile -c ../Cmonkey.xpi *)
(cd dist/chrome && "$PY" -m zipfile -c ../Cmonkey-chrome.zip *)
echo "built dist/chrome dist/firefox dist/Cmonkey.xpi dist/Cmonkey-chrome.zip"
