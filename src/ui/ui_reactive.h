/*
 * UI Reactivity - Audio-reactive terminal UI elements for ASCII Dancer v2.4
 *
 * Provides terminal-safe reactive UI elements:
 * - Border pulse on beat
 * - Energy meter bar
 * - Beat phase indicator
 * - BPM display
 *
 * All rendering uses glyph density only (no color dependence).
 */

#ifndef UI_REACTIVE_H
#define UI_REACTIVE_H

#include <stdbool.h>

/* UI element visibility flags */
typedef struct {
    bool show_border;
    bool show_energy_meter;
    bool show_beat_indicator;
    bool show_bpm;
    bool show_debug;
} UIVisibility;

/* UI state with smoothed values for display */
typedef struct {
    /* Smoothed display values (slower than dancer) */
    float energy_display;
    float bass_display;
    float mid_display;
    float treble_display;
    float beat_phase_display;
    float beat_hit_display;
    float bpm_display;
    
    /* Border pulse state */
    float border_pulse;     /* 0-1, decays after beat hit */
    int border_style;       /* Current border character intensity */
    
    /* Energy meter state */
    float meter_value;      /* Smoothed meter value */
    float meter_peak;       /* Peak hold for meter */
    float peak_hold_time;   /* Time since peak was set */
    
    /* Beat indicator state */
    int beat_frame;         /* Animation frame for beat indicator */
    
    /* Visibility */
    UIVisibility visible;
    
    /* Layout info (set by caller) */
    int screen_width;
    int screen_height;
    int content_x;          /* Content area start X */
    int content_y;          /* Content area start Y */
    int content_width;      /* Content area width */
    int content_height;     /* Content area height */
    
    /* Smoothing parameters */
    float smooth_coef;      /* 0-1, higher = faster response */
} UIReactive;

/* ============ Creation / Destruction ============ */

UIReactive* ui_reactive_create(void);
void ui_reactive_destroy(UIReactive *ui);

/* ============ Update ============ */

/* Update UI state from control bus values
 * Uses slower smoothing than dancer for stable display
 */
void ui_reactive_update(UIReactive *ui,
                        float energy, float bass, float mid, float treble,
                        float beat_phase, float beat_hit, float bpm,
                        float dt);

/* ============ Rendering (ncurses) ============ */

/* Render all visible UI elements
 * Call after clearing screen, before refresh
 */
void ui_reactive_render(UIReactive *ui);

/* Render individual elements (for custom layouts) */
void ui_render_border(UIReactive *ui);
void ui_render_energy_meter(UIReactive *ui, int x, int y, int width);
void ui_render_beat_indicator(UIReactive *ui, int x, int y);
void ui_render_bpm_display(UIReactive *ui, int x, int y);
void ui_render_spectrum_mini(UIReactive *ui, int x, int y, int width);

/* ============ Configuration ============ */

/* Set screen dimensions and content area */
void ui_reactive_set_layout(UIReactive *ui,
                            int screen_width, int screen_height,
                            int content_x, int content_y,
                            int content_width, int content_height);

/* Set visibility of UI elements */
void ui_reactive_set_visible(UIReactive *ui, UIVisibility vis);

/* Toggle individual elements */
void ui_toggle_border(UIReactive *ui);
void ui_toggle_energy_meter(UIReactive *ui);
void ui_toggle_beat_indicator(UIReactive *ui);
void ui_toggle_debug(UIReactive *ui);

/* Set smoothing speed (0 = slow/stable, 1 = fast/responsive) */
void ui_reactive_set_smoothing(UIReactive *ui, float speed);

/* ============ Utility ============ */

/* Get border character for given pulse intensity */
const char* ui_get_border_char(int style, bool is_corner, int corner_type);

/* Get bar character for given fill level (0-8 for sub-character resolution) */
const char* ui_get_bar_char(int level);

#endif /* UI_REACTIVE_H */
