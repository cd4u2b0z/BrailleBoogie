/*
 * Dancer implementation using custom pre-made Braille art frames
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "dancer.h"

void dancer_init(struct dancer_state *state) {
    memset(state, 0, sizeof(*state));
    state->current_frame = 0;
    
    /* Load the custom frames */
    int n = dancer_load_frames();
    if (n < 0) {
        fprintf(stderr, "Warning: Could not load custom frames\n");
    }
}

void dancer_update(struct dancer_state *state, double bass, double mid, double treble) {
    /* Smooth the values with exponential moving average */
    double smooth = 0.3;
    state->bass_intensity = state->bass_intensity * (1.0 - smooth) + bass * smooth;
    state->mid_intensity = state->mid_intensity * (1.0 - smooth) + mid * smooth;
    state->treble_intensity = state->treble_intensity * (1.0 - smooth) + treble * smooth;
    
    /* Select frame based on audio */
    state->current_frame = dancer_select_frame(
        (float)state->bass_intensity,
        (float)state->mid_intensity,
        (float)state->treble_intensity
    );
}

void dancer_compose_frame(struct dancer_state *state, char *output) {
    int height = dancer_get_frame_height();
    char *ptr = output;
    
    for (int row = 0; row < height; row++) {
        char line_buf[256];
        dancer_frame_to_utf8(state->current_frame, row, line_buf, sizeof(line_buf));
        
        /* Copy line and add newline */
        int len = strlen(line_buf);
        memcpy(ptr, line_buf, len);
        ptr += len;
        *ptr++ = '\n';
    }
    *ptr = '\0';
}

void calculate_bands(double *cava_out, int num_bars,
                     double *bass, double *mid, double *treble) {
    /* Split bars into bass, mid, treble */
    int bass_end = num_bars / 3;
    int mid_end = 2 * num_bars / 3;
    
    double bass_sum = 0, mid_sum = 0, treble_sum = 0;
    int bass_count = 0, mid_count = 0, treble_count = 0;
    
    for (int i = 0; i < num_bars; i++) {
        double val = cava_out[i];
        
        if (i < bass_end) {
            bass_sum += val;
            bass_count++;
        } else if (i < mid_end) {
            mid_sum += val;
            mid_count++;
        } else {
            treble_sum += val;
            treble_count++;
        }
    }
    
    /* Normalize to 0.0 - 1.0 range */
    *bass = (bass_count > 0) ? bass_sum / bass_count : 0.0;
    *mid = (mid_count > 0) ? mid_sum / mid_count : 0.0;
    *treble = (treble_count > 0) ? treble_sum / treble_count : 0.0;
    
    /* Apply some compression to make it more reactive */
    *bass = sqrt(*bass);
    *mid = sqrt(*mid);
    *treble = sqrt(*treble);
    
    /* Clamp */
    if (*bass > 1.0) *bass = 1.0;
    if (*mid > 1.0) *mid = 1.0;
    if (*treble > 1.0) *treble = 1.0;
}

void dancer_cleanup(void) {
    /* Nothing to cleanup for static frames */
}
