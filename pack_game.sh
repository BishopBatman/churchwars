#!/usr/bin/env bash
set -euo pipefail

BINARY_NAME="churchwars"

echo "==> Preparing files…"

mkdir -p src
find sounds -type f -iname '*.wav' -exec cp -f {} src/ \; 2>/dev/null || true

if [ -x "build/$BINARY_NAME" ]; then
  cp -f "build/$BINARY_NAME" "./$BINARY_NAME"
fi

cat > run.sh <<RUN
#!/usr/bin/env bash
set -e
export SDL_AUDIODRIVER=coreaudio
exec "\$(dirname "\$0")/$BINARY_NAME" "\$@"
RUN
chmod +x run.sh

STAMP=$(date +%Y%m%d-%H%M)
OUT="churchwars-mac-$STAMP.zip"

zip -r "$OUT" . -x '.git/*' -x '*.o' -x 'CMakeFiles/*' -x 'CMakeCache.txt' -x '*.obj' -x '*.pdb' -x '*.dSYM/*' >/dev/null

echo "==> Done. Created $OUT"
echo "   Send that zip to your Mac (or friends)."
