#!/usr/bin/env bash
export SDL_AUDIODRIVER=alsa
export SDL_ALSA_PCM_DEVICE=default
export SDL_AUDIO_ALSA_SET_BUFFER_SIZE=16384
exec "$(dirname "$0")/../src/churchwars" "$@"
