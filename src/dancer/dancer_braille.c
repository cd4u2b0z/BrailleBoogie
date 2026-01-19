// High-quality Braille dancer with procedural animation
// Draws a stick figure that moves smoothly based on audio

#include "dancer.h"
#include "braille.h"
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Canvas dimensions (in braille characters)
// Actual pixel resolution is 2x width by 4x height
#define CANVAS_CHARS_W 30
#define CANVAS_CHARS_H 15

// Pixel dimensions
#define CANVAS_W (CANVAS_CHARS_W * 2)  // 60 pixels wide
#define CANVAS_H (CANVAS_CHARS_H * 4)  // 60 pixels tall

// Body proportions (in pixels)
#define HEAD_RADIUS 4
#define TORSO_LENGTH 16
#define UPPER_ARM_LENGTH 10
#define LOWER_ARM_LENGTH 8
#define UPPER_LEG_LENGTH 12
#define LOWER_LEG_LENGTH 10

// Smoothing factor (0-1, lower = smoother)
#define SMOOTHING 0.25

// Animation state
static double smooth_bass = 0;
static double smooth_mid = 0;
static double smooth_treble = 0;
static double phase = 0;  // Animation phase for continuous motion

// Static canvas
static struct braille_canvas *canvas = NULL;

void dancer_init(struct dancer_state *state) {
    state->legs = 0;
    state->torso = 0;
    state->arms = 0;
    state->bass_intensity = 0.0;
    state->mid_intensity = 0.0;
    state->treble_intensity = 0.0;
    
    smooth_bass = 0;
    smooth_mid = 0;
    smooth_treble = 0;
    phase = 0;
    
    if (!canvas) {
        canvas = braille_create(CANVAS_CHARS_W, CANVAS_CHARS_H);
    }
}

// Draw a thick line (multiple parallel lines)
static void draw_thick_line(int x0, int y0, int x1, int y1, int thickness) {
    braille_line(canvas, x0, y0, x1, y1);
    if (thickness > 1) {
        // Add parallel lines for thickness
        int dx = y1 - y0;
        int dy = -(x1 - x0);
        double len = sqrt(dx * dx + dy * dy);
        if (len > 0) {
            dx = (int)(dx / len);
            dy = (int)(dy / len);
            for (int t = 1; t < thickness; t++) {
                braille_line(canvas, x0 + dx * t, y0 + dy * t, x1 + dx * t, y1 + dy * t);
                braille_line(canvas, x0 - dx * t, y0 - dy * t, x1 - dx * t, y1 - dy * t);
            }
        }
    }
}

