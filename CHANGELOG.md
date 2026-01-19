# Changelog

All notable changes to ASCII Dancer will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.1.0] - 2026-01-19

## [2.2.0] - 2026-01-18

### Added
- **🎆 Particle System** - Dynamic visual effects
  - Spark particles on bass hits
  - Physics simulation (velocity, gravity, drag)
  - Lifetime with fade out
  - Multiple spawn patterns (burst, fountain, explosion, sparkle)
  - Toggle with `p` key

- **👻 Motion Trails** - Ghost effect on movement
  - Stores history of joint positions
  - Renders trailing ghost limbs with fading opacity
  - Velocity-based trail intensity
  - Toggle with `m` key

- **✨ Visual Enhancements**
  - Breathing animation (subtle idle motion)
  - Floor vibration on heavy bass
  - Screen shake on intense bass hits
  - Glow effect on high energy
  - Toggle breathing with `b` key

### Technical
- New directory: `src/effects/`
- New files: `particles.h/c`, `trails.h/c`, `effects.h/c`
- Updated: `braille_dancer.c`, `dancer.h`, `main.c`
- Effects automatically trigger from audio analysis

---

### Added
- **🎨 256-Color Theme System** - Rich visual customization
  - 7 built-in themes: Default, Fire, Ice, Neon, Matrix, Synthwave, Mono
  - 10-step color gradients based on energy level
  - Theme cycling with `t` key during playback
  - `--theme <name>` CLI option

- **⚙️ Configuration System** - Persistent settings via INI files
  - Auto-loads from `~/.config/asciidancer/config.ini`
  - Sections: [audio], [visual], [terminal], [animation], [debug]
  - `--config <file>` CLI option for custom config paths
  - `config_create_default()` generates sample config

- **🌍 Ground Line & Shadow** - Enhanced visual depth
  - Horizontal ground line at dancer's feet
  - Shadow/reflection effect (inverted, faded dancer below ground)
  - Toggle ground with `g` key, shadow with `r` key
  - `--no-ground` and `--no-shadow` CLI options

- **📐 Adaptive Terminal Scaling** - Dynamic resize handling
  - SIGWINCH handler for terminal resize detection
  - Automatic canvas rescaling to fit new dimensions
  - Maintains aspect ratio during resize

### Changed
- Render system refactored for 256-color support
- Main loop updated with config integration
- Help output now shows theme list with emoji indicators

### Technical
- New files: `src/config/config.h`, `src/config/config.c`
- New files: `src/render/colors.h`, `src/render/colors.c`
- Updated: `src/render/render_new.c`, `src/render/render.h`, `src/main.c`
- Uses xterm 256-color palette (color cube + grayscale ramp)

---

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

## [2.0.0] - 2026-01-18

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

## [2.0.0] - 2026-01-18

## Attribution

This project uses code from:

| Component | Source | License |
|-----------|--------|---------|
| Audio capture (PipeWire/Pulse) | [cava](https://github.com/karlstav/cava) | MIT |
| FFT processing (cavacore) | [cava](https://github.com/karlstav/cava) | MIT |

Special thanks to **Karl Stavestrand** and all cava contributors for their excellent work on audio visualization.

---

## [2.0.0] - 2026-01-18

<sub>Original work by **Dr. Baklava** • [github.com/cd4u2b0z](https://github.com/cd4u2b0z) • 2026</sub>
