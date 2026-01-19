# 󰌷 Contributing to Braille Boogie

Thanks for your interest in contributing! This document will help you get started.

---

## 󰏗 Quick Start

```bash
# Clone the repository
git clone https://github.com/cd4u2b0z/braille-boogie.git
cd braille-boogie

# Install dependencies (Arch Linux)
sudo pacman -S pipewire libpulse fftw ncurses

# Build
make braille

# Run with all effects
./braille-boogie --demo
```

### Dependencies

| Platform | Packages |
|----------|----------|
| Arch | `pipewire libpulse fftw ncurses` |
| Ubuntu/Debian | `libpipewire-0.3-dev libpulse-dev libfftw3-dev libncursesw5-dev` |
| Fedora | `pipewire-devel pulseaudio-libs-devel fftw-devel ncurses-devel` |
| macOS | `brew install fftw ncurses` (uses CoreAudio) |

---

## 󰈙 Code Style

### C Conventions

- **Indentation:** 4 spaces (no tabs)
- **Braces:** K&R style (opening brace on same line)
- **Naming:** `snake_case` for functions/variables, `UPPER_CASE` for constants
- **Line length:** 100 characters soft limit
- **Comments:** `/* Block comments */` for documentation, `// Line comments` inline

```c
/* Good example */
void particles_update(ParticleSystem *ps, float dt) {
    if (!ps || !ps->enabled) {
        return;
    }
    
    for (int i = 0; i < ps->count; i++) {
        ps->particles[i].y += ps->particles[i].vy * dt;  // Apply gravity
    }
}
```

### Header Organization

```c
/* file.h - Brief description */
#ifndef FILE_H
#define FILE_H

#include <standard_headers.h>
#include "project_headers.h"

/* Type definitions */
typedef struct { ... } MyStruct;

/* Public API */
MyStruct *mystruct_create(void);
void mystruct_destroy(MyStruct *s);
void mystruct_update(MyStruct *s, float dt);

#endif /* FILE_H */
```

---

## 󰉋 Project Structure

```
braille-boogie/
├── src/
│   ├── main.c              # Entry point, main loop
│   ├── constants.h         # Magic numbers, centralized
│   ├── audio/              # Audio capture & analysis
│   ├── braille/            # Braille rendering & dancer
│   ├── effects/            # Particles, trails, backgrounds
│   ├── render/             # ncurses output, colors
│   ├── control/            # Control bus signal routing
│   ├── config/             # INI parser, settings
│   ├── ui/                 # Overlays, profiler
│   └── export/             # Frame recording
├── assets/                 # Demo GIFs, docs
├── docs/                   # Feature documentation
└── Makefile
```

### Key Modules

| Module | Purpose |
|--------|---------|
| `braille_canvas.c` | Pixel-to-braille rendering, scanline flood fill |
| `skeleton_dancer.c` | Joint system, physics, poses |
| `control_bus.c` | Unified signal routing (energy, beat, BPM) |
| `cavacore.c` | FFT analysis (from cava project) |

---

## 󰎈 Building

```bash
# Standard build (braille skeleton dancer)
make braille

# Clean build
make clean && make braille

# Debug build with extra checks
make braille CFLAGS="-Wall -Wextra -g -O0 -fsanitize=address"

# Check code with cppcheck
cppcheck --enable=all --suppress=missingIncludeSystem src/
```

### Make Targets

| Target | Description |
|--------|-------------|
| `make braille` | Build braille skeleton dancer (recommended) |
| `make` | Build frame-based dancer (legacy) |
| `make clean` | Remove object files and binary |
| `make info` | Show build configuration |
| `make gif` | Generate demo GIF (requires vhs) |

---

## 󰐕 Adding Features

### New Effect

1. Create `src/effects/myeffect.c` and `.h`
2. Follow the create/destroy/update pattern:
   ```c
   MyEffect *myeffect_create(int width, int height);
   void myeffect_destroy(MyEffect *e);
   void myeffect_update(MyEffect *e, float dt);
   void myeffect_render(MyEffect *e, BrailleCanvas *canvas);
   ```
3. Add to `Makefile` under `V30P_SRCS`
4. Integrate in `main.c` main loop
5. Add keyboard toggle (check for unused keys)

### New Pose Category

1. Edit `src/braille/skeleton_dancer.c`
2. Add poses to appropriate array (`base_poses`, `genre_poses`, etc.)
3. Follow the joint index conventions in `constants.h`
4. Test with multiple songs/genres

### New Audio Backend

1. Create `src/audio/mybackend.c`
2. Implement the `input_*` thread function pattern
3. Add `#ifdef MYBACKEND` guards
4. Update Makefile detection

---

## 󰃢 Pull Request Guidelines

### Before Submitting

- [ ] Code compiles with zero warnings (`-Wall -Wextra`)
- [ ] Run `cppcheck --enable=all src/` - no new issues
- [ ] Test with actual audio playback
- [ ] Test terminal resize (drag window while running)
- [ ] Update documentation if adding features

### PR Title Format

```
type: brief description

Examples:
feat: add --minimal flag for clean skeleton view
fix: prevent crash on empty audio buffer
refactor: consolidate effect toggle functions
docs: update README with new themes
```

### Commit Messages

- Keep first line under 72 characters
- Use imperative mood ("Add feature" not "Added feature")
- Reference issues: `Fixes #123`

---

## 󰆧 Testing

Currently tested manually. Key test cases:

1. **Start/stop:** `./braille-boogie` - should start without errors
2. **Demo mode:** `./braille-boogie --demo` - all effects visible
3. **No audio:** Works gracefully when no audio playing
4. **Resize:** Drag terminal window - should adapt
5. **Theme cycle:** Press `t` repeatedly - no flicker
6. **Effect toggles:** `p`, `m`, `b`, `f`, `e` - each toggles correctly

---

## 󰘥 Getting Help

- Open an issue for bugs or feature requests
- Check existing issues before creating new ones
- Include terminal info (`echo $TERM`) and OS in bug reports

---

## 󰅂 License

Contributions are licensed under MIT. By contributing, you agree to this license.

---

*Happy dancing! 󰝚*
