# OXXY — Minimal, Auditable Linux Native Music Player

OXXY is a lightweight, high‑fidelity music player prototype for Linux (targeted at Arch Linux). It is written primarily in C with a minimal, optional C++ UI front-end — designed for auditability, low latency, and total control. OXXY is not an Electron/GTK/Qt app: it’s a native, portable player with a modern GPU‑accelerated UI option and a small dependency surface.

This repository contains a working prototype and reference implementation demonstrating the architecture, audio pipeline, lock‑free audio buffering, metadata parsing, playlist management, profile storage (XDG), and a GPU UI prototype.

Quick highlights

- Native audio backends: runtime preference for PipeWire with ALSA fallback (ALSA optional at build time).
- Fast, lock‑free audio pipeline (C11 atomics) with producer/consumer ring buffer.
- Minimal, auditable pure‑C ID3v2 and Vorbis comment parsers.
- Playlist support (.m3u) with shuffle/repeat and JSON cache option.
- GPU accelerated UI prototype (OpenGL/GLFW) with waveform visualization, album art crossfade prototype, and neon/cyberpunk accents.
- Profiles and XDG‑compliant configuration/cache layout.
- Optional integrations (VK sharing stub, MPRIS plan, Last.fm scrobbling via TLS libs).

NOTE: This is a prototype and a developer reference. It prioritizes clarity, auditability and low dependency surface. Some modules are stubs (decoder adapters, full PipeWire backend, complete VK sharing) and intended to be completed as part of the project roadmap.

Table of contents

- Features
- Architecture overview
- Build & Run (Arch Linux)
- Development notes & tests
- Configuration & XDG paths
- Roadmap
- Contributing
- License

Features

- Audio: low‑latency playback architecture with ring buffer and playback thread.
- Formats: designed to support MP3, FLAC, OGG, WAV, Opus via pluggable decoders (libavcodec or dr_* single files).
- Metadata: pure‑C parsers for ID3v2 and Vorbis comments; safe, bounded parsing to avoid crashes or overflows.
- Playlist: load/save .m3u, in‑memory playlist with shuffle and repeat, and JSON cache for quick library scans.
- UI: GPU‑accelerated prototype (OpenGL/GLFW) with waveform visualization and theme support; planned ImGui frontend integration.
- Profiles: save/load named profiles in `$XDG_CONFIG_HOME/oxxy/*.json`.
- Integration: design includes MPRIS/DBus hooks and optional Last.fm/VK integrations via minimal TLS/HTTP stacks.

Architecture overview (short)

OXXY is modular, with clear responsibilities:

- core/: bootstrap, config, logging
- audio/: backend abstraction (PipeWire/ALSA), playback thread, lock‑free PCM ring
- decode/: decoder adapters (libavcodec/dr_*)
- metadata/: ID3v2 and Vorbis comment parsers (pure C)
- ui/: OpenGL/GLFW prototype (or ImGui frontend)
- playlist/: in‑memory playlist, .m3u load/save, shuffle/repeat
- profiles/: XDG profile storage (JSON)
- ipc/: optional MPRIS/DBus and media key handling

Build & Run (Arch Linux)

Prerequisites

Install basic dependencies (system):

```sh
sudo pacman -Syu
sudo pacman -S --needed base-devel gcc g++ pkgconf
sudo pacman -S --needed mesa libglvnd libx11 libxrandr libxi libxinerama libxcursor libxss
sudo pacman -S --needed glfw-x11
# Optional for ALSA backend
sudo pacman -S --needed alsa-lib
# Optional for VK/HTTP integration
sudo pacman -S --needed curl
```

Build

```sh
cd /path/to/player_music
make            # builds the core test binary (bin/oxxy-test)
```

Build UI (if OpenGL + GLFW installed)

```sh
make ui         # builds bin/oxxy-ui (GLFW + libGL required)
```

Run

```sh
# test audio pipeline (sine generator -> backend) for debugging
./bin/oxxy-test

# run GPU UI (if built)
./bin/oxxy-ui
```

Debug / verbose build

```sh
make build_verbose   # saves build.log
make run_all         # runs unit tests and the core test, saves run.log
```

Development notes & tests

- Unit tests: `make test` runs small tests for metadata and playlist modules.
- Sanitizers: during development, compile with -fsanitize=address,undefined to catch UB.
- Static analysis: use clang-tidy or cppcheck on modified files.

Configuration & XDG paths

OXXY follows XDG conventions:

- Config: `$XDG_CONFIG_HOME/oxxy/` (defaults to `~/.config/oxxy`)
- Cache: `$XDG_CACHE_HOME/oxxy/` (defaults to `~/.cache/oxxy`)
- Profiles are saved as JSON files in `$XDG_CONFIG_HOME/oxxy/` (e.g. `default.json`).

Roadmap (high priority)

1. Integrate a full Dear ImGui frontend (vendorized) with album art textures and smoother animations.
2. Implement decoder adapters: libavcodec adapter for maximum format coverage and optional dr_* single header decoders for a minimal build.
3. Complete PipeWire backend using libpipewire for professional low‑latency output.
4. Implement MPRIS and media‑key support via DBus.
5. Implement secure VK & Last.fm integrations with proper token storage and minimal TLS (mbedTLS/BearSSL or libcurl+system TLS).
6. Add unit tests for parsers and playlist logic; add CI config targeting Arch Linux container.

Contributing

Contributions are welcome. Please open issues for bugs or feature requests and submit small, focused pull requests. Follow these guidelines:

- Keep changes small and auditable. Prefer pure C fixes for core modules.
- Add unit tests for parser/playlist changes.
- Document new public APIs in headers and update README where needed.

Security & philosophy

OXXY is intentionally minimal. It aims for:

- No garbage‑collected runtimes (no Node/Electron/Python/Java).
- Minimal third‑party dependencies — prefer small, auditable single‑file libraries where appropriate.
- No use of system() or popen() in the final product.
- Defensive parsing: tag parsers enforce size limits and avoid unbounded allocations.

License

This prototype is available under the MIT License. See `LICENSE` for details.

Contact / Maintainers

Create issues and pull requests on GitHub. For rapid design discussion, include design notes and small reproducer code.

Enjoy — and let’s make OXXY a fast, auditable native music player for Linux.
