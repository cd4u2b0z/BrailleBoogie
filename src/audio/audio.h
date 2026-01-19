// Audio data structures and input interfaces
// Derived from cava's input/common.h

#pragma once

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Number of samples to read from audio source per channel
#define BUFFER_SIZE 512

// Audio input methods
enum input_method {
    INPUT_PIPEWIRE,
    INPUT_PULSE,
    INPUT_COREAUDIO
};

// Shared audio data structure between input thread and main thread
struct audio_data {
    double *cava_in;           // Input buffer for FFT processing

    int input_buffer_size;     // Size of input buffer
    int cava_buffer_size;      // Size of cava processing buffer

    int format;                // Bit depth (16, 24, 32)
    unsigned int rate;         // Sample rate
    unsigned int channels;     // Number of channels

    int threadparams;          // Flag for thread parameter sync
    char *source;              // Audio source name
    int im;                    // Input method

    int terminate;             // Flag to terminate audio thread
    char error_message[1024];  // Error message buffer
    int samples_counter;       // Number of samples in buffer
    int IEEE_FLOAT;            // 32-bit format type (0=int, 1=float)

    // PipeWire specific
    int active;                // Active monitoring
    int remix;                 // Remix channels
    int virtual_node;          // Virtual node flag

    pthread_mutex_t lock;      // Thread synchronization
};

// Common functions
void reset_output_buffers(struct audio_data *data);
void signal_threadparams(struct audio_data *data);
void signal_terminate(struct audio_data *data);
int write_to_cava_input_buffers(int16_t size, unsigned char *buf, void *data);

// Audio input thread functions
#ifdef PIPEWIRE
void *input_pipewire(void *data);
#endif

#ifdef PULSE
void *input_pulse(void *data);
void getPulseDefaultSink(void *data);
#endif

#ifdef __APPLE__
void *input_coreaudio(void *data);
#endif
