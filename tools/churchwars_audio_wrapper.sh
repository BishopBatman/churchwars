#!/usr/bin/env bash
set -euo pipefail
export SDL_AUDIODRIVER="\${SDL_AUDIODRIVER:-alsa}"
export SDL_ALSA_PCM_DEVICE="\${SDL_ALSA_PCM_DEVICE:-default}"
export AUDIODEV="\${AUDIODEV:-default}"
export SDL_AUDIO_MINIMUM_LATENCY_MS="\${SDL_AUDIO_MINIMUM_LATENCY_MS:-220}"
if command -v churchwars.real >/dev/null 2>&1; then
  exec churchwars.real "\$@"
fi
if [ -n "\${CHURCHWARS_WRAPPED:-}" ]; then
  exec churchwars "\$@"
fi
export CHURCHWARS_WRAPPED=1
exec churchwars "\$@"
