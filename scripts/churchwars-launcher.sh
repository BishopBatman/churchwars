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

exec "$BIN" -f "$SCORE_PATH" "$@"