// Draw the dancer figure
static void draw_dancer(double bass, double mid, double treble) {
    braille_clear(canvas);
    
    // Center point
    int cx = CANVAS_W / 2;
    int base_y = 8;  // Head position from top
    
    // Animation parameters based on audio
    double bounce = sin(phase * 2) * bass * 6;          // Vertical bounce from bass
    double sway = sin(phase) * mid * 8;                  // Horizontal sway from mids
    double arm_wave = treble * M_PI * 0.8;               // Arm raise from treble
    double leg_spread = bass * 12;                       // Leg spread from bass
    double lean = sin(phase * 0.5) * mid * 0.3;          // Body lean
    
    // Head position (with bounce and sway)
    int head_x = cx + (int)sway;
    int head_y = base_y + (int)bounce;
    
    // Draw head
    braille_filled_circle(canvas, head_x, head_y, HEAD_RADIUS);
    
    // Neck point
    int neck_x = head_x;
    int neck_y = head_y + HEAD_RADIUS + 2;
    
    // Hip point (torso with lean)
    int hip_x = neck_x + (int)(sin(lean) * TORSO_LENGTH);
    int hip_y = neck_y + (int)(cos(lean) * TORSO_LENGTH);
    
    // Draw torso (spine)
    draw_thick_line(neck_x, neck_y, hip_x, hip_y, 1);
    
    // Shoulder point (slightly below neck)
    int shoulder_y = neck_y + 3;
    int shoulder_x = neck_x + (int)(sin(lean) * 3);
    
    // === ARMS ===
    // Left arm angles
    double left_upper_angle = M_PI / 2 + M_PI / 6 - arm_wave + sin(phase * 1.5) * 0.3;
    double left_lower_angle = left_upper_angle + M_PI / 6 + arm_wave * 0.5;
    
    // Right arm angles (opposite phase)
    double right_upper_angle = M_PI / 2 - M_PI / 6 + arm_wave - sin(phase * 1.5 + M_PI) * 0.3;
    double right_lower_angle = right_upper_angle - M_PI / 6 - arm_wave * 0.5;
    
    // Left arm
    int left_elbow_x = shoulder_x - (int)(cos(left_upper_angle) * UPPER_ARM_LENGTH);
    int left_elbow_y = shoulder_y + (int)(sin(left_upper_angle) * UPPER_ARM_LENGTH);
    int left_hand_x = left_elbow_x - (int)(cos(left_lower_angle) * LOWER_ARM_LENGTH);
    int left_hand_y = left_elbow_y + (int)(sin(left_lower_angle) * LOWER_ARM_LENGTH);
    
    draw_thick_line(shoulder_x, shoulder_y, left_elbow_x, left_elbow_y, 1);
    draw_thick_line(left_elbow_x, left_elbow_y, left_hand_x, left_hand_y, 1);
    
    // Right arm
    int right_elbow_x = shoulder_x + (int)(cos(right_upper_angle) * UPPER_ARM_LENGTH);
    int right_elbow_y = shoulder_y + (int)(sin(right_upper_angle) * UPPER_ARM_LENGTH);
    int right_hand_x = right_elbow_x + (int)(cos(right_lower_angle) * LOWER_ARM_LENGTH);
    int right_hand_y = right_elbow_y + (int)(sin(right_lower_angle) * LOWER_ARM_LENGTH);
    
    draw_thick_line(shoulder_x, shoulder_y, right_elbow_x, right_elbow_y, 1);
    draw_thick_line(right_elbow_x, right_elbow_y, right_hand_x, right_hand_y, 1);
    
    // === LEGS ===
    // Leg angles based on bass and phase
    double leg_phase = phase * 2;
    double left_leg_angle = M_PI / 2 + sin(leg_phase) * (0.3 + bass * 0.4);
    double right_leg_angle = M_PI / 2 + sin(leg_phase + M_PI) * (0.3 + bass * 0.4);
    
    // Spread legs apart based on bass
    int leg_offset = (int)(leg_spread / 2);
    
    // Left leg
    int left_hip_x = hip_x - 3 - leg_offset;
    int left_knee_x = left_hip_x + (int)(sin(left_leg_angle - M_PI/2) * UPPER_LEG_LENGTH);
    int left_knee_y = hip_y + (int)(cos(left_leg_angle - M_PI/2) * UPPER_LEG_LENGTH);
    int left_foot_x = left_knee_x + (int)(sin(left_leg_angle - M_PI/2 + 0.3) * LOWER_LEG_LENGTH);
    int left_foot_y = left_knee_y + (int)(cos(left_leg_angle - M_PI/2 + 0.3) * LOWER_LEG_LENGTH);
    
    draw_thick_line(left_hip_x, hip_y, left_knee_x, left_knee_y, 1);
    draw_thick_line(left_knee_x, left_knee_y, left_foot_x, left_foot_y, 1);
    
    // Right leg
    int right_hip_x = hip_x + 3 + leg_offset;
    int right_knee_x = right_hip_x + (int)(sin(right_leg_angle - M_PI/2) * UPPER_LEG_LENGTH);
    int right_knee_y = hip_y + (int)(cos(right_leg_angle - M_PI/2) * UPPER_LEG_LENGTH);
    int right_foot_x = right_knee_x + (int)(sin(right_leg_angle - M_PI/2 - 0.3) * LOWER_LEG_LENGTH);
    int right_foot_y = right_knee_y + (int)(cos(right_leg_angle - M_PI/2 - 0.3) * LOWER_LEG_LENGTH);
    
    draw_thick_line(right_hip_x, hip_y, right_knee_x, right_knee_y, 1);
    draw_thick_line(right_knee_x, right_knee_y, right_foot_x, right_foot_y, 1);
    
    // Draw small circles at joints for better look
    braille_filled_circle(canvas, left_elbow_x, left_elbow_y, 1);
    braille_filled_circle(canvas, right_elbow_x, right_elbow_y, 1);
    braille_filled_circle(canvas, left_knee_x, left_knee_y, 1);
    braille_filled_circle(canvas, right_knee_x, right_knee_y, 1);
}

