# TimeFreeze VST3

Original JUCE/C++ audio effect inspired by the user's visual reference. This is not the original commercial plugin and does not copy its branding or source.

## Build automatically
Every push to `main` and manual workflow dispatch runs **Build TimeFreeze VST3** on a Windows GitHub runner. On success, download the `TimeFreeze-VST3-Windows` artifact from the workflow run page.

## Controls planned
Freeze, bypass, limiter, input/output gain, overlap, grain size, speed, spectral mix, smear, volume, pitch, reducer, mode selection, preset/state recall and a custom GUI.

## Status
The GitHub Actions workflow and build configuration are being established. The source files must be present under `Source/` for the build to succeed. A successful CI build is not yet verified.
