/*
 * Braille Dancer - High-resolution dancer using braille rendering
 * Integrates skeleton animation with the existing dancer interface
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <math.h>
#include "../dancer/dancer.h"
#include "braille_canvas.h"
#include "skeleton_dancer.h"

/* Canvas size in terminal cells */
#define CANVAS_CELLS_W 25
#define CANVAS_CELLS_H 13

static BrailleCanvas *canvas = NULL;
static SkeletonDancer *skeleton = NULL;
static int initialized = 0;

/* Frame timing */
static double last_time = 0.0;

int dancer_load_frames(void) {
    if (initialized) return 1;
    
    setlocale(LC_ALL, "");
    
    /* Create braille canvas */
    canvas = braille_canvas_create(CANVAS_CELLS_W, CANVAS_CELLS_H);
    if (!canvas) return -1;
    
    /* Create skeleton dancer */
    skeleton = skeleton_dancer_create(CANVAS_CELLS_W, CANVAS_CELLS_H);
    if (!skeleton) {
        braille_canvas_destroy(canvas);
        return -1;
    }
    
    initialized = 1;
    return 1;
}

void dancer_init(struct dancer_state *state) {
    memset(state, 0, sizeof(*state));
    dancer_load_frames();
}

void dancer_cleanup(void) {
    if (skeleton) {
        skeleton_dancer_destroy(skeleton);
        skeleton = NULL;
    }
    if (canvas) {
        braille_canvas_destroy(canvas);
        canvas = NULL;
    }
    initialized = 0;
}

void dancer_update(struct dancer_state *state, double bass, double mid, double treble) {
    if (!skeleton) return;
    
    /* Smooth audio input */
    double smooth = 0.88;
    state->bass_intensity = state->bass_intensity * smooth + bass * (1.0 - smooth);
    state->mid_intensity = state->mid_intensity * smooth + mid * (1.0 - smooth);
    state->treble_intensity = state->treble_intensity * smooth + treble * (1.0 - smooth);
    
    /* Calculate dt (approximately 60fps = 0.0167s) */
    float dt = 0.0167f;
    
    /* Update skeleton animation */
    skeleton_dancer_update(skeleton, 
                          (float)state->bass_intensity,
                          (float)state->mid_intensity,
                          (float)state->treble_intensity,
                          dt);
    
    /* Store phase for any external use */
    state->phase = skeleton->phase;
}

void dancer_compose_frame(struct dancer_state *state, char *output) {
    (void)state;
    
    if (!skeleton || !canvas) {
        strcpy(output, "No dancer loaded\n");
        return;
    }
    
    /* Render skeleton to braille canvas */
    skeleton_dancer_render(skeleton, canvas);
    
    /* Convert to UTF-8 output */
    char *ptr = output;
    for (int row = 0; row < canvas->cell_height; row++) {
        int len = braille_canvas_to_utf8(canvas, row, ptr, 256);
        ptr += len;
        *ptr++ = '\n';
    }
    *ptr = '\0';
}

void calculate_bands(double *cava_out, int num_bars,
                     double *bass, double *mid, double *treble) {
    *bass = *mid = *treble = 0.0;
    if (num_bars < 3) return;
    
    int bass_end = num_bars / 3;
    int mid_end = (num_bars * 2) / 3;
    
    for (int i = 0; i < bass_end; i++) *bass += cava_out[i];
    for (int i = bass_end; i < mid_end; i++) *mid += cava_out[i];
    for (int i = mid_end; i < num_bars; i++) *treble += cava_out[i];
    
    *bass /= bass_end;
    *mid /= (mid_end - bass_end);
    *treble /= (num_bars - mid_end);
    
    /* Normalize */
    if (*bass > 1.0) *bass = 1.0;
    if (*mid > 1.0) *mid = 1.0;
    if (*treble > 1.0) *treble = 1.0;
}
