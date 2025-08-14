#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")"/.. && pwd)"
cd "$ROOT"

# macOS Homebrew env (quiet if not installed)
if command -v brew >/dev/null 2>&1; then
  export PATH="$(brew --prefix gettext 2>/dev/null)/bin:$PATH"
  export PKG_CONFIG_PATH="$(brew --prefix gettext 2>/dev/null)/lib/pkgconfig:$(brew --prefix)/lib/pkgconfig:$(brew --prefix gtk+3 2>/dev/null)/lib/pkgconfig:$(brew --prefix glib 2>/dev/null)/lib/pkgconfig:${PKG_CONFIG_PATH:-}"
fi

# Ensure the game’s expected paths exist: put every *.wav under src/
mkdir -p src
# Copy (not symlink) to handle weird paths/spaces; zsh-safe by using bash here
bash -lc 'find . -type f -iname "*.wav" ! -path "./src/*" -print0 | while IFS= read -r -d "" f; do install -m 0644 "$f" "src/$(basename "$f")"; done'

# Optional re-encode for mac (enable with REENCODE_MAC_WAVS=1)
if [[ "${REENCODE_MAC_WAVS:-0}" == "1" ]] && command -v ffmpeg >/dev/null 2>&1; then
  mkdir -p src/_mac
  for f in src/*.wav; do
    ffmpeg -y -i "$f" -ar 44100 -ac 2 -sample_fmt s16 "src/_mac/$(basename "$f")"
  done
  install -m 0644 src/_mac/*.wav src/ || true
fi

# Build
[ -f ./autogen.sh ] && ./autogen.sh || autoreconf -fi
./configure
make -j"$( (sysctl -n hw.ncpu 2>/dev/null) || echo 4 )"

# Make an SDL-only plugin dir so macOS uses SDL (not Cocoa)
make -C src/plugins
mkdir -p src/plugins/sdl_only
cp src/plugins/.libs/libsound_sdl.* src/plugins/sdl_only/ 2>/dev/null || true

# Run with SDL/CoreAudio backend
mkdir -p "$HOME/.local/share/churchwars"
export CHURCHWARS_PLUGIN_DIR="$ROOT/src/plugins/sdl_only"
export DOPEWARS_PLUGIN_DIR="$CHURCHWARS_PLUGIN_DIR"
export SDL_AUDIODRIVER="${SDL_AUDIODRIVER:-coreaudio}"
exec ./src/churchwars -f "$HOME/.local/share/churchwars/churchwars.sco"
