/*
 * Frame Recorder - ASCII Dancer v3.0+
 * 
 * Capture terminal frames to PNG sequence for GIF/video export
 * Supports ANSI escape sequences and true color
 */

#ifndef FRAME_RECORDER_H
#define FRAME_RECORDER_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool recording;
    char output_dir[256];
    int frame_number;
    int width;
    int height;
    
    /* Frame buffer - stores ANSI colored text */
    char **frame_buffer;
    
    /* Recording statistics */
    int total_frames;
    double start_time;
    double duration;
    
} FrameRecorder;

/* Create recorder */
FrameRecorder* frame_recorder_create(int width, int height, const char *output_dir);

/* Destroy recorder */
void frame_recorder_destroy(FrameRecorder *recorder);

/* Start/stop recording */
void frame_recorder_start(FrameRecorder *recorder);
void frame_recorder_stop(FrameRecorder *recorder);

/* Check if recording */
bool frame_recorder_is_recording(FrameRecorder *recorder);

/* Capture current terminal frame */
void frame_recorder_capture(FrameRecorder *recorder);

/* Export captured frames to text files (for post-processing with asciinema or similar) */
void frame_recorder_export_text(FrameRecorder *recorder, const char *filename);

/* Get recording stats */
void frame_recorder_get_stats(FrameRecorder *recorder, int *frames, double *duration);

#endif /* FRAME_RECORDER_H */
