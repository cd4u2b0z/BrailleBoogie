/*
 * Control Bus Implementation - ASCII Dancer v2.4
 *
 * Attack/release envelope follower with derived signals for
 * unified audio-driven animation control.
 */

#include "control_bus.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ============ Internal Helpers ============ */

/* Calculate envelope coefficient from time constant */
static float time_to_coef(float time_ms, float sample_rate) {
    if (time_ms <= 0.0f) return 1.0f;
    float samples = (time_ms / 1000.0f) * sample_rate;
    return 1.0f - expf(-1.0f / samples);
}

/* Apply attack/release envelope to a value */
static void envelope_update(SmoothedValue *sv, float input) {
    sv->raw = input;
    
    /* Calculate velocity (rate of change) */
    sv->velocity = input - sv->smoothed;
    
    /* Apply attack or release based on direction */
    float coef = (input > sv->smoothed) ? sv->attack_coef : sv->release_coef;
    sv->smoothed += coef * (input - sv->smoothed);
    
    /* Update peak with decay */
    if (input > sv->peak) {
        sv->peak = input;
    } else {
        sv->peak *= sv->peak_decay;
    }
    
    /* Clamp to valid range */
    if (sv->smoothed < 0.0f) sv->smoothed = 0.0f;
    if (sv->smoothed > 1.0f) sv->smoothed = 1.0f;
}

/* Initialize a smoothed value with defaults */
static void init_smoothed(SmoothedValue *sv, float attack_ms, float release_ms, float fps) {
    memset(sv, 0, sizeof(*sv));
    sv->attack_coef = time_to_coef(attack_ms, fps);
    sv->release_coef = time_to_coef(release_ms, fps);
    sv->peak_decay = 0.995f;  /* Slow peak decay */
}

/* ============ Creation / Destruction ============ */

ControlBus* control_bus_create(void) {
    ControlBus *bus = calloc(1, sizeof(ControlBus));
    if (!bus) return NULL;
    
    /* Default to 60 FPS timing */
    float fps = 60.0f;
    
    /* Initialize smoothed values with default envelope times */
    init_smoothed(&bus->energy, 5.0f, 80.0f, fps);    /* Fast attack, medium release */
    init_smoothed(&bus->bass, 8.0f, 100.0f, fps);     /* Slightly slower for weight */
    init_smoothed(&bus->mid, 5.0f, 60.0f, fps);       /* Responsive */
    init_smoothed(&bus->treble, 3.0f, 40.0f, fps);    /* Very fast for transients */
    init_smoothed(&bus->onset, 2.0f, 30.0f, fps);     /* Fastest for attack detection */
    
    /* Default configuration */
    bus->silence_threshold = 0.02f;
    bus->onset_sensitivity = 2.0f;
    bus->beat_hit_decay = 0.85f;
    
    /* Initialize beat state */
    bus->beat.bpm = 120.0f;  /* Default BPM */
    
    return bus;
}

void control_bus_destroy(ControlBus *bus) {
    if (bus) free(bus);
}

/* ============ Core Update ============ */

