**English** | [Русский](ARCHITECTURE.ru.md)

# 󰙵 Architecture

Technical architecture documentation for ASCII Dancer v3.2.

---

## 󰈏 Overview

```
┌──────────────────────────────────────────────────────────────────┐
│                        ASCII Dancer v3.2                          │
├──────────────────────────────────────────────────────────────────┤
│  Audio Layer      FFT Layer      Control Bus      Animation      │
│  ┌──────────┐    ┌──────────┐    ┌──────────┐    ┌──────────┐   │
│  │ PipeWire │───>│ cavacore │───>│ Control  │───>│ Skeleton │   │
│  │  Pulse   │    │   FFT    │    │   Bus    │    │  Dancer  │   │
│  └──────────┘    └──────────┘    └──────────┘    └──────────┘   │
│                                         │               │         │
│                                         ├──────────>────┤         │
│                                         │               │         │
│                              ┌──────────┴────┐    ┌────┴──────┐  │
│                              │  Particles    │    │    UI     │  │
│                              │  Trails       │    │ Reactive  │  │
│                              │  Background   │    │ Profiler  │  │
│                              └───────────────┘    └───────────┘  │
│                                         │               │         │
│                                         └───────┬───────┘         │
│                                                 │                 │
│                                          ┌──────┴──────┐          │
│                                          │   Braille   │          │
│                                          │   Canvas    │          │
│                                          └─────────────┘          │
└──────────────────────────────────────────────────────────────────┘
```

**v3.2 Architecture:**
Audio → FFT → **Control Bus** → {Skeleton, Particles, Background FX, UI}
- **Control Bus**: Unified signals with envelope smoothing
- **Genre Detection**: Automatic style classification for easter egg poses
- **228 Poses**: 13 categories including 6 genre-specific dance styles
- **Configurable smoothing**: Fast for dancer, medium for particles, slow for UI

**v3.2.4 Code Quality:**
- **cppcheck clean**: Zero critical/high static analysis issues
- **const correctness**: Read-only pointers properly marked `const`
- **static linkage**: Internal functions scoped to translation units
- **NULL safety**: All malloc() returns checked before use

---

## 󰉋 Directory Structure

```
src/
├── 󰈮 main.c                 # Entry point, main loop
├── 󰈮 constants.h            # v3.2: Centralized magic numbers (~150 params)
│
├── 󰎈 audio/                 # Audio capture
│   ├── pipewire.c          # PipeWire stream (Linux)
│   ├── pulse.c             # PulseAudio capture (Linux)
│   ├── coreaudio.c         # CoreAudio capture (macOS)
│   ├── rhythm.c            # Beat detection (v2.3)
│   ├── bpm_tracker.c       # v3.0: Advanced BPM with confidence/stability
│   ├── energy_analyzer.c   # v3.0: RMS energy, intensity zones
│   ├── audio_picker.c      # v3.0+: Interactive source selection
│   └── common.c            # Shared utilities
│
├── 󰓾 fft/                   # Frequency analysis
│   └── cavacore.c          # FFT (from cava)
│
├── 󱓻 control/               # v2.4: Control bus
│   └── control_bus.c       # Unified audio signals
│
├── 󰕮 braille/               # Rendering
│   ├── braille_canvas.c    # Pixel→braille + scanline flood fill
│   ├── braille_dancer.c    # Interface
│   └── skeleton_dancer.c   # Physics/poses/constraints (228 poses)
│
├── 󱐋 effects/               # Visual effects
│   ├── particles.c         # Particle system + repulsion
│   ├── trails.c            # Motion trails
│   ├── background_fx.c     # v3.0: 7 background effect modes
│   └── effects.c           # Manager
│
├── 󰎁 export/                # v3.0+: Recording/export
│   └── frame_recorder.c    # Frame capture for GIF/video
│
├── 󰍹 render/                # Terminal output
│   ├── render_new.c        # ncurses
│   └── colors.c            # 13 themes (256-color)
│
├── 󰌌 ui/                    # v2.4+: Reactive UI & Help
│   ├── ui_reactive.c       # Border pulse, energy meter, beat indicator
│   ├── help_overlay.c      # Interactive help (? or F1)
│   ├── profiler.c          # v3.0+: Performance profiler (thread-safe)
│   └── term_caps.c         # v3.0+: Terminal capabilities
│
├── 󰚹 dancer/                # Dancer API
│   ├── dancer.h            # Header
│   └── legacy/             # v3.2: Archived legacy code
│
└── 󰒓 config/                # Configuration
    └── config.c            # INI parser
```

