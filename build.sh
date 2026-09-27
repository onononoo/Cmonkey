#!/bin/sh
# Builds core.wasm and assembles dist/firefox, dist/Cmonkey.xpi and dist/Cmonkey-source.zip.
set -e
cd "$(dirname "$0")"
PATH="$PATH:/c/Program Files/LLVM/bin"  # winget's LLVM doesn't add itself to PATH
clang --target=wasm32 -O2 -nostdlib -ffreestanding -Wl,--no-entry -o core.wasm core.c
rm -rf dist && mkdir -p dist/firefox
cp core.wasm core.js background.js popup.html popup.js manifest.json dist/firefox/

PY=$(command -v python || command -v python3)
(cd dist/firefox && "$PY" -m zipfile -c ../Cmonkey.xpi *)
# AMO wants the C source alongside the compiled wasm.
"$PY" -m zipfile -c dist/Cmonkey-source.zip core.c test.c core.js background.js popup.html popup.js \
  manifest.json build.sh README.md LICENSE
echo "built dist/firefox dist/Cmonkey.xpi dist/Cmonkey-source.zip"