void dancer_update(struct dancer_state *state, double bass, double mid, double treble) {
    // Smooth the inputs
    smooth_bass = smooth_bass * (1.0 - SMOOTHING) + bass * SMOOTHING;
    smooth_mid = smooth_mid * (1.0 - SMOOTHING) + mid * SMOOTHING;
    smooth_treble = smooth_treble * (1.0 - SMOOTHING) + treble * SMOOTHING;
    
    // Store in state for external access
    state->bass_intensity = smooth_bass;
    state->mid_intensity = smooth_mid;
    state->treble_intensity = smooth_treble;
    
    // Advance animation phase (speed based on overall energy)
    double energy = (smooth_bass + smooth_mid + smooth_treble) / 3.0;
    phase += 0.15 + energy * 0.25;  // Base speed + audio-reactive speed
    
    // Map to discrete states for compatibility
    state->legs = (int)(smooth_bass * 3.99);
    state->torso = (int)(smooth_mid * 2.99);
    state->arms = (int)(smooth_treble * 3.99);
}

void dancer_compose_frame(struct dancer_state *state, char *output) {
    if (!canvas) {
        strcpy(output, "Canvas not initialized\n");
        return;
    }
    
    // Draw the dancer with current smooth values
    draw_dancer(smooth_bass, smooth_mid, smooth_treble);
    
    // Render to output
    braille_render(canvas, output);
    
    (void)state;  // State already used in update
}

void calculate_bands(double *cava_out, int num_bars,
                     double *bass, double *mid, double *treble) {
    // Use more bars for bass since it's more important for dancing
    int bass_end = num_bars / 3;
    int mid_end = 2 * num_bars / 3;

    double bass_sum = 0.0;
    double mid_sum = 0.0;
    double treble_sum = 0.0;
    double bass_max = 0.0;
    double mid_max = 0.0;
    double treble_max = 0.0;

    // Sum and find max in each band
    for (int i = 0; i < bass_end; i++) {
        bass_sum += cava_out[i];
        if (cava_out[i] > bass_max) bass_max = cava_out[i];
    }
    for (int i = bass_end; i < mid_end; i++) {
        mid_sum += cava_out[i];
        if (cava_out[i] > mid_max) mid_max = cava_out[i];
    }
    for (int i = mid_end; i < num_bars; i++) {
        treble_sum += cava_out[i];
        if (cava_out[i] > treble_max) treble_max = cava_out[i];
    }

    // Use combination of average and max for better response
    double bass_avg = bass_sum / bass_end;
    double mid_avg = mid_sum / (mid_end - bass_end);
    double treble_avg = treble_sum / (num_bars - mid_end);
    
    *bass = (bass_avg * 0.5 + bass_max * 0.5);
    *mid = (mid_avg * 0.5 + mid_max * 0.5);
    *treble = (treble_avg * 0.5 + treble_max * 0.5);

    // Apply gain curves
    *bass = fmin(1.0, *bass * 2.5);
    *mid = fmin(1.0, *mid * 2.0);
    *treble = fmin(1.0, *treble * 3.0);
}

// Cleanup function
void dancer_cleanup(void) {
    if (canvas) {
        braille_free(canvas);
        canvas = NULL;
    }
}
