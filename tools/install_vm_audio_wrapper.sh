#!/usr/bin/env bash
set -euo pipefail
if [ ! -f "tools/churchwars_audio_wrapper.sh" ]; then
  echo "Run from repo root: tools/churchwars_audio_wrapper.sh not found" >&2
  exit 1
fi
if [ "$(id -u)" -ne 0 ]; then
  echo "Re-running with sudo..."
  exec sudo bash "$0" "$@"
fi
if [ -x /usr/local/bin/churchwars ] && ! [ -x /usr/local/bin/churchwars.real ]; then
  mv /usr/local/bin/churchwars /usr/local/bin/churchwars.real
fi
install -m 0755 tools/churchwars_audio_wrapper.sh /usr/local/bin/churchwars
echo "Installed wrapper. Launch with: churchwars"
echo "To undo: sudo mv /usr/local/bin/churchwars.real /usr/local/bin/churchwars"
