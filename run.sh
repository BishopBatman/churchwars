#!/usr/bin/env bash
set -e
export SDL_AUDIODRIVER=coreaudio
exec "$(dirname "$0")/churchwars" "$@"