---

## 󰔏 Audio Pipeline

**Thread Model:**
```
┌─────────────────┐     ┌─────────────────┐
│   Main Thread   │     │  Audio Thread   │
│  - FFT process  │◄────│  - PW/PA/CA     │
│  - Animation    │     │  - Sample write │
│  - Rendering    │     └─────────────────┘
└─────────────────┘
```

**Audio Backends:**
- **Linux:** PipeWire (preferred) or PulseAudio
- **macOS:** CoreAudio (AudioQueue API)

**Flow:** Audio → Ring Buffer → FFT → 256 bins → bass/mid/treble

---

## 󰚹 Skeleton System

**15 Joints:**
```
          HEAD
           │
    SHOULDER─┼─SHOULDER
        │  SPINE  │
      ELBOW   │   ELBOW
        │     │     │
      HAND  HIP   HAND
          ┌─┼─┐
        HIP   HIP
         │     │
       KNEE   KNEE
         │     │
       FOOT   FOOT
```

**Physics:** Spring-damper per joint
- Stiffness: 0.1-0.5
- Damping: 0.7-0.9

**228 Poses** across 13 categories:
| Category | Energy Range | Count |
|----------|--------------|-------|
| IDLE | 0.00-0.15 | 4 |
| CALM | 0.15-0.35 | 5 |
| GROOVE | 0.35-0.55 | 8 |
| ENERGETIC | 0.55-0.75 | 7 |
| INTENSE | 0.75-1.00 | 6 |
| BASS_HIT | Transient | 4 |
| TREBLE_ACCENT | Transient | 4 |
| MOONWALK | Hip-hop/Pop | 4 |
| BALLET | Classical | 5 |
| BREAKDANCE | Hip-hop | 4 |
| WALTZ | Classical | 4 |
| ROBOT | Electronic | 5 |
| HEADBANG | Rock | 4 |

**Genre Detection (v3.2):**
- Electronic: High treble ratio, fast BPM → Robot poses
- Hip-Hop: Strong bass, mid-tempo → Moonwalk, Breakdance
- Rock: High energy, guitar range → Headbang
- Classical: Low energy, balanced → Ballet, Waltz
- Pop: Balanced, steady beat → Moonwalk
- Easter eggs trigger ~15% of the time when genre detected

---

## 󰕮 Braille Canvas

**Resolution:** 50×52 pixels → 25×13 terminal cells

**Encoding:** Unicode U+2800-U+28FF (8 dots per char)
```
┌─┬─┐
│1│4│  Bits: 0x01, 0x08
├─┼─┤
│2│5│  Bits: 0x02, 0x10
├─┼─┤
│3│6│  Bits: 0x04, 0x20
├─┼─┤
│7│8│  Bits: 0x40, 0x80
└─┴─┘
```

**Drawing Primitives:**
- `braille_draw_line()` — Bresenham's algorithm
- `braille_draw_circle()` — Midpoint circle
- `braille_draw_bezier_quad()` — Quadratic curves
- `braille_draw_bezier_cubic()` — Cubic curves
- `braille_flood_fill()` — Scanline algorithm (v3.2: bounded O(4096) queue)

**Flood Fill (v3.2):**
```c
// Scanline flood fill with bounded memory
#define FLOOD_FILL_MAX_QUEUE 4096

typedef struct {
    int x, y;
} FillPoint;

// Uses horizontal scanlines instead of recursive per-pixel
// Prevents stack overflow on large fills
```

---

## 󱐋 Effects System (v3.0)

