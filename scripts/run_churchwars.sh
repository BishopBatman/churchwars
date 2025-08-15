#!/usr/bin/env bash
set -euo pipefail
SCRIPT_PATH="$(readlink -f "${BASH_SOURCE[0]:-$0}")"
SCRIPT_DIR="$(dirname "$SCRIPT_PATH")"
REPO_DIR="$(cd "$SCRIPT_DIR/.." && pwd -P)"

export SDL_AUDIODRIVER=alsa
export SDL_ALSA_PCM_DEVICE=default
export SDL_AUDIO_ALSA_SET_BUFFER_SIZE=16384

exec "$REPO_DIR/src/churchwars" "$@"
