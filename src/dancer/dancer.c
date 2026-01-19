// Dancer animation logic
// Maps frequency bands to body part movements

#include "dancer.h"
#include <string.h>
#include <math.h>

// Smoothing factor for intensity changes (lower = smoother)
#define SMOOTHING 0.3

void dancer_init(struct dancer_state *state) {
    state->legs = 0;
    state->torso = 0;
    state->arms = 0;
    state->bass_intensity = 0.0;
    state->mid_intensity = 0.0;
    state->treble_intensity = 0.0;
}

void dancer_update(struct dancer_state *state, double bass, double mid, double treble) {
    // Smooth the intensity values for natural movement
    state->bass_intensity = state->bass_intensity * (1.0 - SMOOTHING) + bass * SMOOTHING;
    state->mid_intensity = state->mid_intensity * (1.0 - SMOOTHING) + mid * SMOOTHING;
    state->treble_intensity = state->treble_intensity * (1.0 - SMOOTHING) + treble * SMOOTHING;

    // Map intensities to frame indices
    // Use thresholds for more distinct poses

    // Legs respond to bass (0-300Hz)
    if (state->bass_intensity > 0.7) {
        state->legs = 3;  // Wide jump
    } else if (state->bass_intensity > 0.4) {
        state->legs = 2;  // Step out
    } else if (state->bass_intensity > 0.2) {
        state->legs = 1;  // Slight bend
    } else {
        state->legs = 0;  // Standing
    }

    // Torso responds to mids (300-2000Hz)
    if (state->mid_intensity > 0.6) {
        state->torso = 2;  // Lean forward
    } else if (state->mid_intensity > 0.3) {
        state->torso = 1;  // Slight lean
    } else {
        state->torso = 0;  // Upright
    }

    // Arms respond to treble (2000Hz+)
    if (state->treble_intensity > 0.7) {
        state->arms = 3;  // Both up
    } else if (state->treble_intensity > 0.5) {
        state->arms = 2;  // One up
    } else if (state->treble_intensity > 0.25) {
        state->arms = 1;  // Out
    } else {
        state->arms = 0;  // Down
    }
}

void dancer_compose_frame(struct dancer_state *state, char *output) {
    const char **arms = get_arms_frame(state->arms);
    const char **torso = get_torso_frame(state->torso);
    const char **legs = get_legs_frame(state->legs);

    // Arms: lines 0-3
    // Torso: lines 4-7
    // Legs: lines 8-11

    char *ptr = output;
    for (int i = 0; i < 4; i++) {
        strcpy(ptr, arms[i]);
        ptr += strlen(arms[i]);
        *ptr++ = '\n';
    }
    for (int i = 0; i < 4; i++) {
        strcpy(ptr, torso[i]);
        ptr += strlen(torso[i]);
        *ptr++ = '\n';
    }
    for (int i = 0; i < 4; i++) {
        strcpy(ptr, legs[i]);
        ptr += strlen(legs[i]);
        *ptr++ = '\n';
    }
    *ptr = '\0';
}

void calculate_bands(double *cava_out, int num_bars,
                     double *bass, double *mid, double *treble) {
    // Split bars into three bands
    // Cava bars are ordered from low to high frequency
    int bass_end = num_bars / 3;
    int mid_end = 2 * num_bars / 3;

    double bass_sum = 0.0;
    double mid_sum = 0.0;
    double treble_sum = 0.0;

    // Sum and average each band
    for (int i = 0; i < bass_end; i++) {
        bass_sum += cava_out[i];
    }
    for (int i = bass_end; i < mid_end; i++) {
        mid_sum += cava_out[i];
    }
    for (int i = mid_end; i < num_bars; i++) {
        treble_sum += cava_out[i];
    }

    // Average and clamp to 0-1
    *bass = bass_sum / bass_end;
    *mid = mid_sum / (mid_end - bass_end);
    *treble = treble_sum / (num_bars - mid_end);

    // Apply some gain and clamp
    *bass = fmin(1.0, *bass * 1.5);
    *mid = fmin(1.0, *mid * 1.5);
    *treble = fmin(1.0, *treble * 2.0);  // Treble often quieter
}
