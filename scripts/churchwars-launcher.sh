set -euo pipefail

SCRIPT_PATH="${BASH_SOURCE[0]}"
while [ -L "$SCRIPT_PATH" ]; do
  TARGET="$(readlink "$SCRIPT_PATH")"
  case "$TARGET" in
    /*) SCRIPT_PATH="$TARGET" ;;
    *)  SCRIPT_PATH="$(cd "$(dirname "$SCRIPT_PATH")" && cd "$(dirname "$TARGET")" && pwd)/$(basename "$TARGET")" ;;
  esac
done
SCRIPT_DIR="$(cd -- "$(dirname -- "$SCRIPT_PATH")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

if [[ -n "${CHURCHWARS_BIN:-}" && -x "${CHURCHWARS_BIN:-/nonexistent}" ]]; then
  BIN="$CHURCHWARS_BIN"
else
  BIN=""
  for cand in \
    "$ROOT/src/churchwars" \
    "$ROOT/src/dopewars" \
    "$ROOT/build/churchwars" \
    "$ROOT/build/src/churchwars" \
    "$ROOT/build/src/dopewars"
  do
    if [[ -x "$cand" ]]; then BIN="$cand"; break; fi
  done
fi

if [[ -z "${BIN:-}" ]]; then
  echo "churchwars-launcher: no binary found."
  echo "Build first, e.g.:"
  echo "  Autotools:  ./autogen.sh && ./configure && make"
  echo "  CMake:      rm -rf build && cmake -S . -B build && cmake --build build -j"
  exit 1
fi

SCORE_PATH="${CHURCHWARS_SCORE:-$HOME/.local/share/churchwars/churchwars.sco}"
mkdir -p "$(dirname "$SCORE_PATH")"

# When bundled as a macOS application, resources such as GTK data files,
# icon themes and the gdk-pixbuf loader cache live inside the bundle under
# "Contents/Resources".  Adjust a few environment variables so the bundled
# binaries can locate these resources without additional configuration.
if [[ "$(uname)" == "Darwin" ]]; then
  RESOURCES_DIR="$ROOT/Resources"
  if [[ -d "$RESOURCES_DIR" ]]; then
    export GTK_DATA_PREFIX="$RESOURCES_DIR"
    export GTK_EXE_PREFIX="$RESOURCES_DIR"
    if CACHE_FILE=$(find "$RESOURCES_DIR" -path '*/gdk-pixbuf-2.0/*/loaders.cache' -print -quit 2>/dev/null); then
      export GDK_PIXBUF_MODULE_FILE="$CACHE_FILE"
    fi
    export XDG_DATA_DIRS="$RESOURCES_DIR/share:${XDG_DATA_DIRS:-/usr/local/share:/usr/share}"
  fi
fi

exec "$BIN" -f "$SCORE_PATH" "$@"
