# TALK-TRON by Zonium — v0.1.0 experimental beta

Original MIDI-driven synthetic vocal instrument, built with JUCE 8.0.4. VST3 + standalone on Windows, macOS and Linux.

## Build using GitHub Actions

1. Upload the entire contents of this folder to a new GitHub repository, including `.github/workflows/build.yml`.
2. Open **Actions** → **Build TALK-TRON** → **Run workflow**.
3. Download the artifact for your operating system. Copy the `.vst3` bundle into your system VST3 folder and rescan in your DAW.
4. If the CI build reports an error, share the exact log. This beta has not been compiled locally in this environment.

## Use

- Enter English lyrics into the orange LCD text editor. Each incoming MIDI note advances to the next syllable; after the last, it loops.
- Explicitly split words with a hyphen: `I want you to-night`. This beta uses basic heuristics and **does not have a full English phonetic dictionary**.
- `SINGLE SYLLABLE` repeats the syllable selected in the dropdown below the HOLD and SINGLE controls.
- Hold a MIDI note to sustain the voiced part. Release to fade out.
- Five sound modes change the synthesis excitation; all share a basic formant engine.
- The on-screen keyboard sends notes to the same engine. The standalone app may need an audio output selected in its settings.

## Important known limitations

This is an experimental formant synthesizer, not naturalistic text-to-speech. English phoneme conversion is approximate, and consonants and syllable timing require significant refinement. The HOLD SYLLABLE switch is present and saved but its separate behavior is not yet implemented. GLIDE is present and saved but not yet wired to pitch portamento. The UI is programmatically drawn in JUCE to approximate the approved visual concept, not a pixel-perfect image asset. The six controls are visible, but only Character, Breathe, Vibrato, Formant and Output currently influence sound. Voice-mode timbres are exploratory.

## License

Copyright (C) 2026 Zonium. All rights reserved until a license is explicitly chosen. JUCE is subject to its own license terms.
