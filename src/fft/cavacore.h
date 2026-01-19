// cavacore FFT processing header
// Derived from cava's cavacore.h

#pragma once

#include <stdint.h>
#include <fftw3.h>

// cava_plan: parameters used internally by cavacore
struct cava_plan {
    int FFTbassbufferSize;
    int FFTbufferSize;
    int number_of_bars;
    int audio_channels;
    int input_buffer_size;
    int rate;
    int bass_cut_off_bar;
    int sens_init;
    int autosens;
    int frame_skip;
    int status;
    char error_message[1024];

    double sens;
    double framerate;
    double noise_reduction;

    fftw_plan p_bass_l, p_bass_r;
    fftw_plan p_l, p_r;

    fftw_complex *out_bass_l, *out_bass_r;
    fftw_complex *out_l, *out_r;

    double *bass_multiplier;
    double *multiplier;

    double *in_bass_r_raw, *in_bass_l_raw;
    double *in_bass_r, *in_bass_l;

    double *in_r_raw, *in_l_raw;
    double *in_r, *in_l;

    double *prev_cava_out;
    double *cava_fall;
    double *cava_mem;
    double *cava_peak;
    double *input_buffer;
    double *eq;
    double *cut_off_frequency;

    int *FFTbuffer_lower_cut_off;
    int *FFTbuffer_upper_cut_off;
};

// Initialize cavacore
// Returns a plan struct that must be passed to cava_execute
// number_of_bars: total number of frequency bars (must be divisible by channels)
// rate: sample rate in Hz
// channels: 1 for mono, 2 for stereo
// autosens: 1 to enable automatic sensitivity adjustment
// noise_reduction: 0.0-1.0, amount of noise reduction
// low_cut_off: lowest frequency to display (Hz)
// high_cut_off: highest frequency to display (Hz)
struct cava_plan *cava_init(int number_of_bars, unsigned int rate, int channels, int autosens,
                            double noise_reduction, int low_cut_off, int high_cut_off);

// Execute FFT and process audio data
// cava_in: input buffer with interleaved samples
// new_samples: number of new samples to process
// cava_out: output buffer (size = number_of_bars)
// plan: the plan struct from cava_init
void cava_execute(double *cava_in, int new_samples, double *cava_out, struct cava_plan *plan);

// Cleanup and free resources
void cava_destroy(struct cava_plan *plan);
