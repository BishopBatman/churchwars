set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BIN=""
for cand in \
  "$ROOT/src/churchwars" \
  "$ROOT/src/dopewars" \
  "$ROOT/build/churchwars" \
  "$ROOT/build/src/churchwars"
do
  if [[ -x "$cand" ]]; then BIN="$cand"; break; fi
done

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