void control_bus_update(ControlBus *bus, 
                        float bass, float mid, float treble,
                        float dt) {
    if (!bus) return;
    
    bus->dt = dt;
    bus->current_time += dt;
    
    /* Clamp inputs to 0-1 */
    if (bass < 0.0f) bass = 0.0f;
    if (bass > 1.0f) bass = 1.0f;
    if (mid < 0.0f) mid = 0.0f;
    if (mid > 1.0f) mid = 1.0f;
    if (treble < 0.0f) treble = 0.0f;
    if (treble > 1.0f) treble = 1.0f;
    
    /* Calculate overall energy (weighted sum) */
    float energy = bass * 0.5f + mid * 0.3f + treble * 0.2f;
    
    /* Calculate onset from energy derivative */
    float energy_delta = energy - bus->prev_energy;
    float onset = 0.0f;
    if (energy_delta > 0.0f) {
        onset = energy_delta * bus->onset_sensitivity;
        if (onset > 1.0f) onset = 1.0f;
    }
    bus->prev_energy = energy;
    
    /* Update smoothed values */
    envelope_update(&bus->energy, energy);
    envelope_update(&bus->bass, bass);
    envelope_update(&bus->mid, mid);
    envelope_update(&bus->treble, treble);
    envelope_update(&bus->onset, onset);
    
    /* Calculate derived signals */
    float total = bass + mid + treble;
    if (total > 0.01f) {
        bus->bass_ratio = bass / total;
        bus->treble_ratio = treble / total;
        bus->brightness = (mid * 0.5f + treble) / total;
    } else {
        bus->bass_ratio = 0.33f;
        bus->treble_ratio = 0.33f;
        bus->brightness = 0.5f;
    }
    
    /* Update energy history for dynamics calculation */
    bus->energy_history[bus->history_idx] = energy;
    bus->history_idx = (bus->history_idx + 1) % 64;
    
    /* Calculate dynamics (variance of recent energy) */
    float mean = 0.0f;
    for (int i = 0; i < 64; i++) {
        mean += bus->energy_history[i];
    }
    mean /= 64.0f;
    
    float variance = 0.0f;
    for (int i = 0; i < 64; i++) {
        float diff = bus->energy_history[i] - mean;
        variance += diff * diff;
    }
    variance /= 64.0f;
    bus->dynamics = sqrtf(variance) * 3.0f;  /* Scale up for usability */
    if (bus->dynamics > 1.0f) bus->dynamics = 1.0f;
    
    /* Silence detection */
    if (bus->energy.smoothed < bus->silence_threshold) {
        bus->silence_time += dt;
        bus->is_silent = (bus->silence_time > 0.3f);  /* 300ms debounce */
    } else {
        bus->silence_time = 0.0f;
        bus->is_silent = false;
    }
    
    /* Decay beat hit */
    bus->beat.hit *= bus->beat_hit_decay;
}

void control_bus_update_beat(ControlBus *bus, 
                             float beat_phase, float bpm,
                             bool beat_detected) {
    if (!bus) return;
    
    bus->beat.phase = beat_phase;
    if (bpm > 30.0f && bpm < 300.0f) {
        bus->beat.bpm = bpm;
    }
    
    /* Check if we're near a beat */
    bus->beat.on_beat = (beat_phase < 0.1f || beat_phase > 0.9f);
    bus->beat.on_half_beat = (beat_phase > 0.45f && beat_phase < 0.55f);
    
    /* Trigger beat hit impulse */
    if (beat_detected) {
        bus->beat.hit = 1.0f;
        bus->beat.beat_count++;
        bus->beat.last_beat = bus->current_time;
    }
}

/* ============ Signal Access ============ */

float control_get_energy(ControlBus *bus) {
    return bus ? bus->energy.smoothed : 0.0f;
}

float control_get_bass(ControlBus *bus) {
    return bus ? bus->bass.smoothed : 0.0f;
}

float control_get_mid(ControlBus *bus) {
    return bus ? bus->mid.smoothed : 0.0f;
}

float control_get_treble(ControlBus *bus) {
    return bus ? bus->treble.smoothed : 0.0f;
}

float control_get_onset(ControlBus *bus) {
    return bus ? bus->onset.smoothed : 0.0f;
}

float control_get_beat_phase(ControlBus *bus) {
    return bus ? bus->beat.phase : 0.0f;
}

float control_get_beat_hit(ControlBus *bus) {
    return bus ? bus->beat.hit : 0.0f;
}

float control_get_bpm(ControlBus *bus) {
    return bus ? bus->beat.bpm : 120.0f;
}

bool control_on_beat(ControlBus *bus) {
    return bus ? bus->beat.on_beat : false;
}

bool control_on_half_beat(ControlBus *bus) {
    return bus ? bus->beat.on_half_beat : false;
}

float control_get_brightness(ControlBus *bus) {
    return bus ? bus->brightness : 0.5f;
}

float control_get_dynamics(ControlBus *bus) {
    return bus ? bus->dynamics : 0.0f;
}

float control_get_bass_ratio(ControlBus *bus) {
    return bus ? bus->bass_ratio : 0.33f;
}

float control_get_treble_ratio(ControlBus *bus) {
    return bus ? bus->treble_ratio : 0.33f;
}

bool control_is_silent(ControlBus *bus) {
    return bus ? bus->is_silent : true;
}

