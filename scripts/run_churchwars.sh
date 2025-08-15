#!/usr/bin/env bash
export SDL_AUDIODRIVER=alsa
export SDL_ALSA_PCM_DEVICE=default
export AUDIODEV=default
exec "$(dirname "$0")/../src/churchwars" "$@"
