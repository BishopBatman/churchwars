#!/usr/bin/env bash
set -euo pipefail

# Ensure inside git repo with 'origin' remote
git rev-parse --is-inside-work-tree >/dev/null 2>&1 || { echo "Not inside a git repository" >&2; exit 1; }
git remote get-url origin >/dev/null 2>&1 || { echo "No 'origin' remote configured" >&2; exit 1; }

mkdir -p docs tools

# docs/asoundrc.example
cat <<'EOF' > docs/asoundrc.example
pcm.dmix48 {
  type dmix
  ipc_key 2048
  slave {
    pcm "hw:0,0"
    rate 48000
    format S16_LE
    channels 2
    period_size 8192
    buffer_size 32768
  }
}
pcm.!default {
  type plug
  slave.pcm "dmix48"
}
ctl.!default {
  type hw
  card 0
}
EOF

# tools/churchwars_audio_wrapper.sh
cat <<'EOF' > tools/churchwars_audio_wrapper.sh
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
EOF
chmod +x tools/churchwars_audio_wrapper.sh

# tools/install_vm_audio_wrapper.sh
cat <<'EOF' > tools/install_vm_audio_wrapper.sh
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
EOF
chmod +x tools/install_vm_audio_wrapper.sh

# Append README section if missing
README_FILE="README.md"
if [ ! -f "$README_FILE" ]; then
  if [ -f "README" ]; then
    README_FILE="README"
  else
    touch "$README_FILE"
  fi
fi
grep -q '^### VM audio (VirtualBox/Linux)$' "$README_FILE" || cat <<'EOF' >> "$README_FILE"

### VM audio (VirtualBox/Linux)

If sound stutters or crashes in a Linux VM:
1) Copy our sample ALSA config:
   cp docs/asoundrc.example ~/.asoundrc
   sudo alsa force-reload || true
2) Install our launcher wrapper (keeps using the 'churchwars' command):
   sudo tools/install_vm_audio_wrapper.sh
3) Run the game:
   churchwars
If heavy scenes still underrun, increase period/buffer in docs/asoundrc.example and re-copy it.
EOF

# Create/switch branch
if git show-ref --verify --quiet refs/heads/vm-audio-stability; then
  git switch vm-audio-stability
else
  git switch -c vm-audio-stability
fi

# Commit and push
git add docs/asoundrc.example tools/churchwars_audio_wrapper.sh tools/install_vm_audio_wrapper.sh "$README_FILE"
if ! git diff --cached --quiet; then
  git commit -m "Add VM-safe audio wrapper + ALSA example"
fi
git push -u origin vm-audio-stability

echo "On your VM: git pull in the repo, then run \`sudo tools/install_vm_audio_wrapper.sh\` and start the game with \`churchwars\`."
echo "To enable the ALSA config: \`cp docs/asoundrc.example ~/.asoundrc && sudo alsa force-reload || true\`."