float control_get_silence_time(ControlBus *bus) {
    return bus ? bus->silence_time : 0.0f;
}

/* ============ Smoothing Configuration ============ */

void control_set_smoothing(ControlBus *bus, SmoothingPreset preset) {
    if (!bus) return;
    
    float fps = 60.0f;  /* Assume 60fps */
    
    switch (preset) {
    case SMOOTH_FAST:
        init_smoothed(&bus->energy, 3.0f, 40.0f, fps);
        init_smoothed(&bus->bass, 5.0f, 60.0f, fps);
        init_smoothed(&bus->mid, 3.0f, 40.0f, fps);
        init_smoothed(&bus->treble, 2.0f, 30.0f, fps);
        init_smoothed(&bus->onset, 1.0f, 20.0f, fps);
        break;
        
    case SMOOTH_MEDIUM:
        init_smoothed(&bus->energy, 8.0f, 100.0f, fps);
        init_smoothed(&bus->bass, 10.0f, 120.0f, fps);
        init_smoothed(&bus->mid, 8.0f, 80.0f, fps);
        init_smoothed(&bus->treble, 5.0f, 60.0f, fps);
        init_smoothed(&bus->onset, 3.0f, 40.0f, fps);
        break;
        
    case SMOOTH_SLOW:
        init_smoothed(&bus->energy, 20.0f, 200.0f, fps);
        init_smoothed(&bus->bass, 25.0f, 250.0f, fps);
        init_smoothed(&bus->mid, 20.0f, 180.0f, fps);
        init_smoothed(&bus->treble, 15.0f, 150.0f, fps);
        init_smoothed(&bus->onset, 10.0f, 100.0f, fps);
        break;
        
    case SMOOTH_INSTANT:
        bus->energy.attack_coef = 1.0f;
        bus->energy.release_coef = 1.0f;
        bus->bass.attack_coef = 1.0f;
        bus->bass.release_coef = 1.0f;
        bus->mid.attack_coef = 1.0f;
        bus->mid.release_coef = 1.0f;
        bus->treble.attack_coef = 1.0f;
        bus->treble.release_coef = 1.0f;
        bus->onset.attack_coef = 1.0f;
        bus->onset.release_coef = 1.0f;
        break;
    }
}

void control_configure_envelope(SmoothedValue *val, 
                                float attack_time, float release_time,
                                float sample_rate) {
    if (!val) return;
    val->attack_coef = time_to_coef(attack_time * 1000.0f, sample_rate);
    val->release_coef = time_to_coef(release_time * 1000.0f, sample_rate);
}

/* ============ Utility ============ */

void control_get_raw(ControlBus *bus, 
                     float *energy, float *bass, float *mid, float *treble) {
    if (!bus) return;
    if (energy) *energy = bus->energy.raw;
    if (bass) *bass = bus->bass.raw;
    if (mid) *mid = bus->mid.raw;
    if (treble) *treble = bus->treble.raw;
}

void control_bus_reset(ControlBus *bus) {
    if (!bus) return;
    
    /* Reset smoothed values */
    bus->energy.smoothed = 0.0f;
    bus->energy.raw = 0.0f;
    bus->energy.peak = 0.0f;
    bus->bass.smoothed = 0.0f;
    bus->bass.raw = 0.0f;
    bus->bass.peak = 0.0f;
    bus->mid.smoothed = 0.0f;
    bus->mid.raw = 0.0f;
    bus->mid.peak = 0.0f;
    bus->treble.smoothed = 0.0f;
    bus->treble.raw = 0.0f;
    bus->treble.peak = 0.0f;
    bus->onset.smoothed = 0.0f;
    bus->onset.raw = 0.0f;
    bus->onset.peak = 0.0f;
    
    /* Reset beat state */
    bus->beat.phase = 0.0f;
    bus->beat.hit = 0.0f;
    bus->beat.beat_count = 0;
    
    /* Reset derived */
    bus->prev_energy = 0.0f;
    bus->silence_time = 0.0f;
    bus->is_silent = true;
    
    /* Clear history */
    memset(bus->energy_history, 0, sizeof(bus->energy_history));
    bus->history_idx = 0;
}
