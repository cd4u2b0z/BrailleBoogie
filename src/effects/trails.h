/*
 * Motion Trails for ASCII Dancer
 * 
 * Features:
 * - Store history of joint positions
 * - Render ghost limbs with fading opacity
 * - Trail length based on movement speed
 */

#ifndef TRAILS_H
#define TRAILS_H

#include <stdbool.h>
#include "../braille/braille_canvas.h"
#include "../braille/skeleton_dancer.h"

/* Trail configuration */
#define TRAIL_HISTORY_SIZE 8    /* Number of past positions to store */
#define TRAIL_JOINTS 16         /* Number of joints to track */

/* Trail point */
typedef struct {
    float x, y;
    float alpha;        /* Opacity 0-1 */
    bool valid;         /* Has been set */
} TrailPoint;

/* Joint trail - history for one joint */
typedef struct {
    TrailPoint history[TRAIL_HISTORY_SIZE];
    int write_pos;      /* Circular buffer position */
    float last_x, last_y;
    float velocity;     /* Movement speed (for adaptive trail) */
} JointTrail;

/* Motion trails system */
typedef struct {
    JointTrail joints[TRAIL_JOINTS];
    
    /* Settings */
    float fade_rate;        /* How fast trails fade (0-1 per step) */
    float min_velocity;     /* Minimum speed to show trail */
    int trail_length;       /* How many history points to render */
    bool adaptive_length;   /* Adjust length based on speed */
    
    /* Tracking which joints */
    int tracked_joints[TRAIL_JOINTS];
    int num_tracked;
    
    /* Enable/disable */
    bool enabled;
    
    /* Frame counter for update rate */
    int frame_count;
    int update_interval;    /* Update every N frames */
} MotionTrails;

/* Create/destroy */
MotionTrails* trails_create(void);
void trails_destroy(MotionTrails *trails);

/* Configure which joints to track */
void trails_set_tracked_joints(MotionTrails *trails, int *joint_ids, int count);
void trails_track_all_limbs(MotionTrails *trails);  /* Default: hands, feet, head */

/* Update with current joint positions */
void trails_update(MotionTrails *trails, Joint *joints, int num_joints, float dt);

/* Render trails to canvas */
void trails_render(MotionTrails *trails, BrailleCanvas *canvas);

/* Control */
void trails_clear(MotionTrails *trails);
void trails_set_enabled(MotionTrails *trails, bool enabled);
bool trails_is_enabled(MotionTrails *trails);
void trails_set_length(MotionTrails *trails, int length);
void trails_set_fade_rate(MotionTrails *trails, float rate);

#endif /* TRAILS_H */
