# TALK-TRON by Zonium — v0.2.0 (experimental)

JUCE 8.0.4 / CMake 3.22 / C++17; VST3 and Standalone for Windows, Linux and macOS.

## Changes from v0.1.0
- Rebuilt, resizable vintage dark-metal editor with a larger amber lyric display and five physical-style voice-mode buttons.
- CMake explicitly enables both C and C++ and generates JuceHeader.h.
- GitHub Actions is **manual only** (`workflow_dispatch`), with no push or pull_request triggers.
- A small built-in English word-to-phoneme dictionary, English vowel/diphthong handling for common words (including `I`), explicit syllable dictionary (`tonight` -> `to`, `night`) and manual hyphen syllable splitting.
- Active syllable label advances per MIDI note-on. Single Syllable keeps the selected syllable. Hold Syllable toggles sustained versus short articulation.
- Glide parameter is now connected to pitch smoothing.
- Note voices retain their token snapshot so text edits do not invalidate notes already playing.

## Important limitations
This is **still a proof-of-concept formant synthesizer**, NOT a production-grade English singing synthesizer. It does not use a full English pronunciation dictionary, trained singing model, phoneme aligner, or recorded voice database. Many words will sound robotic, unclear, or incorrect. Voice modes are experimental timbral variants, not realistic vocoders/talkboxes. Syllabification is limited to listed words and manual hyphens. The UI is custom JUCE drawing rather than a pixel-perfect reproduction of the concept image. A complete pronunciation/singing engine is a separate, significant engineering project.

## Build on GitHub
Upload the repository files, then Actions > Build TALK-TRON > Run workflow. No builds occur on push. The job matrix builds Windows, Ubuntu 24.04 and macOS 15. Builds are not guaranteed to pass without testing in those environments.

## Local build
```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel 2
```

## Usage
Type lyrics in the amber editor. Each MIDI note-on advances one token/syllable. To force exact boundaries, type e.g. `I want you to-night`. Use the syllable dropdown and Single Syllable to repeat a chosen syllable. Click the mode buttons to change timbre. Keyboard responds to mouse and MIDI input.

Copyright (C) 2026 Zoniumgames. JUCE is fetched from its upstream repository and is subject to its own license.