### 󰸞 Particles
- Pool of 256 particles (no allocations)
- Physics: velocity, gravity, drag
- Spawn patterns: burst, fountain, explosion, sparkle
- Fade based on lifetime

```c
// Particle update
p->vx += p->ax * dt;
p->vy += p->ay * dt;
p->x += p->vx * dt;
p->y += p->vy * dt;
p->brightness = p->lifetime / p->max_life;
```

### 󰘵 Motion Trails
- Ring buffer per joint (8 positions)
- Velocity-based recording
- Alpha fade over time

### 󱐋 Background Effects (v3.0)
7 spectacular effect modes:
| Mode | Description |
|------|-------------|
| Ambient Field | Floating twinkling particles |
| Spectral Waves | Frequency-reactive pulses |
| Energy Aura | Pulsing ring around dancer |
| Beat Burst | Synchronized explosions |
| Frequency Ribbons | Vertical frequency bars |
| Particle Rain | Falling particles from top |
| Spiral Vortex | Rotating spiral arms |

### 󰓾 Coordinate Transform
```c
// Joint (0-1) → Pixel coordinates
px = (joint.x - 0.5) * (width * 0.8) + (width / 2);
py = joint.y * (height * 0.8) + (height * 0.1);
```

---

## 󰓾 Render Pipeline

```
1. Audio capture (thread)
        ↓
2. FFT → frequency bins
        ↓
3. bass/mid/treble calculation
        ↓
4. Skeleton physics update
        ↓
5. Effects triggers (bass→particles, etc.)
        ↓
6. Canvas clear
        ↓
7. Render trails → canvas
        ↓
8. Render skeleton → canvas
        ↓
9. Render particles → canvas
        ↓
10. braille_canvas_render() → convert pixels to cells
        ↓
11. braille_canvas_to_utf8() → UTF-8 string
        ↓
12. ncurses mvprintw() → terminal
```

---

## 󰾆 Performance

- **60 FPS** target (~16.6ms per frame)
- **Particle pooling** — fixed array, no malloc
- **Dirty cell tracking** — skip unchanged cells
- **8× compression** — braille packs 8 pixels/char
- **Single-threaded render** — simplicity over parallelism

---

## 󰒓 Configuration

**File:** `~/.config/braille-boogie/config.ini`

```ini
[audio]
source = auto
backend = pipewire
sensitivity = 1.0

[visual]
theme = matrix
ground = true
shadow = true
particles = true
trails = true

[animation]
fps = 60
```

---

## � Control Bus (v2.4)

**Unified Signal Architecture:**

The control bus separates audio analysis from animation, providing normalized 0-1 signals with configurable attack/release smoothing.

```c
typedef struct {
    SmoothedValue energy;    // RMS loudness
    SmoothedValue bass;      // 20-300 Hz
    SmoothedValue mid;       // 300-2000 Hz
    SmoothedValue treble;    // 2000+ Hz
    SmoothedValue onset;     // Transient detection
    BeatState beat;          // Phase, hit, BPM
} ControlBus;
```

**Attack/Release Envelope:**
```c
// Fast attack, slow release for natural dynamics
coef = input > smoothed ? attack_coef : release_coef;
smoothed += coef * (input - smoothed);
```

**Smoothing Presets:**
| Preset | Attack | Release | Use Case |
|--------|--------|---------|----------|
| FAST | 5ms | 50ms | Dancer animation |
| MEDIUM | 10ms | 100ms | Particle emission |
| SLOW | 20ms | 200ms | UI display |
| INSTANT | 0ms | 0ms | Debug/raw values |

**Derived Signals:**
- **brightness** = (mid × 0.5 + treble) / total
- **dynamics** = √variance(energy history)
- **bass_ratio** = bass / total
- **treble_ratio** = treble / total

---

## 󰘦 Key Algorithms

### Beat Detection
```c
if (bass > threshold && bass_velocity > min_velocity) {
    trigger_bass_hit();
}
```

### Knee Constraint (v2.4)
```c
// Prevent knock-kneed look
float cx = hip_center.x;
bool left_planted = (beat_phase < 0.5f);

if (knee_left.x > cx - knee_offset && left_planted) {
    knee_left.x = cx - knee_offset;  // Force outward
    velocity.x *= -0.3f;             // Bounce back
}
```

