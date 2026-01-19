// Dancer animation system
// Rhythm-based animation using custom Braille art frames

#pragma once

// Frame dimensions
#define FRAME_WIDTH 25
#define FRAME_HEIGHT 13

// Dancer state
struct dancer_state {
    int current_frame;
    double bass_intensity;
    double mid_intensity;
    double treble_intensity;
    double phase;  // Dance rhythm phase
};

void dancer_init(struct dancer_state *state);
void dancer_update(struct dancer_state *state, double bass, double mid, double treble);
void dancer_compose_frame(struct dancer_state *state, char *output);
void calculate_bands(double *cava_out, int num_bars, double *bass, double *mid, double *treble);
void dancer_cleanup(void);
