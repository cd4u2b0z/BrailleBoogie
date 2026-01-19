/*
 * Rhythm-based dancer using custom braille frames
 * Focuses on BEAT and FLOW, not raw audio levels
 * Dances in sequences with proper timing
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <math.h>
#include "dancer.h"

#define MAX_FRAMES 8
#define FILE_FRAME_WIDTH 25
#define FILE_FRAME_HEIGHT 13
#define CHARS_PER_LINE 100
#define LINE_BUFFER_SIZE 512

static wchar_t frames[MAX_FRAMES][FILE_FRAME_HEIGHT][FILE_FRAME_WIDTH + 1];
static int num_frames = 0;
static int frame_loaded = 0;

/* Dance sequence - the ORDER frames play for natural movement */
/* Calm dance: frames 0,1,2,3,2,1 (sway back and forth) */
static int calm_sequence[] = {0, 1, 2, 3, 2, 1};
static int calm_seq_len = 6;

/* Energetic dance: all frames in flowing order */
static int energy_sequence[] = {0, 1, 2, 3, 4, 5, 6, 7, 6, 5, 4, 3, 2, 1};
static int energy_seq_len = 14;

static const char* find_data_file(void) {
    static const char* paths[] = {
        "./dancer_frames.txt",
        "/home/craig/projects/asciidancer/dancer_frames.txt",
        NULL
    };
    for (int i = 0; paths[i]; i++) {
        FILE* f = fopen(paths[i], "r");
        if (f) { fclose(f); return paths[i]; }
    }
    return NULL;
}

int dancer_load_frames(void) {
    if (frame_loaded) return num_frames;
    setlocale(LC_ALL, "");
    
    const char* filepath = find_data_file();
    if (!filepath) return -1;
    
    FILE* f = fopen(filepath, "r");
    if (!f) return -1;
    
    wchar_t all_lines[26][CHARS_PER_LINE + 1];
    memset(all_lines, 0, sizeof(all_lines));
    
    char line_buf[LINE_BUFFER_SIZE];
    int line_count = 0;
    
    while (fgets(line_buf, sizeof(line_buf), f) && line_count < 26) {
        size_t len = strlen(line_buf);
        if (len > 0 && line_buf[len-1] == '\n') line_buf[len-1] = '\0';
        mbstowcs(all_lines[line_count], line_buf, CHARS_PER_LINE);
        line_count++;
    }
    fclose(f);
    
    /* Extract 8 frames, trimming 2 chars from left to remove stray dots */
    for (int fr = 0; fr < 4; fr++) {
        for (int row = 0; row < FILE_FRAME_HEIGHT && row < line_count; row++) {
            for (int i = 0; i < FILE_FRAME_WIDTH; i++) frames[fr][row][i] = L'⠀';
            frames[fr][row][FILE_FRAME_WIDTH] = L'\0';
            wcsncpy(frames[fr][row], all_lines[row] + fr * FILE_FRAME_WIDTH + 2, FILE_FRAME_WIDTH - 2);
        }
    }
    for (int fr = 0; fr < 4; fr++) {
        for (int row = 0; row < FILE_FRAME_HEIGHT && (row + 13) < line_count; row++) {
            for (int i = 0; i < FILE_FRAME_WIDTH; i++) frames[fr+4][row][i] = L'⠀';
            frames[fr+4][row][FILE_FRAME_WIDTH] = L'\0';
            wcsncpy(frames[fr+4][row], all_lines[row + 13] + fr * FILE_FRAME_WIDTH + 2, FILE_FRAME_WIDTH - 2);
        }
    }
    
    num_frames = 8;
    frame_loaded = 1;
    return num_frames;
}

void dancer_init(struct dancer_state *state) {
    memset(state, 0, sizeof(*state));
    dancer_load_frames();
}

void dancer_cleanup(void) {}

void dancer_update(struct dancer_state *state, double bass, double mid, double treble) {
    /* Smooth everything heavily for flow */
    state->bass_intensity = state->bass_intensity * 0.85 + bass * 0.15;
    state->mid_intensity = state->mid_intensity * 0.85 + mid * 0.15;
    state->treble_intensity = state->treble_intensity * 0.85 + treble * 0.15;
    
    /* Advance the dance phase - this is the RHYTHM */
    /* Base tempo + energy makes it faster */
    double energy = (state->bass_intensity + state->mid_intensity) / 2.0;
    double tempo = 0.04 + energy * 0.06;  /* Slow, dance-like tempo */
    
    state->phase += tempo;
    
    /* Pick frame from dance sequence based on phase */
    double phase_mod = fmod(state->phase, 1.0);
    
    int seq_pos;
    int frame;
    
    if (energy > 0.4) {
        /* Energetic dance sequence */
        seq_pos = (int)(phase_mod * energy_seq_len) % energy_seq_len;
        frame = energy_sequence[seq_pos];
    } else {
        /* Calm swaying sequence */
        seq_pos = (int)(phase_mod * calm_seq_len) % calm_seq_len;
        frame = calm_sequence[seq_pos];
    }
    
    state->current_frame = frame;
}

void dancer_compose_frame(struct dancer_state *state, char *output) {
    if (!frame_loaded) dancer_load_frames();
    
    int frame = state->current_frame;
    if (frame < 0 || frame >= num_frames) frame = 0;
    
    char *ptr = output;
    for (int row = 0; row < FILE_FRAME_HEIGHT; row++) {
        char line_buf[128];
        wcstombs(line_buf, frames[frame][row], sizeof(line_buf));
        int len = strlen(line_buf);
        memcpy(ptr, line_buf, len);
        ptr += len;
        *ptr++ = '\n';
    }
    *ptr = '\0';
}

void calculate_bands(double *cava_out, int num_bars,
                     double *bass, double *mid, double *treble) {
    int bass_end = num_bars / 3;
    int mid_end = 2 * num_bars / 3;
    
    double b = 0, m = 0, t = 0;
    for (int i = 0; i < num_bars; i++) {
        if (i < bass_end) b += cava_out[i];
        else if (i < mid_end) m += cava_out[i];
        else t += cava_out[i];
    }
    
    *bass = sqrt(b / bass_end);
    *mid = sqrt(m / (mid_end - bass_end));
    *treble = sqrt(t / (num_bars - mid_end));
    
    if (*bass > 1.0) *bass = 1.0;
    if (*mid > 1.0) *mid = 1.0;
    if (*treble > 1.0) *treble = 1.0;
}
