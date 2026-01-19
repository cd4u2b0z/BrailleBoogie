// Render interface
#pragma once

#include "../dancer/dancer.h"

// Initialize rendering
int render_init(void);

// Cleanup rendering
void render_cleanup(void);

// Clear the screen
void render_clear(void);

// Draw the dancer
void render_dancer(struct dancer_state *state);

// Draw the frequency bars
void render_bars(double bass, double mid, double treble);

// Draw frame info (debug info)
void render_frame_info(struct dancer_state *state);

// Refresh the screen
void render_refresh(void);

// Draw info text at bottom
void render_info(const char *text);

// Get keyboard input (non-blocking)
int render_getch(void);
