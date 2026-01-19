/*
 * Advanced Skeleton Dancer - Rich animation with audio-reactive behavior
 * 
 * Features:
 * - 30+ poses across different energy levels
 * - Frequency-specific reactions (bass = lower body, treble = arms/hands)
 * - Beat detection for rhythmic movement
 * - Genre-aware animation styles
 * - Procedural variation to avoid repetition
 * - Physics-based momentum and follow-through
 */

#ifndef SKELETON_DANCER_H
#define SKELETON_DANCER_H

#include "braille_canvas.h"
#include <stdbool.h>

#define MAX_JOINTS 16
#define MAX_BONES 20
#define MAX_POSES 64
#define POSE_HISTORY 8

/* Joint IDs for humanoid skeleton */
typedef enum {
    JOINT_HEAD = 0,
    JOINT_NECK,
    JOINT_SHOULDER_L,
    JOINT_SHOULDER_R,
    JOINT_ELBOW_L,
    JOINT_ELBOW_R,
    JOINT_HAND_L,
    JOINT_HAND_R,
    JOINT_HIP_CENTER,
    JOINT_HIP_L,
    JOINT_HIP_R,
    JOINT_KNEE_L,
    JOINT_KNEE_R,
    JOINT_FOOT_L,
    JOINT_FOOT_R,
    JOINT_COUNT
} JointID;

/* Pose categories for different contexts */
typedef enum {
    POSE_CAT_IDLE = 0,      /* Very low energy, subtle movement */
    POSE_CAT_CALM,          /* Low energy, gentle swaying */
    POSE_CAT_GROOVE,        /* Medium energy, rhythmic movement */
    POSE_CAT_ENERGETIC,     /* High energy, active dancing */
    POSE_CAT_INTENSE,       /* Very high energy, jumping/wild */
    POSE_CAT_BASS_HIT,      /* Reactive to bass hits */
    POSE_CAT_TREBLE_ACCENT, /* Reactive to treble/hi-hats */
    POSE_CAT_COUNT
} PoseCategory;

/* Detected music style hints */
typedef enum {
    STYLE_UNKNOWN = 0,
    STYLE_ELECTRONIC,   /* Heavy bass, repetitive */
    STYLE_ROCK,         /* Balanced, driving */
    STYLE_HIPHOP,       /* Strong bass, rhythmic */
    STYLE_AMBIENT,      /* Low energy, flowing */
    STYLE_CLASSICAL,    /* Dynamic range, melodic */
    STYLE_COUNT
} MusicStyle;

/* A single joint position */
typedef struct {
    float x, y;
} Joint;

/* A bone connects two joints */
typedef struct {
    JointID from;
    JointID to;
    int thickness;
    bool is_curve;
    float curve_amount;
} Bone;

/* A pose with metadata */
typedef struct {
    Joint joints[MAX_JOINTS];
    int num_joints;
    char name[32];
    PoseCategory category;
    float energy_min;       /* Minimum energy to use this pose */
    float energy_max;       /* Maximum energy for this pose */
    float bass_affinity;    /* How much this pose suits bass-heavy music */
    float treble_affinity;  /* How much this pose suits treble-heavy music */
} Pose;

/* Skeleton definition */
typedef struct {
    Bone bones[MAX_BONES];
    int num_bones;
    int head_radius;
} SkeletonDef;

/* Beat detection state */
typedef struct {
    float energy_history[64];
    int history_idx;
    float beat_threshold;
    float last_beat_time;
    float bpm_estimate;
    int beat_count;
    bool beat_detected;
    float time_since_beat;
} BeatDetector;

/* Audio analysis state */
typedef struct {
    /* Smoothed frequency bands */
    float bass;
    float bass_smooth;
    float bass_peak;
    float bass_velocity;
    
    float mid;
    float mid_smooth;
    float mid_peak;
    float mid_velocity;
    
    float treble;
    float treble_smooth;
    float treble_peak;
    float treble_velocity;
    
    /* Overall metrics */
    float energy;
    float energy_smooth;
    float energy_long;      /* Long-term average for comparison */
    float dynamics;         /* How much energy varies */
    
    /* Derived features */
    float bass_ratio;       /* Bass relative to total */
    float treble_ratio;     /* Treble relative to total */
    float spectral_centroid; /* Brightness measure */
    
    /* Beat info */
    BeatDetector beat;
    
    /* Style detection */
    MusicStyle detected_style;
    float style_confidence;
} AudioAnalysis;

/* Joint physics for smooth motion */
typedef struct {
    Joint position;
    Joint velocity;
    Joint target;
    float stiffness;    /* How quickly it follows target */
    float damping;      /* How much velocity decays */
} JointPhysics;

/* Main dancer state */
typedef struct {
    /* Current interpolated pose with physics */
    Joint current[MAX_JOINTS];
    JointPhysics physics[MAX_JOINTS];
    
    /* Pose blending */
    int pose_primary;
    int pose_secondary;
    float blend;
    
    /* Pose history to avoid repetition */
    int pose_history[POSE_HISTORY];
    int history_idx;
    
    /* Animation timing */
    float phase;
    float tempo;
    float time_total;
    float time_in_pose;
    float pose_duration;
    
    /* Modifiers (added on top of base pose) */
    float head_bob;
    float arm_swing_l;
    float arm_swing_r;
    float hip_sway;
    float bounce;
    float lean;
    
    /* Audio analysis */
    AudioAnalysis audio;
    
    /* Pose library */
    Pose poses[MAX_POSES];
    int num_poses;
    int poses_by_category[POSE_CAT_COUNT][MAX_POSES];
    int category_counts[POSE_CAT_COUNT];
    
    /* Skeleton definition */
    SkeletonDef skeleton;
    
    /* Canvas info */
    int canvas_width;
    int canvas_height;
    float scale;
    float offset_x;
    float offset_y;
    
    /* Random seed for variation */
    unsigned int random_state;
} SkeletonDancer;

/* ============ Creation/Destruction ============ */
SkeletonDancer* skeleton_dancer_create(int canvas_cell_width, int canvas_cell_height);
void skeleton_dancer_destroy(SkeletonDancer *dancer);

/* ============ Animation ============ */
void skeleton_dancer_update(SkeletonDancer *dancer, 
                            float bass, float mid, float treble,
                            float dt);

/* ============ Rendering ============ */
void skeleton_dancer_render(SkeletonDancer *dancer, BrailleCanvas *canvas);

/* ============ Utilities ============ */
float ease_in_out_quad(float t);
float ease_in_out_cubic(float t);
float ease_in_out_elastic(float t);
Joint joint_lerp(Joint a, Joint b, float t);

#endif /* SKELETON_DANCER_H */
