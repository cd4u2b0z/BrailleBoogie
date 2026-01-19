/*
 * Control Bus - Unified audio-driven control signals for ASCII Dancer v2.4
 *
 * Provides normalized 0..1 control signals with configurable attack/release
 * smoothing for consistent, responsive animation control.
 *
 * Signals:
 *   energy   - RMS loudness with attack/release envelope
 *   onset    - Transient strength (envelope derivative)
 *   bass     - Low frequency energy (20-250 Hz)
 *   mid      - Mid frequency energy (250-2000 Hz)
 *   treble   - High frequency energy (2000-20000 Hz)
 *   beat_phase - Normalized position in beat cycle [0..1]
 *   beat_hit   - Impulse at beat start (decays quickly)
 */

#ifndef CONTROL_BUS_H
#define CONTROL_BUS_H

#include <stdbool.h>

/* Smoothing presets */
typedef enum {
    SMOOTH_FAST,      /* Attack: 5ms, Release: 50ms - for dancer */
    SMOOTH_MEDIUM,    /* Attack: 10ms, Release: 100ms - for particles */
    SMOOTH_SLOW,      /* Attack: 20ms, Release: 200ms - for UI */
    SMOOTH_INSTANT    /* No smoothing */
} SmoothingPreset;

/* Individual smoothed value with attack/release */
typedef struct {
    float raw;            /* Unsmoothed input */
    float smoothed;       /* Output after envelope */
    float peak;           /* Recent peak for dynamics */
    float velocity;       /* Rate of change */
    
    /* Envelope parameters */
    float attack_coef;    /* 0-1, higher = faster attack */
    float release_coef;   /* 0-1, higher = faster release */
    float peak_decay;     /* Peak decay rate */
} SmoothedValue;

/* Beat tracking state */
typedef struct {
    float phase;          /* Current beat phase 0..1 */
    float hit;            /* Beat hit impulse (decays quickly) */
    float bpm;            /* Current BPM estimate */
    double last_beat;     /* Timestamp of last beat */
    bool on_beat;         /* True when near beat */
    bool on_half_beat;    /* True when near half-beat */
    int beat_count;       /* Total beats detected */
} BeatState;

/* Control bus - all signals in one place */
typedef struct {
    /* Core frequency bands */
    SmoothedValue energy;
    SmoothedValue bass;
    SmoothedValue mid;
    SmoothedValue treble;
    
    /* Transient detection */
    SmoothedValue onset;
    float prev_energy;    /* For derivative */
    
    /* Derived signals */
    float bass_ratio;     /* Bass relative to total */
    float treble_ratio;   /* Treble relative to total */
    float brightness;     /* Spectral centroid proxy */
    float dynamics;       /* Energy variance */
    
    /* Beat tracking */
    BeatState beat;
    
    /* Silence detection */
    float silence_time;   /* Time since significant audio */
    bool is_silent;       /* True when energy below threshold */
    
    /* Timing */
    double current_time;
    float dt;
    
    /* Energy history for dynamics calculation */
    float energy_history[64];
    int history_idx;
    
    /* Configuration */
    float silence_threshold;
    float onset_sensitivity;
    float beat_hit_decay;
} ControlBus;

/* ============ Creation / Destruction ============ */

ControlBus* control_bus_create(void);
void control_bus_destroy(ControlBus *bus);

/* ============ Core Update ============ */

/* Update control bus with raw audio features
 * bass, mid, treble: 0-1 normalized frequency band energies
 * dt: delta time in seconds
 */
void control_bus_update(ControlBus *bus, 
                        float bass, float mid, float treble,
                        float dt);

/* Update beat tracking with external BPM/phase info */
void control_bus_update_beat(ControlBus *bus, 
                             float beat_phase, float bpm,
                             bool beat_detected);

/* ============ Signal Access (normalized 0-1) ============ */

/* Get smoothed energy (overall loudness) */
float control_get_energy(ControlBus *bus);

/* Get smoothed frequency bands */
float control_get_bass(ControlBus *bus);
float control_get_mid(ControlBus *bus);
float control_get_treble(ControlBus *bus);

/* Get onset strength (transient/attack detection) */
float control_get_onset(ControlBus *bus);

/* Get beat-related signals */
float control_get_beat_phase(ControlBus *bus);   /* 0 = on beat, 0.5 = off beat */
float control_get_beat_hit(ControlBus *bus);     /* Impulse at beat */
float control_get_bpm(ControlBus *bus);
bool control_on_beat(ControlBus *bus);           /* True near beat */
bool control_on_half_beat(ControlBus *bus);      /* True near half-beat */

/* Get derived signals */
float control_get_brightness(ControlBus *bus);   /* Spectral centroid proxy */
float control_get_dynamics(ControlBus *bus);     /* Energy variance */
float control_get_bass_ratio(ControlBus *bus);   /* Bass dominance */
float control_get_treble_ratio(ControlBus *bus); /* Treble dominance */

/* Silence detection */
bool control_is_silent(ControlBus *bus);
float control_get_silence_time(ControlBus *bus);

/* ============ Smoothing Configuration ============ */

/* Set smoothing preset for all signals */
void control_set_smoothing(ControlBus *bus, SmoothingPreset preset);

/* Configure individual signal smoothing (in seconds) */
void control_configure_envelope(SmoothedValue *val, 
                                float attack_time, float release_time,
                                float sample_rate);

/* ============ Utility ============ */

/* Get raw (unsmoothed) values for debugging */
void control_get_raw(ControlBus *bus, 
                     float *energy, float *bass, float *mid, float *treble);

/* Reset all state (for audio restart) */
void control_bus_reset(ControlBus *bus);

#endif /* CONTROL_BUS_H */
