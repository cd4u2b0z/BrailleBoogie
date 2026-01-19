# ASCII Dancer v3.0+ Feature Guide

## v3.0 Core Features

### Advanced BPM Tracking
- Multi-tap tempo averaging with histogram clustering
- Confidence scoring (0-100%) shown in status bar
- Stability tracking with tempo locking
- Half/double time detection (40-240 BPM range)

### Dynamic Energy Analysis
- Real-time RMS energy calculation
- 5 intensity zones displayed in status:
  - **Silent** - No significant audio
  - **Low** - Ambient/quiet sections
  - **Medium** - Normal intensity
  - **High** - Energetic sections
  - **Peak** - Maximum intensity
- Spectral features: centroid, spread, rolloff
- Adaptive thresholds that learn from your music

### Background Effects
7 spectacular audio-reactive particle effects:

1. **None** - Effects disabled
2. **Ambient Field** - Floating particles with subtle movement
3. **Spectral Waves** - Waves responding to frequency spectrum
4. **Energy Aura** - Pulsing aura around the dancer
5. **Beat Burst** - Explosive bursts on beat detection
6. **Frequency Ribbons** - Flowing ribbons mapped to frequencies
7. **Particle Rain** - Rain effect intensity tied to energy
8. **Spiral Vortex** - Spinning vortex with audio modulation

## v3.0+ Professional Features

### Frame Recording (Export Mode)
Record terminal frames for creating GIFs or videos:

- Press **X** to start/stop recording
- Frames saved to timestamped directories
- Export as ANSI text files for post-processing
- Compatible with asciinema, agg, or vhs tools

**Workflow:**
```bash
# Start braille-boogie, press X to record
./braille-boogie

# Find recordings in ~/braille-boogie_recordings/
# Convert with your preferred tool
```

### Performance Profiler
Press **I** to toggle real-time performance overlay:

- Current FPS and average
- Min/Max FPS tracking
- Component breakdown:
  - Audio processing time
  - Update cycle time
  - Render time
- Active particle count
- Visual performance bar (green/yellow/red)

### Audio Source Picker
Launch with `--pick-source` to select audio input:

```bash
./braille-boogie --pick-source
```

Interactively select from available PulseAudio/PipeWire sources.

### Terminal Capabilities Detection
Check what your terminal supports:

```bash
./braille-boogie --show-caps
```

Detects:
- Sixel graphics support
- Kitty graphics protocol
- iTerm2 inline images
- True color (24-bit) support

## Keyboard Controls

### v3.0 Controls
| Key | Action |
|-----|--------|
| F | Toggle background effects |
| E | Cycle through effect types |

### v3.0+ Controls
| Key | Action |
|-----|--------|
| X | Toggle frame recording |
| I | Toggle performance profiler |

### Classic Controls
| Key | Action |
|-----|--------|
| ?, F1 | Toggle help overlay |
| q, ESC | Quit |
| +/- | Adjust sensitivity |
| t | Cycle themes |
| g | Toggle ground line |
| r | Toggle reflection/shadow |
| p | Toggle particles |
| m | Toggle motion trails |
| b | Toggle breathing animation |

## Command Line Options

```
--pick-source     Show interactive audio source picker
--show-caps       Display terminal capabilities and exit
--no-ground       Disable ground line
--no-shadow       Disable reflection/shadow
-s, --source      Specify audio source name
-f, --fps         Target framerate (1-120)
-t, --theme       Color theme name
```

## Status Bar Format

```
120bpm(85%) High synthwave [G][R][P][M][B][FX][REC] p:42
│     │     │    │         │                │      │
│     │     │    │         │                │      └─ Particle count
│     │     │    │         │                └─ Recording active
│     │     │    │         └─ Enabled features
│     │     │    └─ Current theme
│     │     └─ Energy zone (Silent/Low/Medium/High/Peak)
│     └─ BPM confidence percentage
└─ Detected BPM
```

## Architecture

### v3.0 Module Stack
```
┌─────────────────────────────────────────┐
│              Main Loop                  │
├─────────────────────────────────────────┤
│  BPM Tracker │ Energy Analyzer │ Effects│
├──────────────┴──────────────────────────┤
│           Rhythm Detection              │
├─────────────────────────────────────────┤
│        FFT Processing (cavacore)        │
├─────────────────────────────────────────┤
│      Audio Capture (PW/PA)              │
└─────────────────────────────────────────┘
```

### v3.0+ Additions
```
┌──────────────────┬──────────────────┐
│  Frame Recorder  │    Profiler      │
├──────────────────┼──────────────────┤
│  Audio Picker    │  Terminal Caps   │
└──────────────────┴──────────────────┘
```

## Building

```bash
# Build with all v3.0+ features
make braille

# Install
make install
```

## Technical Notes

- BPM tracker uses 40-tap history with histogram binning
- Energy analyzer maintains 6 frequency bands
- Background effects share the main 256-particle system
- Profiler uses 120-frame rolling average (2 seconds at 60fps)
- Frame recorder outputs ANSI-colored text compatible with standard tools
