# Changelog

All notable changes to ASCII Dancer will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.0.0] - 2026-01-18

### Added
- **🎭 Braille Skeleton Dancer** - Complete rewrite with procedural animation
  - 36 unique poses across 7 energy categories
  - Physics-based joint animation with spring-damper system
  - Smooth interpolation via easing functions (quadratic, cubic, bounce, elastic)
  - Momentum and follow-through for natural movement

- **🎨 Braille Canvas System** - High-resolution terminal graphics
  - Pixel-to-braille conversion (2×4 subpixel resolution per cell)
  - Drawing primitives: lines, circles, filled circles, arcs
  - Quadratic and cubic Bézier curves for smooth limbs
  - Thick line support for body parts

- **🎵 Advanced Audio Analysis**
  - Beat detection with BPM estimation
  - Style/genre classification (electronic, rock, hip-hop, ambient, classical)
  - Frequency-specific body mapping (bass→legs, mids→torso, treble→arms)

- **🧠 Smart Animation System**
  - Anti-repetition pose history (avoids last 8 poses)
  - Energy-based category selection
  - Transient detection for bass hits and treble accents
  - Per-joint physics with configurable stiffness/damping

- **📊 Pose Categories**
  - IDLE (4 poses) - Subtle breathing, weight shifts
  - CALM (5 poses) - Gentle swaying, light movement
  - GROOVE (8 poses) - Dance moves, rhythmic motion
  - ENERGETIC (7 poses) - Active dancing, jumping
  - INTENSE (6 poses) - Peak energy, wild movement
  - BASS_HIT (4 poses) - Triggered by bass transients
  - TREBLE_ACCENT (4 poses) - Triggered by treble spikes

### Changed
- Build system now supports both frame-based and braille dancers
  - `make` - Build frame-based dancer (original)
  - `make braille` - Build skeleton dancer (new)

### Technical
- Modular architecture with separate braille/ directory
- ncursesw for proper UTF-8/Unicode rendering
- POSIX threads for audio capture
- FFTW3 for FFT analysis (via cavacore)

---

## [1.0.0] - 2026-01-18

### Added
- **Initial release** - Frame-based ASCII dancer
- **Audio capture** - PipeWire (primary) + PulseAudio (fallback)
- **FFT processing** - cavacore library from cava project
- **Frequency mapping** - Bass/mids/treble to body parts
- **ncurses rendering** - ~60fps terminal output
- **Custom frames** - Support for user-defined braille art frames

### Infrastructure
- Makefile with auto-detection of audio backends
- Cross-platform dependency setup (Arch, Ubuntu, Fedora)

---

## Attribution

This project uses code from:

| Component | Source | License |
|-----------|--------|---------|
| Audio capture (PipeWire/Pulse) | [cava](https://github.com/karlstav/cava) | MIT |
| FFT processing (cavacore) | [cava](https://github.com/karlstav/cava) | MIT |

Special thanks to **Karl Stavestrand** and all cava contributors for their excellent work on audio visualization.

---

<sub>Original work by **Dr. Baklava** • [github.com/cd4u2b0z](https://github.com/cd4u2b0z) • 2026</sub>