### Pose Selection
```c
// Avoid recent poses, select from energy category
Category cat = energy_to_category(energy);
do {
    pose = random_from_category(cat);
} while (in_recent_history(pose));
```

### Spring-Damper Physics
```c
force = (target - position) * stiffness;
velocity += force;
velocity *= damping;
position += velocity * dt;
```

### Particle Repulsion (v2.4)
```c
// Push particles away from dancer center
dx = particle.x - body_center.x;
dy = particle.y - body_center.y;
dist = sqrt(dx² + dy²);

if (dist < body_radius) {
    push_factor = (body_radius - dist) / body_radius;
    particle.vx += (dx / dist) * repulsion * push_factor;
    particle.vy += (dy / dist) * repulsion * push_factor;
}
```

### Genre Detection (v3.2)
```c
// Classify genre from audio features
GenreStyle detect_genre(float bass_ratio, float treble_ratio, 
                        float energy, float bpm) {
    if (treble_ratio > 0.4f && bpm > 120)
        return GENRE_ELECTRONIC;
    if (bass_ratio > 0.45f && bpm >= 80 && bpm <= 115)
        return GENRE_HIPHOP;
    if (energy > 0.6f && treble_ratio > 0.3f)
        return GENRE_ROCK;
    if (energy < 0.3f)
        return GENRE_CLASSICAL;
    return GENRE_POP;
}
```

### Easter Egg Poses (v3.2)
```c
// ~15% chance to trigger genre-specific moves
if (genre != GENRE_UNKNOWN && (rand() % 100) < 15) {
    switch (genre) {
        case GENRE_ELECTRONIC: return random_robot_pose();
        case GENRE_HIPHOP:     return random_moonwalk_or_breakdance();
        case GENRE_ROCK:       return random_headbang_pose();
        case GENRE_CLASSICAL:  return random_ballet_or_waltz();
        // ...
    }
}
```

---

## 󰈮 Constants Header (v3.2)

Centralized tuning parameters in `constants.h`:

```c
// BPM tracking
#define BPM_MIN                 40
#define BPM_MAX                 240
#define BPM_LOCK_CONFIDENCE     0.7f
#define BPM_LOCK_STABILITY      0.85f

// Energy thresholds
#define ENERGY_SILENT           0.02f
#define ENERGY_LOW              0.15f
#define ENERGY_MEDIUM           0.35f
#define ENERGY_HIGH             0.55f
#define ENERGY_PEAK             0.75f

// Animation timing
#define POSE_TRANSITION_FAST    0.15f
#define POSE_TRANSITION_NORMAL  0.25f
#define POSE_TRANSITION_SLOW    0.4f

// Physics
#define JOINT_STIFFNESS         0.15f
#define JOINT_DAMPING           0.85f
#define BREATHING_AMPLITUDE     0.02f
```

---

## 󰾆 Performance Profiler (v3.0+)

Thread-safe timing with atomic operations:

```c
// Thread-safe frame timing
static _Atomic double frame_start_time = 0.0;

void profiler_frame_start(void) {
    atomic_store(&frame_start_time, get_time_ms());
}

double profiler_get_frame_time(void) {
    return get_time_ms() - atomic_load(&frame_start_time);
}
```

**Metrics tracked:**
- Current/avg/min/max FPS
- Audio processing time
- Update cycle time
- Render cycle time
- Particle count
- Trail count

---

## 󰘸 Scanline Flood Fill (v3.2)

Memory-bounded flood fill algorithm:

```c
#define FLOOD_FILL_MAX_QUEUE 4096

// Instead of recursive per-pixel fill:
// 1. Push seed point to queue
// 2. For each point, scan left/right to find span
// 3. Fill entire span
// 4. Queue points above/below span
// 5. Repeat until queue empty or limit reached

// Advantages:
// - O(4096) worst case vs O(width*height)
// - No stack overflow on large fills
// - Predictable memory usage
```

