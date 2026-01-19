/*
 * Effects System for ASCII Dancer
 * 
 * Unified header for all visual effects:
 * - Particle system (sparks, dust, etc.)
 * - Motion trails
 * - Visual enhancements (breathing, glow, vibration)
 */

#ifndef EFFECTS_H
#define EFFECTS_H

#include <stdbool.h>
#include "../braille/braille_canvas.h"
#include "../braille/skeleton_dancer.h"

/* Include sub-systems */
#include "particles.h"
#include "trails.h"

/* Visual enhancement settings */
typedef struct {
    /* Breathing animation */
    bool breathing_enabled;
    float breath_phase;         /* 0 to 2*PI */
    float breath_rate;          /* Cycles per second */
    float breath_amplitude;     /* Pixel displacement */
    
    /* Glow effect */
    bool glow_enabled;
    float glow_intensity;       /* 0-1 */
    int glow_offset;            /* Pixel offset for double-draw */
    
    /* Floor vibration */
    bool floor_vibe_enabled;
    float floor_vibe_amount;    /* Current vibration */
    float floor_vibe_decay;     /* Decay rate */
    int floor_y;                /* Floor Y position */
    
    /* Screen shake */
    bool shake_enabled;
    float shake_amount;
    float shake_decay;
    int shake_offset_x;
    int shake_offset_y;
} VisualEnhancements;

/* Combined effects manager */
typedef struct {
    ParticleSystem *particles;
    MotionTrails *trails;
    VisualEnhancements enhancements;
    
    /* Canvas dimensions */
    int canvas_width;
    int canvas_height;
    
    /* Global enable */
    bool enabled;
} EffectsManager;

/* Create/destroy */
EffectsManager* effects_create(int canvas_width, int canvas_height);
void effects_destroy(EffectsManager *fx);

/* Update all effects */
void effects_update(EffectsManager *fx, float dt, float bass, float treble, float energy);

/* Trigger effects based on audio */
void effects_on_bass_hit(EffectsManager *fx, float intensity, float x, float y);
void effects_on_beat(EffectsManager *fx, float intensity, float x, float y);
void effects_on_treble_spike(EffectsManager *fx, float intensity, float x, float y);

/* Get breathing offset for joints */
void effects_get_breathing_offset(EffectsManager *fx, float *dx, float *dy);

/* Get glow rendering info */
bool effects_should_render_glow(EffectsManager *fx);
void effects_get_glow_offset(EffectsManager *fx, int *dx, int *dy);

/* Get floor vibration */
int effects_get_floor_offset(EffectsManager *fx);

/* Get screen shake */
void effects_get_shake_offset(EffectsManager *fx, int *dx, int *dy);

/* Render all effects to canvas */
void effects_render(EffectsManager *fx, BrailleCanvas *canvas);

/* Control */
void effects_set_enabled(EffectsManager *fx, bool enabled);
void effects_set_particles(EffectsManager *fx, bool enabled);
void effects_set_trails(EffectsManager *fx, bool enabled);
void effects_set_breathing(EffectsManager *fx, bool enabled);
void effects_set_glow(EffectsManager *fx, bool enabled);
void effects_set_floor_vibe(EffectsManager *fx, bool enabled);

/* Query state */
bool effects_particles_enabled(EffectsManager *fx);
bool effects_trails_enabled(EffectsManager *fx);
bool effects_breathing_enabled(EffectsManager *fx);

/* v3.0: Get particle system for background effects */
ParticleSystem* effects_get_particle_system(EffectsManager *fx);

#endif /* EFFECTS_H */
