#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")"/.. && pwd)"
cd "$ROOT"

# Homebrew env (quiet if not installed)
if command -v brew >/dev/null 2>&1; then
  export PATH="$(brew --prefix gettext 2>/dev/null)/bin:$PATH"
  export PKG_CONFIG_PATH="$(brew --prefix gettext 2>/dev/null)/lib/pkgconfig:$(brew --prefix)/lib/pkgconfig:$(brew --prefix gtk+3 2>/dev/null)/lib/pkgconfig:$(brew --prefix glib 2>/dev/null)/lib/pkgconfig:${PKG_CONFIG_PATH:-}"
fi

# Ensure the game’s expected paths exist: put every *.wav under src/
mkdir -p src
bash -lc 'find . -type f -iname "*.wav" ! -path "./src/*" -print0 | while IFS= read -r -d "" f; do install -m 0644 "$f" "src/$(basename "$f")"; done'

# Build (generate configure if needed)
[ -f ./autogen.sh ] && ./autogen.sh || autoreconf -fi
./configure
make -j"$( (sysctl -n hw.ncpu 2>/dev/null) || echo 4 )"

# Build plugins and point to SDL only
make -C src/plugins
mkdir -p src/plugins/sdl_only
cp src/plugins/.libs/libsound_sdl.* src/plugins/sdl_only/ 2>/dev/null || true

mkdir -p "$HOME/.local/share/churchwars"
export CHURCHWARS_PLUGIN_DIR="$ROOT/src/plugins/sdl_only"
export DOPEWARS_PLUGIN_DIR="$CHURCHWARS_PLUGIN_DIR"
export SDL_AUDIODRIVER="${SDL_AUDIODRIVER:-coreaudio}"
exec ./src/churchwars -f "$HOME/.local/share/churchwars/churchwars.sco"
